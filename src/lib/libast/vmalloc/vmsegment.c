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

void _STUB_vmprivate(){}

#else

#include	"vmhdr.h"

/*	This file implements the subsystem to manage memory segments obtained
**	via the discipline memoryf() function. The file also contains a few
**	miscellaneous private functions for data formatting.
**
**	Blocks allocated here are called "superblocks" which will be further
**	partitioned by the respective methods for their own allocation usage.
**	Memory is allocated in a first-fit manner.
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 01/16/94, 12/04/2012.
*/

/* hint to regulate memory requests to discipline functions */
#if _ast_sizeof_size_t > 4 /* the address space is greater than 32-bit	*/
#define VM_SEGSIZE	(4*1024*1024)	/* lots of memory available here	*/
#else
#define VM_SEGSIZE	(64*1024)	/* perhaps more limited memory	*/
#endif

#ifdef DEBUG
static int	N_segalloc;

/* see if this is a block allocated by vmsegalloc() */
static isblock(Vmdata_t* vmdt, Block_t* blk)
{
	Seg_t	*seg;
	Block_t	*bp;

	/**/DEBUG_ASSERT(blk != NIL(Block_t*)); /* find containing segment */
	for(seg = vmdt->seg; seg; seg = seg->next)
		if(seg == SEG(blk) )
			break;

	/**/DEBUG_ASSERT(seg); /* check to see if blk is a valid block */
	for(bp = seg->begb; bp < seg->endb; bp = NEXT(bp) )
		if(bp == blk)
			return 1;

	return 0;
}

static isfree(Vmdata_t* vmdt, Block_t* blk)
{
	Block_t	*bp;

	/**/DEBUG_ASSERT(isblock(vmdt, blk) && !(SIZE(blk) & BUSY) );
	for(bp = vmdt->free; bp; bp = LINK(bp) )
	{	/**/DEBUG_ASSERT(!(SIZE(bp) & BUSY) );
		if(bp == blk)
			return 1;
	}
	return 0;
}
#endif /*DEBUG*/

/* Free blocks are kept in a singly linked list. Atomic ops are used to set pointers
** to reduce the chance of longjmp() leaving the list in an incomplete state.
*/
#define INSERT_BLOCK	0	/* insert a block at head of free list	*/
#define DELETE_BLOCK	1	/* delete a specific block		*/
#define DELETE_ENDB	2	/* delete the end block of a segment	*/

/* lock-less type op for arg on free list */
static Block_t* _vmfreelist(Vmdata_t* vmdt, Void_t* arg, int type)
{
	Block_t	*pb, *bp, *blk;
	/**/DEBUG_ASSERT(vmdt->lock != 0);

	if(type == INSERT_BLOCK)
	{	blk = (Block_t*)arg;
		do LINK(blk) = bp = vmdt->free; while (asocasptr(&vmdt->free, bp, blk) != bp);
	}
	else /* some sort of deletion */
	{	do
		{	for(pb = NIL(Block_t*), bp = vmdt->free; bp; pb = bp, bp = LINK(bp) )
				if((type == DELETE_BLOCK && bp == (Block_t*)arg) ||
				   (type == DELETE_ENDB && NEXT(bp) == ((Seg_t*)arg)->endb) )
					break;
			if(!bp) /* none matches deletion requirement */
				return NIL(Block_t*);

			if(!pb) /* bp should be at head of list */
				blk = asocasptr(&vmdt->free, bp, LINK(bp));
			else
				blk = asocasptr(&LINK(pb), bp, LINK(bp));
		} while(blk != bp);
	}

	return blk;
}

