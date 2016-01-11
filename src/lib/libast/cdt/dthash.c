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

/*	Hash table with chaining for collisions.
**
**      Written by Kiem-Phong Vo, phongvo@gmail.com (05/25/96)
*/

/* these bits should be outside the scope of DT_METHODS */
#define H_FIXED		0100000	/* table size is fixed	*/
#define	H_FLATTEN	0200000	/* table was flattened	*/

#define HLOAD(n)	(n)	/* load one-to-one	*/

/* internal data structure for hash table with chaining */
typedef struct _dthash_s
{	Dtdata_t	data;
	int		type; 
	unsigned int	walk;	/* on-going walks	*/
	Dtlink_t*	here;	/* fingered object	*/
	Dtlink_t**	htbl;	/* hash table slots 	*/
	ssize_t		tblz;	/* size of hash table 	*/
} Dthash_t;

/* make/resize hash table */
static int htable(Dt_t* dt)
{
	Dtlink_t	**htbl, **t, **endt, *l, *next;
	ssize_t		n, k;
	Dtdisc_t	*disc = dt->disc;
	Dthash_t	*hash = (Dthash_t*)dt->data;

	if((n = hash->tblz) > 0 && (hash->type&H_FIXED) )
		return 0; /* fixed size table */

	if(disc && disc->eventf) /* let user have input */
	{	if((*disc->eventf)(dt, DT_HASHSIZE, &n, disc) > 0 )
		{	if(n < 0) /* fix table size */
			{	hash->type |= H_FIXED;
				n = -n; /* desired table size */
				if(hash->tblz >= n ) /* table size is fixed now */
					return 0;
			}
		}
	}

	/* table size should be a power of 2 */
	n = n < HLOAD(hash->data.size) ? HLOAD(hash->data.size) : n;
	for(k = (1<<DT_HTABLE); k < n; )
		k *= 2;
	if((n = k) <= hash->tblz)
		return 0;

	/* allocate new table */
	if(!(htbl = (Dtlink_t**)(*dt->memoryf)(dt, 0, n*sizeof(Dtlink_t*), disc)) )
	{	DTERROR(dt, "Error in allocating an extended hash table");
		return -1;
	}
	memset(htbl, 0, n*sizeof(Dtlink_t*));

	/* move objects into new table */
	for(endt = (t = hash->htbl) + hash->tblz; t < endt; ++t)
	{	for(l = *t; l; l = next)
		{	next = l->_rght;
			l->_rght = htbl[k = l->_hash&(n-1)];
			htbl[k] = l;
		}
	}

	if(hash->htbl) /* free old table and set new table */
		(void)(*dt->memoryf)(dt, hash->htbl, 0, disc);
	hash->htbl = htbl;
	hash->tblz = n;

	return 0;
}

static Void_t* hclear(Dt_t* dt)
{
	Dtlink_t	**t, **endt, *l, *next;
	Dthash_t	*hash = (Dthash_t*)dt->data;

	hash->here = NIL(Dtlink_t*);
	hash->data.size = 0;

	for(endt = (t = hash->htbl) + hash->tblz; t < endt; ++t)
	{	for(l = *t; l; l = next)
		{	next = l->_rght;
			_dtfree(dt, l, DT_DELETE);
		}
		*t = NIL(Dtlink_t*);
	}

	return NIL(Void_t*);
}

static Void_t* hfirst(Dt_t* dt)
{
	Dtlink_t	**tbl, **endt, *lnk;
	Dthash_t	*hash = (Dthash_t*)dt->data;

	lnk = NIL(Dtlink_t*);
	for(endt = (tbl = hash->htbl) + hash->tblz; tbl < endt; ++tbl)
		if((lnk = *tbl) )
			break;
	hash->here = lnk;
	return lnk ? _DTOBJ(dt->disc, lnk) : NIL(Void_t*);
}

static Void_t* hnext(Dt_t* dt, Dtlink_t* lnk)
{
	Dtlink_t	**tbl, **endt, *next;
	Dthash_t	*hash = (Dthash_t*)dt->data;

	if(!(next = lnk->_rght) ) /* search for next object */
	{	tbl = hash->htbl + (lnk->_hash & (hash->tblz-1)) + 1;
		endt = hash->htbl + hash->tblz;
		for(; tbl < endt; ++tbl)
			if((next = *tbl) )
				break;
	}

	hash->here = next;
	return next ? _DTOBJ(dt->disc, next) : NIL(Void_t*);
}

