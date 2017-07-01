/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2011 AT&T Intellectual Property          *
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

#include <ast_lib.h>

#if _lib_execvp

#include <ast.h>

NoN(execvp)

#else

#if defined(__EXPORT__)
__EXPORT__ int execvp(const char*, char* const[]);
#endif

#include <ast.h>

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern int
execvp(const char* name, char* const argv[])
{
	return execvpe(name, argv, environ);
}

#endif
