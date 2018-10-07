/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2011 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Research
 *
 * universe support
 *
 * symbolic link external representation has trailing '\0' and $(...) style
 * conditionals where $(...) corresponds to a kernel object (i.e., probably
 * not environ)
 *
 * universe symlink conditionals use $(UNIVERSE)
 */
#ifndef _UNIVLIB_H
#define _UNIVLIB_H 1

#include <errno.h>

#include "ast.h"

#define UNIV_SIZE 9

extern char univ_env[];

extern int getuniverse(char *);
extern int setuniverse(int);
extern int universe(int);

#endif  // _UNIVLIB_H
