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

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "sfio.h"
#include "terror.h"

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    Sfio_t *f;
    Sfio_t sf;

    if (sfopen(sfstdout, "abc", "s") != sfstdout) terror("Bad reopening of sfstdout");
    if (sfopen(sfstdin, "123", "s") != sfstdin) terror("Bad reopening of sfstdin");
    sfclose(sfstdin);

    if (!(f = sfopen(NULL, "123", "s"))) terror("Opening a stream");
        // The next two statements are commented out because the test is bogus. The sfclose() will
        // free the structure pointed to by `f`. Which means it isn't safe to pass that pointer to a
        // subsequent sfopen() call. This works when linked against Vmalloc by accident. It does not
        // work reliably when linked against the system malloc. Furthermore, I believe you can
        // reopen the closed stream if you first call `sfset(f, SF_STATIC, 1)` to keep sfclose()
        // from freeing the structure. And, in fact, if you do so the subsequent sfopen() test
        // fails.
#if 0
    sfclose(f);
    if (sfopen(f, "123", "s")) terror("can't reopen a closed stream!");
#endif

    memset(&sf, 0, sizeof(sf));
    if (sfnew(&sf, NULL, (size_t)SF_UNBOUND, 0, SF_EOF | SF_READ) != &sf) terror("Did not open sf");
    sfset(&sf, SF_STATIC, 1);
    if (!sfclose(&sf) || errno != EBADF) terror("sfclose(sf) should fail with EBADF");
    if (!(sfset(&sf, 0, 0) & SF_STATIC)) terror("Did not close sf");

    /* test for exclusive opens */
    unlink(tstfile("sf", 0));
    if (!(f = sfopen(NULL, tstfile("sf", 0), "wx"))) terror("sfopen failed");
    if ((f = sfopen(f, tstfile("sf", 0), "wx"))) terror("sfopen should not succeed here");

    texit(0);
}
