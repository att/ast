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
#include	"rshdr.h"

/*	Get a list of objects from a sorting context.
**	Derived contexts will be unionized.
**
**	Written by Kiem-Phong Vo (07/19/96)
*/

typedef struct _union_s
{	Rsobj_t*		obj;
	int			pos;
	struct _union_s*	equi;
} Union_t;

#define ADDOBJ(list,tail,obj)	(tail = tail ? (tail->right = obj) : (list = obj) )

/* Lists are kept in reverse order to ease data movement.
** Ties are broken by list positions to preserve stability.
*/
#if __STD_C
static int uninsert(Union_t** list, int n, Union_t* un)
#else
static int uninsert(list, n, un)
Union_t**	list;
int		n;
Union_t*	un;
#endif
{
	reg Rsobj_t	*obj, *o;
	reg Union_t	**l, **r, **m, *p, *h;
	reg int		cmp;

	obj = un->obj;
	r = (l = list) + n;

	if(n > 4)
	{	while(l != r)
		{	m = l + (r-l)/2;
			o = (*m)->obj;
			OBJCMP(o,obj,cmp);
			if(cmp == 0)
				l = r = m;
			else if(cmp > 0)
				l = l == m ? r : m;
			else	r = m;
		}
	}
	else
	{	for(r -= 1, cmp = 1; r >= l; --r)
		{	o = (*r)->obj;
			OBJCMP(o,obj,cmp);
			if(cmp > 0)
				{ l = r+1; break; }
			else if(cmp == 0)
				{ l = r; break; }
		}
	}

	if(cmp == 0)
	{	for(p = NIL(Union_t*), h = *l;; )
			if(un->pos < h->pos || !(p=h, h=h->equi) )
				break;
		un->equi = h;
		if(p)	p->equi = un;
		else	*l = un;
	}
	else
	{	for(r = list+n; r > l; --r)
			*r = *(r-1);
		*l = un; un->equi = NIL(Union_t*);
		n += 1;
	}

	return n;
}

#if __STD_C
static Rsobj_t* unionize(Rsobj_t** objlist, int n)
#else
static Rsobj_t* unionize(objlist, n)
Rsobj_t**	objlist;
int		n;
#endif
{
	reg int		p, cmp;
	reg Union_t	*un, *u, *pu;
	reg Rsobj_t	*obj, *o, *e, *list, *tail;
	Union_t		**ulist, *uarray;
	reg int		n_list;

	if(!(ulist = (Union_t**)vmalloc(Vmheap,n*sizeof(Union_t*))) )
		return NIL(Rsobj_t*);
	if(!(uarray = (Union_t*)vmalloc(Vmheap,n*sizeof(Union_t))) )
	{	vmfree(Vmheap,ulist);
		return NIL(Rsobj_t*);
	}

	for(p = 0, n_list = 0; p < n; ++p)
	{	/* set up header data for quick comparisons */
		for(obj = objlist[p]; obj; obj = obj->right)
			OBJHEAD(obj);

		uarray[p].obj = objlist[p];
		uarray[p].pos = p;
		n_list = uninsert(ulist,n_list,uarray+p);
	}

	list = tail = NIL(Rsobj_t*);
	while(n_list > 0)
	{	un = ulist[n_list -= 1];
		if(n_list == 0 && !un->equi)	/* last unmerged list */
		{	obj = un->obj;
			ADDOBJ(list,tail,obj);
		}
		else if(un->equi)	/* a set of equivalence classes */
		{	obj = un->obj; un->obj = obj->right;
			for(;;)
			{	u = un->equi;
				if(un->obj)
					n_list = uninsert(ulist,n_list,un);
				if(!(un = u) )
					break;

				/* union with equal list of obj */
				o = u->obj; u->obj = o->right;
				if((e = obj->equal) )
					e->left->right = o;
				else	obj->equal = e = o;

				if((o->right = o->equal) )
					e->left = o->equal->left;
				else	e->left = o;
			}
			obj->equal->left->right = NIL(Rsobj_t*);

			ADDOBJ(list,tail,obj);

			if(n_list == 0)
				tail->right = NIL(Rsobj_t*);
		}
		else /* at least 2 distinct lists are left */
		{	o = ulist[p = n_list-1]->obj;
			for(obj = un->obj;; )	/* keep peeling off least objects */
			{	un->obj = obj->right;
				ADDOBJ(list,tail,obj);

				if(!(obj = un->obj) )
					break;
				else
				{	OBJCMP(obj,o,cmp);
					if(cmp >= 0)
						break;
				}
			}
			if(obj)
			{	if(cmp > 0) /* new min element */
				{	ulist[n_list] = ulist[p];
					if(p == 0)
					{	n_list = 2;
						ulist[0] = un;
					}
					else if(uninsert(ulist,p,un) == p)
						ulist[p] = ulist[n_list];
					else	n_list += 1;
				}
				else /*if(cmp == 0) -- new equivalence class */
				{	for(pu = NIL(Union_t*), u = ulist[p];; )
						if(un->pos < u->pos ||
						   !(pu = u, u = u->equi) )
							break;
					un->equi = u;
					if(pu)	pu->equi = un;
					else	ulist[p] = un;
				}
			}
		}
	}

	vmfree(Vmheap,ulist);
	vmfree(Vmheap,uarray);

	return list;
}

