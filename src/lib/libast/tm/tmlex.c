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

#include "ast_assert.h"
#include "tm.h"

/*
 * return the tab table index that matches s ignoring case and .'s
 * tm_data.format checked if tminfo.format!=tm_data.format
 *
 * ntab and nsuf are the number of elements in tab and suf,
 * -1 for 0 sentinel
 *
 * all isalpha() chars in str must match
 * suf is a table of nsuf valid str suffixes
 * if e is non-null then it will point to first unmatched char in str
 * which will always be non-isalpha()
 */

int tmlex(const char *s, char **e, char **tab, int ntab, char **suf, int nsuf) {
    char **p;
    char *x;
    int n;

    for (p = tab, n = ntab; n-- && (x = *p); p++) {
        if (*x && *x != '%' && tmword(s, e, x, suf, nsuf)) return p - tab;
    }
    if (tm_info.format != tm_data.format && tab >= tm_info.format &&
        tab < tm_info.format + TM_NFORM) {
        tab = tm_data.format + (tab - tm_info.format);
        if (suf && tab >= tm_info.format && tab < tm_info.format + TM_NFORM) {
            suf = tm_data.format + (suf - tm_info.format);
        }
        for (p = tab, n = ntab; n-- && *p; p++) {
            x = *p;
            if (*x && *x != '%') {
                assert(suf);
                if (tmword(s, e, x, suf, nsuf)) return p - tab;
            }
        }
    }
    return -1;
}
