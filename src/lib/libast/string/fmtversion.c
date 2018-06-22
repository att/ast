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
 * return formatted <magicid.h> version string
 */
#include "config_ast.h"  // IWYU pragma: keep

#include "ast.h"

char *fmtversion(unsigned long v) {
    char *cur;
    char *end;
    char *buf;
    int n;

    buf = cur = fmtbuf(n = 18);
    end = cur + n;
    if (v >= 19700101L && v <= 29991231L)
        sfsprintf(cur, end - cur, "%04lu-%02lu-%02lu", (v / 10000) % 10000, (v / 100) % 100,
                  v % 100);
    else {
        n = (v >> 24) & 0xff;
        if (n) cur += sfsprintf(cur, end - cur, "%d.", n);
        n = (v >> 16) & 0xff;
        if (n) cur += sfsprintf(cur, end - cur, "%d.", n);
        sfsprintf(cur, end - cur, "%ld.%ld", (v >> 8) & 0xff, v & 0xff);
    }
    return buf;
}
