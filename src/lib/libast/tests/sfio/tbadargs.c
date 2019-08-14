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
#include <stdarg.h>
#include <stddef.h>

#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"
#include "terror.h"

int Code_line = 42; /* line number of CALL(sfclose(0)) */

#if defined(__LINE__)
#define CALL(x) ((Code_line = __LINE__), (x))
#else
#define CALL(x) ((Code_line += 1), (x))
#endif

void handler(int sig) {
    UNUSED(sig);

    terror("Bad argument handling on code line %d", Code_line);
}

void main_varargs(int argc, char **argv, ...) {
    UNUSED(argc);
    UNUSED(argv);
    va_list args;

    signal(SIGILL, handler);
    signal(SIGBUS, handler);
    signal(SIGSEGV, handler);

    CALL(sfclose(NULL));
    CALL(sfclrlock(NULL));
    CALL(sfopen(NULL, NULL, NULL));
    CALL(sfdisc(NULL, NULL));
    CALL(_sffilbuf(0, 0));
    CALL(_sfflsbuf(0, 0));
    CALL(sfgetd(NULL));
    CALL(sfgetl(NULL));
    CALL(sfgetm(NULL, 0));
    CALL(sfgetr(NULL, 0, 0));
    CALL(sfgetu(NULL));
    CALL(sfmove(NULL, NULL, 0, 0));
    CALL(sfmutex(NULL, 0));
    CALL(sfnew(NULL, NULL, 0, 0, 0));
    CALL(sfnputc(NULL, 0, 0));
    CALL(sfopen(NULL, NULL, NULL));
    CALL(sfpool(NULL, NULL, 0));
    CALL(sfpopen(NULL, NULL, NULL));
    CALL(sfprintf(NULL, NULL));
    CALL(sfsprintf(NULL, 0, NULL));
    CALL(sfprints(NULL));
    CALL(sfpurge(NULL));
    CALL(sfputd(NULL, 0));
    CALL(sfputl(NULL, 0));
    CALL(sfputm(NULL, 0, 0));
    CALL(sfputr(NULL, NULL, 0));
    CALL(sfputu(NULL, 0));
    CALL(sfraise(NULL, 0, NULL));
    CALL(sfrd(NULL, NULL, 0, NULL));
    CALL(sfread(NULL, NULL, 0));
    CALL(sfreserve(NULL, 0, 0));
    CALL(sfresize(NULL, 0));
    CALL(sfscanf(NULL, NULL));
    CALL(sfsscanf(NULL, NULL));
    CALL(sfseek(NULL, 0, 0));
    CALL(sfset(NULL, 0, 0));
    CALL(sfsetbuf(NULL, NULL, 0));
    CALL(sfsetfd(NULL, 0));
    CALL(sfsize(NULL));
    CALL(sfsk(NULL, 0, 0, NULL));
    CALL(sfstack(NULL, NULL));
    CALL(sfswap(NULL, NULL));
    CALL(sfsync(NULL));
    CALL(sftell(NULL));
    CALL(sftmp(0));
    CALL(sfungetc(NULL, 0));
    CALL(sfwr(NULL, NULL, 0, NULL));
    CALL(sfwrite(NULL, NULL, 0));

    va_start(args, argv);
    CALL(sfvprintf(NULL, NULL, args));
    CALL(sfvscanf(NULL, NULL, args));
    CALL(sfvsprintf(NULL, 0, NULL, args));
    CALL(sfvsscanf(NULL, NULL, args));
    va_end(args);
}

tmain() {
    main_varargs(argc, argv);
    texit(0);
}
