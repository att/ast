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
 * return the table cover of a over b
 * the ranges in a that intersect with b
 */

Ptv_t*
ptvcover(Ptv_t* a, Ptv_t* b)
{
	Ptv_t*		t;
	Ptvprefix_t*	ap;
	Ptvprefix_t*	bp;

	if (!(t = ptvopen(a->disc, a->size)))
		return 0;
	ap = (Ptvprefix_t*)dtfirst(a->dict);
	bp = (Ptvprefix_t*)dtfirst(b->dict);
	while (ap && bp)
	{
		if (fvcmp(a->size, ap->min, bp->max) > 0)
			bp = (Ptvprefix_t*)dtnext(b->dict, bp);
		else
		{
			if (fvcmp(a->size, ap->max, bp->min) >= 0 && !ptvinsert(t, ap->min, ap->max))
				break;
			ap = (Ptvprefix_t*)dtnext(a->dict, ap);
		}
	}
	return t;
}
