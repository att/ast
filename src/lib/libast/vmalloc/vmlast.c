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

void _STUB_vmlast(){}

#else

#include	"vmhdr.h"

/*	Allocation with freeing and reallocing of last allocated block only.
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 01/16/1994, 03/30/2012.
*/

#define KEY_LAST		1001
#define LASTLOCK(lst,lcl)	((lcl) ? 0 : asolock(&(lst)->lock, KEY_LAST, ASO_LOCK) )
#define LASTOPEN(lst,lcl)	((lcl) ? 0 : asolock(&(lst)->lock, KEY_LAST, ASO_UNLOCK) )

typedef struct _vmlast_s
{	Vmdata_t	vmdt;
	unsigned int	lock;
	Block_t*	blk;	/* allocate from this	*/
	Vmuchar_t*	data;	/* start of free memory	*/
	ssize_t		size;	/* size of free memory	*/
	Vmuchar_t*	last;	/* last allocated block */
} Vmlast_t;

#if __STD_C
static Void_t* lastalloc(Vmalloc_t* vm, size_t size, int local)
#else
static Void_t* lastalloc(vm, size, local)
Vmalloc_t*	vm;
size_t		size;
int		local;
#endif
{
	Block_t		*blk;
	size_t		sz, blksz;
	size_t		origsz = size;
	Vmlast_t	*last = (Vmlast_t*)vm->data;

	LASTLOCK(last, local);

	size = size < ALIGN ? ALIGN : ROUND(size,ALIGN);

	last->last = NIL(Vmuchar_t*); /* wipe record of last allocation */

	if(last->size < size ) 
	{	if((blk = last->blk) ) /* try extending in place */
		{	blksz = SIZE(blk)&~BITS; 
			sz = blksz + (size - last->size);
			if((blk = (*_Vmsegalloc)(vm, blk, sz, VM_SEGALL|VM_SEGEXTEND)) )
			{	/**/DEBUG_ASSERT(blk == last->blk);
				/**/DEBUG_ASSERT((SIZE(blk)&~BITS) > blksz);
				last->size += (SIZE(blk)&~BITS) - blksz;
			}
		}
		if(!blk ) /* try getting a new block */
		{	if((blk = (*_Vmsegalloc)(vm, NIL(Block_t*), size, VM_SEGALL|VM_SEGEXTEND)) )
			{	/**/DEBUG_ASSERT((SIZE(blk)&~BITS) >= size);
				last->data = DATA(blk);
				last->size = SIZE(blk)&~BITS;
				last->blk  = blk;
			}
		}
	}

	if(last->size >= size) /* allocate memory */
	{	last->last = last->data;	
		last->data += size;
		last->size -= size;
	}
	
	if(last->last && !local && _Vmtrace)
		(*_Vmtrace)(vm, NIL(Vmuchar_t*), last->last, origsz, 0);

	LASTOPEN(last, local);

	return (Void_t*)last->last;
}

#if __STD_C
static int lastfree(Vmalloc_t* vm, Void_t* data, int local )
#else
static int lastfree(vm, data, local)
Vmalloc_t*	vm;
Void_t*		data;
int		local;
#endif
{
	ssize_t		size;
	Vmlast_t	*last = (Vmlast_t*)vm->data;

	if(!data)
		return 0;

	LASTLOCK(last, local);

	if(data != (Void_t*)last->last )
		data = NIL(Void_t*);
	else
	{	size = last->data - last->last;	/**/DEBUG_ASSERT(size > 0 && size%ALIGN == 0);
		last->data -= size;
		last->size += size;
		last->last = NIL(Vmuchar_t*);

		if(!local && _Vmtrace)
			(*_Vmtrace)(vm, (Vmuchar_t*)data, NIL(Vmuchar_t*), size, 0);
	}

	LASTOPEN(last, local);

	return data ? 0 : -1;
}

