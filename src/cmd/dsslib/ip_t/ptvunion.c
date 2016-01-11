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
 * return the table union of a and b
 */

Ptv_t*
ptvunion(Ptv_t* a, Ptv_t* b)
{
	Ptvprefix_t*	bp;

	if (!(a = ptvcopy(a)))
		return 0;
	for (bp = (Ptvprefix_t*)dtfirst(b->dict); bp; bp = (Ptvprefix_t*)dtnext(b->dict, bp))
		if (!ptvinsert(a, bp->min, bp->max))
			break;
	return a;
}
