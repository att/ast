/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2012 AT&T Intellectual Property          *
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
 * uuencode/uudecode methods
 */

static char id[] = "\n@(#)$Id: uulib (AT&T Research) 2003-01-07 $\0\n";

static const char lib[] = "libuu:uu";

#include "uulib.h"

#include <ctype.h>
#include <error.h>
#include <ls.h>
#include <modex.h>
#include <sfdisc.h>

#define UU_BEGIN	"begin"
#define UU_BEGIN_LEN	(sizeof(UU_BEGIN)-1)

#define UUIN		3
#define UUOUT		4
#define UUCHUNK		15
#define UULINE		76

#define UU_END	(UCHAR_MAX)
#define UU_IGN	(UCHAR_MAX-1)
#define UU_PAD	(UCHAR_MAX-2)

typedef struct
{
	const char*	name;
	unsigned long	flag;
} Option_t;

static const Option_t	options[] =
{
	"text",		UU_TEXT,
};

/*
 * initialize the uu map from dp
 */

static unsigned char*
uu_map(register Uudata_t* dp, char* map)
{
	register int		c;
	register char*		p;
	register unsigned char*	m;
	int			x;

	x = (dp->flags & UU_LENGTH) ? 0 : UU_IGN;
	p = map;
	memset(p, x, UCHAR_MAX + 2);
	if (x)
	{
		p++;
		p[dp->pad] = UU_PAD;
		p[EOF] = UU_END;
	}
	for (m = (unsigned char*)dp->map; c = *m; m++)
		p[c] =  m - (unsigned char*)dp->map;
	return (unsigned char*)p;
}

/*
 * grab uu header from input
 */

static int
uu_header(register Uu_t* uu)
{
	register char*	s;
	register int	c;
	int		n;
	int		k;
	Uumeth_t*	meth;
	char*		t;

	c = *UU_BEGIN;
	for (;;)
	{
		if (!(s = sfgetr(uu->ip, '\n', 1)))
		{
			if (uu->disc->errorf)
				(*uu->disc->errorf)(uu, uu->disc, 2, "unknown encoding");
			break;
		}
		if (*s == c && !strncasecmp(s, UU_BEGIN, UU_BEGIN_LEN) && (meth = uumeth(s)))
		{
			s += UU_BEGIN_LEN + strlen(meth->id);
			while (*s == '-')
			{
				for (t = ++s; isalnum(*s); s++);
				k = s - t;
				for (n = 0; n < elementsof(options); n++)
					if (strlen(options[n].name) == k && !strncasecmp(options[n].name, t, k))
					{
						uu->flags |= options[n].flag;
						break;
					}
			}
			if (*s++ == ' ')
			{
				uu->mode = strperm(s, &t, 0) & ~(S_IXUSR|S_IXGRP|S_IXOTH);
				if (*t++ == ' ')
				{
					if (!uu->path)
					{
						uu->path = strdup(t);
						uu->flags |= UU_FREEPATH;
					}
					if (meth->name != uu->meth.name)
					{
						if (!(((Uudata_t*)uu->meth.data)->flags & UU_DEFAULT) && !streq(meth->id, uu->meth.id) && uu->disc->errorf)
							(*uu->disc->errorf)(uu, uu->disc, 1, "switching to %s encoding", meth->name);
						uu->meth = *meth;
					}
					return 0;
				}
			}
		}
	}
	return -1;
}

/*
 * uu encode input to output
 */

static int
uu_encode(register Uu_t* uu)
{
	register char*		e;
	register char*		p;
	register unsigned char*	m;
	register int		c;
	register int		c1;
	register int		c2;
	register int		c3;
	register unsigned long	b;
	int			length;
	int			nl;
	int			pad;
	int			text;
	Uudata_t*		dp;
	struct stat		st;
	char			buf[UUOUT * (UUCHUNK + 1)];

	dp = (Uudata_t*)uu->meth.data;
	m = (unsigned char*)dp->map;
	length = !!(dp->flags & UU_LENGTH);
	text = !!(uu->flags & UU_TEXT);
	pad = dp->pad;
	if (uu->flags & UU_HEADER)
	{
		if (fstat(sffileno(uu->ip), &st) || !(c = modex(st.st_mode) & 0777))
			c = 0644;
		sfprintf(uu->op, "%s%s%s %03o %s\n", UU_BEGIN, uu->meth.id, text ? "-text" : "", c, uu->path ? uu->path : "-");
	}
	if (length)
		*buf = m[UUIN * UUCHUNK];
	p = buf + length;
	e = p + UUOUT * UUCHUNK;
	nl = 0;
	for (;;)
	{
		do {
			if (nl)
			{
				nl = 0;
				c1 = '\n';
				goto get_2;
			}
			if ((c1 = sfgetc(uu->ip)) == EOF)
			{
				if (length)
					*buf = m[((p - buf - length) / UUOUT) * UUIN];
				goto eof;
			}
			if (text && c1 == '\n')
			{
				c1 = '\r';
				c2 = '\n';
				goto get_3;
			}
 get_2:
			if ((c2 = sfgetc(uu->ip)) == EOF)
			{
				if (length)
					*buf = m[((p - buf - length) / UUOUT) * UUIN + 1];
				c2 = dp->fill;
				c3 = dp->fill;
				b = (c1 << 16) | (c2 << 8) | c3;
				*p++ = m[b >> 18];
				*p++ = m[(b >> 12) & 077];
				*p++ = pad ? pad : m[(b >> 6) & 077];
				*p++ = pad ? pad : m[b & 077];
				goto eof;
			}
			if (text && c2 == '\n')
			{
				c2 = '\r';
				c3 = '\n';
				goto put_123;
			}
 get_3:
			if ((c3 = sfgetc(uu->ip)) == EOF)
			{
				if (length)
					*buf = m[((p - buf - length) / UUOUT) * UUIN + 2];
				c3 = dp->fill;
				b = (c1 << 16) | (c2 << 8) | c3;
				*p++ = m[b >> 18];
				*p++ = m[(b >> 12) & 077];
				*p++ = m[(b >> 6) & 077];
				*p++ = pad ? pad : m[b & 077];
				goto eof;
			}
			if (text && c3 == '\n')
			{
				nl = 1;
				c3 = '\r';
			}
 put_123:
			b = (c1 << 16) | (c2 << 8) | c3;
			*p++ = m[b >> 18];
			*p++ = m[(b >> 12) & 077];
			*p++ = m[(b >> 6) & 077];
			*p++ = m[b & 077];
		} while (p < e);
		*p++ = '\n';
		sfwrite(uu->op, buf, p - buf);
		p = buf + length;
	}
 eof:
	if (p > buf + length)
	{
		*p++ = '\n';
		sfwrite(uu->op, buf, p - buf);
	}
	if (length)
		sfprintf(uu->op, "%c\n", m[0]);
	if (uu->flags & UU_HEADER)
		sfputr(uu->op, dp->end, '\n');
	return 0;
}

