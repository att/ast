/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1995-2012 AT&T Intellectual Property          *
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
*              Doug McIlroy <doug@research.bell-labs.com>              *
*                                                                      *
***********************************************************************/
#pragma prototyped

static const char usage[] =
"[-?\n@(#)$Id: grep (AT&T Research) 2012-05-11 $\n]"
USAGE_LICENSE
"[--plugin?ksh]"
"[+NAME?grep - search lines in files for matching patterns]"
"[+DESCRIPTION?The \bgrep\b commands search the named input files for "
    "lines containing a match for the given \apatterns\a. Matching lines are "
    "printed by default. The standard input is searched if no files are "
    "given or when the file \b-\b is specified.]"
"[+?There are six variants of \bgrep\b, each one using a different form "
    "of \apattern\a, controlled either by option or the command path base "
    "name. Details of each variant may be found in \bregex\b(3).]"
    "{"
        "[+grep?The default basic regular expressions (no "
            "alternations.)]"
        "[+egrep?Extended regular expressions (alternations, one or "
            "more.)]"
        "[+pgrep?\bperl\b(1) regular expressions (lenient extended.)]"
        "[+xgrep?Augmented regular expressions (conjunction, negation.)]"
        "[+fgrep?Fixed string expressions.]"
        "[+agrep?Approximate regular expressions (not implemented.)]"
    "}"
"[G:basic-regexp?\bgrep\b mode (default): basic regular expression "
    "\apatterns\a.]"
"[E:extended-regexp?\begrep\b mode: extended regular expression "
    "\apatterns\a.]"
"[X:augmented-regexp?\bxgrep\b mode: augmented regular expression "
    "\apatterns\a.]"
"[P:perl-regexp?\bpgrep\b mode: \bperl\b(1) regular expression "
    "\apatterns\a.]"
"[F:fixed-string?\bfgrep\b mode: fixed string \apatterns\a.]"
"[A:approximate-regexp?\bagrep\b mode: approximate regular expression "
    "\apatterns\a (not implemented.)]"
"[C:context?Set the matched line context \abefore\a and \aafter\a count. "
    "If ,\aafter\a is omitted then it is set to \abefore\a. By default only "
    "matched lines are printed.]:?[before[,after]]:=2,2]"
"[c:count?Only print a matching line count for each file.]"
"[e:expression|pattern|regexp?Specify a matching \apattern\a. More than "
    "one \apattern\a implies alternation. If this option is specified then "
    "the command line \apattern\a must be omitted.]: [pattern]"
"[f:file?Each line in \apattern-file\a is a \apattern\a, placed into a "
    "single alternating expression.]: [pattern-file]"
"[H:filename|with-filename?Prefix each matched line with the containing "
    "file name.]"
"[h:no-filename?Suppress containing file name prefix for each matched "
    "line.]"
"[i:ignore-case?Ignore case when matching.]"
"[l:files-with-matches?Only print file names with at least one match.]"
"[L:files-without-matches?Only print file names with no matches.]"
"[b:highlight?Highlight matches using the ansi terminal bold sequence.]"
"[v:invert-match|revert-match?Invert the \apattern\a match sense.]"
"[m:label?All patterns must be of the form \alabel\a:\apattern\a. Match "
    "and count output will be prefixed by the corresponding \alabel\a:. At "
    "most one label is output for each line; if more than one label matches "
    "a line then it is undefined what label is output.]"
"[O:lenient?Enable lenient \apattern\a interpretation. This is the "
    "default.]"
"[x:line-match|line-regexp?Force \apatterns\a to match complete lines.]"
"[n:number|line-number?Prefix each matched line with its line number.]"
"[N:name?Set the standard input file name prefix to "
    "\aname\a.]:[name:=empty]"
"[q:quiet|silent?Do not print matching lines.]"
"[r|R:recursive?Recursively process all files in each named directory. "
    "Use \btw -e\b \aexpression\a \bgrep ...\b to control the directory "
    "traversal.]"
"[S:strict?Enable strict \apattern\a interpretation with diagnostics.]"
"[s:suppress|no-messages?Suppress error and warning messages.]"
"[t:total?Only print a single matching line count for all files.]"
"[T:test?Enable implementation specific tests.]: [test]"
"[w:word-match|word-regexp?Force \apatterns\a to match complete words.]"
"[a?Ignored for GNU compatibility.]"
"[Y:color|colour?Ignored for GNU compatibility.]:[when]"
"\n"
"\n[ pattern ] [ file ... ]\n"
"\n"
"[+DIAGNOSTICS?Exit status 0 if matches were found, 1 if no matches were "
    "found, where \b-v\b invertes the exit status. Exit status 2 for other "
    "errors that are accompanied by a message on the standard error.]"