/* Clear locks so that allocation can proceed. */
void vmclrlock(int all)
{
	unsigned int	threadid;
	Vmhold_t	*vh;
	Vmalloc_t	*vm;

	threadid = all ? 0 : asothreadid(); 

	if(threadid) /* release sbrklock */
		asocasint(&_Vmsbrklock, threadid, 0); /* only if done by thread */
	else while(_Vmsbrklock != 0) /* or in all cases */
		asosubint(&_Vmsbrklock, _Vmsbrklock);

	for(vh = _Vmhold; ; vh = vh->next) 
	{	if((vm = vh ? vh->vm : Vmheap) == NIL(Vmalloc_t*) )
			continue;

		if(threadid) 
		{	asocasint(&vm->data->lock, threadid, 0); /* lock on segments */
			asocasint(&vm->data->ulck, threadid, 0); /* lock on user data */
		}
		else
		{	while(vm->data->lock)
				asosubint(&vm->data->lock, vm->data->lock);
			while(vm->data->ulck)
				asosubint(&vm->data->ulck, vm->data->ulck);
		}

		if(!vh) /* done checking */
			break;
	}
}

/* Walks all segments held in region "vm" or all regions if vm == NULL */
int vmsegwalk(Vmalloc_t* vm, Vmseg_f segf, Void_t* handle)
{	
	Vmhold_t	*vh;
	Seg_t		*seg;
	Vmalloc_t	*todo;
	int		rv = 0;

	for(vh = _Vmhold;; vh = vh->next)
	{	if((todo = vm ? vm : vh ? vh->vm : Vmheap) && todo->data )
			for(seg = todo->data->seg; seg; seg = seg->next)
				if((rv = (*segf)(todo, seg->base, seg->size, todo->disc, handle)) < 0 )
					return -1;
		if(vm || !vh)
			break;
	}
	return rv;
}

/* find the segment containing a particular address */
Void_t* vmsegfind(Vmalloc_t* vm, Void_t* addr)
{
	Seg_t		*seg;

	if(vm->data)
		for(seg = vm->data->seg; seg; seg = seg->next)
			if((Vmuchar_t*)addr >= (Vmuchar_t*)seg->base &&
			   (Vmuchar_t*)addr <  ((Vmuchar_t*)seg->base + seg->size) )
				return (Void_t*)seg->base;

	return NIL(Void_t*);
}

/* lock-less seg initialization with containing region vmdt and actual memory [base,size] */
static Block_t* _vmseginit(Vmdata_t* vmdt, Seg_t* seg, Vmuchar_t* base, ssize_t size, int insert)
{
	/**/DEBUG_ASSERT((Vmuchar_t*)seg >= base && (Vmuchar_t*)seg < (base+size) );
	seg->vmdt = vmdt;
	seg->base = base;
	seg->size = size;

	/* available memory in Seg_t for allocation usage */
	size = (base+size) - SEGDATA(seg);
	size = (size/ALIGN)*ALIGN; /* must be multiple of ALIGN's */

	/* block at start of memory */
	seg->begb = (Block_t*)SEGDATA(seg);
	SEG(seg->begb) = seg;
	SIZE(seg->begb) = (size - 2*sizeof(Head_t));

	/* sentinel block at end of memory */
	seg->endb = NEXT(seg->begb);
	SEG(seg->endb) = seg;
	SIZE(seg->endb) = BUSY; /* permanently busy */

	/* update segment boundaries */
	base = seg->base;
	while(!vmdt->segmin || base < vmdt->segmin)
		asocasptr(&vmdt->segmin, vmdt->segmin, base);
	base += seg->size;
	while(!vmdt->segmax || base > vmdt->segmax)
		asocasptr(&vmdt->segmax, vmdt->segmax, base);

	/* add the free block to the free list */
	if(insert)
		_vmfreelist(vmdt, (Void_t*)seg->begb, INSERT_BLOCK);
	else
		SIZE(seg->begb) |= BUSY;

	/* update segment list */
	do seg->next = vmdt->seg; while(asocasptr(&vmdt->seg, seg->next, seg) != seg->next);

	return seg->begb;
}