/*
 * uu decode input to output
 */

static int
uu_decode(register Uu_t* uu)
{
	register Uudata_t*	dp;
	register char*		s;
	register char*		e;
	register char*		p;
	register unsigned char*	m;
	register int		c;
	register unsigned long	n;
	int			text;
	int			tl;
	int			x;
	char*			t;
	char			buf[UUIN * UUCHUNK + 1];
	char			map[UCHAR_MAX + 2];

	dp = (Uudata_t*)uu->meth.data;
	if (uu->path && (uu->flags & UU_CLOSEOUT) && (dp->flags & uu->flags & UU_HEADER) && chmod(uu->path, uu->mode) && uu->disc->errorf)
		(*uu->disc->errorf)(uu, uu->disc, ERROR_SYSTEM|2, "%s: cannot change mode to %s", uu->path, fmtperm(uu->mode));
	text = !!(uu->flags & UU_TEXT);
	m = uu_map(dp, map);
	if (dp->flags & UU_LENGTH)
	{
		t = (char*)dp->end;
		tl = strlen(t) + 1;
		while (((s = sfgetr(uu->ip, '\n', 0)) || (s = sfgetr(uu->ip, '\n', -1))) && ((n = sfvalue(uu->ip)) != tl || !strneq(s, t, tl - 1)))
			if (c = m[*((unsigned char*)s++)])
			{
				if (c > sizeof(buf))
				{
					if (uu->disc->errorf)
						(*uu->disc->errorf)(uu, uu->disc, 2, "input is not %s encoded", uu->meth.name);
					return -1;
				}
				p = buf;
				e = s + (c + UUIN - 1) / UUIN * UUOUT;
				while (s < e)
				{
					n = m[*((unsigned char*)s++)];
					n = (n << 6) | ((s < e) ? m[*((unsigned char*)s++)] : 0);
					n = (n << 6) | ((s < e) ? m[*((unsigned char*)s++)] : 0);
					n = (n << 6) | ((s < e) ? m[*((unsigned char*)s++)] : 0);
					if (text)
					{
						if ((x = (n >> 16) & 0xFF) == '\r')
							c--;
						else
							*p++ = x;
						if ((x = (n >> 8) & 0xFF) == '\r')
							c--;
						else
							*p++ = x;
						if ((x = n & 0xFF) == '\r')
							c--;
						else
							*p++ = x;
					}
					else
					{
						*p++ = (n >> 16);
						*p++ = (n >> 8);
						*p++ = n;
					}
				}
				sfwrite(uu->op, buf, c);
			}
		if (!s && (uu->flags & UU_HEADER) && uu->disc->errorf)
			(*uu->disc->errorf)(uu, uu->disc, 1, "end sequence `%s' omitted", t);
	}
	else
	{
		for (;;)
		{
			while ((c = m[sfgetc(uu->ip)]) >= 64)
				if (c != UU_IGN)
					goto pad;
			n = c;
			while ((c = m[sfgetc(uu->ip)]) >= 64)
				if (c != UU_IGN)
				{
					if (uu->disc->errorf)
						(*uu->disc->errorf)(uu, uu->disc, 1, "%c: extra input character ignored", c);
					goto pad;
				}
			n = (n << 6) | c;
			while ((c = m[sfgetc(uu->ip)]) >= 64)
				if (c != UU_IGN)
				{
					if (text)
					{
						if ((x = (n >> 4) & 0xFF) != '\r')
							sfputc(uu->op, x);
					}
					else sfputc(uu->op, n >> 4);
					goto pad;
				}
			n = (n << 6) | c;
			while ((c = m[sfgetc(uu->ip)]) >= 64)
				if (c != UU_IGN)
				{
					if (text)
					{
						if ((x = (n >> 10) & 0xFF) != '\r')
							sfputc(uu->op, x);
						if ((x = (n >> 2) & 0xFF) != '\r')
							sfputc(uu->op, x);
					}
					else
					{
						sfputc(uu->op, n >> 10);
						sfputc(uu->op, n >> 2);
					}
					goto pad;
				}
			n = (n << 6) | c;
			if (text)
			{
				if ((x = (n >> 16) & 0xFF) != '\r')
					sfputc(uu->op, x);
				if ((x = (n >> 8) & 0xFF) != '\r')
					sfputc(uu->op, x);
				if ((x = n & 0xFF) != '\r')
					sfputc(uu->op, x);
			}
			else
			{
				sfputc(uu->op, (n >> 16));
				sfputc(uu->op, (n >> 8));
				sfputc(uu->op, (n));
			}
		}
	pad:
		n = c == UU_PAD;
		while ((c = sfgetc(uu->ip)) != EOF)
			if (c == dp->pad && ++n >= 4)
				break;
		if (n < 4 && (uu->flags & UU_HEADER) && uu->disc->errorf)
			(*uu->disc->errorf)(uu, uu->disc, 1, "input end sequence `%s' omitted", dp->end);
	}
	return 0;
}

