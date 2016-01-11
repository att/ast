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
 * return the table intersection of a and b
 */

Ptv_t*
ptvintersect(Ptv_t* a, Ptv_t* b)
{
	Ptv_t*	t;
	Ptvprefix_t*	ap;
	Ptvprefix_t*	bp;

	if (!(t = ptvopen(a->disc, a->size)))
		return 0;
	ap = (Ptvprefix_t*)dtfirst(a->dict);
	bp = (Ptvprefix_t*)dtfirst(b->dict);
	while (ap && bp)
	{
		if (fvcmp(a->size, ap->max, bp->min) < 0)
			ap = (Ptvprefix_t*)dtnext(a->dict, ap);
		else if (fvcmp(a->size, ap->min, bp->max) > 0)
			bp = (Ptvprefix_t*)dtnext(b->dict, bp);
		else
		{
			if (!ptvinsert(t, fvcmp(a->size, ap->min, bp->min) > 0 ? ap->min : bp->min, fvcmp(a->size, ap->max, bp->max) < 0 ? ap->max : bp->max))
				break;
			if (fvcmp(a->size, ap->max, bp->max) < 0)
				ap = (Ptvprefix_t*)dtnext(a->dict, ap);
			else if (fvcmp(a->size, ap->max, bp->max) > 0)
				bp = (Ptvprefix_t*)dtnext(b->dict, bp);
			else
			{
				ap = (Ptvprefix_t*)dtnext(a->dict, ap);
				bp = (Ptvprefix_t*)dtnext(b->dict, bp);
			}
		}
	}
	return t;
}
