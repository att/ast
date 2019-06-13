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

#include <ctype.h>
#include <stdbool.h>

//
// Match s against t ignoring case and .'s.
//
// suf is an n element table of suffixes that may trail s
// if all isalpha() chars in s match then true is returned
// and if e is non-null it will point to the first unmatched
// char in s, otherwise false is returned
//
bool tmword(const char *s, char **e, const char *t, char **suf, int n) {
    if (!*s || !*t) return false;

    const char *b = s;
    int c;

    while (*s) {
        c = *s;
        if (c != '.') {
            if (!isalpha(c) || (c != *t && (islower(c) ? toupper(c) : tolower(c)) != *t)) break;
            t++;
        }
        s++;
    }

    if (!isalpha(c)) {
        if (c == '_') s++;
        if (e) *e = (char *)s;
        return s > b;
    }

    if (!*t && s > (b + 1)) {
        b = s;
        while (n-- && *suf) {
            t = *suf++;
            s = b;
            while (isalpha(c = *s++) && (c == *t || (islower(c) ? toupper(c) : tolower(c)) == *t)) {
                t++;
            }
            if (!*t && !isalpha(c)) {
                if (c != '_') s--;
                if (e) *e = (char *)s;
                return true;
            }
        }
    }

    return false;
}
