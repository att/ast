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
#include	<sys/types.h>
#include	<sys/shm.h>
#include	<sys/ipc.h>
#include	<signal.h>
#include	<setjmp.h>

#if _mem_mmap_anon
#include	<sys/mman.h>
#ifndef MAP_ANON
#ifdef	MAP_ANONYMOUS
#define	MAP_ANON	MAP_ANONYMOUS
#else
#define MAP_ANON	0
#endif /*MAP_ANONYMOUS*/
#endif /*MAP_ANON*/
#endif /*_mem_mmap_anon*/

/* Heuristic to suggest an address usable for mapping shared memory
**
** Written by Kiem-Phong Vo, phongvo@gmail.com, 07/07/2012
*/

/* see if a given range of address is available for mapping */
#define VMCHKMEM	1 /* set this to zero if signal&sigsetjmp don't work */

#if VMCHKMEM

/*
 * NOTE: all (*_Vmchkmem)() calls are locked by _Vmsbrklock
 *	 make sure future usage follows suit
 */

typedef void (*Sighandler_f)_ARG_((int));

static volatile int	peek;
static sigjmp_buf	jmp;

static void sigsegv(int sig)
{	
	signal(sig, sigsegv);
	siglongjmp(jmp, 1);
}

static int _vmchkmem(Vmuchar_t* area, size_t size)
{
	Sighandler_f	oldsigsegv;
	int		available;

	if (!(_Vmassert & VM_check_seg))
		return 1;
	oldsigsegv = (Sighandler_f)signal(SIGSEGV, sigsegv);
	if (!(available = sigsetjmp(jmp, 1)))
		peek = area[0];
	if (available && !(available = sigsetjmp(jmp, 1)))
		peek = area[size-1];
	if (available && !(available = sigsetjmp(jmp, 1)))
		peek = area[size/2];
	signal(SIGSEGV, oldsigsegv);
	return available;
}

#else

#define	_vmchkmem(a,z)	(1) /* beware of unbounded optimism! */

#endif /*VMCHKMEM*/

/* return page size */
ssize_t _vmpagesize(void)
{
	if (_Vmpagesize <= 0)
	{
#if _lib_getpagesize
		if ((_Vmpagesize = getpagesize()) <= 0)
#endif
			_Vmpagesize = VM_PAGESIZE;
		_Vmpagesize = (*_Vmlcm)(_Vmpagesize, ALIGN);
	}
#if VMCHKMEM
	_Vmchkmem = _vmchkmem; /* _vmchkmem() can check memory availability */
#endif
	return _Vmpagesize;
}

