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

#include <stddef.h>
#include <string.h>

#include "ast_assert.h"
#include "sfhdr.h"
#include "sfio.h"

/*      Read a record delineated by a character.
**      The record length can be accessed via sfvalue(f).
**
**      Written by Kiem-Phong Vo
*/

char *sfgetr(Sfio_t *f, int rc, int type) {
    ssize_t n, un;
    uchar *s, *ends, *us;
    int found;
    Sfrsrv_t *rsrv;
    SFMTXDECL(f)  // declare a local stream variable for multithreading

    SFMTXENTER(f, NULL)

    if (rc < 0 || (f->mode != SF_READ && _sfmode(f, SF_READ, 0) < 0)) SFMTXRETURN(f, NULL)
    SFLOCK(f, 0)

    /* buffer to be returned */
    rsrv = NULL;
    us = NULL;
    un = 0;
    found = 0;

    /* compatibility mode */
    type = type < 0 ? SF_LASTR : type == 1 ? SF_STRING : type;

    if (type & SF_LASTR) /* return the broken record */
    {
        if ((f->flags & SF_STRING) && (un = f->endb - f->next)) {
            us = f->next;
            f->next = f->endb;
            found = 1;
        } else if ((rsrv = f->rsrv) && (un = -rsrv->slen) > 0) {
            us = rsrv->data;
            found = 1;
        }
        goto done;
    }

    while (!found) {  // fill buffer if necessary
        ends = f->endb;
        s = f->next;
        n = ends - s;
        if (n <= 0) {  // for unseekable devices, peek-read 1 record
            f->getr = rc;
            f->mode |= SF_RC;

            // Fill buffer the conventional way.
            if (SFRPEEK(f, s, n) <= 0) {
                us = NULL;
                goto done;
            } else {
                ends = s + n;
                if (f->mode & SF_RC) {
                    s = ends[-1] == rc ? ends - 1 : ends;
                    goto do_copy;
                }
            }
        }

        if (!(s = (uchar *)memchr((char *)s, rc, n))) s = ends;
    do_copy:
        if (s < ends) /* found separator */
        {
            s += 1; /* include the separator */
            found = 1;

            if (!us &&
                (!(type & SF_STRING) || !(f->flags & SF_STRING) ||
                 ((f->flags & SF_STRING) && (f->bits & SF_BOTH)))) { /* returning data in buffer */
                us = f->next;
                un = s - f->next;
                f->next = s;
                goto done;
            }
        }

        /* amount to be read */
        n = s - f->next;

        if (!found && ((_Sfmaxr > 0 && un + n + 1 >= _Sfmaxr) ||
                       (f->flags & SF_STRING))) /* already exceed limit */
        {
            us = NULL;
            goto done;
        }

        /* get internal buffer */
        if (!rsrv || rsrv->size < un + n + 1) {
            if (rsrv) rsrv->slen = un;
            rsrv = _sfrsrv(f, un + n + 1);
            if (rsrv) {
                us = rsrv->data;
            } else {
                us = NULL;
                goto done;
            }
        }

        /* now copy data */
        s = us + un;
        un += n;
        ends = f->next;
        f->next += n;
        memcpy(s, ends, n);
        // These statements are commented out because the assignments are pointless. The updated
        // value of each var is unused on the next execution of the loop. I'm leaving them here as
        // comments in case they are relevant to someone investigating a bug in the future.
        //   s += n;
        //   ends += n;
    }

done:
    _Sfi = f->val = un;
    f->getr = 0;
    if (found && rc != 0 && (type & SF_STRING)) {
        assert(us);
        us[un - 1] = '\0';
        if (us >= f->data && us < f->endb) {
            f->getr = rc;
            f->mode |= SF_GETR;
        }
    }

    /* prepare for a call to get the broken record */
    if (rsrv) rsrv->slen = found ? 0 : -un;

    SFOPEN(f)

    if (us && (type & SF_LOCKR)) {
        f->mode |= SF_PEEK | SF_GETR;
        f->endr = f->data;
    }

    SFMTXRETURN(f, (char *)us)
}