"[+SEE ALSO?\bed\b(1), \bsed\b(1), \bperl\b(1), \btw\b(1), \bregex\b(3)]"
"[+CAVEATS?Some expressions of necessity require exponential space "
    "and/or time.]"
"[+BUGS?Some expressions may use sub-optimal algorithms. For example, "
    "don't use this implementation to compute primes.]"
;

#include <cmd.h>
#include <ctype.h>
#include <ccode.h>
#include <error.h>
#include <fts.h>
#include <regex.h>
#include <vmalloc.h>

#ifndef EISDIR
#define EISDIR		(-1)
#endif

/*
 * snarfed from Doug McElroy's C++ version
 *
 * this grep is based on the Posix re package.
 * unfortunately it has to have a nonstandard interface.
 * 1. fgrep does not have usual operators. REG_LITERAL
 * caters for this.
 * 2. grep allows null expressions, hence REG_NULL.
 * 3. it may be possible to combine the multiple 
 * patterns of grep into single patterns.  important
 * special cases are handled by regcomb().
 * 4. anchoring by -x has to be done separately from
 * compilation (remember that fgrep has no ^ or $ operator),
 * hence REG_LEFT|REG_RIGHT.  (An honest, but slow alternative:
 * run regexec with REG_NOSUB off and nmatch=1 and check
 * whether the match is full length)
 */

struct State_s;
typedef struct State_s State_t;

typedef struct Item_s			/* list item			*/
{
	struct Item_s*	next;		/* next in list			*/
	uintmax_t	hits;		/* labeled pattern matches	*/
	uintmax_t	total;		/* total hits			*/
	char		string[1];	/* string value			*/
} Item_t;

typedef struct List_s			/* generic list			*/
{
	Item_t*		head;		/* list head			*/
	Item_t*		tail;		/* list tail			*/
} List_t;

struct State_s				/* program state		*/
{
	regdisc_t	redisc;		/* regex discipline		*/
	regex_t		re;		/* main compiled re		*/

	Vmalloc_t*	vm;		/* allocation region		*/

	struct
	{
	char*		base;		/* sfsetbuf buffer		*/
	size_t		size;		/* sfsetbuf size		*/
	int		noshare;	/* turn off SF_SHARE		*/
	}		buffer;

	Item_t*		hit;		/* label for most recent match	*/

	Sfio_t*		tmp;		/* tmp re compile string	*/

	List_t		files;		/* pattern file list		*/
	List_t		patterns;	/* pattern list			*/
	List_t		labels;		/* labelled re list		*/

	regmatch_t	posvec[1];	/* match position vector	*/
	regmatch_t*	pos;		/* match position pointer	*/
	int		posnum;		/* number of match positions	*/

	char*		span;		/* line span buffer		*/
	size_t		spansize;	/* span buffer size		*/

	int		any;		/* if any pattern hit		*/
	int		after;		/* # lines to list after match	*/
	int		before;		/* # lines to list before match	*/
	int		list;		/* list files with hits		*/
	int		notfound;	/* some input file not found	*/
	int		options;	/* regex options		*/

	uintmax_t	hits;		/* total matched pattern count	*/

	unsigned char	byline;		/* multiple pattern line by line*/
	unsigned char	count;		/* count number of hits		*/
	unsigned char	label;		/* all patterns labelled	*/
	unsigned char	match;		/* match sense			*/
	unsigned char	query;		/* return status but no output	*/
	unsigned char	number;		/* line numbers			*/
	unsigned char	prefix;		/* print file prefix		*/
	unsigned char	suppress;	/* no unopenable file messages	*/
	unsigned char	words;		/* word matches only		*/
};

static void*
labelcomp(const regex_t* re, const char* s, size_t len, regdisc_t* disc)
{
	const char*	e = s + len;
	uintmax_t	n;

	n = 0;
	while (s < e)
		n = (n << 3) + (*s++ - '0');
	return (void*)((char*)0 + n);
}

