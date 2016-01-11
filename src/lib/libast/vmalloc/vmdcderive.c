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
#include	"vmhdr.h"

/*	Create a discipline to get memory from another region
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 06/12/2012.
*/
typedef struct _drvdisc_s
{	Vmdisc_t	disc;
	int		type;	/* 1: Vcheap, 0: vm, -1: static	*/
	Vmalloc_t*	vm;	/* region being derived from	*/
} Drvdisc_t;

#if __STD_C
static Void_t* drvgetmem(Vmalloc_t* vm, Void_t* caddr, size_t csize, size_t nsize, Vmdisc_t* disc)
#else
static Void_t* drvgetmem(vm, caddr, csize, nsize, disc)
Vmalloc_t*	vm;	/* region doing allocation from 	*/
Void_t*		caddr;	/* current low address			*/
size_t		csize;	/* current size				*/
size_t		nsize;	/* new size				*/
Vmdisc_t*	disc;	/* discipline structure			*/
#endif
{
	Drvdisc_t	*drvdc = (Drvdisc_t*)disc;

	if(csize == 0 && nsize == 0)
		return NIL(Void_t*);
	else if(csize == 0)
		return vmalloc(drvdc->vm, nsize);
	else if(nsize == 0)
		return vmfree(drvdc->vm, caddr) >= 0 ? caddr : NIL(Void_t*);
	else	return vmresize(drvdc->vm, caddr, nsize, 0);
}

static int drvexcept(Vmalloc_t* vm, int type, Void_t* arg, Vmdisc_t* disc)
{
	Drvdisc_t	*dc = (Drvdisc_t*)disc;
	if(type == VM_ENDCLOSE)
	{	if(dc->type > 0)
			vmfree(Vmheap, disc);
		else if(dc->type == 0)
			vmfree(dc->vm, disc);
		return 0;
	}
	else if(type == VM_DISC)
		return -1;
	else	return 0;
}

Vmdisc_t* vmdcderive(Vmalloc_t* vm, ssize_t round, int heap)
{
	Drvdisc_t	*drvdc;

	if(!(drvdc= vmalloc(heap ? Vmheap : vm, sizeof(Drvdisc_t))) )
		return NIL(Vmdisc_t*);
	drvdc->disc.memoryf = drvgetmem;
	drvdc->disc.exceptf = drvexcept;
	drvdc->disc.round   = round;
	drvdc->type = heap ? 1 : 0;
	drvdc->vm = vm;

	return (Vmdisc_t*)drvdc;
}

/* standard discipline to allocate from the heap */
static Drvdisc_t	_Vmdcheap = { {drvgetmem, drvexcept, 0, 0}, -1, &_Vmheap };
__DEFINE__(Vmdisc_t*, Vmdcheap, (Vmdisc_t*)(&_Vmdcheap));
