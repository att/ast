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

#include <string.h>

#include "ast.h"

// This exists solely for libast unit tests which don't link against the ksh code. It emulates the
// behavior of the ksh functions of the same name. As of ksh93v- and earlier releases this was much
// more complex with a dispatch table. The intent was to allow code that used the `getenv()` and
// `setenviron()` functions to work regardless of whether or not the ksh code was linked into the
// binary. It also supported, at least in theory if not practice, other code (e.g., ksh plugins and
// functions from system libraries) to call `getenv()` and get the ksh version.

// TODO: See if this module can be removed. For example, by turning the relevant ksh code into a
// library that is also linked with the libast unit tests.

#undef getenv
extern char *getenv(const char *name);
__attribute__((weak)) char *sh_getenv(const char *name) { return getenv(name); }

/*
 * put name=value in the environment
 * pointer to value returned
 * environ==0 is ok
 *
 *	setenviron("N=V")	add N=V
 *	setenviron("N")		delete N
 *	setenviron(0)		expect more (pre-fork optimization)
 *
 * _ always placed at the top
 */

#define INCREMENT 16 /* environ increment		*/

__attribute__((weak)) char *sh_setenviron(const char *akey) {
    static char **envv;    /* recorded environ		*/
    static char **next;    /* next free slot		*/
    static char **last;    /* last free slot (0)		*/
    static char ok[] = ""; /* delete/optimization ok return*/

    char *key = (char *)akey;
    char **v = environ;
    char **p = envv;
    char *s;
    char *t;
    int n;

    ast.env_serial++;
    if (p && !v) {
        environ = next = p;
        *++next = 0;
    } else if (p != v || !v) {
        if (v) {
            while (*v++)
                ;
            n = v - environ + INCREMENT;
            v = environ;
        } else
            n = INCREMENT;
        if (!p || (last - p + 1) < n) {
            if (!(p = newof(p, char *, n, 0))) return 0;
            last = p + n - 1;
        }
        envv = environ = p;
        if (v && v[0] && v[0][0] == '_' && v[0][1] == '=') {
            *p++ = *v++;
        } else {
            *p++ = "_=";
        }
        if (!v) {
            *p = 0;
        } else {
            while ((*p = *v++)) {
                if (p[0][0] == '_' && p[0][1] == '=') {
                    envv[0] = *p;
                } else {
                    p++;
                }
            }
        }
        next = p;
        p = envv;
    } else if (next == last) {
        n = last - v + INCREMENT + 1;
        if (!(p = newof(p, char *, n, 0))) return 0;
        last = p + n - 1;
        next = last - INCREMENT;
        envv = environ = p;
    }
    if (!key) return ok;
    for (; *p; p++) {
        s = *p;
        t = key;
        do {
            if (!*t || *t == '=') {
                if (*s == '=') {
                    if (!*t) {
                        v = p++;
                        while ((*v++ = *p++)) {
                            ;  // empty loop
                        }
                        next--;
                        return ok;
                    }
                    *p = key;
                    return (s = strchr(key, '=')) ? s + 1 : (char *)0;
                }
                break;
            }
        } while (*t++ == *s++);
    }
    s = strchr(key, '=');
    if (!s) return ok;
    p = next;
    *++next = 0;
    *p = key;
    return s + 1;
}
