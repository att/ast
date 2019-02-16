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

#include <string.h>

#include "sfio.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    Sfio_t *f1;
    Sfio_t *f2;
    char *s;

    if (!(f1 = sfopen(NULL, tstfile("sf", 0), "w+"))) terror("Can't open file");
    if (sfwrite(f1, "0123456789\n", 11) != 11) terror("Can't write to file");

    sfclose(sfstdin);
    if (sfswap(f1, sfstdin) != sfstdin) terror("Can't swap with sfstdin");
    sfseek(sfstdin, (Sfoff_t)0, 0);
    if (!(s = sfgetr(sfstdin, '\n', 1))) terror("sfgetr failed");
    if (strcmp(s, "0123456789") != 0) terror("Get wrong data");

    if (!(f1 = sfswap(sfstdin, NULL))) terror("Failed swapping to NULL");
    if (!sfstack(sfstdout, f1)) terror("Failed stacking f1");

    if (!(f2 = sfopen(NULL, tstfile("sf", 0), "r"))) terror("Can't open for read");

    if (sfswap(f1, f2)) terror("sfswap should have failed");

    texit(0);
}