static int
labelexec(const regex_t* re, void* data, const char* xstr, size_t xlen, const char* sstr, size_t slen, char** snxt, regdisc_t* disc)
{
	((State_t*)disc)->hit = (Item_t*)data;
	return 0;
}

static int
addre(State_t* state, char* s)
{
	int		c;
	int		r;
	char*		b;
	Item_t*		x;

	x = 0;
	r = -1;
	b = s;
	if (state->label)
	{
		if (!(s = strchr(s, ':')))
		{
			error(2, "%s: label:pattern expected", b);
			goto done;
		}
		c = s - b;
		s++;
		if (!(x = vmnewof(state->vm, 0, Item_t, 1, c)))
		{
			error(ERROR_SYSTEM|2, "out of space (pattern `%s')", b);
			goto done;
		}
		if (c)
			memcpy(x->string, b, c);
		x->string[c] = 0;
	}
	if (sfstrtell(state->tmp))
		sfputc(state->tmp, '\n');
	if (state->words)
	{
		if (!(state->options & REG_AUGMENTED))
			sfputc(state->tmp, '\\');
		sfputc(state->tmp, '<');
	}
	sfputr(state->tmp, s, -1);
	if (state->words)
	{
		if (!(state->options & REG_AUGMENTED))
			sfputc(state->tmp, '\\');
		sfputc(state->tmp, '>');
	}
	if (x)
	{
		b = (state->options & (REG_AUGMENTED|REG_EXTENDED)) ? "" : "\\";
		sfprintf(state->tmp, "%s(?{%I*o})", b, sizeof(ptrdiff_t), (char*)x - (char*)0);
		if (state->labels.tail)
			state->labels.tail = state->labels.tail->next = x;
		else
			state->labels.head = state->labels.tail = x;
	}
	state->any = 1;
	r = 0;
 done:
	if (r && x)
		vmfree(state->vm, x);
	return r;
}

static int
addstring(State_t* state, List_t* p, char* s)
{
	Item_t*	x;

	if (!(x = vmnewof(state->vm, 0, Item_t, 1, strlen(s))))
	{
		error(ERROR_SYSTEM|2, "out of space (string `%s')", s);
		return -1;
	}
	strcpy(x->string, s);
	if (p->head)
		p->tail->next = x;
	else
		p->head = x;
	p->tail = x;
	return 0;
}

static int
compile(State_t* state)
{
	int	line;
	int	c;
	int	r;
	size_t	n;
	char*	s;
	char*	t;
	char*	file;
	Item_t*	x;
	Sfio_t*	f;

	r = 1;
	if (!(state->tmp = sfstropen()))
	{
		error(ERROR_SYSTEM|2, "out of space");
		goto done;
	}
	if (state->number || state->before || state->after)
		state->byline = 1;
	for (x = state->patterns.head; x; x = x->next)
		if (addre(state, x->string))
			return r;
	file = error_info.file;
	line = error_info.line;
	f = 0;
	for (x = state->files.head; x; x = x->next)
	{
		s = x->string;
		if (!(f = sfopen(NiL, s, "r")))
		{
			error(ERROR_SYSTEM|2, "%s: cannot open", s);
			r = 2;
			goto done;
		}
		error_info.file = s;
		error_info.line = 0;
		while (s = (char*)sfreserve(f, SF_UNBOUND, SF_LOCKR))
		{
			if (!(n = sfvalue(f)))
				break;
			if (s[n - 1] != '\n')
			{
				for (t = s + n; t > s && *--t != '\n'; t--);
				if (t == s)
				{
					sfread(f, s, 0);
					break;
				}
				n = t - s + 1;
			}
			s[n - 1] = 0;
			if (addre(state, s))
				goto done;
			s[n - 1] = '\n';
			sfread(f, s, n);
		}
		while ((s = sfgetr(f, '\n', 1)) || (s = sfgetr(f, '\n', -1)))
		{
			error_info.line++;
			if (addre(state, s))
				goto done;
		}
		error_info.file = file;
		error_info.line = line;
		sfclose(f);
		f = 0;
	}
	if (!state->any)
	{
		error(2, "no pattern");
		goto done;
	}
	state->any = 0;
	if (!(s = sfstruse(state->tmp)))
	{
		error(ERROR_SYSTEM|2, "out of space");
		goto done;
	}
	error(-1, "RE ``%s''", s);
	state->re.re_disc = &state->redisc;
	if (state->label)
	{
		state->redisc.re_compf = labelcomp;
		state->redisc.re_execf = labelexec;
	}
	if (c = regcomp(&state->re, s, state->options))
	{
		regfatal(&state->re, 2, c);
		goto done;
	}
	if (!regrecord(&state->re))
		state->byline = 1;
	if (!state->label)
	{
		if (!(state->hit = vmnewof(state->vm, 0, Item_t, 1, 0)))
		{
			error(ERROR_SYSTEM|2, "out of space");
			goto done;
		}
		state->labels.head = state->labels.tail = state->hit;
	}
	r = 0;
 done:
	error_info.file = file;
	error_info.line = line;
	if (f)
		sfclose(f);
	if (state->tmp)
		sfstrclose(state->tmp);
	return r;
}

