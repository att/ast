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
#include	"dthdr.h"

/*	Recursive hashing data structure.
**
**      Written by Kiem-Phong Vo (phongvo@gmail.com) and Adam Edgar (09/06/2010).
*/

/* locking and unlocking a class of objects */
#define HCLSLOCK(dt,hh,ty,sh)	((sh) == 0 ? 0 : hclslock((dt), (hh), (ty), 1) )
#define HCLSOPEN(dt,hh,ty,sh)	((sh) == 0 ? 0 : hclslock((dt), (hh), (ty), 0) ) 

#define H_TABLE		DT_HIBIT	/* hibit indicates a table	*/
#define H_NBITS		(ssize_t)(DT_NBITS-1) /* #bits in a hash value	*/
#define HTABLE(l)	((l)->_hash & H_TABLE)  /* test for a table	*/
#define HVALUE(h)	(((uint)(h)) & ~H_TABLE) /* get hash value	*/

#if 0 /* if needed, remix bad hash values with (lbits*511+rbits) */
#define LBITS(v)	(((uint)(v)) >> 5)
#define RBITS(v)	(((uint)(v))  & ((1<<(DT_NBITS-5)) - 1) )
#define REMIX(h)	((LBITS(h) << 9) - LBITS(h) + RBITS(h) )
#define HREMIX(h)	HVALUE(REMIX(h))
#else
#define HREMIX(h)	HVALUE(h)
#endif

/* number of bits and search steps at different trie levels */
#define H_BIT0		20	/* this can be reset by app 	*/
#define H_BIT1		2
#define H_BIT2		2
#define H_BITA		2	/* #bits for all other levels	*/
				/* this must be <= H_BIT[012]	*/

#define H_SRCH0		1	/* 1 to make insert-lock safe	*/
#define H_SRCH1		4
#define H_SRCH2		4
#define H_SRCHA		4

#define H_NLEV		(ssize_t)(DT_NBITS/H_BITA) /* #levels	*/

/* table size at a given level */
#define HSIZE(hh,lv)	((lv) >= (hh)->nlev ? (1<<H_BITA) : (hh)->mask[lv]+1 )

/* the base (starting search) position of an element in a given table and #search steps */
#define HBASP(hh,lv,h)	((lv) >= (hh)->nlev ?  0 : (((h)>>(hh)->shft[lv]) & (hh)->mask[lv]) )
#define HSRCH(hh,lv)	((lv) >= (hh)->nlev ? (1<<H_BITA) : \
				(lv) == 0 ? H_SRCH0 : \
				(lv) == 1 ? H_SRCH1 : \
				(lv) == 2 ? H_SRCH2 : H_SRCHA)

/* the actual address of an element supposed to be at location p */
#define HLNKP(tb,p)	(((tb)->list[p] && HTABLE((tb)->list[p])) ? \
				&((Htbl_t*)(tb)->list[p])->pobj : (tb)->list+(p) )

/* convenient short-hands for operation types requiring locks */
#define H_INSERT	(DT_INSERT|DT_INSTALL|DT_APPEND|DT_ATTACH|DT_RELINK)
#define H_DELETE	(DT_DELETE|DT_DETACH|DT_REMOVE)

typedef struct _htbl_s /* a trie branch or hash table */
{	Dtlink_t	link;	/* parent table & position	*/
	Dtlink_t*	pobj;	/* come down from parent list 	*/
	Dtlink_t*	list[1];/* list of objects or subtables	*/
} Htbl_t;

typedef struct _fngr_s /* finger for faster dtnext(), dtstep()  */
{	Dtlink_t*	here;	/* fingered object		*/
	Htbl_t*		mtbl;	/* table of the fingered object	*/
	ssize_t		mpos;	/* position of object in table	*/
	ssize_t		mlev;	/* level of table in hashtrie	*/
} Fngr_t;

typedef struct _hash_s /* recursive hashing data */
{	Dtdata_t	data;

	Htbl_t*		root;	/* top-most hash table		*/
	ssize_t		nlev;	/* number of nontrivial levels	*/
	ssize_t		mask[H_NLEV];/* bit mask per level 	*/
	ssize_t		shft[H_NLEV];/* shift amount per level	*/

	uchar*		lock;	/* insertion/deletion locks	*/
	ssize_t		lmax;	/* max lock index (2^n -1)	*/
	uint*		refn;	/* reference counts		*/

	Fngr_t		fngr;	/* finger to help dtnext/dtstep	*/
} Hash_t;