static Void_t* hflatten(Dt_t* dt, int type)
{
	Dtlink_t	**tbl, **endt, *head, *tail, *lnk;
	Dthash_t	*hash = (Dthash_t*)dt->data;

	if(type == DT_FLATTEN || type == DT_EXTRACT)
	{	head = tail = NIL(Dtlink_t*);
		for(endt = (tbl = hash->htbl) + hash->tblz; tbl < endt; ++tbl)
		{	for(lnk = *tbl; lnk; lnk = lnk->_rght)
			{	if(tail)
					tail = (tail->_rght = lnk);
				else	head = tail = lnk;

				*tbl = type == DT_FLATTEN ? tail : NIL(Dtlink_t*);
			}
		}

		if(type == DT_FLATTEN)
		{	hash->here = head;
			hash->type |= H_FLATTEN;
		}
		else	hash->data.size = 0;

		return (Void_t*)head;
	}
	else /* restoring a previous flattened list */
	{	head = hash->here;
		for(endt = (tbl = hash->htbl) + hash->tblz; tbl < endt; ++tbl)
		{	if(*tbl == NIL(Dtlink_t*))
				continue;

			/* find the tail of the list for this slot */
			for(lnk = head; lnk && lnk != *tbl; lnk = lnk->_rght)
				;
			if(!lnk) /* something is seriously wrong */
				return NIL(Void_t*);

			*tbl = head; /* head of list for this slot */
			head = lnk->_rght; /* head of next list */
			lnk->_rght = NIL(Dtlink_t*);
		}

		hash->here = NIL(Dtlink_t*);
		hash->type &= ~H_FLATTEN;

		return NIL(Void_t*);
	}
}

static Void_t* hlist(Dt_t* dt, Dtlink_t* list, int type)
{
	Void_t		*obj;
	Dtlink_t	*lnk, *next;
	Dtdisc_t	*disc = dt->disc;

	if(type&DT_FLATTEN)
		return hflatten(dt, DT_FLATTEN);
	else if(type&DT_EXTRACT)
		return hflatten(dt, DT_EXTRACT);
	else /* if(type&DT_RESTORE) */
	{	dt->data->size = 0;
		for(lnk = list; lnk; lnk = next)
		{	next = lnk->_rght;
			obj = _DTOBJ(disc,lnk);
			if((*dt->meth->searchf)(dt, (Void_t*)lnk, DT_RELINK) == obj)
				dt->data->size += 1;
		}
		return (Void_t*)list;
	}
}

static Void_t* hstat(Dt_t* dt, Dtstat_t* st)
{
	ssize_t		n;
	Dtlink_t	**tbl, **endt, *lnk;
	Dthash_t	*hash = (Dthash_t*)dt->data;

	if(st)
	{	memset(st, 0, sizeof(Dtstat_t));
		st->meth  = dt->meth->type;
		st->size  = hash->data.size;
		st->space = sizeof(Dthash_t) + hash->tblz*sizeof(Dtlink_t*) +
			    (dt->disc->link >= 0 ? 0 : hash->data.size*sizeof(Dthold_t));

		for(endt = (tbl = hash->htbl) + hash->tblz; tbl < endt; ++tbl)
		{	for(n = 0, lnk = *tbl; lnk; lnk = lnk->_rght)
			{	if(n < DT_MAXSIZE)
					st->lsize[n] += 1;
				n += 1;
			}
			st->mlev = n > st->mlev ? n : st->mlev;
			if(n < DT_MAXSIZE) /* if chain length is small */
				st->msize = n > st->msize ? n : st->msize;
		}
	}

	return (Void_t*)hash->data.size;
}

