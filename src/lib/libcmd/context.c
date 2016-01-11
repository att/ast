/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2014 AT&T Intellectual Property          *
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
#define CONTEXT_BLOCK		(1024*1024)
#define CONTEXT_LINE		(4*1024)

#define _CONTEXT_LINE_PRIVATE_		\
	unsigned char	show;		\
	char*		drop;		\
	unsigned char	span;

#define _CONTEXT_PRIVATE_		\
	Sfio_t*		ip;		\
	Context_list_f	listf;		\
	uintmax_t	next;		\
	char*		buf;		\
	char*		cur;		\
	char*		end;		\
	unsigned int	before;		\
	unsigned int	after;		\
	unsigned int	total;		\
	unsigned int	curline;	\
	uintmax_t	lineno;		\
	Context_line_t	line[1];

#include "context.h"

Context_t*
context_open(Sfio_t* ip, size_t before, size_t after, Context_list_f listf, void* handle)
{
	Context_t*	cp;
	int		j;

	if (!(cp = newof(0, Context_t, 1, (before + after) * sizeof(Context_line_t))))
		return 0;
	cp->ip = ip;
	cp->before = before;
	cp->after = after;
	cp->total = before + after + 1;
	cp->curline = cp->total - 1;
	cp->listf = listf;
	cp->handle = handle;
	cp->next = 0;
	return cp;
}

Context_line_t*
context_line(Context_t* cp)
{
	Context_line_t*	lp;
	char*		s;
	char*		t;
	char*		e;
	size_t		n;
	size_t		m;
	size_t		o;
	ssize_t		r;

	lp = &cp->line[cp->curline];
	if (lp->show)
	{
		if (cp->listf(lp, lp->show == ':', cp->before ? 0 : lp->line != cp->next && cp->next, cp->handle))
			return 0;
		cp->next = lp->line + 1;
	}
	if (++cp->curline >= cp->total)
		cp->curline = 0;
	if ((n = cp->curline + cp->after) >= cp->total)
		n -= cp->total;
	cp->line[n].show = 0;
	lp = &cp->line[cp->curline];
	if (lp->drop)
	{
		free(lp->drop);
		lp->drop = 0;
	}
	if (cp->cur >= cp->end)
	{
		lp->drop = cp->buf;
		if (!(cp->buf = oldof(0, char, CONTEXT_BLOCK, 0)))
			return 0;
		cp->cur = cp->buf;
		if ((r = sfread(cp->ip, cp->buf, CONTEXT_BLOCK)) <= 0)
			return 0;
		cp->end = cp->buf + r;
	}
	if (lp->span)
	{
		lp->span = 0;
		free(lp->data);
	}
	n = cp->end - cp->cur;
	if (s = memchr(cp->cur, '\n', n))
	{
		lp->data = cp->cur;
		n = s - cp->cur + 1;
		cp->cur += n;
		lp->size = n;
	}
	else
	{
		m = roundof(n, CONTEXT_LINE);
		if (!(t = oldof(0, char, m, 0)))
			return 0;
		lp->data = t;
		lp->span = 1;
		e = t + m;
		memcpy(t, cp->cur, n);
		t += n;
		lp->drop = cp->buf;
		if (!(cp->buf = oldof(0, char, CONTEXT_BLOCK, 0)))
			return 0;
		do
		{
			if ((r = sfread(cp->ip, cp->buf, CONTEXT_BLOCK)) <= 0)
				return 0;
			cp->end = cp->buf + r;
			n = (s = memchr(cp->buf, '\n', r)) ? (s - cp->buf + 1) : r;
			if (n > (e - t))
			{
				r = t - lp->data;
				m = r + (s - cp->buf);
				m = roundof(m, CONTEXT_LINE);
				if (!(lp->data = oldof(lp->data, char, m, 0)))
					return 0;
				t = lp->data + r;
			}
			memcpy(t, cp->buf, n);
			t += n;
			cp->cur = cp->buf + n;
		} while (!s);
		lp->size = t - lp->data;
	}
	lp->line = ++cp->lineno;
	return lp;
}

int
context_show(Context_t* cp)
{
	int	i;
	int	j;
	int	k;

	j = cp->curline;
	for (i = 0; i < cp->after; i++)
	{
		if (++j >= cp->total)
			j = 0;
		cp->line[j].show = '-';
	}
	for (i = 0; i < cp->before; i++)
	{
		if (++j >= cp->total)
			j = 0;
		if (!cp->line[j].show && cp->line[j].line)
		{
			cp->line[j].show = '-';
			if (cp->listf(&cp->line[j], 0, !i && cp->line[j].line != cp->next && cp->next, cp->handle))
				return 0;
		}
	}
	cp->line[cp->curline].show = ':';
	return 0;
}

int
context_close(Context_t* cp)
{
	int	i;
	int	j;

	for (j = 0; j < cp->total; j++)
	{
		if (cp->line[j].drop)
			free(cp->line[j].drop);
		if (cp->line[j].span)
			free(cp->line[j].data);
	}
	free(cp);
	return 0;
}