#if __STD_C
Rsobj_t* rslist(Rs_t* rs)
#else
Rsobj_t* rslist(rs)
Rs_t*	rs;
#endif
{
	reg Rsobj_t	*list, *next, *p, *r, *t, *e;
	reg int		n, type;
	reg uchar*	k;

	if((type = rs->type)&RS_SORTED)
		return rs->sorted;

	if((list = (*rs->meth->listf)(rs)) && rs->n_list > 0)
	{	rs->list = (Rsobj_t**)vmresize(rs->vm,
				    rs->list, (rs->n_list+1)*sizeof(Rsobj_t*),
				    VM_RSCOPY|VM_RSMOVE);
		if(!rs->list)
			return NIL(Rsobj_t*);
		rs->list[rs->n_list] = list;
		rs->n_list += 1;
	}

	if(rs->n_list > 0)
	{	list = rs->n_list > 1 ? unionize(rs->list,rs->n_list) : rs->list[0];

		vmfree(rs->vm,rs->list);
		rs->list = NIL(Rsobj_t**);
		rs->n_list = 0;
	}

	if((type&RS_UNIQ) || !(type&RS_DATA) )
	{	for(r = list; r; r = r->right)
			if((e = r->equal) )
				e->left->right = NIL(Rsobj_t*);
	}
	else
	{	int	(*insertf)_ARG_((Rs_t*, Rsobj_t*)) = rs->meth->insertf;
		Rsobj_t*(*listf)_ARG_((Rs_t*)) = rs->meth->listf;

		for(p = NIL(Rsobj_t*), r = list; r; r = t)
		{	t = r->right;
			if(!(e = r->equal) )
			{	p = r;
				continue;
			}

			/* resort using whole data */
			r->right = e;
			e->left->right = NIL(Rsobj_t*);

			rs->type &= ~RS_KSAMELEN;
			if(type&RS_DSAMELEN)
				rs->type |= RS_KSAMELEN;

			for(; r; r = next)
			{	next = r->right;

				k = r->key;
				r->key = r->data;
				r->data = k;
				n = r->keylen;
				r->keylen = r->datalen;
				r->datalen = n;

				(*insertf)(rs,r);
			}

			r = (*listf)(rs);
			if(p)
				p->right = r;
			else	list = r;

			p = r->left;
			p->right = t;

			for(; r != t; r = r->right)
			{	k = r->key;
				r->key = r->data;
				r->data = k;
				n = r->keylen;
				r->keylen = r->datalen;
				r->datalen = n;

				if((e = r->equal) )
				{	e->left->right = NIL(Rsobj_t*);
					for(; e; e = e->right)
					{	k = e->key;
						e->key = e->data;
						e->data = k;
						n = e->keylen;
						e->keylen = e->datalen;
						e->datalen = n;
					}
				}
			}

			rs->type = type;
		}
	}

	if((type&RS_REVERSE) )
	{	for(p = NIL(Rsobj_t*), r = list; r; r = t)
		{	t = r->right;

			if((e = r->equal) )	/* invert equal list */
			{	reg Rsobj_t*	ende;
				for(list = r, ende = e->left; e != ende; e = next)
				{	next = e->right;
					e->right = list;
					list = e;
				}
				list->left = r; r->right = NIL(Rsobj_t*);
				r = ende; r->equal = list;
			}

			r->right = p;
			p = r;
		}
		list = p;
	}

	if((rs->sorted = list) )
		rs->type |= RS_SORTED;
	return list;
}
