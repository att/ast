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
 * ast private <ast.h>
 */

#ifndef _AST_LIB_H
#define _AST_LIB_H	1

#include <ast.h>

typedef struct Ast_global_s
{
	char*		id;			/* lib id			*/
	uint32_t	version;		/* YYYYMMDD			*/

	uintmax_t	serial;			/* total #instantiations	*/

	uint32_t	restart;		/* EINTR restart serial		*/

} Ast_global_t;

extern Ast_global_t	ast_global;

#endif