static void
highlight(Sfio_t* sp, const char* s, int n, int so, int eo)
{
	static const char	bold[] =	{CC_esc,'[','1','m'};
	static const char	normal[] =	{CC_esc,'[','0','m'};

	sfwrite(sp, s, so);
	sfwrite(sp, bold, sizeof(bold));
	sfwrite(sp, s + so, eo - so);
	sfwrite(sp, normal, sizeof(normal));
	sfwrite(sp, s + eo, n - eo);
}

static int
hit(State_t* state, const char* prefix, int line, const char* s, size_t len)
{
	state->hit->hits++;
	if (state->query || state->list)
		return -1;
	if (!state->count)
	{
		if (state->prefix)
			sfprintf(sfstdout, "%s:", prefix);
		if (state->number && line)
			sfprintf(sfstdout, "%d:", line);
		if (state->label)
			sfprintf(sfstdout, "%s:", state->hit->string);
		if (state->pos)
			highlight(sfstdout, s, len + 1, state->pos[0].rm_so, state->pos[0].rm_eo);
		else
			sfwrite(sfstdout, s, len + 1);
	}
	return 0;
}

static int
record(void* handle, const char* s, size_t len)
{
	return hit((State_t*)handle, error_info.file, 0, s, len);
}

static int
execute(State_t* state, Sfio_t* input, char* name, Shbltin_t* context)
{
	register char*	s;
	char*		file;
	Item_t*		x;
	size_t		len;
	int		result;
	int		line;

	int		r = 1;
	
	if (state->buffer.noshare)
		sfset(input, SF_SHARE, 0);
	if (state->buffer.size)
		sfsetbuf(input, state->buffer.base, state->buffer.size);
	if (!name)
		name = "(standard input)"; /* posix! (ast prefers /dev/stdin) */
	file = error_info.file;
	error_info.file = name;
	line = error_info.line;
	error_info.line = 0;
	if (state->byline)
	{
		for (;;)
		{
			if (sh_checksig(context))
				goto bad;
			error_info.line++;
			if (s = sfgetr(input, '\n', 0))
				len = sfvalue(input) - 1;
			else if (s = sfgetr(input, '\n', -1))
			{
				len = sfvalue(input);
				s[len] = '\n';
#if _you_like_the_noise
				error(1, "newline appended");
#endif
			}
			else if (sferror(input) && errno != EISDIR)
			{
				error(ERROR_SYSTEM|2, "read error");
				goto bad;
			}
			else
				break;
			if ((result = regnexec(&state->re, s, len, state->posnum, state->pos, 0)) && result != REG_NOMATCH)
			{
				regfatal(&state->re, 2, result);
				goto bad;
			}
			if ((result == 0) == state->match && hit(state, name, error_info.line, s, len) < 0)
				break;
		}
	}
	else
	{
		register char*	e;
		register char*	t;
		char*		r;

		s = e = 0;
		for (;;)
		{
			if (sh_checksig(context))
				goto bad;
			if (s < e)
			{
				t = state->span;
				for (;;)
				{
					len = 2 * (e - s) + t - state->span + 1;
					len = roundof(len, SF_BUFSIZE);
					if (state->spansize < len)
					{
						state->spansize = len;
						len = t - state->span;
						if (!(state->span = vmnewof(state->vm, state->span, char, state->spansize, 0)))
						{
							error(ERROR_SYSTEM|2, "%s: line longer than %lu characters", name, len + e - s);
							goto bad;
						}
						t = state->span + len;
					}
					len = e - s;
					memcpy(t, s, len);
					t += len;
					if (!(s = sfreserve(input, SF_UNBOUND, 0)) || (len = sfvalue(input)) <= 0)
					{
						if ((sfvalue(input) || sferror(input)) && errno != EISDIR)
							error(ERROR_SYSTEM|2, "%s: read error", name);
						break;
					}
					else if (!(e = memchr(s, '\n', len)))
						e = s + len;
					else
					{
						r = s + len;
						len = (e - s) + t - state->span;
						len = roundof(len, SF_BUFSIZE);
						if (state->spansize < len)
						{
							state->spansize = len;
							len = t - state->span;
							if (!(state->span = vmnewof(state->vm, state->span, char, state->spansize, 0)))
							{
								error(ERROR_SYSTEM|2, "%s: line longer than %lu characters", name, len + e - s);
								goto bad;
							}
							t = state->span + len;
						}
						len = e - s;
						memcpy(t, s, len);
						t += len;
						s += len + 1;
						e = r;
						break;
					}
				}
				*t = '\n';
				if (!(len = t - state->span))
					len++;
				if (result = regrexec(&state->re, state->span, len, state->posnum, state->pos, state->options, '\n', (void*)state, record))
				{
					if (result < 0)
						goto done;
					if (result != REG_NOMATCH)
					{
						regfatal(&state->re, 2, result);
						goto bad;
					}
				}
				if (!s)
					break;
			}
			else
			{
				if (!(s = sfreserve(input, SF_UNBOUND, 0)))
				{
					if ((sfvalue(input) || sferror(input)) && errno != EISDIR)
						error(ERROR_SYSTEM|2, "%s: read error", name);
					break;
				}
				if ((len = sfvalue(input)) <= 0)
					break;
				e = s + len;
			}
			t = e;
			while (t > s)
				if (*--t == '\n')
				{
					len = t - s;
					if (!len || t > s && *(t - 1) == '\n')
						len++;
					if (result = regrexec(&state->re, s, len, state->posnum, state->pos, state->options, '\n', (void*)state, record))
					{
						if (result < 0)
							goto done;
						if (result != REG_NOMATCH)
						{
							regfatal(&state->re, 2, result);
							goto bad;
						}
					}
					s = t + 1;
					break;
				}
		}
	}
 done:
	error_info.file = file;
	error_info.line = line;
	x = state->labels.head;
	do
	{
		if (x->hits && state->list >= 0)
		{
			state->any = 1;
			if (state->query)
				break;
		}
		if (!state->query)
		{
			if (!state->list)
			{
				if (state->count)
				{
					if (state->count & 2)
						x->total += x->hits;
					else
					{
						if (state->prefix)
							sfprintf(sfstdout, "%s:", name);
						if (*x->string)
							sfprintf(sfstdout, "%s:", x->string);
						sfprintf(sfstdout, "%I*u\n", sizeof(x->hits), x->hits);
					}
				}
			}
			else if ((x->hits != 0) == (state->list > 0))
			{
				if (state->list < 0)
					state->any = 1;
				if (*x->string)
					sfprintf(sfstdout, "%s:%s\n", name, x->string);
				else
					sfprintf(sfstdout, "%s\n", name);
			}
		}
		x->hits = 0;
	} while (x = x->next);
	r = 0;
 bad:
	error_info.file = file;
	error_info.line = line;
	return r;
}