#if __STD_C
static Void_t* dthashchain(Dt_t* dt, Void_t* obj, int type)
#else
static Void_t* dthashchain(dt,obj,type)
Dt_t*	dt;
Void_t*	obj;
int	type;
#endif
{
	Dtlink_t	*lnk, *pp, *ll, *p, *l, **tbl;
	Void_t		*key, *k, *o;
	uint		hsh;
	Dtlink_t	**fngr = NIL(Dtlink_t**);
	Dtdisc_t	*disc = dt->disc;
	Dthash_t	*hash = (Dthash_t*)dt->data;

	type = DTTYPE(dt,type); /* map type for upward compatibility */
	if(!(type&DT_OPERATIONS) )
		return NIL(Void_t*);

	DTSETLOCK(dt);

	if(!hash->htbl && htable(dt) < 0 ) /* initialize hash table */
		DTRETURN(obj, NIL(Void_t*));

	if(hash->type&H_FLATTEN) /* restore flattened list */
		hflatten(dt, 0);

	if(type&(DT_START|DT_STEP|DT_STOP|DT_FIRST|DT_LAST|DT_CLEAR|DT_EXTRACT|DT_RESTORE|DT_FLATTEN|DT_STAT) )
	{	if(type&DT_START)
		{	if(!(fngr = (Dtlink_t**)(*dt->memoryf)(dt, NIL(Void_t*), sizeof(Dtlink_t*), disc)) )
				DTRETURN(obj, NIL(Void_t*));
			if(!obj)
			{	if(!(obj = hfirst(dt)) ) /* nothing to walk over */
				{	(void)(*dt->memoryf)(dt, (Void_t*)fngr, 0, disc);
					DTRETURN(obj, NIL(Void_t*));
				}
				else
				{	asoaddint(&hash->walk, 1); /* increase walk count */
					*fngr = hash->here; /* set finger to first object */
					DTRETURN(obj, (Void_t*)fngr);
				}
			}
			/* else: fall through to search for obj */
		}
		else if(type&DT_STEP)
		{	if(!(fngr = (Dtlink_t**)obj) || !(lnk = *fngr) )
				DTRETURN(obj, NIL(Void_t*));
			obj = _DTOBJ(disc,lnk);
			*fngr = NIL(Dtlink_t*);
			/* fall through to search for obj */
		}
		else if(type&DT_STOP)
		{	if(obj) /* free allocated memory */
				(void)(*dt->memoryf)(dt, obj, 0, disc);
			asosubint(&hash->walk, 1); /* reduce walk count */
			DTRETURN(obj, NIL(Void_t*));
		}
		else if(type&(DT_FIRST|DT_LAST) )
			DTRETURN(obj, hfirst(dt));
		else if(type&DT_CLEAR)
			DTRETURN(obj, hclear(dt));
		else if(type&DT_STAT)
			DTRETURN(obj, hstat(dt, (Dtstat_t*)obj));
		else /*if(type&(DT_EXTRACT|DT_RESTORE|DT_FLATTEN))*/
			DTRETURN(obj, hlist(dt, (Dtlink_t*)obj, type));
	}

	lnk = hash->here; /* fingered object */
	hash->here = NIL(Dtlink_t*);

	if(lnk && obj == _DTOBJ(disc,lnk))
	{	if(type&DT_SEARCH)
			DTRETURN(obj, obj);
		else if(type&(DT_NEXT|DT_PREV) )
			DTRETURN(obj, hnext(dt, lnk) );
		else if(type&DT_START)
		{	*fngr = lnk; /* set finger to found object */
			DTRETURN(obj, (Void_t*)fngr);
		}
		else if(type&DT_STEP) /* return obj and set finger to next */
		{	*fngr = hnext(dt, lnk) ? hash->here : NIL(Dtlink_t*);
			DTRETURN(obj, obj);
		}
	}

	if(type&DT_RELINK)
	{	lnk = (Dtlink_t*)obj;
		obj = _DTOBJ(disc,lnk);
		key = _DTKEY(disc,obj);
	}
	else 
	{	lnk = NIL(Dtlink_t*);
		if((type&DT_MATCH) )
		{	key = obj;
			obj = NIL(Void_t*);
		}
		else	key = _DTKEY(disc,obj);
	}
	hsh = _DTHSH(dt,key,disc);

	tbl = hash->htbl + (hsh & (hash->tblz-1));
	pp = ll = NIL(Dtlink_t*); /* pp is the before, ll is the here */
	for(p = NIL(Dtlink_t*), l = *tbl; l; p = l, l = l->_rght)
	{	if(hsh == l->_hash)
		{	o = _DTOBJ(disc,l); k = _DTKEY(disc,o);
			if(_DTCMP(dt, key, k, disc) != 0 )
				continue;
			else if((type&(DT_REMOVE|DT_NEXT|DT_PREV|DT_STEP)) && o != obj )
			{	if(type&(DT_NEXT|DT_PREV|DT_STEP) )
					{ pp = p; ll = l; }
				continue;
			}
			else	break;
		}
	}
	if(l) /* found an object, use it */ 
		{ pp = p; ll = l; }

	if(ll) /* found object */
	{	if(type&(DT_SEARCH|DT_MATCH|DT_ATLEAST|DT_ATMOST) )
		{	hash->here = ll;
			DTRETURN(obj, _DTOBJ(disc,ll));
		}
		else if(type & DT_START) /* starting a good walk */
		{	*fngr = hash->here = ll;
			asoaddint(&hash->walk,1); /* up reference count */
			DTRETURN(obj, (Void_t*)fngr);
		}
		else if(type & DT_STEP) /* return obj and set finger to next */
		{	*fngr = hnext(dt, ll) ? hash->here : NIL(Dtlink_t*);
			DTRETURN(obj, obj);
		}
		else if(type & (DT_NEXT|DT_PREV) )
			DTRETURN(obj, hnext(dt, ll));
		else if(type & (DT_DELETE|DT_DETACH|DT_REMOVE) )
		{	hash->data.size -= 1;
			if(pp)
				pp->_rght = ll->_rght;
			else	*tbl = ll->_rght;
			_dtfree(dt, ll, type);
			DTRETURN(obj, _DTOBJ(disc,ll));
		}
		else if(type & DT_INSTALL )
		{	if(dt->meth->type&DT_BAG)
				goto do_insert;
			else if(!(lnk = _dtmake(dt, obj, type)) )
				DTRETURN(obj, NIL(Void_t*) );
			else /* replace old object with new one */
			{	if(pp) /* remove old object */
					pp->_rght = ll->_rght;
				else	*tbl = ll->_rght;
				o = _DTOBJ(disc,ll);
				_dtfree(dt, ll, DT_DELETE);
				DTANNOUNCE(dt, o, DT_DELETE);

				goto do_insert;
			}
		}
		else
		{	/**/DEBUG_ASSERT(type&(DT_INSERT|DT_ATTACH|DT_APPEND|DT_RELINK));
			if((dt->meth->type&DT_BAG) )
				goto do_insert;
			else
			{	if(type&(DT_INSERT|DT_APPEND|DT_ATTACH) )
					type |= DT_MATCH; /* for announcement */
				else if(lnk && (type&DT_RELINK) )
				{	/* remove a duplicate */
					o = _DTOBJ(disc, lnk);
					_dtfree(dt, lnk, DT_DELETE);
					DTANNOUNCE(dt, o, DT_DELETE);
				}
				DTRETURN(obj, _DTOBJ(disc,ll));
			}
		}
	}
	else /* no matching object */
	{	if(!(type&(DT_INSERT|DT_INSTALL|DT_APPEND|DT_ATTACH|DT_RELINK)) )
		{	if(type&DT_START) /* cannot start a walk from nowhere */
				(void)(*dt->memoryf)(dt, (Void_t*)fngr, 0, disc);
			else if(type&DT_STEP)
				*fngr = NIL(Dtlink_t*);
			DTRETURN(obj, NIL(Void_t*));
		}

	do_insert: /* inserting a new object */
		if(asogetint(&hash->walk) == 0 && hash->tblz < HLOAD(hash->data.size) )
		{	htable(dt); /* resize table */
			tbl = hash->htbl + (hsh & (hash->tblz-1));
		}

		if(!lnk) /* inserting a new object */
		{	if(!(lnk = _dtmake(dt, obj, type)) )
				DTRETURN(obj, NIL(Void_t*));
			hash->data.size += 1;
		}

		lnk->_hash = hsh; /* memoize the hash value */
		lnk->_rght = *tbl; *tbl = lnk;

		hash->here = lnk;
		DTRETURN(obj, _DTOBJ(disc,lnk));
	}

dt_return:
	DTANNOUNCE(dt, obj, type);
	DTCLRLOCK(dt);
	return obj;
}

