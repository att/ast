/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1995-2013 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Editor (snarfed from v10, now posix compliant, no hard limits)
 */

static const char usage[] =
"[-?\n@(#)$Id: ed (AT&T Research) 2004-06-08 $\n]"
USAGE_LICENSE
"[+NAME?ed - edit text]"
"[+DESCRIPTION?\bed\b is a line-oriented text editor that has two modes:"
"	command mode and input mode. In command mode characters on the"
"	standard input are interpreted as commands, and in input mode they"
"	are interpreted as text.]"

"[h:explain?Explain the details of error conditions rather than the default"
"	``\b?\b'' on the standard error.]"
"[o:output?Write output to the standard output and error messages to the"
"	standard error. By default output is written to the file being edited"
"	and error messages are printed on the standard output.]"
"[p:prompt?Sets the command line prompt to \astring\a. The default is"
"	no prompt.]:[string]"
"[q:test?For testing; enable verbose messages and reset the \bQUIT\b signal"
"	handler to the default action.]"
"[s:silent?Disable verbose messages.]"
"[O:lenient?Enable lenient regular expression interpretation."
"	This is the default if \bgetconf CONFORMANCE\b is not \bstandard\b.]"
"[S:strict?Enable strict regular expression interpretation. This is the"
"	default if \bgetconf CONFORMANCE\b is \bstandard\b. You'd be"
"	suprised what the lenient mode lets by.]"

"\n"
"\n[ file ]\n"
"\n"

"[+SEE ALSO?\bsed\b(1), \bregex\b(3)]"
;

#include <ast.h>
#include <error.h>
#include <ls.h>
#include <sfdisc.h>
#include <sig.h>

#include <ctype.h>
#include <regex.h>
#include <setjmp.h>

#define BLOCK_LINE	1024
#define BLOCK_TMP	(8*SF_BUFSIZE)

#define BREAK_PAGE	23
#define BREAK_LINE	72

#define LINE_GLOBAL	((off_t)((off_t)1)<<(CHAR_BIT*sizeof(off_t)-1))
#define LINE_MARKED	((off_t)((off_t)1)<<(CHAR_BIT*sizeof(off_t)-2))
#define LINE_NONE	((off_t)-1)

#define MARK_MIN	'a'
#define MARK_MAX	'z'

#define MATCH_MIN	'0'
#define MATCH_MAX	'9'

#define REC_IGNORE	001
#define REC_LINE	002
#define REC_SPLICE	004
#define REC_TERMINATE	010
#define REC_TEXT	020

#define BEG(n)		(ed.line+ed.match[n].rm_so)
#define BAS()		(ed.base)
#define CUR()		(ed.line)
#define END(n)		(ed.line+ed.match[n].rm_eo)
#define HIT(n)		(ed.match[n].rm_eo!=-1)
#define NUL(n)		(ed.match[n].rm_so==ed.match[n].rm_eo)
#define NXT()		(ed.line+=ed.match[0].rm_eo)
#define SET(p,n)	(ed.line=(ed.base=(p))+(n))
#define SWP(a,b)	(ed.swp=(a),(a)=(b),(b)=ed.swp)

#define error		fatal
#define errorf		fatalf
#define splice		splicef
#define trap()		do{if(ed.caught)handle();}while(0)

typedef struct
{
	off_t		offset;
	off_t		undo;
	unsigned long	event;
} Line_t;

static int		signals[] = { SIGQUIT, SIGHUP, SIGINT, SIGTERM };

static struct		/* program state -- no other dynamic globals */
{
	struct
	{
	Sfio_t*		file;
	Sfio_t*		global;
	Sfio_t*		help;
	Sfio_t*		line;
	Sfio_t*		prompt;
	Sfio_t*		query;
	Sfio_t*		shell;
	Sfio_t*		work;
	}		buffer;
	struct
	{
	int		print;
	int		size;
	}		page;
	struct
	{
	off_t		marks[MARK_MAX - MARK_MIN + 1];
	unsigned long	dol;
	unsigned long	dot;
	}		undo;
	Line_t*		addr1;
	Line_t*		addr2;
	Line_t*		dol;
	Line_t*		dot;
	Line_t*		zero;
	Sfio_t*		iop;
	Sfio_t*		msg;
	Sfio_t*		tmp;
	Sfio_t*		spl;
	char*		base;
	char*		spbeg;
	char*		spend;
	char*		global;
	char*		input;
	char*		line;
	char*		linebreak;
	char*		tmpfile;
	int		caught;
	int		compiled;
	int		evented;
	int		given;
	int		help;
	int		initialized;
	int		interactive;
	int		lastc;
	int		marked;
	int		modified;
	int		peekc;
	int		pending;
	int		print;
	int		prompt;
	int		reflags;
	int		restricted;
	int		verbose;
	int		warn_newline;
	int		warn_null;
	jmp_buf		again;
	off_t		marks[MARK_MAX - MARK_MIN + 1];
	off_t		tmpoff;
	regex_t		re;
	regdisc_t	redisc;
	regmatch_t	match[MATCH_MAX - MATCH_MIN + 1];
	unsigned long	all;
	unsigned long	bytes;
	unsigned long	event;
	unsigned long	lines;
	Sfio_t*		swp;
}			ed;

#define REG_SUB_LIST	REG_SUB_USER

static const regflags_t	submap[] =
{
	'g',	REG_SUB_ALL,
	'l',	REG_SUB_LIST,
	'n',	REG_SUB_NUMBER,
	'p',	REG_SUB_PRINT,
	'L',	REG_SUB_LOWER,
	'U',	REG_SUB_UPPER,
	0,	0
};

static void		commands(void);
static void		handle(void);
static void		quit(int);

static void
eat(void)
{
	if (ed.global)
	{
		if (!*ed.global++)
			ed.global = 0;
		ed.lastc = '\n';
	}
	else
		ed.input = 0;
}

static void
reset(int level)
{
	if (level >= 2) {
		if (ed.iop) {
			sfclose(ed.iop);
			ed.iop = 0;
			error_info.file = 0;
		}
		if (ed.interactive <= 0 && (ed.interactive = isatty(0)) <= 0)
			quit(1);
		ed.print = 0;
		ed.bytes = 0;
		ed.lines = 0;
		eat();
		if (ed.initialized)
			longjmp(ed.again, 1);
	}
}

