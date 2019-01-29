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

#include "sfhdr.h"
#include "sfio.h"

/* Walk streams and run operations on them
**
** Written by Kiem-Phong Vo.
*/

int sfwalk(Sfwalk_f walkf, void *data, int type) {
    Sfpool_t *p;
    Sfio_t *f;
    int n, rv;

    /* truly initializing std-streams before walking */
    if (sfstdin->mode & SF_INIT) _sfmode(sfstdin, (sfstdin->mode & SF_RDWR), 0);
    if (sfstdout->mode & SF_INIT) _sfmode(sfstdout, (sfstdout->mode & SF_RDWR), 0);
    if (sfstderr->mode & SF_INIT) _sfmode(sfstderr, (sfstderr->mode & SF_RDWR), 0);

    for (rv = 0, p = &_Sfpool; p; p = p->next) {
        for (n = 0; n < p->n_sf;) {
            f = p->sf[n];

            if (type != 0 && (f->flags & type) != type) continue; /* not in the interested set */

            if ((rv = (*walkf)(f, data)) < 0) return rv;

            if (p->sf[n] == f) /* move forward to next stream */
                n += 1;
            /* else - a sfclose() was done on current stream */
        }
    }

    return rv;
}
