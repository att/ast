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
#if defined(_UWIN) && defined(_BLD_ast)

void _STUB_vmset(){}

#else

#include	"vmhdr.h"


/*	Set the control flags for a region.
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 01/16/94.
*/
#if __STD_C
int vmset(Vmalloc_t* vm, int flags, int on)
#else
int vmset(vm, flags, on)
Vmalloc_t*	vm;	/* region being worked on		*/
int		flags;	/* flags must be in VM_FLAGS		*/
int		on;	/* >0 if turning on, else turning off	*/
#endif
{
	int		mode, newm;
	Vmdata_t	*vd = vm->data;

	if(flags == 0 && on == 0)
		return vd->mode;

	for(flags &= VM_SETFLAGS;; ) /* lock-less mode setting */
	{	mode = vd->mode;
		newm = on > 0 ? (mode | flags) : (mode & ~flags);
		if(asocasint(&vd->mode, mode, newm) == mode)
			return newm;
	}
}

#endif
