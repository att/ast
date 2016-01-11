/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"vmtest.h"

int	Release = 0;


#if __STD_C
Void_t*	memory(Vmalloc_t* vm, Void_t* caddr,
		size_t oldsize, size_t newsize, Vmdisc_t* disc)
#else
Void_t*	memory(vm, caddr, oldsize, newsize, disc)
Vmalloc_t*	vm;
Void_t*		caddr;
size_t		oldsize;
size_t		newsize;
Vmdisc_t*	disc;
#endif
{
	if(caddr)
	{	if(newsize != 0)
			return NIL(Void_t*);
		Release += 1;
		vmfree(Vmheap,caddr);
		return caddr;
	}
	return vmalloc(Vmheap,newsize);
}

Vmdisc_t	Disc = {memory, NIL(Vmexcept_f), 64};

tmain()
{
	Void_t*		addr[10];
	Vmalloc_t*	vm;
	int		i;

	if(!(vm = vmopen(&Disc,Vmbest,0)) )
		terror("Can't open");

	for(i = 0; i < 10; ++i)
	{	addr[i] = vmalloc(vm,15);
		if((((Vmulong_t)addr[i])%ALIGN) != 0)
			terror("Unaligned addr");
	}

	for(i = 0; i < 10; ++i)
		if(vmfree(vm,addr[i]) < 0)
			terror("can't free an element?");

	if(!(addr[0] = vmalloc(vm,30)) )
		terror("Can't alloc");

	for(i = 0; i < 10; ++i)
		addr[i] = vmalloc(vm,15);

	if(vmresize(vm,addr[0],16,1) != addr[0])
		terror("Location should not have changed");

#ifdef DEBUG
	for(i = 0; i < 10; ++i)
		printf("size[%d]=%d\n",i,vmsize(vm,addr[i]));
	printf("vmextent=%d\n",vmsize(vm,NIL(Void_t*)));
#endif

	texit(0);
}