static int
grep(char* id, int options, int argc, char** argv, Shbltin_t* context)
{
	int	c;
	char*	s;
	char*	h;
	Sfio_t*	f;
	int	flags;
	int	r = 1;
	FTS*	fts;
	FTSENT*	ent;
	State_t	state;

	cmdinit(argc, argv, context, ERROR_CATALOG, ERROR_NOTIFY);
	flags = fts_flags() | FTS_META | FTS_TOP | FTS_NOPOSTORDER | FTS_NOSEEDOTDIR;
	memset(&state, 0, sizeof(state));
	if (!(state.vm = vmopen(Vmdcheap, Vmbest, 0)))
	{
		error(ERROR_SYSTEM|2, "out of space");
		return 2;
	}
	state.redisc.re_version = REG_VERSION;
	state.redisc.re_flags = REG_NOFREE;
	state.redisc.re_resizef = (regresize_t)vmgetmem;
	state.redisc.re_resizehandle = (void*)state.vm;
	state.match = 1;
	state.options = REG_FIRST|REG_NOSUB|REG_NULL|REG_DISCIPLINE|REG_MULTIPLE|options;
	if (strcmp(astconf("CONFORMANCE", NiL, NiL), "standard"))
		state.options |= REG_LENIENT;
	error_info.id = id;
	h = 0;
	fts = 0;
	while (c = optget(argv, usage))
		switch (c)
		{
		case 'C':
			if (opt_info.arg)
			{
				state.before = (int)strtol(opt_info.arg, &s, 0);
				state.after = (*s == ',') ? (int)strtol(s + 1, &s, 0) : state.before;
				if (*s)
					error(3, "%s: invalid context line count", s);
			}
			else
				state.before = state.after = 2;
			break;
		case 'E':
			state.options |= REG_EXTENDED;
			break;
		case 'F':
			state.options |= REG_LITERAL;
			break;
		case 'G':
			state.options &= ~(REG_AUGMENTED|REG_EXTENDED);
			break;
		case 'H':
			state.prefix = opt_info.num;
			break;
		case 'L':
			state.list = -opt_info.num;
			break;
		case 'N':
			h = opt_info.arg;
			break;
		case 'O':
			state.options |= REG_LENIENT;
			break;
		case 'P':
			state.options |= REG_EXTENDED|REG_LENIENT;
			break;
		case 'S':
			state.options &= ~REG_LENIENT;
			break;
		case 'T':
			s = opt_info.arg;
			switch (*s)
			{
			case 'b':
			case 'm':
				c = *s++;
				state.buffer.size = strton(s, &s, NiL, 1);
				if (c == 'b' && !(state.buffer.base = newof(0, char, state.buffer.size, 0)))
				{
					error(ERROR_SYSTEM|2, "out of space [test buffer]");
					goto done;
				}
				if (*s)
				{
					error(2, "%s: invalid characters after test", s);
					goto done;
				}
				break;
			case 'f':
				state.options |= REG_FIRST;
				break;
			case 'l':
				state.options |= REG_LEFT;
				break;
			case 'n':
				state.buffer.noshare = 1;
				break;
			case 'r':
				state.options |= REG_RIGHT;
				break;
			case 'L':
				state.byline = 1;
				break;
			default:
				error(3, "%s: unknown test", s);
				break;
			}
			break;
		case 'X':
			state.options |= REG_AUGMENTED;
			break;
		case 'a':
			break;
		case 'b':
			state.options &= ~(REG_FIRST|REG_NOSUB);
			break;
		case 'c':
			state.count |= 1;
			break;
		case 'e':
			if (addstring(&state, &state.patterns, opt_info.arg))
				goto done;
			break;
		case 'f':
			if (addstring(&state, &state.files, opt_info.arg))
				goto done;
			break;
		case 'h':
			state.prefix = 2;
			break;
		case 'i':
			state.options |= REG_ICASE;
			break;
		case 'l':
			state.list = opt_info.num;
			break;
		case 'm':
			state.label = 1;
			break;
		case 'n':
			state.number = 1;
			break;
		case 'q':
			state.query = 1;
			break;
		case 'r':
			if (opt_info.num)
				flags &= ~FTS_TOP;
			break;
		case 's':
			state.suppress = opt_info.num;
			break;
		case 't':
			state.count |= 2;
			break;
		case 'v':
			if (state.match = !opt_info.num)
				state.options &= ~REG_INVERT;
			else
				state.options |= REG_INVERT;
			break;
		case 'w':
			state.words = 1;
			break;
		case 'x':
			state.options |= REG_LEFT|REG_RIGHT;
			break;
		case 'Y':
			/* ignored for GNU compatibility */
			break;
		case '?':
			vmclose(state.vm);
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			goto done;
		default:
			error(2, "%s: not implemented", opt_info.name);
			goto done;
		}
	argv += opt_info.index;
	if ((state.options & REG_LITERAL) && (state.options & (REG_AUGMENTED|REG_EXTENDED)))
	{
		error(2, "-F and -A or -P or -X are incompatible");
		goto done;
	}
	if ((state.options & REG_LITERAL) && state.words)
	{
		error(ERROR_SYSTEM|2, "-F and -w are incompatible");
		goto done;
	}
	if (!state.files.head && !state.patterns.head)
	{
		if (!argv[0])
		{
			error(2, "no pattern");
			goto done;
		}
		if (addstring(&state, &state.patterns, *argv++))
			goto done;
	}
	if (!(state.options & (REG_FIRST|REG_NOSUB)))
	{
		if (state.count || state.list || state.query || (state.options & REG_INVERT))
			state.options |= REG_FIRST|REG_NOSUB;
		else
		{
			state.pos = state.posvec;
			state.posnum = elementsof(state.posvec);
		}
	}
	if (r = compile(&state))
		goto done;
	sfset(sfstdout, SF_LINE, 1);
	if (!argv[0])
	{
		state.prefix = h ? 1 : 0;
		if (r = execute(&state, sfstdin, h, context))
			goto done;
	}
	if (state.prefix > 1)
		state.prefix = 0;
	else if (!(flags & FTS_TOP) || argv[1])
		state.prefix = 1;
	if (!(fts = fts_open(argv, flags, NiL)))
	{
		error(ERROR_SYSTEM|2, "%s: not found", argv[0]);
		r = 1;
		goto done;
	}
	while (!sh_checksig(context) && (ent = fts_read(fts)))
		switch (ent->fts_info)
		{
		case FTS_F:
			if (f = sfopen(NiL, ent->fts_accpath, "r"))
			{
				r = execute(&state, f, ent->fts_path, context);
				sfclose(f);
				if (r)
					goto done;
				if (state.query && state.any)
					goto quit;
				break;
			}
			/*FALLTHROUGH*/
		case FTS_NS:
		case FTS_SLNONE:
			state.notfound = 1;
			if (!state.suppress)
				error(ERROR_SYSTEM|2, "%s: cannot open", ent->fts_path);
			break;
		case FTS_DC:
			error(ERROR_WARNING|1, "%s: directory causes cycle", ent->fts_path);
			break;
		case FTS_DNR:
			error(ERROR_SYSTEM|2, "%s: cannot read directory", ent->fts_path);
			break;
		case FTS_DNX:
			error(ERROR_SYSTEM|2, "%s: cannot search directory", ent->fts_path);
			break;
		}
 quit:
	if ((state.count & 2) && !state.query && !state.list)
	{
		Item_t*		x;

		x = state.labels.head;
		do
		{
			if (*x->string)
				sfprintf(sfstdout, "%s:", x->string);
			sfprintf(sfstdout, "%I*u\n", sizeof(x->total), x->total);
		} while (x = x->next);
	}
	r = (state.notfound && !state.query) ? 2 : !state.any;
 done:
	if (fts)
		fts_close(fts);
	vmclose(state.vm);
	sfset(sfstdout, SF_LINE, 0);
	if (sfsync(sfstdout))
		error(ERROR_SYSTEM|2, "write error");
	if (sh_checksig(context))
	{
		errno = EINTR;
		r = 2;
	}
	return r;
}

