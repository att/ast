/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#include	"dthdr.h"

/*	Walk a dictionary and all dictionaries viewed through it.
**	userf:	user function
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com (5/25/96)
*/

#if __STD_C
int dtwalk(Dt_t* dt, int (*userf)(Dt_t*, Void_t*, Void_t*), Void_t* data)
#else
int dtwalk(dt,userf,data)
Dt_t*	dt;
int(*	userf)();
Void_t*	data;
#endif
{
	Void_t	*obj, *next;
	Dt_t	*walk;
	int	rv;

	for(obj = dtfirst(dt); obj; )
	{	if(!(walk = dt->walk) )
			walk = dt;
		next = dtnext(dt,obj);
		if((rv = (*userf)(walk, obj, data )) < 0)
			return rv;
		obj = next;
	}

	return 0;
}