static const char	hex[] = "0123456789ABCDEFabcdef";

/*
 * quoted-printable encode input to output
 */

static int
qp_encode(register Uu_t* uu)
{
	register unsigned char*	s;
	register unsigned char*	e;
	register char*		b;
	register char*		x;
	register int		c;
	char			buf[UULINE + 1];

	b = buf;
	x = b + UULINE - 4;
	while ((s = (unsigned char*)sfgetr(uu->ip, '\n', 0)) || (s = (unsigned char*)sfgetr(uu->ip, '\n', -1)))
	{
		e = s + sfvalue(uu->ip);
		switch (*s)
		{
		case 'F':
			if ((e - s) >= 5 && strneq((char*)s, "From ", 5))
			{
				c = *s++;
				goto quote;
			}
			break;
		case '.':
			if ((e - s) == 2)
			{
				c = *s++;
				goto quote;
			}
			break;
		}
		while (s < e)
		{
			if ((c = *s++) == '\n')
			{
				*b++ = c;
				sfwrite(uu->op, buf, b - buf);
				b = buf;
				break;
			}
			if (b >= x)
			{
				*b++ = '=';
				*b++ = '\n';
				sfwrite(uu->op, buf, b - buf);
				b = buf;
			}
			if (c == ' ' || c == '\t')
			{
				if (s < e && *s != '\n')
				{
					*b++ = c;
					continue;
				}
			}
			else if (isprint(c) && !iscntrl(c) && c != '=')
			{
				*b++ = c;
				continue;
			}
		quote:
			*b++ = '=';
			*b++ = hex[(c >> 4) & 0xF];
			*b++ = hex[c & 0xF];
		}
	}
	if (b > buf)
	{
		*b++ = '=';
		*b++ = '\n';
		sfwrite(uu->op, buf, b - buf);
	}
	return 0;
}

/*
 * quoted-printable decode input to output
 */

static int
qp_decode(register Uu_t* uu)
{
	register unsigned char*	s;
	register unsigned char*	b;
	register unsigned char*	x;
	register int		c;
	register int		d;

	short			xeh[UCHAR_MAX + 1];

	for (c = 0; c < elementsof(xeh); c++)
		xeh[c] = -1;
	for (c = 0; c < elementsof(hex) - 1; c++)
		xeh[hex[c]] = c >= 16 ? (c - 6) : c;
	while (s = (unsigned char*)sfgetr(uu->ip, '\n', 1))
	{
		if (((b = s + sfvalue(uu->ip)) > s) && !*--b)
		{
			while (b > s && ((c = *(b - 1)) == ' ' || c == '\t'))
				b--;
			*b = 0;
		}
		x = b = s;
		for (;;)
		{
			switch (c = *s++)
			{
			case 0:
				*b++ = '\n';
				break;
			case '=':
				if ((c = xeh[*s++]) < 0 || (d = xeh[*s++]) < 0)
					break;
				*b++ = (c << 4) | d;
				continue;
			default:
				*b++ = c;
				continue;
			}
			break;
		}
		sfwrite(uu->op, x, b - x);
	}
	return 0;
}

/*
 * binhex based on
 *
 *	xbin Version 2.3 09/30/85
 *	Dave Johnson, Brown University Computer Science
 */

#define BX_REPEAT	0x90

#define BX_OLD		(UU_METHOD<<0)

typedef struct
{
	int		col;
	int		eof;
	int		last;
	int		repeat;
	off_t		size;
	unsigned long	crc;
	unsigned char*	qp;
	unsigned char*	qe;
	unsigned char	qbuf[3];
	char		map[UCHAR_MAX + 2];
} Bx_t;

/*
 * add c to the binhex Q format crc
 */

