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
/*
 * Glenn Fowler
 * AT&T Research
 *
 * signal critical region support
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <signal.h>
#include <stddef.h>

#include "ast.h"
#include "sig.h"

static struct {
    int sig;
    int op;
} signals[] = /* held inside critical region    */
    {
        {SIGINT, SIG_REG_EXEC},
#ifdef SIGPIPE
        {SIGPIPE, SIG_REG_EXEC},
#endif
#ifdef SIGQUIT
        {SIGQUIT, SIG_REG_EXEC},
#endif
#ifdef SIGHUP
        {SIGHUP, SIG_REG_EXEC},
#endif
#if defined(SIGCHLD) && (!defined(SIGCLD) || SIGCHLD != SIGCLD)
        {SIGCHLD, SIG_REG_PROC},
#endif
#ifdef SIGTSTP
        {SIGTSTP, SIG_REG_TERM},
#endif
#ifdef SIGTTIN
        {SIGTTIN, SIG_REG_TERM},
#endif
#ifdef SIGTTOU
        {SIGTTOU, SIG_REG_TERM},
#endif
};

/*
 * critical signal region handler
 *
 * op>0         new region according to SIG_REG_*, return region level
 * op==0        pop region, return region level
 * op<0         return non-zero if any signals held in current region
 *
 * signals[] held until region popped
 */

int sigcritical(int op) {
    int i;
    static int region;
    static int level;
    static sigset_t mask;
    sigset_t nmask;

    if (op > 0) {
        if (!level++) {
            region = op;
            sigemptyset(&nmask);
            for (i = 0; i < elementsof(signals); i++) {
                if (op & signals[i].op) sigaddset(&nmask, signals[i].sig);
            }
            sigprocmask(SIG_BLOCK, &nmask, &mask);
        }
        return level;
    } else if (op < 0) {
        sigpending(&nmask);
        for (i = 0; i < elementsof(signals); i++) {
            if (region & signals[i].op) {
                if (sigismember(&nmask, signals[i].sig)) return 1;
            }
        }
        return 0;
    }

    // A fork() may have intervened so we allow apparent nesting mismatches.
    // TODO: The original statement mentioned vfork(), not fork(). Is this still correct since
    // we no longer support vfork()? Does it apply to fork()?
    if (--level <= 0) {
        level = 0;
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }
    return level;
}
