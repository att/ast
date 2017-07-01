/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * tag parse interface
 */

#ifndef _TAG_H
#define _TAG_H		1

#include <sfio.h>

#define TAG_VERSION	20021122L

#define TAG_ATTR_conv	0x0001

#define TAG_SCAN_dat	0x0000
#define TAG_SCAN_beg	0x0001
#define TAG_SCAN_end	0x0002
#define TAG_SCAN_rec	0x0010
#define TAG_SCAN_rep	0x0020

struct Tag_s; typedef struct Tag_s Tag_t;
struct Tagframe_s; typedef struct Tagframe_s Tagframe_t;
struct Tags_s; typedef struct Tags_s Tags_t;
struct Tagdisc_s; typedef struct Tagdisc_s Tagdisc_t;

typedef Tags_t* (*Tagbeg_f)(Tag_t*, Tagframe_t*, const char*, Tagdisc_t*);
typedef int (*Tagdat_f)(Tag_t*, Tagframe_t*, const char*, Tagdisc_t*);
typedef int (*Tagend_f)(Tag_t*, Tagframe_t*, Tagdisc_t*);

typedef int (*Tagscan_f)(Tag_t*, Tagframe_t*, void*, unsigned int, Tagdisc_t*);

struct Tagdisc_s			/* user discipline		*/
{
	unsigned long	version;	/* TAG_VERSION			*/
	const char*	id;		/* pathfind() lib id		*/
	Error_f		errorf;		/* error function		*/
};

struct Tagframe_s			/* tag parse stack frame	*/
{
	Tagframe_t*	prev;		/* previous in stack		*/
	unsigned int	level;		/* nesting level		*/
	Tags_t*		tag;		/* current tag			*/
	void*		data;		/* user defined pointer		*/
	void*		head;		/* user defined pointer		*/
	void*		tail;		/* user defined pointer		*/
	unsigned long	flags;		/* user defined flags		*/
	unsigned long	attr;		/* TAG_ATTR_* flags		*/
#ifdef _TAG_FRAME_PRIVATE_
	_TAG_FRAME_PRIVATE_
#endif
};

struct Tags_s				/* tag table entry		*/
{
	const char*	name;		/* tag name			*/
	const char*	description;	/* tag description		*/
	unsigned int	index;		/* user defined index		*/
	Tagbeg_f	begf;		/* tag begin function		*/
	Tagdat_f	datf;		/* tag data function		*/
	Tagend_f	endf;		/* tag end function		*/
};

struct Tag_s
{
	const char*	id;		/* interface id			*/
	void*		caller;		/* caller defined handle	*/
#ifdef _TAG_PRIVATE_
	_TAG_PRIVATE_
#endif
};

#define taginit(d,e)	(memset(d,0,sizeof(Tagdisc_t)),(d)->version=TAG_VERSION,(d)->errorf=(Error_f)(e))

#if _BLD_tag && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Tag_t*		tagopen(Sfio_t*, const char*, int, Tags_t*, Tagdisc_t*);
extern Sfio_t*		tagsync(Tag_t*);
extern int		tagclose(Tag_t*);

extern char*		tagcontext(Tag_t*, Tagframe_t*);

extern int		tagscan(Tags_t*, Tagscan_f, void*, Tagdisc_t*);

extern int		tagusage(Tags_t*, Sfio_t*, Tagdisc_t*);

#undef	extern

#endif
