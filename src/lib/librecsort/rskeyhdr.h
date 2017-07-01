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
#pragma prototyped

/*
 * rskey internal interface
 */

#ifndef _RSKEYHDR_H
#define _RSKEYHDR_H	1

#if _PACKAGE_ast
#include <ast.h>
#endif

#include <ctype.h>
#include <ccode.h>

#ifndef UCHAR_MAX
#define UCHAR_MAX	((unsigned char)(~0))
#endif
#ifndef SHRT_MAX
#define SHRT_MAX	((short)(~((unsigned short)0)) >> 1)
#endif
#ifndef INT_MAX
#define INT_MAX		((int)(~((unsigned int)0)) >> 1)
#endif
#ifndef LONG_MAX
#define LONG_MAX	((long)(~((unsigned long)0)) >> 1)
#endif

#define INSIZE		PROCSIZE	/* default insize		*/
#define OUTSIZE		(64*1024)	/* default outsize		*/
#define PROCSIZE	(4*1024*1024)	/* default procsize		*/

#define MAXFIELD	INT_MAX

#define blank(c)	((c)==' '||(c)=='\t')

typedef struct Position_s		/* field position		*/
{
	int		field;		/* field offset			*/
	int		index;		/* char offset			*/
} Position_t;

#define _RSKEYFIELD_PRIVATE_ \
	unsigned char	aflag;		/* accumulate dups here		*/ \
	unsigned char	bflag;		/* skip initial blanks		*/ \
	unsigned char	eflag;		/* skip trailing blanks		*/ \
	unsigned char	standard;	/* 1:-k 0:+pos-pos		*/ \
	int		binary;		/* binary data			*/ \
	int		code;		/* coder ccode or conversion	*/ \
	int		index;		/* field definition index	*/ \
	int		freetrans;	/* free trans on close		*/ \
	unsigned char*	trans;		/* translation table		*/ \
	unsigned char*	keep;		/* deletion table		*/ \
	void*		data;		/* coder specific data		*/ \
	Position_t	begin;		/* key begins here		*/ \
	Position_t	end;		/* and ends here		*/

typedef struct
{
	unsigned char	ident[UCHAR_MAX + 1];	/* identity transform	*/
	unsigned char	fold[UCHAR_MAX + 1];	/* fold case		*/

	unsigned char	all[UCHAR_MAX + 1];	/* all significant	*/
	unsigned char	dict[UCHAR_MAX + 1];	/* dictionary order	*/
	unsigned char	print[UCHAR_MAX + 1];	/* printable significant*/
} State_t;

#define _RSKEY_PRIVATE_ \
	State_t*	state;		/* readonly state		*/ \
	Rsdisc_t*	disctail;	/* rslib() disc stack tail	*/ \
	struct								   \
	{								   \
	Rskeyfield_t	global;		/* global field info		*/ \
	Rskeyfield_t*	prev;		/* previous field list tail	*/ \
	int		index;		/* last field index		*/ \
	int		maxfield;	/* max field position		*/ \
	unsigned char**	positions;	/* field start positions	*/ \
	}		field;		/* key field info		*/ \
	struct								   \
	{								   \
	Rskeyfield_t*	head;		/* accumulate list head		*/ \
	Rskeyfield_t*	tail;		/* accumulate list tail		*/ \
	}		accumulate;	/* accumulate field info	*/ \
	unsigned char*	xfrmbuf;	/* strxfrm buffer		*/ \
	unsigned int	xfrmsiz;	/* strxfrm buffer size		*/ \
	unsigned long	shuffle;	/* shuffle seed			*/ \
	unsigned char	coded;		/* coded keys specified		*/

#include "rshdr.h"

#if !__STD_C && !defined(const)
#define const
#endif

#if !_PACKAGE_ast
#if __STD_C
#include <string.h>
#endif
#define elementsof(x)	(sizeof(x)/sizeof(x[0]))
#define roundof(x,y)	(((x)+(y)-1)&~((y)-1))
#define streq(a,b)	(*(a)==*(b)&&!strcmp(a,b))
#endif

#endif
