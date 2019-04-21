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

#include <stddef.h>

#include "cdt.h"
#include "cdtlib.h"

/*      Change search method.
**
**      Written by Kiem-Phong Vo, phongvo@gmail.com (5/25/96)
*/

Dtmethod_t *dtmethod(Dt_t *dt, Dtmethod_t *meth) {
    Dtlink_t *list;
    Dtdisc_t *disc = dt->disc;
    Dtmethod_t *oldmt = dt->meth;
    Dtdata_t *newdt, *olddt = dt->data;

    if (!meth || meth == oldmt) return oldmt;

    /* ask discipline if switching to new method is ok */
    if (disc->eventf && (*disc->eventf)(dt, DT_METH, (void *)meth, disc) < 0) return NULL;

    list = dtextract(dt); /* extract elements out of dictionary */

    /* try to create internal structure for new method */
    if (dt->searchf == oldmt->searchf) { /* ie, not viewpathing */
        dt->searchf = meth->searchf;
    }
    dt->meth = meth;
    dt->data = NULL;
    if ((*dt->meth->eventf)(dt, DT_OPEN, NULL) < 0) {
        newdt = NULL;
    } else {
        newdt = dt->data;
    }

    /* see what need to be done to data of the old method */
    if (dt->searchf == meth->searchf) dt->searchf = oldmt->searchf;
    dt->meth = oldmt;
    dt->data = olddt;
    if (newdt) {  // switch was successful, remove old data
        (void)(*dt->meth->eventf)(dt, DT_CLOSE, NULL);

        if (dt->searchf == oldmt->searchf) dt->searchf = meth->searchf;
        dt->meth = meth;
        dt->data = newdt;
        dtrestore(dt, list);
        return oldmt;
    }

    // Switch failed, restore dictionary to previous states.
    dtrestore(dt, list);
    return NULL;
}

/* customize certain actions in a container data structure */
int dtcustomize(Dt_t *dt, int type, int action) {
    int done = 0;

    if ((type & DT_SHARE) &&
        (!dt->meth->eventf || (*dt->meth->eventf)(dt, DT_SHARE, (void *)((long)action)) >= 0)) {
        if (action <= 0) {
            dt->data->type &= ~DT_SHARE;
        } else {
            dt->data->type |= DT_SHARE;
        }
        done |= DT_SHARE;
    }

    if ((type & DT_ANNOUNCE) &&
        (!dt->meth->eventf || (*dt->meth->eventf)(dt, DT_ANNOUNCE, (void *)((long)action)) >= 0)) {
        if (action <= 0) {
            dt->data->type &= ~DT_ANNOUNCE;
        } else {
            dt->data->type |= DT_ANNOUNCE;
        }
        done |= DT_ANNOUNCE;
    }

    if ((type & DT_OPTIMIZE) &&
        (!dt->meth->eventf || (*dt->meth->eventf)(dt, DT_OPTIMIZE, (void *)((long)action)) >= 0)) {
        done |= DT_OPTIMIZE;
    }

    return done;
}
