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
/*	Radix sort.
**	Strategy:
**	1. All records are kept on a linked list.
**	2. In each phase, portions of the linked list are sorted using
**	   bucketing based on the byte at the position for that phase.
**
**	Written by Kiem-Phong Vo (07/08/96).
*/

#include	"rshdr.h"

typedef struct rsradix_s
{	Rsobj_t*	list;
} Rsradix_t;

#if __STD_C
static int radixinsert(Rs_t* rs, reg Rsobj_t* obj)
#else
static int radixinsert(rs, obj)
Rs_t*		rs;
reg Rsobj_t*	obj;
#endif
{
	reg Rsobj_t*	r;
	reg Rsradix_t*	radix = (Rsradix_t*)rs->methdata;

	obj->equal = NIL(Rsobj_t*);
	if((r = radix->list) )
		r->left->right = obj;
	else	radix->list = (r = obj);
	r->left = obj;
	return 0;
}

#if __STD_C
static Rsobj_t* radixlist(Rs_t* rs)
#else
static Rsobj_t* radixlist(rs)
Rs_t*		rs;
#endif
{
	reg Rsobj_t	*work, *r;
	reg ssize_t	ph;
	reg Rsobj_t	**bin, *t, *empty, *list, *endl, *next, **lo, **maxpart;
	reg ssize_t	n, maxph;
	Rsobj_t		*part[UCHAR_MAX+1];
	reg Rsradix_t*	radix = (Rsradix_t*)rs->methdata;

	if (!radix->list)
		return NIL(Rsobj_t*);
	for(lo = part, maxpart = part + UCHAR_MAX+1; lo < maxpart; ++lo)
		*lo = NIL(Rsobj_t*);

	work = radix->list; radix->list = NIL(Rsobj_t*);
	work->left->right = NIL(Rsobj_t*);
	list = NIL(Rsobj_t*);

	if(rs->type&RS_KSAMELEN)
	{	maxph = work->keylen-1;
		for(work->order = 0; work; )
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

				endl = list ? (endl->right = *lo) : (list = *lo);
				*lo = NIL(Rsobj_t*);
				for(bin = lo+1, n -= 1; n > 0; ++bin)
				{	if(!(r = *bin) )
						continue;
					n -= 1;
					endl = (endl->right = r);
					*bin = NIL(Rsobj_t*);
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
			{	endl = list ? (endl->right = work) : (list = work);
				for(work = work->right; work; work = work->right)
				{	if(work->left != work)
						break;
					endl = endl->right = work;
				}
			}
		}
	}
	else
	{	for(work->order = 0; work; )
		{	next = work->left->right; work->left->right = NIL(Rsobj_t*);
			empty = NIL(Rsobj_t*);
			lo = maxpart; n = 0;
			ph = (ssize_t)work->order;
			for(; work; work = work->right)
			{	if(ph >= work->keylen)
				{	if(!empty)
						empty = work;
					else	EQUAL(empty,work,t);
				}
				else
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
			}

			if(empty)
			{	if(list)
					endl->right = empty;
				else	list = empty;
				endl = empty;
			}

			if(n <= 0)
				work = next;
			else
			{	ph += 1;
				work = *lo; *lo = NIL(Rsobj_t*);
				t = work->left;
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
			{	endl = list ? (endl->right = work) : (list = work);
				for(work = work->right; work; work = work->right)
				{	if(work->left != work)
						break;
					endl = endl->right = work;
				}
			}
		}
	}

	if(list)
	{	list->left = endl;
		endl->right = NIL(Rsobj_t*);
	}

	return list;
}

/* public method */
static Rsmethod_t _Rsradix =
{	radixinsert,
	radixlist,
	sizeof(Rsradix_t),
	RS_MTRADIX,
	"radix",
	"Radix sort."
};

__DEFINE__(Rsmethod_t*, Rsradix, &_Rsradix);

#ifdef NoF
NoF(rsradix)
#endif
