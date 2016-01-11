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
/*
 * Glenn Fowler
 * AT&T Research
 *
 * write n bytes to fd, using multiple write(2) if necessary
 */

#include "cslib.h"

ssize_t
cswrite(register Cs_t* state, int fd, const void* buf, register size_t n)
{
	register char*		p = (char*)buf;
	register ssize_t	i;

	while (n > 0)
	{
		messagef((state->id, NiL, -9, "write(%d,%d) `%-.*s'", fd, n, n - 1, (n > 0 && *((char*)buf + n - 1) == '\n') ? (char*)buf : "..."));
		if ((i = write(fd, p, n)) <= 0)
		{
			messagef((state->id, NiL, -9, "write(%d,%d) [%d]", fd, n, i));
			if (i && p == (char*)buf)
				return i;
			break;
		}
		n -= i;
		p += i;
	}
	return p - (char*)buf;
}

ssize_t
_cs_write(int fd, const void* buf, size_t n)
{
	return cswrite(&cs, fd, buf, n);
}
