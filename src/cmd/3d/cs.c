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
#pragma prototyped

#define msgreserve(p)	reserve(p)

#define gethostname	______gethostname

#include "3d.h"

#undef	gethostname
#undef	FSYNC

#include <ctype.h>

#define CS_LIB_LOCAL	1

#define _3d_fmttime(a,b)	"[NOW]"

#if FS

#undef	NoN
#define NoN(x)

#include "csdata.c"
#undef	csaddr
#define csaddr(p,x)	0
#include "csauth.c"
#include "csbind.c"
#include "cslocal.c"
#include "csname.c"
#include "csntoa.c"
#include "csread.c"
#include "csrecv.c"
#include "cssend.c"
#include "csvar.c"
#include "cswrite.c"

#include "msgbuf.c"
#include "msgread.c"
#include "msguser.c"

#endif

#include "cspeek.c"
#include "cspipe.c"

#if FS
#include "cspoll.c"	/* follows cs*.c because of #undef's */
#endif

#include "msggetmask.c"
#include "msgindex.c"
#include "msginfo.c"
#include "msgname.c"
#include "msgsetmask.c"
