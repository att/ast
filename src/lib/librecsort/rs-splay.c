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
/*	Splay sort
**
**	Written by Kiem-Phong Vo (08/24/96).
*/

#include	"rshdr.h"

typedef struct rssplay_s
{	Rsobj_t*	root;
} Rssplay_t;

#if __STD_C
static int splayinsert(Rs_t* rs, reg Rsobj_t* obj)
#else
static int splayinsert(rs, obj)
Rs_t*		rs;
reg Rsobj_t*	obj;
#endif
{
	reg int		cmp;
	reg Rsobj_t	*r, *root, *t, *l;
	Rsobj_t		link;
	reg Rssplay_t*	splay = (Rssplay_t*)rs->methdata;

	obj->equal = NIL(Rsobj_t*);
	OBJHEAD(obj);

	if(!(root = splay->root))
	{	obj->left = obj->right = NIL(Rsobj_t*);
		splay->root = obj;
		return 0;
	}

	OBJCMP(obj,root,cmp);
	if(cmp == 0)
	{	EQUAL(root,obj,t);
		return 0;
	}
	else if(cmp > 0)
	{	if(!root->right)
		{	obj->left = root;
			obj->right = NIL(Rsobj_t*);
			splay->root = obj;
			return 0;
		}
	}
	else
	{	if(!root->left)
		{	obj->right = root;
			obj->left = NIL(Rsobj_t*);
			splay->root = obj;
			return 0;
		}
	}

	for(l = r = &link;; )
	{	if(cmp < 0)
		{	if((t = root->left))
			{	OBJCMP(obj,t,cmp);
				if(cmp < 0)
				{	RROTATE(root,t);
					RLINK(r,root);
					if(!(root = root->left))
						goto no_root;
				}
				else if(cmp == 0)
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
		else /*if(cmp > 0)*/
		{	if((t = root->right))
			{	OBJCMP(obj,t,cmp);
				if(cmp > 0)
				{	LROTATE(root,t);
					LLINK(l,root);
					if(!(root = root->right))
						goto no_root;
				}
				else if(cmp == 0)
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
		OBJCMP(obj,root,cmp);
		if(cmp == 0)
			goto has_root;
	}

 has_root:
	EQUAL(root,obj,t);

	l->right = root->left;
	r->left = root->right;

	root->left = link.right;
	root->right = link.left;
	splay->root = root;
	return 0;

 no_root:
	l->right = NIL(Rsobj_t*);
	r->left = NIL(Rsobj_t*);

	obj->left  = link.right;
	obj->right = link.left;
	splay->root = obj;
	return 0;
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
static Rsobj_t* splaylist(Rs_t* rs)
#else
static Rsobj_t* splaylist(rs)
Rs_t*	rs;
#endif
{
	reg Rsobj_t*	list;
	reg Rssplay_t*	splay = (Rssplay_t*)rs->methdata;

	if (!splay->root)
		return NIL(Rsobj_t*);
	list = flatten(splay->root);
	splay->root = NIL(Rsobj_t*);

	return list;
}

/* public method */
static Rsmethod_t _Rssplay =
{	splayinsert,
	splaylist,
	sizeof(Rssplay_t),
	RS_MTSPLAY,
	"splay",
	"Splay tree sort."
};

__DEFINE__(Rsmethod_t*, Rssplay, &_Rssplay);

#ifdef NoF
NoF(rssplay)
#endif
