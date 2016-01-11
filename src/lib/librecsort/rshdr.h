/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2013 AT&T Intellectual Property          *
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
#ifndef _RSHDR_H
#define _RSHDR_H	1

#if _PACKAGE_ast
#include	<ast.h>
#include	<recfmt.h>
#endif

#include	<vmalloc.h>

#include	"FEATURE/recsort"

/* because of sfio's sfreserve() semantics,
** the below parameter also limits the max size of a record that can be
** processed in rsmerge().
*/
#define RS_RESERVE	(256*1024)	/* for I/O reservation		*/

#define _RS_PRIVATE_ \
	unsigned long	events;		/* active events		*/ \
	Void_t*		methdata;	/* private method data		*/ \
	Vmalloc_t*	vm;		/* region to allocate temp data	*/ \
	ssize_t		c_max;		/* max datasize per chain	*/ \
	ssize_t		c_size;		/* current size			*/ \
	Rsobj_t*	sorted;		/* defined after a reclist call	*/ \
	Rsobj_t**	list;		/* list of processed contexts	*/ \
	int		n_list;		/* number of such contexts	*/ \
	Rsobj_t*	free;		/* free list of objects		*/ \
	Rskey_t*	key;		/* rsinit() key coder state	*/ \
	Sfio_t*		f;		/* current output stream	*/ \
	unsigned char	*rsrv, *endrsrv, *cur;	/* for fast writes	*/

#include		"recsort.h"

/* internal control bits */
#define RS_SORTED	010000		/* context has been sorted	*/
#define RS_LOCAL	020000		/* local call			*/

#if !_PACKAGE_ast
#if __STD_C
#include	<string.h>
#endif
#endif

#ifndef uchar
#define uchar		unsigned char
#endif
#ifndef ushort
#define ushort		unsigned short
#endif
#ifndef uint
#define uint		unsigned int
#endif
#ifndef ulong
#define ulong		unsigned long
#endif
#ifndef reg
#define reg		register
#endif
#ifndef NIL
#define NIL(type)	((type)0)
#endif

#ifndef UCHAR_MAX
#define UCHAR_MAX	((uchar)(~0) )
#endif
#ifndef UINT_MAX
#define UINT_MAX	((uint)(~0) )
#endif
#ifndef INT_MAX
#define INT_MAX		((int)(UINT_MAX >> 1) )
#endif

/* splay tree operations */
#define RLINK(r,x)	(r = r->left  = x)
#define LLINK(l,x)	(l = l->right = x)
#define RROTATE(r,t) 	(r->left = t->right, t->right = r, r = t)
#define LROTATE(r,t)	(r->right = t->left, t->left = r, r = t)

#define SETLOCAL(rs)	(rs->type |= RS_LOCAL)
#define GETLOCAL(rs,l)	((l = (rs->type&RS_LOCAL)), (rs->type &= ~RS_LOCAL), l)
#define RSWRITE(rs,f,t)	(SETLOCAL(rs), rswrite(rs,f,t))

/* do quick key comparisons using first 4 bytes */
#if SIZEOF_LONG == 8
#define OBJHEAD(obj)	\
	{ reg uchar*	k = obj->key; reg ulong	h = 0; \
	  switch(obj->keylen) \
	  { default :	h  = ((ulong)k[7]); \
	    case 7 :	h |= ((ulong)k[6]) << (1*CHAR_BIT); \
	    case 6 :	h |= ((ulong)k[5]) << (2*CHAR_BIT); \
	    case 5 :	h |= ((ulong)k[4]) << (3*CHAR_BIT); \
	    case 4 :	h |= ((ulong)k[3]) << (4*CHAR_BIT); \
	    case 3 :	h |= ((ulong)k[2]) << (5*CHAR_BIT); \
	    case 2 :	h |= ((ulong)k[1]) << (6*CHAR_BIT); \
	    case 1 :	h |= ((ulong)k[0]) << (7*CHAR_BIT); \
	    case 0 :	obj->order = h; \
	  } \
	}
#else /* SIZEOF_LONG == 4*/
#define OBJHEAD(obj)	\
	{ reg uchar*	k = obj->key; reg ulong	h = 0; \
	  switch(obj->keylen) \
	  { default :	h  = k[3]; \
	    case 3 :	h |= k[2] << (1*CHAR_BIT); \
	    case 2 :	h |= k[1] << (2*CHAR_BIT); \
	    case 1 :	h |= k[0] << (3*CHAR_BIT); \
	    case 0 :	obj->order = h; \
	  } \
	}
#endif

#define OBJCMP(one,two,cmp) \
	{ if((one)->order != (two)->order ) \
	    cmp = (one)->order < (two)->order ? -1 : 1; \
	  else \
	  { reg uchar *ok, *tk; reg ssize_t l, d; \
	    ok = (one)->key+SIZEOF_LONG; tk = (two)->key+SIZEOF_LONG; \
	    if((d = (l = (one)->keylen) - (two)->keylen) > 0) l -= d; \
	    for(l -= SIZEOF_LONG;;) \
	    { if(l-- <= 0)			{ cmp = d; break; } \
	      else if((cmp = *ok++ - *tk++) )	break; \
	    } \
	  } \
	}

#define MEMCPY(to,fr,n) \
	switch(n) \
	{ default:	memcpy(to,fr,n); to += n; fr += n; break; \
	  case 8 :	*to++ = *fr++; \
	  case 7 :	*to++ = *fr++; \
	  case 6 :	*to++ = *fr++; \
	  case 5 :	*to++ = *fr++; \
	  case 4 :	*to++ = *fr++; \
	  case 3 :	*to++ = *fr++; \
	  case 2 :	*to++ = *fr++; \
	  case 1 :	*to++ = *fr++; \
	}

/* merging equivalent records */
#define EQUAL(r,o,t) \
	{	if((t = r->equal) ) \
		     { t->left = (t->left->right = o); } \
		else { r->equal = (o->left = o); } \
	}

#if !_PACKAGE_ast && !__STD_C
_BEGIN_EXTERNS_
Kpvimport Void_t*	memchr _ARG_((const Void_t*, int, size_t));
Kpvimport Void_t*	memcpy _ARG_((Void_t*, const Void_t*, size_t));
_END_EXTERNS_
#endif

#define RSNOTIFY(r,o,v,x,d)	((r->events&o)?rsnotify(r,o,(Void_t*)v,(Void_t*)x,d):(0))

#define rsnotify	_rs_notify

extern int		rsnotify _ARG_((Rs_t*, int, Void_t*, Void_t*, Rsdisc_t*));

#endif /*_RSHDR_H*/
