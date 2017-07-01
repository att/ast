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

void _STUB_vmbest(){}

#else

#include	"vmhdr.h"

/*	Best-fit allocation method extended for concurrency.
**
**	The memory arena of a region is organized as follows:
**	1. Raw memory are (possibly noncontiguous) segments (Seg_t) obtained
**	   via Vmdisc_t.memoryf. Segments may be further partitioned into
**	   "superblocks" on requests from memory packs (more below). The
**	   collection of superblocks is called the "segment pool". 
**	2. A region consists of one or more packs. Each pack typically holds
**	   one or more superblocks from the segment pool. These blocks are
**	   partitioned into smaller blocks for allocation. General allocation
**	   is done in a best-fit manner with a splay tree holding free blocks
**	   by size. Caches of small blocks are kept to speed up allocation.
**	3. Packs are created dynamically and kept in an array. An allocation
**	   request uses the ASO's ID of the calling thread/process to hash
**	   into this array and search for packs with available memory. Thus,
**	   allocation is thread-preferred but not thread-specific since
**	   all packs may be used by all threads/processes. 
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 01/16/1994, 12/21/2012.
*/

/* drop annoying macros courtesy various native system implementations */
#undef	small
#undef	DELETE

/**/DEBUG_DECLARE(int, N_free)	 	/* number of bestfree calls		*/
/**/DEBUG_DECLARE(int, N_alloc) 	/* number of bestalloc calls		*/
/**/DEBUG_DECLARE(int, N_resize) 	/* number of bestresize calls		*/
/**/DEBUG_DECLARE(int, N_small) 	/* number of bestsmallalloc calls	*/

#define KEY_BEST	(0x19770825) /* locked together forever - ILUN	*/

#define PK_INITIALIZING	((Pack_t*)(~((unsigned long)0)) )
#define	PK_ARRAY	256	/* size of pack array			*/
#define	PK_ALLOW	128	/* min #packs allowed to be created	*/

/* Small requests are rounded to 0%SM_RNDx */
#define SM_RND0	(1*ALIGN)	/* round value:  1*ALIGN  ==   16	*/
#define SM_BIT0	4		/* (1<<SM_BIT0) == SM_RND0		*/
#define SM_CNT0	16	   	/* # caches as rounded by SM_RND0	*/
#define SM_IDX0	0		/* starting cache index of this group	*/
#define SM_MAX0	(SM_CNT0*SM_RND0)
	
#define SM_RND1	(2*ALIGN)	/* round value:  2*ALIGN  ==   32	*/
#define SM_CNT1	8
#define SM_BIT1	5
#define SM_IDX1	(SM_IDX0+SM_CNT0)
#define SM_MAX1	(SM_MAX0 + SM_CNT1*SM_RND1)

#define SM_RND2	(4*ALIGN)	/* round value:  4*ALIGN  ==   64	*/
#define SM_BIT2	6
#define SM_CNT2	8
#define SM_IDX2	(SM_IDX1+SM_CNT1)
#define SM_MAX2	(SM_MAX1 + SM_CNT2*SM_RND2)

#define SM_SLOT	(SM_CNT0 + SM_CNT1 + SM_CNT2) /* number of caches	*/
#define SM_MIN	SM_RND0	/* min size of all cached blocks	*/
#define SM_MAX	SM_MAX2	/* max size of all cached blocks	*/

#define SM_MAKE		4 /* cache preallocation gradually increases up to a limit */
#define SMmake(z)	((z) <= SM_MAX0 ? 4*SM_MAKE : SM_MAKE  )
#define SMMAKE(pk,k,z)	((pk)->small[k].make >= 1024 ? 1024 : ((pk)->small[k].make += SMmake(z)) )

/* round each value z to the next appropriate boundary */
#define SMROUND(z)	((z) <= SM_MAX0 ? ROUND((z), SM_RND0) : \
			 (z) <= SM_MAX1 ? ROUND((z), SM_RND1) : ROUND((z), SM_RND2) )

/* compute the index of a given size in the cache array */
#define SMINDEXZ(z)	((z) <= SM_MAX0 ? ((z)>>SM_BIT0) + ((SM_IDX0-1) - 0) : \
			 (z) <= SM_MAX1 ? ((z)>>SM_BIT1) + ((SM_IDX1-1) - (SM_MAX0>>SM_BIT1)) : \
			 		  ((z)>>SM_BIT2) + ((SM_IDX2-1) - (SM_MAX1>>SM_BIT2)) )

/* rounding for allocation requests via the free tree */
#define LG_RND0		BODYSIZE
#define LGROUND(z)	(ROUND((z), LG_RND0) == 0 ? LG_RND0 : ROUND((z), LG_RND0) )

/* amount to add to each memory extension to reduce future work */
#define EXTRA(pk)	((pk)->extz >= 256*1024 ? 256*1024 : ((pk)->extz *= 4) )

typedef struct _small_s		Small_t;
typedef struct _pack_s		Pack_t;
typedef struct _vmbest_s	Vmbest_t;

struct _small_s /* type to allocate small blocks */
{	Block_t* volatile	free;	/* list of free blocks		*/
	unsigned char 		lock;	/* allocation/free lock		*/
	unsigned short		make;	/* #blocks to make each time	*/
};

struct _pack_s  /* type of a pack */
{	Block_t* volatile	free;	/* recently freed blocks	*/
	unsigned int 		lock;	/* best-fit allocation locking 	*/
	unsigned short		ppos;	/* position in Vmbest_t.pack[] 	*/
	Vmbest_t*		best;	/* region containing this pack	*/
	unsigned short		lpos;	/* position in Vmbest_t.list[]	*/
	size_t			extz;	/* amount to add when extending	*/
	Block_t*		pblk;	/* superblock to allocate from	*/
	Block_t*		endb;	/* sentinel block at end of blk	*/
	Block_t*		root;	/* root of the free tree  	*/
	Block_t*		wild;	/* the wilderness block		*/
	Block_t*		alloc;	/* block being allocated from	*/
	Small_t		small[SM_SLOT]; /* array of small block types	*/
};

struct _vmbest_s /* type of a Vmbest region */
{	Vmdata_t		vmdt;	/* inheritance from Vmdata_t 	*/
	unsigned int		nomem;	/* no raw memory available	*/
	unsigned int		pkcnt;	/* #packs already created	*/
	unsigned int		pkmax;	/* max number of packs allowed	*/
	unsigned int		tid[PK_ARRAY];	/* initializing tid	*/
	Pack_t* volatile	pack[PK_ARRAY];	/* array by thread ID	*/
	Pack_t*			list[PK_ARRAY];	/* list of packs	*/
};

