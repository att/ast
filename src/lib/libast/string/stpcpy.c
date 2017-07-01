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
 * stpcpy implementation
 */

#define stpcpy		______stpcpy

#include <ast.h>

#undef	stpcpy

#undef	_def_map_ast
#include <ast_map.h>

#if _lib_stpcpy

NoN(stpcpy)

#else

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

/*
 * copy f into t, return a pointer to the end of t ('\0')
 */

extern char*
stpcpy(register char* t, register const char* f)
{
	if (!f)
		return t;
	while (*t++ = *f++);
	return t - 1;
}

#endif