static void _vmsegmerge(Vmdata_t* vmdt, Block_t* blk)
{
	ssize_t		size;
	Block_t*	next;

	for(;;) /* forward merge as much as possible */
	{	next = NEXT(blk); /**/DEBUG_ASSERT(SEG(next) == SEG(blk));
		if(!(SIZE(next)&BUSY) )
		{	_vmfreelist(vmdt, (Void_t*)next, DELETE_BLOCK);
			size = SIZE(blk) + BDSZ(next) + sizeof(Head_t);
			asocassize(&SIZE(blk), SIZE(blk), size); /**/DEBUG_ASSERT(SIZE(blk) == size);
		}
		else	break;
	}
}

/* add new memory to blk if not NULL; or make a new block */
static Block_t* _vmsegalloc(Vmalloc_t* vm, Block_t* blk, ssize_t size, int type)
{
	unsigned int	key;
	int		heisus;
	ssize_t		sz, segsz;
	Block_t		*np, *endb;
	Seg_t		*seg;
	Vmuchar_t	*base;
	Vmdata_t	*vmdt = vm->data;
	Vmdisc_t	*disc = vm->disc;
	static size_t	Segunit = 0;
	/**/DEBUG_COUNT(N_segalloc);
	/**/DEBUG_ASSERT(!blk || (isblock(vmdt, blk) && (SIZE(blk)&BUSY) ) );
	/**/DEBUG_ASSERT(_Vmpagesize > 0 && _Vmpagesize%ALIGN == 0 );

/* transition to a fixed place to unlock before returning */
#undef	RETURN
#define RETURN(x)	do {(x); goto re_turn; } while(0)
	if(Segunit == 0)
		Segunit = ROUND(8*1024, _Vmpagesize);
	size = size < Segunit ? Segunit : ROUND(size, Segunit);

	key = asothreadid();
	if(vmdt->lock == key)
	{	heisus = 1; /* we have met the enemy and he is us */
		if(blk && (type&VM_SEGEXTEND) ) /* no physical extension when he is us */
		{	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: VM_SEGEXTEND: %s\n", __FILE__, __LINE__, "we have met the enemy and he is us");
			RETURN(blk = NIL(Block_t*));
		}
		if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: %s\n", __FILE__, __LINE__, "we have met the enemy and he is us");
	}
	else
	{	heisus = 0;
		asolock(&vmdt->lock, key, ASO_LOCK);

		if(blk) /* try extending an existing block in place */
		{	_vmsegmerge(vmdt, blk); 
			if(BDSZ(blk) >= size )
				RETURN(blk);
			if(NEXT(blk) != ((Seg_t*)SEG(blk))->endb )
				RETURN(blk = NIL(Block_t*));
		}
		else /* see if anything available for requested size */
		{	for(blk = vmdt->free; blk; blk = LINK(blk))
			{	_vmsegmerge(vmdt, blk);
				if(BDSZ(blk) >= size)
				{	_vmfreelist(vmdt, (Void_t*)blk, DELETE_BLOCK);
					RETURN(blk);
				}
			}
		}

		if(!(type&VM_SEGEXTEND) ) /* no physical extension */
			RETURN(blk = NIL(Block_t*));

		if(blk) 
		{	seg = SEG(blk);
			/**/DEBUG_ASSERT((SIZE(blk)&BUSY) && NEXT(blk) == seg->endb);
		}
		else
		{	seg = vmdt->seg; 
			blk = seg->iffy ? NIL(Block_t*) : _vmfreelist(vmdt, (Void_t*)seg, DELETE_ENDB);
			/**/DEBUG_ASSERT(!blk || (SEG(blk) == seg && NEXT(blk) == seg->endb));
		}

		if(asocasint(&seg->iffy, 0, 1) == 0)
		{	/* a longjmp() below will make seg not further extendible */
			if((sz = blk ? BDSZ(blk) : 0) < size) /* try extending segment */
			{	/**/DEBUG_ASSERT((blk && SEG(blk) == seg) || (!blk) );

				/* amount of new memory to request */
				segsz = seg->size + (size - sz) + sizeof(Block_t) + Segunit;
				segsz = ROUND(segsz, vmdt->incr);
				if((sz = segsz - seg->size) <= 0 ) /* wrapped around, not good! */
				{	seg->iffy = 0;
					RETURN(blk = NIL(Block_t*));
				}

				/* Be careful with editing the below section of code. It was written to
				** allow longjmp() while keeping the memory layout consistent.
				*/
				base = (Vmuchar_t*)(*disc->memoryf)(vm, seg->base, seg->size, segsz, disc);
				if(base == seg->base) /* successful extension */
				{	/* keeping segment size right is first priority */
					seg->size = segsz;

					if(!vmdt->segmin || base < vmdt->segmin)
						vmdt->segmin = base;
					if(!vmdt->segmax || (base+segsz) > vmdt->segmax)
						vmdt->segmax = base+segsz;

					/* Next, construct the new end block. Note that the old end block
					** is still intact so that a test for next free block that touches
					** it will continue to fail if an unfortunate longjmp() occurs.
					*/
					endb = seg->endb; /* current end block */
					np = (Block_t*)((Vmuchar_t*)endb + sz); /* new end block */
					SEG(np) = seg;
					SIZE(np) = BUSY;
					seg->endb = np;

					/* now we can construct the new block */
					if(blk) /* existing block got extended */
					{	/**/DEBUG_ASSERT(NEXT(blk) == endb);
						SIZE(blk) += sz;
					}
					else /* new allocatable end block */
					{	blk = endb;
						SIZE(blk) = sz - sizeof(Head_t);
					}
					/**/DEBUG_ASSERT(NEXT(blk) == seg->endb);
				}
			}
			seg->iffy = 0; /* segment info is now completed */
		}
	}

	if((sz = blk ? BDSZ(blk) : 0) < size ) /* must make a new segment */
	{	if(blk) 
		{	if(SIZE(blk)&BUSY ) /* unextensible busy block */
				RETURN(blk = NIL(Block_t*));

			_vmfreelist(vmdt, (Void_t*)blk, INSERT_BLOCK); 
			blk = NIL(Block_t*);
		}

		/* make sure that new segment size isn't too large to wrap around */
		segsz = size + sizeof(Seg_t) + sizeof(Block_t) + Segunit;
		if(segsz <= size || (segsz = ROUND(segsz,vmdt->incr)) <= size) 
			RETURN(blk = NIL(Block_t*)); /* did wrap around */

		if(!(base = (Vmuchar_t*)(*disc->memoryf)(vm, NIL(Void_t*), 0, segsz, disc)) )
		{	if(disc->exceptf) /* announce that no more memory is available */
				(void)(*disc->exceptf)(vm, VM_NOMEM, (Void_t*)segsz, disc);
			RETURN(blk = NIL(Block_t*));
		}

		/* segment must start at an aligned address */
		if((sz = (size_t)(VMLONG(base)%ALIGN)) == 0)
			seg = (Seg_t*)base;
		else	seg = (Seg_t*)(base + ALIGN-sz);
		blk = _vmseginit(vmdt, seg, base, segsz, 0);
	}

re_turn:
	if(blk) /* keep any excess memory for future allocations */
	{	sz = BDSZ(blk); /**/DEBUG_ASSERT(sz >= size );
		if(!(type&VM_SEGALL) && (sz - size) > (Segunit + sizeof(Block_t)) )
		{	/* make block of unused space */	
			np = (Block_t*)((Vmuchar_t*)blk + size + sizeof(Head_t));
			SEG(np) = SEG(blk);
			SIZE(np) = sz - size - sizeof(Head_t); /**/DEBUG_ASSERT(BDSZ(np) >= Segunit);

			/* this makes new blk and causes np to be visible */
			size |= SIZE(blk)&BUSY;
			asocassize(&SIZE(blk), SIZE(blk), size);

			_vmfreelist(vmdt, (Void_t*)np, INSERT_BLOCK);
		}
		SIZE(blk) |= BUSY; /**/DEBUG_ASSERT(BDSZ(blk) >= Segunit);
	}

	if(heisus == 0)
	{	/**/DEBUG_ASSERT(vmdt->lock == key);
		asolock(&vmdt->lock, key, ASO_UNLOCK);
		/**/DEBUG_ASSERT(vmdt->lock != key);
	}

	return blk;
}

