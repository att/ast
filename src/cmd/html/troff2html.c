/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2012 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Research
 *
 * troff to html filter
 *
 * NOTE: not handled in state.groff mode
 *
 *	\?anything\?
 */

static const char usage[] =
"[-?\n@(#)$Id: troff2html (AT&T Research) 2004-04-26 $\n]"
USAGE_LICENSE
"[+NAME?troff2html - convert troff/groff input to html]"
"[+DESCRIPTION?\btroff2html\b converts \btroff\b(1) (or \bgroff\b(1),"
"	depending on the processing mode) input documents to an \bhtml\b"
"	document on the standard output. Although a full \atroff\a parse"
"	is done, many features are ignored in the mapping to \bhtml\b.]"
"[+?The \atroff\a \bt\b condition test evaluates \btrue\b, the \bn\b"
"	condition tests evaluates \bfalse\b, and the \agroff\a compatibility"
"	register \b\\\\n[.C]]\b evaluates to 0 (enable \agroff\a parsing)"
"	if it is referenced before the first \agroff\a \b.cp\b (compatibility"
"	mode) request.]"
"[+?The generated \bhtml\b has properly nested begin/end tags, even though most"
"	browsers don't care.]"

"[i:identify?Reads identification options from \afile\a. Unknown options"
"	are silently ignored. See the \b.xx\b request below for a description"
"	of the options.]:[file]"
"[I:include?Appends \adirectory\a to the list of directories searched"
"	for \b--macros\b and \b.so\b request files.]:[directory]"
"[m:macros?Locates and reads the macro package file \apackage\a. In order"
"	to accomodate different \atroff\a installation styles the file search"
"	order is fairly involved:]:[package]{"
"		[+./package?]"
"		[+directory/package?for all \b--include\b directories]"
"		[+directory/tmac.package?for all \b--include\b directories]"
"		[+../lib/tmac/tmac.package?for all directories on \b$PATH\b]"
"		[+../lib/html/package?for all directories on \b$PATH\b]"
"		[+../lib/html/mpackage.tr?for all directories on \b$PATH\b]"
"		[+../share/groff/tmac/tmac.package?for all directories on \b$PATH\b]"
"		[+../groff/share/groff/tmac/tmac.package?for all directories on \b$PATH\b]"
"}"
"[r:register?Initializes the number \aregister\a to \aN\a.]:[registerN]"
"[s:script?Reads \ascript\a as if it came from a file. \b--script='.nr A 123'\b"
"	is equivalent to \b-rA123\b.]:[script]"
"[v:verbose?Enables verbose error and warning messages. Following \atroff\a"
"	tradition, \btroff2html\b by default does not warn about unknown"
"	requests; \b--verbose\b enables such warnings.]"

"[+EXTENSIONS?\b.xx\b \aname\a[=\avalue\a]] is a special \btroff2html\b"
"	request that handles program tracing, \bhtml\b extensions and \atroff\a"
"	macro package magic that went way past the author's willingness"
"	to understand. Supported operations are:]{"
"	[+author=text?Specifies the contact information for the document"
"		HEAD section.]"
"	[+background=URL?Specifies the document background URL.]"
"	[+debug=level?The debug trace \alevel\a; higher levels produce"
"		more output.]"
"	[+get=+-register?Traces each \bget\b for the named number"
"		\aregister\a. \b-\b turns tracing off.]"
"	[+hot='word ...'?Adds (\b+\b) or removes (\b-\b) \aword\a ... from"
"		the hot-word list. Constructs that match (\ahot-word\a ..."
"		\aSWITCH-FONT\a text \aSWITCH-FONT\a ... ) adds \atext\a as"
"		a hot link to another portion of the document. \brefer\b and"
"		\bsee\b are the default hot words. Case is ignored when"
"		matching hot words.]"
"	[+logo=URL?Specifies the logo/banner image URL that is centered"
"		at the top of the document.]"
"	[+mailto=address?Sets the email \aaddress\a to send comments and"
"		suggestions.]"
"	[+meta.name?Emits the \bhtml\b tag \b<META name=\b\"\aname\a\""
"		\bcontent=\b\"\acontent\a\"\b>\b.]"
"	[+package=text?\atext\a is prepended to the \bhtml\b document title.]"
"	[+set=+-register?Traces each \bset\b for the named number"
"		\aregister\a. \b-\b turns tracing off.]"
"	[+title=text?Sets the document title.]"
"}"
"[+?Local URL links are generated for all top level headings. These can be"
"	referenced by embedding the benign (albeit convoluted) \atroff\a"
"	construct \\h'0*\\w\"label\"'text\\g'0', where \alabel\a is the"
"	local link label and \atext\a is the hot link text. If \alabel\a"
"	and \atext\a are the same then use \\h'0*1'text\\h'0'.]"

"\n"
"\n[ file ... ]\n"
"\n"
"[+SEE ALSO?\bgroff\b(1), \bhtml2rtf\b(1), \bman\b(1), \bmm\b(1),"
"	\bmm2html\b(1), \btroff\b(1)]"
;

#include "troff2html.h"

#include <error.h>
#include <hashkey.h>
#include <ls.h>
#include <tm.h>

/*
 * intermediate code
 *
 *	CODE_0
 *	CODE_1 <op>
 *	CODE_2 <data> <op>
 *	CODE_n <length> <op> <length-data>
 */

#define DEFAULT_cc	'.'
#define DEFAULT_c2	'\''
#define DEFAULT_ec	'\\'
#define DEFAULT_ft	1
#define DEFAULT_pc	'%'
#define DEFAULT_ps	10
#define DEFAULT_vs	12

#define CODE_1		1
#define CODE_2		2
#define CODE_n		3
#define CODE_0		4

#define END(x)		((x)|OP_END)
#define LABEL(x)	((x)|OP_LABEL)
#define OP(x)		((x)&077)

#define OP_END		0200
#define OP_LABEL	0100

#define OP_a		1
#define OP_body		2
#define OP_br		3
#define OP_cc		4
#define OP_center	5
#define OP_comment	6
#define OP_dd		7
#define OP_div		8
#define OP_dl		9
#define OP_dt		10
#define OP_fn		11
#define OP_ft1		12
#define OP_ft2		13
#define OP_ft3		14
#define OP_ft4		15
#define OP_ft5		16
#define OP_h2		17
#define OP_h3		18
#define OP_h4		19
#define OP_head		20
#define OP_hr		21
#define OP_html		22
#define OP_link		23
#define OP_p		24
#define OP_pre		25
#define OP_ps		26
#define OP_sub		27
#define OP_sup		28
#define OP_ta		29
#define OP_tab		30
#define OP_tab_data	31
#define OP_tab_head	32
#define OP_tab_row	33
#define OP_title	34
#define OP_RAW		35

#define OP_ft		(OP_ft1-1)

#define ARG_ALIGN(a)	(((a)&3)-1)
#define ARG_ATTR(a)	((a)&0x0fff)

#define ARG_left	0x0001
#define ARG_center	0x0002
#define ARG_right	0x0003

#define ARG_compact	0x0010
#define ARG_wide	0x0020

#define ATT_INDEX(a)	(((a)&0x0fff)-1)

#define ATT_NUMBER	0x8000

#define ATT_background	1
#define ATT_href	2
#define ATT_lref	3
#define ATT_id		4
#define ATT_name	5
#define ATT_size	(ATT_NUMBER|6)
#define ATT_src		7

#define LINE		0x2000
#define LIST		0x4000
#define STACK		0x8000

#define DIVERTED()	(state.out_top>state.out_stack)
#undef	EOF
#define EOF		0
#define INDEX(a,b)	(((a)<<8)|(b))
#define ISFILE()	((state.in_top-1)->ip)
#define GETC()		(*state.in++)
#define GROFFINIT()	do{if(!state.groff_init)state.groff|=state.groff_init=1;}while(0)
#define UNGETC(c)	(*--state.in=(c))

#define UNGET_MAX	(SF_BUFSIZE/8)

State_t			state;

/*
 * sfstruse(sp) with fatal error check
 */

static char*
use(Sfio_t* sp)
{
	char*	s;

	if (!(s = sfstruse(sp)))
		error(ERROR_SYSTEM|3, "out of space");
	return s;
}

/*
 * push input file/string
 */

static void
pushin(char* name, int line, Sfio_t* ip, char* data, Arg_t* arg)
{
	register Pushin_t*	pp;
	int			n;

	if (state.in_top >= &state.in_stack[elementsof(state.in_stack)])
		error(3, "input stack overflow");
	pp = state.in_top++;
	pp->in = state.in;
	if (!(pp->ip = ip))
	{
		if (!(state.in = (unsigned char*)data) || !*state.in)
		{
			state.in = pp->in;
			--state.in_top;
			return;
		}
	}
	else if (!(pp->buf = oldof(0, unsigned char, SF_BUFSIZE, UNGET_MAX + 1)))
		error(ERROR_SYSTEM|3, "out of space [file pushin]");
	else
	{
		pp->buf += UNGET_MAX;
		if ((n = sfread(ip, pp->buf, SF_BUFSIZE)) <= 0)
		{
			if (ip && ip != sfstdin)
				sfclose(ip);
			state.in_top--;
			return;
		}
		pp->end = pp->buf + n;
		*pp->end++ = EOF;
		state.in = pp->buf;
	}
	pp->file = error_info.file;
	if (name)
	{
		if (!state.original.file)
		{
			state.original.file = error_info.file;
			state.original.line = error_info.line;
		}
		error_info.file = name;
	}
	pp->line = error_info.line;
	if (line < 0)
		state.noline++;
	else
	{
		if (line > 0)
			error_info.line = line;
		if (state.noline > 0)
			state.noline++;
	}
	if (arg)
	{
		pp->mac = state.mac;
		state.mac = arg;
		if (pp->top.sp)
			sfstrseek(pp->top.sp, 0, SEEK_SET);
		else if (!(pp->top.sp = sfstropen()))
			error(ERROR_SYSTEM|3, "out of space [macro call]");
		pp->tag = state.tag;
		state.tag = &pp->top;
	}
	else
		pp->tag = 0;
}

static long	expression(char*, char**, int);

/*
 * return the next expression operand
 */

static long
operand(register char* s, char** e, int scale)
{
	register int	c;
	register long	n;
	register long	f;
	int		abs;
	int		neg;
	long		d;
	char*		x;

	while (isspace(*s))
		s++;
	if (abs = *s == '|')
		s++;
	if (neg = *s == '-')
		s++;
	else if (*s == '+')
		s++;
	d = 1;
	if (*s == '(')
	{
		n = (state.groff && isalpha(*(s + 1)) && *(s + 2) == ';') ?
			expression(s + 3, &x, *(s + 1)) :
			expression(s + 1, &x, scale);
		s = x;
		if (*s == ')')
			s++;
		c = *s++;
	}
	else
	{
		n = 0;
		while ((c = *s++) >= '0' && c <= '9')
			n = n * 10 + c - '0';
		if (c == '.')
		{
			f = 0;
			while ((c = *s++) >= '0' && c <= '9')
			{
				d *= 10;
				n *= 10;
				f = f * 10 + c - '0';
			}
			n += f;
		}
	}
	for (;;)
	{
		switch (c)
		{
		case 'P':
			n *= 72;
			break;
		case 'c':
			n *= 170;
			break;
		case 'i':
			n *= 432;
			break;
		case 'm':
			n *= 8 * state.env->ps.current;
			break;
		case 'n':
		case 'w':
			n *= 6 * state.env->ps.current;
			break;
		case 'p':
			n *= 6;
			break;
		case 's':
		case 'u':
		case 'z':
			break;
		case 'v':
			n *= 6 * state.env->vs.current;
			break;
		default:
			s--;
			if (c = scale)
			{
				scale = 0;
				continue;
			}
			break;
		}
		break;
	}
	n /= d;
	if (e)
	{
		while (isspace(*s))
			s++;
		*e = s;
	}
	if (abs)
		return 1;
	if (neg)
		n = -n;
	return n;
}

/*
 * convert units to scale
 */

static long
convert(long n, int up, int scale)
{
	long	m;

	m = n;
	switch (scale)
	{
	case 'M':
		if (up)
			n *= state.env->ps.current / 12;
		else
			n /= state.env->ps.current / 12;
		break;
	case 'P':
		if (up)
			n *= 72;
		else
			n /= 72;
		break;
	case 'c':
		if (up)
			n *= 170;
		else
			n /= 170;
		break;
	case 'i':
		if (up)
			n *= 432;
		else
			n /= 432;
		break;
	case 'm':
		if (up)
			n *= 8 * state.env->ps.current;
		else
			n /= 8 * state.env->ps.current;
		break;
	case 'n':
	case 'w':
		if (up)
			n *= 6 * state.env->ps.current;
		else
			n /= 6 * state.env->ps.current;
		break;
	case 'p':
		if (up)
			n *= 6;
		else
			n /= 6;
		break;
	case 's':
	case 'u':
	case 'z':
		break;
	case 'v':
		if (up)
			n *= 6 * state.env->vs.current;
		else
			n /= 6 * state.env->vs.current;
		break;
	}
	return n ? n : (m > 0) ? 1 : 0;
}

/*
 * evaluate numeric expression in s
 */

static long
expression(char* s, char** e, int scale)
{
	long	n;
	long	m;

	n = operand(s, &s, scale);
	for (;;)
	{
		switch (*s++)
		{
		case CODE_2:
			s++;
			/*FALLTHROUGH*/
		case CODE_1:
			s++;
			/*FALLTHROUGH*/
		case CODE_0:
			continue;
		case '+':
			n += operand(s, &s, scale);
			continue;
		case '-':
			n -= operand(s, &s, scale);
			continue;
		case '/':
			if (m = operand(s, &s, scale))
				n /= m;
			else
				n = 0;
			continue;
		case '*':
			n *= operand(s, &s, scale);
			continue;
		case '%':
			if (m = operand(s, &s, scale))
				n %= m;
			else
				n = 0;
			continue;
		case '<':
			if (state.groff && *s == '?')
			{
				m = operand(s + 1, &s, scale);
				if (m < n)
					n = m;
			}
			else if (*s == '=')
				n = n <= operand(s + 1, &s, scale);
			else
				n = n < operand(s, &s, scale);
			continue;
		case '>':
			if (state.groff && *s == '?')
			{
				m = operand(s + 1, &s, scale);
				if (m > n)
					n = m;
			}
			else if (*s == '=')
				n = n >= operand(s + 1, &s, scale);
			else
				n = n > operand(s, &s, scale);
			continue;
		case '=':
			if (*s == '=')
				s++;
			n = n == operand(s, &s, scale);
			continue;
		case '&':
			if (*s == '&')
				s++;
			n = (operand(s, &s, scale) > 0) && (n > 0);
			continue;
		case ':':
			if (*s == ':')
				s++;
			n = (operand(s, &s, scale) > 0) || (n > 0);
			continue;
		default:
			s--;
			break;
		}
		break;
	}
	if (e)
	{
		while (isspace(*s))
			s++;
		*e = s;
	}
	if (scale)
		n = convert(n, 0, scale);
	return n;
}

/*
 * evaluate a conditional expression
 */

static long
cond(register char* s, char** e)
{
	register int	c;
	register int	i;
	register int	q;
	register long	v;
	register char*	t;
	char*		u;

	while (isspace(*s))
		s++;
	if (i = *s == '!')
		s++;
	while (isspace(*s))
		s++;
	switch (c = *s)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '-':
	case '+':
	case '(':
		v = expression(s, &u, 0) > 0;
		s = u;
		break;
	case 'c':
		s++;
		if (*s == 'c' && *(s + 1) == 'h')
			s += 2;
		v = 0;
		break;
	case 'd':
	case 'r':
		t = s;
		while (*++s && !isspace(*s));
		q = *s;
		*s = 0;
		*t = c == 'd' ? '.' : 'n';
		v = hashget(state.symbols, t) != 0;
		*t = c;
		*s = q;
		break;
	case 'g':
		GROFFINIT();
		v = state.groff != 0;
		s++;
		break;
	case 'o':
	case 't':
		v = 1;
		s++;
		break;
	default:
		if (isalpha(c))
		{
			v = 0;
			s++;
		}
		else
		{
			for (u = ++s; *s && *s != c; s++);
			if (*s)
			{
				*s++ = 0;
				for (t = s; *s && *s != c; s++);
				if (*s)
					*s++ = 0;
				v = !strcmp(u, t);
			}
			else 
				v = 0;
		}
		break;
	}
	if (i)
		v = !v;
	while (isspace(*s))
		s++;
	if (e)
		*e = s;
	return v;
}

static void	expand(Sfio_t*, char*);

/*
 * pop input stream
 * return non-0 on EOF
 */

static int
popin(void)
{
	register Pushin_t*	pp;
	int			n;

	if (state.in_top <= state.in_stack)
	{
		state.in--;
		return 1;
	}
	pp = state.in_top;
	if (pp->loop)
	{
		state.in = (unsigned char*)sfstrbase(pp->loop);
		return 0;
	}
	if (--pp == state.in_stack)
	{
		if (state.cond.level > 1 && state.verbose)
			error(1, "%d missing .el request%s", state.cond.level - 1, state.cond.level == 2 ? "" : "s");
		state.cond.level = 0;
	}
	if (pp->ip)
	{
		if (state.in < pp->end)
			return 0;
		if ((n = sfread(pp->ip, pp->buf, SF_BUFSIZE)) > 0)
		{
			pp->end = pp->buf + n;
			*pp->end++ = EOF;
			state.in = pp->buf;
			return 0;
		}
		if (pp->ip && pp->ip != sfstdin)
			sfclose(pp->ip);
		free(pp->buf - UNGET_MAX);
	}
	state.in_top = pp;
	if (state.noline > 0)
		state.noline--;
	if ((error_info.file = pp->file) == state.original.file)
	{
		state.original.file = 0;
		state.original.line = 0;
	}
	error_info.line = pp->line;
	if (pp->tag)
	{
		state.tag = pp->tag;
		state.mac = pp->mac;
	}
	state.in = pp->in;
	return 0;
}

/*
 * copy <type><name> into buf of size n
 */