int _vmboundaries(void)
{
	ssize_t		memz, z;
	unsigned long	left, rght, size;
	int		shmid; /* shared memory id */
	Vmuchar_t	*tmp, *shm, *min, *max;

	VMPAGESIZE();
#if !_WINIX
	/* try to get a shared memory segment, memz is the successful size */
	memz = sizeof(void*) < 8 ? 1024*1024 : 64*1024*1024;
	for(; memz >= _Vmpagesize; memz /= 2)
	{	z = ROUND(memz, _Vmpagesize);
		if((shmid = shmget(IPC_PRIVATE, z, IPC_CREAT|0600)) >= 0 ) 
			break;
	}
	if(memz >= _Vmpagesize) /* did get a shared segment */
		memz = ROUND(memz, _Vmpagesize);
	else
	{	/**/DEBUG_MESSAGE("shmget() failed");
		return (int)_Vmpagesize;
	}

	/* the stack and the heap in Unix programs are conventionally set
	** at opposite ends of the available address space. So, we use them
	** as candidate boundaries for mappable memory.
	*/
	min = (Vmuchar_t*)sbrk(0); min = (Vmuchar_t*)ROUND((unsigned long)min, _Vmpagesize); /* heap  */
	max = (Vmuchar_t*)(&max);  max = (Vmuchar_t*)ROUND((unsigned long)max, _Vmpagesize); /* stack */
	if(min > max)
		{ tmp = min; min = max; max = tmp; }

	/* now attach a segment to see where it falls in the range */
	if(!(shm = shmat(shmid, NIL(Void_t*), 0600)) || shm == (Vmuchar_t*)(-1) )
	{	/**/DEBUG_MESSAGE("shmat() failed first NULL attachment");
		goto done;
	}
	else	shmdt((Void_t*)shm);
	if(shm < min || shm > max )
	{	/**/DEBUG_MESSAGE("shmat() got an out-of-range address");
		goto done;
	}

	/* Heuristic: allocate address in the larger side */
	left = shm - min;
	rght = max - shm;

	min = max = shm; /* compute bounds of known mappable memory */
	for(size = 7*(left > rght ? left : rght)/8; size > memz; size /= 2 )
	{	size = ROUND(size, _Vmpagesize);
		shm = left > rght ? max-size : min+size;
		if((tmp = shmat(shmid, shm, 0600)) == shm )
		{	shmdt((Void_t*)tmp);
			if(left > rght)
				min = shm;
			else	max = shm;
			break;
		}
	}

	if((min+memz) >= max ) /* no mappable region of memory */
	{	/**/DEBUG_MESSAGE("vmmaddress: No mappable memory region found");
		goto done;
	}

	/* search outward from last computed bound for a better bound */
	for(z = memz; z < size; z *= 2 )
	{	shm = left > rght ? min-z : max+z;
		if((tmp = shmat(shmid, shm, 0600)) == shm )
			shmdt((Void_t*)tmp);
		else /* failing to attach means at limit or close to it */
		{	if(left > rght)
				min -= z/2;
			else	max += z/2;
			break;
		}
	}

	/* amount to offset from boundaries to avoid random collisions */
	z = (max - min)/(sizeof(Void_t*) > 4 ? 4 : 8);
	z = ROUND(z, _Vmpagesize);

	/* these are the bounds that we can use */
	_Vmmemmin = min;
	_Vmmemmax = max;

	_Vmmemaddr = max - z; /* address usable by vmmaddress() */
	_Vmmemsbrk = NIL(Vmuchar_t*); /* address usable for sbrk() simulation */

#if _mem_mmap_anon /* see if we can simulate sbrk(): memory grows from low to high */
	/* map two consecutive pages to see if they come out adjacent */
	tmp = (Void_t*)mmap((Void_t*)(min+z), _Vmpagesize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
	shm = (Void_t*)mmap((Void_t*)(tmp+_Vmpagesize), _Vmpagesize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
	if(tmp && tmp != (Vmuchar_t*)(-1) )
		munmap((Void_t*)tmp, _Vmpagesize);
	if(shm && shm != (Vmuchar_t*)(-1) )
		munmap((Void_t*)shm, _Vmpagesize);

	if(tmp && tmp != (Vmuchar_t*)(-1) && shm && shm != (Vmuchar_t*)(-1) )
	{	_Vmmemsbrk = shm+_Vmpagesize; /* mmap starts from here */

		if(tmp >= (_Vmmemmin + (_Vmmemmax - _Vmmemmin)/2) ||
		   shm >= (_Vmmemmin + (_Vmmemmax - _Vmmemmin)/2) ||
		   shm < tmp ) /* mmap can be used but needs MAP_FIXED! */
		{
#if !VMCHKMEM
			_Vmmemsbrk = NIL(Vmuchar_t*); /* no memory checking, must use sbrk() */
#endif /*VMCHKMEM*/
		}
	}
#endif /*_mem_mmap_anon_*/

#endif /*!_WINIX*/

done:	(void)shmctl(shmid, IPC_RMID, 0);
	return 0;
}

/* Function to suggest an address usable for mapping shared memory. */
Void_t* vmmaddress(size_t size)
{
	Vmuchar_t	*addr, *memaddr;

	VMBOUNDARIES();
	if(_Vmmemaddr)
		for(size = ROUND(size, _Vmpagesize); (addr = (memaddr = _Vmmemaddr) - size) >= _Vmmemmin; )
			if(asocasptr(&_Vmmemaddr, memaddr, addr) == memaddr && _vmchkmem(addr, size))
				return addr;
	return NIL(Void_t*);
}
