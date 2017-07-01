/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2013 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * David Korn
 * Glenn Fowler
 * AT&T Research
 */

static const char usage[] =
"[-?\n@(#)$Id: tr (AT&T Research) 2013-09-13 $\n]"
USAGE_LICENSE
"[+NAME?tr - translate, squeeze, and/or delete characters]"
"[+DESCRIPTION?\btr\b copies the standard input to the standard output "
    "with substitution or deletion of selected characters. Input characters "
    "in \aset1\a are mapped to corresponding characters in \aset2\a.]"

"[c:complement?Complement \aset1\a using the character value order.]"
"[C:collate?Complement \aset1\a using the character collation order.]"
"[d:delete?Delete characters in \aset1\a but do not translate.]"
"[q:quiet?Suppress invalid multibyte character warnings.]"
"[s:squeeze-repeats?Replace sequences of the same character with one.]"
"[t:truncate-set1?Truncate \aset1\a to the length of \aset2\a.]"

"[+?\asets\a are specified as strings of characters. Most represent"
    "themselves. Interpreted sequences are:]"
    "{"
        "[+\\nnn?character with octal value \annn\a]"
        "[+\\xnn?character with hexadecimal value \ann\a]"
        "[+\\\\?backslash]"
        "[+\\a?alert]"
        "[+\\b?backpace]"
        "[+\\f?form feed]"
        "[+\\r?return]"
        "[+\\t?horizontal tab]"
        "[+\\v?vertical tab]"
        "[+\\E?escape]"
        "[+c1-c2?all characters from \ac1\a to \ac2\a in ascending order]"
        "[+[c1-c2]]?same as \ac1-c2\a if both \asets\a use this form]"
        "[+[c*]]?in \aset2\a, copies of \ac\a until length of \aset1\a]"
        "[+[c*n]]?\an\a copies of \ac\a]"
        "[+[::alnum::]]?all letters and digits]"
        "[+[::alpha::]]?all letters]"
        "[+[::blank::]]?all horizontal whitespace]"
        "[+[::cntrl::]]?all control characters]"
        "[+[::digit::]]?all digits]"
        "[+[::graph::]]?all printable characters, not including space]"
        "[+[::lower::]]?all lower case letters]"
        "[+[::print::]]?all printable characters, including space]"
        "[+[::punct::]]?all punctuation characters]"
        "[+[::space::]]?all horizontal or vertical whitespace]"
        "[+[::upper::]]?all upper case letters]"
        "[+[::xdigit::]]?all hexadecimal digits]"
        "[+[=c=]]]]?all characters which are equivalent to \ac\a]"
    "}"
"[+?Translation occurs if \b-d\b is not given and both \aset1\a and "
    "\aset2\a appear. \b-t\b may be used only when translating. \aset2\a is "
    "extended to the length of \aset1\a by repeating its last character as "
    "necessary. Excess characters in \aset2\a are ignored. Only [:lower:]]"
    "and [:upper:]] are guaranteed to expand in ascending order. They may "
    "only be used in pairs to specify case conversion. \b-s\b uses \aset1\a "
    "if neither translating nor deleting, otherwise squeeze uses \aset2\a "
    "and occurs after translation or deletion.]"
"[+?The \bLC_COLLATE\b and \bLC_CTYPE\b locale categories affect the "
    "character collation order for the \b-C\b option and how characters are "
    "interpreted (single/multi byte.) Somebody please design a portable "
    "defacto standard locale aware collation sequence API.]"

"\n"
"\n[ set1 [ set2 ] ]\n"
"\n"

"[+SEE ALSO?\bsed\b(1), \bwcscmp\b(3), \bwcsxfrm\b(3), \bascii\b(5)]"

;

#include <cmd.h>
#include <ctype.h>
#include <error.h>
#include <regex.h>

#if _hdr_wctype
#include <wctype.h>
#endif

#define TR_COLLATE	0x0001
#define TR_COMPLEMENT	0x0002
#define TR_DELETE	0x0004
#define TR_QUIET	0x0008
#define TR_SQUEEZE	0x0010
#define TR_TRUNCATE	0x0020

#define HITBIT		0x80000000
#define DELBIT		0x40000000
#define ONEBIT		0x20000000

#define setchar(p,s,t)	((p)->type=(t),(p)->prev=(p)->last=(-1),(p)->isit=0,(p)->iseq=0,(p)->count=0,(p)->base=(p)->next=(s))

