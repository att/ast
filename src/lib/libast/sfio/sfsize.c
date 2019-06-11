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

#include <stdio.h>
#include <sys/stat.h>

#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"

/*      Get the size of a stream.
**
**      Written by Kiem-Phong Vo.
*/
Sfoff_t sfsize(Sfio_t *f) {
    Sfdisc_t *disc;
    int mode;
    Sfoff_t s;
    SFMTXDECL(f)

    SFMTXENTER(f, (Sfoff_t)(-1))

    if ((mode = f->mode & SF_RDWR) != (int)f->mode && _sfmode(f, mode, 0) < 0)
        SFMTXRETURN(f, (Sfoff_t)-1)

    if (f->flags & SF_STRING) {
        SFSTRSIZE(f);
        SFMTXRETURN(f, f->extent)
    }

    SFLOCK(f, 0)

    s = f->here;

    if (f->extent >= 0) {
        if (f->flags & (SF_SHARE | SF_APPENDWR)) {
            for (disc = f->disc; disc; disc = disc->disc) {
                if (disc->seekf) break;
            }
            if (disc) {
                Sfoff_t e;
                if ((e = SFSK(f, 0, SEEK_END, disc)) >= 0) f->extent = e;
                if (SFSK(f, f->here, SEEK_SET, disc) != f->here) {
                    f->here = SFSK(f, (Sfoff_t)0, SEEK_CUR, disc);
                }
            } else {
                struct stat st;
                if (fstat(f->file, &st) < 0) {
                    f->extent = -1;
                } else if ((f->extent = st.st_size) < f->here) {
                    f->here = SFSK(f, (Sfoff_t)0, SEEK_CUR, disc);
                }
            }
        }

        if ((f->flags & (SF_SHARE | SF_PUBLIC)) == (SF_SHARE | SF_PUBLIC)) {
            f->here = SFSK(f, (Sfoff_t)0, SEEK_CUR, f->disc);
        }
    }

    if (f->here != s && (f->mode & SF_READ)) { /* buffered data is known to be invalid */
        if ((f->bits & SF_MMAP) && f->data) {
            SFMUNMAP(f, f->data, f->endb - f->data);
            f->data = NULL;
        }
        f->next = f->endb = f->endr = f->endw = f->data;
    }

    if (f->here < 0) {
        f->extent = -1;
    } else if (f->extent < f->here) {
        f->extent = f->here;
    }

    if ((s = f->extent) >= 0) {
        if (f->flags & SF_APPENDWR) {
            s += (f->next - f->data);
        } else if (f->mode & SF_WRITE) {
            s = f->here + (f->next - f->data);
            if (s < f->extent) s = f->extent;
        }
    }

    SFOPEN(f)
    SFMTXRETURN(f, s)
}
