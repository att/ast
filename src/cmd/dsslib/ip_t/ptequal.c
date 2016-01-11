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
 * return non-zero if table a and b are equal
 */

int
ptequal(Pt_t* a, Pt_t* b)
{
	Ptprefix_t*	ap;
	Ptprefix_t*	bp;

	if (a == b)
		return 1;
	ap = (Ptprefix_t*)dtfirst(a->dict);
	bp = (Ptprefix_t*)dtfirst(b->dict);
	while (ap && bp)
	{
		if (ap->min != bp->min)
			return 0;
		if (ap->max != bp->max)
			return 0;
		ap = (Ptprefix_t*)dtnext(a->dict, ap);
		bp = (Ptprefix_t*)dtnext(b->dict, bp);
	}
	return !ap && !bp;
}
