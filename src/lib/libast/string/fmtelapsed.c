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
// Return pointer to human readable elapsed time for u * 1/n secs
// Compatible with strelapsed()
// Return value length is at most 7
//
#include "config_ast.h"  // IWYU pragma: keep

#include "ast.h"
#include "sfio.h"

// This function returns memory allocated by `fmtbuf()` function.
// So it's return value should be dup'ed if it's kept in a non-local variable.
char *fmtelapsed(unsigned long u, int n) {
    unsigned long t;
    char *buf;
    int z;

    if (u == 0L) return "0";
    if (u == ~0L) return "%";
    buf = fmtbuf(z = 8);
    t = u / n;
    if (t < 60) {
        sfsprintf(buf, z, "%lu.%02lus", t, (u * 100 / n) % 100);
    } else if (t < 60 * 60) {
        sfsprintf(buf, z, "%lum%02lus", t / 60, t - (t / 60) * 60);
    } else if (t < 24 * 60 * 60) {
        sfsprintf(buf, z, "%luh%02lum", t / (60 * 60), (t - (t / (60 * 60)) * (60 * 60)) / 60);
    } else if (t < 7 * 24 * 60 * 60) {
        sfsprintf(buf, z, "%lud%02luh", t / (24 * 60 * 60),
                  (t - (t / (24 * 60 * 60)) * (24 * 60 * 60)) / (60 * 60));
    } else if (t < 31 * 24 * 60 * 60) {
        sfsprintf(buf, z, "%luw%02lud", t / (7 * 24 * 60 * 60),
                  (t - (t / (7 * 24 * 60 * 60)) * (7 * 24 * 60 * 60)) / (24 * 60 * 60));
    } else if (t < 365 * 24 * 60 * 60) {
        sfsprintf(buf, z, "%luM%02lud", (t * 12) / (365 * 24 * 60 * 60),
                  ((t * 12) - ((t * 12) / (365 * 24 * 60 * 60)) * (365 * 24 * 60 * 60)) /
                      (12 * 24 * 60 * 60));
    } else if (t < (365UL * 4UL + 1UL) * 24UL * 60UL * 60UL) {
        sfsprintf(
            buf, z, "%luY%02luM", t / (365 * 24 * 60 * 60),
            ((t - (t / (365 * 24 * 60 * 60)) * (365 * 24 * 60 * 60)) * 5) / (152 * 24 * 60 * 60));
    } else {
        sfsprintf(buf, z, "%luY%02luM", (t * 4) / ((365UL * 4UL + 1UL) * 24UL * 60UL * 60UL),
                  (((t * 4) - ((t * 4) / ((365UL * 4UL + 1UL) * 24UL * 60UL * 60UL)) *
                                  ((365UL * 4UL + 1UL) * 24UL * 60UL * 60UL)) *
                   5) /
                      ((4 * 152 + 1) * 24 * 60 * 60));
    }
    return buf;
}
