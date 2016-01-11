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

#include "ptlib.h"

/*
 * dump table a on sp
 */

int
ptdump(Pt_t* a, Sfio_t* sp)
{
	Ptprefix_t*	p;

	for (p = (Ptprefix_t*)dtfirst(a->dict); p; p = (Ptprefix_t*)dtnext(a->dict, p))
		sfprintf(sp, "%0*I*x %0*I*x %-16s %-16s\n", sizeof(p->min) * 2, sizeof(p->min), p->min, sizeof(p->max) * 2, sizeof(p->max), p->max, fmtip4(p->min, -1), fmtip4(p->max, -1));
	return sfsync(sp);
}
