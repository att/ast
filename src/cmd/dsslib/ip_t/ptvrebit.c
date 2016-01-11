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
 * return a copy of table a with prefixes limited to a maximum of m bits
 */

Ptv_t*
ptvrebit(Ptv_t* a, int m)
{
	Ptv_t*		b;
	Ptvprefix_t*	ap;

	if (b = ptvopen(a->disc, a->size))
		for (ap = (Ptvprefix_t*)dtfirst(a->dict); ap; ap = (Ptvprefix_t*)dtnext(a->dict, ap))
			if (!ptvinsert(b, ptvmin(a->size, b->r[0], ap->min, m), ptvmin(a->size, b->r[1], ap->max, m)))
			{
				ptvclose(b);
				return 0;
			}
	return b;
}