/* Tree rotation functions */
#define RROTATE(x,y)	(LEFT(x) = RGHT(y), RGHT(y) = (x), (x) = (y))
#define LROTATE(x,y)	(RGHT(x) = LEFT(y), LEFT(y) = (x), (x) = (y))
#define RLINK(s,x)	((s) = LEFT(s) = (x))
#define LLINK(s,x)	((s) = RGHT(s) = (x))

/* manage the doubly linked lists of elts of the same size in a free tree */
#define SETLIST(b)	(RGHT(b) =  (b) )
#define TSTLIST(b)	(RGHT(b) == (b) )
#define APPEND(e,b)	(((LINK(b) = LINK(e)) ? LEFT(LINK(b)) = (b) : NIL(Block_t*)), \
			 (LEFT(b) = (e)), LINK(e) = (b), SETLIST(b) )
#define DELETE(b)	((LEFT(b) ? (LINK(LEFT(b)) = LINK(b)) : NIL(Block_t*)), \
			 (LINK(b) ? (LEFT(LINK(b)) = LEFT(b)) : NIL(Block_t*)) )
#define REMOVE(pk,b)	((b) == (pk)->wild ? ((pk)->wild = NIL(Block_t*)) : \
			 TSTLIST(b) ? DELETE((b)) : bestpackextract((pk), BDSZ(b)) )

#define PACKWILD(pk,b)	(NEXT(b) == (pk)->endb ) /* test wilderness */
#define	METHOD(bst)	((bst)->vmdt.mode & VM_METHODS) /* allocation method */

static int chktree(Pack_t* pack, Block_t* node) /* check the shape of a free tree */
{	
	Block_t		*a, *b, **s;

	if(_Vmassert & VM_check_reg)
	{	if(!node) /* the empty tree is always good */
			return 0;
		/**/DEBUG_ASSERT(BDSZ(node) >= BODYSIZE && (BDSZ(node)%ALIGN) == 0 );

		if(SIZE(node) & (BUSY|PFREE)) /* should be BITS-free */
			{ /**/DEBUG_MESSAGE("Free block corrupted"); /**/DEBUG_ASSERT(0); return -1; }

		for(b = node; b; b = LINK(b))
		{	if(PACK(b) != pack ) /* wrong pack! */
				{ /**/DEBUG_MESSAGE("Free block with wrong pack"); /**/DEBUG_ASSERT(0); return -1; }
			if(BDSZ(b) != BDSZ(node) ) /* not equal the head node */
				{ /**/DEBUG_MESSAGE("Free block with wrong size"); /**/DEBUG_ASSERT(0); return -1; }
			s = SELF(b);
			if(*s != b) /* no self-reference pointer! */
				{ /**/DEBUG_MESSAGE("Free block without self-pointer"); /**/DEBUG_ASSERT(0); return -1; }
			a = NEXT(b);
			if((SIZE(a)&(BUSY|PFREE)) != (BUSY|PFREE) )
				{ /**/DEBUG_MESSAGE("Free block with corrupted next block"); /**/DEBUG_ASSERT(0); return -1; }
		}

		if((b = LEFT(node)) )
		{	if(SIZE(b) >= SIZE(node) )
				{ /**/DEBUG_MESSAGE("Free tree out of shape (left)"); /**/DEBUG_ASSERT(0); return -1; }
			else	return chktree(pack, b);
		}
		if((b = RGHT(node)) )
		{	if(SIZE(b) <= SIZE(node) )
				{ /**/DEBUG_MESSAGE("Free tree out of shape (right)"); /**/DEBUG_ASSERT(0); return -1; }
			else	return chktree(pack, b);
		}
	}

	return 0;
}

static int chkregion(Vmbest_t* best, int local) /* check region integrity */
{
	ssize_t		k;
	Pack_t		*pack;
	int		rv = 0;

	if(!local && (_Vmassert & VM_check_reg) )
	{	for(k = 0; k < best->pkcnt; ++k) /* check integrity of packs */
		{	if(!(pack = best->list[k]) )
				continue;
			if(asocasint(&pack->lock, 0, KEY_BEST) == 0 )
			{	if(pack->root)
				{	if(PACK(pack->root) != pack)
						{ /**/DEBUG_MESSAGE("Free tree with bad root"); rv = -1; /**/DEBUG_ASSERT(0); }
					if(chktree(pack, pack->root) < 0 )
						{ rv = -1; /**/DEBUG_ASSERT(0); }
				}
				asocasint(&pack->lock, KEY_BEST, 0);
			}
		}
	}

	return rv;
}

/* free an allocated block */
static int bestfree(Vmalloc_t* vm, Void_t* data, int local )
{
	size_t			sz;
	Block_t			*blk, *head;
	Block_t* volatile	*listp;
	Pack_t			*pack;
	asospindecl();

#if _BLD_DEBUG /* special use for debugging */
	if(data == (Void_t*)(1) || data == (Void_t*)(-1))
	{	int save = _Vmassert; _Vmassert |= VM_check_reg;
		chkregion((Vmbest_t*)vm->data, 0);
		_Vmassert = save;
		return 0;
	}
#endif
	/**/DEBUG_COUNT(N_free);
	/**/DEBUG_ASSERT(chkregion((Vmbest_t*)vm->data, local) >= 0);

	if(!data) /* ANSI-ism */
		return 0;

	/* quick sanity check */
	if((Vmuchar_t*)data < vm->data->segmin || (Vmuchar_t*)data >= vm->data->segmax)
		return -1;

	blk = BLOCK(data); /**/DEBUG_ASSERT((SIZE(blk)&BUSY) && (BDSZ(blk)%ALIGN) == 0 );
	pack = PACK(blk); /**/DEBUG_ASSERT(pack->best == (Vmbest_t*)vm->data);
	if((sz = SIZE(blk))&SMALL )
		listp = &pack->small[SMDECODE(sz)].free;
	else	listp = &pack->free;

	for(asospininit();; asospinnext() ) /* lock-free insertion */
	{	LINK(blk) = head = *listp;
		if(asocasptr(listp, head, blk) == head )
			break;
	}

	if(!local && _Vmtrace )
		(*_Vmtrace)(vm, (Vmuchar_t*)data, NIL(Vmuchar_t*), TRUESIZE(sz) , 0);

	return 0;
}

