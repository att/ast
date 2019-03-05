/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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

static char Space[16 * 1024 * 1024];
static char *Current = &Space[0];
static int Close = 0;
static int Free = 0;

static int event(Dt_t *dt, int type, void *obj, Dtdisc_t *disc) {
    UNUSED(dt);
    UNUSED(disc);

    if (type == DT_OPEN) { /* opening first dictionary */
        if (obj) {
            if (Current == &Space[0]) return 0;
            // opening a dictionary sharing with some previous one
            *((void **)obj) = (void *)(&Space[0]);
            return 1;
        }
        return 0;
    } else if (type == DT_CLOSE) {
        if (Close == 0) return 1;  // do not free objects
        return 0;
    }
    return 0;
}

static void *memory(Dt_t *dt, void *buf, size_t size, Dtdisc_t *disc) {
    UNUSED(dt);
    UNUSED(disc);

    if (!buf) {
        size = ((size + sizeof(void *) - 1) / sizeof(void *)) * sizeof(void *);
        buf = (void *)Current;
        Current += size;
    } else {
        if (Close > 0) Free += 1;
    }
    return buf;
}

Dtdisc_t Disc = {0, sizeof(long), -1, newint, NULL, compare, hashint, memory, event};

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    Dt_t *dt1, *dt2;
    long i, k;

    if (!(dt1 = dtopen(&Disc, Dtoset))) terror("Opening Dtoset1");
    if ((long)dtinsert(dt1, 1L) != 1) terror("Inserting 1");
    if ((long)dtinsert(dt1, 3L) != 3) terror("Inserting 3");
    if ((long)dtinsert(dt1, 5L) != 5) terror("Inserting 5");

    if (!(dt2 = dtopen(&Disc, Dtoset))) terror("Opening Dtoset2");
    if ((long)dtinsert(dt2, 2L) != 2) terror("Inserting 2");
    if ((long)dtinsert(dt2, 4L) != 4) terror("Inserting 4");
    if ((long)dtinsert(dt2, 6L) != 6) terror("Inserting 6");

    for (i = 1; i <= 6; ++i) {
        if ((long)dtsearch(dt1, i) != i) terror("Didn't find a long");
    }

    for (i = (long)dtlast(dt2), k = 6; i != 0; i = (long)dtprev(dt2, i), k -= 1) {
        if (i != k) terror("Didn't walk a long");
    }

    /* this test makes sure that dtclose() does not free objects */
    Close = 0;
    dtclose(dt1);
    if (Free > 0) terror("Memory should not have been freed");

    /* this test makes sure that all objects are freed */
    Close = 1;
    dtclose(dt2);
    if (Free <= 0) terror("Memory should have been freed");

    /* test to make sure that shared dictionaries use the same method */
    Current = &Space[0];
    if (!(dt1 = dtopen(&Disc, Dtrhset))) terror("Opening first dictionary");
    if ((dt2 = dtopen(&Disc, Dtset))) terror("This open should have failed");

    texit(0);
}