#if !defined(towupper) && !_lib_towupper
#define towupper(x)	toupper(x)
#endif

#if !defined(towlower) && !_lib_towlower
#define towlower(x)	tolower(x)
#endif

typedef int (*Compare_f)(const void*, const void*);

typedef struct Collate_s
{
	wchar_t		seq[15];
	wchar_t		chr;
} Collate_t;

typedef struct Tr_s
{
	Shbltin_t*	context;
	int		convert;
	int		count;
	int		prev;
	int		last;
	int		level;
	int		mb;
	int		position;
	int		src;
	int		dst;
	int		type;
	int		truncate;
	int		chars;
	int		warn;
	regclass_t	isit;
	regex_t*	iseq;
	regex_t		eqre;
	unsigned char*	base;
	unsigned char*	next;
	unsigned char*	hold;
	Mbstate_t	q;
	uint32_t	code[1];
} Tr_t;

static const char*	typename[] = { "source", "destination" };

/*
 * return next string character
 * the string pointer is advanced
 * returns -1 for end of string
 * returns -2 for string format error
 */

static int
nextchar(register Tr_t* tr)
{
	register int	c;
	int		q;
	unsigned char*	e;
	regclass_t	f;
	wchar_t		w;
	char		buf[32];

	/*
	 * tr.count>0 when tr.type==1 string contains x*count
	 */

	if (tr->count)
	{
		if (tr->count > 0)
			tr->count--;
		return tr->prev;
	}

	/*
	 * tr.last>=0 when string contains char/equiv class
	 */

 next:
	if (tr->last >= 0)
	{
		while (++tr->prev <= tr->last)
		{
			if (tr->isit)
			{
				if (!(*tr->isit)(tr->prev))
					continue;
			}
			if (tr->iseq)
			{
				if ((c = mbtconv(buf, tr->prev, &tr->q)) <= 0 || regnexec(tr->iseq, buf, c, 0, NiL, 0))
					continue;
			}
			return (!tr->type || !tr->convert) ? tr->prev : tr->convert == 'l' ? towlower(tr->prev) : towupper(tr->prev);
		}
		if (tr->isit)
			tr->isit = 0;
		else if (tr->iseq)
		{
			regfree(tr->iseq);
			tr->iseq = 0;
		}
		tr->prev--;
		tr->last = -1;
		tr->hold = tr->next;
		mbtinit(&tr->q);
		mbtchar(&w, tr->hold, MB_LEN_MAX, &tr->q);
	}
	switch (c = mbtchar(&w, tr->next, MB_LEN_MAX, &tr->q))
	{
	case -1:
		c = *(tr->next - 1);
		if (tr->warn)
			error(2, "%s operand: \\x%02x: invalid multibyte character byte", typename[tr->type], c);
		break;
	case 0:
		tr->next--;
		c = tr->level ? -2 : tr->type && !tr->truncate ? tr->prev : -1;
		break;
	case '\\':
		c = chresc((char*)tr->next - 1, (char**)&tr->next);
		break;
	case '[':
		switch (*tr->next)
		{
		case ':':
			f = 0;
			if (tr->convert)
			{
				c = *(tr->next + 1);
				if (tr->convert == c || tr->type && !tr->position)
				{
					c = *tr->next;
					goto member;
				}
				else if (!strncmp((char*)tr->next, ":lower:", 7) || !strncmp((char*)tr->next, ":upper:", 7))
				{
					f = tr->isit;
					tr->convert = c;
					c = tr->next - tr->base;
					if (!tr->type)
						tr->position = c;
					else if (tr->position != c)
						return -2;
				}
			}
			if (!(tr->isit = regclass((char*)tr->next, (char**)&e)))
			{
				if (f)
					tr->isit = f;
				c = ':';
				goto member;
			}
			tr->next = e;
			if (f)
				tr->isit = f;
			tr->prev = -1;
			tr->last = tr->chars;
			return nextchar(tr);
		case '.':
		case '=':
			if ((q = regcollate((char*)tr->next, (char**)&e, buf, sizeof(buf), &w)) >= 0)
			{
				if (*tr->next == '=' && q > 0)
				{
					sfsprintf(buf, sizeof(buf), "^[[%-.*s]$", e - tr->next, tr->next);
					if (c = regcomp(&tr->eqre, buf, REG_EXTENDED))
						regfatalpat(&tr->eqre, 3, c, buf);
					tr->next = e;
					tr->iseq = &tr->eqre;
					tr->prev = -1;
					tr->last = tr->chars;
					return nextchar(tr);
				}
				tr->next = e;
				c = q > 1 ? w : q ? buf[0] : 0;
				break;
			}
			/*FALLTHROUGH*/
		member:
			if (*(e = tr->next + 1) && *e != ']')
			{
				while (*++e && *e != c && *e != ']');
				if (*e != ']' && *++e == ']')
					return -2;
			}
		default:
			if (!tr->level)
			{
				tr->level++;
				c = nextchar(tr);
				if (tr->type == 1 && *tr->next == '*')
				{
					e = tr->next + 1;
					if (!(tr->count = (int)strtol((char*)tr->next + 1, (char**)&tr->next, 0)) && tr->next == e)
						tr->count = -1;
					if (*tr->next++ != ']')
						return -2;
					if (tr->count < 0)
					{
						/*
						 * tr->src chars total
						 * tr->dst chars so far
						 * count what's left
						 */

						Tr_t	peek;

						peek = *tr;
						peek.count = 0;
						peek.last = -1;
						while (nextchar(&peek) >= 0)
							peek.dst++;
						tr->count = tr->src - peek.dst;
					}
					else if (tr->count > tr->chars)
						tr->count = tr->chars;
					if (!tr->count)
						goto next;
					tr->count--;
					tr->level--;
				}
			}
			break;
		}
		break;
	case '-':
		if (tr->prev >= 0 && tr->next != tr->hold && *tr->next)
		{
			c = tr->prev;
			tr->last = nextchar(tr);
			if (c > tr->last)
				return -2;
			tr->prev = c;
			goto next;
		}
		break;
	case ']':
		if (tr->level > 0 && tr->next > tr->base + 2)
		{
			tr->level--;
			c = nextchar(tr);
		}
		break;
	}
	return tr->prev = c;
}

