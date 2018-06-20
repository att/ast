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
 * strlcpy implementation
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <ast.h>

#if _lib_strlcat

NoN(strlcpy)

#else

/*
 * copy at most n chars from t into s
 * result 0 terminated if n>0
 * strlen(t) returned
 */
size_t strlcpy(char *s, const char *t, size_t n) {
    const char *o = t;

    if (n) do {
            if (!--n) {
                *s = 0;
                break;
            }
        } while ((*s++ = *t++));
    if (!n)
        while (*t++)
            ;
    return t - o - 1;
}

#endif
