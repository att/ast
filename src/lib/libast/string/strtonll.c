/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2012 AT&T Intellectual Property          *
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
/*
 * strtonll() implementation
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <stdint.h>

#include "ast.h"

#define S2I_function strton64
#define S2I_number int64_t
#define S2I_unumber uint64_t
#define S2I_multiplier 1

// TODO: There should be an unsigned variant that defines the following symbols:
// #define S2I_function strtonu64
// #define S2I_unsigned 1

#include "strtoi.h"
