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
#include <string.h>
#include <unistd.h>

#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"

/*      Resize a stream.
        Written by Kiem-Phong Vo.
*/

int sfresize(Sfio_t *f, Sfoff_t size) {
    SFMTXDECL(f)

    SFMTXENTER(f, -1)

    if (size < 0 || f->extent < 0 || (f->mode != SF_WRITE && _sfmode(f, SF_WRITE, 0) < 0))
        SFMTXRETURN(f, -1)

    SFLOCK(f, 0)

    if (f->flags & SF_STRING) {
        SFSTRSIZE(f);

        if (f->extent >= size) {
            if ((f->flags & SF_MALLOC) && (f->next - f->data) <= size) {
                size_t s = (((size_t)size + 1023) / 1024) * 1024;
                void *d;
                if (s < f->size && (d = realloc(f->data, s))) {
                    f->data = d;
                    f->size = s;
                    f->extent = s;
                }
            }
            memset(f->data + size, 0, f->extent - size);
        } else {
            if (SFSK(f, size, SEEK_SET, f->disc) != size) SFMTXRETURN(f, -1)
            memset(f->data + f->extent, 0, size - f->extent);
        }
    } else {
        if (f->next > f->data) SFSYNC(f);
        if (ftruncate(f->file, (sfoff_t)size) < 0) SFMTXRETURN(f, -1)
    }

    f->extent = size;

    SFOPEN(f)

    SFMTXRETURN(f, 0)
}