int
b_grep(int argc, char** argv, Shbltin_t* context)
{
	char*	s;
	int	options;

	NoP(argc);
	options = 0;
	if (s = strrchr(argv[0], '/'))
		s++;
	else
		s = argv[0];
	switch (*s)
	{
	case 'e':
	case 'E':
		s = "egrep";
		options = REG_EXTENDED;
		break;
	case 'f':
	case 'F':
		s = "fgrep";
		options = REG_LITERAL;
		break;
	case 'p':
	case 'P':
		s = "pgrep";
		options = REG_EXTENDED|REG_LENIENT;
		break;
	case 'x':
	case 'X':
		s = "xgrep";
		options = REG_AUGMENTED;
		break;
	default:
		s = "grep";
		break;
	}
	return grep(s, options, argc, argv, context);
}

int
b_egrep(int argc, char** argv, Shbltin_t* context)
{
	NoP(argc);
	return grep("egrep", REG_EXTENDED, argc, argv, context);
}

int
b_fgrep(int argc, char** argv, Shbltin_t* context)
{
	NoP(argc);
	return grep("fgrep", REG_LITERAL, argc, argv, context);
}

int
b_pgrep(int argc, char** argv, Shbltin_t* context)
{
	NoP(argc);
	return grep("pgrep", REG_EXTENDED|REG_LENIENT, argc, argv, context);
}

int
b_xgrep(int argc, char** argv, Shbltin_t* context)
{
	NoP(argc);
	return grep("xgrep", REG_AUGMENTED, argc, argv, context);
}

#if STANDALONE

int
main(int argc, char** argv)
{
	return b_grep(argc, argv, 0);
}

#endif
