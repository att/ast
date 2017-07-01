/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2013 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#include	"vmtest.h"

typedef union _myalign_u
{	void*		v;
	unsigned long	l;
	double		d;
	unsigned int	i;
} Align_t;

static Align_t		Algn[1024*1024];
static Vmuchar_t	*Buf, *Endbuf, *Avail;
static int		Count = 0;
static int		Walk = 0;

#if __STD_C
static Void_t*	memory(Vmalloc_t* vm, Void_t* caddr,
		size_t oldsize, size_t newsize, Vmdisc_t* disc)
#else
static Void_t*	memory(vm, caddr, oldsize, newsize, disc)
Vmalloc_t*	vm;
Void_t*		caddr;
size_t		oldsize;
size_t		newsize;
Vmdisc_t*	disc;
#endif
{
	if(!Avail)
	{	Avail = Buf = (Vmuchar_t*)(&Algn[0]);
		Endbuf = Buf + sizeof(Algn);
	}

	if(oldsize)
		return NIL(Void_t*);

	Count += 1;
	caddr = (Void_t*)Avail;
	Avail += newsize;
	if(Avail >= Endbuf)
		terror("No more buffer");

	return caddr;
}

static int except(Vmalloc_t* vm, int type, Void_t* data, Vmdisc_t* disc)
{
	/* make the eventual handle be a part of our memory */
	if(type == VM_OPEN && data)
		*((Void_t**)data) = data;
	return 0;
}

static Vmdisc_t	Disc = {memory, except, 64};

#if __STD_C
static int walk(Vmalloc_t* vm, Void_t* addr, size_t size, Vmdisc_t* disc, Void_t* handle)
#else
static int walk(vm, addr, size, disc)
Vmalloc_t*	vm;
Void_t*		addr;
size_t		size;
Vmdisc_t*	disc;
#endif
{
	if(disc == &Disc)
		Walk += 1;
	if(handle != &Walk)
		terror("handle != &Walk");
	return 0;
}

tmain()
{
	Void_t		*m1, *m2, *m3, *m4;
	Vmalloc_t	*vm1, *vm2, *vm3, *vm4;

	if(!(vm1 = vmopen(&Disc,Vmbest,0)) )
		terror("Failed to open vm1");
	if((Vmuchar_t*)vm1 < Buf || (Vmuchar_t*)vm1 >= Endbuf)
		terror("Failed to get vm1 memory to be ours");

	if(!(vm2 = vmopen(&Disc,Vmbest,0)) )
		terror("Failed to open vm2");
	if((Vmuchar_t*)vm2 < Buf || (Vmuchar_t*)vm2 >= Endbuf)
		terror("Failed to get vm2 memory to be ours");

	if(!(vm3 = vmopen(&Disc,Vmbest,0)) )
		terror("Failed to open vm3");
	if((Vmuchar_t*)vm3 < Buf || (Vmuchar_t*)vm3 >= Endbuf)
		terror("Failed to get vm3 memory to be ours");

	Disc.exceptf = NIL(Vmexcept_f);
	if(!(vm4 = vmopen(&Disc,Vmbest,0)) )
		terror("Failed to open vm4");
	if((Vmuchar_t*)vm4 >= Buf && (Vmuchar_t*)vm4 < Endbuf)
		terror("vm4 memory should not be ours");

	if(!(m1 = vmalloc(vm1,1024)) )
		terror("vmalloc failed on m1");
	if(!(m2 = vmalloc(vm2,1024)) )
		terror("vmalloc failed on m2");
	if(!(m3 = vmalloc(vm3,1024)) )
		terror("vmalloc failed on m3");
	if(!(m4 = vmalloc(vm4,1024)) )
		terror("vmalloc failed on m4");

	if(!(m1 = vmresize(vm1, m1, 4*1024, VM_RSMOVE|VM_RSCOPY)) )
		terror("vmresize failed on m1");
	if(!(m2 = vmresize(vm2, m2, 4*1024, VM_RSMOVE|VM_RSCOPY)) )
		terror("vmresize failed on m2");
	if(!(m3 = vmresize(vm3, m3, 4*1024, VM_RSMOVE|VM_RSCOPY)) )
		terror("vmresize failed on m3");
	if(!(m4 = vmresize(vm4, m4, 4*1024, VM_RSMOVE|VM_RSCOPY)) )
		terror("vmresize failed on m4");

	vmsegwalk(NIL(Vmalloc_t*),walk,&Walk);
	if(Walk != Count)
		terror("Wrong walk count");
	tinfo("Number of segments: %d", Count);

	if(vmfree(vm1,m1) < 0 )
		terror("vmfree failed on m1");
	if(vmfree(vm2,m2) < 0 )
		terror("vmfree failed on m2");
	if(vmfree(vm3,m3) < 0 )
		terror("vmfree failed on m3");
	if(vmfree(vm4,m4) < 0 )
		terror("vmfree failed on m4");

	texit(0);
}
