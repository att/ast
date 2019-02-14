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
 * AT&T Bell Laboratories
 *
 * time conversion support
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <stddef.h>

#include "sfio.h"

/*
 * n is minutes west of UTC
 *
 * append p and SHHMM part of n to s
 * where S is + or -
 *
 * n ignored if n==d
 * end of s is returned
 */

char *tmpoff(char *s, size_t z, const char *p, int n, int d) {
    char *e = s + z;

    while (s < e && (*s = *p++)) s++;
    if (n != d && s < e) {
        if (n < 0) {
            n = -n;
            *s++ = '+';
        } else {
            *s++ = '-';
        }
        s += sfsprintf(s, e - s, "%02d%s%02d", n / 60, d == -24 * 60 ? ":" : "", n % 60);
    }
    return s;
}
