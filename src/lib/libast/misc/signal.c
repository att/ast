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
//
// Signal function that disables syscall restart on interrupt except for a handful of signals. It
// also puts a clear signal mask in effect when the signal is delivered. If fun == SIG_DFL it also
// unblocks the signal.
//
#include "config_ast.h"  // IWYU pragma: keep

#include "ast.h"
#include "sig.h"

Sig_handler_t signal(int sig, Sig_handler_t sigfun) {
    struct sigaction na, oa;

    na.sa_handler = sigfun;
    sigemptyset(&na.sa_mask);
    switch (sig) {
        case -1:  // just in case none of the following symbols are defined, not likely but...
#if defined(SIGIO)
        case SIGIO:
#endif
#if defined(SIGTSTP)
        case SIGTSTP:
#endif
#if defined(SIGTTIN)
        case SIGTTIN:
        case SIGTTOU:
#endif
            na.sa_flags = SA_RESTART;
            break;
        default:
#if defined(SA_INTERRUPT)
            // Some systems (e.g., Linux) automatically restart syscalls.
            // If we're on such a system tell it not to do that.
            na.sa_flags = SA_INTERRUPT;
#endif
            break;
    }

    if (sigaction(sig, &na, &oa)) return NULL;
    if (sigfun == SIG_DFL) sigunblock(sig);
    return oa.sa_handler;
}
