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
#include <unistd.h>

#include "sfio.h"
#include "terror.h"

ssize_t myread(Sfio_t *f, void *buf, size_t n, Sfdisc_t *disc) { return sfrd(f, buf, n, disc); }

Sfdisc_t Disc = {.readf = myread};

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    int fd[2];

    if (pipe(fd) < 0) terror("Can't open pipe");

    if (sfnew(sfstdin, NULL, (size_t)SF_UNBOUND, fd[0], SF_READ) != sfstdin) {
        terror("Can't initialize sfstdin");
    }
    sfset(sfstdin, SF_SHARE, 1);
    sfdisc(sfstdin, &Disc);

    if (sfnew(sfstdout, NULL, 0, fd[1], SF_WRITE) != sfstdout) terror("Can't initialize sfstdout");
    sfputr(sfstdout, "111\n222\n333\n", -1);
    sfsync(sfstdout);

    if (strcmp(sfgetr(sfstdin, '\n', 1), "111") != 0) terror("sfgetr failed1");
    if (sfstdin->endb > sfstdin->next) terror("sfgetr reads too much1");

    if (strcmp(sfgetr(sfstdin, '\n', 1), "222") != 0) terror("sfgetr failed2");
    if (sfstdin->endb > sfstdin->next) terror("sfgetr reads too much2");

    texit(0);
}
