/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
/*	Radix + splay tree
**	Strategy:
**	1. Records are partitioned by first bytes.
**	2. Short records are sorted by a radix sort.
**	3. Long records are sorted in splay trees.
**	4. A final merge phase put things back together.
**
**	Written by Kiem-Phong Vo (07/08/96).
*/

#include	"rshdr.h"

#define SLOT		SIZEOF_LONG
#define PREFIX		(SLOT + 1)

typedef struct rsrasp_s
{	Rsobj_t*	empty;
	Rsobj_t*	slot[SLOT][UCHAR_MAX+1];
	Rsobj_t*	tree[UCHAR_MAX+1];
} Rsrasp_t;

#define SPLAYCMP(one,two,o,t,endo,mp,cr) \
	{ if(one->order != two->order) \
	    cr = one->order < two->order ? -1 : 1; \
	  else \
	  { if((mp = (cr = one->keylen) - two->keylen) > 0)	cr -= mp; \
	    o = one->key+PREFIX; t = two->key+PREFIX; \
	    for(endo = one->key+cr;; ) \
	    { if(o >= endo)			{ cr = mp; break; } \
	      else if((cr = (int)*o++ - (int)*t++))	break; \
	    } \
	  } \
	}

#if __STD_C
static int raspinsert(Rs_t* rs, reg Rsobj_t* obj)
#else
static int raspinsert(rs, obj)
Rs_t*		rs;
reg Rsobj_t*	obj;
#endif
{
	reg ssize_t	cr, mp;
	reg uchar	*o, *k;
	reg Rsobj_t	*r, *root, *t, *l;
	reg uchar*	endo;
	reg int		index;
	Rsobj_t		link;
	reg Rsrasp_t*	rasp = (Rsrasp_t*)rs->methdata;

	obj->equal = NIL(Rsobj_t*);

	if((cr = obj->keylen) == 0)
	{	if((r = rasp->empty) )
			{ EQUAL(r,obj,t); }
		else	rasp->empty = obj;
		return 0;
	}

	index = *(k = obj->key);

	if(cr == 1)
	{	if((r = rasp->slot[0][index]) )
			{ EQUAL(r,obj,t); }
		else	rasp->slot[0][index] = obj;
		return 0;
	}
	else if((cr -= 1) < SLOT)
	{	if((r = rasp->slot[cr][index]) )
			r->left->right = obj;
		else	r = rasp->slot[cr][index] = obj;
		r->left = obj;
		return 0;
	}

#if SIZEOF_LONG == 8
	obj->order = (((ulong)k[1]) << (CHAR_BIT*7)) |
		     (((ulong)k[2]) << (CHAR_BIT*6)) |
		     (((ulong)k[3]) << (CHAR_BIT*5)) |
		     (((ulong)k[4]) << (CHAR_BIT*4)) |
		     (((ulong)k[5]) << (CHAR_BIT*3)) |
		     (((ulong)k[6]) << (CHAR_BIT*2)) |
	 	     (((ulong)k[7]) << (CHAR_BIT*1)) |
		     (((ulong)k[8]) << (CHAR_BIT*0)) ;
#else /* sizeof(long) == 4 */
	obj->order = (k[1] << (CHAR_BIT*3)) | (k[2] << (CHAR_BIT*2)) |
		     (k[3] << (CHAR_BIT*1)) | (k[4] << (CHAR_BIT*0)) ;
#endif

	if(!(root = rasp->tree[index]))
	{	rasp->tree[index] = obj;
		obj->left = obj->right = NIL(Rsobj_t*);
		return 0;
	}

	SPLAYCMP(obj,root,o,k,endo,mp,cr);
	if(cr == 0)
	{	EQUAL(root,obj,t);
		return 0;
	}

	for(l = r = &link;; )
	{	if(cr < 0)
		{	if((t = root->left))
			{	SPLAYCMP(obj,t,o,k,endo,mp,cr);
				if(cr < 0)
				{	RROTATE(root,t);
					RLINK(r,root);
					if(!(root = root->left))
						goto no_root;
				}
				else if(cr == 0)
				{	RROTATE(root,t);
					goto has_root;
				}
				else
				{	LLINK(l,t);
					RLINK(r,root);
					if(!(root = t->right))
						goto no_root;
				}
			}
			else
			{	RLINK(r,root);
				goto no_root;
			}
		}
		else /*if(cr > 0)*/
		{	if((t = root->right))
			{	SPLAYCMP(obj,t,o,k,endo,mp,cr);
				if(cr > 0)
				{	LROTATE(root,t);
					LLINK(l,root);
					if(!(root = root->right))
						goto no_root;
				}
				else if(cr == 0)
				{	LROTATE(root,t);
					goto has_root;
				}
				else
				{	RLINK(r,t);
					LLINK(l,root);
					if(!(root = t->left))
						goto no_root;
				}
			}
			else
			{	LLINK(l,root);
				goto no_root;
			}
		}
		SPLAYCMP(obj,root,o,k,endo,mp,cr);
		if(cr == 0)
			goto has_root;
	}

 has_root:
	EQUAL(root,obj,t);

	l->right = root->left;
	r->left = root->right;

	root->left = link.right;
	root->right = link.left;
	rasp->tree[index] = root;
	return 0;

 no_root:
	l->right = NIL(Rsobj_t*);
	r->left = NIL(Rsobj_t*);

	obj->left  = link.right;
	obj->right = link.left;
	rasp->tree[index] = obj;
	return 0;
}