/* free a superblock back to the free list. this is currently unused
** but the lock-less algorithm is cool so we keep it for future use.
*/
static void _vmsegfree(Vmalloc_t* vm, Block_t* blk)
{
	SIZE(blk) &= ~BUSY;
	_vmfreelist(vm->data, (Void_t*)blk, INSERT_BLOCK);
}

/* copy a string and add a special end of character */
static char* _vmstrcpy(char* to, const char* from, int endc)
{	int	n;

	n = strlen(from);
	memcpy(to,from,n);
	to += n;
	if((*to = endc) )
		to += 1;
	return to; /* return next available location in 'to' */
}

/* convert a long value to ascii.
** type=0: base-16, >0: unsigned base-10, <0: signed base-10
*/
static char* _vmitoa(Vmulong_t v, int type)
{
	char		*s;
	static char	buf[256];

	*(s = &buf[255]) = 0;
	if(type == 0)		/* base-16 */
	{	char	*digit = "0123456789abcdef";
		do *--s = digit[v&0xf];
		while((v >>= 4) > 0 );
		*--s = 'x';
		*--s = '0';
	}
	else if(type > 0)	/* unsigned base-10 */
	{	do *--s = (char)('0' + (v%10));
		while((v /= 10) > 0);
	}
	else			/* signed base-10 */
	{	int	negative = ((long)v < 0) ? 1 : 0;
		if(negative)
			v = (Vmulong_t)(-((long)v));
		do *--s = (char)('0' + (v%10));
		while((v /= 10) > 0);
		if(negative)
			*--s = '-';
	}

	return s;
}

