/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * integer list regular expression interface
 *
 * Glenn Fowler
 * AT&T research
 */

#ifndef _IRE_H
#define _IRE_H

#include <ast_common.h>

#define IRE_VERSION	20030115L		/* interface version	*/

#define IRE_SET		(~0)			/* as path set marker	*/

#define ireinit(p)	(memset(p,0,sizeof(*(p))),(p)->version=IRE_VERSION)

typedef uint32_t Ireint_t;

struct Ire_s; typedef struct Ire_s Ire_t;
struct Iredisc_s; typedef struct Iredisc_s Iredisc_t;

typedef void* (*Ireresize_f)(void*, void*, size_t);

struct Iredisc_s
{
	unsigned long	version;	/* discipline version		*/
	Error_f		errorf;		/* error function		*/
	Ireresize_f	resizef;	/* alloc/free function		*/
	void*		resizehandle;	/* resizef handle		*/
};

struct Ire_s				/* RE handle			*/
{
	const char*	id;		/* interface id			*/
	int		element;	/* element size			*/
	int		dots;		/* element dots			*/
	int		tuple;		/* tuple size			*/
	unsigned long	group;		/* embedded group mark		*/
#ifdef _IRE_PRIVATE_
	_IRE_PRIVATE_
#endif
};

#if _BLD_bgp && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Ire_t*		irecomp(const char*, int, int, int, int, Iredisc_t*);
extern int		ireexec(Ire_t*, void*, size_t);
extern int		irefree(Ire_t*);

#undef	extern

#endif