#if __STD_C
static Rsobj_t* radix(Rsobj_t* list)
#else
static Rsobj_t* radix(list)
Rsobj_t*	list;
#endif
{
	reg Rsobj_t	*work, *r;
	reg ssize_t	ph;
	reg Rsobj_t	**bin, *t, *endl, *next, **lo, **maxpart;
	reg ssize_t	n, maxph;
	Rsobj_t		*part[UCHAR_MAX+1];

	for(lo = part, maxpart = part + UCHAR_MAX+1; lo < maxpart; ++lo)
		*lo = NIL(Rsobj_t*);

	work = list; list = NIL(Rsobj_t*);
	work->left->right = NIL(Rsobj_t*);
	maxph = work->keylen-1;

	for(work->order = 1; work; )
	{	next = work->left->right; work->left->right = NIL(Rsobj_t*);

		lo = maxpart; n = 0;
		if((ph = (ssize_t)work->order) == maxph)
		{	for(; work; work = work->right)
			{	bin = part + work->key[ph];
				if(!(r = *bin) )
				{	*bin = work;
					if(lo > bin)
						lo = bin;
					n += 1;
				}
				else	EQUAL(r,work,t);
			}

			if(list)
				endl = (endl->right = *lo);
			else	endl = (list = *lo);
			*lo = NIL(Rsobj_t*);
			for(bin = lo+1, n -= 1; n > 0; ++bin)
			{	if((r = *bin) )
				{	n -= 1;
					endl = (endl->right = r);
					*bin = NIL(Rsobj_t*);
				}
			}

			work = next;
		}
		else
		{	for(; work; work = work->right)
			{	bin = part + work->key[ph];
				if((r = *bin) )
					r->left->right = work;
				else
				{	r = *bin = work;
					if(lo > bin)
						lo = bin;
					n += 1;
				}
				r->left = work;
			}

			ph += 1;
			work = *lo; t = work->left; *lo = NIL(Rsobj_t*);
			work->order = ph;
			for(bin = lo+1, n -= 1; n > 0; ++bin)
			{	if((r = *bin) )
				{	n -= 1;

					r->order = ph;
					t->right = r;
					t = r->left;

					*bin = NIL(Rsobj_t*);
				}
			}

			t->right = next;
		}

		if(work && work->left == work)
		{	if(list)
				endl = (endl->right = work);
			else	endl = (list = work);
			for(work = work->right; work; work = work->right)
			{	if(work->left != work)
					break;
				endl = (endl->right = work);
			}
		}
	}

	list->left = endl;
	return list;
}

#define CHARCMP(k1,k2,v,i)	(v = (int)k1[i] - (int)k2[i])
#define STRNCMP(k1,k2,v,i,n) \
	{	if(CHARCMP(k1,k2,v,1) == 0) \
		{	for(i = 2; i <= n; ++i) \
				if(CHARCMP(k1,k2,v,i) != 0) \
					break; \
		} \
	}

