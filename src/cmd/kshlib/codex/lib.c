/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2007-2012 AT&T Intellectual Property          *
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

#include <ast.h>
#include <shcmd.h>

extern int	b_codex(int, char**, Shbltin_t*);

void
lib_init(int flag, void* context)
{
	if (!flag)
		sh_builtin(context, "codex", b_codex, 0); 
}

SHLIB(codex)
