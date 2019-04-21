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

#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"

/*      Push/pop streams
**
**      Written by Kiem-Phong Vo.
*/

Sfio_t *sfstack(Sfio_t *f1, Sfio_t *f2) {
    int n;
    Sfio_t *rf;
    Sfrsrv_t *rsrv;
    void *mtx;

    SFMTXLOCK(f1)
    SFMTXLOCK(f2)

    if (f1 && (f1->mode & SF_RDWR) != f1->mode && _sfmode(f1, 0, 0) < 0) {
        SFMTXUNLOCK(f1)
        SFMTXUNLOCK(f2)
        return NULL;
    }
    if (f2 && (f2->mode & SF_RDWR) != f2->mode && _sfmode(f2, 0, 0) < 0) {
        SFMTXUNLOCK(f1)
        SFMTXUNLOCK(f2)
        return NULL;
    }
    if (!f1) {
        SFMTXUNLOCK(f2)
        return f2;
    }

    /* give access to other internal functions */
    _Sfstack = sfstack;

    if (f2 == SF_POPSTACK) {
        f2 = f1->push;
        if (!f2) {
            SFMTXUNLOCK(f1)
            return NULL;
        }
        f2->mode &= ~SF_PUSH;
    } else {
        if (f2->push) {
            SFMTXUNLOCK(f1)
            SFMTXUNLOCK(f2)
            return NULL;
        }
        if (f1->pool && f1->pool != &_Sfpool && f1->pool != f2->pool &&
            f1 == f1->pool->sf[0]) { /* get something else to pool front since f1 will be locked */
            for (n = 1; n < f1->pool->n_sf; ++n) {
                if (!SFFROZEN(f1->pool->sf[n])) {
                    (*_Sfpmove)(f1->pool->sf[n], 0);
                    break;
                }
            }
        }
    }

    if (f2->pool && f2->pool != &_Sfpool && f2 != f2->pool->sf[0]) (*_Sfpmove)(f2, 0);

    /* swap streams */
    sfswap(f1, f2);

    /* but the reserved buffer and mutex must remain the same */
    rsrv = f1->rsrv;
    f1->rsrv = f2->rsrv;
    f2->rsrv = rsrv;
    mtx = f1->mutex;
    f1->mutex = f2->mutex;
    f2->mutex = mtx;

    SFLOCK(f1, 0)
    SFLOCK(f2, 0)

    if (f2->push != f2) { /* freeze the pushed stream */
        f2->mode |= SF_PUSH;
        f1->push = f2;
        rf = f1;
    } else { /* unfreeze the just exposed stream */
        f1->mode &= ~SF_PUSH;
        f2->push = NULL;
        rf = f2;
    }

    SFOPEN(f1)
    SFOPEN(f2)

    SFMTXUNLOCK(f1)
    SFMTXUNLOCK(f2)
    return rf;
}
