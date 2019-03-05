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

#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "sfio.h"
#include "terror.h"

static int Fd[2];

void alarmhandler(int sig) {
    UNUSED(sig);

    if (write(Fd[1], "01234\n56789\n", 12) != 12) terror("Writing to pipe");
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    char *s;
    char buf[1024];
    int n;

    if (pipe(Fd) < 0) terror("Can't make pipe");

    if (sfnew(sfstdin, NULL, (size_t)SF_UNBOUND, Fd[0], SF_READ) != sfstdin) {
        terror("Can't renew stdin");
    }
    sfset(sfstdin, SF_SHARE, 1);

    if (sfpkrd(Fd[0], (void *)buf, 10, -1, 1000, 1) >= 0) terror("There isn't any data yet");

    if ((n = sfpkrd(Fd[0], (void *)buf, sizeof(buf), -1, 0L, 0)) >= 0) {
        terror("Wrong data size %d, expecting < 0", n);
    }

    if (write(Fd[1], "abcd", 4) != 4) terror("Couldn't write to pipe");

    if ((n = sfpkrd(Fd[0], (void *)buf, sizeof(buf), -1, 0L, 0)) != 4) {
        terror("Wrong data size %d, expecting 4", n);
    }

    signal(SIGALRM, alarmhandler);
    alarm(2);
    if (!(s = sfgetr(sfstdin, '\n', 1)) || strcmp(s, "01234") != 0) terror("Expecting 01234");

    if (sfstdin->next < sfstdin->endb) terror("Sfgetr read too much");

    if (!(s = sfgetr(sfstdin, '\n', 1)) || strcmp(s, "56789") != 0) terror("Expecting 56789");

    texit(0);
}
