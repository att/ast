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
 * return the table difference of a and b
 */

Pt_t*
ptdifference(Pt_t* a, Pt_t* b)
{
	Pt_t*	t;
	Ptprefix_t*	ap;
	Ptprefix_t*	bp;
	Ptaddr_t	m;

	if (!(t = ptopen(a->disc)))
		return 0;
	m = 0;
	ap = (Ptprefix_t*)dtfirst(a->dict);
	bp = (Ptprefix_t*)dtfirst(b->dict);
	while (ap)
	{
		if (!bp || ap->max < bp->min)
		{
			if (!ptinsert(t, ap->min, ap->max))
				break;
			ap = (Ptprefix_t*)dtnext(a->dict, ap);
		}
		else if (ap->min > bp->max)
			bp = (Ptprefix_t*)dtnext(b->dict, bp);
		else
		{
			if (ap->min < bp->min && !ptinsert(t, ap->min, bp->min - 1))
				break;
			if (ap->max < bp->max)
				ap = (Ptprefix_t*)dtnext(a->dict, ap);
			else if (ap->max == bp->max)
			{
				ap = (Ptprefix_t*)dtnext(a->dict, ap);
				bp = (Ptprefix_t*)dtnext(b->dict, bp);
			}
			else
			{
				while (ap->max > bp->max)
				{
					m = bp->max + 1;
					if (!(bp = (Ptprefix_t*)dtnext(b->dict, bp)) || bp->min > ap->max)
					{
						if (!ptinsert(t, m, ap->max))
							goto done;
						break;
					}
					if (bp->min > m && !ptinsert(t, m, bp->min - 1))
						goto done;
				}
				ap = (Ptprefix_t*)dtnext(a->dict, ap);
			}
		}
	}
 done:
	return t;
}