static void
error(int level, ...)
{
	va_list		ap;

	trap();
	va_start(ap, level);
	errorv(NiL, level, ap);
	va_end(ap);
	reset(level);
}

static int
errorf(const regex_t* re, regdisc_t* disc, int level, ...)
{
	va_list		ap;

	trap();
	va_start(ap, level);
	errorv(NiL, level, ap);
	va_end(ap);
	reset(level);
	return 0;
}

static void
interrupt(int sig)
{
	signal(sig, interrupt);
	if (ed.initialized) {
		if (!ed.caught)
			ed.caught = sig;
	}
	else if (!ed.pending)
		ed.pending = sig;
}

static int
getchr(void)
{
	int	n;

	if (ed.lastc = ed.peekc) {
		ed.peekc = 0;
		return ed.lastc;
	}
	if (ed.global) {
		if (ed.lastc = *ed.global++)
			return ed.lastc;
		ed.global = 0;
		return EOF;
	}
	if (!ed.input) {
		if (!(ed.input = sfgetr(sfstdin, '\n', 1))) {
			trap();
			return EOF;
		}
		if ((n = sfvalue(sfstdin) - 2) >= 0 && ed.input[n] == '\r')
			ed.input[n--] = 0;
		ed.spbeg = ed.input;
		ed.spend = (n >= 0 && ed.input[n] == '\\') ? (ed.input + n) : (char*)0;
	}
	if (!(ed.lastc = *ed.input++)) {
		ed.input = 0;
		ed.lastc = '\n';
	}
	return ed.lastc;
}

static void
splice(void)
{
	char*	s;
	int	n;

	if (ed.spend) {
		if (!ed.spl && !(ed.spl = sfstropen()))
			error(ERROR_SYSTEM|3, "cannot initialize splice buffer");
		sfwrite(ed.spl, ed.spbeg, ed.spend - ed.spbeg);
		ed.spend = 0;
		sfputc(ed.spl, '\n');
		while (s = sfgetr(sfstdin, '\n', 1)) {
			if ((n = sfvalue(sfstdin) - 1) > 0 && s[n - 1] == '\r')
				n--;
			if (n > 0 && s[n - 1] == '\\') {
				sfwrite(ed.spl, s, n - 1);
				sfputc(ed.spl, '\n');
			}
			else {
				sfwrite(ed.spl, s, n);
				break;
			}
		}
		if (!(s = sfstruse(ed.spl)))
			error(ERROR_SYSTEM|3, "out of space");
		ed.input = s + (ed.input - ed.spbeg);
	}
}

static char*
input(int n)
{
	if (ed.peekc) {
		ed.peekc = 0;
		n--;
	}
	if (ed.global)
		return ed.global += n;
	else if (ed.input)
		return ed.input += n;
	else
		return 0;
}

static ssize_t
helpwrite(int fd, const void* buf, size_t len)
{
	ssize_t	n;

	NoP(fd);
	n = ed.help ? sfwrite(sfstderr, buf, len) : ed.verbose ? sfputr(ed.msg, "?", '\n') : 0;
	sfstrseek(ed.buffer.help, 0, SEEK_SET);
	sfwrite(ed.buffer.help, buf, len - 1);
	sfputc(ed.buffer.help, 0);
	return n;
}

static void
init(void)
{
	register Sfio_t**	ss;
	register int		c;

	ed.interactive = -1;
	ed.msg = sfstdout;
	ed.all = BLOCK_LINE;
	ed.page.size = BREAK_PAGE;
	ed.redisc.re_version = REG_VERSION;
	ed.redisc.re_errorf = errorf;
	ed.re.re_disc = &ed.redisc;
	ed.reflags = REG_DISCIPLINE|REG_DELIMITED;
	if (!conformance(0, 0))
		ed.reflags |= REG_LENIENT;
	ed.verbose = 1;
	for (c = 0; c < elementsof(signals); c++)
		if (signal(signals[c], interrupt) == SIG_IGN)
			signal(signals[c], SIG_IGN);
	for (ss = (Sfio_t**)&ed.buffer; ss < (Sfio_t**)(((char*)&ed.buffer) + sizeof(ed.buffer)); ss++) {
		if (!(*ss = sfstropen()))
			error(ERROR_SYSTEM|3, "cannot initialize internal buffer");
		sfputc(*ss, 0);
		sfstrseek(*ss, 0, SEEK_SET);
	}
	sfputr(ed.buffer.help, "?", 0);
	if (!(ed.zero = newof(NiL, Line_t, ed.all, 0)))
		error(ERROR_SYSTEM|3, "out of space [zero]");
}

static char*
getrec(register Sfio_t* sp, register int delimiter, register int flags)
{
	register int	c;
	register char*	glob;

	sfstrseek(sp, 0, SEEK_SET);
	glob = ed.global;
	while ((c = getchr()) != delimiter) {
		if (c == '\n') {
			ed.peekc = c;
			break;
		}
		if (c == EOF) {
			if (glob)
				ed.peekc = (flags & REC_LINE) ? 0 : c;
			else if (delimiter != '\n' || (flags & (REC_LINE|REC_SPLICE)))
				error(2, "unexpected EOF");
			else if (flags & REC_TEXT)
				return 0;
			break;
		}
		if (c == '\\' && ((c = getchr()) != delimiter || (flags & REC_SPLICE) && c != '\n') && c && !(flags & REC_IGNORE))
			sfputc(sp, '\\');
		if (!c)
			error(1, "null character ignored");
		else if (!(flags & REC_IGNORE))
			sfputc(sp, c);
	}
	if (flags & REC_TERMINATE)
		sfputc(sp, c);
	if (!(glob = sfstruse(sp)))
		error(ERROR_SYSTEM|3, "out of space");
	return glob;
}