#if __STD_C /* lock/unlock a class of objects by hash values */
static int hclslock(Dt_t* dt, uint hsh, int type, int locking)
#else
static int hclslock(dt, hsh, type, locking)
Dt_t*	dt;
uint	hsh;	/* hash to class	*/
int	type;	/* operation type	*/
int	locking; /* (0)1: (un)locking	*/
#endif
{
	Hash_t		*hash = (Hash_t*)dt->data;
	uchar		*lckp = hash->lock + (hsh & hash->lmax);
	uint		*refn = hash->refn + (hsh & hash->lmax);

	if(locking )
	{	if(type & (H_INSERT|H_DELETE) )
		{	asospindecl();
			for(asospininit();; asospinnext())
				if(asocaschar(lckp, 0, 1) == 0) /* got lock */
					break;
		}

		/* increase reference count */
		asoaddint(refn, 1); /**/DEBUG_ASSERT(refn > 0 );
	}
	else
	{	/* decrease reference count */
		asosubint(refn, 1); /**/DEBUG_ASSERT(refn >= 0 );

		if(type & (H_INSERT|H_DELETE) )
		{	/**/DEBUG_ASSERT(*lckp == 1);
			asocaschar(lckp, 1, 0); /* unlock */
		}
	}

	return 0;
}

#if __STD_C /* allocating a trie branch/hash table */
static Htbl_t*	htable(Dt_t* dt, ssize_t lev, Htbl_t* ptbl, ssize_t ppos)
#else
static Htbl_t*	htable(dt, lev, ptbl, hsh)
Dt_t*		dt;	/* dictionary holding objects	*/
ssize_t		lev;	/* level of table		*/
Htbl_t*		ptbl;	/* parent hash table		*/
ssize_t		ppos;	/* position in parent table	*/
#endif
{
	ssize_t		z;
	Htbl_t		*tbl;
	Hash_t		*hash = (Hash_t*)dt->data;
	/**/DEBUG_ASSERT(lev <= 0 || (ppos >= 0 && ppos < HSIZE(hash,lev-1)) );

	/* allocate table and initialize data */
	z = sizeof(Htbl_t) + (HSIZE(hash,lev)-1) * sizeof(Dtlink_t*);
	if(!(tbl = (Htbl_t*)(*dt->memoryf)(dt, 0, z, dt->disc)) )
	{	DTERROR(dt, "Error in allocating a hashtrie table");
		return NIL(Htbl_t*);
	}
	memset(tbl, 0, z);
	tbl->link._ptbl = (Dtlink_t*)ptbl;
	tbl->link._ppos = ((uint)ppos) | H_TABLE;
	return tbl;
}

#if __STD_C /* delete an object from the hashtrie */
static void hdelete(Dt_t* dt, Dtlink_t** lnkp, int type)
#else
static void hdelete(dt, lnkp, type)
Dt_t*		hash;
Dtlink_t**	lnkp;
int		type;
#endif
{
	Dtlink_t	*lnk;
	Hash_t		*hash = (Hash_t*)dt->data;
	int		share = hash->data.type&DT_SHARE;

	if(!(lnk = asogetptr(lnkp)) )
		return;

	/* get the object off the table first */
	/**/DEBUG_ASSERT(!share || hash->lock[lnk->_hash&hash->lmax] != 0);
	/**/DEBUG_ASSERT(!share || hash->refn[lnk->_hash&hash->lmax] >= 1);
	asocasptr(lnkp, lnk, NIL(Dtlink_t*) );

	if(share) /* then wait until no more references to object */
	{	uint *refn = hash->refn + (lnk->_hash & hash->lmax);
		asospindecl();
		for(asospininit();; asospinnext())
			if(asogetint(refn) == 1)
				break;
	}

	_dtfree(dt, lnk, type); /* now it's safe to free object */
}

#if __STD_C /* clear everything from tbl and its children tables */
static Void_t* hclear(Dt_t* dt, Htbl_t* tbl, ssize_t lev, int zap)
#else
static Void_t* hclear(dt, tbl, lev, zap)
Dt_t*	dt;	/* dictionary structure		*/
Htbl_t*	tbl;	/* current table to be cleared	*/
ssize_t	lev;	/* level of current table	*/
int	zap;	/* < 0: free all structures	*/
		/* >=0: free all but save root	*/
