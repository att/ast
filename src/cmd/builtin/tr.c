/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2012 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * David Korn
 * Glenn Fowler
 * AT&T Research
 *
 * tr
 */

static const char usage[] =
"[-?\n@(#)$Id: tr (AT&T Research) 2012-05-31 $\n]"
USAGE_LICENSE
"[+NAME?tr - translate, squeeze, and/or delete characters]"
"[+DESCRIPTION?\btr\b copies the standard input to the standard output"
"	with substitution or deletion of selected characters. Input"
"	characters in \aset1\a are mapped to corresponding characters"
"	in \aset2\a.]"

"[c:complement?Complement \aset1\a.]"
"[d:delete?Delete characters in \aset1\a but do not translate.]"
"[s:squeeze-repeats?Replace sequences of the same character with one.]"
"[t:truncate-set1?Truncate \aset1\a to the length of \aset2\a.]"

"[+?\asets\a are specified as strings of characters. Most represent"
"	themselves. Interpreted sequences are:]{"
"	[+\\nnn?character with octal value \annn\a]"
"	[+\\xnn?character with hexadecimal value \ann\a]"
"	[+\\\\?backslash]"
"	[+\\a?alert]"
"	[+\\b?backpace]"
"	[+\\f?form feed]"
"	[+\\r?return]"
"	[+\\t?horizontal tab]"
"	[+\\v?vertical tab]"
"	[+\\E?escape]"
"	[+c1-c2?all characters from \ac1\a to \ac2\a in ascending order]"
"	[+[c1-c2]]?same as \ac1-c2\a if both \asets\a use this form]"
"	[+[[c*]]]]?in \aset2\a, copies of \\ac\\a until length of \aset1\a]"
"	[+[[c*n]]]]?\\an\\a copies of \\ac\\a]"
"	[+[[::alnum::]]]]?all letters and digits]"
"	[+[[::alpha::]]]]?all letters]"
"	[+[[::blank::]]]]?all horizontal whitespace]"
"	[+[[::cntrl::]]]]?all control characters]"
"	[+[[::digit::]]]]?all digits]"
"	[+[[::graph::]]]]?all printable characters, not including space]"
"	[+[[::lower::]]]]?all lower case letters]"
"	[+[[::print::]]]]?all printable characters, including space]"
"	[+[[::punct::]]]]?all punctuation characters]"
"	[+[[::space::]]]]?all horizontal or vertical whitespace]"
"	[+[[::upper::]]]]?all upper case letters]"
"	[+[[::xdigit::]]]]?all hexadecimal digits]"
"	[+[[=c=]]]]?all characters which are equivalent to \\ac\\a]"
"	}"
"[+?Translation occurs if \b-d\b is not given and both \aset1\a"
"	and \aset2\a appear. \b-t\b may be used only when translating."
"	\aset2\a is extended to the length of \aset1\a by repeating its last"
"	character as necessary.  Excess characters in \aset2\a are ignored."
"	Only [:lower:]] and [:upper:]] are guaranteed to expand in ascending"
"	order. They may only be used in pairs to specify case conversion."
"	\b-s\b uses \aset1\a if neither translating nor deleting, otherwise"
"	squeeze uses \aset2\a and occurs after translation or deletion.]"

"\n"
"\n[ set1 [ set2 ] ]\n"
"\n"
"[+SEE ALSO?\bsed\b(1), \bascii\b(5)]"
;

#include <cmd.h>
#include <ctype.h>
#include <error.h>
#include <regex.h>

#define TR_COMPLEMENT	(1<<0)
#define TR_DELETE	(1<<1)
#define TR_SQUEEZE	(1<<2)
#define TR_TRUNCATE	(1<<3)

#define HITBIT		(1<<(CHAR_BIT+1))
#define DELBIT		(1<<(CHAR_BIT+2))
#define ONEBIT		(1<<(CHAR_BIT+3))

#define setchar(p,s,t)	((p)->type=(t),(p)->prev=(p)->last=(-1),(p)->isit=0,(p)->count=0,(p)->base=(p)->next=(s))

