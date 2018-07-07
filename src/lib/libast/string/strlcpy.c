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
//
// strlcpy() fallback implementation
//
#include "config_ast.h"  // IWYU pragma: keep

#if _lib_strlcat

// This is to silence the linker about modules that have no content.
int AST_strlcpy = 0;

#else  // _lib_strlcat

#include <sys/types.h>

size_t strlcpy(char *s, const char *t, size_t n) {
    const char *o = t;

    if (n) {
        do {
            if (!--n) {
                *s = 0;
                break;
            }
        } while ((*s++ = *t++));
    }

    if (!n) {
        while (*t++) {
            ;  // empty loop
        }
    }

    return t - o - 1;
}

#endif  // _lib_strlcat
