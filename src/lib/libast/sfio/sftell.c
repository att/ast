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

#include <stdio.h>

#include "sfhdr.h"
#include "sfio.h"

/*      Tell the current location in a given stream
**
**      Written by Kiem-Phong Vo.
*/

Sfoff_t sftell(Sfio_t *f) {
    int mode;
    Sfoff_t p;
    SFMTXDECL(f)

    SFMTXENTER(f, (Sfoff_t)(-1))

    /* set the stream to the right mode */
    if ((mode = f->mode & SF_RDWR) != (int)f->mode && _sfmode(f, mode, 0) < 0)
        SFMTXRETURN(f, (Sfoff_t)-1)

    /* throw away ungetc data */
    if (f->disc == _Sfudisc) (void)sfclose((*_Sfstack)(f, NULL));

    if (f->flags & SF_STRING) SFMTXRETURN(f, (Sfoff_t)(f->next - f->data))

    /* let sfseek() handle the hard case */
    if (f->extent >= 0 && (f->flags & (SF_SHARE | SF_APPENDWR))) {
        p = sfseek(f, (Sfoff_t)0, SEEK_CUR);
    } else {
        p = f->here + ((f->mode & SF_WRITE) ? f->next - f->data : f->next - f->endb);
    }

    SFMTXRETURN(f, p)
}
