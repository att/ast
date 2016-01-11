/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
#define _def_map_ast	1
#define _std_strtol	1

#ifndef _GNU_SOURCE
#define _GNU_SOURCE	1
#endif
#ifndef __EXTENSIONS__
#define __EXTENSIONS__	1
#endif

#define strmode		______strmode

#include <ast_std.h>

#undef	strmode

#include "dll_3d.h"

#undef	strtol
#undef	strtoul
#undef	strtoll
#undef	strtoull

#include "dllnext.c"
#include "dlfcn.c"