static int
bx_q_crc(register Bx_t* bx, int c)
{
	register int		i = 8;
	register unsigned int	k = c;
	register unsigned long	crc = bx->crc;

	while (i--)
	{
		k <<= 1;
		if ((crc <<= 1) & 0x10000)
			crc = (crc & 0xFFFF) ^ 0x1021;
		crc ^= k >> 8;
		k &= 0xFF;
	}
	bx->crc = crc;
	return c;
}

/*
 * return next binhex Q format char
 */

static int
bx_q_getc(register Uu_t* uu, register Bx_t* bx)
{
	register int		c;
	register unsigned char*	ip;
	register unsigned char*	ie;
	register unsigned char*	m;
	int			x;
	unsigned long		crc;
	unsigned char		ibuf[4];

	if (bx->repeat > 0)
	{
		bx->repeat--;
		return bx_q_crc(bx, bx->last);
	}
	if (bx->qp >= bx->qe)
	{
		if (bx->eof)
			return EOF;
		bx->qp = bx->qbuf;
		m = (unsigned char*)bx->map + 1;
		ie = (ip = ibuf) + sizeof(ibuf);
		while (ip < ie)
		{
			while ((c = m[sfgetc(uu->ip)]) >= 64)
				if (c != UU_IGN)
				{
					bx->eof = 1;
					bx->qe = bx->qbuf;
					if ((c = (ip - ibuf) - 1) <= 0)
						return EOF;
					bx->qe += c;
					break;
				}
			*ip++ = c;
		}
		ip = ibuf;
		ie = bx->qp;
		ie[0] = (ip[0] << 2) | (ip[1] >> 4);
		ie[1] = (ip[1] << 4) | (ip[2] >> 2);
		ie[2] = (ip[2] << 6) | (ip[3]     );
	}
	if ((c = *bx->qp++) == BX_REPEAT && !bx->repeat)
	{
		c = bx->last;
		crc = bx->crc;
		bx->repeat = -1;
		x = bx_q_getc(uu, bx);
		bx->crc = crc;
		switch (x)
		{
		case EOF:
			return EOF;
		case 0:
			bx->repeat = 0;
			c = BX_REPEAT;
			break;
		case 1:
			bx->repeat = 0;
			break;
		default:
			bx->repeat = x - 2;
			break;
		}
	}
	return bx->last = bx_q_crc(bx, c);
}

/*
 * return binhex Q format n byte int
 */

static long
bx_q_getn(register Uu_t* uu, register Bx_t* bx, register int n)
{
	register long	v = 0;

	while (n--)
		v = (v << 8) | bx_q_getc(uu, bx);
	return v;
}

/*
 * return binhex Q format buffer of size n
 */

static ssize_t
bx_q_gets(register Uu_t* uu, register Bx_t* bx, register char* s, size_t n)
{
	register int	c;
	register char*	e;

	e = s + n;
	while (s < e)
	{
		if ((c = bx_q_getc(uu, bx)) == EOF)
			return -1;
		*s++ = c;
	}
	return n;
}

/*
 * low level for bx_q_putc()
 */

static int
bx_q_put(register Uu_t* uu, register Bx_t* bx, register int c)
{
	register unsigned char*	p;
	register unsigned char*	m;

	*bx->qp++ = c;
	if (bx->qp >= bx->qe)
	{
		m = (unsigned char*)((Uudata_t*)uu->meth.data)->map;
		p = bx->qp = bx->qbuf;
		c = (p[0] << 16) | (p[1] << 8) | p[2];
		sfputc(uu->op, m[(c >> 18) & 0x3f]);
		sfputc(uu->op, m[(c >> 12) & 0x3f]);
		sfputc(uu->op, m[(c >>  6) & 0x3f]);
		sfputc(uu->op, m[(c      ) & 0x3f]);
		if ((bx->col += 4) >= 63)
		{
			bx->col = 0;
			sfputc(uu->op, '\n');
		}
	}
	return 0;
}

/*
 * output binhex Q format char
 */

static int
bx_q_putc(register Uu_t* uu, register Bx_t* bx, register int c)
{
	if (c == bx->last)
	{
		if (!bx->repeat++)
		{
			bx_q_put(uu, bx, c);
			bx_q_put(uu, bx, BX_REPEAT);
		}
		bx_q_crc(bx, c);
		if (bx->repeat >= 0xFF)
		{
			bx_q_put(uu, bx, bx->repeat);
			bx->repeat = 0;
			bx->last = -1;
		}
	}
	else
	{
		if (bx->repeat)
		{
			bx_q_put(uu, bx, bx->repeat);
			bx->repeat = 0;
			bx->last = -1;
		}
		if (c == BX_REPEAT)
		{
			bx_q_put(uu, bx, c);
			bx_q_put(uu, bx, 0);
			bx->last = -1;
		}
		else if (c == '\n' && (uu->flags & UU_TEXT))
		{
			bx_q_put(uu, bx, '\r');
			bx_q_crc(bx, '\r');
			bx_q_put(uu, bx, '\n');
			bx->last = -1;
		}
		else
		{
			bx_q_put(uu, bx, c);
			bx->last = c;
		}
		bx_q_crc(bx, c);
	}
	return 0;
}

/*
 * output binhex Q format n byte int
 */