#endif
{
	ssize_t		p, tblz;
	Dtlink_t	*t;
	Htbl_t		*ptbl;
	Hash_t		*hash = (Hash_t*)dt->data;
	int		share = hash->data.type&DT_SHARE;

	for(tblz = HSIZE(hash,lev), p = 0; p < tblz; ++p)
	{	if(lev == 0 ) /* lock the class as necessary */
			HCLSLOCK(dt, p, DT_DELETE, share);

		hdelete(dt, HLNKP(tbl,p), DT_DELETE);

		if((t = asogetptr(tbl->list+p)) && HTABLE(t) )
			hclear(dt, (Htbl_t*)t, lev+1, zap);

		if(lev == 0 )
			HCLSOPEN(dt, p, DT_DELETE, share);
	}

	if(zap < 0 || (zap >= 0 && lev > 0) )
	{	if((ptbl = (Htbl_t*)tbl->link._ptbl) )
			ptbl->list[HVALUE(tbl->link._ppos)] = NIL(Dtlink_t*);
		(*dt->memoryf)(dt, (Void_t*)tbl, 0, dt->disc);
	}

	if(lev == 0)
		hash->data.size = 0;

	return NIL(Void_t*);
}

#if __STD_C /* this constitutes the core of dtfirst() and dtlast() */
static Void_t* hfirst(Dt_t* dt, Fngr_t* fngr, Htbl_t* tbl, ssize_t lev, ssize_t pos, uint hsh, int type)
#else
static Void_t* hfirst(dt, fngr, tbl, lev, pos, hsh, type)
Dt_t*	dt;
Fngr_t*	fngr;	/* finger to speed search	*/
Htbl_t*	tbl;	/* subtable to search		*/
ssize_t	lev;	/* level of given subtable	*/
ssize_t	pos;	/* starting position in table 	*/
uint	hsh;	/* top level hash value		*/
int	type;	/* operation type		*/
#endif
{
	ssize_t		tblz;
	Void_t		*obj;
	Dtlink_t	*t, *p;
	Hash_t		*hash = (Hash_t*)dt->data;
	int		share = hash->data.type & DT_SHARE;

	obj = NIL(Void_t*);
	for(tblz = HSIZE(hash,lev); pos < tblz && !obj; ++pos)
	{	if(lev == 0 || type != 0 ) /* type != 0 means not a recursion */
			HCLSLOCK(dt, lev == 0 ? pos : hsh, DT_SEARCH, share);

		if((p = t = asogetptr(tbl->list+pos)) )
		{	if(HTABLE(t) )
			{	if(!(p = asogetptr(&(((Htbl_t*)t)->pobj) ) ) ) /* type == 0 */
					obj = hfirst(dt, fngr, (Htbl_t*)t, lev+1, 0, 0, 0);
				else	goto o_bj; /* p is a valid object */
			}
			else 
			{ o_bj:	obj = _DTOBJ(dt->disc, p);
				if(fngr) /* faster dtnext/dtprev next time */
				{	fngr->here = p;
					fngr->mtbl = tbl;
					fngr->mpos = pos;
					fngr->mlev = lev;
				}
			}
		}

		if(lev == 0 || type != 0)
		{	DTANNOUNCE(dt, obj, type); /* announce obj before opening */
			HCLSOPEN(dt, lev == 0 ? pos : hsh, DT_SEARCH, share);
		}
	}
	return obj;
}

#if __STD_C /* this constitutes the core of dtnext() and dtprev() */
static Void_t* hnext(Dt_t* dt, Fngr_t* fngr, Htbl_t* tbl, ssize_t lev, ssize_t pos, uint hsh, int type)
#else
static Void_t* hnext(dt, fngr, tbl, lev, pos, hsh, type)
Dt_t*	dt;
Fngr_t*	fngr;	/* finger to speed up search	*/
Htbl_t*	tbl;	/* hash subtable to search	*/
ssize_t	lev;	/* subtable level		*/
ssize_t	pos;	/* starting position in table 	*/
uint	hsh;	/* top level hash value		*/
int	type;	/* operation type		*/
#endif
{
	Dtlink_t	*t;
	Void_t		*obj;

	if((t = asogetptr(tbl->list+pos)) && HTABLE(t) )
		if((obj = hfirst(dt, fngr, (Htbl_t*)t, lev+1, 0, hsh, type)) )
			return obj;

	for(;;) /* search forward from current position in table */
	{	if((obj = hfirst(dt, fngr, tbl, lev, pos+1, hsh, type)) )
			return obj;
		else if((lev -= 1) < 0 ) /* just did root table */
			return NIL(Void_t*);
		else /* back up to parent table */
		{	pos = HVALUE(tbl->link._ppos);
			tbl = (Htbl_t*)tbl->link._ptbl;
		}
	}
}

