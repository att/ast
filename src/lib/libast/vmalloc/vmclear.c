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

void _STUB_vmclear(){}

#else

#include	"vmhdr.h"

/*	Clear a region.
*/

#if __STD_C
int vmclear(Vmalloc_t* vm)
#else
int vmclear(vm)
Vmalloc_t*	vm;
#endif
{
	Seg_t		*seg, *next;
	Vmdata_t	*vmdt = vm->data;
	Vmdisc_t	*disc = vm->disc;

	if(!vm || vm == Vmheap || (vmdt->mode&VM_MEMORYF)) /* the heap and VM_MEMORYF regions are never cleared */
		return -1;

	/* vm stays on the list of all regions so the seg list pointer is
	** cleared just in case vmsegwalk() is called before _vmopen() finishes
	*/
	seg = vmdt->seg;
	vmdt->seg = 0;

	/* memory obtained from discipline can be deallocated */
	for(; seg; seg = next)
	{	next = seg->next; 
		(void)(*disc->memoryf)(vm, seg->base, seg->size, 0, disc);
	}

	return _vmopen(vm, vm->disc, &vm->meth, 0) ? 0 : -1;
}

#endif
