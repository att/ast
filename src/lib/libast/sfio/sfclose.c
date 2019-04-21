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
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "ast_assert.h"
#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"
#include "vthread.h"

/*      Close a stream. A file stream is synced before closing.
**
**      Written by Kiem-Phong Vo
*/

int sfclose(Sfio_t *f) {
    int local, ex, rv;
    void *data = NULL;
    SFMTXDECL(f)  // declare a local stream variable for multithreading

    SFMTXENTER(f, -1)

    GETLOCAL(f, local);

    if (!(f->mode & SF_INIT) && SFMODE(f, local) != (f->mode & SF_RDWR) &&
        SFMODE(f, local) != (f->mode & (SF_READ | SF_SYNCED)) && _sfmode(f, SF_SYNCED, local) < 0)
        SFMTXRETURN(f, -1)

    /* closing a stack of streams */
    while (f->push) {
        Sfio_t *pop;

        if (!(pop = (*_Sfstack)(f, NULL))) SFMTXRETURN(f, -1)

        if (sfclose(pop) < 0) {
            (*_Sfstack)(f, pop);
            SFMTXRETURN(f, -1)
        }
    }

    rv = 0;
    if (f->disc == _Sfudisc) { /* closing the ungetc stream */
        f->disc = NULL;
    } else if (f->file >= 0) /* sync file pointer */
    {
        f->bits |= SF_ENDING;
        rv = sfsync(f);
    }

    SFLOCK(f, 0)

    /* raise discipline exceptions */
    if (f->disc && (ex = SFRAISE(f, local ? SF_NEW : SF_CLOSING, NULL)) != 0) SFMTXRETURN(f, ex)

    if (!local && f->pool) { /* remove from pool */
        if (f->pool == &_Sfpool) {
            int n;

            POOLMTXLOCK(&_Sfpool);
            for (n = 0; n < _Sfpool.n_sf; ++n) {
                if (_Sfpool.sf[n] == f) {  // found it
                    _Sfpool.n_sf -= 1;
                    for (; n < _Sfpool.n_sf; ++n) _Sfpool.sf[n] = _Sfpool.sf[n + 1];
                    break;
                }
            }
            POOLMTXUNLOCK(&_Sfpool);
        } else {
            f->mode &= ~SF_LOCK; /**/
            assert(_Sfpmove);
            if ((*_Sfpmove)(f, -1) < 0) {
                SFOPEN(f)
                SFMTXRETURN(f, -1)
            }
            f->mode |= SF_LOCK;
        }
        f->pool = NULL;
    }

    if (f->data && (!local || (f->flags & SF_STRING) || (f->bits & SF_MMAP))) { /* free buffer */
        if (f->bits & SF_MMAP) {
            SFMUNMAP(f, f->data, f->endb - f->data);
        } else if (f->flags & SF_MALLOC) {
            data = (void *)f->data;
        }

        f->data = NULL;
        f->size = -1;
    }

    /* zap the file descriptor */
    if (_Sfnotify) (*_Sfnotify)(f, SF_CLOSING, (void *)((long)f->file));
    if (f->file >= 0 && !(f->flags & SF_STRING)) {
        while (close(f->file) < 0) {
            if (errno == EINTR) {
                errno = 0;
            } else {
                rv = -1;
                break;
            }
        }
    }
    f->file = -1;

    SFKILL(f);
    f->flags &= SF_STATIC;
    f->here = 0;
    f->extent = -1;
    f->endb = f->endr = f->endw = f->next = f->data;

    /* zap any associated auxiliary buffer */
    if (f->rsrv) {
        free(f->rsrv);
        f->rsrv = NULL;
    }

    /* delete any associated sfpopen-data */
    if (f->proc) rv = _sfpclose(f);

    /* destroy the mutex */
    if (f->mutex) {
        (void)vtmtxclrlock(f->mutex);
        if (f != sfstdin && f != sfstdout && f != sfstderr) {
            (void)vtmtxclose(f->mutex);
            f->mutex = NULL;
        }
    }

    if (!local) {
        if (f->disc && (ex = SFRAISE(f, SF_FINAL, NULL)) != 0) {
            rv = ex;
            goto done;
        }

        if (!(f->flags & SF_STATIC)) {
            free(f);
        } else {
            f->disc = NULL;
            f->stdio = NULL;
            f->mode = SF_AVAIL;
        }
    }

done:
    if (data) free(data);
    return rv;
}