static int
bx_q_putn(register Uu_t* uu, register Bx_t* bx, register unsigned long v, int n)
{
	switch (n)
	{
	case 4:	bx_q_putc(uu, bx, (v >> 24) & 0xFF);
	case 3:	bx_q_putc(uu, bx, (v >> 16) & 0xFF);
	case 2:	bx_q_putc(uu, bx, (v >>  8) & 0xFF);
	case 1:	bx_q_putc(uu, bx, (v >>  0) & 0xFF);
	}
	return 0;
}

/*
 * grab binhex header from input
 */

static int
bx_header(register Uu_t* uu)
{
	register Bx_t*	bx = (Bx_t*)(uu + 1);
	Uudata_t*	dp = (Uudata_t*)uu->meth.data;
	register int	c;
	register int	bol;
	register char*	s;
	register char*	m;
	unsigned long	crc;
	unsigned long	crx;
	char		buf[UCHAR_MAX + 2];

	if (uu->flags & UU_HEADER)
		do
		{
			if (!(s = sfgetr(uu->ip, '\n', 0)))
			{
				if (uu->disc->errorf)
					(*uu->disc->errorf)(uu, uu->disc, 2, "unknown encoding");
				return -1;
			}
		} while (*s != '(' || strncmp(s, "(This file", 10));
	bol = 1;
	for (;;)
	{
		switch (c = sfgetc(uu->ip))
		{
		case EOF:
			return -1;
		case '\n':
		case '\r':
			bol = 1;
			break;
		case ':':
			if (bol)
			{
				/*
				 * Q format
				 *
				 *	1 n name length
				 *	n s name
				 *	4 s type
				 *	4 s author
				 *	2 n flags
				 *	4 n data size
				 *	4 n resource size
				 *	2 n header checksum
				 */

				memset(s = bx->map, UU_END, sizeof(bx->map));
				for (s++, m = (char*)dp->map; c = *m; m++)
					s[c] =  m - (char*)dp->map;
				s['\n'] = UU_IGN;
				s['\r'] = UU_IGN;
				bx->qp = bx->qe = bx->qbuf + sizeof(bx->qbuf);
				if ((c = bx_q_getc(uu, bx)) == EOF)
					return -1;
				if (bx_q_gets(uu, bx, buf, c + 1) < 0)
					return -1;
				if (!uu->path)
				{
					uu->path = strdup(buf);
					uu->flags |= UU_FREEPATH;
				}
				if (bx_q_gets(uu, bx, buf, 4) < 0)
					return -1;
				if (bx_q_gets(uu, bx, buf, 4) < 0)
					return -1;
				bx_q_getn(uu, bx, 2);
				bx->size = bx_q_getn(uu, bx, 4);
				bx_q_getn(uu, bx, 4);
				bx_q_crc(bx, 0);
				bx_q_crc(bx, 0);
				crc = bx->crc;
				crx = bx_q_getn(uu, bx, 2);
				if (crc != crx)
				{
					if (uu->disc->errorf)
						(*uu->disc->errorf)(uu, uu->disc, 2, "%s format header checksum mismatch", uu->meth.name);
					return -1;
				}
				return 0;
			}
			break;
		case '#':
			if (bol)
			{
				/*
				 * old format
				 */

				sfungetc(uu->ip, c);
				uu->flags |= BX_OLD;

				/*
				 * #<TYPE><AUTH>$<flag>
				 */

				if (!sfgetr(uu->ip, '\n', 0))
					return -1;
				return 0;
			}
			break;
		default:
			bol = 0;
			break;
		}
	}
}

/*
 * old binhex line decode
 */

static int
bx_o_decode(register Uu_t* uu, Bx_t* bx, char* buf, register size_t n)
{
	register int		c;
	register int		d;
	register unsigned long	crc = bx->crc;
	register unsigned char*	m = (unsigned char*)bx->map;
	register char*		t = (char*)hex;
	register unsigned char*	s = (unsigned char*)buf;

	memset(m, UU_END, sizeof(bx->map));
	for (c = 0; c < elementsof(hex); c++)
		m[t[c]] = c;
	n <= 2;
	while (n--)
	{
		if ((c = m[*s++]) == UU_END || (d = m[*s++]) == UU_END)
			return -1;
		crc += c = (c << 4) | d;
		sfputc(uu->op, c);
	}
	bx->crc = crc;
	return 0;
}

/*
 * old binhex compressed line decode
 */

#define BX_O_MASK(c)	(((c)-0x20)&0x3F)
#define BX_O_CRC(s,c)	((s=(s+c)&0xFF),(s=((s<<3)&0xFF)|(s>>13)))

static int
bx_c_decode(register Uu_t* uu, Bx_t* bx, register char* s, size_t n)
{
	register int		c;
	register int		oc;
	register unsigned long	crc;
	char*			e;
	int			ic;
	char			buf[SF_BUFSIZE];

	crc = bx->crc;
	oc = (BX_O_MASK(s[0]) << 2) | (BX_O_MASK(s[1]) >> 4);
	ic = ((oc / 3) + 1) * 4;
	if (ic > SF_BUFSIZE)
		ic = SF_BUFSIZE;
	if (n > SF_BUFSIZE)
		n = SF_BUFSIZE;
	memcpy(buf, s, n);
	s = buf + n;
	e = buf + ic;
	while (s < e)
		*s++ = ' ';
	s = buf;
	while ((oc -= 3) >= 0)
	{
		c = (BX_O_MASK(s[0]) << 2) | (BX_O_MASK(s[1]) >> 4);
		BX_O_CRC(crc, c);
		sfputc(uu->op, c);
		c = (BX_O_MASK(s[1]) << 4) | (BX_O_MASK(s[2]) >> 2);
		BX_O_CRC(crc, c);
		sfputc(uu->op, c);
		c = (BX_O_MASK(s[2]) << 6) | (BX_O_MASK(s[3])     );
		BX_O_CRC(crc, c);
		sfputc(uu->op, c);
		s += 4;
	}
	bx->crc = crc;
	return 0;
}

