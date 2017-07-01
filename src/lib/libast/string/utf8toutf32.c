/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * convert utf8 to utf32
 * return value is the number of chars consumed from s
 */

#include <ast.h>
#include <error.h>

static const uint32_t		utf8mask[] =
{
	0x00000000,
	0x00000000,
	0xffffff80,
	0xfffff800,
	0xffff0000,
	0xffe00000,
	0xfc000000,
};

static const signed char	utf8tab[256] =
{
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6,-1,-1,
};

size_t
utf8towc(wchar_t* wp, const char* str, size_t n)
{
	register unsigned char*	sp = (unsigned char*)str;
	register int		m;
	register int		i;
	register int		c;
	register wchar_t	w = 0;

	if (!sp || !n)
		goto nul;
	if ((m = utf8tab[*sp]) > 0)
	{
		if (m > n)
			return (size_t)-2;
		if (wp)
		{
			if (m == 1)
				*wp = *sp;
			else
			{
				w = *sp & ((1<<(8-m))-1);
				for (i = m - 1; i > 0; i--)
				{
					c = *++sp;
					if ((c&0xc0) != 0x80)
						goto invalid;
					w = (w<<6) | (c&0x3f);
				}
				if (!(utf8mask[m] & w) || utf32invalid(w))
					goto invalid;
				*wp = w;
			}
		}
		return m;
	}
	if (!*sp)
	{
 nul:
		if (wp)
			*wp = 0;
		return 0;
	}
 invalid:
	errno = EILSEQ;
	return (size_t)-1;
}

size_t
utf8toutf32(uint32_t* up, const char* str, size_t n)
{
	wchar_t		wc;
	int		r;

	if ((r = utf8towc(&wc, str, n)) > 0)
		*up = (uint32_t)wc;
	return r;
}

/*
 * str known to contain only valid UTF-8 characters
 */

size_t
utf8toutf32v(uint32_t* up, const char* str)
{
	register unsigned char*	s = (unsigned char*)str;

	switch (utf8tab[*s])
	{
	case 1:
		*up = s[0];
		return 1;
	case 2:
		*up = (((s[0] & 0x1c) << 6) | ((s[0] & 0x03) << 6) | (s[1] & 0x3f));
		return 2;
	case 3:
		*up = (((s[0] & 0x0f) << 12) | ((s[1] & 0x3c) << 6) | ((s[1] & 0x03) << 6) | (s[2] & 0x3f));
		return 3;
	case 4:
		*up = (((s[0] & 0x07) << 18) | ((s[1] & 0x30) << 12) | ((s[1] & 0x0f) << 12) | ((s[2] & 0x3c) << 6) | ((s[2] & 0x03) << 6) | (s[3] & 0x3f));
		return 4;
	}
	*up = 0;
	return 0;
}
