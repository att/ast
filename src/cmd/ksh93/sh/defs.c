/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2011 AT&T Intellectual Property          *
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
 *                    David Korn <dgkorn@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
//
// Ksh - AT&T Labs
// Written by David Korn
// This file defines all the  read/write shell global variables
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stddef.h>
#include <sys/types.h>

#include "cdt.h"
#include "defs.h"
#include "jobs.h"
#include "name.h"

Shell_t sh = {.shcomp = 0};
struct jobs job = {.pwlist = NULL};
Dtdisc_t _Nvdisc = {.key = offsetof(Namval_t, nvname), .size = -1, .comparf = nv_compare};
struct shared *shgd = NULL;
int32_t sh_mailchk = 600;

// These two magic pointers are used to distinguish the purpose of the `extra` parameter of the
// `sh_addbuiltin()` function. It should be one of these two values, NULL, or a `Namfun_t*`.
static int _builtin_delete;
static int _builtin_disable;
void *const builtin_delete = &_builtin_delete;
void *const builtin_disable = &_builtin_disable;
