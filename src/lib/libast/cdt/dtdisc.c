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

#include "cdt.h"

/*      Change discipline.
**      dt :    dictionary
**      disc :  discipline
**
**      Written by Kiem-Phong Vo, phongvo@gmail.com (5/25/96)
*/

static_fn void *dtmemory(Dt_t *dt, void *addr, size_t size, Dtdisc_t *disc) {
    UNUSED(dt);
    UNUSED(disc);

    if (addr) {
        if (size == 0) {
            free(addr);
            return NULL;
        }
        return realloc(addr, size);
    }

    return size > 0 ? malloc(size) : NULL;
}

Dtdisc_t *dtdisc(Dt_t *dt, Dtdisc_t *disc, int type) {
    Dtdisc_t *old;
    Dtlink_t *list;

    if (!(old = dt->disc)) /* initialization call from dtopen() */
    {
        dt->disc = disc;
        if (!(dt->memoryf = disc->memoryf)) dt->memoryf = dtmemory;
        return disc;
    }

    if (!disc) { /* only want to know current discipline */
        return old;
    }

    if (old->eventf && (*old->eventf)(dt, DT_DISC, (void *)disc, old) < 0) return NULL;

    if ((type & (DT_SAMEHASH | DT_SAMECMP)) != (DT_SAMEHASH | DT_SAMECMP)) {
        list = dtextract(dt); /* grab the list of objects if any */
    } else {
        list = NULL;
    }

    dt->disc = disc;
    if (!(dt->memoryf = disc->memoryf)) dt->memoryf = dtmemory;

    if (list) { /* reinsert extracted objects (with new discipline) */
        dtrestore(dt, list);
    }

    return old;
}
