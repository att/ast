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

void _STUB_vmopen(){}

#else

#include	"vmhdr.h"

static char*    Version = "\n@(#)$Id: Vmalloc (AT&T Labs - Research) 2013-06-10 $\0\n";

/*	Opening a new region for memory allocation.
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, (01/16/1994, 01/16/2013).
*/
#ifdef DEBUG
static int	N_open;
#endif

#if __STD_C
Vmalloc_t* _vmopen(Vmalloc_t* vmo, Vmdisc_t* disc, Vmethod_t* meth, int mode)
#else
Vmalloc_t* _vmopen(vmo, disc, meth, mode)
Vmalloc_t*	vmo;	/* from previous vmopen if != 0	*/
Vmdisc_t*	disc;	/* discipline to get segments	*/
Vmethod_t*	meth;	/* method to manage space	*/
int		mode;	/* type of region		*/
#endif
{
	Vmalloc_t	*vm, *vmp, vmproto;
	Vmdata_t	*vd, vdproto;
	ssize_t		algn, incr, vdsz, vmsz, sgsz, size;
	Vmuchar_t	*addr, *base;
	Seg_t		*seg = (Seg_t*)Version; /* stop compiler's warning */
	int		rv, mt, initheap = 0;
	/**/DEBUG_COUNT(N_open);

	if(mode & VM_HEAPINIT)
		initheap = 1; /* we are building the heap */
	else if(_vmheapinit(Vmheap) != Vmheap ) /* make sure Vmheap is done first */
		return NIL(Vmalloc_t*);

	if(!meth || !disc || !disc->memoryf )
		return NIL(Vmalloc_t*);

	VMPAGESIZE(); /**/DEBUG_ASSERT(_Vmpagesize > 0);

	mode = (mode&VM_OPENFLAGS) | meth->meth; /* user-settable flags & method */

	vmp = &vmproto; /* avoid memory allocation at the start */
	memset(vmp, 0, sizeof(Vmalloc_t));
	memcpy(&vmp->meth, meth, sizeof(Vmethod_t));
	mt = vmp->meth.meth;
	vmp->meth.meth = 0;
	vmp->data = &vdproto;
	memset(vmp->data, 0, sizeof(Vmdata_t) );
	vmp->data->mode = mode;
	vmp->disc = disc;

	vd = NIL(Vmdata_t*);
	addr = NIL(Vmuchar_t*);
	size = 0;

	if(disc->exceptf)
	{	if((rv = (*disc->exceptf)(vmp,VM_OPEN,(Void_t*)(&addr),disc)) < 0)
		{	if(initheap)
				write(9, "vmalloc: panic: heap initialization error #1\n", 45);
			return NIL(Vmalloc_t*);
		}
		else if(rv == 0 ) /* normal case of region opening */
		{	if(addr) /* Vmalloc_t will be kept along with Vmdata_t */
				mode |= VM_MEMORYF;
		}
		else if(rv > 0) /* region opened before and now being restored */
		{	if(!(vd = (Vmdata_t*)addr)) /* addr should point to Vmdata_t */
			{	if(initheap)
					write(9, "vmalloc: panic: heap initialization error #2\n", 45);
				return NIL(Vmalloc_t*);
			}
			/**/DEBUG_ASSERT(VMLONG(vd)%ALIGN == 0);

			if(vd->mode & VM_MEMORYF) /* point addr to Vmalloc_t */
				addr -= ROUND(sizeof(Vmalloc_t), ALIGN);
		}
	}

	/* construct the Vmdata_t structure */
	if(!vd)
	{	/* amount to round to when getting raw memory via discipline */
		incr = disc->round <= 0 ? _Vmpagesize : ROUND(disc->round, _Vmpagesize);

		/* size of Vmalloc_t if embedded in the same initial segment */
		vmsz = (mode&VM_MEMORYF) ? ROUND(sizeof(Vmalloc_t),ALIGN) : 0;

		vdsz = 0; /* get actual size of Vmdata_t including method specific data */
		if(!meth->eventf || (*meth->eventf)(vmp, VM_OPEN, &vdsz) < 0 || vdsz <= 0 )
		{	if(initheap)
				write(9, "vmalloc: panic: heap initialization error #3\n", 45);
			return NIL(Vmalloc_t*);
		}
		vdsz = ROUND(vdsz, ALIGN);

		sgsz = ROUND(sizeof(Seg_t), ALIGN); /* size of segment structure */

		/* get initial memory segment containing Vmdata_t, Seg_t and some extra */
		size = vmsz + vdsz + sgsz + 8*_Vmpagesize;
		size = ROUND(size,incr); /**/DEBUG_ASSERT(size%ALIGN == 0 );
		if(!(base = (Vmuchar_t*)(*disc->memoryf)(vmp, NIL(Void_t*), 0, size, disc)) )
		{	if(initheap)
				write(9, "vmalloc: panic: heap initialization error #4\n", 45);
			return NIL(Vmalloc_t*);
		}
		if (_Vmassert & 0x10)
			memset(base, 0, size);
		else if (!(_Vmassert & 0x20))
			memset(base, 0, vmsz + vdsz + sgsz);

		/* make sure memory is properly aligned */
		if((algn = (ssize_t)(VMLONG(base)%ALIGN)) == 0 )
			addr = base;
		else	addr = base + (ALIGN-algn);
		/**/DEBUG_ASSERT(VMLONG(addr)%ALIGN == 0 );

		/* addresses of Vmdata_t and root segment */
		vd = (Vmdata_t*)(addr + vmsz); /**/DEBUG_ASSERT(VMLONG(vd)%ALIGN == 0);
		seg = (Seg_t*)(addr + vmsz + vdsz); /**/DEBUG_ASSERT(VMLONG(seg)%ALIGN == 0);

		/* set Vmdata_t data */
		vd->mode = mode;
		vd->incr = incr;
		vd->seg  = NIL(Seg_t*);
		vd->free = NIL(Block_t*);

		vd->lock = 1; /* initialize seg and add it to segment list */
		(*_Vmseginit)(vd, seg, base, size, 1);
		vd->lock = 0;
	}
	vmp->data = vd;
	vmp->meth.meth = mt;

	/* now make the region handle */
	if(vmo)
		vm = vmp;
	else
	{	if(initheap)
			vm = Vmheap;
		else if(vd->mode&VM_MEMORYF) /* keep in app-defined memory as Vmdata_t */
			vm = (Vmalloc_t*)addr;
		else if(!(vm = (Vmalloc_t*)vmalloc(Vmheap, sizeof(Vmalloc_t))) ) /* on the heap */
		{	(void)(*disc->memoryf)(vmp, vd->seg->base, vd->seg->size, 0, disc);
			if(initheap)
				write(9, "vmalloc: panic: heap initialization error #5\n", 45);
			return NIL(Vmalloc_t*);
		}
		*vm = *vmp;
	}

	if(size > 0 && meth->eventf) /* finalize internal method data structures */
		(void)(*meth->eventf)(vm, VM_ENDOPEN, NIL(Void_t*) );

	if(disc->exceptf) /* signaling to application that vmopen succeeded */
		(void)(*disc->exceptf)(vm, VM_ENDOPEN, NIL(Void_t*), disc);

	if(vmo)
	{	*vmo = *vm;
		vm = vmo;
	}
	else if(vm != Vmheap) /* locklessly adding vm to list of regions */
	{	Vmhold_t	*vh, *cvh;

		for(vh = _Vmhold; vh; vh = vh->next) /* try adding to an existing slot */
			if(asocasptr(&vh->vm, NIL(Vmalloc_t*), vm) == NIL(Vmalloc_t*) )
				break;
		if(!vh) /* need a new slot */
		{	if(!(vh = vmalloc(Vmheap, sizeof(Vmhold_t))) )
			{	vmclose(vm);
				if(initheap)
					write(9, "vmalloc: panic: heap initialization error #6\n", 45);
				return NIL(Vmalloc_t*);
			}
			else	vh->vm = vm;
			for(;;) 
			{	vh->next = cvh = _Vmhold;
				if(asocasptr(&_Vmhold, cvh, vh) == cvh)
					break;
			}
		}
	}

	return vm;
}

#if __STD_C
Vmalloc_t* vmopen(Vmdisc_t* disc, Vmethod_t* meth, int mode)
#else
Vmalloc_t* vmopen(vm, disc, meth, mode)
Vmdisc_t*	disc;	/* discipline to get segments	*/
Vmethod_t*	meth;	/* method to manage space	*/
int		mode;	/* type of region		*/
#endif
{
	return _vmopen(NIL(Vmalloc_t*), disc, meth, mode);
}

#endif
