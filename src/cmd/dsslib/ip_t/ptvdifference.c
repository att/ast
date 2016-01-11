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
 * return the table difference of a and b
 */

Ptv_t*
ptvdifference(Ptv_t* a, Ptv_t* b)
{
	Ptv_t*		t;
	Ptvprefix_t*	ap;
	Ptvprefix_t*	bp;
	int		c;

	if (!(t = ptvopen(a->disc, a->size)))
		return 0;
	ap = (Ptvprefix_t*)dtfirst(a->dict);
	bp = (Ptvprefix_t*)dtfirst(b->dict);
	while (ap)
	{
		if (!bp || fvcmp(a->size, ap->max, bp->min) < 0)
		{
			if (!ptvinsert(t, ap->min, ap->max))
				break;
			ap = (Ptvprefix_t*)dtnext(a->dict, ap);
		}
		else if (fvcmp(a->size, ap->min, bp->max) > 0)
			bp = (Ptvprefix_t*)dtnext(b->dict, bp);
		else
		{
			if (fvcmp(a->size, ap->min, bp->min) < 0)
			{
				fvset(a->size, a->r[0], 1);
				fvsub(a->size, a->r[1], bp->min, a->r[0]);
				if (!ptvinsert(t, ap->min, a->r[1]))
					break;
			}
			if ((c = fvcmp(a->size, ap->max, bp->max)) < 0)
				ap = (Ptvprefix_t*)dtnext(a->dict, ap);
			else if (!c)
			{
				ap = (Ptvprefix_t*)dtnext(a->dict, ap);
				bp = (Ptvprefix_t*)dtnext(b->dict, bp);
			}
			else
			{
				fvset(a->size, a->r[0], 1);
				while (fvcmp(a->size, ap->max, bp->max) > 0)
				{
					fvadd(a->size, a->r[1], bp->max, a->r[0]);
					if (!(bp = (Ptvprefix_t*)dtnext(b->dict, bp)) || fvcmp(a->size, bp->min, ap->max) > 0)
					{
						if (!ptvinsert(t, a->r[1], ap->max))
							goto done;
						break;
					}
					if (fvcmp(a->size, bp->min, a->r[1]) > 0)
					{
						fvsub(a->size, a->r[2], bp->min, a->r[0]);
						if (!ptvinsert(t, a->r[1], a->r[2]))
							goto done;
					}
				}
				ap = (Ptvprefix_t*)dtnext(a->dict, ap);
			}
		}
	}
 done:
	return t;
}
