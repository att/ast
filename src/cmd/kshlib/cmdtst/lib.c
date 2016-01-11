/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1995-2012 AT&T Intellectual Property          *
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
*              Doug McIlroy <doug@research.bell-labs.com>              *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include <ast.h>
#include <shcmd.h>

#undef	CMDLIST
#if __STDC__
#define CMDLIST(f)	extern int b_##f(int, char**, Shbltin_t*);
#else
#define CMDLIST(f)	extern int b_##f();
#endif
#include "cmdtstlist.h"

void
cmdtst_init(int flag, void* context)
{
	if (!flag)
	{
#undef	CMDLIST
#define CMDLIST(f)	sh_builtin(context, #f, b_##f, 0);
#include "cmdtstlist.h"
	}
}

void
lib_init(int flags, void* context)
{
	cmdtst_init(flags, context);
}

SHLIB(cmdtst)