#if __STD_C /* construct a flat list of objects */
static Void_t* hflatten(Dt_t* dt, Dtlink_t** list,Dtlink_t* last, Htbl_t* tbl,ssize_t lev, int zap)
#else
static Void_t* hflatten(dt, list,last, tbl,lev, zap)
Dt_t*		dt;
Dtlink_t**	list;	/* to return chain of objects	*/
Dtlink_t*	last;	/* currently last elt of chain	*/
Htbl_t*		tbl;	/* current hash table to search */
ssize_t		lev;	/* table level			*/
int		zap;	/* erase the location		*/
#endif
{
	ssize_t		tblz, p;
	Dtlink_t	*t, **lnkp;
	Hash_t		*hash = (Hash_t*)dt->data;
	int		share = hash->data.type & DT_SHARE;

	for(tblz = HSIZE(hash,lev), p = 0; p < tblz; ++p)
	{	if(lev == 0 ) /* hard lock in case we need to zap it */
			HCLSLOCK(dt, p, DT_DELETE, share);

		lnkp = HLNKP(tbl, p);
		if((t = asogetptr(lnkp)) )
		{	if(last) /* append to flattened list */
				last = (last->_rght = t);
			else	last = (*list = t);
			if(zap) /* just clearing structure, not freeing object */
				*lnkp = NIL(Dtlink_t*);
		}

		if((t = asogetptr(tbl->list+p)) && HTABLE(t) )
			last = hflatten(dt, list, last, (Htbl_t*)t, lev+1, zap);

		if(lev == 0 )
			HCLSOPEN(dt, p, DT_DELETE, share);
	}

	if(last)
		last->_rght = NIL(Dtlink_t*);
	return last;
}

#if __STD_C /* construct/extract a list of objects, or reconstruct from a list */
static Void_t* hlist(Dt_t* dt, Dtlink_t* list, int type)
#else
static Void_t* hlist(dt, list, type)
Dt_t*		dt;
Dtlink_t*	list;
int		type;
#endif
{
	Void_t		*obj;
	Dtlink_t	*next, *l;
	Dtdisc_t	*disc = dt->disc;
	Hash_t		*hash = (Hash_t*)dt->data;

	if(type&(DT_EXTRACT|DT_FLATTEN) )
	{	list = NIL(Dtlink_t*);
		(void)hflatten(dt, &list, NIL(Dtlink_t*), hash->root, 0, (type&DT_EXTRACT) );
	}
	else /*if(type&DT_RESTORE)*/
	{	hash->data.size = 0;
		for(l = list; l; l = next)
		{	next = l->_rght;
			obj = _DTOBJ(disc,l);
			if((dt->meth->searchf)(dt, (Void_t*)l, DT_RELINK) == obj)
				hash->data.size += 1;
		}
	}

	return (Void_t*)list;
}

