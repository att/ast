/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2011 AT&T Intellectual Property          *
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
 * uu encode/decode interface definitions
 *
 * AT&T Research
 */

#ifndef _UU_H
#define _UU_H

#include <ast.h>

#define UU_VERSION	19980611L

#define UU_HEADER	(1<<0)		/* header/trailer encoded too	*/
#define UU_TEXT		(1<<1)		/* process text file		*/
#define UU_LOCAL	(1<<2)		/* embedded paths in .		*/

struct Uu_s; typedef struct Uu_s Uu_t;
struct Uudisc_s; typedef struct Uudisc_s Uudisc_t;
struct Uumeth_s; typedef struct Uumeth_s Uumeth_t;

typedef int (*Uu_f)(Uu_t*);

struct Uumeth_s
{
	const char*	name;
	const char*	alias;
	const char*	id;
	Uu_f		headerf;
	Uu_f		encodef;
	Uu_f		decodef;
	void*		data;
};

struct Uudisc_s
{
	unsigned long	version;
	unsigned long	flags;
	Error_f		errorf;
};

struct Uu_s
{
	const char*	id;
	Uumeth_t	meth;
	Uudisc_t*	disc;
	char*		path;

#ifdef _UU_PRIVATE_
	_UU_PRIVATE_
#endif

};

#if _BLD_uu && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Uu_t*		uuopen(Uudisc_t*, Uumeth_t*);
extern int		uuclose(Uu_t*);

extern ssize_t		uuencode(Uu_t*, Sfio_t*, Sfio_t*, size_t, const char*);
extern ssize_t		uudecode(Uu_t*, Sfio_t*, Sfio_t*, size_t, const char*);

extern int		uumain(char**, int);

extern int		uulist(Sfio_t*);
extern Uumeth_t*	uumeth(const char*);

#undef	extern

#endif