/* compute greatest common divisor and least common multiple */
static ssize_t _vmgcd(ssize_t one, ssize_t two)
{
	if(one == two)
		return one;
	else if(one == 0 || two == 0)
		return one ? one : two;
	else if(one == 1 || two == 1)
		return 1;
	else if(one&1)
	{	if(two&1)
			return one > two ? _vmgcd((one-two)/2, two) : _vmgcd(one, (two-one)/2);
		else	return _vmgcd(one, two/2);
	}
	else
	{	if(two&1)
			return _vmgcd(one/2, two);
		else	return 2*_vmgcd(one/2, two/2);
	}
}

static ssize_t _vmlcm(ssize_t one, ssize_t two)
{
	if(one == 0 || two == 0)
		return 0;
	else	return (one/_vmgcd(one,two))*two;
}

/* Externally visible names to all modules but local to library */
Vmextern_t	_Vmextern =
{	_vmseginit,								/* _Vmseginit	*/
	_vmsegalloc,								/* _Vmsegalloc	*/
	_vmsegfree,								/* _Vmsegfree	*/
	_vmstrcpy,								/* _Vmstrcpy	*/
	_vmitoa,								/* _Vmitoa	*/
	_vmlcm,									/* _Vmlcm	*/
	NIL(void(*)_ARG_((Vmalloc_t*, Vmuchar_t*,Vmuchar_t*,size_t,size_t))),	/* _Vmtrace	*/
	NIL(int(*)_ARG_((Vmuchar_t*,size_t))),					/* _Vmchkmem	*/
	0,									/* _Vmmemmin	*/
	0,									/* _Vmmemmax	*/
	0,									/* _Vmmemaddr	*/
	0,									/* _Vmmemsbrk	*/
	0,									/* _Vmhold	*/
	0,									/* _Vmpagesize	*/
	VM_SEGSIZE,								/* _Vmsegsize	*/
	0,									/* _Vmsbrklock	*/
	0									/* _Vmassert	*/
};

#endif
