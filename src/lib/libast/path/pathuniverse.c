/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2014 AT&T Intellectual Property          *
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
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <string.h>

// Returns true if the current universe as defined by the PATH env var, or the default universe,
// indicates the command should provide BSD rather than SysV semantics.
//
// This code was lifted out of the astconf.c module and was changed only as needed to make it a
// standalone function. I mention that because, beyond the problems with the style, it has several
// undesirable behaviors. But since changing the behavior could conceivably be noticed by someone
// (thus changing how `echo` works) we should not change the implementation.
bool path_is_bsd_universe() {
    bool is_bsd = strcmp(_UNIV_DEFAULT, "att") != 0;

    const char *p = getenv("PATH");
    if (!p) return is_bsd;

    int r = 1;
    while (true) {
        switch (*p++) {
            case 0:
                break;
            case ':':
                r = 1;
                continue;
            case '/':
                if (r) {
                    r = 0;
                    if (p[0] == 'u' && p[1] == 's' && p[2] == 'r' && p[3] == '/') {
                        for (p += 4; *p == '/'; p++) {
                            ;  // empty loop
                        }
                    }
                    if (p[0] == 'b' && p[1] == 'i' && p[2] == 'n') {
                        for (p += 3; *p == '/'; p++) {
                            ;  // empty loop
                        }
                        if (!*p || *p == ':') break;
                    }
                }
                if (!strncmp(p, "xpg", 3)) {
                    is_bsd = false;
                    break;
                }
                if (!strncmp(p, "bsd", 3) || !strncmp(p, "ucb", 3)) {
                    is_bsd = true;
                    break;
                }
                continue;
            default:
                r = 0;
                continue;
        }
        break;
    }

    return is_bsd;
}
