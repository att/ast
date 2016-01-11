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
 * integer tuple list cx internal/external interface
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _ITL_H
#define _ITL_H

#include <cx.h>

#if _BLD_itl && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern ssize_t		itl1external(Cx_t*, Cxtype_t*, int, int, int, const char*, Cxformat_t**, Cxvalue_t*, char*, size_t, Cxdisc_t*);
extern ssize_t		itl2external(Cx_t*, Cxtype_t*, int, int, int, const char*, Cxformat_t**, Cxvalue_t*, char*, size_t, Cxdisc_t*);
extern ssize_t		itl4external(Cx_t*, Cxtype_t*, int, int, int, const char*, Cxformat_t**, Cxvalue_t*, char*, size_t, Cxdisc_t*);

extern ssize_t		itl1internal(Cx_t*, Cxvalue_t*, int, int, int, const char*, size_t, Vmalloc_t*, Cxdisc_t*);
extern ssize_t		itl2internal(Cx_t*, Cxvalue_t*, int, int, int, const char*, size_t, Vmalloc_t*, Cxdisc_t*);
extern ssize_t		itl4internal(Cx_t*, Cxvalue_t*, int, int, int, const char*, size_t, Vmalloc_t*, Cxdisc_t*);

#undef	extern

#endif
