/***********************************************************************
*                                                                      *
*               This software is part of the BSD package               *
*Copyright (c) 1978-1996 The Regents of the University of California an*
*                                                                      *
* Redistribution and use in source and binary forms, with or           *
* without modification, are permitted provided that the following      *
* conditions are met:                                                  *
*                                                                      *
*    1. Redistributions of source code must retain the above           *
*       copyright notice, this list of conditions and the              *
*       following disclaimer.                                          *
*                                                                      *
*    2. Redistributions in binary form must reproduce the above        *
*       copyright notice, this list of conditions and the              *
*       following disclaimer in the documentation and/or other         *
*       materials provided with the distribution.                      *
*                                                                      *
*    3. Neither the name of The Regents of the University of California*
*       names of its contributors may be used to endorse or            *
*       promote products derived from this software without            *
*       specific prior written permission.                             *
*                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND               *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,          *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF             *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE             *
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS    *
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,             *
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED      *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,        *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON    *
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,      *
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY       *
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE              *
* POSSIBILITY OF SUCH DAMAGE.                                          *
*                                                                      *
* Redistribution and use in source and binary forms, with or without   *
* modification, are permitted provided that the following conditions   *
* are met:                                                             *
* 1. Redistributions of source code must retain the above copyright    *
*    notice, this list of conditions and the following disclaimer.     *
* 2. Redistributions in binary form must reproduce the above copyright *
*    notice, this list of conditions and the following disclaimer in   *
*    the documentation and/or other materials provided with the        *
*    distribution.                                                     *
* 3. Neither the name of the University nor the names of its           *
*    contributors may be used to endorse or promote products derived   *
*    from this software without specific prior written permission.     *
*                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS"    *
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED    *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A      *
* PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS    *
* OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT     *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF     *
* USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND  *
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   *
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT   *
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF   *
* SUCH DAMAGE.                                                         *
*                                                                      *
*                          Kurt Shoens (UCB)                           *
*                                 gsf                                  *
*                                                                      *
***********************************************************************/
#include	"dthdr.h"

/*
**	Search for an element in a hash table.
**	dt:	dictionary
**	obj:	what to look for
**	type:	type of search
**
**      Written by Kiem-Phong Vo (07/15/95)
*/

/* resize the hash table */
#if __STD_C
static void dthtab(Dt_t* dt)
#else
static void dthtab(dt)
Dt_t*	dt;
#endif
{
	reg Dtlink_t	*t, *next, **chain, **slot, **oslot, **eslot;
	reg int		s, nslot;

	/* compute new table size */
	s = dt->data->size;
	if((nslot = dt->data->ntab) == 0)
		nslot = HSLOT;
	while(s > HLOAD(nslot))
		nslot = HRESIZE(nslot);

	/* allocate new table */
	slot = (Dtlink_t**)(*dt->memoryf)
				(dt,NIL(Void_t*),nslot*sizeof(Dtlink_t*),dt->disc);
	if(!slot)
		return;
	for(eslot = slot+nslot-1; eslot >= slot; --eslot)
		*eslot = NIL(Dtlink_t*);

	/* rehash elements */
	eslot = (oslot = dt->data->htab) + dt->data->ntab;
	for(; oslot < eslot; ++oslot)
	{	for(t = *oslot; t; )
		{	chain = slot + HINDEX(nslot,t->hash);
			next = t->right;
			t->right = *chain;
			*chain = t;
			t = next;
		}
	}

	if(dt->data->ntab > 0)
		(*dt->memoryf)(dt,(Void_t*)dt->data->htab,0,dt->disc);
	dt->data->htab = slot;
	dt->data->ntab = nslot;
}

