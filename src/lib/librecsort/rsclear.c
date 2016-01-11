/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#include	"rshdr.h"

/*	Clear space in a region
**
**	Written by Kiem-Phong Vo (07/18/96)
*/

#if __STD_C
int rsclear(Rs_t* rs)
#else
int rsclear(rs)
Rs_t*	rs;
#endif
{
	reg uchar	*m, *endm;

	for(m = (uchar*)rs->methdata, endm = m+rs->meth->size; m < endm; ++m)
		*m = 0;

	if(rs->vm)
		vmclear(rs->vm);
	rs->c_size = 0;
	rs->type &= RS_TYPES;
	rs->free = rs->sorted = NIL(Rsobj_t*);

	return 0;
}
