/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1999-2013 AT&T Intellectual Property          *
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
 *                                                                      *
 ***********************************************************************/
#include "config_ast.h"  // IWYU pragma: keep

#include <stddef.h>

#include "cdt.h"
#include "dttest.h"
#include "terror.h"

/* test to see if the Dtuser_t structure has the right elements */

Dtdisc_t Disc = {0, sizeof(long), -1, newint, NULL, compare, hashint, NULL, NULL};

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    Dt_t *dt;

    if (!(dt = dtopen(&Disc, Dtset))) terror("Opening Dtset");

    if (dtuserlock(dt) != 0) terror("dtuserlock() should have succeeded to lock");
    if (dtuserunlock(dt) != 0) terror("dtuserlock() should have succeeded to unlock");

    if (dtuserdata(dt, (void *)11, 1)) terror("dtuserdata() should have returned NULL");
    if (dt->user->data != (void *)11) terror("user->data should be 11");

    if (dtuserdata(dt, NULL, 0) != (void *)11) terror("dtuserdata() should have returned 11");

    if (dtuserdata(dt, (void *)22, 1) != (void *)11) terror("dtuserdata() should have returned 11");
    if (dt->user->data != (void *)22) terror("user->data should be 22");

    texit(0);
}
