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
 * integer tuple list internal/external implementation
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "itl.h"

#include <error.h>

#define ITLINT		uint16_t
#define ITLINTERNAL	itl2internal
#define ITLEXTERNAL	itl2external

#include "itlie.h"

#define ITLINT		uint8_t
#define ITLINTERNAL	itl1internal
#define ITLEXTERNAL	itl1external

#include "itlie.h"

#define ITLINT		uint32_t
#define ITLINTERNAL	itl4internal
#define ITLEXTERNAL	itl4external

#include "itlie.h"