#if __STD_C
static Rsobj_t* listmerge(reg Rsobj_t* one, reg Rsobj_t* two, reg int n)
#else
static Rsobj_t* listmerge(one, two, n)
reg Rsobj_t*	one;
reg Rsobj_t*	two;
reg int		n;	/* number of bytes to compare	*/
#endif
{
	reg int		v, i;
	reg uchar	*k1, *k2;
	reg Rsobj_t	*list, *endl, *endone, *endtwo;

	endone = one->left; one->left->right = NIL(Rsobj_t*);
	endtwo = two->left; two->left->right = NIL(Rsobj_t*);

	k1 = one->key; k2 = two->key;
	STRNCMP(k1,k2,v,i,n);
	if(v <= 0)
	{	list = endl = one;
		if((one = one->right) )
			k1 = one->key;
	}
	else
	{	list = endl = two;
		if((two = two->right) )
			k2 = two->key;
	}

	if(one && two)
	{	for(;;)
		{	STRNCMP(k1,k2,v,i,n);
			if(v <= 0)
			{	endl = (endl->right = one);

				if((one = one->right) )
					k1 = one->key;
				else	break;
			}
			else
			{	endl = (endl->right = two);

				if((two = two->right) )
					k2 = two->key;
				else	break;
			}
		}
	}

	if(one)
	{	endl->right = one;
		endl = endone;
	}
	else if(two)
	{	endl->right = two;
		endl = endtwo;
	}

	list->left = endl;
	return list;
}

#if __STD_C
static Rsobj_t* flatten(reg Rsobj_t* r)
#else
static Rsobj_t* flatten(r)
reg Rsobj_t*	r;
#endif
{	reg Rsobj_t	*t, *p, *list;

	/* find smallest element and make it head of list */
	while((t = r->left) )
		RROTATE(r,t);

	/* flatten tree */
	for(list = p = r, r = r->right;; p = r, r = r->right)
	{	if(!r)
		{	list->left = p;
			return list;
		}
		else if((t = r->left) )
		{	do	RROTATE(r,t);
			while((t = r->left) );

			p->right = r;
		}
	}
}

#if __STD_C
static Rsobj_t* rasplist(Rs_t* rs)
#else
static Rsobj_t* rasplist(rs)
Rs_t*	rs;
#endif
{
	reg Rsobj_t	*r, *t, *list, *endl;
	reg int		n, e;
	reg Rsrasp_t*	rasp = (Rsrasp_t*)rs->methdata;

	list = endl = rasp->empty; rasp->empty = NIL(Rsobj_t*);

	for(n = 0, t = NIL(Rsobj_t*); n <= UCHAR_MAX; ++n)
	{
		if((r = rasp->tree[n]) )
		{	t = flatten(r);
			rasp->tree[n] = NIL(Rsobj_t*);
		}

		for(e = SLOT-1; e > 0; --e)
		{	if(!(r = rasp->slot[e][n]) )
				continue;

			r = radix(r);
			t = t ? listmerge(r,t,e) : r;

			rasp->slot[e][n] = NIL(Rsobj_t*);
		}

		if((r = rasp->slot[0][n]) )
		{	if((r->right = t) )
				r->left = t->left;
			else	r->left = r;
			t = r;

			rasp->slot[0][n] = NIL(Rsobj_t*);
		}

		if(t)
		{	if(list)
				endl->right = t;
			else	list = t;
			endl = t->left;

			t = NIL(Rsobj_t*);
		}
	}

	if(list)
	{	list->left = endl;
		endl->right = NIL(Rsobj_t*);
	}

	return list;
}

/* public method */
static Rsmethod_t _Rsrasp =
{	raspinsert,
	rasplist,
	sizeof(Rsrasp_t),
	RS_MTRASP,
	"rasp",
	"Initial radix split into a forest of splay trees."
};

__DEFINE__(Rsmethod_t*, Rsrasp, &_Rsrasp);

#ifdef NoF
NoF(rsrasp)
#endif
