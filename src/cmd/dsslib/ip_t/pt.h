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

#ifndef _PT_H
#define _PT_H

#include <ast.h>
#include <cdt.h>

#define PT_VERSION		20090315L

#define PTBITS			32
#define PTSHIFT			1

#define PTMIN(a,b)		((a)&~((b)?((((Ptaddr_t)1)<<(PTBITS-(b)))-1):~((Ptaddr_t)0)))
#define PTMAX(a,b)		((a)|((b)?((((Ptaddr_t)1)<<(PTBITS-(b)))-1):~((Ptaddr_t)0)))

struct Pt_s; typedef struct Pt_s Pt_t;
struct Ptdisc_s; typedef struct Ptdisc_s Ptdisc_t;
struct Ptprefix_s; typedef struct Ptprefix_s Ptprefix_t;

typedef  uint32_t Ptaddr_t;
typedef uintmax_t Ptcount_t;

struct Ptprefix_s
{
	Ptaddr_t		min;
	Ptaddr_t		max;
	union
	{
	long			number;
	void*			pointer;
	}			data;
	Dtlink_t		link;
};

struct Ptdisc_s				/* user discipline		*/
{
	unsigned long	version;	/* interface version		*/
	Error_f		errorf;		/* error function		*/
};

struct Pt_s
{
	Ptcount_t	entries;
	Dt_t*		dict;
	Ptdisc_t*	disc;
#ifdef _PT_PRIVATE_
	_PT_PRIVATE_
#endif
};

#if _BLD_pt && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

#define ptinit(d)	(memset(d,0,sizeof(Ptdisc_t)),(d)->version=PT_VERSION)

extern Pt_t*		ptopen(Ptdisc_t*);
extern int		ptclose(Pt_t*);
extern Ptprefix_t*	ptinsert(Pt_t*, Ptaddr_t, Ptaddr_t);
extern int		ptdelete(Pt_t*, Ptaddr_t, Ptaddr_t);

extern Ptprefix_t*	ptmatch(Pt_t*, Ptaddr_t);

extern int		ptprint(Pt_t*, Sfio_t*);
extern int		ptstats(Pt_t*, Sfio_t*);
extern int		ptdump(Pt_t*, Sfio_t*);

extern Ptcount_t	ptaddresses(Pt_t*);
extern Ptcount_t	ptranges(Pt_t*);
extern Ptcount_t	ptsize(Pt_t*);

extern Pt_t*		ptcopy(Pt_t*);
extern Pt_t*		ptinvert(Pt_t*);
extern Pt_t*		ptintersect(Pt_t*, Pt_t*);
extern Pt_t*		ptunion(Pt_t*, Pt_t*);
extern Pt_t*		ptdifference(Pt_t*, Pt_t*);
extern Pt_t*		ptcover(Pt_t*, Pt_t*);
extern Pt_t*		ptrebit(Pt_t*, int);

extern int		ptequal(Pt_t*, Pt_t*);
extern int		ptsubset(Pt_t*, Pt_t*);

#undef	extern

#endif