/* Make a new pack with initial memory large enough per first request */
static Pack_t* bestpackget(Vmalloc_t* vm, unsigned int ppos, unsigned int tid)
{
	unsigned int	lpos;
	Block_t		*blk, *pb, *nb;
	Pack_t		*pack;
	Vmbest_t	*best = (Vmbest_t*)vm->data;

	if((pack = asocasptr(&best->pack[ppos], NIL(Pack_t*), PK_INITIALIZING)) != NIL(Pack_t*) )
	{	asospindecl(); /* pack is initializing -- wait until its done */
		for(asospininit();; asospinnext() )
		{	/* if we are inititializing then a signal hit -- skip it or deadlock */
			if(best->tid[ppos] == tid)
				return NIL(Pack_t*);
			if((pack = best->pack[ppos]) != PK_INITIALIZING)
				return pack;
		}
	} /**/DEBUG_ASSERT(pack == NIL(Pack_t*) && best->pack[ppos] == PK_INITIALIZING);
	/* set tid for the deadlock check above (which must happen inside the spin loop!) */
	best->tid[ppos] = tid;

#define EXTZ	(8*1024) /* extra on each memory extension */
	if(!(blk = (*_Vmsegalloc)(vm, NIL(Block_t*), sizeof(Pack_t)+EXTZ, VM_SEGEXTEND)) )
	{	best->nomem = 1; /**/DEBUG_ASSERT(best->list[0]);
		asocasptr(&best->pack[ppos], PK_INITIALIZING, NIL(Pack_t*));
		return best->list[0];
	}

	pb = (Block_t*)DATA(blk); /* block holding pack structure */
	pack = (Pack_t*)DATA(pb);

	PACK(pb) = pack;
	SIZE(pb) = ROUND(sizeof(Pack_t),BODYSIZE)|BUSY;

	nb = NEXT(pb); /* usable memory (minus sentinel block) */
	PACK(nb) = pack;
	SIZE(nb) = BDSZ(blk) - (BDSZ(pb) + sizeof(Head_t)) - 2*sizeof(Head_t);

	pb = NEXT(nb); /* sentinel block to signal end of memory */
	PACK(pb) = pack;
	SIZE(pb) = BUSY|PFREE; 

	/* set initial values of fields in Pack_t */
	memset(pack, 0, sizeof(Pack_t));
	pack->best = best;
	pack->extz = EXTZ; /* extra for physical extension */
	pack->pblk = blk; /* first allocatable free memory */
	pack->endb = pb; /**/DEBUG_ASSERT(pack->endb == ENDB(pack->pblk));
	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) WILD(%p) => WILD(%p)=%zd\n", __FILE__, __LINE__, pack, pack->wild, nb, BDSZ(nb));
	pack->wild = nb; /**/DEBUG_ASSERT(PACKWILD(pack, pack->wild));

	lpos = asoaddint(&best->pkcnt,1); /* get position in best->list[] to insert */
	pack->lpos = lpos; /**/DEBUG_ASSERT(lpos < best->pkmax && best->list[lpos] == NIL(Pack_t*));
	asocasptr(&best->list[lpos], NIL(Pack_t*), pack);

	pack->ppos = ppos; /**/DEBUG_ASSERT(best->pack[ppos] == PK_INITIALIZING);
	asocasptr(&best->pack[ppos], PK_INITIALIZING, pack);

	return pack;
}

/* extend pack->pblk to accomodate "size". "wild", if not NULL, is the bottom piece */
static Block_t* bestpackextend(Vmalloc_t* vm, Pack_t* pack, Block_t* wild, ssize_t size, int segtype)
{
	size_t		blkz;
	Block_t		*pblk, *endb;

	/**/DEBUG_ASSERT(!wild || (PACK(wild) == pack && BDSZ(wild) < size && PACKWILD(pack,wild)) );
	blkz = BDSZ(pack->pblk); /**/DEBUG_ASSERT(blkz >= _Vmpagesize);
	size += blkz - (wild ? BDSZ(wild) : 0) + EXTRA(pack); /**/DEBUG_ASSERT(size%ALIGN == 0);
	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) WILD(%p)=%zd BDSZ(%p)=%zd blkz=%zd size=%zu\n", __FILE__, __LINE__, pack, wild, wild ? BDSZ(wild) : 0, pack->pblk, BDSZ(pack->pblk), blkz, size);
	if(!(pblk = (*_Vmsegalloc)(vm, pack->pblk, size, segtype)) )
		pblk = pack->pblk;
	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) WILD(%p)=%zd BDSZ(%p)=%zd blkz=%zd size=%zu\n", __FILE__, __LINE__, pack, wild, wild ? BDSZ(wild) : 0, pblk, BDSZ(pblk), blkz, size);

	if(BDSZ(pblk) > blkz ) /* superblock increased in size */
	{	if(!wild) /* new wild starts at the previous end block */
		{	wild = pack->endb;
			if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) WILD(%p)=%zd\n", __FILE__, __LINE__, pack, wild, BDSZ(wild));
		}
		if((_Vmassert & VM_debug) && (Vmuchar_t*)wild > ((Vmuchar_t*)ENDB(pblk) - sizeof(Head_t))) /* heisus in _Vmsegalloc */
		{	wild = (Block_t*)((char*)DATA(pblk) + blkz);
			if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) WILD(%p)=%zd\n", __FILE__, __LINE__, pack, wild, BDSZ(wild));
		}

		/* new end block */
		pack->endb = endb = ENDB(pblk);
		PACK(endb) = pack;
		SIZE(endb) = BUSY;

		/* new size for wild */
		size = ((Vmuchar_t*)endb - (Vmuchar_t*)wild) - sizeof(Head_t);
		if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) pblk=%p endb=%p wild=%p size=%zu\n", __FILE__, __LINE__, pack, pblk, endb, wild, size);
		PACK(wild) = pack;
		SIZE(wild) = size | (SIZE(wild)&BITS) | BUSY; /**/DEBUG_ASSERT(PACKWILD(pack,wild));
		if(_Vmassert & 1)
		{	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) WILD(%p) => WILD(%p)=%zd\n", __FILE__, __LINE__, pack, pack->wild, wild, BDSZ(wild));
			pack->wild = wild;
		}
	}

	return wild;
}

