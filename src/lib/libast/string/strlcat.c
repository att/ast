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
 * strlcat implementation
 */
#include "config_ast.h"  // IWYU pragma: keep

#include "ast.h"

#if _lib_strlcat

NoN(strlcat)

#else

/*
 * append t onto s limiting total size of s to n
 * s 0 terminated if n>0
 * min(n,strlen(s))+strlen(t) returned
 */
size_t strlcat(char *s, const char *t, size_t n) {
    size_t m = n;
    const char *o = t;

    if (n) {
        while (n && *s) {
            n--;
            s++;
        }
        m -= n;
        if (n) do {
                if (!--n) {
                    *s = 0;
                    break;
                }
            } while ((*s++ = *t++));
        else
            *s = 0;
    }
    if (!n)
        while (*t++)
            ;
    return (t - o) + m - 1;
}

#endif