/*
 * return a tr handle for <src,dst>
 */

static Tr_t*
tropen(unsigned char* src, unsigned char* dst, unsigned int flags, Shbltin_t* context)
{
	register Tr_t*	tr;
	register int	c;
	register int	n;
	register int	x;
	register int	squeeze;
	int		m;
	size_t		z;
	uint32_t*	set;

	m = mbwide() ? 0x110000 : 0x100;
	z = 0x100;
	if (!(tr = newof(0, Tr_t, 1, (m - 1) * sizeof(uint32_t))) || !(set = newof(0, uint32_t, z, 0)))
	{
		if (tr)
			free(tr);
		error(2, "out of space [code]");
		return 0;
	}
	tr->context = context;
	if (!(flags & TR_QUIET))
		tr->warn = 1;
#if DEBUG_TRACE
	error(-1, "AHA#%d m=%d tr=%p set=%p", __LINE__, m, tr, set);
#endif
	tr->chars = m;
	switch (flags & (TR_DELETE|TR_SQUEEZE))
	{
	case TR_DELETE:
	case TR_SQUEEZE:
	case TR_DELETE|TR_SQUEEZE:
		break;
	default:
		tr->convert = '?';
		break;
	}
	tr->truncate = flags & TR_TRUNCATE;
	if (dst && !*dst)
		dst = 0;
	squeeze = (flags & TR_SQUEEZE) ? ONEBIT : 0;
	for (n = 0; n < m; n++)
		tr->code[n] = n;
	n = 0;
	if (src)
	{
		setchar(tr, src, 0);
		while ((c = nextchar(tr)) >=0)
		{
			tr->code[c] |= HITBIT;
#if DEBUG_TRACE
error(-1, "src %d '%c'", n, c);
#endif
			if (n >= z)
			{
				z *= 2;
				if (!(set = newof(set, uint32_t, z, 0)))
				{
					error(ERROR_SYSTEM|2, "out of memory");
					goto big;
				}
			}
			set[n++] = c;
			if (c > 0x7f)
				tr->mb = 1;
		}
		if (c < -1)
			goto bad;
	}
	if (flags & TR_COMPLEMENT)
	{
		for (n = c = 0; n < m; n++)
			if (!(tr->code[n] & HITBIT))
			{
				if (c >= z)
				{
					z *= 2;
					if (!(set = newof(set, uint32_t, z, 0)))
					{
						error(ERROR_SYSTEM|2, "out of memory");
						goto big;
					}
				}
				set[c++] = n;
				if (n > 0x7f)
					tr->mb = 1;
			}
#if _lib_wcscmp && _lib_wcsxfrm
		if (flags & TR_COLLATE)
		{
			Collate_t*	col;
			wchar_t		w[2];

			if (!(col = newof(0, Collate_t, c, 0)))
			{
				error(ERROR_SYSTEM|2, "out of space (how about a standard collating sequence api)");
				goto big;
			}
			w[1] = 0;
			for (n = 0; n < c; n++)
			{
				w[0] = set[n];
				col[n].chr = set[n];
				wcsxfrm(col[n].seq, w, sizeof(col[n].seq));
			}
			qsort(col, c, sizeof(col[0]), (Compare_f)wcscmp);
			for (n = 0; n < c; n++)
				set[n] = col[n].chr;
		}
#endif
		tr->src = c;
	}
	else
		tr->src = n;
	if (tr->convert == '?')
		tr->convert = 0;
	setchar(tr, dst, 1);
	for (tr->dst = 0; tr->dst < tr->src; tr->dst++)
	{
		c = set[tr->dst];
		if (flags & TR_DELETE)
			tr->code[c] |= DELBIT;
		else if (dst)
		{
			if ((x = nextchar(tr)) >= 0)
			{
#if DEBUG_TRACE
error(-1, "dst %d '%c' => '%c'", tr->dst, c, x);
#endif
				tr->code[c] = x | squeeze;
				if (x > 0x7f)
					tr->mb = 1;
			}
			else if (x < -1)
				goto bad;
			else if (tr->truncate)
			{
				while (tr->dst < tr->src)
				{
					c = set[tr->dst++];
					tr->code[c] = c | squeeze;
					if (c > 0x7f)
						tr->mb = 1;
				}
				break;
			}
		}
		else
		{
			x = squeeze ? c : 0;
			tr->code[c] = x | squeeze;
			if (x > 0x7f)
				tr->mb = 1;
		}
	}
	if ((flags & (TR_DELETE|TR_SQUEEZE)) == (TR_DELETE|TR_SQUEEZE))
	{
		tr->truncate = 1;
		for (tr->dst = 0; (x = nextchar(tr)) >= 0; tr->dst++)
			if (!(tr->code[x] & DELBIT))
			{
#if DEBUG_TRACE
error(-1, "dst %d '%c'", tr->dst, x);
#endif
				tr->code[x] = x | ONEBIT;
				if (x > 0x7f)
					tr->mb = 1;
			}
		if (x < -1)
			goto bad;
	}
	free(set);
	if (tr->mb)
		tr->mb = mbwide();
	return tr;
 bad:
	error(2, "%s: invalid %s string", tr->base, typename[tr->type]);
 big:
	if (set)
		free(set);
	free(tr);
	return 0;
}

