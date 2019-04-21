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
#include <string.h>

#include "ast_assert.h"
#include "cdt.h"
#include "cdtlib.h"

/*      Close a dictionary
**
**      Written by Kiem-Phong Vo, phongvo@gmail.com (5/25/96) (11/15/2010)
*/
int dtclose(Dt_t *dt) {
    int ev, type;
    Dt_t pdt;

    if (!dt || dt->nview > 0) { /* can't close if being viewed */
        return -1;
    }

    Dtdisc_t *disc = dt->disc;
    if (disc && disc->eventf) { /* announce closing event */
        ev = (*disc->eventf)(dt, DT_CLOSE, (void *)1, disc);
    } else {
        ev = 0;
    }
    if (ev < 0) { /* cannot close */
        return -1;
    }

    if (dt->view) { /* turn off viewing at this point */
        dtview(dt, NULL);
    }

    type = dt->data->type; /* save before memory is freed */
    memcpy(&pdt, dt, sizeof(Dt_t));

    if (ev == 0) /* release all allocated data */
    {
        (void)(*(dt->meth->searchf))(dt, NULL, DT_CLEAR);
        (void)(*dt->meth->eventf)(dt, DT_CLOSE, NULL);
        assert(!dt->data);
    }
    if (!(type & DT_INDATA)) (void)free(dt);

    if (disc && disc->eventf) { /* announce end of closing activities */
        (void)(*disc->eventf)(&pdt, DT_ENDCLOSE, NULL, disc);
    }

    return 0;
}
