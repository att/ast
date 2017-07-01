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
 * ast global debugging/profiling hooks
 */

#ifndef _AST_DEBUG_H
#define _AST_DEBUG_H	1

/*
 * really handy malloc()/free() (__FILE__,__LINE__,__FUNCTION__) tracing
 * make with VMDEBUG==1 or debug=1 or CCFLAGS=$(CC.DEBUG)
 * VMDEBUG==0 disables
 * at runtime export VMALLOC_OPTIONS per vmalloc.3
 * to list originating call locations
 */

#if !_std_malloc && !defined(VMFL) && !defined(_VMHDR_H) && \
	(VMDEBUG || !defined(VMDEBUG) && _BLD_DEBUG)

#define VMFL	1

#include <vmalloc.h>

#endif

#if _AST_DEBUG_newof

#include <error.h>

static void* _ast_newof(void* _ast_p, ssize_t _ast_t, ssize_t _ast_n, ssize_t _ast_x, const char* _ast_f, unsigned long _ast_l)
{
	void*	_ast_r;

	r = newof(p, t, n, x);
	error(-2, "%s:%lu: newof(%p,%zd,%zd,%zd)=%p", _ast_f, _ast_l, _ast_p, _ast_t, _ast_n, _ast_x, _ast_r)
	return _ast_r;
}

#undef	newof
#define newof(p,t,n,x)	_ast_newof(p,t,n,x,__FILE__,__LINE__)

static void* _ast_oldof(void* _ast_p, ssize_t _ast_t, ssize_t _ast_n, ssize_t _ast_x, const char* _ast_f, unsigned long _ast_l)
{
	void*	_ast_r;

	r = oldof(p, t, n, x);
	error(-2, "%s:%lu: oldof(%p,%zd,%zd,%zd)=%p", _ast_f, _ast_l, _ast_p, _ast_t, _ast_n, _ast_x, _ast_r)
	return _ast_r;
}

#undef	oldof
#define oldof(p,t,n,x)	_ast_oldof(p,t,n,x,__FILE__,__LINE__)

#endif

#endif
