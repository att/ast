/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "ptvlib.h"

/*
 * dump table a on sp
 */

int
ptvdump(Ptv_t* a, Sfio_t* sp)
{
	Ptvprefix_t*	p;

	for (p = (Ptvprefix_t*)dtfirst(a->dict); p; p = (Ptvprefix_t*)dtnext(a->dict, p))
		sfprintf(sp, "%s %s %-24s %-24s\n", fmtfv(a->size, p->min, 16, 0, 0), fmtfv(a->size, p->max, 16, 0, 0), fmtip6(p->min, -1), fmtip6(p->max, -1));
	return sfsync(sp);
}
