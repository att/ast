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
 * convert msg name list s to mask
 * *s=='!' inverts the mask
 */

unsigned long
msgsetmask(register const char* s)
{
	register int		c;
	register unsigned long m;
	int			invert;

	m = 0;
	invert = *s == '!';
	for (;;)
	{
		do if (!(c = *s++)) goto done; while (!isalnum(c));
		m |= MSG_MASK(msgindex(--s));
		do if (!(c = *s++)) goto done; while (isalnum(c));
	}
 done:
	return invert ? ~m : m;
}
