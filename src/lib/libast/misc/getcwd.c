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
 * return a pointer to the absolute path name of .
 */

#include <ast.h>

#if _WINIX

NoN(getcwd)

#else

#include "FEATURE/sys_calls"

#if defined(SYSGETCWD)

#include <error.h>

#define ERROR(e)	{ errno = e; return 0; }

char*
getcwd(char* buf, size_t len)
{
	size_t		n;
	size_t		r;
	int		oerrno;

	if (buf)
		return SYSGETCWD(buf, len) < 0 ? 0 : buf;
	oerrno = errno;
	n = PATH_MAX;
	for (;;)
	{
		if (!(buf = newof(buf, char, n, 0)))
			ERROR(ENOMEM);
		if (SYSGETCWD(buf, n) >= 0)
		{
			if ((r = strlen(buf) + len + 1) != n && !(buf = newof(buf, char, r, 0)))
				ERROR(ENOMEM);
			break;
		}
		if (errno != ERANGE)
		{
			free(buf);
			return 0;
		}
		n += PATH_MAX / 4;
	}
	errno = oerrno;
	return buf;
}

#else

char*
getcwd(char* buf, size_t len)
{
	return fgetcwd(AT_FDCWD, buf, len);
}

#endif

#endif
