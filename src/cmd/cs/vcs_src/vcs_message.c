/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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

#include "vcs_rscs.h"

extern int	debug;

static void tracev(int level, va_list ap)
{
	char*	format;

	if (!level || debug && level <= debug)
	{
		if (level)
			sfprintf(sfstdout, "[%d] ", level);
		format = va_arg(ap, char*);
		sfvprintf(sfstdout, format, ap);
		sfprintf(sfstdout, "\n");
	}
}

void trace(int level, ...)
{
	va_list		ap;
	
	va_start(ap, level);
	tracev(level, ap);
	va_end(ap);
}