static char*
nam(int type, register char* name, char* buf, size_t n)
{
	register char*	t = buf;
	register char*	e = t + n - 1;

	*t++ = type;
	if ((*t++ = *name++) && (*t++ = *name))
	{
		if (state.groff)
			while (t < e && (*t++ = *++name));
		else
			*t = 0;
	}
	return buf;
}

/*
 * return number register pointer given name
 * pointer created if not found
 */

static Num_t*
num(register char* s)
{
	register Num_t*	np;
	char		buf[MAXNAME];

	if (!(np = (Num_t*)hashget(state.symbols, nam('n', s, buf, sizeof(buf)))))
	{
		if (!(np = newof(0, Num_t, 1, 0)))
			error(ERROR_SYSTEM|3, "out of space [number]");
		np->name = hashput(state.symbols, NiL, np);
		np->format = '1';
		np->increment = 1;
	}
	return np;
}

/*
 * lookup or create (force!=0) io stream handle
 */

static Stream_t*
iop(char* s, int force)
{
	register Stream_t*	sp;
	char			buf[MAXNAME];

	if (!(sp = (Stream_t*)hashget(state.symbols, nam('s', s, buf, sizeof(buf)))))
	{
if (!force) error(1, "iop %s lookup failed", buf);
		if (!force)
			return 0;
		if (!(sp = newof(0, Stream_t, 1, 0)))
			error(ERROR_SYSTEM|3, "out of space [stream]");
		hashput(state.symbols, NiL, sp);
	}
	return sp;
}

/*
 * push output stream np
 */

static void
pushout(Sfio_t* np)
{
	if (state.out_top >= &state.out_stack[elementsof(state.out_stack)])
		error(3, "output stack overflow");
	*state.out_top++ = state.out;
	iop("stdout", 1)->sp = state.out = np;
}

/*
 * pop ouput stream
 * if retain>0 then strdup() buffer
 * append an extra 0 for easy arg '\n' append by if/ie/el
 */

static char*
popout(void)
{
	char*	s;

	if (!DIVERTED())
		return 0;
	if (state.out == state.tag->sp)
		sfputc(state.out, 0);
	if (state.out == state.nul)
		s = 0;
	else
		s = use(state.out);
	iop("stdout", 1)->sp = state.out = *--state.out_top;
	return s;
}

/*
 * return next character with possible line join
 */

static int
nextchar(void)
{
	int	c;
	int	n;

	for (;;)
	{
		if ((c = GETC()) == EOF)
		{
			if (popin())
				break;
			continue;
		}
		if (c == state.ec)
		{
		escape:
			switch (n = GETC())
			{
			case EOF:
				if (popin())
					break;
				goto escape;
			case '\n':
				error_info.line++;
				continue;
			case 't':
				c = '\t';
				break;
			case '"':
				for (;;)
				{
					switch (c = GETC())
					{
					case EOF:
						if (popin())
							break;
						continue;
					case '\n':
						break;
					default:
						if (c == state.ec)
						{
						escape_2:
							switch (n = GETC())
							{
							case EOF:
								if (popin())
									break;
								goto escape_2;
							case '\n':
								error_info.line++;
								continue;
							default:
								continue;
							}
							break;
						}
						continue;
					}
					break;
				}
				break;
			default:
				UNGETC(n);
				break;
			}
			break;
		}
		break;
	}
	return c;
}

/*
 * call a macro
 */

static void
macro(Tag_t* tp, Arg_t* ap)
{
	if (tp->body)
		pushin(tp->file, tp->line, NiL, tp->body, ap);
}

/*
 * set macro value
 */

static void
set(Tag_t* mp, char* value, int append)
{
	int	n;

	if (append && mp->body)
	{
		sfputr(state.tmp, mp->body, -1);
		sfputr(state.tmp, value, -1);
		value = use(state.tmp);
	}
	n = strlen(value) + 1;
	if (mp->size < n)
	{
		if (mp->size)
			free(mp->body);
		mp->size = n = roundof(n, 64);
		if (!(mp->body = newof(0, char, n, 0)))
			error(ERROR_SYSTEM|3, "out of space [set value]");
	}
	strcpy(mp->body, value);
	mp->call = macro;
	mp->flags |= TAG_PASS;
	if (!mp->line)
	{
		mp->line = error_info.line;
		mp->file = error_info.file;
	}
	if (mp->flags & TAG_TRACE_SET)
		error(2, "set macro %s `%s'", mp->name + 1, mp->body);
}

/*
 * return macro pointer given name
 * pointer created if not found
 */

static Tag_t*
mac(char* name)
{
	register Tag_t*	mp;
	char		buf[MAXNAME];

	if (!(mp = (Tag_t*)hashget(state.symbols, nam('.', name, buf, sizeof(buf)))))
	{
		if (!(mp = newof(0, Tag_t, 1, 0)))
			error(ERROR_SYSTEM|3, "out of space [mac]");
		mp->name = hashput(state.symbols, NiL, mp);
	}
	return mp;
}

static Env_t*
env(char* s)
{
	register Env_t*	v;
	char*		e;
	long		n;
	char		buf[MAXNAME];

	n = expression(s, &e, 0);
	if (*e)
		sfsprintf(buf, sizeof(buf), "E%s", s);
	else
		sfsprintf(buf, sizeof(buf), "E%d", n);
	if (!(v = (Env_t*)hashget(state.symbols, buf)))
	{
		if (!(v = newof(0, Env_t, 1, 0)))
			error(ERROR_SYSTEM|3, "out of space [environment]");
		v->name = hashput(state.symbols, NiL, v);
	}
	if (v->generation < state.generation)
	{
		v->generation = state.generation;
		v->c2 = DEFAULT_c2;
		v->cc = DEFAULT_cc;
		v->ft.current = v->ft.previous = DEFAULT_ft;
		v->nf = 0;
		v->ps.current = v->ps.previous = DEFAULT_ps;
		v->ss = 0;
		v->vs.current = v->vs.previous = DEFAULT_vs;
	}
	return v;
}

/*
 * open name for reading
 * looking in state.dirs if needed
 * verbose<0 for -m option
 */

static Sfio_t*
find(char* name, char** found, int verbose)
{
	register char*		s;
	register Sfio_t*	sp;
	char*			path;
	char*			t;
	char*			x;
	int			i;
	Dir_t*			dp;
	char			buf[PATH_MAX];

	static const char*	pathdirs[] =
				{
					"lib/tmac/tmac.%s",
					"lib/tmac/%s",
					"lib/html/%s",
					"lib/html/m%s.tr",
					"share/groff/tmac/tmac.%s",
					"share/groff/tmac/%s",
					"groff/share/groff/tmac/tmac.%s",
					"groff/share/groff/tmac/%s",
				};

	if (verbose >= 0 && (sp = sfopen(NiL, name, "r")))
	{
		sfprintf(state.tmp, "/%s", name);
		path = use(state.tmp);
		goto hit;
	}
	if (*name != '/')
	{
		if (*name == '.' && (x = getenv("HOME")))
		{
			sfprintf(state.tmp, "/%s/%s", x, name);
			path = use(state.tmp);
			if (sp = sfopen(NiL, path + 1, "r"))
				goto hit;
		}
		for (dp = state.dirs; dp; dp = dp->next)
		{
			if (verbose >= 0)
			{
				sfprintf(state.tmp, "/%s/%s", dp->name, name);
				path = use(state.tmp);
				if (sp = sfopen(NiL, path + 1, "r"))
					goto hit;
			}
			sfprintf(state.tmp, "/%s/tmac.%s", dp->name, name);
			path = use(state.tmp);
			if (sp = sfopen(NiL, path + 1, "r"))
				goto hit;
		}
		for (i = 0; i < elementsof(pathdirs); i++)
		{
			sfprintf(state.tmp, pathdirs[i], name);
			path = use(state.tmp);
			if (pathpath(path, "", PATH_REGULAR|PATH_READ, buf + 1, sizeof(buf) - 1) && (sp = sfopen(NiL, buf + 1, "r")))
			{
				*(path = buf) = '/';
				goto hit;
			}
		}
	}
	if (verbose > 0)
	{
		/*
		 * some systems provide
		 *	/usr/lib/tmac.mXXX
		 * that references
		 *	/usr/lib/macros/mXXX[tn]
		 * that is not provided
		 */

		if (*(s = name) == '/')
			while (s = strchr(s, '/'))
				if (!strncmp(++s, "macros/", 7) && *(s += 7))
				{
					t = s + strlen(s) - 1;
					x = strchr(s, '.') ? "" : ".tr";
					while (*s)
					{
						sfprintf(state.tmp, "lib/html/%s%s", s, x);
						path = use(state.tmp);
						if (pathpath(path, "", PATH_REGULAR|PATH_READ, buf + 1, sizeof(buf) - 1) && (sp = sfopen(NiL, buf + 1, "r")))
						{
							*(path = buf) = '/';
							goto hit;
						}
						if (*t != 't')
							break;
						*t-- = 0;
					}
				}
		error(ERROR_SYSTEM|2, "%s: cannot read", name);
	}
	else if (verbose < 0)
		error(ERROR_SYSTEM|3, "-m%s: cannot find macro package", name);
	return 0;
 hit:
	message((-1, "FIND %s => %s", name, path + 1));
	if (found)
		*found = hashput(state.symbols, path, path + 1) + 1;
	s = path + strlen(path);
	while (s > path + 2)
		if (*--s == '/')
		{
			if (!strcmp(s, "/lib"))
			{
				*s = 0;
				set(mac(".P"), path + 1, 0);
				break;
			}
			*s = 0;
		}
	return sp;
}

/*
 * generate intermediate CODE_0
 */

static void
code_0(void)
{
	sfputc(state.out, CODE_0);
}

/*
 * generate intermediate CODE_1
 */

static void
code_1(int code)
{
	sfputc(state.out, CODE_1);
	sfputc(state.out, code);
}

/*
 * generate intermediate CODE_2
 */

static void
code_2(int code, int data)
{
	sfputc(state.out, CODE_2);
	sfputc(state.out, data);
	sfputc(state.out, code);
}

/*
 * generate intermediate CODE_n
 */

static void
code_n(int code, char* s)
{
	int	n;

	sfputc(state.out, CODE_n);
	n = s ? (strlen(s) + 1) : 1;
	if (n > 0177)
		sfputc(state.out, ((n >> 8) & 0177) | 0200);
	sfputc(state.out, n & 0177);
	sfputc(state.out, code);
	if (s)
		sfputr(state.out, s, 0);
	else
		sfputc(state.out, 0);
}

/*
 * join all args from n on into 1 arg
 */

static char*
join(Arg_t* ap, int n)
{
	if (n < 0 || ap->argc < n)
		return 0;
	n++;
	while (ap->argc >= n)
		*(ap->argv[ap->argc--] - 1) = ' ';
	return ap->argv[n - 1];
}

/*
 * output error message line
 */

static void
notify(register char* s)
{
	register int	c;
	Sfoff_t		p;

	p = sftell(sfstderr);
	for (;;)
	{
		switch (c = *s++)
		{
		case 0:
			break;
		case CODE_0:
			continue;
		case CODE_1:
			s++;
			continue;
		case CODE_2:
			c = *s++;
			if (*s++ == OP_cc)
				sfputc(sfstderr, c);
			continue;
		default:
			sfputc(sfstderr, c);
			continue;
		}
		break;
	}
	if (sftell(sfstderr) != p)
		sfputc(sfstderr, '\n');
}

/*
 * add trap s to xp
 */

static void
trap(Trap_t** xp, char* s)
{
	Trap_t*	ip;
	Trap_t*	pp;

	for (pp = 0, ip = *xp; ip; pp = ip, ip = ip->next)
		if (streq(ip->name + 1, s))
			return;
	if (!(ip = newof(0, Trap_t, 1, strlen(s) + 1)))
		error(ERROR_SYSTEM|3, "out of space [trap]");
	*ip->name = '.';
	strcpy(ip->name + 1, s);
	if (pp)
		pp->next = ip;
	else
		*xp = ip;
}

/*
 * trigger traps on xp
 */

static void
trigger(Trap_t** xp)
{
	Trap_t*	ip;
	Trap_t*	pp;

	if (!DIVERTED() && (ip = *xp))
	{
		state.t = 1;
		do
		{
			sfputr(state.req, ip->name, '\n');
			pp = ip;
			ip = ip->next;
			free(pp);
		} while (ip);
		*xp = 0;
		pushin(xp == &state.fini ? "[*EOF*]" : "[*TRAP*]", 1, NiL, use(state.req), NiL);
	}
}

/*
 * return value for register name with type
 * and increment/decrement i={0,'+','-'}
 */

static char*
value(char* name, int type, int i)
{
	register Num_t*	np;
	register char*	b;
	register char*	x;
	register long	n;
	register long	m;
	Tag_t*		tp;
	char		buf[8];

	b = hashget(state.symbols, name);
	switch (type)
	{
	case 'g':
		if (!(np = (Num_t*)b))
			return "";
		b = np->value;
		if (isalnum(np->format))
			*b++ = np->format;
		else
		{
			for (n = 0; n < np->format; n++)
				*b++ = '0';
			*b++ = '1';
		}
		*b = 0;
		return np->value;
	case 'j':
		return "";
	case 'n':
		break;
	case '.':
		if (!(tp = (Tag_t*)b))
			return 0;
		if (tp->flags & TAG_TRACE_GET)
			error(2, "get macro %s `%s'", tp->name + 1, tp->body);
		return tp->body;
	default:
		return b;
	}
	if (!b)
	{
		name = strcpy(buf, "n");
		if (!(b = (char*)hashget(state.symbols, name)))
			return "0";
	}
	np = (Num_t*)b;
	if (np->flags & TAG_TRACE_GET)
		error(2, "get register %s %d %d", np->name + 1, np->number, np->increment);
	if (np->internal)
	{
		switch (INDEX(name[1], name[2]))
		{
		case INDEX('%',0):
			n = np->number;
			break;
		case INDEX('.','$'):
			n = state.mac ? state.mac->argc : 0;
			break;
		case INDEX('.','C'):
			GROFFINIT();
			n = state.groff == 0;
			break;
		case INDEX('.','F'):
			return state.original.file ? state.original.file : error_info.file;
		case INDEX('.','H'):
			n = 3000;
			break;
		case INDEX('.','L'):
			n = 1;
			break;
		case INDEX('.','P'):
			n = 1;
			break;
		case INDEX('.','R'):
			n = 256;
			break;
		case INDEX('.','V'):
			n = 6000;
			break;
		case INDEX('.','c'):
			if (name[3] == 'e')
			{
				n = state.it.center;
				break;
			}
			/*FALLTHROUGH*/
		case INDEX('c','.'):
			n = state.original.line ? state.original.line : error_info.line;
			break;
		case INDEX('.','e'):
			if (name[3] == 'v')
				n = state.it.center; /*GROFF: string valued?*/
			break;
		case INDEX('.','f'):
			n = state.env->ft.current;
			break;
		case INDEX('.','g'):
			GROFFINIT();
			n = state.groff != 0;
			break;
		case INDEX('.','i'):
			n = state.env->in.current;
			break;
		case INDEX('.','k'):
			n = 100;
			break;
		case INDEX('.','l'):
			n = state.env->ll.current;
			break;
		case INDEX('.','n'):
			n = state.n;
			break;
		case INDEX('.','p'):
			if (name[3] == 'n')
				n = 2;
			else if (name[3] == 's' && name[4] == 'r')
				n = state.env->ps.previous;
			else
				n = state.env->ps.current;
			break;
		case INDEX('.','r'):
			n = 0;
			break;
		case INDEX('.','s'):
			if (name[3] == 'r')
				n = state.env->ps.previous;
			else
				n = state.env->ps.current;
			break;
		case INDEX('.','t'):
			n = state.t;
			break;
		case INDEX('.','u'):
			n = !state.env->nf;
			break;
		case INDEX('.','v'):
			if (name[3] == 'p')
				n = 1;
			else
				n = 6 * state.env->vs.current;
			break;
		case INDEX('.','w'):
			if (name[3] == 'a')
				n = 0;
			else
				n = 6 * state.env->ps.current;
			break;
		case INDEX('.','x'):
			n = 19980401;
			break;
		case INDEX('.','y'):
			n = 1;
			break;
		case INDEX('.','z'):
			return state.divert ? state.divert->tag->name : "";
		case INDEX('d','l'):
			if (state.test & 0x1000) n = 0; else
			n = state.dl;
			break;
		case INDEX('d','n'):
			if (state.test & 0x2000) n = 0; else
			n = state.dn;
			break;
		case INDEX('l','n'):
			if (state.test & 0x4000) n = 0; else
			n = state.ln;
			break;
		case INDEX('n','l'):
			if (!(state.test & 0x8000)) n = 0; else
			n = state.nl;
			break;
		default:
			n = 0;
			break;
		}
		np->number = n;
	}
	if (i)
	{
		switch (i)
		{
		case '+':
			np->number += np->increment;
			break;
		case '-':
			np->number -= np->increment;
			break;
		}
		if (np->flags & TAG_TRACE_SET)
			error(2, "set register %s %d %d", np->name + 1, np->number, np->increment);
	}
	n = np->number;
	b = np->value;
	switch (np->format)
	{
	case '1':
		sfsprintf(b, sizeof(np->value), "%ld", np->number);
		break;
	case 'A':
	case 'a':
		x = islower(np->format) ? "abcdefghijklmnopqrstuvwxyz" : "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		if (n < 0)
		{
			n = -n;
			*b++ = '-';
		}
		for (m = 1; m > 0 && m < n; m *= 26);
		if (m <= 0)
			sfsprintf(b, sizeof(buf) - 1, "<%ld>", n);
		else
		{
			for (; m > 0; m /= 26)
			{
				i = n / m;
				n -= m * i;
				*b++ = x[i];
			}
			*b = 0;
		}
		break;
	case 'I':
	case 'i':
		x = islower(np->format) ? "zwmdclxvi" : "ZWMDCLXVI";
		if (n <= -40000 || n >= 40000)
			sfsprintf(b, sizeof(buf), "<%ld>", n);
		else if (!n)
			sfsprintf(b, sizeof(buf), "0");
		else
		{
			if (n < 0)
			{
				n = -n;
				*b++ = '-';
			}
			while (n >= 10000)
			{
				n -= 10000;
				*b++ = x[0];
			}
			for (i = 1000; i > 0; i /= 10, x += 2)
			{
				m = n / i;
				n -= m * i;
				switch (m)
				{
				case 9:
					*b++ = x[2];
					*b++ = x[0];
					break;
				case 8:
					*b++ = x[1];
					*b++ = x[2];
					*b++ = x[2];
					*b++ = x[2];
					break;
				case 7:
					*b++ = x[1];
					*b++ = x[2];
					*b++ = x[2];
					break;
				case 6:
					*b++ = x[1];
					*b++ = x[2];
					break;
				case 5:
					*b++ = x[1];
					break;
				case 4:
					*b++ = x[2];
					*b++ = x[1];
					break;
				case 3:
					*b++ = x[2];
					/*FALLTHROUGH*/
				case 2:
					*b++ = x[2];
					/*FALLTHROUGH*/
				case 1:
					*b++ = x[2];
					break;
				}
			}
			*b = 0;
		}
		break;
	default:
		sfsprintf(b, sizeof(np->value), "%0*ld", np->format, np->number);
		break;
	}
	return np->value;
}

