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

#include <ast.h>
#include <aso.h>

/*
 * return small format buffer chunk of size n
 * small buffers thread safe but short-lived
 * only one concurrent buffer with size > sizeof(buf)
 */

static char		buf[16 * 1024];
static char*		nxt = buf;

static char*		big;
static size_t		bigsiz;

char*
fmtbuf(size_t n)
{
	char*	cur;
	char*	tst;

	do
	{
		cur = nxt;
		if (n > (&buf[sizeof(buf)] - cur))
		{
			if (n > sizeof(buf))
			{
				if (n > bigsiz)
				{
					bigsiz = roundof(n, 8 * 1024);
					if (!(big = oldof(big, char, bigsiz, 0)))
						return 0;
				}
				return big;
			}
			tst = buf;
		}
		else
			tst = cur;
	} while (asocasptr(&nxt, cur, tst + n) != cur);
	return tst;
}