/*
 * binhex decode input to output
 */

static int
bx_decode(register Uu_t* uu)
{
	register Bx_t*	bx = (Bx_t*)(uu + 1);
	register off_t	n;
	register int	c;
	register char*	s;
	unsigned long	crc;
	int		(*decode)(Uu_t*, Bx_t*, char*, size_t);

	crc = 0x10000;
	bx->crc = 0;
	if (uu->flags & BX_OLD)
	{
		decode = bx_o_decode;
		c = 0;
		while (s = sfgetr(uu->ip, '\n', 0))
			if (*s++ == '*' && *s++ == '*' && *s++ == '*')
			{
				switch (*s)
				{
				case 'C':
					switch (s[1])
					{
					case 'O':
						if (c || strncmp(s, "COMPRESSED", 10))
							continue;
						decode = bx_c_decode;
						continue;
					case 'H':
						if (decode == bx_c_decode || strncmp(s, "CHECKSUM:", 9))
							continue;
						crc = bx->crc & 0xFF;
						bx->crc = strtoul(s + 9, NiL, 16) & 0xFF;
						break;
					case 'R':
						if (decode == bx_o_decode || strncmp(s, "CRC:", 4))
							continue;
						crc = bx->crc & 0xFFFF;
						bx->crc = strtoul(s + 4, NiL, 16) & 0xFFFF;
						break;
					default:
						continue;
					}
					break;
				case 'D':
					if (strncmp(s, "DATA", 4))
						continue;
					while (s = sfgetr(uu->ip, '\n', 0))
					{
						if (strneq(s, "***END", 6))
							break;
						if ((*decode)(uu, bx, s, sfvalue(uu->ip) - 1) < 0)
							return -1;
					}
					break;
				case 'R':
					if (c || strncmp(s, "RESOURCE", 8))
						continue;
					c = 1;
					continue;
				default:
					continue;
				}
				break;
			}
	}
	else
	{
		if ((n = bx->size) > 0)
			while (n--)
			{
				if ((c = bx_q_getc(uu, bx)) == EOF)
					return -1;
				sfputc(uu->op, c);
			}

		/*
		 * check the header crc
		 */

		bx_q_crc(bx, 0);
		bx_q_crc(bx, 0);
		crc = bx->crc;
		bx->crc = bx_q_getn(uu, bx, 2);
	}
	if (crc != bx->crc)
	{
		if (uu->disc->errorf)
		{
			if (crc == 0x10000)
				(*uu->disc->errorf)(uu, uu->disc, 2, "%s format checksum missing", uu->meth.name);
			else
				(*uu->disc->errorf)(uu, uu->disc, 2, "%s format data checksum mismatch", uu->meth.name);
		}
		return -1;
	}
	return 0;
}

/*
 * binhex encode input to output
 */

static int
bx_encode(register Uu_t* uu)
{
	register Bx_t*		bx = (Bx_t*)(uu + 1);
	register unsigned char*	m;
	register int		c;
	register int		i;
	struct stat		st;

	bx->last = -1;
	bx->qp = bx->qbuf;
	bx->qe = bx->qbuf + sizeof(bx->qbuf);
	if (fstat(sffileno(uu->ip), &st))
		st.st_size = 0;
	sfprintf(uu->op, "(This file must be converted with BinHex 4.0)\n:");
	if (!(m = (unsigned char*)uu->path))
		m = (unsigned char*)"-";
	if ((c = strlen((char*)m)) > 63)
		c = 63;
	bx_q_putc(uu, bx, c);
	for (i = 0; i < c; i++)
		bx_q_putc(uu, bx, m[i]);
	bx_q_putc(uu, bx, 0);
	bx_q_putn(uu, bx, 0, 4);
	bx_q_putn(uu, bx, 0, 4);
	bx_q_putn(uu, bx, 0xF800, 2);
	bx_q_putn(uu, bx, st.st_size, 4);
	bx_q_putn(uu, bx, 0, 4);
	bx_q_crc(bx, 0);
	bx_q_crc(bx, 0);
	bx_q_putn(uu, bx, bx->crc, 2);
	bx->crc = 0;
	while ((c = sfgetc(uu->ip)) != EOF)
		bx_q_putc(uu, bx, c);
	bx_q_crc(bx, 0);
	bx_q_crc(bx, 0);
	bx_q_putn(uu, bx, bx->crc, 2);
	bx->crc = 0;
	bx_q_crc(bx, 0);
	bx_q_crc(bx, 0);
	bx_q_putn(uu, bx, bx->crc, 2);
	while (bx->qp != bx->qbuf)
		bx_q_putc(uu, bx, 0);
	sfputc(uu->op, ':');
	sfputc(uu->op, '\n');
	return 0;
}

