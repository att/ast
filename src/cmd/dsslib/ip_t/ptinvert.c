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
 * return an inverted copy of table a
 */

Pt_t*
ptinvert(Pt_t* a)
{
	Pt_t*		t;
	Ptprefix_t*	ap;
	Ptaddr_t	m;

	if (t = ptopen(a->disc))
	{
		m = 0;
		for (ap = (Ptprefix_t*)dtfirst(a->dict); ap; ap = (Ptprefix_t*)dtnext(a->dict, ap))
		{
			if (m < ap->min && !ptinsert(t, m, ap->min - 1))
				break;
			m = ap->max + 1;
		}
		if (m || !dtsize(a->dict))
			ptinsert(t, m, ~((Ptaddr_t)0));
	}
	return t;
}