#if __STD_C
static Void_t* lastresize(Vmalloc_t* vm, Void_t* data, size_t size, int type, int local)
#else
static Void_t* lastresize(vm, data, size, type, local )
Vmalloc_t*	vm;
Void_t*		data;
size_t		size;
int		type;
int		local;
#endif
{
	Block_t		*blk;
	ssize_t		sz, oldz, blksz;
	Void_t		*origdt = data;
	size_t		origsz = size;
	Vmlast_t	*last = (Vmlast_t*)vm->data;

	if(!data)
	{	data = lastalloc(vm, size, local);
		if(data && (type&VM_RSZERO) )
			memset(data, 0, size);
		return data;
	}
	else if(size <= 0)
	{	(void)lastfree(vm, data, local);
		return NIL(Void_t*);
	}

	LASTLOCK(last, local);

	if(data != (Void_t*)last->last )
		data = NIL(Void_t*);
	else
	{	oldz = last->data - last->last; /**/DEBUG_ASSERT(oldz > 0 && oldz%ALIGN == 0);
		size = ROUND(size, ALIGN);
		if(size <= oldz) /* getting smaller */
		{	sz = oldz - size;
			last->data -= sz;
			last->size += sz;
		}
		else /* getting larger */
		{	if((oldz + last->size) < size && (blk = last->blk) != NIL(Block_t*) )
			{	/* try to extend in place */
				blksz = SIZE(blk)&~BITS;
				sz = blksz + size - (oldz + last->size);
				if((blk = (*_Vmsegalloc)(vm, blk, sz, VM_SEGALL|VM_SEGEXTEND)) )
				{	/**/DEBUG_ASSERT((SIZE(blk)&~BITS) >= sz);
					/**/DEBUG_ASSERT(blk == last->blk);
					last->size += (SIZE(blk)&~BITS) - blksz;
				}
			}

			if((oldz + last->size) < size && (type&VM_RSMOVE) )
			{	/* try to get new memory */
				if((blk = (*_Vmsegalloc)(vm, NIL(Block_t*), size, VM_SEGALL|VM_SEGEXTEND)) )
				{	/**/DEBUG_ASSERT((SIZE(blk)&~BITS) >= size);
					last->size = SIZE(blk)&~BITS;
					last->data = (Vmuchar_t*)DATA(blk);
					last->last = NIL(Vmuchar_t*);
					last->blk  = blk;
				}
			}

			if((oldz + last->size) < size)
				data = NIL(Void_t*);
			else
			{	if(data != (Void_t*)last->last)
				{	/* block moved, reset location */
					last->last = last->data;
					last->data += oldz;
					last->size -= oldz;

					if(type&VM_RSCOPY)
						memcpy(last->last, data, oldz);

					data = (Void_t*)last->last;
				}

				if(type&VM_RSZERO)
					memset(last->last+oldz, 0, size-oldz);

				last->data += size-oldz;
				last->size -= size-oldz;
			}
		}
	}	

	if(data && !local && _Vmtrace)
		(*_Vmtrace)(vm, (Vmuchar_t*)origdt, (Vmuchar_t*)data, origsz, 0);

	LASTOPEN(last, local);

	return (Void_t*)data;
}


#if __STD_C
static Void_t* lastalign(Vmalloc_t* vm, size_t size, size_t align, int local)
#else
static Void_t* lastalign(vm, size, align, local)
Vmalloc_t*	vm;
size_t		size;
size_t		align;
int		local;
#endif
{
	Vmuchar_t	*data;
	size_t		algn;
	size_t		 orgsize = size, orgalign = align;
	Vmlast_t	*last = (Vmlast_t*)vm->data;

	if(size <= 0 || align <= 0)
		return NIL(Void_t*);

	LASTLOCK(last, local);

	size = ROUND(size,ALIGN);
	align = (*_Vmlcm)(align, 2*sizeof(Block_t));

	if((data = (Vmuchar_t*)KPVALLOC(vm, size + align, lastalloc)) )
	{	if((algn = (size_t)(VMLONG(data)%align)) != 0)
		{	/* move forward for required alignment */	
			data += align - algn; /**/DEBUG_ASSERT((VMLONG(data)%align) == 0);
			last->last = data;
		}
	}

	if(data && !local && _Vmtrace)
		(*_Vmtrace)(vm, NIL(Vmuchar_t*), data, orgsize, orgalign);

	LASTOPEN(last, local);

	return (Void_t*)data;
}

static int laststat(Vmalloc_t* vm, Vmstat_t* st, int local)
{
	Vmlast_t	*last = (Vmlast_t*)vm->data;

	if(!st) /* just returning the lock state */
		return last->lock ? 1 : 0;

	LASTLOCK(last, local);

	if(last->last)
	{	st->n_busy = 1;
		st->s_busy = last->data - last->last;
	}
	if(last->data)
	{	st->n_free = 1;
		st->s_free = last->size;
	}

	LASTOPEN(last, local);
	return 0;
}

static int lastevent(Vmalloc_t* vm, int event, Void_t* arg)
{
	Vmlast_t	*last;

	if(event == VM_OPEN ) /* return the size of Vmpool_t */
	{	if(!arg)
			return -1;
		*((ssize_t*)arg) = sizeof(Vmlast_t);
	}
	else if(event == VM_ENDOPEN) /* start as if region was cleared */
	{	if(!(last = (Vmlast_t*)vm->data) )
			return -1;
		last->lock = 0;
		last->blk  = NIL(Block_t*);
		last->data = NIL(Vmuchar_t*);
		last->size = 0;
		last->last = NIL(Vmuchar_t*);
	}

	return 0;
}

/* Public method for free-1 allocation */
static Vmethod_t _Vmlast =
{	lastalloc,
	lastresize,
	lastfree,
	0,
	laststat,
	lastevent,
	lastalign,
	VM_MTLAST
};

__DEFINE__(Vmethod_t*,Vmlast,&_Vmlast);

#ifdef NoF
NoF(vmlast)
#endif

#endif
