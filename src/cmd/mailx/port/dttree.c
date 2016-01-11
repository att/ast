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
**	Look up an element in a tree.
**	dt:	dictionary being searched
**	obj:	the object to look for.
**	type:	search type.
**
**      Written by Kiem-Phong Vo (07/15/95)
*/

#if __STD_C
static Void_t* dttree(Dt_t* dt, reg Void_t* obj, int type)
#else
static Void_t* dttree(dt,obj,type)
Dt_t*		dt;
reg Void_t* 	obj;
int		type;
#endif
{
	reg char	*key, *k;
	reg int		cmp;
	reg Dtdisc_t*	disc = dt->disc;
	reg Dtlink_t	*root, *l, *r, *t, *me = NIL(Dtlink_t*);
	Dtlink_t	link;

	UNFLATTEN(dt);

	root = dt->data->here;
	if(!obj)
	{	if(!root || !(type&(DT_CLEAR|DT_FIRST|DT_LAST)) )
			return NIL(Void_t*);

		if(type&DT_CLEAR) /* delete all objects */
		{	if(disc->freef || disc->link < 0)
			{	do
				{	while((t = root->left) )
						RROTATE(root,t);
					t = root->right;
					if(disc->freef)
						(*disc->freef)(dt,OBJ(root,disc),disc);
					if(disc->link < 0)
						(*dt->memoryf)(dt,(Void_t*)root,0,disc);
				} while((root = t) );
			}

			dt->data->size = 0;
			dt->data->here = NIL(Dtlink_t*);
			return NIL(Void_t*);
		}
		else /* computing largest/smallest element */
		{	if(type&DT_LAST)
			{	while((t = root->right) )
					LROTATE(root,t);
			}
			else /* type == DT_FIRST */
			{	while((t = root->left) )
					RROTATE(root,t);
			}

			dt->data->here = root;
			return OBJ(root,disc);
		}
	}

	/* object and key */
	if(type&DT_MATCH)
		key = (char*)obj;
	else
	{	if(type&DT_RENEW)
		{	me = (Dtlink_t*)obj;
			obj = OBJ(me,disc);
		}
		key = KEY(obj,disc);
	}

	/* note that link.right is LEFT tree and link.left is RIGHT tree */
	l = r = &link;

	if(root && obj != OBJ(root,disc))
	{	do /* top-down splaying */
		{	k = (char*)OBJ(root,disc); k = KEY((Void_t*)k,disc);
			if((cmp = CMP(dt,key,k,disc)) == 0)
				break;
			else if(cmp < 0)	/* left turn */
			{	if((t = root->left) )
				{	k = (char*)OBJ(t,disc); k = KEY((Void_t*)k,disc);
					if((cmp = CMP(dt,key,k,disc)) <= 0)
					{	RROTATE(root,t);
						if(cmp == 0)
							break;
						t = root->left;
					}
					else	/* left,right */
					{	LLINK(l,t);
						t = t->right;
					}
				}
				RLINK(r,root);
			}
			else			/* right turn */
			{	if((t = root->right) )
				{	k = (char*)OBJ(t,disc); k = KEY((Void_t*)k,disc);
					if((cmp = CMP(dt,key,k,disc)) >= 0)
					{	LROTATE(root,t);
						if(cmp == 0)
							break;
						t = root->right;
					}
					else		/* right, left */
					{	RLINK(r,t);
						t = t->left;
					}
				}
				LLINK(l,root);
			}
		} while((root = t) );
	}

	if(root)
	{	/* found it, now isolate it */
		l->right = root->left;
		r->left = root->right;

		if(type&(DT_SEARCH|DT_MATCH))
			;
		else if(type&DT_DELETE)
		{	obj = OBJ(root,disc);
			if(disc->freef)
				(*disc->freef)(dt,obj,disc);
			if(disc->link < 0)
				(*dt->memoryf)(dt,(Void_t*)root,0,disc);
			if((dt->data->size -= 1) < 0)
				dt->data->size = -1;
			root = NIL(Dtlink_t*);
		}
		else if(type&DT_NEXT)	/* looking for immediate successor */
		{	root->left = link.right;	/* put root to LEFT tree */
			root->right = NIL(Dtlink_t*);
			link.right = root;
			goto dt_next;
		}
		else if(type&DT_PREV) /* looking for immediate predecessor */
		{	root->right = link.left;	/* put root to RIGHT tree */
			root->left = NIL(Dtlink_t*);
			link.left = root;
			goto dt_prev;
		}
		else if(type&DT_RENEW)
		{	/* duplicated elt */
			if(disc->freef)
				(*disc->freef)(dt,obj,disc);
			if(disc->link < 0)
				(*dt->memoryf)(dt,(Void_t*)me,0,disc);
			root = NIL(Dtlink_t*);
		}
	}
	else
	{	/* not found, finish up LEFT and RIGHT trees */
		r->left = NIL(Dtlink_t*);
		l->right = NIL(Dtlink_t*);

		if(type&(DT_SEARCH|DT_DELETE|DT_MATCH))
			obj = NIL(Void_t*);
		else if(type&DT_RENEW)
		{	root = me;
			dt->data->size += 1;
		}
		else if(type&DT_INSERT)
		{	if(disc->makef)
				obj = (*disc->makef)(dt,obj,disc);
			if(obj)
			{	if(disc->link < 0)
				{	root = (Dtlink_t*)(*dt->memoryf)
						(dt,NIL(Void_t*),sizeof(Dthold_t),disc);
					if(root)
						((Dthold_t*)root)->obj = obj;
					else if(disc->makef && disc->freef)
						(*disc->freef)(dt,obj,disc);
				}
				else	root = ELT(obj,disc);

				if((dt->data->size += 1) <= 0)
					dt->data->size = -1;
			}
		}
		else if(type&DT_NEXT)
		{ dt_next : /* immediate successor is smallest in RIGHT tree */
			if((root = link.left) )	
			{	while((t = root->left) )
					RROTATE(root,t);
				link.left = root->right;
			}
		}
		else if(type&DT_PREV)
		{ dt_prev : /* immediate predecessor is largest in LEFT tree */
			if((root = link.right) )
			{	while((t = root->right) )
					LROTATE(root,t);
				link.right = root->left;
			}
		}
	}

	if(root) /* got it, reconstruct the tree with it as root */
	{	root->left = link.right;
		root->right = link.left;
		dt->data->here = root;
		return OBJ(root,disc);
	}
	else	/* not found, reconstruct tree by hooking LEFT tree to RIGHT tree */
	{	while((t = r->left) )
			r = t;
		r->left = link.right;
		dt->data->here = link.left;
		return (type&DT_DELETE) ? obj : NIL(Void_t*);
	}
}

/* make this method available */
Dtmethod_t	_Dttree =  { dttree, DT_TREE };
