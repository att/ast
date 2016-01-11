/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2013 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#ifndef _CONTEXT_H
#define _CONTEXT_H		1

#include <ast.h>

typedef struct Context_line_s
{
	char*		data;
	size_t		size;
	uintmax_t	line;
#ifdef _CONTEXT_LINE_PRIVATE_
	_CONTEXT_LINE_PRIVATE_
#endif
} Context_line_t;

typedef int (*Context_list_f)(Context_line_t*, int, int, void*);

typedef struct Context_s
{
	void*		handle;
#ifdef _CONTEXT_PRIVATE_
	_CONTEXT_PRIVATE_
#endif
} Context_t;

extern Context_t*	context_open(Sfio_t*, size_t, size_t, Context_list_f, void*);
extern Context_line_t*	context_line(Context_t*);
extern int		context_show(Context_t*);
extern int		context_close(Context_t*);

#endif