#if __STD_C /* compute size and depth of a hash table */
static ssize_t hsize(Dt_t* dt, Htbl_t* tbl, ssize_t lev, Dtstat_t* st)
#else
static ssize_t hsize(dt, tbl, lev, st)
Dt_t*		dt;
Htbl_t*		tbl;
ssize_t		lev;
Dtstat_t*	st;
#endif
{
	Dtlink_t	*t;
	ssize_t		p, z, rz, tblz, size;
	Hash_t		*hash = (Hash_t*)dt->data;
	int		share = hash->data.type & DT_SHARE;

	if(lev >= DT_MAXRECURSE) /* avoid blowing the stack */
		return -1;

	size = 0;
	for(tblz = HSIZE(hash,lev), p = 0; p < tblz; ++p)
	{	if(lev == 0)
			HCLSLOCK(dt, p, DT_SEARCH, share);

		z = rz = 0;
		if((t = asogetptr(tbl->list+p)) )
		{	if(!HTABLE(t))
				z = 1;
			else if((rz = hsize(dt, (Htbl_t*)t, lev+1, st)) >= 0 )
				z = ((Htbl_t*)t)->pobj ? 1 : 0;
		}
		if(rz >= 0)
		{	size += z+rz;
			if(z > 0 && st && lev < DT_MAXSIZE)
				st->lsize[lev] += z;
		}

		if(lev == 0)
			HCLSOPEN(dt, p, DT_SEARCH, share);

		if(rz < 0) /* failed at some recursion level */
			return -1;
	}

	if(st)
	{	st->tslot = HSIZE(hash,0);
		st->mlev = lev > st->mlev ? lev : st->mlev;
		if(lev < DT_MAXSIZE)
		{	st->msize = lev > st->msize ? lev : st->msize;
			st->tsize[lev] += HSIZE(hash,lev); /* #slots per level */
		}
		st->space += sizeof(Htbl_t) + (tblz-1)*sizeof(Dtlink_t*);
	}

	return size;
}

#if __STD_C
static Void_t* hstat(Dt_t* dt, Dtstat_t* st)
#else
static Void_t* hstat(dt, st)
Dt_t*		dt;
Dtstat_t*	st;
#endif
{
	ssize_t	size;
	Hash_t	*hash = (Hash_t*)dt->data;

	if(!st)
		size = hash->data.size;
	else
	{	memset(st, 0, sizeof(Dtstat_t));
		st->meth  = dt->meth->type;
		st->size  = size = hsize(dt, hash->root, 0, st);
		st->space = sizeof(Hash_t) + (dt->disc->link >= 0 ? 0 : size*sizeof(Dthold_t));
		/**/DEBUG_ASSERT((dt->data->type&DT_SHARE) || size == hash->data.size);
	}

	return (Void_t*)size;
}

