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

#include "sfio.h"
#include "stk.h"
#include "terror.h"

// TODO: Write better documentation of `stk` related functions and extend these tests
tmain() {
    UNUSED(argc);
    UNUSED(argv);

    Sfio_t *stk;
    stk = stkopen(0);

    char *foo = stkcopy(stk, "foo");
    char *bar = stkcopy(stk, "bar");
    char *baz = stkcopy(stk, "baz");

    if (!stkon(stk, foo) || !stkon(stk, bar) || !stkon(stk, baz)) {
        terror("stkon() failed!");
    }

    // Increase reference count by 1
    if (stklink(stk) != 1) {
        terror("Failed to increase reference count");
    }

    stkset(stk, bar, 0);
    if (stkfreeze(stk, 0) != bar) {
        terror("Failed");
    }

    stkset(stk, baz, 0);
    if (stkfreeze(stk, 0) != baz) {
        terror("Failed");
    }

    // This should reset stack back to foo
    stkset(stk, "sfsfd", 0);
    if (stkfreeze(stk, 0) != foo) {
        terror("Failed");
    }

    // This should decrease reference count by 1
    if (stkclose(stk) != 1) {
        terror("stkclose() should return 1!");
    }

    // This should set reference count to 0 and try to close stream
    if (stkclose(stk) != -1) {
        terror("stkclose() should return -1!");
    }

    texit(0);
}
