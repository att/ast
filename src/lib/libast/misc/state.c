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

static const char id[] = "\n@(#)$Id: ast (AT&T Research) 2013-11-26 $\0\n";

#include <ast.h>

#undef	strcmp

_Ast_info_t	_ast_info =
{
	"libast",	/* id */
	{ 0 },
	0,0,0,0,0,
	strcmp,		/* collate */
	0,0,
	1,		/* mb_cur_max */
	0,0,0,0,0,0,0,
	20130624,	/* version */
	0,
	AT_FDCWD	/* pwd */
};

__EXTERN__(_Ast_info_t, _ast_info);
