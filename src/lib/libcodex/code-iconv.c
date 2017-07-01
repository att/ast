/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
 * character codeset coder
 */

#include <codex.h>
#include <iconv.h>

typedef struct State_s
{
	iconv_t		cvt;

	char*		bp;

	char		buf[SF_BUFSIZE];
} State_t;

static int
cc_options(Codexmeth_t* meth, Sfio_t* sp)
{
	register iconv_list_t*	ic;
	register const char*	p;
	register int		c;

	for (ic = iconv_list(NiL); ic; ic = iconv_list(ic))
	{
		sfputc(sp, '[');
		sfputc(sp, '+');
		sfputc(sp, '\b');
		p = ic->match;
		if (*p == '(')
			p++;
		while (c = *p++)
		{
			if (c == ')' && !*p)
				break;
			if (c == '?' || c == ']')
				sfputc(sp, c);
			sfputc(sp, c);
		}
		sfputc(sp, '?');
		p = ic->desc;
		while (c = *p++)
		{
			if (c == ']')
				sfputc(sp, c);
			sfputc(sp, c);
		}
		sfputc(sp, ']');
	}
	return 0;
}

static int
cc_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	State_t*	state;
	const char*	src;
	const char*	dst;
	iconv_t		cvt;

	dst = (src = args[2]) ? args[3] : 0;
	if (flags & CODEX_DECODE)
	{
		if (!src)
		{
			if (p->disc->errorf)
				(*p->disc->errorf)(NiL, p->disc, 2, "%s: source codeset option must be specified", p->meth->name);
			return -1;
		}
	}
	else
	{
		if (!src)
		{
			if (p->disc->errorf)
				(*p->disc->errorf)(NiL, p->disc, 2, "%s: destination codeset option must be specified", p->meth->name);
			return -1;
		}
		if (!dst)
		{
			dst = src;
			src = 0;
		}
	}
	if ((cvt = iconv_open(dst, src)) == (iconv_t)(-1))
	{
		if (p->disc->errorf)
		{
			if ((cvt = iconv_open("utf-8", src)) == (iconv_t)(-1))
			{
				(*p->disc->errorf)(NiL, p->disc, 2, "%s: %s: unknown source codeset", p->meth->name, src);
				return -1;
			}
			iconv_close(cvt);
			if ((cvt = iconv_open(dst, "utf-8")) == (iconv_t)(-1))
			{
				(*p->disc->errorf)(NiL, p->disc, 2, "%s: %s: unknown destination codeset", p->meth->name, dst);
				return -1;
			}
			iconv_close(cvt);
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: cannot convert from %s to %s", p->meth->name, src, dst);
		}
		return -1;
	}
	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		iconv_close(cvt);
		return 0;
	}
	state->cvt = cvt;
	state->bp = state->buf;
	p->data = state;
	return 0;
}

static int
cc_close(Codex_t* p)
{
	State_t*	state = (State_t*)p->data;
	int		r;

	if (!state)
		r = -1;
	else
	{
		r = iconv_close(state->cvt);
		free(state);
	}
	return r;
}

static ssize_t
cc_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)((Codex_t*)disc)->data;
	char*			fb;
	char*			tb;
	size_t			fn;
	size_t			tn;
	ssize_t			r;

	fn = sizeof(state->buf) - (state->bp - state->buf);
	if (n < fn)
		fn = n;
	if ((r = sfrd(sp, state->bp, fn, disc)) <= 0)
		return (state->bp > state->buf) ? -1 : r;
	fb = state->buf;
	fn = r + (state->bp - state->buf);
	tb = buf;
	tn = n;
	n = 0;
	while (fn > 0 && tn > 0)
	{
		if ((r = iconv(state->cvt, &fb, &fn, &tb, &tn)) == -1)
		{
			if (!n)
				n = -1;
			break;
		}
		n += r;
	}
	if (fn && fb > state->buf)
	{
		tb = state->buf;
		while (fn--)
			*tb++ = *fb++;
		state->bp = tb;
	}
	return n;
}

static ssize_t
cc_write(Sfio_t* sp, const void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)((Codex_t*)disc)->data;
	char*			fb;
	char*			tb;
	size_t			fn;
	size_t			tn;
	size_t			r;

	fb = (char*)buf;
	fn = n;
	n = 0;
	while (fn > 0)
	{
		tb = (char*)state->buf;
		tn = sizeof(buf);
		if ((r = iconv(state->cvt, &fb, &fn, &tb, &tn)) == (size_t)(-1))
			return n ? n : -1;
		n += r;
		if (sfwr(sp, state->buf, r, disc) != r)
			return n ? n : -1;
	}
	return n;
}

static int
cc_sync(Codex_t* p)
{
	State_t*	state = (State_t*)p->data;

	(void)iconv(state->cvt, NiL, NiL, NiL, NiL);
	return 0;
}

Codexmeth_t	codex_iconv =
{
	"iconv",
	"iconv character codeset conversion. One or two character codeset"
	" options must be specified. Two options specify the source and"
	" destination codesets. One option specifies the decode source or"
	" encode destination codeset; the implied second codeset defaults"
	" to \bnative\b.",
	"[-?\n@(#)$Id: codex-iconv (AT&T Research) 2000-05-09 $\n]" USAGE_LICENSE,
	CODEX_DECODE|CODEX_ENCODE|CODEX_ICONV,
	cc_options,
	0,
	cc_open,
	cc_close,
	cc_sync,
	cc_sync,
	cc_read,
	cc_write,
	cc_sync,
	0,
	0,
	0,
	0,
	CODEXNEXT(iconv)
};

CODEXLIB(iconv)
