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
 * return a copy of table a with prefixes limited to a maximum of m bits
 */

Pt_t*
ptrebit(Pt_t* a, int m)
{
	Pt_t*		b;
	Ptprefix_t*	ap;

	if (b = ptopen(a->disc))
		for (ap = (Ptprefix_t*)dtfirst(a->dict); ap; ap = (Ptprefix_t*)dtnext(a->dict, ap))
			if (!ptinsert(b, PTMIN(ap->min, m), PTMAX(ap->max, m)))
			{
				ptclose(b);
				return 0;
			}
	return b;
}
