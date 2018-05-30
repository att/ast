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
/*
 * dtopen() with handle placed in specific vm region
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <dt.h>

typedef struct Dc_s {
    Dtdisc_t ndisc;
    Dtdisc_t *odisc;
} Dc_t;

static int eventf(Dt_t *dt, int op, void *data, Dtdisc_t *disc) {
    Dc_t *dc = (Dc_t *)disc;
    int r;

    if (dc->odisc->eventf && (r = (*dc->odisc->eventf)(dt, op, data, dc->odisc))) return r;
    return op == DT_ENDOPEN ? 1 : 0;
}

static void *memoryf(Dt_t *dt, void *addr, size_t size, Dtdisc_t *disc) {
    // This was the original Vmalloc based implementation:
    //   return vmresize(((Dc_t *)disc)->vm, addr, size, VM_RSMOVE);
    // That vmresize() call initializes the new bytes to zero which a simple realloc() won't.
    return realloc(addr, size);
}

/*
 * open a dictionary using disc->memoryf if set or vm otherwise
 */

Dt_t *_dtnew(Dtdisc_t *disc, Dtmethod_t *meth, unsigned long version) {
    Dt_t *dt;
    Dc_t dc;

    dc.odisc = disc;
    dc.ndisc = *disc;
    dc.ndisc.eventf = eventf;
    if (!dc.ndisc.memoryf) dc.ndisc.memoryf = memoryf;
    if (dt = _dtopen(&dc.ndisc, meth, version)) dtdisc(dt, disc, DT_SAMECMP | DT_SAMEHASH);
    return dt;
}

#undef dtnew

Dt_t *dtnew(Dtdisc_t *disc, Dtmethod_t *meth) { return _dtnew(disc, meth, 20050420L); }