/*
 * cat input to output
 */

static int
cat(register Uu_t* uu)
{
	return sfmove(uu->ip, uu->op, SF_UNBOUND, -1) >= 0 && sfeof(uu->ip) ? 0 : -1;
}

static Uudata_t	uu_posix =
{
	"end",
	0,
	0,
	UU_HEADER|UU_LENGTH,
	0,
	" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
};

static const Uudata_t	uu_ucb =
{
	"end",
	0,
	0156,
	UU_HEADER|UU_LENGTH,
	0,
	"`!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
};

static const Uudata_t	uu_base64 =
{
	"====",
	'=',
	0,
	0,
	0,
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
};

static const Uudata_t	uu_bx =
{
	0,
	0,
	0,
	UU_HEADER|UU_HEADERMUST,
	sizeof(Bx_t),
	"!\"#$%&'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr"
};

static const Uumeth_t	methods[] =
{

{
	"posix",			"uuencode",	"",	  
	uu_header,	uu_encode,	uu_decode,	(void*)&uu_posix
},
{
	"ucb",				"bsd",		"",	  
	uu_header,	uu_encode,	uu_decode,	(void*)&uu_ucb
},
{
	"mime",				"base64",	"-base64",
	0,		uu_encode,	uu_decode,	(void*)&uu_base64
},
{
	"quoted-printable",		"qp",		"",	  
	0,		qp_encode,	qp_decode,	0
},
{
	"binhex",			"mac-binhex",	"",	  
	bx_header,	bx_encode,	bx_decode,	(void*)&uu_bx
},
{
	"sevenbit",			"7bit",		"",	  
	0,		cat,		cat,		0
},

{ 0 }

};

/*
 * list the method names/alternates on fp
 */

int
uulist(Sfio_t* fp)
{
	register const Uumeth_t*	mp;

	sfprintf(fp, "ENCODING          ALIAS\n");
	for (mp = methods; mp->name; mp++)
		sfprintf(fp, "%-17s %s\n", mp->name, mp->alias);
	return 0;
}

/*
 * return method pointer given name
 */

Uumeth_t*
uumeth(const char* name)
{
	register const Uumeth_t*	mp;
	register int			c;
	register const char*		v;
	register int			vl;
	const char*			np;

	/*
	 * first entry is the default
	 */

	if (!name || !*name)
	{
		((Uudata_t*)methods->data)->flags |= UU_DEFAULT;
		return (Uumeth_t*)methods;
	}
	if ((*name == 'x' || *name == 'X') && *(name + 1) == '-')
		name += 2;
	if (*name == 'b' && !strncasecmp(name, UU_BEGIN, UU_BEGIN_LEN))
	{
		/*
		 * id prefix match
		 */

		if ((c = *(name += UU_BEGIN_LEN)) == ' ')
			return (Uumeth_t*)methods;
		for (mp = methods; mp->name; mp++)
			if (*mp->id == c && !strncasecmp(name, mp->id, strlen(mp->id)))
				return (Uumeth_t*)mp;
		if (c == '-')
			return (Uumeth_t*)methods;
	}
	else
	{
		c = *name;
		for (v = name + strlen(name); v > name && (isdigit(*(v - 1)) || *(v - 1) == '.'); v--);
		vl = *v ? (v - name) : 0;

		/*
		 * exact name or alias match
		 */
	
		for (;;)
		{
			for (mp = methods; mp->name; mp++)
				if (*mp->name == c && (!strcasecmp(name, mp->name) || vl && !strncasecmp(name, mp->name, vl)) ||
			    	mp->alias && *mp->alias == c && (!strcasecmp(name, mp->alias) || vl && !strncasecmp(name, mp->alias, vl)))
					return (Uumeth_t*)mp;
			np = name;
			if (!(name = strchr(name, '/')))
				break;
			if (((c = *++name) == 'x' || *name == 'X') && *(name + 1) == '-')
				c = *(name += 2);
			if (vl)
				vl -= (name - np);
		}
	
		/*
		 * first char name match
		 */
	
		for (mp = methods; mp->name; mp++)
			if (*mp->name == c)
				return (Uumeth_t*)mp;
	}
	return 0;
}

/*
 * open an encode/decode handle
 */

Uu_t*
uuopen(Uudisc_t* disc, Uumeth_t* meth)
{
	register Uu_t*		uu;
	register Uudata_t*	data;
	int			extra;

	if (data = (Uudata_t*)meth->data)
		extra = data->size;
	else
		extra = 0;
	if (!(uu = newof(0, Uu_t, 1, extra)))
		return 0;
	uu->id = lib;
	uu->disc = disc;
	uu->meth = *meth;
	return uu;
}

/*
 * close an encode/decode handle
 */

int
uuclose(Uu_t* uu)
{
	if (!uu)
		return -1;
	if ((uu->flags & UU_FREEPATH) && uu->path)
		free(uu->path);
	free(uu);
	return 0;
}

/*
 * common encode/decode tail
 */

