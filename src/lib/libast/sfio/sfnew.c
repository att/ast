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

#include <stdlib.h>

#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"
#include "vthread.h"

/*      Fundamental function to create a new stream.
**      The argument flags defines the type of stream and the scheme
**      of buffering.
**
**      Written by Kiem-Phong Vo.
*/

Sfio_t *sfnew(Sfio_t *oldf, void *buf, size_t size, int file, int flags) {
    SFONCE()  // initialize mutexes

    if (!(flags & SF_RDWR)) return NULL;

    Sfio_t *f = oldf;
    int sflags = 0;

    if (f) {
        if (flags & SF_EOF) {
            if (f != sfstdin && f != sfstdout && f != sfstderr) f->mutex = NULL;
            SFCLEAR(f, f->mutex);
            // Is this supposed to be `f = NULL;` ?
            // oldf = NULL;
        } else if (f->mode & SF_AVAIL) { /* only allow SF_STATIC to be already closed */
            if (!(f->flags & SF_STATIC)) return NULL;
            sflags = f->flags;
            // Is this supposed to be `f = NULL;` ?
            // oldf = NULL;
        } else { /* reopening an open stream, close it first */
            sflags = f->flags;

            if (((f->mode & SF_RDWR) != f->mode && _sfmode(f, 0, 0) < 0) || SFCLOSE(f) < 0) {
                return NULL;
            }

            if (f->data && ((flags & SF_STRING) || size != (size_t)SF_UNBOUND)) {
                if (sflags & SF_MALLOC) free(f->data);
                f->data = NULL;
            }
            if (!f->data) sflags &= ~SF_MALLOC;
        }
    }

    if (!f) {  // reuse a standard stream structure if possible
        if (!(flags & SF_STRING) && file >= 0 && file <= 2) {
            f = file == 0 ? sfstdin : file == 1 ? sfstdout : sfstderr;
            if (f) {
                if (f->mode & SF_AVAIL) {
                    sflags = f->flags;
                    SFCLEAR(f, f->mutex);
                } else {
                    f = NULL;
                }
            }
        }

        if (!f) {
            f = malloc(sizeof(Sfio_t));
            if (!f) return NULL;
            SFCLEAR(f, NULL);
        }
    }

    /* create a mutex */
    if (!f->mutex) f->mutex = vtmtxopen(NULL, VT_INIT);

    /* stream type */
    f->mode = (flags & SF_READ) ? SF_READ : SF_WRITE;
    f->flags = (flags & SFIO_FLAGS) | (sflags & (SF_MALLOC | SF_STATIC));
    f->bits = (flags & SF_RDWR) == SF_RDWR ? SF_BOTH : 0;
    f->file = file;
    f->here = f->extent = 0;
    f->getr = f->tiny[0] = 0;

    f->mode |= SF_INIT;
    if (size != (size_t)SF_UNBOUND) {
        f->size = size;
        f->data = size <= 0 ? NULL : (uchar *)buf;
    }
    f->endb = f->endr = f->endw = f->next = f->data;

    if (_Sfnotify) (*_Sfnotify)(f, SF_NEW, (void *)((long)f->file));

    if (f->flags & SF_STRING) (void)_sfmode(f, f->mode & SF_RDWR, 0);

    return f;
}
