/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
 *                    David Korn <dgkorn@gmail.com>                     *
 *                     Phong Vo <phongvo@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
#ifndef _ASO_H
#define _ASO_H 1

#define ASO_VERSION 20130501L

#include <sched.h>

#include "ast_aso.h"
#include "tm.h"

/*
 * ast atomic scalar operations interface definitions
 */

/* usable in a spin-loop to acquire resource */
#define asospinrest() tmsleep(0, 1 << 18)
#define asospindecl() unsigned int _asor
#define asospininit() (_asor = 1 << 17)
#define asospinnext() \
    (tmsleep(0, _asor <<= 1), _asor >= (1 << 21) ? (sched_yield(), asospininit()) : 0)

#define asocaschar(p, o, n) asocas8((uint8_t volatile *)p, o, n)
#define asogetchar(p) asoget8((uint8_t volatile *)p)
#define asoaddchar(p, n) asoadd8((uint8_t volatile *)p, n)
#define asosubchar(p, n) asosub8((uint8_t volatile *)p, n)
#define asoincchar(p) asoinc8((uint8_t volatile *)p)
#define asodecchar(p) asodec8((uint8_t volatile *)p)
#define asominchar(p, n) asomin8((uint8_t volatile *)p, n)
#define asomaxchar(p, n) asomax8((uint8_t volatile *)p, n)

#define asocasshort(p, o, n) asocas16((uint16_t volatile *)p, o, n)
#define asogetshort(p) asoget16((uint16_t volatile *)p)
#define asoaddshort(p, n) asoadd16((uint16_t volatile *)p, n)
#define asosubshort(p, n) asosub16((uint16_t volatile *)p, n)
#define asoincshort(p) asoinc16((uint16_t volatile *)p)
#define asodecshort(p) asodec16((uint16_t volatile *)p)
#define asominshort(p, n) asomin16((uint16_t volatile *)p, n)
#define asomaxshort(p, n) asomax16((uint16_t volatile *)p, n)

#if _ast_sizeof_int == 4
#define asocasint(p, o, n) asocas32((uint32_t volatile *)p, o, n)
#define asogetint(p) asoget32((uint32_t volatile *)p)
#define asoaddint(p, n) asoadd32((uint32_t volatile *)p, n)
#define asosubint(p, n) asosub32((uint32_t volatile *)p, n)
#define asoincint(p) asoinc32((uint32_t volatile *)p)
#define asodecint(p) asodec32((uint32_t volatile *)p)
#define asominint(p, n) asomin32((uint32_t volatile *)p, n)
#define asomaxint(p, n) asomax32((uint32_t volatile *)p, n)
#endif

#if _ast_sizeof_long == 4
#define asocaslong(p, o, n) asocas32((uint32_t volatile *)p, o, n)
#define asogetlong(p) asoget32((uint32_t volatile *)p)
#define asoaddlong(p, n) asoadd32((uint32_t volatile *)p, n)
#define asosublong(p, n) asosub32((uint32_t volatile *)p, n)
#define asoinclong(p) asoinc32((uint32_t volatile *)p)
#define asodeclong(p) asodec32((uint32_t volatile *)p)
#define asominlong(p, n) asomin32((uint32_t volatile *)p, n)
#define asomaxlong(p, n) asomax32((uint32_t volatile *)p, n)
#endif

#if _ast_sizeof_size_t == 4
#define asocassize(p, o, n) asocas32((uint32_t volatile *)p, o, n)
#define asogetsize(p) asoget32((uint32_t volatile *)p)
#define asoaddsize(p, n) asoadd32((uint32_t volatile *)p, n)
#define asosubsize(p, n) asosub32((uint32_t volatile *)p, n)
#define asoincsize(p) asoinc32((uint32_t volatile *)p)
#define asodecsize(p) asodec32((uint32_t volatile *)p)
#define asominsize(p, n) asomin32((uint32_t volatile *)p, n)
#define asomaxsize(p, n) asomax32((uint32_t volatile *)p, n)
#endif

#if _ast_sizeof_off_t == 4
#define asocasoff(p, o, n) asocas32((uint32_t volatile *)p, o, n)
#define asogetoff(p) asoget32((uint32_t volatile *)p)
#define asoaddoff(p, n) asoadd32((uint32_t volatile *)p, n)
#define asosuboff(p, n) asosub32((uint32_t volatile *)p, n)
#define asoincoff(p) asoinc32((uint32_t volatile *)p)
#define asodecoff(p) asodec32((uint32_t volatile *)p)
#define asominoff(p, n) asomin32((uint32_t volatile *)p, n)
#define asomaxoff(p, n) asomax32((uint32_t volatile *)p, n)
#endif

#ifdef _ast_int8_t

#if _ast_sizeof_int == 8
#define asocasint(p, o, n) asocas64((uint64_t volatile *)p, o, n)
#define asogetint(p) asoget64((uint64_t volatile *)p)
#define asoaddint(p, n) asoadd64((uint64_t volatile *)p, n)
#define asosubint(p, n) asosub64((uint64_t volatile *)p, n)
#define asoincint(p) asoinc64((uint64_t volatile *)p)
#define asodecint(p) asodec64((uint64_t volatile *)p)
#define asominint(p, n) asomin64((uint64_t volatile *)p, n)
#define asomaxint(p, n) asomax64((uint64_t volatile *)p, n)
#endif

#if _ast_sizeof_long == 8
#define asocaslong(p, o, n) asocas64((uint64_t volatile *)p, o, n)
#define asogetlong(p) asoget64((uint64_t volatile *)p)
#define asoaddlong(p, n) asoadd64((uint64_t volatile *)p, n)
#define asosublong(p, n) asosub64((uint64_t volatile *)p, n)
#define asoinclong(p) asoinc64((uint64_t volatile *)p)
#define asodeclong(p) asodec64((uint64_t volatile *)p)
#define asominlong(p, n) asomin64((uint64_t volatile *)p, n)
#define asomaxlong(p, n) asomax64((uint64_t volatile *)p, n)
#endif

#if _ast_sizeof_size_t == 8
#define asocassize(p, o, n) asocas64((uint64_t volatile *)p, o, n)
#define asogetsize(p) asoget64((uint64_t volatile *)p)
#define asoaddsize(p, n) asoadd64((uint64_t volatile *)p, n)
#define asosubsize(p, n) asosub64((uint64_t volatile *)p, n)
#define asoincsize(p) asoinc64((uint64_t volatile *)p)
#define asodecsize(p) asodec64((uint64_t volatile *)p)
#define asominsize(p, n) asomin64((uint64_t volatile *)p, n)
#define asomaxsize(p, n) asomax64((uint64_t volatile *)p, n)
#endif

#if _ast_sizeof_off_t == 8
#define asocasoff(p, o, n) asocas64((uint64_t volatile *)p, o, n)
#define asogetoff(p) asoget64((uint64_t volatile *)p)
#define asoaddoff(p, n) asoadd64((uint64_t volatile *)p, n)
#define asosuboff(p, n) asosub64((uint64_t volatile *)p, n)
#define asoincoff(p) asoinc64((uint64_t volatile *)p)
#define asodecoff(p) asodec64((uint64_t volatile *)p)
#define asominoff(p, n) asomin64((uint64_t volatile *)p, n)
#define asomaxoff(p, n) asomax64((uint64_t volatile *)p, n)
#endif

#endif

#endif  // _ASO_H
