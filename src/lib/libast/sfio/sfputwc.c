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

#include <stdlib.h>

#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"

#include "ast.h"

/*      Write out a rune (wide char) as a multibyte char on f.
**
**      Written by Kiem-Phong Vo.
*/

/*
 * we use an almost empty discipline to keep the stream mb state
 * the discpline is identified by the private _sfmbexcept address
 */

typedef struct Sfmbstate_s {
    Sfdisc_t disc;
    Mbstate_t mbs;
} Sfmbstate_t;

static_fn int _sfmbexcept(Sfio_t *f, int type, void *arg, Sfdisc_t *disc) {
    UNUSED(f);
    UNUSED(arg);

    if (type == SF_DPOP || type == SF_FINAL) free(disc);
    return 0;
}

Mbstate_t *_sfmbstate(Sfio_t *f) {
    Sfdisc_t *disc;
    Sfmbstate_t *mbs;

    for (disc = f->disc; disc; disc = disc->disc) {
        if (disc->exceptf == _sfmbexcept) return &((Sfmbstate_t *)disc)->mbs;
    }
    mbs = calloc(1, sizeof(Sfmbstate_t));
    if (mbs) {
        mbs->disc.exceptf = _sfmbexcept;
        sfdisc(f, &mbs->disc);
    }
    return &mbs->mbs;
}

int sfputwc(Sfio_t *f, int w) {
    uchar *s = NULL;
    char *b;
    int n, m;
    char buf[32];
    SFMTXDECL(f)

    SFMTXENTER(f, -1)

    if (f->mode != SF_WRITE && _sfmode(f, SF_WRITE, 0) < 0) SFMTXRETURN(f, -1)
    SFLOCK(f, 0)

    n = mbconv(buf, w, SFMBSTATE(f));

    if (n > 8 || SFWPEEK(f, s, m) < n) {
        n = SFWRITE(f, (void *)s, n); /* write the hard way */
    } else {
        b = buf;
        switch (n) {
            case 8:
                *s++ = *b++;
            // FALLTHRU
            case 7:
                *s++ = *b++;
            // FALLTHRU
            case 6:
                *s++ = *b++;
            // FALLTHRU
            case 5:
                *s++ = *b++;
            // FALLTHRU
            case 4:
                *s++ = *b++;
            // FALLTHRU
            case 3:
                *s++ = *b++;
            // FALLTHRU
            case 2:
                *s++ = *b++;
            // FALLTHRU
            case 1:
                *s++ = *b++;
            // FALLTHRU
            default:;  // EMPTY BLOCK
        }
        f->next = s;
    }

    SFOPEN(f)
    SFMTXRETURN(f, (int)n)
}