/*
 * close a tr handle
 */

void
trclose(Tr_t* tr)
{
	free(tr);
}

/*
 * tr each char of ip and put results to op
 * stop after <ncopy> bytes are written
 */

static ssize_t
trcopy(Tr_t* tr, Sfio_t* ip, Sfio_t* op, ssize_t ncopy)
{
	register int		c;
	register int		oldc = -1;
	register uint32_t*	code = tr->code;
	register unsigned char*	inp = 0;
	register unsigned char*	outp = 0;
	register unsigned char*	inend;
	register unsigned char*	outend = 0;
	register ssize_t	nwrite = 0;
	unsigned char*		inbuf = 0;
	unsigned char*		outbuf = 0;

	if (tr->mb)
	{
		unsigned char*		pushbuf = 0;
		unsigned char*		pushend;
		size_t			line;
		size_t			eline;
		ssize_t			o;
		wchar_t			w;
		int			n;
		int			m;
		int			eof;
		unsigned char		side[256];

		line = 1;
		eline = 0;
		eof = 0;
		if ((m = mbmax()) > (sizeof(side) / 2))
			m = sizeof(side) / 2;
		inp = inend = 0;
		while (!sh_checksig(tr->context))
		{
			if ((o = inend - inp) <= 0 || (mbchar(&w, inp, o, &tr->q), mberrno(&tr->q)))
			{
				if (o >= m || o && mberrno(&tr->q) == EILSEQ)
					w = -1;
				else if (pushbuf)
				{
					if (inp >= &side[sizeof(side)/2])
					{
						inp = pushbuf + (inp - &side[sizeof(side)/2]);
						pushbuf = 0;
						inend = pushend;
						continue;
					}
					if (eof)
					{
						if (o <= 0)
							break;
						w = -1;
					}
					else if ((c = &side[sizeof(side)] - inend) <= 0)
						w = -1;
					else if (!(pushbuf = (unsigned char*)sfreserve(ip, SF_UNBOUND, 0)) || (n = sfvalue(ip)) <= 0)
					{
						eof = 1;
						w = -1;
					}
					else
					{
						pushend = pushbuf + n;
						if (c > n)
							c = n;
						memcpy(inend, pushbuf, c);
						inend += c;
						continue;
					}
				}
				else if (eof)
				{
					if (o <= 0)
						break;
					w = -1;
				}
				else
				{
					if (inp)
					{
						if (o)
							memcpy(side, inp, o);
						mbinit(&tr->q);
					}
					else
						o = 0;
					if (!(inp = (unsigned char*)sfreserve(ip, SF_UNBOUND, 0)) || (n = sfvalue(ip)) <= 0)
					{
						eof = 1;
						inp = side;
						inend = inp + o;
					}
					else if (o)
					{
						pushbuf = inp;
						pushend = pushbuf + n;
						inp = side + o;
						if ((c = sizeof(side) - o) > n)
							c = n;
						if (c)
							memcpy(inp, pushbuf, c);
						inend = inp + c;
						inp = side;
					}
					else
						inend = inp + n;
					continue;
				}
				if (w < 0)
				{
					w = *(inp - 1);
					sfputc(sfstdout, w);
					if (tr->warn && eline != line)
					{
						eline = line;
						error(2, "line %zu: \\x%02x: invalid multibyte character byte", line, w);
					}
					continue;
				}
			}
			if (w == '\n')
				line++;
			w = code[w];
			if (!(w & DELBIT) && w != oldc)
			{
				sfputwc(sfstdout, w & ~(HITBIT|ONEBIT));
				oldc = w | ONEBIT;
			}
		}
	}
	else
	{
		while (nwrite != ncopy && !sh_checksig(tr->context))
		{
			if (!(inbuf = (unsigned char*)sfreserve(ip, SF_UNBOUND, SF_LOCKR)))
			{
				if (sfvalue(ip))
				{
					error(2, ERROR_SYSTEM|2, "read error");
					return -1;
				}
				break;
			}
			c = sfvalue(ip);
			inend = (inp = inbuf) + c;

			/*
			 * process the next input buffer
			 */

			while (inp < inend)
			{
				if (outp >= outend)
				{
					/*
					 * write out current buffer
					 */

					if ((c = outp - outbuf) > 0)
					{
						if ((nwrite += c) == ncopy)
							break;
						sfwrite(op, outbuf, c);
					}

					/*
					 * get write buffer space
					 */

					if (!(outbuf = (unsigned char*)sfreserve(op, (ncopy < 0) ? SF_UNBOUND : (ncopy - nwrite), SF_LOCKR)))
						break;
					outend = (outp = outbuf) + sfvalue(op);
				}
				c = code[*inp++];
				if (!(c & DELBIT) && c != oldc)
				{
					*outp++ = c;
					oldc = c | ONEBIT;
				}
			}
			sfread(ip, inbuf, inp - inbuf);
			inp = inbuf;
		}
		if (inbuf && (c = inp - inbuf) > 0)
			sfread(ip, inbuf, c);
		if (outbuf && (c = outp - outbuf) >= 0)
			sfwrite(op, outbuf, c);
	}
	if (sfsync(op))
	{
		if (!ERROR_PIPE(errno))
			error(ERROR_SYSTEM|2, "write error [%d]", c);
		return -1;
	}
	return nwrite;
}

int
b_tr(int argc, char** argv, Shbltin_t* context)
{
	register unsigned int	flags = 0;
	Tr_t*			tr;

	cmdinit(argc, argv, context, ERROR_CATALOG, 0);
	flags = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'c':
			flags |= TR_COMPLEMENT;
			continue;
		case 'C':
			flags |= TR_COMPLEMENT|TR_COLLATE;
			continue;
		case 'd':
			flags |= TR_DELETE;
			continue;
		case 'q':
			flags |= TR_QUIET;
			continue;
		case 's':
			flags |= TR_SQUEEZE;
			continue;
		case 't':
			flags |= TR_TRUNCATE;
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (tr = tropen((unsigned char*)argv[0], (unsigned char*)argv[0] ? (unsigned char*)argv[1] : (unsigned char*)0, flags, context))
	{
		trcopy(tr, sfstdin, sfstdout, SF_UNBOUND);
		trclose(tr);
	}
	return error_info.errors != 0;
}