/*
 * return the value for typed register
 */

static char*
interpolate(int type)
{
	register int	c;
	register char*	b;
	register char*	x;
	register char*	t;
	int		i;
	int		k;
	char		buf[MAXNAME];

	i = k = 0;
	b = buf;
	*b++ = (type == 'g' || type == 'j') ? 'f' : type;
	x = buf + 2;
	do
	{
		switch (c = GETC())
		{
		case EOF:
			if (popin())
				return 0;
			continue;
		case '\n':
			error_info.line++;
			break;
		case '(':
			if (x == buf + 2)
			{
				x++;
				continue;
			}
			break;
		case '[':
			if (state.groff && x == buf + 2)
			{
				x = buf + sizeof(buf) - 2;
				k = 1;
				continue;
			}
			break;
		case ']':
			if (k)
				goto done;
			break;
		case '-':
		case '+':
			if (type == 'n' && b == buf + 1)
			{
				i = c;
				continue;
			}
			break;
		default:
			if (c == state.ec)
			{
			escape:
				switch (c = GETC())
				{
				case EOF:
					if (popin())
						return 0;
					goto escape;
				case '\n':
					error_info.line++;
					continue;
				case '*':
					c = '.';
					/*FALLTHROUGH*/
				case 'n':
					if (t = interpolate(c))
						while (b < x && (*b = *t++))
							b++;
					continue;
				}
			}
			break;
		}
		*b++ = c;
	} while (b < x);
 done:
	*b = 0;
	return value(buf, type, i);
}

/*
 * switch to font s
 */

static void
ft(char* s)
{
	int	c;

	if (!s || (c = *s) < '0' || c > '5')
		c = 0;
	else
		c -= '0';
	if (!c)
		c = state.env->ft.previous;
	state.env->ft.previous = state.env->ft.current;
	state.env->ft.current = c;
	code_1(OP_ft + c);
}

/*
 * switch to point size n
 */

static void
ps(int n)
{
	state.env->ps.previous = state.env->ps.current;
	state.env->ps.current = n;
	code_2(OP_ps, n);
}

/*
 * sync list state
 */

static void
li(int force)
{
	List_t*	list = state.list;

	state.it.dc = 0;
	if (state.divert || state.env->nf)
	{
		if (state.env->nf)
			state.it.dl = 0;
		return;
	}
	if (force)
	{
		while (state.env->in.current < state.list->in)
		{
			message((-2, "DROP %d %d %d:%d %d:%d", state.list - state.list_stack, state.list->dl, state.list->ti, state.list->in, force ? state.env->ti.current : 0, state.env->in.current));
			if (state.list->dl)
				code_1(END(OP_dl));
			state.list--;
		}
	}
	else
		message((-2, "TEST %d %d %d:%d %d:%d ops=%d df=%d dc=%d dl=%d", state.list - state.list_stack, state.list->dl, state.list->ti, state.list->in, force ? state.env->ti.current : 0, state.env->in.current, state.list - list, force, state.it.dc, state.it.dl));
	if (state.env->in.current > state.list->in || force > 0 && state.env->ti.current > state.list->in)
	{
		if (state.list >= &state.list_stack[elementsof(state.list_stack)-1])
			error(2, "list stack overflow");
		else
			state.list++;
		state.list->dl = 0;
		state.list->in = state.env->in.current;
		state.list->ti = state.env->in.current > state.list->in ? state.env->in.current : state.env->ti.current;
		state.it.dl++;
	}
	else if (!force && state.env->in.current < state.list->in)
		state.it.dc = 1;
	message((-2, "LIST %d %d %d:%d %d:%d ops=%d df=%d dc=%d dl=%d", state.list - state.list_stack, state.list->dl, state.list->ti, state.list->in, force ? state.env->ti.current : 0, state.env->in.current, state.list - list, force, state.it.dc, state.it.dl));
}

/*
 * pop inline traps
 */

static void
it(void)
{
	register List_t*	p;

	if (state.env->nf)
	{
		if (state.env->in.current > 0)
			sfputc(state.out, '\t');
	}
	else if (!DIVERTED())
	{
		if (state.it.dc && ISFILE())
			li(-1);
		p = state.list;
		while (state.it.dl > 0)
		{
			state.it.dl--;
			for (; p >= state.list_stack; p--)
				if (!p->dl)
				{
					p->dl = 1;
					code_1(OP_dl);
					break;
				}
		}
		if (state.it.dt)
		{
			state.it.dt = 0;
			code_1((state.list - state.list_stack) <= 1 ? LABEL(OP_dt) : OP_dt);
			state.it.dd = 1;
		}
		else if (state.it.dd)
		{
			state.it.dd = 0;
			code_1(OP_dd);
		}
	}
}

/*
 * initialize number register
 */

static void
nr(char* s, int v, int format, int internal)
{
	register Num_t*	np;

	np = num(s);
	np->number = v;
	np->format = format;
	if (internal > 0)
		np->internal = internal;
}

/*
 * set time vars to t
 */

static void
tm(time_t t)
{
	Tm_t*	xp;

	state.date = t;
	xp = tmmake(&t);
	nr("dw", xp->tm_wday + 1, 0, 0);
	nr("dy", xp->tm_mday, 0, 0);
	nr("mo", xp->tm_mon + 1, 0, 0);
	nr("yr", xp->tm_year % 100, 2, 0);
	nr("YR", 1900 + xp->tm_year, 4, 0);
}

/*
 * expand if expression s into op
 */

static void
expand(register Sfio_t* op, register char* s)
{
	int	c;
	int	d;
	int	q;
	int	v;
	int	w;
	int	z;
	char*	t;
	char*	b;
	char	buf[MAXNAME];

	for (;;)
	{
		switch (c = *s++)
		{
		case 0:
			break;
		case CODE_2:
			sfputc(op, c);
			c = *s++;
			/*FALLTHROUGH*/
		case CODE_1:
			sfputc(op, c);
			c = *s++;
			/*FALLTHROUGH*/
		case CODE_0:
			sfputc(op, c);
			continue;
		default:
			if (c == state.ec)
				switch (w = c = *s++)
			{
			case 0:
				s--;
				break;
			case 'A':
			case 'C':
			case 'L':
			case 'N':
			case 'R':
			case 'V':
			case 'X':
			case 'Y':
			case 'Z':
			case 'b':
			case 'h':
			case 'l':
			case 'o':
			case 'w':
			case 'x':
				v = 0;
				c = *s++;
				if (c == '(')
					z = 2;
				else
				{
					z = -1;
					if (state.groff && c == '[')
						c = ']';
				}
				b = s;
				for (;;)
				{
					switch (d = *s++)
					{
					case 0:
						s--;
						break;
					case CODE_2:
						s++;
						/*FALLTHROUGH*/
					case CODE_1:
						if (*s++ == OP_cc)
							v++;
						/*FALLTHROUGH*/
					case CODE_0:
						continue;
					default:
						if (d == state.ec)
						{
							switch (d = *s++)
							{
							case 0:
								s--;
								break;
							case 'A':
							case 'C':
							case 'L':
							case 'N':
							case 'R':
							case 'V':
							case 'X':
							case 'Y':
							case 'Z':
							case 'b':
							case 'h':
							case 'l':
							case 'o':
							case 'w':
							case 'x':
								q = *s++;
								for (;;)
								{
									switch (d = *s++)
									{
									case 0:
										s--;
										break;
									case CODE_2:
										s++;
										/*FALLTHROUGH*/
									case CODE_1:
										s++;
										/*FALLTHROUGH*/
									case CODE_0:
										continue;
									default:
										if (d == q)
											break;
										continue;
									}
									break;
								}
								v++;
								break;
							case 'f':
							case 's':
								if (*s)
									s++;
								break;
							case 'g':
							case 'j':
								t = buf;
								*t++ = 'n';
								if ((*t++ = *s++) == '(')
								{
									*(t - 1) = *s++;
									*t++ = *s++;
								}
								*t = 0;
								if (t = value(buf, d, 0))
									v += strlen(t);
								break;
							}
						}
						else if (z > 0)
							z--;
						else if (d == c || z == 0)
							break;
						v++;
						continue;
					}
					break;
				}
				switch (w)
				{
				case 'A':
					sfprintf(op, "1");
					break;
				case 'R':
				case 'X':
				case 'V':
					c = *(s - 1);
					*(s - 1) = 0;
					switch (w)
					{
					case 'R':
						/*HERE*/
						break;
					case 'V':
						if (t = getenv(b))
							sfprintf(op, "%s", t);
						break;
					case 'X':
						/*HERE*/
						break;
					}
					*(s - 1) = c;
					break;
				case 'w':
					sfprintf(op, "%d", convert(v, 1, w));
					break;
				}
				break;
			case 'g':
			case 'j':
				t = buf;
				*t++ = 'n';
				if ((*t++ = *s++) == '(')
				{
					*(t - 1) = *s++;
					*t++ = *s++;
				}
				*t = 0;
				if (t = value(buf, c, 0))
					sfputr(op, t, -1);
				break;
			default:
				sfputc(op, state.ec);
				sfputc(op, c);
				break;
			}
			else
				sfputc(op, c);
			continue;
		}
		break;
	}
}

/*
 * remove macro/register s
 */

static void
rm(char* s)
{
	register Tag_t*	mp;
	char		buf[MAXNAME];

	if (mp = (Tag_t*)hashget(state.symbols, nam('.', s, buf, sizeof(buf))))
	{
		if (mp->flags & TAG_TRACE_SET)
			error(2, "remove macro %s", mp->name + 1);
		if (mp->size)
			free(mp->body);
		if (!(mp->flags & TAG_STATIC))
			free(mp);
		hashput(state.symbols, NiL, NiL);
	}
	if (mp = (Tag_t*)hashget(state.symbols, nam('n', s, buf, sizeof(buf))))
	{
		if (mp->flags & TAG_TRACE_SET)
			error(2, "remove register %s", mp->name + 1);
		if (mp->size)
			free(mp->body);
		if (!(mp->flags & TAG_STATIC))
			free(mp);
		hashput(state.symbols, NiL, NiL);
	}
}

/*
 * skip one conditional block
 * if op!=0 then copy block data to s
 */

static void
skip(char* s, int f, register Sfio_t* op)
{
	register int	c;
	register int	n;

	n = (f & COND_BLOCK) != 0;
	if (s)
		pushin(NiL, 0, NiL, s, NiL);
	else if (!n)
		return;
	for (;;)
	{
		switch (c = GETC())
		{
		case EOF:
			if (s && n <= 0)
				break;
			if (popin())
				break;
			continue;
		case CODE_2:
			if (op)
				sfputc(op, c);
			c = GETC();
			/*FALLTHROUGH*/
		case CODE_1:
			if (op)
				sfputc(op, c);
			c = GETC();
			/*FALLTHROUGH*/
		case CODE_0:
			if (op)
				sfputc(op, c);
			continue;
		case '\n':
			if (op)
				sfputc(op, c);
			error_info.line++;
			if (n <= 0)
				break;
			continue;
		default:
			if (op)
				sfputc(op, c);
			if (c == state.ec)
			{
			escape:
				switch (c = GETC())
				{
				case EOF:
					if (popin())
						break;
					goto escape;
				case '\n':
					if (op)
						sfputc(op, c);
					error_info.line++;
					continue;
				case '{':
					if (op)
						sfputc(op, c);
					n++;
					continue;
				case '}':
					if (op)
						sfputc(op, c);
					if (--n <= 0)
					{
						switch (c = nextchar())
						{
						case '\n':
							if (op)
								sfputc(op, c);
							error_info.line++;
							break;
						default:
							UNGETC(c);
							break;
						}
						break;
					}
					continue;
				default:
					if (op)
						sfputc(op, c);
					continue;
				}
				break;
			}
			continue;
		}
		break;
	}
	if (n > 0 && state.verbose)
		error(2, "unbalanced \\{ ... \\} block");
}

/*
 * internal groff requests
 */

static void
groff_aln(Tag_t* tp, Arg_t* ap)
{
	Tag_t*	xp;
	char	buf[MAXNAME];

	if (ap->argc != 2)
		error(1, "%s: two arguments expected", ap->argv[0]);
	else if (!(xp = (Tag_t*)hashget(state.symbols, nam('n', ap->argv[2], buf, sizeof(buf)))))
		error(1, "%s: not defined", buf);
	else
		hashput(state.symbols, nam('n', ap->argv[1], buf, sizeof(buf)), xp);
}

static void
groff_als(Tag_t* tp, Arg_t* ap)
{
	Tag_t*	xp;
	char	buf[MAXNAME];

	if (ap->argc != 2)
		error(1, "%s: two arguments expected", ap->argv[0]);
	else if (!(xp = (Tag_t*)hashget(state.symbols, nam('.', ap->argv[2], buf, sizeof(buf)))))
		error(1, "%s: not defined", buf);
	else
		hashput(state.symbols, nam('.', ap->argv[1], buf, sizeof(buf)), xp);
}

static void
groff_asciify(Tag_t* tp, Arg_t* ap)
{
	static int	warned;

	if (!warned++)
		error(1, "%s: not implemented yet", tp->name);
}

static void
groff_break(Tag_t* tp, Arg_t* ap)
{
	register Pushin_t*	pp;

	for (pp = state.in_top; pp >= state.in_stack; pp--)
		if (pp->loop)
		{
			sfclose(pp->loop);
			pp->loop = 0;
			return;
		}
	error(1, "%s: outside of loop", tp->name);
}

static void
groff_chop(Tag_t* tp, Arg_t* ap)
{
	register char*	s;
	register Tag_t*	mp;

	if (ap->argc >= 1)
	{
		mp = mac(ap->argv[1]);
		if ((s = mp->body) && *s)
			*(s + strlen(s) - 1) = 0;
	}
}

static void
groff_close(Tag_t* tp, Arg_t* ap)
{
	Stream_t*	sp;

	if (ap->argc < 1)
	{
		error(1, "%s: one argument expected", ap->argv[0]);
		return;
	}
	if (!(sp = iop(ap->argv[1], 0)))
	{
		error(1, "%s: %s: stream not open", ap->argv[0], ap->argv[1]);
		return;
	}
	sfclose(sp->sp);
	sp->sp = 0;
}

static void
groff_continue(Tag_t* tp, Arg_t* ap)
{
	register Pushin_t*	pp;

	for (pp = state.in_top; pp >= state.in_stack; pp--)
		if (pp->loop)
		{
			pp->in = (unsigned char*)sfstrbase(pp->loop);
			return;
		}
	error(1, "%s: outside of loop", tp->name);
}

static void
groff_cp(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	if (ap->argc < 1 || expression(ap->argv[1], NiL, 0))
		state.groff &= 2;
	else
		state.groff |= 1;
}

static void
groff_open(Tag_t* tp, Arg_t* ap)
{
	char*		path;
	char*		mode;
	Stream_t*	sp;

	if (ap->argc < 2)
	{
		error(1, "%s: two arguments expected", ap->argv[0]);
		return;
	}
	mode = strlen(ap->argv[0]) > 5 ? (ap->argv[0] + 5) : "w";
	path = ap->argv[2];
	sp = iop(ap->argv[1], 1);
	if (*mode == 'w' || !streq(sp->path, path))
	{
		if (sp->sp)
		{
			sfclose(sp->sp);
			sp->sp = 0;
		}
		if (sp->path)
			free(sp->path);
		if (!(sp->path = strdup(path)))
			error(ERROR_SYSTEM|3, "out of space [stream path]");
	}
	if (!sp->sp && !(sp->sp = sfopen(NiL, path, mode)))
		error(ERROR_SYSTEM|2, "%s: %s: \"%s\" mode open error", ap->argv[0], path, mode);
}

static void
groff_pso(Tag_t* tp, Arg_t* ap)
{
	char*	s;
	Sfio_t*	sp;

	if (s = join(ap, 1))
	{
		if (!(sp = sfpopen(NiL, s, "r")))
			error(ERROR_SYSTEM|2, "%s: %s: command error", ap->argv[0], s);
		else
			pushin(NiL, 0, sp, NiL, NiL);
	}
}

static void
groff_rnn(Tag_t* tp, Arg_t* ap)
{
	Num_t*	mp;
	char*	s;

	NoP(tp);
	if (ap->argc >= 2)
	{
		s = ap->argv[2];
		rm(s);
		*--s = 'n';
		mp = num(ap->argv[1]);
		hashput(state.symbols, NiL, NiL);
		mp->name = hashput(state.symbols, s, mp);
	}
}

static void
groff_shift(Tag_t* tp, Arg_t* ap)
{
	register int	n;
	register int	i;
	register Arg_t*	pp;

	if (pp = state.mac)
	{
		n = (ap->argc >= 1) ? expression(ap->argv[1], NiL, 0) : 1;
		i = 0;
		while (n < pp->argc)
			pp->argv[++i] = pp->argv[++n];
		pp->argc = i;
	}
}

