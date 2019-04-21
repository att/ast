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
#include "config_ast.h"  // IWYU pragma: keep

#include <math.h>

#include "sfhdr.h"
#include "sfio.h"

/*      Read a portably coded double value
**
**      Written by Kiem-Phong Vo
*/

Sfdouble_t sfgetd(Sfio_t *f) {
    uchar *s, *ends, c;
    int p, sign, exp;
    Sfdouble_t v;
    SFMTXDECL(f)  // declare a local stream variable for multithreading

    SFMTXENTER(f, -1.0)

    if ((sign = sfgetc(f)) < 0 || (exp = (int)sfgetu(f)) < 0) SFMTXRETURN(f, -1.0)

    if (f->mode != SF_READ && _sfmode(f, SF_READ, 0) < 0) SFMTXRETURN(f, -1.0)

    SFLOCK(f, 0)

    v = 0.;
    for (;;) { /* fast read for data */
        if (SFRPEEK(f, s, p) <= 0) {
            f->flags |= SF_ERROR;
            v = -1.;
            goto done;
        }

        for (ends = s + p; s < ends;) {
            c = *s++;
            v += SFUVALUE(c);
            v = ldexpl(v, -SF_PRECIS);
            if (!(c & SF_MORE)) {
                f->next = s;
                goto done;
            }
        }
        f->next = s;
    }

done:
    v = ldexpl(v, (sign & 02) ? -exp : exp);
    if (sign & 01) v = -v;

    SFOPEN(f)
    SFMTXRETURN(f, v)
}