static void
putrec(register char* s)
{
	register int	n;
	register char*	t;

	if ((ed.print & REG_SUB_LIST) && (t = fmtesc(s))) {
		s = t;
		n = strlen(s);
		while (n > BREAK_LINE) {
			n -= BREAK_LINE;
			sfprintf(ed.msg, "%-*.*s\\\n", BREAK_LINE, BREAK_LINE, s);
			s += BREAK_LINE;
		}
		sfprintf(ed.msg, "%s$\n", s);
	}
	else
		sfputr(ed.msg, s, '\n');
}

static void
modify(void)
{
	if (!ed.evented) {
		ed.evented = ed.modified = 1;
		ed.event++;
		ed.undo.dot = ed.dot - ed.zero;
		ed.undo.dol = ed.dol - ed.zero;
		if (ed.marked) {
			register int	c;

			for (c = 0; c < elementsof(ed.marks); c++)
				ed.undo.marks[c] = ed.marks[c];
		}
	}
}

static void
undo(void)
{
	register Line_t*	a1;
	register Line_t*	a3;
	register unsigned long	event;
	int			c;
	off_t			t;
	unsigned long		n;

	c = 0;
	event = ed.event;
	a1 = ed.zero;
	a3 = ed.zero + ed.all;
	while (++a1 < a3)
		if (a1->event == event) {
			c = 1;
			t = a1->offset;
			a1->offset = a1->undo;
			a1->undo = t;
		}
	if (!c)
		error(2, "nothing to undo");
	if (ed.marked)
		for (c = 0; c < elementsof(ed.marks); c++) {
			t = ed.marks[c];
			ed.marks[c] = ed.undo.marks[c];
			ed.undo.marks[c] = t;
		}
	n = ed.dot - ed.zero;
	ed.dot = ed.zero + ed.undo.dot;
	ed.undo.dot = n;
	n = ed.dol - ed.zero;
	ed.dol = ed.zero + ed.undo.dol;
	ed.undo.dol = n;
}

static char*
lineget(off_t off)
{
	char*	s;

	off &= ~(LINE_GLOBAL|LINE_MARKED);
	if (sfseek(ed.tmp, off, SEEK_SET) != off)
		error(ERROR_SYSTEM|2, "temp file read seek error");
	if (!(s = sfgetr(ed.tmp, 0, 0)))
		error(ERROR_SYSTEM|2, "temp file read error at offset %I*d", sizeof(off), off);
	return s;
}

static off_t
lineput(char* s)
{
	off_t	off;

	modify();
	off = ed.tmpoff;
	if (sfseek(ed.tmp, off, SEEK_SET) != off)
		error(ERROR_SYSTEM|2, "temp file write seek error");
	if (sfputr(ed.tmp, s, 0) < 0)
		error(ERROR_SYSTEM|2, "temp file write error at offset %I*d", sizeof(off), off);
	if ((ed.tmpoff = sfseek(ed.tmp, (off_t)0, SEEK_CUR)) == (off_t)-1)
		error(ERROR_SYSTEM|2, "temp file tell error");
	return off;
}

static void
replace(register Line_t* a1, char* s)
{
	register off_t	off;

	off = lineput(s);
	if (a1->offset & LINE_MARKED) {
		register off_t*	mp;

		a1->offset &= ~LINE_GLOBAL;
		off |= LINE_MARKED;
		for (mp = ed.marks; mp < &ed.marks[elementsof(ed.marks)]; mp++)
			if (*mp == a1->offset)
				*mp = off;
	}
	a1->event = ed.event;
	a1->undo = a1->offset;
	a1->offset = off;
}

static void
squeeze(int i)
{
	if (ed.addr1 < ed.zero + i)
		error(2, "at top of file");
	if (ed.addr2 > ed.dol)
		error(2, "at end of file");
	if (ed.addr1 > ed.addr2)
		error(2, "first address exceeds second");
}

static void
nonzero(void)
{
	squeeze(1);
}

static char*
getfile(void)
{
	register char*	s;
	register int	n;
	register int	m;

	if (!(s = sfgetr(ed.iop, '\n', 1))) {
		if (!(s = sfgetr(ed.iop, '\n', -1)))
			return 0;
		ed.warn_newline = 1;
	}
	if ((n = sfvalue(ed.iop)) > 0 && s[n - 1] == '\r')
		s[--n] = 0;
	if ((m = strlen(s)) < n) {
		register char*	t;
		register char*	u;
		register char*	x;

		t = u = s + m;
		x = s + n;
		while (u < x)
			if (!(*t++ = *u++))
				t--;
		*t++ = 0;
		n = t - s;
		ed.warn_null += x - t;
	}
	ed.bytes += n;
	ed.lines++;
	return s;
}

static char*
getline(void)
{
	register char*	s;

	if ((s = getrec(ed.buffer.line, '\n', REC_TEXT)) && s[0] == '.' && !s[1])
		s = 0;
	return s;
}

static char*
getbreak(void)
{
	char*	s;

	if ((s = ed.linebreak) && (ed.linebreak = strchr(s, '\n')))
		*ed.linebreak++ = 0;
	return s;
}

static char*
getcopy(void)
{
	if (ed.addr1 > ed.addr2)
		return 0;
	return lineget((ed.addr1++)->offset);
}

static void
print(void)
{
	register Line_t* a1;

	nonzero();
	a1 = ed.addr1;
	do {
		if (ed.print & REG_SUB_NUMBER)
			sfprintf(ed.msg, "%d\t", a1 - ed.zero);
		putrec(lineget((a1++)->offset));
	} while (a1 <= ed.addr2);
	ed.dot = ed.addr2;
	ed.print = 0;
}

static int
getnum(void)
{
	register int c;
	register int r;

	r = 0;
	while ((c = getchr()) >= '0' && c <= '9')
		r = r * 10 + c - '0';
	ed.peekc = c;
	return r;
}

static void
compile(void)
{
	register char*	s;
	int		c;

	s = input(0);
	if (*s) {
		if (*(s + 1)) {
			if (*s == *(s + 1))
				input(2);
			else {
				if (ed.compiled) {
					ed.compiled = 0;
					regfree(&ed.re);
				}
				if (c = regcomp(&ed.re, s, ed.reflags)) {
					regfatal(&ed.re, 2, c);
					eat();
				}
				else
					input(ed.re.re_npat);
				ed.compiled = 1;
				return;
			}
		}
		else
			input(1);
	}
	if (!ed.compiled)
		error(2, "no previous regular expression");
}