typedef struct
{
	int		code[1<<CHAR_BIT];
	int		convert;
	int		count;
	int		prev;
	int		last;
	int		level;
	int		position;
	int		src;
	int		dst;
	int		type;
	int		truncate;
	regclass_t	isit;
	unsigned char*	base;
	unsigned char*	next;
	unsigned char*	hold;
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
	wchar_t		wc;
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
	 * tr.last>=0 when string contains char class
	 */

 next:
	if (tr->last >= 0)
	{
		while (++tr->prev <= tr->last)
			if (!tr->isit || (*tr->isit)(tr->prev))
				return (!tr->type || !tr->convert) ? tr->prev : tr->convert == 'l' ? tolower(tr->prev) : toupper(tr->prev);
		tr->last = -1;
		tr->hold = tr->next + 1;
	}
	switch (c = *tr->next++)
	{
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
			tr->last = UCHAR_MAX + 1;
			return nextchar(tr);
		case '.':
		case '=':
			if ((q = regcollate((char*)tr->next, (char**)&e, buf, sizeof(buf), &wc)) >= 0)
			{
				tr->next = e;
				c = q ? buf[0] : 0;
				break;
			}
			/*FALLTHROUGH*/
		member:
			if (*(e = tr->next + 1))
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
				if (*tr->next == '*')
				{
					e = tr->next + 1;
					if (!(tr->count = (int)strtol((char*)tr->next + 1, (char**)&tr->next, 0)) && tr->next == e)
					{
						if (tr->type == 0)
							return -2;
						tr->count = -1;
					}
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
					else if (tr->count > (1<<CHAR_BIT))
						tr->count = (1<<CHAR_BIT);
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
tropen(unsigned char* src, unsigned char* dst, int flags)
{
	register Tr_t*	tr;
	register int	c;
	register int	n;
	register int	x;
	register int	squeeze;
	unsigned int	set[1<<(CHAR_BIT+1)];

	if (!(tr = newof(0, Tr_t, 1, 0)))
	{
		error(2, "out of space [code]");
		return 0;
	}
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
	for (n = 0; n < (1<<CHAR_BIT); n++)
		tr->code[n] = n;
	n = 0;
	if (src)
	{
		setchar(tr, src, 0);
		while ((c = nextchar(tr)) >=0 && n < elementsof(set))
		{
			tr->code[c] |= HITBIT;
#if DEBUG_TRACE
error(-1, "src %d '%c'", n, c);
#endif
			set[n++] = c;
		}
		if (c < -1)
			goto bad;
	}
	tr->src = n;
	if (flags & TR_COMPLEMENT)
	{
		for (n = c = 0; n < (1<<CHAR_BIT); n++)
			if (!(tr->code[n] & HITBIT))
				set[c++] = n;
		tr->src = c;
	}
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
			}
			else if (x < -1)
				goto bad;
			else if (tr->truncate)
			{
				while (tr->dst < tr->src)
				{
					c = set[tr->dst++];
					tr->code[c] = c | squeeze;
				}
				break;
			}
		}
		else
		{
			x = squeeze ? c : 0;
			tr->code[c] = x | squeeze;
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
			}
		if (x < -1)
			goto bad;
	}
	return tr;
 bad:
	error(2, "%s: invalid %s string", tr->base, typename[tr->type]);
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
	register int*		code = tr->code;
	register unsigned char*	inp = 0;
	register unsigned char*	outp = 0;
	register unsigned char*	inend;
	register unsigned char*	outend = 0;
	register ssize_t	nwrite = 0;
	unsigned char*		inbuff = 0;
	unsigned char*		outbuff = 0;

	while (nwrite != ncopy)
	{
		if (!(inbuff = (unsigned char*)sfreserve(ip, SF_UNBOUND, SF_LOCKR)))
		{
			if (sfvalue(ip))
			{
				error(2, ERROR_SYSTEM|2, "read error");
				return -1;
			}
			break;
		}
		c = sfvalue(ip);
		inend = (inp = inbuff) + c;

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

				if ((c = outp - outbuff) > 0)
				{
					if ((nwrite += c) == ncopy)
						break;
					sfwrite(op, outbuff, c);
				}

				/*
				 * get write buffer space
				 */

				if (!(outbuff = (unsigned char*)sfreserve(op, (ncopy < 0) ? SF_UNBOUND : (ncopy - nwrite), SF_LOCKR)))
					break;
				outend = (outp = outbuff) + sfvalue(op);
			}
			c = code[*inp++];
			if (!(c & DELBIT) && c != oldc)
			{
				*outp++ = c;
				oldc = c | ONEBIT;
			}
		}
		sfread(ip, inbuff, inp - inbuff);
		inp = inbuff;
	}
	if (inbuff && (c = inp - inbuff) > 0)
		sfread(ip, inbuff, c);
	if (outbuff && (c = outp - outbuff) >= 0)
		sfwrite(op, outbuff, c);
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
	register int	flags = 0;
	Tr_t*		tr;

	cmdinit(argc, argv, context, ERROR_CATALOG, 0);
	flags = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'c':
			flags |= TR_COMPLEMENT;
			continue;
		case 'd':
			flags |= TR_DELETE;
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
	if (tr = tropen((unsigned char*)argv[0], (unsigned char*)argv[0] ? (unsigned char*)argv[1] : (unsigned char*)0, flags))
	{
		trcopy(tr, sfstdin, sfstdout, SF_UNBOUND);
		trclose(tr);
	}
	return error_info.errors != 0;
}
