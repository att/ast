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

/*	Change method for a context
**
**	Written by Kiem-Phong Vo (07/29/96).
*/

#if __STD_C
Rsmethod_t* rsmethod(Rs_t* rs, Rsmethod_t* meth)
#else
Rsmethod_t* rsmethod(rs, meth)
Rs_t*		rs;
Rsmethod_t*	meth;
#endif
{
	reg Rsmethod_t*	old = rs->meth;
	reg Void_t*	methdata;

	if(!meth)
		return old;

	if (RSNOTIFY(rs,RS_METHOD,meth,0,rs->disc) < 0)
		return NIL(Rsmethod_t*);

	if(meth->size <= 0)
		methdata = NIL(Void_t*);
	else if(!(methdata = vmresize(Vmheap,NIL(Void_t*),meth->size,VM_RSZERO)) )
		return NIL(Rsmethod_t*);

	if(rs->methdata)
		vmfree(Vmheap,rs->methdata);

	rs->meth = meth;
	rs->methdata = methdata;

	return old;
}