static ssize_t
uuop(register Uu_t* uu, Uu_f fun)
{
	int		n;
	ssize_t		r;
	Sfoff_t		p;

	if (uu->count != SF_UNBOUND && ((p = sfseek(uu->ip, (Sfoff_t)0, SEEK_CUR)) < 0 || !(uu->ip = sfdcsubstream(NiL, uu->lp = uu->ip, p, uu->count))))
	{
		if (uu->disc->errorf)
			(*uu->disc->errorf)(uu, uu->disc, 2, "cannot initialize substream at %I*d for %I*d bytes", sizeof(p), p, sizeof(uu->count), uu->count);
		return -1;
	}
	p = sfseek(uu->op, (Sfoff_t)0, SEEK_CUR);
	n = (*fun)(uu);
	if (uu->lp)
	{
		sfclose(uu->ip);
		uu->ip = uu->lp;
		uu->lp = 0;
	}
	if (n < 0)
		r = -1;
	else if (sfsync(uu->op) || sferror(uu->op))
	{
		r = -1;
		if (uu->disc->errorf)
			(*uu->disc->errorf)(uu, uu->disc, 2, "write error");
	}
	else if (sferror(uu->ip))
	{
		r = -1;
		if (uu->disc->errorf)
			(*uu->disc->errorf)(uu, uu->disc, 2, "read error");
	}
	else
		r = sfseek(uu->op, (Sfoff_t)0, SEEK_CUR) - p;
	if (uu->flags & UU_CLOSEOUT)
	{
		uu->flags &= ~UU_CLOSEOUT;
		sfclose(uu->op);
	}
	return r;
}

/*
 * encode n bytes (or all if SF_UNBOUND) from ip to op
 */

ssize_t
uuencode(register Uu_t* uu, Sfio_t* ip, Sfio_t* op, size_t n, const char* path)
{
	if (!uu->meth.encodef)
	{
		if (uu->disc->errorf)
			(*uu->disc->errorf)(uu, uu->disc, 2, "%s format encoding not supported", uu->meth.name);
		return -1;
	}
	if (!(uu->ip = ip) || !(uu->op = op))
		return -1;
	uu->count = n;
	if ((uu->flags & UU_FREEPATH) && uu->path)
		free(uu->path);
	uu->path = (char*)path;
	uu->flags = uu->disc->flags;
	return uuop(uu, uu->meth.encodef);
}

/*
 * decode n bytes (or all if SF_UNBOUND) from ip to op
 */

ssize_t
uudecode(register Uu_t* uu, Sfio_t* ip, Sfio_t* op, size_t n, const char* path)
{
	register char*	s;
	unsigned char*	m;
	int		c;
	int		headerpath;
	Uudata_t*	data;
	const Uumeth_t*	mp;
	char		map[UCHAR_MAX + 2];

	if (!uu->meth.decodef)
	{
		if (uu->disc->errorf)
			(*uu->disc->errorf)(uu, uu->disc, 2, "%s format decoding not supported", uu->meth.name);
		return -1;
	}
	if (!(uu->ip = ip))
		return -1;
	uu->op = op;
	uu->count = n;
	if ((uu->flags & UU_FREEPATH) && uu->path)
		free(uu->path);
	uu->path = (char*)path;
	uu->flags = uu->disc->flags;
	data = (Uudata_t*)uu->meth.data;
	if (((uu->flags & UU_HEADER) || data && (data->flags & UU_HEADERMUST)) && uu->meth.headerf)
	{
		if ((*uu->meth.headerf)(uu))
			return -1;
		headerpath = 1;
	}
	else
	{
		headerpath = 0;
		if (data && (data->flags & UU_DEFAULT) && (c = sfgetc(uu->ip)) != EOF)
		{
			sfungetc(uu->ip, c);
			for (mp = methods; ((Uudata_t*)mp->data)->flags & UU_LENGTH; mp++)
			{
				m = uu_map((Uudata_t*)mp->data, map);
				if (m[c] > 0 && m[c] < (UUIN * UUCHUNK + 1))
					break;
			}
			if (mp > methods)
			{
				uu->meth = *mp;
				if (uu->disc->errorf)
					(*uu->disc->errorf)(uu, uu->disc, 1, "assuming %s encoding", mp->name);
			}
		}
	}
	if (!uu->op)
	{
		if (!uu->path && headerpath)
		{
			if (uu->disc->errorf)
				(*uu->disc->errorf)(uu, uu->disc, 2, "%s format header has no output file name", uu->meth.name);
			return -1;
		}
		if (!uu->path || !*uu->path || streq(uu->path, "-") || streq(uu->path, "/dev/stdout"))
			uu->op = sfstdout;
		else
		{
			if (headerpath && (uu->flags & UU_LOCAL))
			{
				for (s = uu->path; *s; s++)
					if (isspace(*s) || iscntrl(*s) || !isprint(*s) || *s == '/' || *s == '\\')
						*s = '_';
				s = uu->path;
				if (isalpha(s[0]) && s[1] == ':')
					s[1] = '_';
			}
			if (!(uu->op = sfopen(NiL, uu->path, "w")))
			{
				if (uu->disc->errorf)
					(*uu->disc->errorf)(uu, uu->disc, 2, "%s: cannot write", uu->path);
				return -1;
			}
			uu->flags |= UU_CLOSEOUT;
		}
	}
	return uuop(uu, uu->meth.decodef);
}