static Void_t* dthashtrie(Dt_t* dt, Void_t* obj, int type)
{
	Void_t		*o, *ky, *key;
	uint		hsh;
	Dtlink_t	*lnk, **lnkp, *t;
	ssize_t		k, pos, lev, srch, hshp, modz, opnp, oplv;
	Htbl_t		*tbl, *opnt;
	Dtdisc_t	*disc = dt->disc;
	Hash_t		*hash = (Hash_t*)dt->data;
	Fngr_t		*fngr = &hash->fngr; /* default finger */
	uint		share = hash->data.type&DT_SHARE;
	/**/DEBUG_DECLARE(static int, N_trie) DEBUG_COUNT(N_trie);

	type = DTTYPE(dt,type); /* map type for upward compatibility */
	if(!(type&DT_OPERATIONS) || !hash->root )
		return NIL(Void_t*);

	/* wipe cached data as they may become stale after these ops */
	if(type&(H_INSERT|H_DELETE|DT_CLEAR) )
		fngr->here = NIL(Dtlink_t*);

	/* operations not dealing with a particular object or walk-related */
	if(type&(DT_START|DT_STEP|DT_STOP|DT_FIRST|DT_LAST|DT_CLEAR|DT_STAT|DT_EXTRACT|DT_RESTORE|DT_FLATTEN) )
	{	if(type&DT_START) /* starting a walk */
		{	if(!(fngr = (*dt->memoryf)(dt, NIL(Void_t*), sizeof(Fngr_t), disc)) )
				return NIL(Void_t*);
			if(!obj) /* start walk from the first element */
			{	if(!(obj = hfirst(dt, fngr, hash->root, 0, 0, 0, type)) )
					(*dt->memoryf)(dt, fngr, 0, disc);
				return obj ? (Void_t*)fngr : NIL(Void_t*);
			}
			/* else, search for obj below */
		}
		else if(type&DT_STEP ) /* take a step forward in a walk */
		{	if(!(fngr = (Fngr_t*)obj) || !(lnk = fngr->here) )
				return NIL(Void_t*);
			obj = _DTOBJ(disc,lnk);
			if(!hnext(dt, fngr, fngr->mtbl, fngr->mlev, fngr->mpos, lnk->_hash, type) )
				fngr->here = NIL(Dtlink_t*); /* walk will end after this call */

			DTANNOUNCE(dt, obj, type);
			return obj;
		}
		else if(type&DT_STOP) /* stop a walk */
		{	if(obj) /* free associated memory */
				(*dt->memoryf)(dt, obj, 0, disc);
			return NIL(Void_t*);
		}
		else if(type&(DT_FIRST|DT_LAST))
			return hfirst(dt, fngr, hash->root, 0, 0, 0, type);
		else if(type&DT_CLEAR)
			return hclear(dt, hash->root, 0, 0);
		else if(type&DT_STAT)
			return hstat(dt, (Dtstat_t*)obj);
		else /*if(type&(DT_EXTRACT|DT_RESTORE|DT_FLATTEN))*/
			return hlist(dt, (Dtlink_t*)obj, type);
	}

	if(!obj) /* from here on, a non-NULL object is needed */
		return NIL(Void_t*);

	/* optimization for fast walking of a dictionary */
	if(!share && (type&(DT_NEXT|DT_PREV)) && (lnk = fngr->here) && obj == _DTOBJ(disc,lnk) &&
		(tbl = fngr->mtbl) && (pos = fngr->mpos) >= 0 && (lev = fngr->mlev) >= 0 )
			return hnext(dt, fngr, tbl, lev, pos, lnk->_hash, type);

	if(type&DT_RELINK) /* reinserting an object */
	{	lnk = (Dtlink_t*)obj;
		obj = _DTOBJ(disc,lnk);
		key = _DTKEY(disc,obj);
	}
	else
	{	lnk = NIL(Dtlink_t*);
		if(type&DT_MATCH)
		{	key = obj;
			obj = NIL(Void_t*);
		}
		else	key = _DTKEY(disc,obj);
	}
	hsh = _DTHSH(dt,key,disc); /* hash value from hash function */
	hsh = HREMIX(hsh); /* remix value for what we need */

	HCLSLOCK(dt, hsh, type, share); /* lock and reference counting */

	opnt = NIL(Htbl_t*); opnp = oplv = -1; /* open space suitable for insertion */
	for(tbl = hash->root, lev = 0;; )
	{	hshp = HBASP(hash,lev,hsh); /* base location of object */
		srch = HSRCH(hash,lev); /* number of search steps */
		modz = HSIZE(hash,lev)-1; /* circular search of table */
		for(pos = hshp, k = 0; k < srch; ++k, pos = ((pos+1)&modz) )
		{	if((t = asogetptr(tbl->list+pos)) && HTABLE(t) )	
				t = ((Htbl_t*)t)->pobj;

			if(!t) /* empty slot */
			{	if(!opnt && (type&H_INSERT) )
					{ opnt = tbl; opnp = pos; }
				continue;
			}
			else if(t->_hash != hsh ) /* cannot match */
				continue;
			else /* potential match, verify */
			{	o = _DTOBJ(disc,t); ky = _DTKEY(disc,o);
				if(_DTCMP(dt, key, ky, disc) != 0 )
					continue;
				else if(type&(DT_REMOVE|DT_NEXT|DT_PREV|DT_START) )
				{	if(type&DT_START) /* starting a walk, return fingered data */
					{	fngr->here = t;
						fngr->mtbl = tbl;
						fngr->mpos = pos;
						fngr->mlev = lev;

						HCLSOPEN(dt, hsh, type, share);
						return (Void_t*)fngr;
					}
					else if(o != obj) /* try to find exact object in case of a bag */
					{	if(type&(DT_NEXT|DT_PREV) ) /* track last matched object */
							{ opnt = tbl; opnp = pos; oplv = lev; }
						continue;
					}
				}
			}

			/* if get here, a matching object was found */
			if(type&(DT_SEARCH|DT_MATCH|DT_ATLEAST|DT_ATMOST) )
			{	obj = _DTOBJ(disc,t); /* save in case deleted on unlocking */
				DTANNOUNCE(dt, obj, type);
				HCLSOPEN(dt, hsh, type, share);
				return obj;
			}
			else if(type&(DT_NEXT|DT_PREV) )
			{	HCLSOPEN(dt, hsh, type, share);
				return hnext(dt, fngr, tbl, lev, pos, hsh, type);
			}
			else if(type&(DT_DELETE|DT_DETACH|DT_REMOVE) ) 
			{	obj = _DTOBJ(disc,t); /* save before deletion */
				hdelete(dt, HLNKP(tbl,pos), type);
				asosubsize(&hash->data.size, 1);
				DTANNOUNCE(dt, obj, type);
				HCLSOPEN(dt, hsh, type, share);
				return obj;
			}
			else
			{	/**/DEBUG_ASSERT(type&H_INSERT);
				if(!(dt->meth->type&DT_RHBAG) ) /* no duplicates */
				{	if(type & (DT_INSERT|DT_APPEND|DT_ATTACH) )
						type |= DT_MATCH; /* for announcement */
					else if(type&DT_RELINK )
					{	/**/DEBUG_ASSERT(lnk);
						o = _DTOBJ(disc, t); /* remove a duplicate */
						_dtfree(dt, lnk, DT_DELETE);
						DTANNOUNCE(dt, o, DT_DELETE);
					}
					else if(type&DT_INSTALL)
					{	o = _DTOBJ(disc, t); /* remove old object */
						lnkp = HLNKP(tbl, pos);
						hdelete(dt, lnkp, DT_DELETE);
						DTANNOUNCE(dt, o, DT_DELETE);
						if(!opnt)
							{ opnt = tbl; opnp = pos; oplv = lev; }
						goto do_insert;
					}

					obj = _DTOBJ(disc,t); /* save before unlocking */
					DTANNOUNCE(dt, obj, type);
					HCLSOPEN(dt, hsh, type, share);
					return obj;
				}
				else if(opnt) /* already got an open slot */
					goto do_insert;
				else for(;;) /* try finding an open slot */
				{	for(; k < srch; ++k, pos = ((pos+1)&modz) )
					{	lnkp = HLNKP(tbl,pos);
						if(!asogetptr(lnkp) )
						{	opnt = tbl; opnp = pos;
							goto do_insert;
						}
					}

					if((t = asogetptr(tbl->list+hshp)) && HTABLE(t))
					{	tbl = (Htbl_t*)t;
						lev += 1;
						hshp = HBASP(hash,lev,hsh); pos = hshp;
						srch = HSRCH(hash,lev); k = 0;
						modz = HSIZE(hash,lev)-1;
					}
					else	goto do_insert;
				}
			}
		}

		if((t = asogetptr(tbl->list+hshp)) && HTABLE(t) )
		{	tbl = (Htbl_t*)t;
			lev += 1; /* continue search by recursion */
		}
		else if(type&(DT_NEXT|DT_PREV|DT_START) )
		{	HCLSOPEN(dt, hsh, type, share);
			if(type&DT_START) /* no matching object, no walk */
			{	(void)(*dt->memoryf)(dt, (Void_t*)fngr, 0, disc);
				return NIL(Void_t*);
			}
			else if(opnt) /* (opnt,opnp,oplv) is the last known matching obj */
				return hnext(dt, fngr, opnt, oplv, opnp, hsh, type);
			else	return NIL(Void_t*);
		}
		else if(type&H_INSERT)
			goto do_insert; /* inserting a  new object */
		else /* search/delete failed */
		{	HCLSOPEN(dt, hsh, type, share);
			return NIL(Void_t*);
		}
	}

do_insert: /**/DEBUG_ASSERT(tbl && hshp >= 0 && hshp < HSIZE(hash,lev) );
	if(!opnt) /* make a new subtable */
	{	lev += 1;
		if(!(opnt = htable(dt, lev, tbl, hshp)) )
		{	HCLSOPEN(dt, hsh, type, share);
			return NIL(Void_t*);
		}
		opnp = HBASP(hash,lev,hsh); /* new insert location */

		/**/DEBUG_ASSERT(!tbl->list[hshp] || !HTABLE(tbl->list[hshp]));
		opnt->pobj = tbl->list[hshp]; /* move slot content to drop down slot */
		asocasptr(tbl->list+hshp, opnt->pobj, (Dtlink_t*)opnt);
	}

	if(!lnk && (lnk = _dtmake(dt, obj, type) ) )
		asoaddsize(&hash->data.size, 1);
	if(lnk)
	{	lnk->_hash = hsh; /* memoize hash for fast compares */
		lnkp = HLNKP(opnt,opnp); /**/DEBUG_ASSERT(*lnkp == NIL(Dtlink_t*) );
		asocasptr(lnkp, NIL(Dtlink_t*), lnk);
	}

	DTANNOUNCE(dt, obj, type);
	HCLSOPEN(dt, hsh, type, share);
	return lnk ? _DTOBJ(disc,lnk) : NIL(Void_t*);
}

