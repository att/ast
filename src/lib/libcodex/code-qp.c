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
 * quoted printable coder
 */

#include <codex.h>
#include <ctype.h>
#include <ccode.h>

#define LINE		76
#define BUFFER		SF_BUFSIZE

typedef struct State_s
{
	Codex_t*	codex;

	unsigned char*	bp;
	unsigned char*	be;

	unsigned char*	pp;
	unsigned char	prv[5];

	short		xxx;
	short		xeh[UCHAR_MAX+1];

	int		col;

	unsigned char	buf[LINE + BUFFER + 1];
} State_t;

static const char	hex[] = "0123456789ABCDEFabcdef";

#define GETCHAR(p)	((p)->bp < (p)->be ? (int)*(p)->bp++ : fill(p))
#define PUTCHAR(p,c)	((p)->bp < (p)->be ? (int)(*(p)->bp++=(c)) : flush(p,c))

static int
fill(State_t* state)
{
	ssize_t	r;

	state->bp = state->buf + LINE;
	if ((r = sfrd(state->codex->sp, state->bp, BUFFER, &state->codex->sfdisc)) <= 0)
	{
		state->be = state->bp;
		return EOF;
	}
	state->be = state->bp + r;
	return *state->bp++;
}

static int
flush(register State_t* state, int c)
{
	size_t	n;

	if (c < 0 && state->col)
	{
		state->col = 0;
		PUTCHAR(state, '=');
		PUTCHAR(state, '\n');
	}
	if (state->bp && (n = state->bp - state->buf) && sfwr(state->codex->sp, state->buf, n, &state->codex->sfdisc) != n)
		return EOF;
	state->be = (state->bp = state->buf) + sizeof(state->buf);
	if (c >= 0)
		*state->bp++ = c;
	return 0;
}

static int
qp_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	register int		i;

	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	if (flags & CODEX_DECODE)
	{
		for (i = -1; i < elementsof(state->xeh); i++)
			state->xeh[i] = -1;
		for (i = 0; i < elementsof(hex) - 1; i++)
			state->xeh[hex[i]] = i >= 16 ? (i - 6) : i;
	}
	p->data = state;
	state->codex = p;
	return 0;
}

static ssize_t
qp_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register char*		s = (char*)buf;
	register char*		e = s + n;
	register char*		x;
	register int		c;
	register int		d;

	x = 0;
	while (s < e)
	{
		switch (c = GETCHAR(state))
		{
		case '=':
			if ((c = GETCHAR(state)) == '\n')
				continue;
			if ((d = state->xeh[c]) != EOF && (c = state->xeh[GETCHAR(state)]) != EOF)
			{
				c |= (d << 4);
				x = 0;
				break;
			}
			/*FALLTHROUGH*/
		case EOF:
			return s - (char*)buf;
		case '\n':
			if (x)
			{
				s = x;
				x = 0;
			}
			break;
		case ' ':
		case '\t':
		case '\r':
			if (!x)
				x = s;
			break;
		default:
			x = 0;
			break;
		}
		*s++ = c;
	}
	if (x)
		while (s > x && state->bp > state->buf)
			*--state->bp = *--s;
	return s - (char*)buf;
}

static ssize_t
qp_write(Sfio_t* sp, const void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register unsigned char*	s;
	register unsigned char*	e;
	register int		c;
	register int		col;

 again:
	if (state->pp)
	{
		s = state->prv;
		e = state->pp;
		state->col = 0;
	}
	else
	{
		s = (unsigned char*)buf;
		e = s + n;
		col = state->col;
	}
	for (;;)
	{
		if (s >= e)
		{
			if (state->pp)
			{
				state->pp = 0;
				state->col = col;
				goto again;
			}
			break;
		}
		c = *s++;
		if (!col++)
		{
			if (c == 'F')
			{
				if ((e - s) < 4)
				{
					s--;
					col--;
					state->pp = state->prv;
					for (c = 0; c < (e - s); ++c)
						*state->pp++ = s[c];
					break;
				}
				else if (s[0] == 'r' && s[1] == 'o' && s[2] == 'm' && s[3] == ' ')
					goto quote;
			}
			else if (c == '.')
			{
				if ((e - s) < 1)
				{
					s--;
					col--;
					state->pp = state->prv;
					*state->pp++ = c;
					break;
				}
				else if (s[0] == '\r' || s[0] == '\n')
					goto quote;
			}
		}
		if (c == '\n')
		{
			col = 0;
			PUTCHAR(state, c);
			continue;
		}
		else if (col >= (LINE - 4))
		{
			col = 0;
			PUTCHAR(state, '=');
			PUTCHAR(state, '\n');
		}
		if (c == ' ' || c == '\t')
		{
			if ((e - s) < 1)
			{
				s--;
				col--;
				state->pp = state->prv;
				*state->pp++ = c;
				break;
			}
			else if (s[0] == '\r' || s[0] == '\n')
				goto quote;
			else
			{
				if (c == '\t')
					col |= 7;
				PUTCHAR(state, c);
				continue;
			}
		}
#if CC_NATIVE == CC_ASCII
		else if (c >= 0x21 && c <= 0x7e && c != '=')
#else
		else if (isprint(c) && !iscntrl(c) && c != '=')
#endif
		{
			PUTCHAR(state, c);
			continue;
		}
	quote:
		col += 2;
		PUTCHAR(state, '=');
		PUTCHAR(state, hex[(c >> 4) & 0xF]);
		PUTCHAR(state, hex[c & 0xF]);
	}
	state->col = col;
	return n;
}

static int
qp_sync(Codex_t* p)
{
	return flush((State_t*)p->data, -1);
}

Codexmeth_t	codex_qp =
{
	"qp",
	"quoted printable encoding.",
	"[-?\n@(#)$Id: codex-qp (AT&T Research) 1998-11-11 $\n]" USAGE_LICENSE,
	CODEX_DECODE|CODEX_ENCODE|CODEX_UU,
	0,
	0,
	qp_open,
	0,
	0,
	0,
	qp_read,
	qp_write,
	qp_sync,
	0,
	0,
	0,
	0,
	CODEXNEXT(qp)
};

CODEXLIB(qp)
