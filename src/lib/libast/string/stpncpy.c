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
 * stpncpy implementation
 */

#define stpncpy		______stpncpy

#include <ast.h>

#undef	stpncpy

#undef	_def_map_ast
#include <ast_map.h>

#if _lib_stpncpy

NoN(stpncpy)

#else

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

/*
 * |stpncpy| - like |strncpy()| but returns the end of the buffer
 *
 * Copy s2 to s1, truncating or '\0'-padding to always copy n
 * bytes, return position of string terminator ('\0') in
 * destination buffer or if s1 is not '\0'-terminated, s1+n.
 *
 * See http://pubs.opengroup.org/onlinepubs/9699919799/functions/strncpy.html
 */

extern char*
stpncpy(char *t, const char *f, size_t n)
{
	const char*	e = f + n;

	while (f < e)
		if (!(*t++ = *f++))
		{
			if (n = e - f)
				memset(t, 0, n);
			return t - 1;
		}
	return t;
}

#endif
