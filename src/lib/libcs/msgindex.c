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

#include <ctype.h>

/*
 * return call index given name
 * !isalnum() chars at end of name are ignored
 * 0 returned if not found
 */

int
msgindex(const char* name)
{
	register const char**	lo;
	register const char**	hi;
	register const char**	mid;
	register unsigned char*	n;
	register unsigned char*	m;
	register int		v;

 again:
	lo = msg_info.name;
	hi = msg_info.name + MSG_STD;
	while (lo <= hi)
	{
		mid = lo + (hi - lo) / 2;
		n = (unsigned char*)name;
		m = (unsigned char*)*mid;
		for (;;)
		{
			if (!*m)
			{
				if (!isalnum(*n))
					return mid - msg_info.name;
				return 0;
			}
			if ((v = *n++ - *m++) > 0)
			{
				lo = mid + 1;
				break;
			}
			if (v < 0)
			{
				hi = mid - 1;
				break;
			}
		}
	}
	if (name[0] == 'f')
	{
		name++;
		goto again;
	}
	if (name[0] == 'i' && name[1] == 'p' && name[2] == 'c')
	{
		name += 3;
		goto again;
	}
	if (name[0] == 's' && name[1] == 'y' && name[2] == 'm')
	{
		name += 3;
		goto again;
	}
	return 0;
}