/* Find and extract a suitable element in a free tree. */
static Block_t* bestpackextract(Pack_t* pack, size_t size )
{
	size_t	sz;
	Block_t	*t, *root, *l, *r, link;
	/**/DEBUG_ASSERT(chktree(pack, pack->root) == 0);

	l = r = &link;
	if((root = pack->root) ) do /* top-down splay tree search */
	{	/**/DEBUG_ASSERT((size%ALIGN) == 0 && !(SIZE(root)&(BUSY|PFREE)) );
		if(size == (sz = BDSZ(root)) )
			break;
		if(size < sz)
		{	if((t = LEFT(root)) )
			{	if(size <= (sz = BDSZ(t)) )
				{	RROTATE(root,t);
					if(size == sz)
						break;
					t = LEFT(root);
				}
				else
				{	LLINK(l,t);
					t = RGHT(t);
				}
			}
			RLINK(r,root);
		}
		else
		{	if((t = RGHT(root)) )
			{	if(size >= (sz = BDSZ(t)) )
				{	LROTATE(root,t);
					if(size == sz)
						break;
					t = RGHT(root);
				}
				else
				{	RLINK(r,t);
					t = LEFT(t);
				}
			}
			LLINK(l,root);
		}
		/**/DEBUG_ASSERT(root != t);
	} while((root = t) );

	if(root) /* found it, now isolate it */
	{	RGHT(l) = LEFT(root);
		LEFT(r) = RGHT(root);
	}
	else /* nothing exactly fit */
	{	LEFT(r) = NIL(Block_t*);
		RGHT(l) = NIL(Block_t*);

		if((root = LEFT(&link)) ) /* grab the least one > size */
		{	while((t = LEFT(root)) )
				RROTATE(root,t);
			LEFT(&link) = RGHT(root);
		}
	}

	if(root && (r = LINK(root)) ) /* head of a link list, use next one for root */
	{	LEFT(r) = RGHT(&link);
		RGHT(r) = LEFT(&link);
	}
	else if(!(r = LEFT(&link)) )
		r = RGHT(&link);
	else /* graft left tree to right tree */
	{	while((t = LEFT(r)) )
			RROTATE(r,t);
		LEFT(r) = RGHT(&link);
	}
	pack->root = r; /**/DEBUG_ASSERT(!r || !(SIZE(r)&(BUSY|PFREE)) );

	/**/DEBUG_ASSERT(chktree(pack, pack->root) == 0);
	return root;
}

/* Reclaim all delayed free blocks of a pack into its free tree */
static int bestlistreclaim(Vmalloc_t* vm, Pack_t* pack, Block_t* volatile *listp)
{
	ssize_t		size, sz;
	Block_t		*fp, *np, *t, *list, *adj, **s;
	asospindecl();
	/**/DEBUG_ASSERT(chktree(pack, pack->root) == 0);

	for(asospininit();; asospinnext()) /* lock-free grab of the free list */
	{	if(!(list = *listp) )
			return 0;
		if(asocasptr(listp, list, NIL(Block_t*)) == list)
			break;
	}

	/* merge all adjacent free blocks */
	for(fp = list; fp; fp = LINK(fp))
		SIZE(fp) |= MARK;
	for(adj = NIL(Block_t*), fp = list;; )
	{	/**/DEBUG_ASSERT(!(SIZE(fp)&SMALL) );
		if(SIZE(fp)&PFREE) /* merge with a preceding free block */
		{	np = PREV(fp); /**/DEBUG_ASSERT(!(SIZE(np)&SMALL) && NEXT(np) == fp && PACK(np) == pack);
			REMOVE(pack, np); /**/DEBUG_ASSERT((SIZE(np)&BITS) == 0);
			SIZE(np) += (BDSZ(fp) + sizeof(Head_t))|BUSY|MARK;

			LINK(np) = LINK(fp); fp = np; /* replace fp by np in the list */
			if(adj)
				LINK(adj) = fp;
			else	list = fp;
		}
		else if(SIZE(fp) != 0)
		{	np = NEXT(fp); sz = SIZE(np); /* try merging next block */
			if(sz&MARK)
			{	SIZE(fp) += (sz&~BITS) + sizeof(Head_t);
				SIZE(np) = 0;
			}
			else if(!(sz&BUSY) )
			{	SIZE(fp) += (sz&~BITS) + sizeof(Head_t);
				REMOVE(pack, np);
				t = NEXT(np);
				SIZE(t) &= ~PFREE;
			}
			else	goto n_ext;
		}
		else
		{ n_ext: adj = fp; /* move forward */
			if(!(fp = LINK(fp)) )
				break;
		}
	}

	for(fp = list; fp != NIL(Block_t*); fp = adj)
	{	adj = LINK(fp);
		if(SIZE(fp) == 0)
			continue;

		/**/DEBUG_ASSERT((SIZE(fp)&(BUSY|MARK)) == (BUSY|MARK) );
		/**/DEBUG_ASSERT(BDSZ(fp) >= sizeof(Body_t) && BDSZ(fp)%ALIGN == 0);
		SIZE(fp) &= ~BITS; 
		t = NEXT(fp);
		SIZE(t) |= PFREE; /**/DEBUG_ASSERT(SIZE(NEXT(fp))&BUSY);
		s = SELF(fp);
		*s = fp; /* self-pointer for a free block */

		if(PACKWILD(pack, fp) ) /* wilderness preservation */
		{	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) WILD(%p) => WILD(%p)=%zd\n", __FILE__, __LINE__, pack, pack->wild, fp, BDSZ(fp));
			pack->wild = fp;
		}
		else /* insert into free tree */
		{	LEFT(fp) = RGHT(fp) = LINK(fp) = NIL(Block_t*);
			if(!(np = pack->root) )	/* empty tree	*/
				pack->root = fp;
			else for(size = SIZE(fp);; ) /* leaf insertion */
			{	/**/DEBUG_ASSERT(np != fp);
				if((sz = SIZE(np)) > size)
				{	if((t = LEFT(np)) )
					{	/**/DEBUG_ASSERT(t != np && t != fp);
						np = t;
					}
					else /* left leaf insertion */
					{	LEFT(np) = fp;
						break;
					}
				}
				else if(sz < size)
				{	if((t = RGHT(np)) )
					{	/**/DEBUG_ASSERT(t != np && t != fp);
						np = t;
					}
					else /* right leaf insertion */
					{	RGHT(np) = fp;
						break;
					}
				}
				else /* sz == size, list insertion */
				{	APPEND(np, fp);
					break;
				}
			}
		}
	}

	/**/DEBUG_ASSERT(chktree(pack, pack->root) == 0);
	return 0;
}

