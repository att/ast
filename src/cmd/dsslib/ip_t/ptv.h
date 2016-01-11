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
 * prefix table interface definitions
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _PTV_H
#define _PTV_H

#include <ast.h>
#include <cdt.h>
#include <pt.h>

#define PTV_VERSION		20090315L

#define PTV_REGISTERS		6

#define Ptvdisc_t		Ptdisc_t

struct Ptv_s; typedef struct Ptv_s Ptv_t;
struct Ptvprefix_s; typedef struct Ptvprefix_s Ptvprefix_t;

typedef unsigned char* Ptvaddr_t;
typedef uintmax_t Ptvcount_t;

struct Ptvprefix_s
{
	unsigned char*		min;
	unsigned char*		max;
	union
	{
	long			number;
	void*			pointer;
	}			data;
	Dtlink_t		link;
};

struct Ptv_s
{
	Ptvcount_t	entries;
	int		size;
	Dt_t*		dict;
	Ptvdisc_t*	disc;
	unsigned char*	r[PTV_REGISTERS];
#ifdef _PTV_PRIVATE_
	_PTV_PRIVATE_
#endif
};

#if _BLD_pt && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

#define ptvinit(d)	(memset(d,0,sizeof(Ptvdisc_t)),(d)->version=PTV_VERSION)

extern unsigned char*	ptvmin(int, unsigned char*, const unsigned char*, int);
extern unsigned char*	ptvmax(int, unsigned char*, const unsigned char*, int);

extern Ptv_t*		ptvopen(Ptvdisc_t*, int);
extern int		ptvclose(Ptv_t*);
extern Ptvprefix_t*	ptvinsert(Ptv_t*, unsigned char*, unsigned char*);
extern int		ptvdelete(Ptv_t*, unsigned char*, unsigned char*);

extern Ptvprefix_t*	ptvmatch(Ptv_t*, unsigned char*);

extern int		ptvprint(Ptv_t*, Sfio_t*);
extern int		ptvstats(Ptv_t*, Sfio_t*);
extern int		ptvdump(Ptv_t*, Sfio_t*);

extern int		ptvaddresses(Ptv_t*, unsigned char*);
extern Ptvcount_t	ptvranges(Ptv_t*);
extern Ptvcount_t	ptvsize(Ptv_t*);

extern Ptv_t*		ptvcopy(Ptv_t*);
extern Ptv_t*		ptvinvert(Ptv_t*);
extern Ptv_t*		ptvintersect(Ptv_t*, Ptv_t*);
extern Ptv_t*		ptvunion(Ptv_t*, Ptv_t*);
extern Ptv_t*		ptvdifference(Ptv_t*, Ptv_t*);
extern Ptv_t*		ptvcover(Ptv_t*, Ptv_t*);
extern Ptv_t*		ptvrebit(Ptv_t*, int);

extern int		ptvequal(Ptv_t*, Ptv_t*);
extern int		ptvsubset(Ptv_t*, Ptv_t*);

#undef	extern

#endif
