/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2012 AT&T Intellectual Property          *
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
 * posix regex executor
 * single unsized-string interface
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"  // IWYU pragma: keep
#include "ast_regex.h"

//
// Standard wrapper for the sized-record interface.
//
int regexec(const regex_t *p, const char *s, size_t nmatch, regmatch_t *match, regflags_t flags) {
    if (flags & REG_STARTEND) {
        int r;
        int m = match->rm_so;
        regmatch_t *e;

        if (!(r = regnexec(p, s + m, match->rm_eo - m, nmatch, match, flags)) && m > 0) {
            for (e = match + nmatch; match < e; match++) {
                if (match->rm_so >= 0) {
                    match->rm_so += m;
                    match->rm_eo += m;
                }
            }
        }
        return r;
    }
    return regnexec(p, s, s ? strlen(s) : 0, nmatch, match, flags);
}