/* allocating a normal block from the given pack */
static Block_t* bestpackalloc(Vmalloc_t* vm, Pack_t* pack, size_t size, size_t minz )
{
	ssize_t		sz;
	Block_t		*tp, *np, *pblk;
	Vmbest_t	*best = (Vmbest_t*)vm->data;
	/**/DEBUG_ASSERT(size >= sizeof(Body_t) && size%ALIGN == 0);

	if((tp = pack->alloc) ) /* fast allocation from recent memory */
	{	pack->alloc = NIL(Block_t*);
		if(SIZE(tp) >= size )
		{	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) SIZE(%p)=%zd size=%zu minz=%zu\n", __FILE__, __LINE__, pack, tp, SIZE(tp), size, minz);
			goto a_lloc;
		}
		KPVFREE(vm, DATA(tp), bestfree);
	}

	if(!(tp = bestpackextract(pack, size)) )
	{	bestlistreclaim(vm, pack, &pack->free);
		if(!(tp = bestpackextract(pack, size)) && minz < size )
			tp = bestpackextract(pack, minz);
	}
	if(tp) /* got one from the free tree */
	{	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) SIZE(%p)=%zd size=%zu minz=%zu\n", __FILE__, __LINE__, pack, tp, SIZE(tp), size, minz);
		SIZE(tp) |= BUSY; /**/DEBUG_ASSERT(BDSZ(tp) >= minz);
		np = NEXT(tp);
		SIZE(np) &= ~PFREE;
		goto a_lloc;
	}

	if(vm->data->lock == 0) /* could be asothreadid() */
	{	if(!(tp = pack->wild) || SIZE(tp) < size ) /* check wilderness */
		{	if(tp)
			{	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) SIZE(%p)=%zd size=%zu minz=%zu\n", __FILE__, __LINE__, pack, tp, SIZE(tp), size, minz);
				if(!PACKWILD(pack,tp))
				{	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) WILD(%p)=%zd orphaned wild\n", __FILE__, __LINE__, pack, tp, SIZE(tp));
					pack->wild = tp = NIL(Block_t*);
				}
			}
			tp = bestpackextend(vm, pack, tp, size, VM_SEGEXTEND);
			if(tp)
			{	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) SIZE(%p)=%zd size=%zu minz=%zu\n", __FILE__, __LINE__, pack, tp, SIZE(tp), size, minz);
				/**/DEBUG_ASSERT(PACKWILD(pack,tp));
			}
		}
		if(tp)
		{	if(pack->wild == tp) /* got a non-trivial wild block */
			{	pack->wild = NIL(Block_t*);
				if(!PACKWILD(pack,tp))
				{	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) WILD(%p)=%zd orphaned wild\n", __FILE__, __LINE__, pack, tp, SIZE(tp));
					tp = 0;
				}
			}
			if(tp)
			{	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) SIZE(%p)=%zd size=%zu minz=%zu\n", __FILE__, __LINE__, pack, tp, SIZE(tp), size, minz);
				np = NEXT(tp);
				SIZE(np) &= ~PFREE;
				SIZE(tp) |= BUSY;
				if(SIZE(tp) >= minz ) /* small but good enough */
					goto a_lloc;
				KPVFREE(vm, DATA(tp), bestfree);
			}
		}
	}

	if(!(pblk = (*_Vmsegalloc)(vm, NIL(Block_t*), size+EXTRA(pack), VM_SEGEXTEND)) )
	{	best->nomem = 1;
		return NIL(Block_t*);
	}
	else /* got a new superblock */
	{	tp = (Block_t*)DATA(pblk);
		PACK(tp) = pack;
		SIZE(tp) = (BDSZ(pblk) - 2*sizeof(Head_t))|BUSY;
		if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) SIZE(%p)=%zd size=%zu minz=%zu\n", __FILE__, __LINE__, pack, tp, SIZE(tp), size, minz);

		pack->endb = np = NEXT(tp);
		PACK(np) = pack;
		SIZE(np) = BUSY;
		if((_Vmassert & 2) && pack->wild)
		{	if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) WILD(%p) => WILD(%p)\n", __FILE__, __LINE__, pack, pack->wild, tp);
			pack->wild = tp; /**/DEBUG_ASSERT(PACKWILD(pack, pack->wild));
		}

		pack->pblk = pblk; /**/DEBUG_ASSERT(BDSZ(pblk) > size);
	}

a_lloc: /**/DEBUG_ASSERT(tp && BDSZ(tp) >= minz && PACK(tp) == pack);
	if((sz = (ssize_t)BDSZ(tp) - (ssize_t)size) >= (ssize_t)sizeof(Block_t) )
	{	SIZE(tp) = size | (SIZE(tp)&(BUSY|PFREE));
		if(_Vmassert & VM_debug) debug_printf(2, "%s:%d: PACK(%p) SIZE(%p)=%zd size=%zu minz=%zu\n", __FILE__, __LINE__, pack, tp, SIZE(tp), size, minz);

		np = NEXT(tp);  /**/DEBUG_ASSERT(!pack->alloc);
		PACK(np) = pack;
		SIZE(np) = (sz - sizeof(Head_t)) | BUSY;
		pack->alloc = np;
	}

	return tp;
}

/* allocate a small block from a specific pack */
static Block_t* bestsmallalloc(Vmalloc_t* vm, Pack_t* pack, ssize_t size)
{
	int		n;
	Block_t		*blk, *list, *last, *head;
	size_t		memz, blkz, smiz;
	Small_t		*small;
	asospindecl();
	/**/DEBUG_ASSERT(size >= SM_MIN && size <= SM_MAX);

	small = &pack->small[smiz = SMINDEXZ(size)]; /* cache */
	if(asocaschar(&small->lock, 0, 1) != 0 ) /* lock to prevent concurrent allocs */
		return NIL(Block_t*); /* list is busy; return to explore a different pack */

	for(asospininit();; asospinnext()) /* loop is needed because bestfree() is lock-free */
		if(!(blk = small->free) || asocasptr(&small->free, blk, LINK(blk)) == blk )
			break;

	if(!blk && asocasint(&pack->lock, 0, KEY_BEST) == 0 )
	{	blkz = sizeof(Head_t) + size; /* block size + header */
		memz = SMMAKE(pack, smiz, size)*blkz;
		if((blk = bestpackalloc(vm, pack, memz, SM_MAKE*blkz)) ) 
		{	memz = SIZE(blk) + sizeof(Head_t); /**/DEBUG_ASSERT(memz > SM_MAKE*blkz);
			memz = memz - (n = memz/blkz-1)*blkz - sizeof(Head_t); /* size of end block */

			smiz = SMENCODE(smiz) | size | (SMALL|BUSY);
			for(list = blk; n > 0; --n) /* make list of blocks */
			{	last = blk;
				PACK(blk) = pack;
				SIZE(blk) = smiz; /* Note: small blocks code index and size */
				LINK(last) = blk = (Block_t*)((Vmuchar_t*)blk + blkz);
			} /**/DEBUG_ASSERT(last > list && last < blk);
			SIZE(list) |= memz&PFREE; memz &= ~BITS;

			for(asospininit();; asospinnext() ) /* prepend list to cache */
			{	LINK(last) = head = small->free;
				if(asocasptr(&small->free, head, list) == head)
					break;
			}

			/* the last block may or may not be cached */
			PACK(blk) = pack; /**/DEBUG_ASSERT(memz >= size );
			SIZE(blk) = memz == size ? smiz : (memz|BUSY);
		}
		asocasint(&pack->lock, KEY_BEST, 0);
	}

	asocaschar(&small->lock, 1, 0);
	return blk;
}