static int hashevent(Dt_t* dt, int event, Void_t* arg)
{
	ssize_t		z, b, k;
	Dtdisc_t	*disc = dt->disc;
	Hash_t		*hash = (Hash_t*)dt->data;

	if(!(disc = dt->disc) )
		return -1;

	if(event == DT_OPEN)
	{	if(hash) /* already allocated private data */
			return 0;

		if(!(hash = (Hash_t*)(*dt->memoryf)(dt, 0, sizeof(Hash_t), dt->disc)) )
		{	DTERROR(dt, "Error in allocating a hashtrie table");
			return -1;
		}
		memset(hash, 0, sizeof(Hash_t));
		dt->data = (Dtdata_t*)hash;

		z = 0; /* get size of the root table and #bits */
		if(disc && disc->eventf &&
		   (*disc->eventf)(dt, DT_HASHSIZE, &z, disc) > 0 )
			z = z < 0 ? -z : z;
		z = z == 0 ? (1<<H_BIT0)-1 : z;
		for(b = DT_HTABLE; b < H_NBITS; ++b) /* count #bits */
			if((1<<b) >= z)
				break;

		hash->shft[0] = 0; /* amount to shift right before masking */
		hash->mask[0] = (1<<b) - 1; /* mask to get bits after shifting */
		for(k = 1; k < H_NLEV && b < H_NBITS; ++k)
		{	z = k == 1 ? H_BIT1 : k == 2 ? H_BIT2 :
			    k >= H_NLEV-1 ? H_NBITS-b : /* last level gets all bits */
			    (H_NBITS-b) >= H_BITA ? H_BITA : (H_NBITS-b);

			hash->shft[k] = b; b += z;
			hash->mask[k] = (1<<z) - 1;
		}
		hash->nlev = k; /* total number of levels for trie */

		if(!(hash->root = htable(dt, 0, NIL(Htbl_t*), -1)) ) /* make root table */
		{	(void)(*dt->memoryf)(dt, hash, 0, disc);
			dt->data = NIL(Dtdata_t*);
			return -1;
		}
		else	return 1;
	}
	else if(event == DT_CLOSE)
	{	if(!hash)
			return 0;
		if(hash->root) /* free all objects in dictionary */
			(void)hclear(dt, hash->root, 0, -1);
		if(hash->lock) /* free the lock table, if any */
			(void)(*dt->memoryf)(dt, hash->lock, 0, disc);
		if(hash->refn) /* free the hazard table, if any */
			(void)(*dt->memoryf)(dt, hash->refn, 0, disc);
		(void)(*dt->memoryf)(dt, hash, 0, disc);
		dt->data = NIL(Dtdata_t*);
		return 0;
	}
	else if(event == DT_SHARE) /* turn on/off concurrency - return 1 on success */
	{	if(!hash)
			return -1;

		if((int)((Dtuint_t)arg) > 0 ) /* set up structures for share mode */
		{	if(!hash->lock) /* allocate lock table */
			{	z = (hash->mask[0]+1)*sizeof(uchar);
				if(!(hash->lock = (uchar*)(*dt->memoryf)(dt, 0, z, dt->disc)) )
				{	DTERROR(dt, "Error in allocating hashtrie locks");
					return -1;
				}
				memset(hash->lock, 0, z);
				hash->lmax = z-1; /* max lock index - usable for modulo 2^n */
			}

			if(!hash->refn) /* allocate hazard table */
			{	z = (hash->mask[0]+1)*sizeof(uint);
				if(!(hash->refn = (uint*)(*dt->memoryf)(dt, 0, z, dt->disc)) )
				{	DTERROR(dt, "Error in allocating hashtrie references");
					return -1;
				}
				memset(hash->refn, 0, z);
			}
		}
		return 1;
	}
	else	return 0;
}


static Dtmethod_t	_Dtrhset = { dthashtrie, DT_RHSET, hashevent, "Dtrhset" };
static Dtmethod_t	_Dtrhbag = { dthashtrie, DT_RHBAG, hashevent, "Dtrhbag" };
__DEFINE__(Dtmethod_t*,Dtrhset,&_Dtrhset);
__DEFINE__(Dtmethod_t*,Dtrhbag,&_Dtrhbag);

#ifdef NoF
NoF(dthashtrie)
#endif