static int hashevent(Dt_t* dt, int event, Void_t* arg)
{
	Dthash_t	*hash = (Dthash_t*)dt->data;

	if(event == DT_OPEN)
	{	if(hash)
			return 0;
		if(!(hash = (Dthash_t*)(*dt->memoryf)(dt, 0, sizeof(Dthash_t), dt->disc)) )
		{	DTERROR(dt, "Error in allocating a hash table with chaining");
			return -1;
		}
		memset(hash, 0, sizeof(Dthash_t));
		dt->data = (Dtdata_t*)hash;
		return 1;
	}
	else if(event == DT_CLOSE)
	{	if(!hash)
			return 0;
		if(hash->data.size > 0 )
			(void)hclear(dt);
		if(hash->htbl)
			(void)(*dt->memoryf)(dt, hash->htbl, 0, dt->disc);
		(void)(*dt->memoryf)(dt, hash, 0, dt->disc);
		dt->data = NIL(Dtdata_t*);
		return 0;
	}
	else	return 0;
}

static Dtmethod_t	_Dtset = { dthashchain, DT_SET, hashevent, "Dtset" };
static Dtmethod_t	_Dtbag = { dthashchain, DT_BAG, hashevent, "Dtbag" };
__DEFINE__(Dtmethod_t*,Dtset,&_Dtset);
__DEFINE__(Dtmethod_t*,Dtbag,&_Dtbag);

#ifdef NoF
NoF(dthashchain)
#endif
