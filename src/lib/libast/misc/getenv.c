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
#include "config_ast.h"  // IWYU pragma: keep

#include "intercepts.h"

/*
 * NOTE: the "intercepts" definition is here instead of astintercept.c because some
 *	 static linkers miss lone references to "intercepts" without "astintercept()"
 * ALSO: { 0 } definition required by some dynamic linkers averse to common symbols
 * UWIN: no _ast_getenv macro map to maintain ast54 compatibility
 */
Intercepts_t intercepts = {0};

/*
 * get name from the environment
 */

char *getenv(const char *name) {
#undef getenv
    return intercepts.intercept_getenv ? (*intercepts.intercept_getenv)(name) : getenv(name);
}