/* allocate a block big enough to cover a requested size */
static Void_t* bestalloc(Vmalloc_t* vm, size_t size, int local)
{
	unsigned int	pkid, ppos, lpos, begp, tid;
	ssize_t		smsz, pksz;
	Block_t		*blk;
	Pack_t		*pk;
	Vmbest_t	*best = (Vmbest_t*)vm->data;
	asospindecl();

	/**/DEBUG_COUNT(N_alloc);
	/**/DEBUG_ASSERT((ALIGN%(BITS+1)) == 0 );
	/**/DEBUG_ASSERT((sizeof(Head_t)%ALIGN) == 0 );
	/**/DEBUG_ASSERT((sizeof(Body_t)%ALIGN) == 0 );
	/**/DEBUG_ASSERT(sizeof(Block_t) == (sizeof(Body_t)+sizeof(Head_t)) );
	/**/DEBUG_ASSERT(chkregion((Vmbest_t*)vm->data, local) >= 0);

	/* start with thread-specific pack but will cycle through all */
	tid = asothreadid();
	ppos = pkid = tid%best->pkmax; /* hash by thread id */
	if(!(pk = best->pack[ppos]) || pk == PK_INITIALIZING)
		pk = bestpackget(vm, ppos, tid); /**/DEBUG_ASSERT(pk != PK_INITIALIZING);
	if(pk)
		lpos = begp = pk->lpos;
	else	begp = (lpos = 0) + 1;

	smsz = (local || size > SM_MAX) ? 0 : SMROUND(size);
	for(asospininit();; )
	{	if(pk)
		{
			if(smsz > 0 && (blk = bestsmallalloc(vm, pk, smsz)) )
				break;
			if(asocasint(&pk->lock, 0, KEY_BEST) == 0 )
			{	pksz = LGROUND(size); /* general purpose allocation */
				blk = bestpackalloc(vm, pk, pksz, pksz);
				asocasint(&pk->lock, KEY_BEST, 0);
				if(blk)
					break;
			}
		}

		do /* get next pack to allocate from */
		{	if(!best->pkcnt || (lpos = (lpos+1)%best->pkcnt) == begp) /* cycling over list[] */
			{	if(best->nomem) /* nothing more possible to do */
					return NIL(Void_t*);
	
				do
				{	/* conflict at ppos so move it forward to reduce contention */
					if((ppos = (ppos+1)%best->pkmax) == pkid) /* cycling over pack[] */
						asospinnext(); /* yield a bit before getting going again */

				} while (!(pk = bestpackget(vm, ppos, tid)));
				lpos = begp = pk->lpos;
			}
		} while(!(pk = best->list[lpos]) );
	} 

	if(_Vmtrace && !local )
		(*_Vmtrace)(vm,NIL(Vmuchar_t*),(Vmuchar_t*)DATA(blk),size,0);
	return DATA(blk);
}

/* resize a block to a new size */
static Void_t* bestresize(Vmalloc_t* vm, Void_t* data, size_t size, int type, int local)
{
	Block_t		*rp, *np;
	ssize_t		sz, oldz, newz, incz;
	Pack_t		*pack;
	Void_t		*rsdt = data;
	/**/DEBUG_DECLARE(Vmbest_t, *best = (Vmbest_t*)vm->data)

	/**/DEBUG_COUNT(N_resize);
	/**/DEBUG_ASSERT(chkregion(best, local) >= 0);

	if(!rsdt) /* resizing a NULL block is the same as allocating */
	{	if((rsdt = bestalloc(vm, size, local)) && (type&VM_RSZERO) )
		{	rp = BLOCK(rsdt);
			memset((Void_t*)rsdt, 0, TRUEBDSZ(rp));
		}
		return rsdt;
	}
	if(size == 0) /* resizing to zero size is the same as freeing */
	{	(void)bestfree(vm, rsdt, local);
		return NIL(Void_t*);
	}

	/* quick sanity check */
	if((Vmuchar_t*)rsdt < vm->data->segmin || (Vmuchar_t*)rsdt >= vm->data->segmax)
		return NIL(Void_t*);

	rp = BLOCK(rsdt); /**/DEBUG_ASSERT(SIZE(rp)&BUSY );
	pack = PACK(rp); /**/DEBUG_ASSERT(pack->best == best);
	oldz = TRUEBDSZ(rp);
	newz = LGROUND(size);
	if(oldz == newz )
		goto done;

	if(SIZE(rp)&SMALL) /* small blocks cannot be extended */
	{	if(oldz >= newz)
			goto done; /* small stuff, just keep it */
		else	goto a_lloc; /* move to a new area */
	}

	/* resized blocks may get extra space to reduce future work */
	incz = sizeof(Head_t); /* add tail space for block info */
	np = (Block_t*)((Vmuchar_t*)NEXT(rp) - sizeof(Head_t));
	if((Block_t*)PACK(np) == rp && (SIZE(np)&MARK) )
	{	if(newz <= oldz && newz >= BDSZ(np) ) /* increasing within bound */
		{	SIZE(np) = newz|MARK; /* easy work, just update current size */
			goto done;
		}
		if((sz = newz-oldz) > 0 && 8*sz < oldz ) /* add to reduce future work */
		{	sz = oldz < 4024 ? 2*sz : oldz/(oldz < 16*1024 ? 4 : 2);
			incz += ROUND(sz, BODYSIZE);
		}
	}
	newz += incz; /* add space for tail info and perhaps to reduce future work */

	if(asocasint(&pack->lock, 0, KEY_BEST) == 0 /* pack is open */ )
	{	if(oldz < newz )
		{	np = NEXT(rp); /* try forward merging if possible */
			if(np == pack->alloc) /* np is being allocated from */
			{	pack->alloc = NIL(Block_t*);
				SIZE(rp) += BDSZ(np) + sizeof(Head_t);
			}
			else if(!(SIZE(np)&BUSY) ) /* np is free */
			{	/**/DEBUG_ASSERT(!(SIZE(np)&PFREE) && PACK(np) == pack );
				REMOVE(pack, np); 
				SIZE(rp) += BDSZ(np) + sizeof(Head_t);
				np = NEXT(rp);
				SIZE(np) &= ~PFREE; /**/DEBUG_ASSERT(SIZE(np)&BUSY );
			}

			if(SIZE(rp) < newz && PACKWILD(pack, rp) ) /* extend arena */
			{	/**/DEBUG_ASSERT((SIZE(rp)&BUSY) && SIZE(NEXT(rp)) == BUSY );
				np = bestpackextend(vm, pack, rp, newz, VM_SEGEXTEND);
				/**/DEBUG_ASSERT(np == rp);
			}
		}

		/* release left-over if large enough */
		if((sz = (ssize_t)BDSZ(rp) - newz) >= (ssize_t)sizeof(Block_t) )
		{	SIZE(rp) = newz | (SIZE(rp)&BITS);
			np = NEXT(rp);
			PACK(np) = pack;
			SIZE(np) = (sz - sizeof(Head_t)) | BUSY;
			KPVFREE(vm, DATA(np), bestfree);
		}
		asocasint(&pack->lock, KEY_BEST, 0);
	}

	if((sz = BDSZ(rp)) < newz)
	{ a_lloc: /* see if we can move to a new area */
		if(!(type&(VM_RSMOVE|VM_RSCOPY)) || !(rsdt = KPVALLOC(vm, newz, bestalloc)) )
			return NIL(Void_t*);

		if(type&VM_RSCOPY) /* save old data then free old memory */
			memcpy(rsdt, data, oldz);
		KPVFREE(vm, data, bestfree);

		rp = BLOCK(rsdt);
		sz = BDSZ(rp);
	}

	/* if get here, resizing was successful */
	if((type&VM_RSZERO) && sz > oldz ) /* zero out new mememory */
		memset((Void_t*)((Vmuchar_t*)rsdt + oldz), 0, sz-oldz);

	if(incz > 0) /* add information to tell that we added more space */
	{	np = (Block_t*)((Vmuchar_t*)NEXT(rp) - sizeof(Head_t));
		PACK(np) = (Pack_t*)rp;
		SIZE(np) = (newz-incz)|MARK; /* !!!actual requested size */
	}

done:	if(!local && _Vmtrace )
		(*_Vmtrace)(vm, (Vmuchar_t*)data, (Vmuchar_t*)rsdt, size, 0);
	return rsdt;
}