static int
execute(Line_t* addr, regflags_t flags)
{
	register char*	s;
	register int	c;

	trap();
	if (!addr)
		s = CUR();
	else if (addr == ed.zero)
		return 0;
	else {
		s = lineget(addr->offset);
		SET(s, 0);
	}
	if (c = regexec(&ed.re, s, elementsof(ed.match), ed.match, ed.reflags|flags)) {
		if (c != REG_NOMATCH)
			regfatal(&ed.re, 2, c);
		return 0;
	}
	return 1;
}

static Line_t*
address(void)
{
	register int		c;
	register int		sign;
	register Line_t*	a;
	register Line_t*	b;
	int			opcnt;
	int			nextopand;

	nextopand = -1;
	sign = 1;
	opcnt = 0;
	a = ed.dot;
	do {
		do c = getchr(); while (isspace(c) && c != '\n');
		if (c >= '0' && c <= '9') {
			ed.peekc = c;
			if (!opcnt)
				a = ed.zero;
			a += sign * getnum();
		}
		else switch (c) {

		case '$':
			a = ed.dol;
			/*FALLTHROUGH*/
		case '.':
			if (opcnt)
				error(2, "invalid address");
			break;

		case '\'':
			if ((c = getchr()) == EOF || (c -= MARK_MIN) < 0 || c >= elementsof(ed.marks) || opcnt)
				error(2, "invalid mark");
			a = ed.marked && ed.marks[c] != LINE_NONE ? ed.zero : ed.dol;
			do {
				if (++a > ed.dol)
					error(2, "undefined mark referenced");
			} while (ed.marks[c] != (a->offset & ~LINE_GLOBAL));
			break;

		case '?':
			sign = -sign;
			/*FALLTHROUGH*/
		case '/':
			input(-1);
			compile();
			b = a;
			for (;;) {
				a += sign;
				if (a <= ed.zero)
					a = ed.dol;
				if (a > ed.dol)
					a = ed.zero;
				if (execute(a, 0))
					break;
				if (a == b)
					error(2, "pattern not found");
			}
			break;

		default:
			if (nextopand == opcnt) {
				a += sign;
				if (a < ed.zero || ed.dol < a)
					continue;       /* error? */
			}
			if (c != '+' && c != '-' && c != '^') {
				ed.peekc = c;
				if (!opcnt)
					a = 0;
				return a;
			}
			sign = 1;
			if (c != '+')
				sign = -sign;
			nextopand = ++opcnt;
			continue;

		}
		sign = 1;
		opcnt++;
	} while (a >= ed.zero && a <= ed.dol);
	error(2, "address out of range");
	return 0;
}

static void
setwide(void)
{
	if (!ed.given) {
		ed.addr1 = ed.zero + (ed.dol > ed.zero);
		ed.addr2 = ed.dol;
	}
}

static void
setnoaddr(void)
{
	if (ed.given)
		error(2, "invalid address count");
}

static void
newline(void)
{
	register int	warned = 0;

	for (;;)
		switch (getchr()) {

		case EOF:
		case '\n':
			return;

		case 'l':
			ed.print = REG_SUB_LIST;
			continue;

		case 'n':
			ed.print = REG_SUB_NUMBER;
			continue;

		case 'p':
			ed.print = REG_SUB_PRINT;
			continue;

		default:
			if (!warned) {
				warned = 1;
				error(2, "extra characters at end of command");
			}
			continue;
		}
}

static char*
plural(unsigned long count)
{
	return count == 1 ? "" : "s";
}

static void
exfile(void)
{
	if (sfclose(ed.iop))
		error(ERROR_SYSTEM|1, "io error");
	ed.iop = 0;
	if (ed.verbose) {
		if (ed.help) {
			sfprintf(ed.msg, "\"%s\" %lu line%s, %lu character%s", error_info.file, ed.lines, plural(ed.lines), ed.bytes, plural(ed.bytes));
			if (ed.warn_null) {
				sfprintf(ed.msg, ", %lu null%s", ed.warn_null, plural(ed.warn_null));
				ed.warn_null = 0;
			}
			if (ed.warn_newline) {
				sfprintf(ed.msg, ", newline appended");
				ed.warn_newline = 0;
			}
			sfputc(ed.msg, '\n');
		}
		else
			sfprintf(ed.msg, "%d\n", ed.bytes);
	}
	if (ed.warn_null || ed.warn_newline) {
		char*	sep = "";

		sfstrseek(ed.buffer.line, 0, SEEK_SET);
		if (ed.warn_null) {
			sfprintf(ed.buffer.line, "%d null character%s ignored", ed.warn_null, plural(ed.warn_null));
			ed.warn_null = 0;
			sep = ", ";
		}
		if (ed.warn_newline) {
			sfprintf(ed.buffer.line, "%snewline appended to last line", sep);
			ed.warn_newline = 0;
		}
		if (!(sep = sfstruse(ed.buffer.line)))
			error(ERROR_SYSTEM|3, "out of space");
		error(1, "%s", sep);
	}
	error_info.file = 0;
}

static void
putfile(void)
{
	register Line_t*	a1;
	register int		n;

	ed.bytes = 0;
	ed.lines = 0;
	a1 = ed.addr1;
	do {
		if ((n = sfputr(ed.iop, lineget((a1++)->offset), '\n')) < 0)
			error(ERROR_SYSTEM|2, "write error");
		ed.bytes += n;
		ed.lines++;
	} while (a1 <= ed.addr2);
	if (sfsync(ed.iop))
		error(ERROR_SYSTEM|2, "write error");
}

static void
quit(int code)
{
	if (ed.tmpfile) {
		remove(ed.tmpfile);
		ed.tmpfile = 0;
	}
	if (ed.verbose && ed.modified && ed.dol != ed.zero) {
		ed.modified = 0;
		error(2, "file changed but not written");
	}
	if (ed.caught == SIGQUIT) {
		signal(ed.caught, SIG_DFL);
		kill(0, ed.caught);
	}
	exit(code);
}

