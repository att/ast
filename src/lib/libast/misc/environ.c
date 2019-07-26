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
#include "config_ast.h"  // IWYU pragma: keep

#include <stdlib.h>
#include <string.h>

#include "ast.h"

#undef getenv
extern char *getenv(const char *name);
char *sh_getenv(const char *name) { return getenv(name); }

//
// Put name=value in the environment.
// Pointer to value returned.
// environ==0 is ok
//
//      setenviron("N=V")       add N=V
//      setenviron("N")         delete N
//
char *sh_setenviron(const char *akey) {
    ast.env_serial++;
    if (strchr(akey, '=')) {
        putenv((char *)akey);
    } else {
        unsetenv(akey);
    }
    return (char *)akey;
}