/* allocate a block with a specific alignment requirement */
static Void_t* bestalign(Vmalloc_t* vm, size_t size, size_t align, int local)
{
	Vmuchar_t	*data;
	Block_t		*tp, *np;
	ssize_t		sz, remz, algz, algn, extra;
	Pack_t		*pack;
	Vmbest_t	*best = (Vmbest_t*)vm->data;
	asospindecl();
	/**/DEBUG_ASSERT(chkregion(best, local) >= 0);

	if(size <= 0 || align <= 0)
		return NIL(Void_t*);

	algz = LGROUND(size);
	algn = (*_Vmlcm)(align,ALIGN);

	/* non-Vmbest methods may require extra header space */
	if(METHOD(best) != VM_MTBEST && vm->meth.eventf)
	{	extra = (size_t)(*vm->meth.eventf)(vm, VM_BLOCKHEAD, 0);
		while(algn < extra || (algn - extra) < sizeof(Block_t))
			algn *= 2;
	}
	else	extra = 0;

	sz = algz + 2*(algn+sizeof(Head_t)+extra); 
	if(!(data = (Vmuchar_t*)KPVALLOC(vm, sz, bestalloc)) )
		return NIL(Void_t*);

	tp = BLOCK(data); pack = PACK(tp);
	for(asospininit();; asospinnext()) /* acquire lock to alter arena */
		if(asocasint(&pack->lock, 0, KEY_BEST) == 0 )
			break;

	if((remz = (size_t)((VMLONG(data)+extra)%algn)) != 0) /* shift to alignment */
	{	data += algn - remz; /**/DEBUG_ASSERT(((VMLONG(data)+extra)%algn) == 0);
		np = BLOCK(data);
		if(((Vmuchar_t*)np - (Vmuchar_t*)tp) < (ssize_t)(sizeof(Block_t)+extra) )
		{	data += algn;
			np = BLOCK(data);
		} /**/DEBUG_ASSERT(((VMLONG(data)+extra)%algn) == 0);

		sz = (Vmuchar_t*)np - (Vmuchar_t*)tp; /**/DEBUG_ASSERT(sz >= sizeof(Block_t));
		PACK(np) = pack;
		SIZE(np) = (BDSZ(tp) - sz)|BUSY;

		SIZE(tp) = (sz - sizeof(Head_t)) | (SIZE(tp)&BITS) | BUSY;
		KPVFREE(vm, DATA(tp), bestfree);
	}

	tp = BLOCK(data); pack = PACK(tp);
	if((sz = SIZE(tp) - algz) >= sizeof(Block_t)) /* free right side */
	{	SIZE(tp) = algz | (sz&BITS);
		np = NEXT(tp);
		PACK(np) = pack;
		SIZE(np) = ((sz & ~BITS) - sizeof(Head_t)) | BUSY;
		KPVFREE(vm, DATA(np), bestfree);
	}

	asocasint(&pack->lock, KEY_BEST, 0);

	if(!local && _Vmtrace )
		(*_Vmtrace)(vm,NIL(Vmuchar_t*), data, size, align);
	return (Void_t*)data;
}