static void
groff_sy(Tag_t* tp, Arg_t* ap)
{
	char*	s;

	nr("systat", (long)((s = join(ap, 1)) ? system(s) : 0), 0, 0);
}

static void
groff_while(Tag_t* tp, Arg_t* ap)
{
	register char*	s;
	register char*	t;
	Sfio_t*		op;

	if (s = join(ap, 1))
	{
		if (!(op = sfstropen()))
			error(ERROR_SYSTEM|3, "out of space [%s]", tp->name);
		sfputr(op, ".ie", ' ');
		sfputr(op, s, '\n');
		t = s + strlen(s);
		while (t > s && isspace(*--t));
		if (t > s && *t == '{' && *(t - 1) == state.ec)
			skip(NiL, COND_BLOCK, op);
		sfputr(op, "\n.el .break\n", 0);
		pushin(NiL, 0, NiL, sfstrbase(op), NiL);
		state.in_top->loop = op;
	}
}

static void
groff_write(Tag_t* tp, Arg_t* ap)
{
	char*		s;
	Stream_t*	sp;

	if (ap->argc < 1)
	{
		error(1, "%s: at least one argument expected", ap->argv[0]);
		return;
	}
	if (!(sp = iop(ap->argv[1], 0)))
	{
		error(1, "%s: %s: stream not open", ap->argv[0], ap->argv[1]);
		return;
	}
	if (!(s = join(ap, 2)))
		s = "";
	if (*s == '"')
		s++;
	if (sfputr(sp->sp, s, '\n') < 0)
		error(ERROR_SYSTEM|2, "%s: %s: write error", ap->argv[0], ap->argv[1]);
}

/*
 * internal troff requests
 */

static void
troff_ab(Tag_t* tp, Arg_t* ap)
{
	char*	s;

	NoP(tp);
	if (!(s = join(ap, 1)))
		s = "User abort";
	notify(s);
	exit(1);
}

static void
troff_ad(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	if (ap->argc >= 1)
		switch (ap->argv[1][0])
		{
		case 'b':
		case 'B':
		case 'n':
		case 'N':
			break;
		case 'c':
		case 'C':
			break;
		case 'l':
		case 'L':
			break;
		case 'r':
		case 'R':
			break;
		}
}

static void
troff_af(Tag_t* tp, Arg_t* ap)
{
	Num_t*	np;
	char*	s;

	NoP(tp);
	if (ap->argc >= 2)
	{
		np = num(ap->argv[1]);
		switch (*(s = ap->argv[2]))
		{
		case '0':
			for (np->format = 0; *s++ == '0'; np->format++);
			break;
		case 'a':
		case 'A':
		case 'i':
		case 'I':
			np->format = *s;
			break;
		case '1':
		default:
			np->format = '1';
			break;
		}
	}
}

static void
troff_br(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	NoP(ap);
	if (state.it.dd)
	{
		state.it.dd = 0;
		code_1(OP_dd);
	}
	code_1(OP_br);
}

static void
troff_c2(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	state.env->c2 = (ap->argc >= 1) ? ap->argv[1][0] : DEFAULT_c2;
}

static void
troff_cc(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	state.env->cc = (ap->argc >= 1) ? ap->argv[1][0] : DEFAULT_cc;
}

static void
troff_ce(Tag_t* tp, Arg_t* ap)
{
	int	n;
	int	r;

	NoP(tp);
	n = (ap->argc >= 1) ? expression(ap->argv[1], NiL, 0) : 1;
	r = *(ap->argv[0] + 1) == 'r';
	if (n >= 1)
	{
		if (state.it.center && state.it.right != r)
		{
			state.it.center = 0;
			code_1(state.it.right ? END(OP_p) : END(OP_center));
		}
		if (!state.it.center)
		{
			if (r)
				code_2(OP_p, ARG_right);
			else
				code_1(OP_center);
		}
	}
	else
	{
		if (state.it.right)
			code_1(END(OP_p));
		else if (state.it.center)
			code_1(END(OP_center));
		n = 0;
		r = 0;
	}
	state.it.center = n;
	state.it.right = r;
}

static void
troff_cf(Tag_t* tp, Arg_t* ap)
{
	Sfio_t*	sp;

	if (ap->argc >= 1 && (sp = find(ap->argv[1], NiL, 1)))
	{
		sfmove(sp, state.out, SF_UNBOUND, -1);
		sfclose(sp);
	}
}

static void	troff_wh(Tag_t*, Arg_t*);

static void
troff_ch(Tag_t* tp, Arg_t* ap)
{
	char*	s;
	Trap_t*	xp;

	NoP(tp);
	NoP(ap);
	if (ap->argc == 1)
	{
		for (xp = state.trap; xp; xp = xp->next)
			if (streq(xp->name + 1, ap->argv[1]))
				xp->name[0] = 0;
	}
	else if (ap->argc >= 2)
	{
		s = ap->argv[1];
		ap->argv[1] = (state.test & 1) ? "1" : ap->argv[2];
		ap->argv[2] = s;
		troff_wh(tp, ap);
	}
}

static void
troff_de(Tag_t* tp, Arg_t* ap)
{
	register Tag_t*	mp;

	if (ap->argc >= 1)
	{
		mp = mac(ap->argv[1]);
		if (mp->body && tp->name[1] == 'a')
			sfputr(state.tmp, mp->body, -1);
		mp->file = error_info.file;
		mp->line = error_info.line;
		mp->flags |= TAG_PASS;
		mp->call = macro;
		state.define = mp;
		state.end = (Tag_t*)hashget(state.symbols, "..");
		state.pass = 1;
		pushout(state.tmp);
	}
}

