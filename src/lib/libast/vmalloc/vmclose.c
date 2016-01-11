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

void _STUB_vmclose(){}

#else

#include	"vmhdr.h"

/*	Close down a region.
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 01/16/94.
*/

#if __STD_C
int vmclose(Vmalloc_t* vm)
#else
int vmclose(vm)
Vmalloc_t*	vm;
#endif
{
	int		mode;
	Seg_t		*seg, *next;
	Vmhold_t	*vh;
	Vmdata_t	*vmdt = vm->data;
	Vmdisc_t	*disc = vm->disc;
	int		rv = 0;

	if(!vm || vm == Vmheap) /* the heap is never freed */
		return -1;

	/* announce closing event */
	if(disc->exceptf && (rv = (*disc->exceptf)(vm, VM_CLOSE, (Void_t*)1, disc)) < 0 )
		return -1;

	if(vm->meth.eventf) /* let method clean up */
		(*vm->meth.eventf)(vm, VM_CLOSE, NIL(Void_t*));

	/* remove from list of regions */
	for(vh = _Vmhold; vh; vh = vh->next)
		if(asocasptr(&vh->vm, vm, NIL(Vmalloc_t*)) == vm)
			break;

	mode = vmdt->mode; /* remember this in case it gets destroyed below */
	if(rv == 0 ) /* memory obtained from discipline can be deallocated */
	{	for(seg = vmdt->seg; seg; seg = next)
		{	next = seg->next; 
			(void)(*disc->memoryf)(vm, seg->base, seg->size, 0, disc);
		}
	}

	if(disc->exceptf) /* the 3rd argument tells if vm might have been destroyed */
		(void)(*disc->exceptf)(vm, VM_ENDCLOSE, VMCAST(Void_t*, mode&VM_MEMORYF), disc);

	if(!(mode & VM_MEMORYF) )
		vmfree(Vmheap,vm);

	return 0;
}

#endif
