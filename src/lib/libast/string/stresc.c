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
 * convert \X character constants in s in place
 * the length of the converted s is returned (may have embedded \0's)
 * strexp() FMT_EXP_* flags passed to chrexp() for selective conversion
 */

#include <ast.h>

int
strexp(register char* s, int flags)
{
	register char*		t;
	register int		c;
	char*			b;
	char*			e;
	int			w;
	Mbstate_t		q;

	b = t = s;
	while (c = *s++)
	{
		if (c == '\\')
		{
			c = chrexp(s - 1, &e, &w, flags);
			s = e;
			if (c < 0)
				continue;
			if (w)
			{
				mbinit(&q);
				t += mbconv(t, c, &q);
				continue;
			}
		}
		*t++ = c;
	}
	*t = 0;
	return t - b;
}

int
stresc(register char* s)
{
	return strexp(s, FMT_EXP_CHAR|FMT_EXP_LINE|FMT_EXP_WIDE);
}
