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

#include <errno.h>
#include <stdlib.h>

#include "sfhdr.h"

#include "sfdisc.h"

/*	Make a stream op return immediately on interrupts.
**	This is useful on slow streams (hence the name).
**
**	Written by Glenn Fowler (03/18/1998).
*/

static_fn int sfdc_slowexcept(Sfio_t *f, int type, void *v, Sfdisc_t *disc) {
    NOTUSED(f);
    NOTUSED(v);
    NOTUSED(disc);

    switch (type) {
        case SF_FINAL:
        case SF_DPOP:
            free(disc);
            break;
        case SF_READ:
        case SF_WRITE:
            if (errno == EINTR) return (-1);
            break;
    }

    return (0);
}

int sfdcslow(Sfio_t *f) {
    Sfdisc_t *disc;

    if (!(disc = (Sfdisc_t *)malloc(sizeof(Sfdisc_t)))) return (-1);

    disc->readf = NULL;
    disc->writef = NULL;
    disc->seekf = NULL;
    disc->exceptf = sfdc_slowexcept;

    if (sfdisc(f, disc) != disc) {
        free(disc);
        return (-1);
    }
    sfset(f, SF_IOINTR, 1);

    return (0);
}