static int beststat(Vmalloc_t* vm, Vmstat_t* st, int local)
{
	ssize_t		k, ty, sz;
	Block_t		*sgb, *bp, *endbp;
	Pack_t		*pack;
	Seg_t		*seg;
	Vmbest_t	*best = (Vmbest_t*)vm->data;

	if(!st) 
		return 0;

	st->n_pack = best->pkcnt; 
	for(seg = best->vmdt.seg; seg; seg = seg->next )
	for(sgb = (Block_t*)SEGDATA(seg); sgb < seg->endb; sgb = NEXT(sgb) )
	{	/**/DEBUG_ASSERT(BDSZ(sgb) < seg->size);
		if(!(SIZE(sgb)&BUSY) )
		{	st->n_free += 1;
			st->s_free += BDSZ(sgb);
			continue;
		}

		/* make sure that block is well-defined */
		bp = (Block_t*)DATA(sgb);
		pack = PACK(bp);
		for(k = 0; k < best->pkcnt; ++k)
			if(pack == best->list[k])
				break;
		if(k >= best->pkcnt || pack->lock && !local)
			return -1;

		for(endbp = ENDB(sgb); bp < endbp; bp = TRUENEXT(bp) )
		{	pack = PACK(bp);
			if(DATA(bp) == (Void_t*)pack) /* administrative data for the pack */
				continue;

			sz = TRUEBDSZ(bp); /**/DEBUG_ASSERT(sz < BDSZ(sgb));	
			if(SIZE(bp)&BUSY )
			{	st->n_busy += 1;
				st->s_busy += sz;
			}
			else /* a free block */
			{	st->n_free += 1;
				st->s_free += sz;
			}
		}
	}

	/* adjust statistics for delayed free blocks and cached small blocks */
	for(k = 0; k < best->pkcnt; ++k)
	{	if(!(pack = best->list[k]) || pack->lock && !local)
			return -1;

		if((bp = pack->alloc) )
		{	sz = BDSZ(bp);
			st->n_free += 1; st->s_free += sz;
			st->n_busy -= 1; st->s_busy -= sz;
		}

		for(bp = pack->free; bp; bp = LINK(bp) )
		{	sz = BDSZ(bp);
			st->n_free += 1; st->s_free += sz;
			st->n_busy -= 1; st->s_busy -= sz;
		}

		for(ty = 0; ty < SM_SLOT; ++ty)
		{	if(pack->small[ty].lock )
				return -1;
			for(bp = pack->small[ty].free; bp; bp = LINK(bp) )
			{	sz = SMBDSZ(bp);
				st->n_free += 1; st->s_free += sz;
				st->n_busy -= 1; st->s_busy -= sz;
				st->n_cache += 1; st->s_cache += sz;
			}
		}
	}

	return 0;
}

static int bestevent(Vmalloc_t* vm, int event, Void_t* arg)
{
	Vmbest_t	*best = (Vmbest_t*)vm->data;

	if(event == VM_OPEN ) /* return the size of Vmbest_t */
	{	if(!arg) /* bad call */
			return -1;
		*((ssize_t*)arg) = sizeof(Vmbest_t);
	}
	else if(event == VM_ENDOPEN)
	{	if((best->pkmax = asoactivecpu()) < 2)
			best->pkmax = 2;
		if((best->pkmax *= 2) > PK_ARRAY )
			best->pkmax = PK_ARRAY;
		if(best->pkmax < PK_ALLOW)
			best->pkmax = PK_ALLOW;
	}
	else if(event == VM_CHECKARENA) /* check integrity of the arena */
	{	if(chkregion((Vmbest_t*)vm->data, 0) < 0)
			return -1;
	}

	return 0;
}

void _vmchkaddress(Vmalloc_t* vm, Vmuchar_t* addr, int tag)
{
	if(vm && addr && (addr < vm->data->segmin || addr >= vm->data->segmax) )
		_vmmessage("Address not belonging to a region", 0, tag ? "Tag" : NIL(const char*), tag);
}

void _vmchkall(int tag) /* check to see if some region may have a bad free list or cache */
{
	Vmhold_t	*vh;
	Vmbest_t	*best;
	Pack_t		*pack;
	int		p, k;

	for(vh = _Vmhold; vh; vh = vh->next)
	{	if(!vh->vm || !(vh->vm->meth.meth&VM_MTBEST) )
			continue;

		best = (Vmbest_t*)vh->vm->data;
		for(p = 0; p < best->pkcnt; ++p)
		{	if(!(pack = best->list[p]) )
				continue;

			_vmchkaddress(vh->vm, (Vmuchar_t*)pack->free, tag);
			for(k = 0; k < SM_SLOT; ++k)
				_vmchkaddress(vh->vm, (Vmuchar_t*)pack->small[k].free, tag);
		}
	}
}

/* find the region containing a block allocated by Vmalloc  */
Vmalloc_t* vmregion(Void_t* addr)
{
	Seg_t		*seg;
	Pack_t		*pack;
	Vmdata_t	*vmdt;
	Vmhold_t	*vh;
	Block_t		*blk = BLOCK(addr);

#define VMHOLD(vm,b)	((Vmuchar_t*)(b) >= (vm)->data->segmin && (Vmuchar_t*)(b) < (vm)->data->segmax )
#define SGHOLD(sg,b)	((Vmuchar_t*)(b) >= (sg)->base && (Vmuchar_t*)(b) < (sg)->base+(sg)->size )

#define CAREFUL	1 /* 1 for any system where different malloc packages may be mixed together */
#ifdef CAREFUL /* blk is ascertained to be allocated in a region before SEG() is invoked */
	if(VMHOLD(Vmheap, addr) )
		for(seg = (vmdt = Vmheap->data)->seg; seg; seg = seg->next)
			if(SGHOLD(seg,blk) && (pack = (Pack_t*)PACK(blk)) && pack->best == (Vmbest_t*)vmdt )
				return Vmheap;

	for(vh = _Vmhold; vh; vh = vh->next)
		if(vh->vm && VMHOLD(vh->vm, addr) ) /* could be in region vh->vm */
			for(seg = (vmdt = vh->vm->data)->seg; seg; seg = seg->next)
				if(SGHOLD(seg,blk) && (pack = (Pack_t*)PACK(blk)) && pack->best == (Vmbest_t*)vmdt )
					return vh->vm;

#else /* !CAREFUL: blk is (optimistically) assumed to be allocated by Vmalloc */
	if(VMHOLD(Vmheap, addr) )
		if((pack = (Pack_t*)PACK(blk))->best == (Vmbest_t*)Vmheap->data )
			return Vmheap; 

	for(vh = _Vmhold; vh; vh = vh->next)
		if(VMHOLD(vh->vm, addr) )
			if(pack->best == (Vmbest_t*)vh->vm->data)
				return vh->vm;
#endif

	if(Vmregion->meth.meth == VM_MTDEBUG)
		return Vmregion;

	return NIL(Vmalloc_t*);
}

/* now define the method */
static Vmethod_t _Vmbest =
{	bestalloc,
	bestresize,
	bestfree,
	0,
	beststat,
	bestevent,
	bestalign,
	VM_MTBEST
};

__DEFINE__(Vmethod_t*, Vmbest, &_Vmbest);

#ifdef NoF
NoF(vmbest)
#endif

#endif
