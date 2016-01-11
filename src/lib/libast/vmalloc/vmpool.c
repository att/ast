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

void _STUB_vmpool(){}

#else

#include	"vmhdr.h"

/*	Method for pool allocation.
**	All elements in a pool have the same size.
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 01/16/94, 06/22/2012.
*/

/* data structures to keep pool elements */
#define FOOBAR	0xf00ba5
typedef struct _pool_s
{	struct _pool_s*	next;	/* linked list		*/
	unsigned int	foo;	/* free indicator	*/
} Pool_t;

typedef struct _vmpool_s
{	Vmdata_t	vmdt;
	ssize_t		size;	/* size of a block	*/
	ssize_t		nblk;	/* total #blocks	*/
	Pool_t*		free;	/* list of free blocks	*/
} Vmpool_t;

#define POOLSIZE(sz)	ROUND(ROUND((sz), sizeof(Pool_t)), ALIGN)

#ifdef DEBUG
static int	N_pool;	/* counter for Vmpool calls	*/
#endif

#if __STD_C
static Void_t* poolalloc(Vmalloc_t* vm, size_t size, int local)
#else
static Void_t* poolalloc(vm, size, local )
Vmalloc_t*	vm;
size_t		size;
int		local;
#endif
{
	Pool_t		*pl, *last, *list, *free;
	Block_t		*blk;
	Vmuchar_t	*dt, *enddt;
	Vmpool_t	*pool = (Vmpool_t*)vm->data;

	if(size <= 0)
		return NIL(Void_t*);

	if(size != pool->size )
	{	if(pool->size <= 0) /* first time */
			pool->size = size;
		else	return NIL(Void_t*);
	}

	list = last = NIL(Pool_t*);
	for(;;) /* grab the free list */
	{	if(!(list = pool->free) )
			break;
		if(asocasptr(&pool->free, list, NIL(Block_t*)) == list)
			break;
	}

	if(!list) /* need new memory */
	{	size = POOLSIZE(pool->size);
		if(!(blk = (*_Vmsegalloc)(vm, NIL(Block_t*), ROUND(2*size, pool->vmdt.incr), VM_SEGALL|VM_SEGEXTEND)) )
			return NIL(Void_t*);

		dt = DATA(blk); enddt = dt + BDSZ(blk);
		list = NIL(Pool_t*); last = (Pool_t*)dt;
		for(; dt+size <= enddt; dt += size)
		{	pl = (Pool_t*)dt;
			pl->foo = FOOBAR;
			pl->next = list; list = pl;
		}
		asoaddsize(&pool->nblk, BDSZ(blk)/size);
	}

	pl = list; /* grab 1 then reinsert the rest */
	if((list = list->next) )
	{	if(asocasptr(&pool->free, NIL(Block_t*), list) != NIL(Block_t*))
		{	if(!last)
				for(last = list;; last = last->next)
					if(!last->next)
						break;
			for(;;)
			{	last->next = free = pool->free;
				if(asocasptr(&pool->free, free, list) == free)
					break;
			}
		}
	}

	if(!local && pl && _Vmtrace)
		(*_Vmtrace)(vm, NIL(Vmuchar_t*), (Vmuchar_t*)pl, pool->size, 0);

	return (Void_t*)pl;
}

#if __STD_C
static int poolfree(Vmalloc_t* vm, Void_t* data, int local )
#else
static int poolfree(vm, data, local)
Vmalloc_t*	vm;
Void_t*		data;
int		local;
#endif
{
	Pool_t		*pl, *free;
	Vmpool_t	*pool = (Vmpool_t*)vm->data;

	if(!data)
		return 0;
	if(pool->size <= 0)
		return -1;

	pl = (Pool_t*)data;
	pl->foo = FOOBAR;
	for(;;)
	{	pl->next = free = pool->free;
		if(asocasptr(&pool->free, free, pl) == free)
			break;
	}
		
	if(!local && _Vmtrace)
		(*_Vmtrace)(vm, (Vmuchar_t*)data, NIL(Vmuchar_t*), pool->size, 0);

	return 0;
}

#if __STD_C
static Void_t* poolresize(Vmalloc_t* vm, Void_t* data, size_t size, int type, int local )
#else
static Void_t* poolresize(vm, data, size, type, local )
Vmalloc_t*	vm;
Void_t*		data;
size_t		size;
int		type;
int		local;
#endif
{
	NOTUSED(type);

	if(!data)
	{	data = poolalloc(vm, size, local);
		if(data && (type&VM_RSZERO) )
			memset(data, 0, size);
		return data;
	}
	else if(size == 0)
	{	(void)poolfree(vm, data, local);
		return NIL(Void_t*);
	}
	else	return NIL(Void_t*);
}

#if __STD_C
static Void_t* poolalign(Vmalloc_t* vm, size_t size, size_t align, int local)
#else
static Void_t* poolalign(vm, size, align, local)
Vmalloc_t*	vm;
size_t		size;
size_t		align;
int		local;
#endif
{
	NOTUSED(vm);
	NOTUSED(size);
	NOTUSED(align);
	return NIL(Void_t*);
}

/* get statistics */
static int poolstat(Vmalloc_t* vm, Vmstat_t* st, int local )
{
	size_t		size;
	Pool_t		*pl;
	Vmpool_t	*pool = (Vmpool_t*)vm->data;

	if(!st) /* just checking lock state */
		return 0;

	if(pool->size <= 0 )
		return -1;

	size = ROUND(pool->size, ALIGN);

	for(pl = pool->free; pl; pl = pl->next )
		st->n_free += 1;
	st->s_free = st->n_free * size;

	st->n_busy = pool->nblk - st->n_free;
	st->s_busy = st->n_busy * size;

	return 0;
}

static int poolevent(Vmalloc_t* vm, int event, Void_t* arg)
{
	Vmpool_t	*pool;

	if(event == VM_OPEN ) /* return the size of Vmpool_t */
	{	if(!arg)
			return -1;
		*((ssize_t*)arg) = sizeof(Vmpool_t);
	}
	else if(event == VM_ENDOPEN) /* start as if region was cleared */
	{	if(!(pool = (Vmpool_t*)vm->data) )
			return -1;
		pool->size = 0;
		pool->free = NIL(Pool_t*);
	}
	return 0;
}

/* Public interface */
static Vmethod_t _Vmpool =
{	poolalloc,
	poolresize,
	poolfree,
	0,
	poolstat,
	poolevent,
	poolalign,
	VM_MTPOOL
};

__DEFINE__(Vmethod_t*,Vmpool,&_Vmpool);

#ifdef NoF
NoF(vmpool)
#endif

#endif
