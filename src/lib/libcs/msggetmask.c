/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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

#include "msglib.h"

/*
 * put list of msg names in mask into buffer b,n
 * ! as first char means inverted mask
 * size of mask is returned
 * negative of required size returned on overflow
 * if b==0 then size is just returned
 */

int
msggetmask(char* buf, register int n, register unsigned long mask)
{
	register char*		b;
	register const char*	s;
	register int		m;
	register int		n0;
	register int		n1;
	char*			e;

	if (n <= 1) return 0;
	if (b = buf) e = b + n;
	for (n0 = n1 = 0, m = 1; m <= MSG_STD; m++)
	{
		n = strlen(msgname(m)) + 1;
		if (mask & MSG_MASK(m)) n1 += n;
		else n0 += n;
	}
	if (n = (n1 > n0))
	{
		n0++;
		if (!b) return n0;
		n0 = -n0;
		if (b >= e) return n0;
		*b++ = '!';
	}
	else if (!b) return n1;
	else n0 = -n1;
	for (m = 1; m <= MSG_STD; m++)
		if ((mask & MSG_MASK(m)) == 0 == n)
		{
			s = msg_info.name[m];
			while (n1 = *s++)
			{
				if (b >= e) return n0;
				*b++ = n1;
			}
			if (b >= e) return n0;
			*b++ = ',';
		}
	if (b > buf && *(b - 1) == ',') b--;
	*b = 0;
	return b - buf;
}