#if __STD_C
static Void_t* dthash(Dt_t* dt, reg Void_t* obj, int type)
#else
static Void_t* dthash(dt,obj,type)
Dt_t*		dt;
reg Void_t*	obj;
int		type;
#endif
{
	reg char	*key, *k;
	reg ulong	hsh;
	reg Dtdisc_t*	disc = dt->disc;
	reg Dtlink_t	*t, *prev, *r = NIL(Dtlink_t*);
	reg Dtlink_t	**slot = NIL(Dtlink_t**), **eslot;

	UNFLATTEN(dt);

	if(!obj)
	{	if(dt->data->size <= 0 || !(type&(DT_CLEAR|DT_FIRST|DT_LAST)) )
			return NIL(Void_t*);

		eslot = (slot = dt->data->htab) + dt->data->ntab;
		if(type&DT_CLEAR)
		{	/* clean out all objects */
			for(; slot < eslot; ++slot)
			{	t = *slot;
				*slot = NIL(Dtlink_t*);
				if(!disc->freef && disc->link >= 0)
					continue;
				while(t)
				{	r = t->right;
					if(disc->freef)
						(*disc->freef)(dt,OBJ(t,disc),disc);
					if(disc->link < 0)
						(*dt->memoryf)(dt,(Void_t*)t,0,disc);
					t = r;
				}
			}
			dt->data->here = NIL(Dtlink_t*);
			dt->data->size = 0;
			return NIL(Void_t*);
		}
		else	/* computing the first/last object */
		{	t = NIL(Dtlink_t*);
			while(slot < eslot && !t )
				t = (type&DT_LAST) ? *--eslot : *slot++;
			if(t && (type&DT_LAST))
				for(; t->right; t = t->right)
					;

			dt->data->here = t;
			return t ? OBJ(t,disc) : NIL(Void_t*);
		}
	}

	/* object, key, hash value */
	if(type&DT_MATCH)
		key = (char*)obj;
	else
	{	if(type&DT_RENEW)
		{	r = (Dtlink_t*)obj;
			obj = OBJ(r,disc);
		}
		key = KEY(obj,disc);
	}

	/* find the object */
	prev = NIL(Dtlink_t*);
	if(!(t = dt->data->here) || OBJ(t,disc) != obj)
	{	hsh = HASH(dt,key,disc);
		if(dt->data->ntab == 0)
			t = NIL(Dtlink_t*);
		else
		{	t = *(slot = dt->data->htab + HINDEX(dt->data->ntab,hsh));
			for(; t; prev = t, t = t->right)
			{	if(hsh != t->hash)
					continue;
				k = (char*)OBJ(t,disc); k = KEY((Void_t*)k,disc);
				if(CMP(dt,key,k,disc) == 0)
					break;
			}
		}
	}

	if(!t)	/* didn't find a current version */
	{	if(!(type&(DT_RENEW|DT_INSERT)) )
			return NIL(Void_t*);

		/* about to insert, resize table if necessary */
		if((dt->data->size += 1) > HLOAD(dt->data->ntab))
			dthtab(dt);
		if(dt->data->ntab == 0)
		{	dt->data->size -= 1;
			return NIL(Void_t*);
		}

		if(type&DT_INSERT)
		{	if(disc->makef && !(obj = (*disc->makef)(dt,obj,disc)) )
				return NIL(Void_t*);
			if(disc->link < 0)
			{	t = (Dtlink_t*)(*dt->memoryf)
					(dt,NIL(Void_t*),sizeof(Dthold_t),disc);
				if(!t)
				{	if(disc->makef && disc->freef)
						(*disc->freef)(dt,obj,disc);
					return NIL(Void_t*);
				}
				((Dthold_t*)t)->obj = obj;
			}
			else	t = ELT(obj,disc);
			t->hash = hsh;
		}
		else	t = r;

		/* insert object */
		slot = dt->data->htab + HINDEX(dt->data->ntab,hsh);
		t->right = *slot; *slot = t;
		dt->data->here = t;
		return obj;
	}

	if(type&(DT_MATCH|DT_SEARCH|DT_INSERT) )
	{	if(prev) /* move to front heuristic */
		{	prev->right = t->right;
			t->right = *slot;
			*slot = t;
		}
		dt->data->here = t;
		return OBJ(t,disc);
	}

	if(type&DT_RENEW ) /* already seen, eliminate duplicate */
	{	if(disc->freef)
			(*disc->freef)(dt,obj,disc);
		if(disc->link < 0)
			(*dt->memoryf)(dt,(Void_t*)r,0,disc);
		return NIL(Void_t*);
	}

	slot = dt->data->htab + HINDEX(dt->data->ntab,t->hash);
	if(type&DT_NEXT)
	{	if(t->right )
			t = t->right;
		else
		{	t = NIL(Dtlink_t*);
			eslot = dt->data->htab + dt->data->ntab;
			for(slot += 1; slot < eslot; ++slot)
				if((t = *slot) )
					break;
		}
		dt->data->here = t;
		return t ? OBJ(t,disc) : NIL(Void_t*);
	}
	else if(type&DT_DELETE)
	{	if(!prev && t != *slot)
			for(prev = *slot; prev->right != t; prev = prev->right)
				;
		r = t->right;
		if(prev)
			prev->right = r;
		else	*slot = r;
		obj = OBJ(t,disc);
		if(disc->freef)
			(*disc->freef)(dt,obj,disc);
		if(disc->makef)
			(*dt->memoryf)(dt,(Void_t*)t,0,disc);
		dt->data->size -= 1;
		dt->data->here = r;
		return obj;
	}
	else /* if(type&DT_PREV) */
	{	if(!prev && t != *slot)
			for(prev = *slot; prev->right != t; prev = prev->right)
				;
		if(prev)
			t = prev;
		else
		{	t = NIL(Dtlink_t*);
			eslot = dt->data->htab;
			for(slot -= 1; slot >= eslot; --slot)
				if((t = *slot) )
					break;
			if(t)
				for(; t->right; t = t->right)
					;
		}

		dt->data->here = t;
		return t ? OBJ(t,disc) : NIL(Void_t*);
	}
}

/* make this method available */
Dtmethod_t	_Dthash = { dthash, DT_HASH };