static void
troff_di(Tag_t* tp, Arg_t* ap)
{
	Divert_t*	dp;
	int		n;

	if (!ap->argc)
	{
		if (dp = state.divert)
		{
			state.dl = state.env->dl;
			state.dn = state.env->dn;
			if (dp->env->ft.current != state.env->ft.current)
				code_1(OP_ft + dp->env->ft.current);
			if (dp->env->ps.current != state.env->ps.current)
				code_2(OP_ps, dp->env->ps.current);
			state.env = dp->env;
			state.pass = 0;
			state.divert = dp->next;
			if ((n = sfstrtell(state.out)) > 0 && *(sfstrbase(state.out) + n - 1) != '\n')
				sfputc(state.out, '\n');
			set(dp->tag, popout(), 0);
			message((-2, "%s: pop diversion `%s'", dp->tag->name, dp->tag->body));
			sfstrclose(dp->sp);
			free(dp);
		}
	}
	else if (!(dp = newof(0, Divert_t, 1, 0)) || !(dp->sp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [divert]");
	else
	{
		dp->tag = mac(ap->argv[1]);
		if (dp->tag->body && tp->name[2] == 'a')
			sfputr(dp->sp, dp->tag->body, -1);
		dp->next = state.divert;
		state.divert = dp;
		state.pass = 1;
		pushout(dp->sp);
		dp->env = state.env;
		dp->environment = *state.env;
		state.env = &dp->environment;
		state.env->dl = 0;
		state.env->dn = 0;
	}
}

static void
troff_ds(Tag_t* tp, Arg_t* ap)
{
	char*	v;

	if (v = join(ap, 2))
		set(mac(ap->argv[1]), v, tp->name[1] == 'a');
}

static void
troff_ec(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	state.ec = state.eo = (ap->argc >= 1) ? ap->argv[1][0] : DEFAULT_ec;
}

static void
troff_eo(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	if (state.ec)
	{
		state.eo = state.ec;
		state.ec = 0;
	}
}

static void
troff_ev(Tag_t* tp, Arg_t* ap)
{
	register Env_t*	oe;
	register Env_t*	ne;

	if (ap->argc < 1)
	{
		if (state.env_sp <= state.env_stack)
			error(2, "%s: stack underflow", tp->name);
		else
			state.env_sp--;
	}
	else if (state.env_sp >= &state.env_stack[elementsof(state.env_stack)])
		error(2, "%s: stack overflow", tp->name);
	else
		*++state.env_sp = env(ap->argv[1]);
	oe = state.env;
	ne = state.env = *state.env_sp;
	if (oe != ne)
	{
		if (oe->ft.current != ne->ft.current)
			code_1(OP_ft + ne->ft.current);
		if (oe->nf != ne->nf)
			code_1(ne->nf ? OP_pre : END(OP_pre));
		if (oe->ps.current != ne->ps.current)
			code_2(OP_ps, ne->ps.current);
	}
}

static void
troff_em(Tag_t* tp, Arg_t* ap)
{
	int	i;

	NoP(tp);
	for (i = 1; i <= ap->argc; i++)
		trap(&state.fini, ap->argv[i]);
}

static void
troff_fi(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	NoP(ap);
	if (state.env->nf)
	{
		state.env->nf = 0;
		code_1(END(OP_pre));
	}
}

static void
troff_fp(Tag_t* tp, Arg_t* ap)
{
	char*	s;
	char*	t;

	NoP(tp);
	if (ap->argc >= 2)
	{
		s = ap->argv[1];
		*--s = 'f';
		t = ap->argv[1];
		*--t = 'f';
		hashput(state.symbols, s, hashget(state.symbols, t));
	}
}

static void
troff_ft(Tag_t* tp, Arg_t* ap)
{
	char*	s;

	NoP(tp);
	if (ap->argc >= 1)
	{
		s = ap->argv[1];
		*--s = 'f';
		s = (char*)hashget(state.symbols, s);
	}
	else s = 0;
	ft(s);
}

static void
troff_ie(Tag_t* tp, Arg_t* ap)
{
	int	f;
	int	q;
	long	v;
	char*	s;
	char*	t;
	char*	e;

	if (tp->name[1] == 'e')
	{
		if (!state.cond.level || !(state.cond.flags[state.cond.level] & COND_IE))
		{
			if (state.verbose)
				error(2, "%s: no matching .ie", tp->name);
			if (!state.cond.level)
				state.cond.level++;
			state.cond.flags[state.cond.level] = COND_IE|COND_KEPT;
		}
		f = COND_EL;
	}
	else
	{
		if (state.cond.level >= elementsof(state.cond.flags) - 1)
			error(3, "%s: conditional stack too deep", tp->name);
		f = tp->name[2] == 'f' ? COND_IF : COND_IE;
	}
	if (error_info.trace <= -5)
	{
		int	g = state.cond.flags[state.cond.level];

		error(-5, "IE +++ %s level=%d |%s%s%s%s%s", tp->name, state.cond.level, (g & COND_IF) ? "IF|" : "", (g & COND_IE) ? "IE|" : "", (g & COND_BLOCK) ? "BLOCK|" : "", (g & COND_SKIP) ? "SKIP|" : "", (g & COND_KEPT) ? "KEPT|" : "");
	}
	if (s = join(ap, 1))
	{
		if (state.ec)
		{
			t = s;
			while (t = strchr(t, state.ec))
				if (*++t != '{' && *t != '}')
					break;
			if (t)
			{
				if (state.test & 0x100)
					error(2, "EXPAND +++ `%s'", s);
				expand(state.arg, s);
				sfputr(ap->sp, use(state.arg), 0);
				s = use(ap->sp);
				if (state.test & 0x100)
					error(2, "EXPAND --- `%s'", s);
			}
		}
		for (;;)
		{
			if (f & COND_EL)
				v = !(state.cond.flags[state.cond.level] & COND_KEPT);
			else
			{
				v = cond(s, &e);
				s = e;
			}
			if (v || s[0] != state.env->cc && s[0] != state.env->c2 || s[1] != 'i')
				break;
			q = f;
			if (s[2] == 'e')
				f = COND_IE|COND_KEPT|COND_SKIP;
			else if (s[2] == 'f')
				f = COND_IF|COND_SKIP;
			else
				break;
			if (q & COND_EL)
				state.cond.level--;
		}
		if (*s == state.ec && state.ec && *(s + 1) == '{')
		{
			s += 2;
			f |= COND_BLOCK;
		}
		while (isspace(*s))
			s++;
		if (!*s)
			s = 0;
		else
			*(s + strlen(s)) = '\n';
	}
	if (!v)
	{
		f |= COND_SKIP;
		skip(s, f, NiL);
	}
	else
	{
		if (s)
		{
			pushin(NiL, 0, NiL, s, ap);
			error_info.line--;
		}
		if (f & (COND_EL|COND_IE))
			f |= COND_KEPT;
	}
	if (f & COND_EL)
	{
		if ((f & (COND_SKIP|COND_BLOCK)) == COND_BLOCK)
			state.cond.flags[state.cond.level] = COND_IE|COND_EL|COND_KEPT|COND_BLOCK;
		else
			state.cond.level--;
	}
	else if ((f & COND_IE) || (f & (COND_BLOCK|COND_SKIP)) == COND_BLOCK)
		state.cond.flags[++state.cond.level] = f;
	if (error_info.trace <= -5)
	{
		int	g = state.cond.flags[state.cond.level];

		error(-5, "IE --- %s level=%d |%s%s%s%s%s", tp->name, state.cond.level, (g & COND_IF) ? "IF|" : "", (g & COND_IE) ? "IE|" : "", (g & COND_BLOCK) ? "BLOCK|" : "", (g & COND_SKIP) ? "SKIP|" : "", (g & COND_KEPT) ? "KEPT|" : "");
	}
}

static void
troff_ignore(Tag_t* tp, Arg_t* ap)
{
	char*	s;

	if (!state.end)
	{
		*tp->name = 'e';
		state.end = (s = (char*)hashget(state.symbols, tp->name)) ? (Tag_t*)hashget(state.symbols, s) : (Tag_t*)0;
		*tp->name = '.';
		if (state.end)
		{
			sfprintf(state.out, "<BR>%s ... %s OMITTED<BR>\n", tp->name, state.end->name);
			state.pass = 1;
			pushout(state.nul);
		}
	}
}

static void
troff_in(Tag_t* tp, Arg_t* ap)
{
	int	n;

	NoP(tp);
	if (ap->argc < 1)
		n = state.env->in.previous;
	else
	{
		n = expression(ap->argv[1], NiL, 'u');
		switch (ap->argv[1][0])
		{
		case '-':
		case '+':
			n += state.env->in.current;
			break;
		}
	}
	state.env->in.previous = state.env->in.current;
	state.env->in.current = n;
	li(0);
}

static void
troff_it(Tag_t* tp, Arg_t* ap)
{
	char*	s;
	char*	t;

	NoP(tp);
	state.it.count = 0;
	if (ap->argc >= 2 && (state.it.count = expression(ap->argv[1], NiL, 0)) > 0 && *(s = ap->argv[2]))
	{
		t = state.it.trap;
		*t++ = '.';
		*t++ = *s++;
		if (*s)
		{
			*t++ = *s;
			if (state.groff)
				while (t < &state.it.trap[sizeof(state.it.trap)-2] && (*t = *++s))
					t++;
		}
		*t++ = '\n';
		*t = 0;
	}
}

static void
troff_ll(Tag_t* tp, Arg_t* ap)
{
	int	n;

	NoP(tp);
	if (ap->argc < 1)
		n = state.env->ll.previous;
	else
	{
		n = expression(ap->argv[1], NiL, 'u');
		switch (ap->argv[1][0])
		{
		case '-':
		case '+':
			n += state.env->ll.current;
			break;
		}
	}
	state.env->ll.previous = state.env->ll.current;
	state.env->ll.current = n;
	li(0);
}

static void
troff_ne(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	state.t = (ap->argc < 1) ? 1 : INT_MAX;
}

static void
troff_nf(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	NoP(ap);
	if (!state.env->nf)
	{
		if (!state.it.interrupt)
			it();
		li(-1);
		state.env->nf = 1;
		code_1(OP_pre);
	}
}

static void
troff_nr(Tag_t* tp, Arg_t* ap)
{
	int	i;
	long	n;
	char*	s;
	Num_t*	np;

	NoP(tp);
	if (ap->argc >= 2)
	{
		np = num(ap->argv[1]);
		if ((i = *(s = ap->argv[2])) == '+' || i == '-')
			s++;
		n = expression(s, NiL, 0);
		switch (i)
		{
		case '-':
			n = np->number - n;
			break;
		case '+':
			n = np->number + n;
			break;
		}
		np->number = n;
		if (np->internal)
			switch (INDEX(np->name[1], np->name[2]))
			{
			case INDEX('c','.'):
				if (state.original.line)
					state.original.line = n;
				else
					error_info.line = n;
				break;
			case INDEX('d','l'):
				state.dl = n;
				break;
			case INDEX('d','n'):
				state.dn = n;
				break;
			case INDEX('l','n'):
				state.ln = n;
				break;
			case INDEX('n','l'):
				state.nl = n;
				break;
			}
		if (ap->argc >= 3)
			np->increment = expression(ap->argv[3], NiL, 0);
		if (np->flags & TAG_TRACE_SET)
			error(2, "set register %s %d %d", np->name + 1, np->number, np->increment);
	}
}

static void
troff_pc(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	state.pc = (ap->argc >= 1) ? ap->argv[1][0] : 0;
}

static void
troff_ps(Tag_t* tp, Arg_t* ap)
{
	int	n;

	NoP(tp);
	if (ap->argc < 1)
		n = state.env->ps.previous;
	else
	{
		n = expression(ap->argv[1], NiL, 'p');
		switch (ap->argv[1][0])
		{
		case '-':
		case '+':
			n += state.env->ps.current;
			break;
		default:
			if (!n)
				n = state.env->ps.previous;
			break;
		}
	}
	if (n > 0)
		ps(n);
}

static void
troff_rm(Tag_t* tp, Arg_t* ap)
{
	int	i;

	NoP(tp);
	for (i = 1; i <= ap->argc; i++)
		rm(ap->argv[i]);
}

static void
troff_rn(Tag_t* tp, Arg_t* ap)
{
	Tag_t*	mp;
	char*	s;

	NoP(tp);
	if (ap->argc >= 2)
	{
		s = ap->argv[2];
		rm(s);
		*--s = '.';
		mp = mac(ap->argv[1]);
		hashput(state.symbols, NiL, NiL);
		mp->name = hashput(state.symbols, s, mp);
	}
}

static void
rr(char* name)
{
	register Num_t*	np;
	char		buf[MAXNAME];

	if (np = (Num_t*)hashget(state.symbols, nam('.', name, buf, sizeof(buf))))
	{
		if (np->internal)
			return;
		if (np->flags & TAG_TRACE_SET)
			error(2, "remove register %s", np->name + 1);
		free(np);
		hashput(state.symbols, NiL, NiL);
	}
}

static void
troff_rr(Tag_t* tp, Arg_t* ap)
{
	int	i;

	NoP(tp);
	for (i = 1; i <= ap->argc; i++)
		rr(ap->argv[i]);
}

static void
troff_so(Tag_t* tp, Arg_t* ap)
{
	Sfio_t*	sp;
	char*	path;

	NoP(tp);
	if (ap->argc >= 1 && (sp = find(ap->argv[1], &path, 1)))
		pushin(path, 1, sp, NiL, NiL);
}

static void
troff_sp(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	if (state.it.dd)
	{
		state.it.dd = 0;
		code_1(OP_dd);
	}
	code_1((ap->argc < 1 || expression(ap->argv[1], NiL, 'v') > 0) ? OP_p : OP_br);
}

static void
troff_ta(Tag_t* tp, Arg_t* ap)
{
	int		i;
	unsigned char	ta[elementsof(state.ta)];

	NoP(tp);
	if (ap->argc < 1)
	{
		state.ta[0] = 8;
		i = 1;
	}
	else
	{
		if (ap->argc >= elementsof(ta))
			ap->argc = elementsof(ta) - 1;
		for (i = 0; i < ap->argc; i++)
			ta[i] = expression(ap->argv[i+1], NiL, 'u');
	}
	ta[i] = 0;
	code_n(OP_ta, (char*)ta);
}

static void
troff_ti(Tag_t* tp, Arg_t* ap)
{
	int	n;

	NoP(tp);
	if (ap->argc < 1)
		n = state.env->ti.previous;
	else
	{
		n = expression(ap->argv[1], NiL, 'u');
		switch (ap->argv[1][0])
		{
		case '-':
		case '+':
			n += state.env->ti.current;
			break;
		}
	}
	state.env->ti.previous = state.env->ti.current;
	state.env->ti.current = n;
	li(1);
	if (state.list > state.list_stack && state.env->ti.current < state.list->in)
		state.it.dt = 1;
}

static void
troff_tl(Tag_t* tp, Arg_t* ap)
{
	register int	c;
	register int	q;
	register char*	s;
	register char*	t;
	int		n;

	NoP(tp);
	if (s = join(ap, 1))
	{
		if (state.head)
			state.footer = 1;
		else
		{
			state.head = 1;
			code_1(END(OP_head));
			code_1(OP_body);
		}
		code_1(OP_h3);
		code_2(OP_tab, ARG_wide);
		code_1(OP_tab_row);
		if (q = *s++)
			for (n = 1; n <= 3; n++)
			{
				code_2(OP_tab_head, n);
				if (n == 1)
					code_2(OP_cc, ' ');
				while ((c = *s++) != q)
				{
					if (!c)
					{
						s--;
						break;
					}
					if (c != state.pc)
						sfputc(state.out, c);
					else if (t = value("n%", 'n', 0))
						sfputr(state.out, t, -1);
				}
				if (n == 1)
					code_2(OP_cc, ' ');
			}
		code_1(END(OP_tab));
		code_1(END(OP_h3));
		sfputc(state.out, '\n');
	}
}

static void
troff_tm(Tag_t* tp, Arg_t* ap)
{
	char*	s;

	NoP(tp);
	if (s = join(ap, 1))
		notify(s);
}

static void
troff_vs(Tag_t* tp, Arg_t* ap)
{
	int	n;

	NoP(tp);
	if (ap->argc < 1)
		n = state.env->vs.previous;
	else
	{
		n = expression(ap->argv[1], NiL, 'p');
		switch (ap->argv[1][0])
		{
		case '-':
		case '+':
			n += state.env->vs.current;
			break;
		default:
			if (!n)
				n = state.env->vs.previous;
			break;
		}
	}
	if (n > 0)
	{
		state.env->vs.previous = state.env->vs.current;
		state.env->vs.current = n;
	}
}

/*
 * return 1 if any non-diverted text escaped
 */

static int
text(void)
{
	register char*	s;
	register char*	e;

	if (state.it.text)
		return 1;
	s = sfstrbase(state.out);
	e = sfstrseek(state.out, 0, SEEK_CUR);
	while (s < e)
	{
		switch (*s++)
		{
		case CODE_2:
			s++;
			/*FALLTHROUGH*/
		case CODE_1:
			s++;
			/*FALLTHROUGH*/
		case ' ':
		case '\t':
		case '\n':
		case CODE_0:
			continue;
		default:
			message((-9, "TEXT begin"));
			return 1;
		}
	}
	return 0;
}

static void
troff_wh(Tag_t* tp, Arg_t* ap)
{
	int		i;
	Trap_t**	xp;

	if (ap->argc > 1)
	{
		if (!(i = expression(ap->argv[1], NiL, 'u')))
		{
			if (!state.it.text && text())
				state.it.text = 1;
			if (state.it.text)
				i = -1;
		}
		xp = (i < 0) ? &state.fini : &state.trap;
		for (i = 2; i <= ap->argc; i++)
			trap(xp, ap->argv[i]);
	}
}

static void
hot(register char* s, int add)
{
	register Dir_t*	x;
	register Dir_t*	p;

	for (p = 0, x = state.hot; x; p = x, x = x->next)
		if (streq(s, x->name))
		{
			if (!add)
			{
				if (p)
					p->next = x->next;
				else
					state.hot = state.hot->next;
				free(x);
			}
			return;
		}
	if (add)
	{
		if (!(x = newof(0, Dir_t, 1, strlen(s) + 1)))
			error(ERROR_SYSTEM|3, "out of space [hot]");
		strcpy(x->name = (char*)x + sizeof(*x), s);
		x->next = state.hot;
		state.hot = x;
	}
}

#define OPT_SWITCH	1
#define OPT_begin	2
#define OPT_debug	3
#define OPT_end		4
#define OPT_footnote	5
#define OPT_get		6
#define OPT_hot		7
#define OPT_label	8
#define OPT_link	9
#define OPT_test	10
#define OPT_toolbar	11
#define OPT_set		12

typedef struct
{
	const char*	name;
	char**		value;
	int		index;
} Option_t;

static Option_t	options[] =
{
	"author",	&state.author,		0,
	"background",	&state.background,	0,
	"begin",	0,			OPT_begin,
	"company",	&state.company,		0,
	"corporation",	&state.corporation,	0,
	"debug",	0,			OPT_debug,
	"end",		0,			OPT_end,
	"footnote",	0,			OPT_footnote,
	"get",		0,			OPT_get,
	"hot",		0,			OPT_hot,
	"label",	0,			OPT_label,
	"link",		0,			OPT_link,
	"location",	&state.location,	0,
	"logo",		&state.logo,		0,
	"mailto",	&state.mailto,		0,
	"organization",	&state.organization,	0,
	"package",	&state.package,		0,
	"set",		0,			OPT_set,
	"test",		0,			OPT_test,
	"title",	&state.title,		0,
	"toolbar",	&state.toolbar,		OPT_toolbar,
	"verbose",	(char**)&state.verbose,	OPT_SWITCH,
	0,		0,			0
};

/*
 * called by stropt() to set name=value
 */

static int
setopt(void* a, const void* x, register int n, const char* v)
{
	register char*		s;
	register char*		t;
	register Option_t*	p = (Option_t*)x;
	register int		f;

	if (p)
		switch (p->index)
		{
		case OPT_begin:
			sfprintf(state.tmp, "<!-- %s -->", v + n);
			code_n(OP_RAW, use(state.tmp));
			break;
		case OPT_debug:
			error_info.trace = n ? -expression((char*)v, NiL, 'u') : 0;
			break;
		case OPT_end:
			sfprintf(state.tmp, "<!-- /%s -->", v + n);
			code_n(OP_RAW, use(state.tmp));
			break;
		case OPT_footnote:
			if (!state.out)
				error(1, "%s: option valid from document body only", p->name);
			else
			{
				/*
				 * NOTE: mm .FS/.FE is so convoluted that I
				 *       punted to this; trying to decipher
				 *	 it prompted .xx [debug|get|set] in
				 *	 the first place; I'll get back to a
				 *	 real solution on a meetingless day.
				 */

				if (n)
				{
					register char*	b;
					long		m;

					m = sfstrtell(state.out);
					b = sfstrbase(state.out);
					s = b + m;
					while (s > b)
						if (!isspace(*--s))
						{
							s++;
							break;
						}
					t = s;
					while (s > b)
						if (isspace(*--s))
						{
							s++;
							break;
						}
					sfprintf(state.tmp, "FN%ld\t%-.*s", m, t - s, s);
					sfstrseek(state.out, s - b, SEEK_SET);
					code_n(OP_link, use(state.tmp));
					sfprintf(state.tmp, "FN%ld", m);
					code_n(OP_fn, use(state.tmp));
				}
				else
					code_1(END(OP_fn));
			}
			break;
		case OPT_get:
		case OPT_hot:
		case OPT_set:
			switch (p->index)
			{
			case OPT_get:
				f = TAG_TRACE_GET;
				break;
			case OPT_hot:
				f = 0;
				break;
			case OPT_set:
				f = TAG_TRACE_SET;
				break;
			}
			s = (char*)v;
			do
			{
				while (isspace(*s))
					s++;
				for (t = s;; t++)
					if (!*t)
					{
						t = 0;
						break;
					}
					else if (isspace(*t))
					{
						*t++ = 0;
						break;
					}
				if (!s[0])
					break;
				if (s[0] == '+' && !s[1])
					n = 1;
				else if (s[0] == '-' && !s[1])
					n = 0;
				else if (!f)
					hot(s, n);
				else if (n)
				{
					num(s)->flags |= f;
					mac(s)->flags |= f;
				}
				else
				{
					num(s)->flags &= ~f;
					mac(s)->flags &= ~f;
				}
			} while (s = t);
			break;
		case OPT_label:
			if (!state.out)
				error(1, "%s: option valid from document body only", p->name);
			else if (n)
			{
				sfprintf(state.tmp, "%s\t", v);
				code_n(LABEL(OP_link), use(state.tmp));
			}
			break;
		case OPT_link:
			if (!state.out)
				error(1, "%s: option valid from document body only", p->name);
			else if (n)
				code_n(OP_link, (char*)v);
			break;
		case OPT_test:
			if (n)
				state.test |= expression((char*)v, NiL, 'u');
			else
				state.test &= ~expression((char*)v, NiL, 'u');
			break;
		case OPT_SWITCH:
			*((int*)p->value) = n && expression((char*)v, NiL, 'u');
			break;
		default:
			if (*p->value)
				free(*p->value);
			*p->value = n && v ? strdup(v) : (char*)0;
			break;
		}
	else if (a)
	{
		if (strneq(v, "meta.", 5))
		{
			sfprintf(state.tmp, "<META name=\"%s\" content=\"%s\">", v + 5, v + n);
			code_n(OP_RAW, use(state.tmp));
		}
		else
			error(1, "%s: unknown option", v);
	}
	return 0;
}

static void
troff_xx(Tag_t* tp, Arg_t* ap)
{
	NoP(tp);
	stropt(join(ap, 1), options, sizeof(*options), setopt, options);
}

#define COMMENT		001
#define COPY		002
#define EAT		004
#define RAW		010
#define STRING		020

#define ONE() \
	do \
	{\
		if (cc <= 0) \
		{ \
			if (cc < 0) \
				cc++; \
			if (!argc && !state.it.interrupt) \
				it(); \
		} \
		cc++; \
	} while (0)

/*
 * convert troff file ip to intermediate file op
 */

static void
process(char* file, Sfio_t* ip, Sfio_t* op)
{
	register int	c;
	register int	cc;
	register int	lastc;
	register char*	s;
	int		argc;
	int		quote;
	int		n;
	int		m;
	unsigned char*	map;
	Tag_t*		tp;
	int		argv[ARGS];
	struct stat	st;
	char		buf[MAXNAME];

	pushin(file, 1, ip, NiL, NiL);
	state.generation++;
	*(state.env_sp = state.env_stack) = state.env = env("0");
	state.it.dd = state.it.dl = state.it.dt = 0;
	state.link = 0;
	iop("stdout", 1)->sp = state.out = op;
	state.pass = 0;
	if (ip && !fstat(sffileno(ip), &st))
		tm(st.st_mtime);
	argc = 0;
	argv[0] = 0;
	cc = 0;
	lastc = 0;
	quote = 0;
	map = ccmap(CC_NATIVE, CC_ASCII);
	for (;;)
	{
		switch (c = GETC())
		{
		case EOF:
			if (popin())
				goto done;
			continue;
		case CODE_n:
			if (ISFILE())
			{
				cc++;
				code_2(OP_cc, c);
			}
			else
			{
				sfputc(state.out, c);
				n = GETC();
				sfputc(state.out, n);
				if (n & 0200)
				{
					n = (n & 0177) << 8;
					c = GETC();
					sfputc(state.out, c);
					n |= c;
				}
				do
				{
					c = GETC();
					sfputc(state.out, c);
				} while (n-- > 0);
			}
			continue;
		case CODE_2:
			if (ISFILE())
			{
				cc++;
				code_2(OP_cc, c);
			}
			else
			{
				m = GETC();
				n = GETC();
				if (cc <= 0 && (n == OP_cc || n >= OP_ft1 && n <= OP_ft5 || n == OP_ps))
					ONE();
				sfputc(state.out, c);
				sfputc(state.out, m);
				sfputc(state.out, n);
			}
			continue;
		case CODE_1:
			if (ISFILE())
			{
				cc++;
				code_2(OP_cc, c);
			}
			else
			{
				sfputc(state.out, c);
				switch (c = GETC())
				{
				case OP_br:
				case OP_p:
					cc = 0;
					break;
				}
				sfputc(state.out, c);
			}
			continue;
		case CODE_0:
			if (ISFILE())
			{
				cc++;
				code_2(OP_cc, c);
			}
			else
			{
				ONE();
				sfputc(state.out, c);
			}
			continue;
		case '"':
			if (argc && !(quote & RAW))
			{
				lastc = c;
				switch ((quote & STRING) ? (c = nextchar()) : EOF)
				{
				case '"':
					break;
				default:
					UNGETC(c);
					/*FALLTHROUGH*/
				case EOF:
					quote ^= STRING;
					continue;
				}
			}
			break;
		case '\n':
			if (state.noline)
			{
				if (isspace(lastc))
					continue;
				c = ' ';
				break;
			}
			if (quote & COMMENT)
			{
				quote &= ~COMMENT;
				if (quote & EAT)
				{
					quote &= ~EAT;
					c = ' ';
				}
				popout();
			}
			if (argc)
			{
				cc = 0;
				lastc = c;
				quote = 0;
				state.groff &= 1;
				state.pass = 0;
				if (!(s = popout()))
				{
					argc = 0;
					continue;
				}
				if (!s[argv[--argc]] && argc > 0)
					argc--;
				for (state.tag->argc = argc; argc >= 0; argc--)
					state.tag->argv[argc] = s + argv[argc];
				argc = 0;
				if (error_info.trace <= -4)
				{
					if (state.end)
						sfprintf(state.tmp, "    ");
					sfprintf(state.tmp, "%s", state.tag->argv[0]);
					if (!tp)
						sfprintf(state.tmp, " [UNKNOWN]");
					for (n = 1; n <= state.tag->argc; n++)
						sfprintf(state.tmp, " `%s'", state.tag->argv[n]);
					error(-4, "%s:%d: %s", error_info.file, error_info.line, use(state.tmp));
				}
				if (tp)
				{
					error_info.line++;
					if (tp->call)
						(*tp->call)(tp, state.tag);
					if (tp->flags & TAG_TRIGGER)
					{
						tp->flags &= ~TAG_TRIGGER;
						trigger(&state.trap);
					}
				}
				else if (state.verbose)
				{
					switch (state.tag->argv[0][1])
					{
					case 0:
						break;
					case '#':
						if (!state.tag->argv[0][2])
							break;
						/*FALLTHROUGH*/
					default:
						if (error_info.trace >= 0)
							error(1, "%s: unknown request", state.tag->argv[0]);
						sfprintf(state.tmp, "UNKNOWN REQUEST %s", join(state.tag, 0));
						code_n(OP_comment, use(state.tmp));
						break;
					}
					error_info.line++;
				}
				continue;
			}
			error_info.line++;
			n = 6 * state.env->vs.current;
			state.nl += n;
			state.env->dn += n;
			n = 6 * state.env->ps.current;
			if (n > state.env->dl)
				state.env->dl = n;
			if (!DIVERTED())
			{
				state.ln++;
				if (state.it.text || cc > 1)
					state.it.text++;
				else
				{
					cc = 0;
					continue;
				}
			}
			state.it.interrupt = 0;
			if (state.it.center > 0)
			{
				if (!--state.it.center)
					code_1(state.it.right ? END(OP_p) : END(OP_center));
				else
					code_1(OP_br);
			}
			if (state.it.count > 0 && !--state.it.count)
			{
				sfputc(state.out, '\n');
				pushin(NiL, 0, NiL, state.it.trap, NiL);
				cc = 0;
				continue;
			}
			cc = -2;
			break;
		case ' ':
		case '\t':
			if (argc)
			{
				if (!quote || (quote == RAW || quote == (COPY|RAW)) && argc == 1)
				{
					if (lastc != ' ' && argc < elementsof(argv))
					{
						sfputc(state.out, 0);
						argv[argc++] = sftell(state.out);
						lastc = ' ';
					}
					continue;
				}
			}
			break;
		default:
			if (c == state.ec)
			{
			escape:
				switch (c = GETC())
				{
				case EOF:
					if (popin())
					{
						c = '\n';
						break;
					}
					goto escape;
				case '\n':
					if (ISFILE())
						error_info.line++;
					else
						UNGETC(c);
					continue;
				case 'a':
					if (state.pass)
						goto passthrough;
					c = 1;
					break;
				case 'c':
					if (state.pass || (quote & RAW))
						goto passthrough;
					if ((c = nextchar()) == '\n')
					{
						message((-9, "INTERRUPT %s:%d: it.center=%d it.count=%d it.dt", error_info.file, error_info.line, state.it.center, state.it.count));
						state.it.interrupt = 1;
						error_info.line++;
						cc = 0;
						continue;
					}
					UNGETC(c);
					continue;
				case 'd':
					if (state.env->ss <= 0)
					{
						state.env->ss--;
						code_1(OP_sub);
					}
					else
					{
						state.env->ss++;
						code_1(END(OP_sup));
					}
					continue;
				case 'e':
				case 'E':
					if (state.pass)
						goto passthrough;
					ONE();
					code_2(OP_cc, state.eo);
					lastc = state.eo;
					continue;
				case 'f':
					if (state.pass || (quote & RAW))
						goto passthrough;
					ft(interpolate('f'));
					continue;
				case 'g':
				case 'j':
					if (state.pass || (quote & RAW))
						goto passthrough;
					pushin(NiL, 0, NiL, interpolate(c), NiL);
					continue;
				case 'k':
					nextchar();
					continue;
				case '*':
					c = '.';
					goto interp;
				case '[':
					if (!state.groff)
						goto passthrough;
				case '(':
					if (state.pass)
						goto passthrough;
					UNGETC(c);
					/*FALLTHROUGH*/
				case 'n':
				interp:
					if (quote & COPY)
						goto passthrough;
					if (s = interpolate(c))
						pushin(NiL, -1, NiL, s, NiL);
					continue;
				case 'p':
				case 'r':
				case 'z':
					continue;
				case 's':
					if (state.pass || (quote & RAW))
						goto passthrough;
					switch (c = nextchar())
					{
					case '-':
					case '+':
						m = c;
						c = nextchar();
						break;
					default:
						m = 0;
						break;
					}
					switch (c)
					{
					case '(':
						n = 1;
						sfputc(state.tmp, c);
						break;
					case '\'':
						n = c;
						goto size_long;
					case '[':
						n = ']';
					size_long:
						c = nextchar();
						if (!m)
							switch (c)
							{
							case '-':
							case '+':
								m = c;
								c = nextchar();
								break;
							}
						while (c != EOF && c != n)
						{
							sfputc(state.tmp, c);
							c = nextchar();
						}
						goto size_eval;
					default:
						n = 0;
						break;
					}
					if (c == '0' && m)
					{
						if ((n = nextchar()) == '\'')
						{
							while ((c = nextchar()) != EOF && c != '\'')
								sfputc(state.tmp, c);
							code_n(m == '+' ? OP_link : LABEL(OP_link), use(state.tmp));
							continue;
						}
						UNGETC(n);
						n = 0;
					}
					if (c != EOF)
					{
						if (n)
						{
							for (;; c = nextchar())
							{
								switch (c)
								{
								case EOF:
									break;
								case '(':
									sfputc(state.tmp, c);
									n++;
									continue;
								case ')':
									sfputc(state.tmp, c);
									if (--n <= 0)
										break;
									continue;
								default:
									sfputc(state.tmp, c);
									continue;
								}
								break;
							}
						size_eval:
							n = expression(use(state.tmp), NiL, 'p');
						}
						else
							n = isdigit(c) ? (c - '0') : 0;
						if (!n)
							n = state.env->ps.previous;
						else switch (m)
						{
						case '-':
							n = state.env->ps.current - n;
							break;
						case '+':
							n = state.env->ps.current + n;
							break;
						}
						if (n > 0)
							ps(n);
					}
					continue;
				case 't':
					if (state.pass)
						goto passthrough;
					c = '\t';
					break;
				case 'u':
					if (state.env->ss >= 0)
					{
						state.env->ss++;
						code_1(OP_sup);
					}
					else
					{
						state.env->ss--;
						code_1(END(OP_sub));
					}
					continue;
				case 'v':
				case 'w':
					if (state.pass || (quote & RAW))
						goto passthrough;
					/*FALLTHROUGH*/
				case 'b':
				case 'h':
				case 'l':
				case 'L':
				case 'o':
				case 'x':
					if ((n = nextchar()) != EOF)
					{
						while ((m = nextchar()) != n)
							sfputc(state.arg, m);
						s = use(state.arg);
						switch (c)
						{
						case 'h':
							if (*s++ == '0')
							{
								/*UNDENT...*/
	if ((c = *s++) == '*' || c == '/')
	{
		state.link = c == '*' ? OP_link : LABEL(OP_link);
		if (*s++ == '\\' && *s++ == 'w' && *s++ == '"')
		{
			if ((c = strlen(s)) > 0 && s[--c] == '"')
				s[c] = 0;
			sfputr(state.ref, s, '\t');
		}
		pushout(state.ref);
	}
	else if (!c && state.link)
	{
		code_n(state.link, popout());
		state.link = 0;
	}
								/*...INDENT*/
							}
							/* yep, this is a grade A hack, even for this file */
							if ((c = nextchar()) == state.ec)
							{
								if ((n = nextchar()) == 'c')
									break;
								UNGETC(n);
							}
							UNGETC(c);
							break;
						case 'v':
							c = expression(s, NiL, 0) >= 0 ? 'd' : 'u';
							sfprintf(state.arg, "%c%c", state.ec, c);
							pushin(NiL, 0, NiL, use(state.arg), NiL);
							break;
						case 'w':
							n = convert(strlen(s), 1, 'n');
							sfprintf(state.arg, "%d", n);
							pushin(NiL, 0, NiL, use(state.arg), NiL);
							break;
						}
					}
					continue;
				case '$':
					if (state.mac)
					{
						c = nextchar();
						if (c == '(')
						{
							if ((c = nextchar()) != EOF)
							{
								sfputc(state.tmp, c);
								if ((c = nextchar()) != EOF)
									sfputc(state.tmp, c);
							}
							goto arg_eval;
						}
						else if (c == '[')
						{
							while ((c = nextchar()) != EOF && c != ']')
								sfputc(state.tmp, c);
						arg_eval:
							c = expression(use(state.tmp), NiL, 0);
							if (c >= 0 && c <= state.mac->argc)
								pushin(NiL, -argc, NiL, state.mac->argv[c], NiL);
						}
						else if (c == '@')
						{
							for (c = 1; c < state.mac->argc; c++)
								sfprintf(state.tmp, "\"%s\" ", state.mac->argv[c]);
							if (c == state.mac->argc)
								sfprintf(state.tmp, "\"%s\"", state.mac->argv[c]);
							pushin(NiL, -argc, NiL, use(state.tmp), NiL);
						}
						else if (c < '0' || c > '9')
						{
							for (c = 1; c < state.mac->argc; c++)
								sfputr(state.tmp, state.mac->argv[c], ' ');
							if (c == state.mac->argc)
								sfputr(state.tmp, state.mac->argv[c], -1);
							pushin(NiL, -argc, NiL, use(state.tmp), NiL);
						}
						else if ((c -= '0') <= state.mac->argc)
							pushin(NiL, -argc, NiL, state.mac->argv[c], NiL);
					}
					continue;
				case '{':
					for (;;)
					{
						if ((n = GETC()) == EOF)
						{
							if (popin())
								break;
							continue;
						}
						if (n == state.ec)
						{
						escape_splice:
							switch (m = GETC())
							{
							case EOF:
								if (popin())
									break;
								goto escape_splice;
							case '\n':
								UNGETC(m);
								break;
							default:
								UNGETC(m);
								UNGETC(n);
								break;
							}
							break;
						}
						else
							UNGETC(n);
						break;
					}
					if (state.pass)
						goto passthrough;
					continue;
				case '}':
					if (state.end || !(state.cond.flags[state.cond.level] & COND_BLOCK))
						goto passthrough;
					if (state.cond.flags[state.cond.level] & (COND_EL|COND_IF))
						state.cond.level--;
					else
						state.cond.flags[state.cond.level] &= ~COND_BLOCK;
					continue;
				case '0':
				case '|':
				case '^':
				case ' ':
				case '/':
				case ',':
				case '~':
					if (state.pass)
						goto passthrough;
					ONE();
					code_2(OP_cc, ' ');
					continue;
				case '&':
					if (state.pass || (quote & RAW))
						goto passthrough;
					if (!cc)
						cc = -1;
					code_0();
					continue;
				case '-':
				case '%':
					if (state.pass)
						goto passthrough;
					ONE();
					code_2(OP_cc, 45);
					continue;
				case '"':
					if (!quote)
					{
						quote |= COMMENT;
						pushout(state.nul);
					}
					cc++;
					continue;
				case '#':
					if (!quote)
					{
						quote |= COMMENT|EAT;
						pushout(state.nul);
					}
					cc++;
					continue;
				case '!':
					if (!cc)
					{
						cc++;
						continue;
					}
					break;
				default:
					if (c == state.ec)
						break;
					if ((c == state.env->cc || c == state.env->c2) && !cc)
						goto request;
					if (state.pass)
						goto passthrough;
					break;
				}
				break;
			passthrough:
				ONE();
				sfputc(state.out, state.ec);
				break;
			}
			else if ((c == state.env->cc || c == state.env->c2) && !cc)
			{
			request:
				n = c;
				s = buf;
				*s++ = '.';
				if ((c = nextchar()) != EOF)
				{
					while (c == ' ' || c == '\t')
						c = nextchar();
					if (c == state.ec || c == '\n')
						UNGETC(c);
					else if (c != EOF)
					{
						*s++ = c;
						while (s < &buf[sizeof(buf)-1] && (c = nextchar()) != EOF)
						{
							if (c == state.ec || isspace(c))
							{
								UNGETC(c);
								break;
							}
							*s++ = c;
							if (!state.groff)
							{
								if ((c = nextchar()) != EOF)
								{
									UNGETC(c);
									if (!isspace(c))
										pushin(NiL, 0, NiL, " ", NiL);
								}
								break;
							}
						}
					}
				}
				*s = 0;
				tp = (Tag_t*)hashget(state.symbols, buf);
				if (tp && (tp->flags & TAG_DO))
				{
					state.groff |= 2;
					c = n;
					goto request;
				}
				if (state.end)
				{
					if (tp == state.end)
					{
						state.end = 0;
						state.pass = 0;
						s = popout();
						if (tp = state.define)
						{
							state.define = 0;
							set(tp, s, 0);
						}
						quote |= COMMENT;
						pushout(state.nul);
					}
					else
					{
						pushin(NiL, 0, NiL, buf + 1, NiL);
						c = n;
					}
					break;
				}
				if (tp)
				{
					if ((tp->flags & TAG_BREAK) && n == state.env->cc)
					{
						if (!DIVERTED() && (state.it.text || text()))
							state.it.text++;
						if (state.it.interrupt)
						{
							message((-9, "BREAK %s:%d: it.center=%d it.count=%d", error_info.file, error_info.line, state.it.center, state.it.count));
							state.it.interrupt = 0;
							sfputc(state.out, '\n');
							ONE();
						}
						tp->flags |= TAG_TRIGGER;
					}
					if (tp->flags & TAG_COPY)
						quote |= COPY|RAW;
					else if (tp->flags & TAG_RAW)
						quote |= RAW;
					if (tp->flags & TAG_PASS)
						state.pass = 1;
				}
				else
					state.pass = 1;
				argc = 1;
				pushout(state.tag->sp);
				sfputr(state.out, buf, -1);
				cc = s - buf;
				continue;
			}
			else if ((n = ccmapchr(map, c)) > 0177)
			{
				ONE();
				code_2(OP_cc, n & 0377);
				continue;
			}
			break;
		}
		ONE();
		sfputc(state.out, c);
		lastc = c;
	}
 done:
	if (state.end)
	{
		if (state.define)
		{
			error(2, "%s macro definition end tag %s not found", state.define->name, state.end->name);
			state.define = 0;
		}
		else
			error(2, "group end tag %s not found", state.end->name);
		state.end = 0;
	}
	while (DIVERTED())
		popout();
}

static Tag_t tags[] =
{
	".",		0,		0,			0,0,0,0,
	".'",		0,		0,			0,0,0,0,
	".''",		0,		0,			0,0,0,0,
	"..",		0,		0,			0,0,0,0,
	".EN",		0,		0,			0,0,0,0,
	".EQ",		troff_ignore,	0,			0,0,0,0,
	".TE",		0,		0,			0,0,0,0,
	".TS",		troff_ignore,	0,			0,0,0,0,
	".\"",		0,		0,			0,0,0,0,
	".\\\"",0,			0,			0,0,0,0,
	".ab",		troff_ab,	0,			0,0,0,0,
	".ad",		troff_ad,	TAG_PASS,		0,0,0,0,
	".af",		troff_af,	0,			0,0,0,0,
	".al",		0,		0,			0,0,0,0,
	".aln",		groff_aln,	0,			0,0,0,0,
	".als",		groff_als,	0,			0,0,0,0,
	".am",		troff_de,	TAG_PASS,		0,0,0,0,
	".as",		troff_ds,	TAG_PASS,		0,0,0,0,
	".asciify",	groff_asciify,	0,			0,0,0,0,
	".backtrace",	0,		0,			0,0,0,0,
	".bd",		0,		0,			0,0,0,0,
	".blm",		0,		0,			0,0,0,0,
	".bp",		0,		TAG_BREAK,		0,0,0,0,
	".br",		troff_br,	TAG_BREAK,		0,0,0,0,
	".break",	groff_break,	0,			0,0,0,0,
	".c2",		troff_c2,	0,			0,0,0,0,
	".cc",		troff_cc,	0,			0,0,0,0,
	".ce",		troff_ce,	TAG_BREAK,		0,0,0,0,
	".cf",		troff_cf,	0,			0,0,0,0,
	".cflags",	0,		0,			0,0,0,0,
	".ch",		troff_ch,	0,			0,0,0,0,
	".char",	0,		0,			0,0,0,0,
	".chop",	groff_chop,	0,			0,0,0,0,
	".close",	groff_close,	0,			0,0,0,0,
	".continue",	groff_continue,	0,			0,0,0,0,
	".cp",		groff_cp,	0,			0,0,0,0,
	".cs",		0,		0,			0,0,0,0,
	".cu",		0,		0,			0,0,0,0,
	".da",		troff_di,	TAG_PASS,		0,0,0,0,
	".de",		troff_de,	TAG_PASS,		0,0,0,0,
	".di",		troff_di,	TAG_PASS,		0,0,0,0,
	".do",		0,		TAG_DO,			0,0,0,0,
	".ds",		troff_ds,	TAG_PASS,		0,0,0,0,
	".dt",		troff_wh,	TAG_PASS,		0,0,0,0,
	".ec",		troff_ec,	0,			0,0,0,0,
	".eo",		troff_eo,	0,			0,0,0,0,
	".el",		troff_ie,	TAG_PASS|TAG_RAW,	0,0,0,0,
	".em",		troff_em,	TAG_PASS,		0,0,0,0,
	".ev",		troff_ev,	TAG_PASS,		0,0,0,0,
	".fam",		0,		0,			0,0,0,0,
	".fc",		0,		0,			0,0,0,0,
	".fi",		troff_fi,	TAG_BREAK,		0,0,0,0,
	".fl",		0,		TAG_BREAK,		0,0,0,0,
	".fp",		troff_fp,	0,			0,0,0,0,
	".fspecial",	0,		0,			0,0,0,0,
	".ft",		troff_ft,	0,			0,0,0,0,
	".ftr",		0,		0,			0,0,0,0,
	".hcode",	0,		0,			0,0,0,0,
	".hla",		0,		0,			0,0,0,0,
	".hlm",		0,		0,			0,0,0,0,
	".hpf",		0,		0,			0,0,0,0,
	".hw",		0,		0,			0,0,0,0,
	".hy",		0,		0,			0,0,0,0,
	".hym",		0,		0,			0,0,0,0,
	".hys",		0,		0,			0,0,0,0,
	".ie",		troff_ie,	TAG_PASS|TAG_RAW,	0,0,0,0,
	".if",		troff_ie,	TAG_PASS|TAG_RAW,	0,0,0,0,
	".ig",		troff_ignore,	TAG_PASS,		0,0,0,0,
	".in",		troff_in,	TAG_BREAK,		0,0,0,0,
	".it",		troff_it,	0,			0,0,0,0,
	".kern",	0,		0,			0,0,0,0,
	".lc",		0,		0,			0,0,0,0,
	".lf",		0,		0,			0,0,0,0,
	".ll",		troff_ll,	0,			0,0,0,0,
	".ls",		0,		0,			0,0,0,0,
	".lt",		0,		0,			0,0,0,0,
	".mk",		0,		0,			0,0,0,0,
	".mso",		troff_so,	0,			0,0,0,0,
	".na",		0,		0,			0,0,0,0,
	".ne",		troff_ne,	0,			0,0,0,0,
	".nf",		troff_nf,	TAG_BREAK,		0,0,0,0,
	".nh",		0,		0,			0,0,0,0,
	".nm",		0,		0,			0,0,0,0,
	".nn",		0,		0,			0,0,0,0,
	".nr",		troff_nr,	0,			0,0,0,0,
	".nroff",	0,		0,			0,0,0,0,
	".ns",		0,		0,			0,0,0,0,
	".open",	groff_open,	0,			0,0,0,0,
	".opena",	groff_open,	0,			0,0,0,0,
	".os",		0,		0,			0,0,0,0,
	".pc",		troff_pc,	0,			0,0,0,0,
	".pl",		0,		0,			0,0,0,0,
	".pm",		0,		0,			0,0,0,0,
	".pn",		0,		0,			0,0,0,0,
	".pnr",		0,		0,			0,0,0,0,
	".po",		0,		0,			0,0,0,0,
	".ps",		troff_ps,	0,			0,0,0,0,
	".pso",		groff_pso,	0,			0,0,0,0,
	".ptr",		0,		0,			0,0,0,0,
	".rchar",	0,		0,			0,0,0,0,
	".rj",		troff_ce,	0,			0,0,0,0,
	".rm",		troff_rm,	0,			0,0,0,0,
	".rn",		troff_rn,	0,			0,0,0,0,
	".rnn",		groff_rnn,	0,			0,0,0,0,
	".rr",		troff_rr,	0,			0,0,0,0,
	".rs",		0,		0,			0,0,0,0,
	".rt",		0,		0,			0,0,0,0,
	".shc",		0,		0,			0,0,0,0,
	".shift",	groff_shift,	0,			0,0,0,0,
	".so",		troff_so,	0,			0,0,0,0,
	".sp",		troff_sp,	TAG_BREAK,		0,0,0,0,
	".special",	0,		0,			0,0,0,0,
	".ss",		0,		0,			0,0,0,0,
	".sty",		0,		0,			0,0,0,0,
	".sv",		0,		0,			0,0,0,0,
	".sy",		groff_sy,	0,			0,0,0,0,
	".ta",		troff_ta,	TAG_BREAK,		0,0,0,0,
	".tc",		0,		0,			0,0,0,0,
	".ti",		troff_ti,	TAG_BREAK,		0,0,0,0,
	".tkf",		0,		0,			0,0,0,0,
	".tl",		troff_tl,	0,			0,0,0,0,
	".tm",		troff_tm,	0,			0,0,0,0,
	".tr",		0,		0,			0,0,0,0,
	".trf",		0,		0,			0,0,0,0,
	".trnt",	0,		0,			0,0,0,0,
	".troff",	0,		0,			0,0,0,0,
	".uf",		0,		0,			0,0,0,0,
	".ul",		0,		0,			0,0,0,0,
	".vpt",		0,		0,			0,0,0,0,
	".vs",		troff_vs,	0,			0,0,0,0,
	".warn",	0,		0,			0,0,0,0,
	".wh",		troff_wh,	TAG_PASS,		0,0,0,0,
	".while",	groff_while,	TAG_PASS|TAG_COPY,	0,0,0,0,
	".write",	groff_write,	0,			0,0,0,0,
	".xx",		troff_xx,	TAG_RAW,		0,0,0,0,
};

static Var_t vars[] =
{
	"(**",	"*",
	"(+-",	"\261",
	"(ap",	"~",
	"(bu",	"\267",
	"(bv",	"|",
	"(co",	"\251",
	"(dg",	"\247",
	"(fm",	"'",
	"(lq",	"``",
	"(rg",	"\256",
	"(rq",	"''",
	"(sq",	"\244",
	"eEQ",	".EN",
	"eTS",	".TE",
	"eig",	"..",
	"f",	"",
	"f1",	"1",
	"f2",	"2",
	"f3",	"3",
	"f5",	"5",
	"fB",	"3",
	"fCW",	"5",
	"fF",	"5",
	"fI",	"2",
	"fL",	"5",
	"fM",	"5",
	"fR",	"1",
	"fX",	"5",
};

static Dir_t dot =
{
	0,	".",
};

/*
 * initialize the global data
 */

static void
init(void)
{
	register int	i;

	state.groff |= 2;
	state.tag = &state.top;
	if (!(state.top.sp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [top buffer]");
	if (!(state.arg = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [arg buffer]");
	if (!(state.nul = sfopen(NiL, "/dev/null", "w")))
		error(ERROR_SYSTEM|3, "out of space [nul buffer]");
	if (!(state.ref = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [ref buffer]");
	if (!(state.req = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [req buffer]");
	if (!(state.tmp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [tmp buffer]");
	if (!(state.symbols = hashalloc(NiL, HASH_name, "symbols", 0)))
		error(ERROR_SYSTEM|3, "out of space [symbol hash]");
	for (i = 0; i < elementsof(vars); i++)
		if (!hashput(state.symbols, vars[i].name, vars[i].value))
			error(ERROR_SYSTEM|3, "out of space [var hash put]");
	for (i = 0; i < elementsof(tags); i++)
	{
		tags[i].flags |= TAG_STATIC;
		if (!hashput(state.symbols, tags[i].name, &tags[i]))
			error(ERROR_SYSTEM|3, "out of space [tag hash put]");
	}
	hashset(state.symbols, HASH_ALLOCATE);
	nr("%", 1, 0, 1);
	nr(".$", 0, 0, 1);
	nr(".A", 0, 0, 1);
	nr(".C", 0, 0, 1);
	nr(".F", 0, 0, 1);
	nr(".H", 0, 0, 1);
	nr(".L", 0, 0, 1);
	nr(".P", 0, 0, 1);
	nr(".T", 0, 0, 1);
	nr(".R", 0, 0, 1);
	nr(".V", 0, 0, 1);
	nr(".a", 0, 0, 1);
	nr(".b", 0, 0, 1);
	nr(".c", 0, 0, 1);
	nr(".ce", 0, 0, 1);
	nr(".d", 0, 0, 1);
	nr(".ev", 0, 0, 1);
	nr(".f", 0, 0, 1);
	nr(".g", 0, 0, 1);
	nr(".i", 0, 0, 1);
	nr(".in", 0, 0, 1);
	nr(".j", 0, 0, 1);
	nr(".k", 0, 0, 1);
	nr(".l", 0, 0, 1);
	nr(".ll", 0, 0, 1);
	nr(".n", 0, 0, 1);
	nr(".pn", 0, 0, 1);
	nr(".ps", 0, 0, 1);
	nr(".psr", 0, 0, 1);
	nr(".s", 0, 0, 1);
	nr(".sr", 0, 0, 1);
	nr(".t", 0, 0, 1);
	nr(".u", 0, 0, 1);
	nr(".v", 0, 0, 1);
	nr(".vpt", 0, 0, 1);
	nr(".w", 0, 0, 1);
	nr(".warn", 0, 0, 1);
	nr(".x", 0, 0, 1);
	nr(".y", 0, 0, 1);
	nr(".z", 0, 0, 1);
	nr("c.", 0, 0, 1);
	nr("dl", 0, 0, 1);
	nr("dn", 0, 0, 1);
	nr("ln", 0, 0, 1);
	nr("nl", 0, 0, 1);
	nr("systat", 0, 0, 0);
	tm(NiL);
	hot("see", 1);
	hot("refer", 1);
	state.ec = state.eo = DEFAULT_ec;
	state.in = (unsigned char*)"";
	state.in_top = state.in_stack;
	state.out_top = state.out_stack;
	state.tag_top = state.tag_stack;
	state.pc = DEFAULT_pc;
	state.list = state.list_stack;
	state.ta[0] = 8;
	state.ta[1] = 0;
	state.t = 1;
	iop("stderr", 1)->sp = sfstderr;
	state.groff &= 1;
}

/*
 * convert intermediate code in s to html on op
 */

static const char*	opt_align[] =
{
	"LEFT", "CENTER", "RIGHT",
};

static const char*	opt_attribute[] =
{
	"BACKGROUND", "HREF", "HREF", "ID", "NAME", "SIZE", "SRC",
};

static const char*	tag_name[] =
{
	0,		/* unknown	*/
	"A",		/* OP_a		*/
	"BODY",		/* OP_body	*/
	"BR",		/* OP_br	*/
	0,		/* OP_cc	*/
	"CENTER",	/* OP_center	*/
	0,		/* OP_comment	*/
	"DD",		/* OP_dd	*/
	"DIV",		/* OP_div	*/
	"DL",		/* OP_dl	*/
	"DT",		/* OP_dt	*/
	"FN",		/* OP_fn	*/
	0,		/* OP_ft1	*/
	"EM",		/* OP_ft2	*/
	"STRONG",	/* OP_ft3	*/
	0,		/* OP_ft4	*/
	"TT",		/* OP_ft5	*/
	"H2",		/* OP_h2	*/
	"H3",		/* OP_h3	*/
	"H4",		/* OP_h4	*/
	"HEAD",		/* OP_head	*/
	"HR",		/* OP_hr	*/
	"HTML",		/* OP_html	*/
	0,		/* OP_link	*/
	"P",		/* OP_p		*/
	"PRE",		/* OP_pre	*/
	"FONT",		/* OP_ps	*/
	"SUB",		/* OP_sub	*/
	"SUP",		/* OP_sup	*/
	"TABLE",	/* OP_tab	*/
	"TD",		/* OP_tab_data	*/
	"TH",		/* OP_tab_head	*/
	"TR",		/* OP_tab_row	*/
	"TITLE",	/* OP_title	*/
};

/*
 * emit tag and optionally check stack
 */

static void
tag(Sfio_t* op, int index, register int flags, int att, char* att_str, int att_num)
{
	register int		n;
	register int		m;
	register unsigned char*	sp;

	if (index & OP_END)
	{
		index |= OP_LABEL;
		sp = state.tag_top;
		m = 1;
		for (;;)
		{
			if (sp <= state.tag_stack)
			{
				error(2, "tag stack underflow trying to match <%s>", tag_name[OP(index)]);
				sfprintf(sfstderr, "stack contents:\n");
				sp = state.tag_top;
				while (--sp >= state.tag_stack)
					sfprintf(sfstderr, "\t<%s%s%s>\n", (*sp & OP_END) ? "/" : "", tag_name[OP(*sp)], (*sp & OP_LABEL) ? " label=1" : "");
				return;
			}
			n = *--sp;
			if (!(n & OP_END))
			{
				if (n == OP_pre && OP(index) != OP_pre)
				{
					m = 0;
					break;
				}
				n |= OP_END|OP_LABEL;
				if (tag_name[OP(n)])
				{
					flags |= LIST;
					sfprintf(op, "</%s>", tag_name[OP(n)]);
				}
				*sp = n;
			}
			if (n == index)
			{
				*sp &= ~OP_LABEL;
				break;
			}
			m = 0;
		}
		if (m)
		{
			if ((flags & (LINE|LIST)) == (LINE|LIST))
				sfputc(op, '\n');
			state.tag_top = sp;
		}
	}
	else
	{
		if (flags & STACK)
		{
			if (state.tag_top >= &state.tag_stack[elementsof(state.tag_stack)])
				error(3, "tag stack overflow");
			*state.tag_top++ = tag_name[index] ? index : END(index);
		}
		if (tag_name[index])
		{
			sfprintf(op, "<%s", tag_name[index]);
			if (att && ((att & ATT_NUMBER) || att_str))
			{
				sfprintf(op, " %s=", opt_attribute[ATT_INDEX(att)]);
				if (att & ATT_NUMBER)
				{
					if (att == ATT_size)
					{
						if (att_num < 0)
						{
							att_num = -att_num;
							sfputc(op, '-');
						}
						else
							sfputc(op, '+');
						att_num = (att_num + 5) / 6;
					}
					sfprintf(op, "%d", att_num);
				}
				else
					sfprintf(op, "\"%s%s\"", att == ATT_lref ? "#" : "", att_str);
			}
			if (ARG_ATTR(flags))
			{
				if ((n = ARG_ALIGN(flags)) >= 0)
					sfprintf(op, " align=%s", opt_align[n]);
				if (flags & ARG_compact)
					sfputr(op, " COMPACT", -1);
				if (flags & ARG_wide)
					sfputr(op, " width=100%", -1);
			}
			if (index == OP_body)
			{
				if (state.background)
					sfprintf(op, " background=\"%s\"", state.background);
				if (state.logo)
					sfprintf(op, ">\n<CENTER><IMG src=\"%s\"></CENTER", state.logo);
			}
			sfputc(op, '>');
			if (flags & LINE)
				sfputc(op, '\n');
		}
	}
}

/*
 * if OP_dl follows OP_ft,OP_ps then do it now
 */

static void
peeklist(Sfio_t* op, register char* s)
{
	for (;;)
	{
		switch (*s++)
		{
		case 0:
			break;
		case CODE_0:
			continue;
		case CODE_2:
			s++;
			/*FALLTHROUGH*/
		case CODE_1:
			switch (*s++)
			{
			case OP_ft1:
			case OP_ft2:
			case OP_ft3:
			case OP_ft4:
			case OP_ft5:
			case OP_ps:
				continue;
			case OP_dl:
				*--s = CODE_0;
				*--s = CODE_0;
				tag(op, OP_dl, STACK|ARG_compact, 0, NiL, 0);
				continue;
			}
			break;
		case ' ':
		case '\t':
		case '\n':
			continue;
		default:
			break;
		}
		break;
	}
}

#define P() \
	col = br = p = 0

#define DATA() \
	do \
	{ \
		if (li) \
		{ \
			if (li == 1) \
				li = 0; \
			P(); \
		} \
		else if (p) \
		{ \
			P(); \
			sfputr(op, "<P>", '\n'); \
		} \
		else if (br) \
		{ \
			P(); \
			sfputr(op, "<BR>", '\n'); \
		} \
	} while (0)

static void
html(register unsigned char* s, Sfio_t* op)
{
	register int		c;
	register int		br = 0;
	register int		col = 0;
	register int		li = 0;
	register int		p = 0;
	register int		nf = 0;
	register unsigned char*	v;
	unsigned char*		t;
	unsigned char*		u;
	int			n;
	int			m;
	int			ta;
	int			ts;
	int			ft = 1;
	int			hot = 0;
	int			label = 0;
	int			ps = DEFAULT_ps;
	int			ps_set = 0;
	Dir_t*			x;
	Sfio_t*			subject;
	char			a[2];

	sfputr(op, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">", '\n');
	tag(op, OP_html, STACK|LINE, 0, NiL, 0);
	tag(op, OP_head, STACK|LINE, 0, NiL, 0);
	t = (unsigned char*)strchr(usage, '\n') + 5;
	sfprintf(op, "<META name=\"generator\" content=\"%-.*s", strchr((char*)t, '\n') - (char*)t, t);
	for (x = state.macros; x; x = x->next)
		sfprintf(op, " -m%s", x->name);
	sfputr(op, "\">\n", -1);
	tag(op, OP_title, STACK, 0, NiL, 0);
	if (!(subject = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [subject]");
	if (state.package)
		sfputr(subject, state.package, ' ');
	if (state.title)
		sfputr(subject, state.title, -1);
	else
	{
		if (state.input)
		{
			if (t = (unsigned char*)strrchr(state.input, '/'))
				t++;
			else
				t = (unsigned char*)state.input;
			sfputr(subject, (char*)t, -1);
		}
		if (state.macros)
			sfprintf(subject, " m%s document", state.macros->name);
	}
	sfputr(op, use(subject), -1);
	tag(op, END(OP_title), STACK|LINE, 0, NiL, 0);
	if (state.author)
		sfprintf(op, "<AUTHOR>%s</AUTHOR>\n", state.author);
	if (!state.head)
	{
		tag(op, END(OP_head), STACK|LINE, 0, NiL, 0);
		tag(op, OP_body, STACK|LINE, 0, NiL, 0);
	}
	for (;;)
	{
		switch (*s++)
		{
		case 0:
			break;
		case '&':
			DATA();
			col++;
			sfputr(op, "&amp;", -1);
			continue;
		case '<':
			DATA();
			col++;
			sfputr(op, "&lt;", -1);
			continue;
		case '>':
			DATA();
			col++;
			sfputr(op, "&gt;", -1);
			continue;
		case CODE_0:
			continue;
		case CODE_2:
			c = *s++;
			/*FALLTHROUGH*/
		case CODE_1:
			if (!nf)
			{
				n = *s ^ OP_END;
				for (v = s + 1;;)
				{
					switch (*v++)
					{
					case '\n':
					case ' ':
					case '\t':
						continue;
					case CODE_0:
						continue;
					case CODE_1:
						if ((m = *v++) == n)
						{
							n = 0;
							s = v;
							break;
						}
						else if (m != OP_br && m != OP_p)
							break;
						continue;
					case CODE_2:
						if (*v++ != ' ' || *v++ != OP_cc)
							break;
						continue;
					default:
						break;
					}
					break;
				}
				if (!n)
					continue;
			}
			switch (m = *s++)
			{
			case END(OP_a):
				tag(op, m, STACK, 0, NiL, 0);
				*--s = a[1];
				*--s = a[0];
				break;
			case OP_body:
				tag(op, m, STACK|LINE, 0, NiL, 0);
				tag(op, OP_hr, LINE, 0, NiL, 0);
				col = 0;
				goto compact;
			case OP_br:
				while (*s == ' ')
					s++;
				if (!li)
				{
					if (nf)
						goto compact;
					else
						br++;
				}
				break;
			case OP_cc:
				DATA();
				sfputc(op, '&');
				switch (c)
				{
				case '&':
					sfputr(op, "amp", -1);
					break;
				case '<':
					sfputr(op, "lt", -1);
					break;
				case '>':
					sfputr(op, "gt", -1);
					break;
				case ' ':
					sfputr(op, "nbsp", -1);
					break;
				default:
					sfprintf(op, "#%03d", c);
					break;
				}
				sfputc(op, ';');
				col++;
				break;
			case OP_center:
				if (col)
				{
					col = 0;
					sfputc(op, '\n');
				}
				tag(op, m, STACK|c, 0, NiL, 0);
				if (!state.head)
				{
					tag(op, OP_h2, STACK, 0, NiL, 0);
					for (;;)
					{
						switch (n = *s++)
						{
						case 0:
						case '\n':
						case CODE_1:
						case CODE_2:
						case CODE_n:
							s--;
							break;
						default:
							sfputc(op, n);
							continue;
						}
						break;
					}
					tag(op, END(OP_h2), STACK, 0, NiL, 0);
					tag(op, OP_h4, STACK, 0, NiL, 0);
				}
				break;
			case END(OP_center):
				col = 0;
				if (!state.head)
				{
					state.head = 1;
					tag(op, END(OP_h4), STACK, 0, NiL, 0);
				}
				tag(op, m, STACK, 0, NiL, 0);
				break;
			case LABEL(OP_dd):
				tag(op, END(OP_a), STACK, 0, NiL, 0);
				tag(op, END(OP_h3), STACK, 0, NiL, 0);
				/*FALLTHROUGH*/
			case OP_dd:
				li = 1;
				if (col)
				{
					col = 0;
					sfputc(op, '\n');
				}
				tag(op, OP_dd, 0, 0, NiL, 0);
				break;
			case OP_div:
				if (col)
				{
					col = 0;
					sfputc(op, '\n');
				}
				tag(op, m, STACK|c, 0, NiL, 0);
				break;
			case END(OP_div):
				col = 0;
				tag(op, m, STACK, 0, NiL, 0);
				break;
			case OP_dl:
				li = 0;
				if (col)
				{
					col = 0;
					sfputc(op, '\n');
				}
				tag(op, m, STACK|LINE|ARG_compact, 0, NiL, 0);
				break;
			case END(OP_dl):
				v = s;
				for (;;)
				{
					switch (*v++)
					{
					case CODE_0:
						continue;
					case CODE_1:
						switch (*v++)
						{
						case OP_dl:
							*(v - 2) = *(v - 1) = CODE_0;
							v = 0;
							break;
						default:
							continue;
						}
						break;
					case CODE_2:
						v += 2;
						continue;
					default:
						break;
					}
					break;
				}
				if (v)
				{
					li = 0;
					col = 0;
					tag(op, m, STACK|LINE, 0, NiL, 0);
				}
				break;
			case LABEL(OP_dt):
				label = 1;
				n = 1;
				/*FALLTHROUGH*/
			case OP_dt:
				if (col)
				{
					col = 0;
					sfputc(op, '\n');
				}
				if (p)
				{
					P();
					tag(op, OP_p, LINE, 0, NiL, 0);
				}
				v = s;
				for (;;)
				{
					switch (*v++)
					{
					case CODE_0:
						continue;
					case CODE_2:
						v++;
						/*FALLTHROUGH*/
					case CODE_1:
						switch (*v++)
						{
						case OP_dl:
							*(v - 2) = *(v - 1) = CODE_0;
							tag(op, OP_dl, STACK|LINE|ARG_compact, 0, NiL, 0);
							if (!label)
								break;
							continue;
						case OP_dd:
							if (label)
								*(v - 1) = LABEL(OP_dd);
							break;
						default:
							continue;
						}
						break;
					case ' ':
					case '\t':
					case '\n':
						if (!label)
							break;
						if (!n)
						{
							n = 1;
							sfputc(state.tmp, ' ');
						}
						continue;
					case '"':
						if (!label)
							break;
						continue;
					default:
						if (!label)
							break;
						n = 0;
						sfputc(state.tmp, *(v - 1));
						continue;
					}
					break;
				}
				li = 2;
				tag(op, OP_dt, LINE, 0, NiL, 0);
				if (label)
				{
					label = 0;
					n = sfstrtell(state.tmp);
					v = (unsigned char*)use(state.tmp);
					while (--n > 0 && (isspace(v[n]) || v[n] == '.'));
					v[n + 1] = 0;
					if (isdigit(*v))
						tag(op, OP_hr, LINE, 0, NiL, 0);
					tag(op, OP_h3, STACK, 0, NiL, 0);
					tag(op, OP_a, STACK, ATT_name, (char*)v, 0);
				}
				break;
			case END(OP_fn):
				tag(op, m, STACK, 0, NiL, 0);
				break;
			case OP_ft1:
			case OP_ft2:
			case OP_ft3:
			case OP_ft4:
			case OP_ft5:
				if (hot)
				{
					int	ob = 0;
					int	cb = 0;
					int	p = 0;
					int	r = ATT_lref;
					int	z = 0;

					v = s;
					for (;;)
					{
						switch (n = *v++)
						{
						case 0:
							hot = 0;
							break;
						case CODE_0:
							continue;
						case CODE_2:
							v++;
							/*FALLTHROUGH*/
						case CODE_1:
							n = *v++;
							if (!p && n >= OP_ft1 && n <= OP_ft5)
								p = -1;
							continue;
						case '(':
						case '[':
						case '{':
						case '<':
							if (n == ob)
								p++;
							else if (p <= 0)
							{
								r = ATT_href;
								p = 1;
								switch (ob = n)
								{
								case '(':
									cb = ')';
									break;
								case '[':
									cb = ']';
									break;
								case '{':
									cb = '}';
									break;
								case '<':
									cb = '>';
									break;
								}
							}
							z = 0;
							sfputc(state.tmp, n);
							continue;
						case ')':
						case ']':
						case '}':
						case '>':
							if (p <= 0)
							{
								v--;
								break;
							}
							z = 0;
							sfputc(state.tmp, n);
							if (n == cb && !--p)
								break;
							continue;
						case ' ':
						case '\t':
						case '\n':
							if (!z)
							{
								z = 1;
								sfputc(state.tmp, ' ');
							}
							continue;
						case '"':
							continue;
						default:
							if (p < 0)
							{
								v--;
								break;
							}
							z = 0;
							sfputc(state.tmp, n);
							continue;
						}
						break;
					}
					n = sfstrtell(state.tmp);
					if (!*(t = (unsigned char*)use(state.tmp)))
						hot = 0;
					if (hot)
					{
						hot = 0;
						while (--n > 0 && (isspace(t[n]) || t[n] == '.'));
						t[n + 1] = 0;
						tag(op, OP_a, STACK, r, (char*)t, 0);
						a[0] = v[0];
						v[0] = CODE_1;
						a[1] = v[1];
						v[1] = END(OP_a);
					}
				}
				c = m - OP_ft;
				if (c != ft)
				{
					peeklist(op, (char*)s);
					if (ft != 1)
						tag(op, END(OP_ft + ft), STACK, 0, NiL, 0);
					ft = c;
					if (ft != 1)
						tag(op, OP_ft + ft, STACK, 0, NiL, 0);
				}
				break;
			case OP_h2:
			case OP_h3:
			case OP_h4:
			case END(OP_h2):
			case END(OP_h3):
			case END(OP_h4):
				tag(op, m, STACK, 0, NiL, 0);
				break;
			case OP_head:
			case END(OP_head):
				col = 0;
				tag(op, m, STACK|LINE, 0, NiL, 0);
				goto compact;
			case OP_hr:
				col = 0;
				tag(op, OP_hr, LINE, 0, NiL, 0);
				goto compact;
			case OP_p:
				while (*s == ' ')
					s++;
				if (c)
				{
					if (col)
					{
						col = 0;
						sfputc(op, '\n');
					}
					P();
					tag(op, m, STACK|c, 0, NiL, 0);
				}
				else if (!li)
				{
					if (nf)
						goto compact;
					else
						p++;
				}
				break;
			case END(OP_p):
				col = 0;
				tag(op, m, STACK, 0, NiL, 0);
				break;
			case OP_pre:
				if (!nf)
				{
					nf = 03;
					tag(op, m, STACK, 0, NiL, 0);
				}
				goto compact;
			case END(OP_pre):
				if (nf)
				{
					nf = 02;
					tag(op, m, STACK, 0, NiL, 0);
				}
				goto compact;
			case OP_ps:
				if (c != ps)
				{
					peeklist(op, (char*)s);
					if (ps_set)
					{
						ps = ps_set;
						ps_set = 0;
						tag(op, END(OP_ps), STACK, 0, NiL, 0);
					}
					if (n = c - ps)
					{
						ps_set = ps;
						ps = c;
						tag(op, OP_ps, STACK, ATT_size, NiL, n);
					}
				}
				break;
			case OP_sub:
			case END(OP_sub):
			case OP_sup:
			case END(OP_sup):
				tag(op, m, STACK, 0, NiL, 0);
				break;
			case OP_tab:
				if (col)
				{
					col = 0;
					sfputc(op, '\n');
				}
				tag(op, m, STACK|c, 0, NiL, 0);
				goto compact;
			case END(OP_tab):
				tag(op, m, STACK|LINE, 0, NiL, 0);
				break;
			case OP_tab_data:
			case OP_tab_head:
			case OP_tab_row:
				tag(op, m, c, 0, NiL, 0);
				break;
			default:
				if (!(v = (unsigned char*)tag_name[OP(m)]))
				{
					sfprintf(state.tmp, "(%d)", OP(m));
					v = (unsigned char*)use(state.tmp);
				}
				error(2, "internal error: <%s%s%s %d> ignored", (m & OP_END) ? "/" : "", v, (m & OP_LABEL) ? " label=" : "", c);
				break;
			compact:
				if (col)
					sfputc(op, '\n');
				P();
				v = s;
				for (;;)
				{
					switch (*s++)
					{
					case 0:
						break;
					case '\n':
						p++;
						col = 0;
						v = s;
						continue;
					case ' ':
					case '\t':
						if (!nf)
							break;
						col = 1;
						continue;
					case CODE_0:
						continue;
					case CODE_1:
						switch (*s)
						{
						case OP_pre:
							if (!nf)
								break;
							/*FALLTHROUGH*/
						case OP_br:
						case OP_p:
							s++;
							p++;
							col = 0;
							v = s;
							continue;
						case OP_body:
							p = 0;
							col = 0;
							break;
						}
						break;
					}
					if (col)
					{
						col = 0;
						s = v;
					}
					else
						s--;
					break;
				}
				if (nf > 1)
					nf &= 01;
				else if (nf && p)
					sfputc(op, '\n');
				p = 0;
				break;
			}
			c = 0;
			continue;
		case CODE_n:
			n = *s++;
			if (n & 0200)
			{
				n = (n & 0177) << 8;
				n |= *s++;
			}
			c = *s++;
			v = s;
			s += n;
			switch (c)
			{
			case OP_comment:
				if (col)
				{
					col = 0;
					sfputc(op, '\n');
				}
				sfprintf(op, "<!--\"%s\"-->\n", v);
				break;
			case OP_fn:
				tag(op, c, STACK, ATT_id, (char*)v, 0);
				break;
			case OP_link:
				DATA();
				if (u = (unsigned char*)strchr((char*)v, '\t'))
					*u++ = 0;
				else
					u = 0;
				t = v;
				while (isdigit(*v))
					v++;
				while (isalpha(*v))
					v++;
				if (*v == ':' || *v == '/' || *v == '.' || *(v + 1) == '/')
				{
					if (!u)
						u = v + 1;
					v = (unsigned char*)"";
				}
				else
				{
					if (!u)
						u = t;
					v = (unsigned char*)"#";
				}
				sfprintf(op, "<A href=\"%s%s\">%s</A>", v, t, u);
				break;
			case LABEL(OP_link):
				DATA();
				if (u = (unsigned char*)strchr((char*)v, '\t'))
					*u++ = 0;
				else
					u = v;
				sfprintf(op, "<A name=\"%s\">%s</A>", v, u);
				break;
			case OP_ta:
				strcpy((char*)state.ta, (char*)v);
				break;
			case OP_RAW:
				DATA();
				if (col)
				{
					col = 0;
					sfputc(op, '\n');
				}
				sfputr(op, (char*)v, '\n');
				break;
			}
			continue;
		case ' ':
			if (nf)
			{
				col++;
				sfputc(op, *(s - 1));
			}
			else
			{
				while (isspace(*s))
					s++;
				if (col >= 70)
				{
					col = 0;
					sfputc(op, '\n');
				}
				else if (col > 0)
				{
					col++;
					sfputc(op, ' ');
				}
			}
			continue;
		case '\t':
			if (nf)
			{
				ta = state.ta[ts = 0];
				while (col >= ta)
				{
					if (state.ta[ts+1])
						ts++;
					ta += state.ta[ts];
				}
				do
				{
					sfputc(op, ' ');
				} while (++col < ta);
			}
			else
			{
				col++;
				sfputc(op, '\t');
			}
			continue;
		case '\n':
			if (nf)
			{
				v = s;
				col = 0;
				for (;;)
				{
					switch (*v++)
					{
					case 0:
						break;
					case '\n':
						continue;
					case ' ':
					case '\t':
						continue;
					case CODE_0:
						continue;
					case CODE_1:
						switch (*v++)
						{
						case OP_br:
						case OP_p:
						case OP_pre:
							continue;
						case END(OP_pre):
							col = 1;
							s = v - 2;
							break;
						}
						break;
					}
					break;
				}
				if (col)
				{
					col = 0;
					continue;
				}
				sfputc(op, '\n');
			}
			else
			{
				while (isspace(*s))
					s++;
				if (col >= 70)
				{
					col = 0;
					sfputc(op, '\n');
				}
				else if (col > 0)
				{
					col++;
					sfputc(op, ' ');
				}
			}
			continue;
		case '(':
			if (hot)
				hot = 0;
			else
				for (x = state.hot; x; x = x->next)
				{
					v = s;
					u = (unsigned char*)x->name;
					do
					{
						if (!*u)
						{
							if (isspace(*v))
							{
								hot = 1;
								goto data;
							}
							break;
						}
					} while (*u++ == *v++);
				}
			goto data;
		case ')':
			hot = 0;
			goto data;
		case '.':
			if (!nf && isspace(*s))
			{
				while (isspace(*s))
					s++;
				col = 0;
				sfputc(op, '.');
				sfputc(op, '\n');
				continue;
			}
			/*FALLTHROUGH*/
		default:
		data:
			DATA();
			col++;
			sfputc(op, *(s - 1));
			continue;
		}
		break;
	}
	if (col)
		sfputc(op, '\n');
	if (state.mailto)
		sfprintf(op, "<P>Send comments and suggestions to <A href=\"mailto:%s?subject=%s\">%s</A>.\n", state.mailto, state.mailto, sfstrbase(subject));
	if (state.author || state.corporation || state.company || state.location)
	{
		t = (unsigned char*)"<P>";
		u = (unsigned char*)"<BR>";
		if (state.author)
		{
			sfprintf(op, "%s%s\n", t, state.author);
			t = u;
		}
		if (state.organization)
		{
			sfprintf(op, "%s%s\n", t, state.organization);
			t = u;
		}
		if (state.corporation || state.company)
		{
			sfputr(op, (char*)t, -1);
			t = u;
			if (state.corporation)
				sfputr(op, state.corporation, state.company ? ' ' : '\n');
			if (state.company)
				sfputr(op, state.company, '\n');
		}
		if (state.address)
		{
			sfprintf(op, "%s%s\n", t, state.address);
			t = u;
		}
		if (state.location)
		{
			sfprintf(op, "%s%s\n", t, state.location);
			t = u;
		}
		if (state.phone)
		{
			sfprintf(op, "%s%s\n", t, state.phone);
			t = u;
		}
	}
	sfstrclose(subject);
	if (!state.footer)
		sfprintf(op, "<P>%s\n", fmttime("%B %d, %Y", state.date));
	if (state.toolbar && (subject = find(state.toolbar, NiL, 1)))
	{
		sfmove(subject, sfstdout, SF_UNBOUND, -1);
		sfclose(subject);
	}
	tag(op, END(OP_body), STACK, 0, NiL, 0);
	tag(op, END(OP_html), STACK, 0, NiL, 0);
	sfputc(op, '\n');
}

int
main(int argc, char** argv)
{
	register int		n;
	register char*		s;
	char*			v;
	Dir_t*			x;
	Dir_t*			lastdir;
	Dir_t*			lastmac;
	Sfio_t*			ip;
	Sfio_t*			op;
	Sfio_t*			script;

	NoP(argc);
	error_info.id = "troff2html";
	state.dirs = lastdir = &dot;
	init();
	script = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 0:
			break;
		case 'i':
			if (!(op = sfopen(NiL, opt_info.arg, "r")))
				error(ERROR_SYSTEM|2, "%s: cannot read", opt_info.arg);
			else
			{
				if (!(s = sfreserve(op, SF_UNBOUND, 0)) || (n = sfvalue(op)) <= 0 || s[n - 1] != '\n')
					error(1, "%s: invalid info file", opt_info.arg);
				else
				{
					s[n] = 0;
					stropt(s, options, sizeof(*options), setopt, NiL);
				}
				sfclose(op);
			}
			continue;
		case 'm':
			if (!(x = newof(0, Dir_t, 1, 0)))
				error(ERROR_SYSTEM|3, "out of space [macros]");
			x->name = opt_info.arg;
			if (state.macros)
				lastmac = lastmac->next = x;
			else
				state.macros = lastmac = x;
			continue;
		case 'r':
			if (*(s = opt_info.arg))
			{
				opt_info.num = expression(s + 1, NiL, 0);
				s[1] = 0;
				nr(s, opt_info.num, 0, 0);
			}
			continue;
		case 's':
			if (!script && !(script = sfstropen()))
				error(ERROR_SYSTEM|3, "out of space [script]");
			sfputr(script, opt_info.arg, '\n');
			continue;
		case 'v':
			state.verbose = 1;
			continue;
		case 'I':
			if (streq(opt_info.arg, "-"))
				dot.name[0] = 0;
			else if (!(x = newof(0, Dir_t, 1, 0)))
				error(ERROR_SYSTEM|3, "out of space [dir]");
			else
			{
				x->name = opt_info.arg;
				lastdir = lastdir->next = x;
			}
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			if (opt_info.name[1] != '-')
			{
				error(2, "%s", opt_info.arg);
				continue;
			}
			if (!(v = strchr(argv[opt_info.index - 1], '=')))
				v = opt_info.name + 2;
			else if (!(v = sfprints("%s=%s", opt_info.name + 2, fmtquote(v + 1, "\"", "\"", strlen(v + 1), FMT_ALWAYS))))
				error(ERROR_SYSTEM|3, "out of space");
			stropt(v, options, sizeof(*options), setopt, options);
			continue;
		}
		break;
	}
	if (!dot.name[0])
		state.dirs = state.dirs->next;
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!(op = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [output]");
	if (script)
	{
		pushin("script", 1, NiL, use(script), NiL);
		process(NiL, NiL, op);
		sfstrclose(script);
	}
	for (x = state.macros; x; x = x->next)
		if (ip = find(x->name, &v, -1))
			process(v, ip, op);
	if (!(state.input = *argv))
		process(NiL, sfstdin, op);
	else
		while (s = *argv++)
			if (ip = find(s, &v, 1))
				process(v, ip, op);
	if (state.out)
	{
		if (state.it.center)
		{
			state.it.center = 0;
			code_1(END(OP_center));
		}
		while (state.list > state.list_stack)
		{
			if (state.list->dl)
				code_1(END(OP_dl));
			state.list--;
		}
		code_1(OP_hr);
		trigger(&state.fini);
		process(NiL, NiL, op);
		html((unsigned char*)use(op), sfstdout);
	}
	exit(error_info.errors != 0);
}
