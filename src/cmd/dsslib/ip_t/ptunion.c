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
 * return the table union of a and b
 */

Pt_t*
ptunion(Pt_t* a, Pt_t* b)
{
	Ptprefix_t*	bp;

	if (!(a = ptcopy(a)))
		return 0;
	for (bp = (Ptprefix_t*)dtfirst(b->dict); bp; bp = (Ptprefix_t*)dtnext(b->dict, bp))
		if (!ptinsert(a, bp->min, bp->max))
			break;
	return a;
}
