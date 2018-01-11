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
#include "vmhdr.h"

/*	Get statistics of a region.
**
**	Written by Kiem-Phong Vo, phongvo@gmail.com, 01/16/94, 03/31/2012.
*/

int _vmstat(Vmalloc_t *vm, Vmstat_t *st, size_t extra)
{
    Seg_t *seg;
    int rv;

    memset(st, 0, sizeof(Vmstat_t));
    for (seg = vm->data->seg; seg; seg = seg->next) {
        st->n_seg += 1;
        st->extent += seg->size;
    }
    if (!vm->meth.meth)
        rv = -1;
    else if ((rv = (*vm->meth.statf)(vm, st, extra != 0)) >= 0) {
        st->extent += extra;
        debug_sprintf(st->mesg, sizeof(st->mesg),
                      "region %p size=%zu segs=%zu packs=%zu busy=%zu%% cache=%zu/%zu", vm,
                      st->extent, st->n_seg, st->n_pack, (st->s_busy * 100) / st->extent,
                      st->s_cache, st->n_cache);
        st->mode = vm->data->mode;
    }

    return rv;
}

int vmstat(Vmalloc_t *vm, Vmstat_t *st)
{
    if (!st) return _vmheapbusy();
    if (!vm) /* getting stats for Vmregion */
    {
        if (_vmheapinit(Vmheap) != Vmheap) /* initialize heap if not done yet */
            return -1;
        vm = Vmregion;
    }
    return _vmstat(vm, st, 0);
}