static void
handle(void)
{
	register int	c;
	char*		s;
	char*		b;
	mode_t		mask;

	if (ed.caught == SIGINT) {
		ed.caught = 0;
		ed.lastc = '\n';
		sfputc(ed.msg, '\n');
		error(2, "interrupt");
	}
	for (c = 0; c < elementsof(signals); c++)
		signal(signals[c], SIG_IGN);
	if (ed.dol > ed.zero) {
		ed.addr1 = ed.zero + 1;
		ed.addr2 = ed.dol;
		mask = umask(S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
		b = "ed.hup";
		if (!(ed.iop = sfopen(NiL, b, "w")) && !ed.restricted && (s = getenv("HOME"))) {
			sfstrseek(ed.buffer.line, 0, SEEK_SET);
			sfprintf(ed.buffer.line, "%s/%s", s, b);
			if (!(b = sfstruse(ed.buffer.line)))
				error(ERROR_SYSTEM|3, "out of space");
			ed.iop = sfopen(NiL, b, "w");
		}
		umask(mask);
		if (!ed.iop)
			error(ERROR_SYSTEM|1, "%s: cannot save changes", b);
		else {
			error_info.file = b;
			putfile();
		}
	}
	ed.modified = 0;
	quit(0);
}

static Line_t*
append(char* (*f)(void), Line_t* a, Line_t** r)
{
	register char*		s;
	register Line_t*	a1;
	register Line_t*	a2;
	register Line_t*	a3;
	off_t			t;
	long			added;

	added = 0;
	ed.dot = a;
	while (s = (*f)()) {
		trap();
		if ((ed.dol - ed.zero) + 1 >= ed.all) {
			unsigned long	dot_off = ed.dot - ed.zero;
			unsigned long	dol_off = ed.dol - ed.zero;
			unsigned long	r_off = r ? *r - ed.zero : 0;

			ed.all += BLOCK_LINE;
			if (!(ed.zero = newof(ed.zero, Line_t, ed.all, 0))) {
				error(ERROR_SYSTEM|1, "no space [zero]");
				ed.caught = SIGHUP;
				trap();
			}
			ed.dot = ed.zero + dot_off;
			ed.dol = ed.zero + dol_off;
			if (r)
				*r = ed.zero + r_off;
		}
		t = lineput(s);
		added++;
		a1 = ++ed.dol;
		a2 = a1 + 1;
		a3 = ++ed.dot;
		while (a1 > a3) {
			(--a2)->event = ed.event;
			a2->undo = a2->offset;
			a2->offset = (--a1)->offset;
		}
		a3->event = ed.event;
		a3->undo = a3->offset;
		a3->offset = t;
	}
	if (r)
		*r += added;
	return ed.dot;
}

static void
add(int i)
{
	if (i && (ed.given || ed.dol > ed.zero)) {
		ed.addr1--;
		ed.addr2--;
	}
	squeeze(0);
	newline();
	append(getline, ed.addr2, NiL);
}

static void
page(void)
{
	register int	direction;
	register int	n;

	switch (direction = getchr()) {

	case '-':
	case '.':
	case '+':
		break;

	default:
		ed.peekc = direction;
		direction = '+';
		break;

	}
	if ((n = getnum()) > 0)
		ed.page.size = n;
	newline();
	if (ed.print)
		ed.page.print = ed.print;
	else
		ed.print = ed.page.print;
	switch (direction) {

	case '-':
		ed.addr1 = ed.addr2 - ed.page.size + 1;
		break;

	case '.':
		ed.addr2 += ed.page.size / 2;
		ed.addr1 = ed.addr2 - ed.page.size + 1;
		break;

	case '+':
		ed.addr1 = ed.addr2;
		ed.addr2 += ed.page.size - 1;
		break;

	}
	if (ed.addr1 <= ed.zero)
		ed.addr1 = ed.zero + 1;
	if (ed.addr2 > ed.dol)
		ed.addr2 = ed.dol;
	print();
}

static void
rdelete(register Line_t* a1, register Line_t* a2)
{
	register Line_t*	a3;

	modify();
	a3 = ed.dol;
	ed.dol -= ++a2 - a1;
	ed.dot = a1 > ed.dol ? ed.dol : a1;
	do {
		a1->undo = a1->offset;
		a1->event = ed.event;
		(a1++)->offset = (a2++)->offset;
	} while (a2 <= a3);
	while (a1 <= a3) {
		a1->undo = a1->offset;
		(a1++)->event = ed.event;
	}
}

static void
gdelete(void)
{
	register Line_t*	a1;
	register Line_t*	a2;
	register Line_t*	a3;

	a3 = ed.dol;
	for (a1 = ed.zero; !(a1->offset & LINE_GLOBAL); a1++)
		if (a1 >= a3)
			return;
	modify();
	for (a2 = a1 + 1; a2 <= a3;) {
		a1->event = ed.event;
		a1->undo = a1->offset;
		if (a2->offset & LINE_GLOBAL) {
			a2++;
			ed.dot = a1;
		}
		else
			(a1++)->offset = (a2++)->offset;
	}
	ed.dol = a1 - 1;
	if (ed.dot > ed.dol)
		ed.dot = ed.dol;
	while (a1 <= a3) {
		a1->undo = a1->offset;
		(a1++)->event = ed.event;
	}
}

static void
shell(void)
{
	register char*	s;
	register char*	f = 0;
	register int	c;

	if (ed.given)
		squeeze(ed.dol > ed.zero);
	s = getrec(ed.buffer.line, '\n', 0);
	if (s[0] == '!' && !s[1]) {
		if (!*sfstrbase(ed.buffer.shell))
			error(2, "no saved shell command");
		f = sfstrbase(ed.buffer.file);
	}
	else if (!s[0])
		error(2, "empty shell command");
	else
		SWP(ed.buffer.shell, ed.buffer.line);
	s = sfstrbase(ed.buffer.shell);
	sfstrseek(ed.buffer.line, 0, SEEK_SET);
	sfputc(ed.buffer.line, '!');
	while (c = *s++) {
		if (c == '\\') {
			if (*s != '%')
				sfputc(ed.buffer.line, c);
			sfputc(ed.buffer.line, *s++);
		}
		else if (c == '%')
			sfputr(ed.buffer.line, f = sfstrbase(ed.buffer.file), -1);
		else
			sfputc(ed.buffer.line, c);
	}
	if (ed.given) {
		if (!ed.tmpfile && !(ed.tmpfile = pathtemp(NiL, 0, NiL, error_info.id, NiL)))
			error(ERROR_SYSTEM|2, "cannot generate temp file name");
		if (!(ed.iop = sfopen(NiL, ed.tmpfile, "w")))
			error(ERROR_SYSTEM|2, "%s: cannot create temp file", ed.tmpfile);
		error_info.file = ed.tmpfile;
		if (ed.dol > ed.zero)
			putfile();
		exfile();
		ed.bytes = 0;
		ed.lines = 0;
		sfprintf(ed.buffer.line, " < %s", ed.tmpfile);
		if (!(s = sfstruse(ed.buffer.line)))
			error(ERROR_SYSTEM|3, "out of space");
		if (!(ed.iop = sfpopen(NiL, s + 1, "r")))
			error(ERROR_SYSTEM|2, "%s: cannot execute shell command", s);
		error_info.file = s;
		rdelete(ed.addr1, ed.addr2);
		append(getfile, ed.dot, NiL);
		exfile();
		remove(ed.tmpfile);
	}
	else {
		if (!(s = sfstruse(ed.buffer.line)))
			error(ERROR_SYSTEM|3, "out of space");
		s++;
		if (f)
			putrec(s);
		if (!(ed.iop = sfpopen(NiL, s, "")))
			error(ERROR_SYSTEM|2, "%s: cannot execute shell command", s);
		if (sfclose(ed.iop)) {
			ed.iop = 0;
			error(ERROR_SYSTEM|2, "%s: shell command exit error", s);
		}
		if (ed.verbose)
			putrec("!");
	}
}

static void
edit(void)
{
	register off_t*	mp;

	if (ed.tmp) {
		sfclose(ed.tmp);
		ed.tmp = 0;
	}
	ed.tmpoff = 0;
	if (!(ed.tmp = sftmp(BLOCK_TMP)))
		error(ERROR_SYSTEM|3, "cannot create temp file");
	for (mp = ed.marks; mp < &ed.marks[elementsof(ed.marks)]; )
		*mp++ = LINE_NONE;
	ed.marked = 0;
	ed.event++;
	ed.dot = ed.dol = ed.zero;
	if (!ed.initialized) {
		ed.initialized = 1;
		if (ed.pending)
			ed.caught = ed.pending;
	}
}

static void
filename(int c)
{
	register char*	p;
	register int	sh = 0;

	ed.bytes = 0;
	ed.lines = 0;
	p = getrec(ed.buffer.line, '\n', REC_LINE);
	if (*p) {
		if (!isspace(*p))
			error(2, "no space after command");
		for (p++; isspace(*p); p++)
			;
		if (!*p)
			error(2, "file name expected");
		if (c != 'f') {
			if (*p == '!') {
				p++;
				sh = 1;
			}
			else if (*p == '\\' && *(p + 1) == '!')
				p++;
		}
		if (ed.restricted) {
			register char*	s = p;

			if (sh)
				p--;
			else
				for (;;)
				{
					switch (*s++)
					{
					case 0:
						break;
					case '/':
					case '\n':
					case '\\':
						sh = 1;
						break;
					default:
						continue;
					}
					break;
				}
			if (sh)
				error(2, "%s: restricted file name", p);
		}
		if (!sh && (!*sfstrbase(ed.buffer.file) || c == 'e' || c == 'f')) {
			sfstrseek(ed.buffer.file, 0, SEEK_SET);
			sfputr(ed.buffer.file, p, 0);
		}
		if (c == 'f')
			return;
	}
	else if (c == 'f')
		return;
	else if (!*(p = sfstrbase(ed.buffer.file)))
		error(2, "file name expected");
	if (c == 'e') {
		edit();
		ed.addr2 = ed.zero;
	}
	if (sh) {
		if (!(ed.iop = sfpopen(NiL, p, (c == 'e' || c == 'r') ? "r" : "w")))
			error(ERROR_SYSTEM|2, "%s: cannot execute shell command", p);
		p--;
	}
	else if (c == 'e' || c == 'r') {
		if (!(ed.iop = sfopen(NiL, p, "r")))
			error(ERROR_SYSTEM|2, "%s: cannot read", p);
	}
	else if ((c != 'W' || !(ed.iop = sfopen(NiL, p, "a"))) && !(ed.iop = sfopen(NiL, p, "w")))
		error(ERROR_SYSTEM|2, "%s: cannot write", p);
	error_info.file = p;
}

static void
global(int sense, int query)
{
	register char*		s;
	register int		c;
	register Line_t*	a1;

	if (ed.global)
		error(2, "recursive global not allowed");
	setwide();
	squeeze(ed.dol > ed.zero);
	compile();
	if (query)
		newline();
	else {
		s = getrec(ed.buffer.global, '\n', REC_SPLICE|REC_TERMINATE);
		if (s[0] == '\n' && !s[1])
			sfputr(ed.buffer.global, "p\n", 0);
	}
	for (a1 = ed.zero; a1 <= ed.dol; a1++) {
		a1->offset &= ~LINE_GLOBAL;
		if (a1 >= ed.addr1 && a1 <= ed.addr2 && execute(a1, 0) == sense)
			a1->offset |= LINE_GLOBAL;
	}

	/* special case: g/.../d (avoid n^2 algorithm) */

	if (!query && s[0] == 'd' && s[1] == '\n' && !s[2])
		gdelete();
	else {
		for (a1 = ed.zero; a1 <= ed.dol; a1++) {
			if (a1->offset & LINE_GLOBAL) {
				a1->offset &= ~LINE_GLOBAL;
				ed.dot = a1;
				if (query) {
					putrec(lineget(a1->offset));
					if ((c = getchr()) == EOF)
						break;
					else if (c == '\n')
						continue;
					else if (c == '&') {
						newline();
						if (!*(ed.global = sfstrbase(ed.buffer.query)))
							error(2, "no saved command");
					}
					else {
						ed.peekc = c;
						ed.global = getrec(ed.buffer.query, '\n', REC_TERMINATE);
					}
				}
				else
					ed.global = s;
				commands();
				a1 = ed.zero;
			}
		}
	}
}

static void
join(void)
{
	register Line_t*	a1;
	char*			s;

	nonzero();
	sfstrseek(ed.buffer.work, 0, SEEK_SET);
	for (a1 = ed.addr1; a1 <= ed.addr2;)
		sfputr(ed.buffer.work, lineget((a1++)->offset), -1);
	a1 = ed.dot = ed.addr1;
	if (!(s = sfstruse(ed.buffer.work)))
		error(ERROR_SYSTEM|3, "out of space");
	replace(a1, s);
	if (a1 < ed.addr2)
		rdelete(a1 + 1, ed.addr2);
}

static void
substitute(int inglob)
{
	register Line_t*	a1;
	char*			s;
	char*			e;
	int			n;

	n = getnum();
	compile();
	splice();
	if (n = regsubcomp(&ed.re, input(0), submap, n, 0))
		regfatal(&ed.re, 2, n);
	else {
		if (!(ed.re.re_sub->re_flags & REG_SUB_FULL))
			ed.re.re_sub->re_flags |= REG_SUB_PRINT;
		if ((n = *input(ed.re.re_npat)) && n != '\r' && n != '\n')
			error(2, "extra characters at end of command");
		ed.print = ed.re.re_sub->re_flags;
	}
	eat();
	for (a1 = ed.addr1; a1 <= ed.addr2; a1++) {
		if (execute(a1, 0)) {
			if (!regsubexec(&ed.re, CUR(), elementsof(ed.match), ed.match)) {
				inglob = 1;
				s = ed.re.re_sub->re_buf;
				SET(s, ed.re.re_sub->re_len);
				if (e = strchr(s, '\n'))
					*e++ = 0;
				replace(a1, s);
				if (e) {
					ed.linebreak = e;
					a1 = append(getbreak, a1, &ed.addr2);
				}
			}
		}
	}
	if (!inglob)
		error(2, "global pattern not found");
	ed.dot = ed.addr2;
}

static void
reverse(register Line_t* a1, register Line_t* a2)
{
	modify();
	while (--a2 > a1) {
		a1->event = a2->event = ed.event;
		a2->undo = a2->offset;
		a2->offset = a1->undo = a1->offset;
		(a1++)->offset = a2->undo;
	}
}

static void
move(int cflag)
{
	register Line_t*	ad1;
	register Line_t*	ad2;
	Line_t*			adt;
	unsigned long		ad1_off;
	unsigned long		adt_off;

	nonzero();
	if (!(adt = address()))
		error(2, "invalid move destination");
	newline();
	if (cflag) {
		ad1_off = ed.dol - ed.zero + 1;
		adt_off = adt - ed.zero;
		append(getcopy, ed.dol, NiL);
		ad1 = ed.zero + ad1_off;
		ad2 = ed.dol;
		adt = ed.zero + adt_off;
	}
	else {
		ad2 = ed.addr2;
		for (ad1 = ed.addr1; ad1 <= ad2; ad1++)
			ad1->offset &= ~LINE_GLOBAL;
		ad1 = ed.addr1;
	}
	ad2++;
	if (adt < ad1) {
		ed.dot = adt + (ad2 - ad1);
		if (++adt == ad1)
			return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
	}
	else if (adt >= ad2) {
		ed.dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
	}
	else
		error(2, "move would do nothing");
}

static void
commands(void)
{
	register Line_t*	a1;
	register int		c;
	register int		n;
	char*			s;
	int			lastsep;

	for (;;) {
		trap();
		if (ed.print & (REG_SUB_LIST|REG_SUB_NUMBER|REG_SUB_PRINT)) {
			ed.addr1 = ed.addr2 = ed.dot;
			print();
		}
		if (!ed.global) {
			ed.evented = 0;
			if (ed.prompt > 0)
				sfputr(ed.msg, sfstrbase(ed.buffer.prompt), -1);
		}
		if ((c = getchr()) == ',' || c == ';') {
			ed.given = 1;
			ed.addr1 = (lastsep = c) == ',' ? ed.zero + 1 : ed.dot;
			a1 = ed.dol;
			c = getchr();
		}
		else {
			ed.addr1 = 0;
			ed.peekc = c;
			c = '\n';
			for (;;) {
				lastsep = c;
				a1 = address();
				c = getchr();
				if (c != ',' && c != ';')
					break;
				if (lastsep == ',')
					error(2, "invalid address");
				if (!a1) {
					a1 = ed.zero + 1;
					if (a1 > ed.dol)
						a1--;
				}
				ed.addr1 = a1;
				if (c == ';')
					ed.dot = a1;
			}
			if (lastsep != '\n' && !a1)
				a1 = ed.dol;
		}
		if (!(ed.addr2 = a1)) {
			ed.given = 0;
			ed.addr2 = ed.dot;	
		}
		else
			ed.given = 1;
		if (!ed.addr1)
			ed.addr1 = ed.addr2;
		switch (c) {

		case 'a':
			add(0);
			continue;

		case 'c':
			nonzero();
			newline();
			rdelete(ed.addr1, ed.addr2);
			append(getline, ed.addr1 - 1, NiL);
			continue;

		case 'd':
			nonzero();
			newline();
			rdelete(ed.addr1, ed.addr2);
			continue;

		case 'E':
			ed.modified = 0;
			c = 'e';
			/*FALLTHROUGH*/
		case 'e':
			setnoaddr();
			if (ed.verbose && ed.modified) {
				ed.modified = 0;
				error(2, "modified data not written");
			}
			/*FALLTHROUGH*/
		case 'r':
			filename(c);
			setwide();
			squeeze(0);
			c = ed.zero != ed.dol;
			append(getfile, ed.addr2, NiL);
			ed.modified = c;
			exfile();
			continue;

		case 'f':
			setnoaddr();
			filename(c);
			putrec(sfstrbase(ed.buffer.file));
			continue;

		case 'G':
			global(1, 1);
			continue;

		case 'g':
			global(1, 0);
			continue;

		case 'H':
			ed.help = !ed.help;
			/*FALLTHROUGH*/
		case 'h':
			setnoaddr();
			newline();
			if (ed.help || c == 'h')
				sfputr(ed.msg, sfstrbase(ed.buffer.help), '\n');
			continue;

		case 'i':
			add(-1);
			continue;

		case 'j':
			if (!ed.given)
				ed.addr2++;
			newline();
			join();
			continue;

		case 'k':
			nonzero();
			if ((c = getchr()) == EOF || (c -= MARK_MIN) < 0 || c >= elementsof(ed.marks))
				error(2, "invalid mark");
			newline();
			ed.addr2->offset |= LINE_MARKED;
			ed.marks[c] = ed.addr2->offset & ~LINE_GLOBAL;
			ed.marked = 1;
			continue;

		case 'm':
			move(0);
			continue;

		case 'n':
			ed.print |= REG_SUB_NUMBER;
			newline();
			print();
			continue;

		case '\n':
			if (!a1) {
				a1 = ed.dot + 1;
				ed.addr2 = a1;
				ed.addr1 = a1;
			}
			if (lastsep == ';')
				ed.addr1 = a1;
			print();
			continue;

		case 'l':
			ed.print |= REG_SUB_LIST;
			/*FALLTHROUGH*/
		case 'p':
			newline();
			print();
			continue;

		case 'P':
			setnoaddr();
			s = getrec(ed.buffer.line, '\n', 0);
			if (*s || !(ed.prompt = -ed.prompt) && (s = "*")) {
				sfstrseek(ed.buffer.prompt, 0, SEEK_SET);
				sfputr(ed.buffer.prompt, s, 0);
				ed.prompt = 1;
			}
			continue;

		case 'Q':
			ed.modified = 0;
			/*FALLTHROUGH*/
		case 'q':
			setnoaddr();
			newline();
			quit(0);
			continue;

		case 'S':
			setnoaddr();
			newline();
			s = strchr(usage, '\n') + 5;
			sfprintf(ed.msg, "file=\"%s\"%s%s%s prompt=\"%s\" tmp=%lu%s event=%lu version=\"%-.*s\"\n", sfstrbase(ed.buffer.file), ed.modified ? " modified" : "", ed.help ? " help" : "", ed.verbose ? " verbose" : "", sfstrbase(ed.buffer.prompt), ed.tmpoff, ed.tmpoff > BLOCK_TMP ? "[file]" : "", ed.event, strchr(s, '\n') - s, s);
			continue;

		case 's':
			nonzero();
			substitute(ed.global != 0);
			continue;

		case 't':
			move(1);
			continue;

		case 'u':
			setnoaddr();
			newline();
			undo();
			continue;

		case 'V':
			global(0, 1);
			continue;

		case 'v':
			global(0, 0);
			continue;

		case 'W':
		case 'w':
			setwide();
			squeeze(ed.dol > ed.zero);
			if ((n = getchr()) != 'q' && n != 'Q') {
				ed.peekc = n;
				n = 0;
			}
			filename(c);
			if (ed.dol > ed.zero)
				putfile();
			exfile();
			if (n == 'Q' || ed.addr1 <= ed.zero + 1 && ed.addr2 == ed.dol)
				ed.modified = 0;
			if (n)
				quit(0);
			continue;

		case 'z':
			nonzero();
			page();
			continue;

		case '=':
			setwide();
			squeeze(0);
			newline();
			sfprintf(ed.msg, "%d\n", ed.addr2 - ed.zero);
			continue;

		case '!':
			if (ed.restricted)
				error(2, "%c: restricted command", c);
			shell();
			continue;

		case '#':
			setnoaddr();
			getrec(ed.buffer.line, '\n', REC_IGNORE);
			continue;

		case EOF:
			return;

		}
		error(2, "unknown command");
	}
}

int
main(int argc, char** argv)
{
	char*	s;

	NoP(argc);
	if (s = strrchr(*argv, '/')) s++;
	else s = *argv;
	ed.restricted = streq(s, "red");
	error_info.id = s;
	error_info.write = helpwrite;
	init();
	for (;;)
	{
		for (;;) {
			switch (optget(argv, usage)) {

			case 'O':
				ed.reflags |= REG_LENIENT;
				continue;

			case 'S':
				ed.reflags &= ~REG_LENIENT;
				continue;

			case 'h':
				ed.help = 1;
				continue;

			case 'o':
				ed.msg = sfstderr;
				sfstrseek(ed.buffer.file, 0, SEEK_SET);
				sfputr(ed.buffer.file, "/dev/stdout", 0);
				continue;

			case 'p':
				sfstrseek(ed.buffer.prompt, 0, SEEK_SET);
				sfputr(ed.buffer.prompt, opt_info.arg, 0);
				ed.prompt = 1;
				continue;

			case 'q':
				signal(SIGQUIT, SIG_DFL);
				ed.verbose = 1;
				continue;

			case 's':
				ed.verbose = 0;
				continue;

			case '?':
				ed.help++;
				error(ERROR_USAGE|4, "%s", opt_info.arg);
				ed.help--;
				break;

			case ':':
				ed.help++;
				error(2, "%s", opt_info.arg);
				ed.help--;
				continue;

			}
			break;
		}
		if (!*(argv += opt_info.index) || **argv != '-' || *(*argv + 1))
			break;
		ed.verbose = 0;
	}
	if (*argv) {
		if (*(argv + 1))
			error(ERROR_USAGE|4, "%s", optusage(NiL));
		sfprintf(ed.buffer.global, "e %s", *argv);
		if (!(ed.global = sfstruse(ed.buffer.global)))
			error(ERROR_SYSTEM|3, "out of space");
	}
	edit();
	sfdcslow(sfstdin);
	setjmp(ed.again);
	commands();
	quit(0);
	exit(0);
}
