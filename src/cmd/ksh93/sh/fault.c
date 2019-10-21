/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2014 AT&T Intellectual Property          *
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
 *                    David Korn <dgkorn@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
//
// Fault handling routines
//
//   David Korn
//   AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#include "ast.h"
#include "ast_aso.h"
#include "ast_assert.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "fcin.h"
#include "jobs.h"
#include "name.h"
#include "sfio.h"
#include "shcmd.h"
#include "shlex.h"
#include "shnodes.h"
#include "shtable.h"
#include "stk.h"
#include "terminal.h"
#include "variables.h"

#define abortsig(sig) (sig == SIGABRT || sig == SIGBUS || sig == SIGILL || sig == SIGSEGV)

static char indone;
static int cursig = -1;

// Default function for handling a SIGSEGV. It simply writes a backtrace to
// stderr then terminates the process with prejudice. The primary purpose is to
// help ensure we get some useful info when the shell dies due to dereferencing
// an invalid address.
extern int b_print(int argc, char *argv[], Shbltin_t *context);
static_fn void handle_sigsegv(int signo, siginfo_t *info, void *context) {
    UNUSED(signo);
    UNUSED(info);
    UNUSED(context);

    dump_backtrace(0);
    abort();
}

static_fn int notify_builtin(Shell_t *shp, int sig) {
    int action = 0;

    if (!shp->bltinfun) return action;
    if (error_info.flags & ERROR_NOTIFY) {
        action = (*shp->bltinfun)(-sig, NULL, NULL);
        sfsync(NULL);
    }
    if (action > 0) return action;
    if (shp->bltindata.notify) shp->bltindata.sigset = 1;
    return action;
}

void set_trapinfo(Shell_t *shp, int sig, siginfo_t *info) {
    if (!info) return;

    siginfo_ll_t *ip = malloc(sizeof(siginfo_ll_t));
    ip->next = NULL;
    memcpy(&ip->info, info, sizeof(*info));

    siginfo_ll_t *jp = shp->siginfo[sig];
    if (!jp) {
        ip->last = ip;
        shp->siginfo[sig] = ip;
    } else {
        ip->last = NULL;
        jp->last->next = ip;
        jp->last = ip;
    }
}

//
// Most signals caught or ignored by the shell come here.
//
void sh_fault(int sig, siginfo_t *info, void *context) {
    UNUSED(context);
    int saved_errno = errno;  // many platforms do not save/restore errno for signal handlers
    Shell_t *shp = sh_getinterp();
    int flag = 0;
    char *trap;
    checkpt_t *pp = shp->jmplist;

    if (sig == SIGABRT) {
        sh_signal(sig, (sh_sigfun_t)(SIG_DFL));
        sh_sigaction(sig, SIG_UNBLOCK);
        kill(getpid(), sig);
    }
    if (sig == SIGSEGV) {
        dump_backtrace(0);
        // The preceding call should call `abort()` which means this shouldn't be reached but
        // be paranoid.
        sh_signal(sig, (sh_sigfun_t)(SIG_DFL));
        sh_sigaction(sig, SIG_UNBLOCK);
        kill(getpid(), sig);
    }
    if (sig == SIGCHLD) sfprintf(sfstdout, "childsig\n");
#ifdef SIGWINCH
    if (sig == SIGWINCH) shp->winch = 1;
#endif  // SIGWINCH
    trap = shp->st.trapcom[sig];
    if (sig == SIGBUS) {
        sh_signal(sig, (sh_sigfun_t)(SIG_DFL));
        sh_sigaction(sig, SIG_UNBLOCK);
        kill(getpid(), sig);
    }
    if (shp->savesig) {
        // Critical region, save and process later.
        if (!(shp->sigflag[sig] & SH_SIGIGNORE)) shp->savesig = sig;
        goto done;
    }
    // Handle ignored signals.
    if (trap && *trap == 0) goto done;
    flag = shp->sigflag[sig] & ~SH_SIGOFF;
    if (!trap) {
        if (sig == SIGINT && (shp->trapnote & SH_SIGIGNORE)) goto done;
        if (flag & SH_SIGIGNORE) {
            if (shp->subshell) shp->ignsig = sig;
            goto done;
        }
        if (flag & SH_SIGDONE) {
            void *ptr = NULL;

            (void)notify_builtin(shp, sig);
            if ((flag & SH_SIGINTERACTIVE) && sh_isstate(shp, SH_INTERACTIVE) &&
                !sh_isstate(shp, SH_FORKED) && !shp->subshell) {
                // Check for TERM signal between fork/exec.
                if (sig == SIGTERM && job.in_critical) shp->trapnote |= SH_SIGTERM;
                goto done;
            }
            shp->lastsig = sig;
            sh_sigaction(sig, SIG_UNBLOCK);
            if (pp->mode != SH_JMPSUB) {
                if (pp->mode < SH_JMPSUB) {
                    pp->mode = shp->subshell ? SH_JMPSUB : SH_JMPFUN;
                } else {
                    pp->mode = SH_JMPEXIT;
                }
            }
            if (shp->subshell) sh_exit(shp, SH_EXITSIG);
            if (sig == SIGABRT || (abortsig(sig) && (ptr = malloc(1)))) {
                if (ptr) free(ptr);
                sh_done(shp, sig);
            }
            // mark signal and continue.
            shp->trapnote |= SH_SIGSET;
            if (sig < shp->gd->sigmax) shp->sigflag[sig] |= SH_SIGSET;
            goto done;
        }
    }
    errno = 0;
    if (pp->mode == SH_JMPCMD || ((pp->mode == 1 && shp->bltinfun) && !(flag & SH_SIGIGNORE))) {
        shp->lastsig = sig;
    }
    if (trap) {
        // Propogate signal to foreground group.
        set_trapinfo(shp, sig, info);
        if (sig == SIGHUP && job.curpgid) killpg(job.curpgid, SIGHUP);
        flag = SH_SIGTRAP;
    } else {
        shp->lastsig = sig;
        flag = SH_SIGSET;
#ifdef SIGTSTP
        if (sig == SIGTSTP) {
            shp->trapnote |= SH_SIGTSTP;
            if (pp->mode == SH_JMPCMD && sh_isstate(shp, SH_STOPOK)) {
                sh_sigaction(sig, SIG_UNBLOCK);
                sh_exit(shp, SH_EXITSIG);
                goto done;
            }
        }
#endif  // SIGTSTP
    }

    int action = notify_builtin(shp, sig);
    if (action > 0) goto done;
    shp->trapnote |= flag;
    if (sig < shp->gd->sigmax) shp->sigflag[sig] |= flag;
    if (pp->mode == SH_JMPCMD && sh_isstate(shp, SH_STOPOK)) {
        if (action < 0) goto done;
        sh_sigaction(sig, SIG_UNBLOCK);
        sh_exit(shp, SH_EXITSIG);
    }

done:
    errno = saved_errno;
}

//
// Initialize signal handling.
//
void sh_siginit(Shell_t *shp) {
    const struct shtable2 *tp = shtab_signals;

    // Make sure no signals are blocked.
    sigset_t ss;
    sigemptyset(&ss);
    sigprocmask(SIG_SETMASK, &ss, NULL);

    shp->gd->sigmax = MAX_SIGNUM + 1;
    shp->st.trapcom = calloc(shp->gd->sigmax, sizeof(char *));
    shp->sigflag = calloc(shp->gd->sigmax, sizeof(unsigned char));
    shp->gd->sigmsg = calloc(shp->gd->sigmax, sizeof(char *));
    shp->siginfo = calloc(shp->gd->sigmax, sizeof(siginfo_ll_t *));

    for (tp = shtab_signals; tp->sh_number; tp++) {
        int sig = tp->sh_number;
        int n = (sig >> SH_SIGBITS);

        sig &= (1 << SH_SIGBITS) - 1;
        if (sig >= shp->gd->sigmax) continue;
        sig--;
        if (n & SH_SIGRUNTIME) {
            // Coverity CID#253727 caught that the use of `sig` two lines below could theoretically
            // index past the end of `sigruntime[]`. It doesn't only by virtue of the preconditions
            // around `tp->sh_number`. Verify those predconditions are true.
            assert(sig < sizeof(shp->gd->sigruntime) / sizeof(*shp->gd->sigruntime));
            sig = shp->gd->sigruntime[sig];
        }
        if (sig >= 0) {
            shp->sigflag[sig] = n;
            if (tp->sh_name) shp->gd->sigmsg[sig] = tp->sh_value;
        }
    }

    // Make sure we get some useful information if the shell receives a SIGSEGV.
    sh_signal(SIGSEGV, handle_sigsegv);
}

//
// Turn on trap handler for signal <sig>.
//
void sh_sigtrap(Shell_t *shp, int sig) {
    int flag;
    sh_sigfun_t fun;
    shp->st.otrapcom = NULL;
    if (sig == 0) {
        sh_sigdone(shp);
    } else if (!((flag = shp->sigflag[sig]) & (SH_SIGFAULT | SH_SIGOFF))) {
        // Don't set signal if already set or off by parent.
        fun = sh_signal(sig, sh_fault);
        if (fun == (sh_sigfun_t)(SIG_IGN)) {
            sh_signal(sig, (sh_sigfun_t)(SIG_IGN));
            flag |= SH_SIGOFF;
        } else {
            flag |= SH_SIGFAULT;
            if (sig == SIGALRM && fun != (sh_sigfun_t)(SIG_DFL) && fun != sh_fault) {
                sh_signal(sig, fun);
            }
        }
        flag &= ~(SH_SIGSET | SH_SIGTRAP);
        shp->sigflag[sig] = flag;
    }
}

//
// Set signal handler so sh_done is called for all caught signals.
//
void sh_sigdone(Shell_t *shp) {
    int flag, sig = shgd->sigmax;
    shp->sigflag[0] |= SH_SIGFAULT;
    while (--sig >= 0) {
        flag = shp->sigflag[sig];
        if ((flag & (SH_SIGDONE | SH_SIGIGNORE | SH_SIGINTERACTIVE)) &&
            !(flag & (SH_SIGFAULT | SH_SIGOFF))) {
            sh_sigtrap(shp, sig);
        }
    }
}

//
// Restore to default signals. Free the trap strings if mode is non-zero. If
// mode>1 then ignored traps cause signal to be ignored.
//
void sh_sigreset(Shell_t *shp, int mode) {
    char *trap;
#ifdef SIGRTMIN
    int flag, sig = SIGRTMIN;
#else
    int flag, sig = shp->st.trapmax;
#endif

    while (sig-- > 0) {
        trap = shp->st.trapcom[sig];
        if (trap) {
            flag = shp->sigflag[sig] & ~(SH_SIGTRAP | SH_SIGSET);
            if (*trap) {
                if (mode) free(trap);
                shp->st.trapcom[sig] = 0;
            } else if (sig && mode > 1) {
                if (sig != SIGCHLD) sh_signal(sig, (sh_sigfun_t)(SIG_IGN));
                flag &= ~SH_SIGFAULT;
                flag |= SH_SIGOFF;
            }
            shp->sigflag[sig] = flag;
        }
    }
    for (sig = SH_DEBUGTRAP - 1; sig >= 0; sig--) {
        trap = shp->st.trap[sig];
        if (trap) {
            if (mode) free(trap);
            shp->st.trap[sig] = 0;
        }
    }
    shp->st.trapcom[0] = 0;
    if (mode) shp->st.trapmax = 0;
    shp->trapnote = 0;
}

//
// Free up trap if set and restore signal handler if modified.
//
void sh_sigclear(Shell_t *shp, int sig) {
    int flag = shp->sigflag[sig];
    char *trap;

    shp->st.otrapcom = NULL;
    if (!(flag & SH_SIGFAULT)) return;
    flag &= ~(SH_SIGTRAP | SH_SIGSET);
    trap = shp->st.trapcom[sig];
    if (trap) {
        if (!shp->subshell) free(trap);
        shp->st.trapcom[sig] = 0;
    }
    shp->sigflag[sig] = flag;
}

//
// Check for traps.
//
void sh_chktrap(Shell_t *shp) {
    int sig = shp->st.trapmax;
    char *trap;
    int count = 0;
    if (!(shp->trapnote & ~SH_SIGIGNORE)) sig = 0;
    if (sh.intrap) return;
    shp->trapnote &= ~SH_SIGTRAP;
    if (shp->winch) {
        int rows = 0, cols = 0;
        int32_t v;
        astwinsize(2, &rows, &cols);
        v = cols;
        if (v) nv_putval(VAR_COLUMNS, (char *)&v, NV_INT32 | NV_RDONLY);
        v = rows;
        if (v) nv_putval(VAR_LINES, (char *)&v, NV_INT32 | NV_RDONLY);
        shp->winch = 0;
    }

    // Execute errexit trap first.
    if (sh_isstate(shp, SH_ERREXIT) && shp->exitval) {
        int sav_trapnote = shp->trapnote;
        shp->trapnote &= ~SH_SIGSET;
        if (shp->st.trap[SH_ERRTRAP]) {
            trap = shp->st.trap[SH_ERRTRAP];
            shp->st.trap[SH_ERRTRAP] = 0;
            sh_trap(shp, trap, 0);
            shp->st.trap[SH_ERRTRAP] = trap;
        }
        shp->trapnote = sav_trapnote;
        if (sh_isoption(shp, SH_ERREXIT)) {
            shp->jmplist->mode = SH_JMPEXIT;
            sh_exit(shp, shp->exitval);
        }
    }
    if (!shp->sigflag) return;
    while (--sig >= 0) {
        if (sig == cursig) continue;
        if ((shp->sigflag[sig] & SH_SIGTRAP) || (shp->siginfo && shp->siginfo[sig])) {
            shp->sigflag[sig] &= ~SH_SIGTRAP;
            if (sig == SIGCHLD) {
                job_chldtrap(shp, shp->st.trapcom[SIGCHLD], 1);
                continue;
            }
            trap = shp->st.trapcom[sig];
            if (trap) {
                siginfo_ll_t *ip = NULL;
                siginfo_ll_t *ipnext;
            retry:
                if (shp->siginfo) {
                    do {
                        ip = shp->siginfo[sig];
                    } while (asocasptr(&shp->siginfo[sig], ip, 0) != ip);
                }
            again:
                if (!ip) continue;
                sh_setsiginfo(&ip->info);
                ipnext = ip->next;
                cursig = sig;
                if (trap) sh_trap(shp, trap, 0);
                count++;
                free(ip);
                ip = ipnext;
                if (ip) {
                    // Handling the trap via sh_trap() may unset the trap (e.g., `trap - INT`)
                    // or install a different handler. Either will cause the buffer pointed to
                    // by `trap` to be freed thus invaliding our cached pointer. Ensure we're
                    // using the current trap text when dealing with another instance of the
                    // signal. See https://github.com/att/ast/issues/1272.
                    trap = shp->st.trapcom[sig];
                    goto again;
                }
                if (shp->siginfo[sig]) goto retry;
                cursig = -1;
            }
        }
        if (sig == 1 && count) {
            count = 0;
            sig = shp->st.trapmax;
        }
    }
}

//
// Exit the current scope and jump to an earlier one based on pp->mode.
//
void sh_exit(Shell_t *shp, int xno) {
    Sfio_t *pool;
    int sig = 0;
    checkpt_t *pp = shp->jmplist;

    shp->exitval = xno;
    if (xno == SH_EXITSIG) shp->exitval |= (sig = shp->lastsig);
    if (pp && pp->mode > 1) cursig = -1;
    if (shp->procsub) *shp->procsub = 0;

#ifdef SIGTSTP
    if ((shp->trapnote & SH_SIGTSTP) && job.jobcontrol) {
        // ^Z detected by the shell.
        shp->trapnote = 0;
        shp->sigflag[SIGTSTP] = 0;
        if (!shp->subshell && sh_isstate(shp, SH_MONITOR) && !sh_isstate(shp, SH_STOPOK)) return;
        if (sh_isstate(shp, SH_TIMING)) return;
        // Handles ^Z for shell builtins, subshells, and functs.
        shp->lastsig = 0;
        sh_onstate(shp, SH_MONITOR);
        sh_offstate(shp, SH_STOPOK);
        shp->trapnote = 0;
        shp->forked = 1;
        if (!shp->subshell && (sig = sh_fork(shp, 0, NULL))) {
            job.curpgid = 0;
            job.parent = (pid_t)-1;
            job_wait(sig);
            shp->forked = 0;
            job.parent = 0;
            shp->sigflag[SIGTSTP] = 0;
            // Wait for child to stop.
            shp->exitval = (SH_EXITSIG | SIGTSTP);
            // Return to prompt mode.
            assert(pp);
            pp->mode = SH_JMPERREXIT;
        } else {
            if (shp->subshell) sh_subfork();
            // Child process, put to sleep.
            sh_offstate(shp, SH_STOPOK);
            sh_offstate(shp, SH_MONITOR);
            shp->sigflag[SIGTSTP] = 0;
            // Stop child job.
            killpg(job.curpgid, SIGTSTP);
            // child resumes.
            job_clear(shp);
            shp->exitval = (xno & SH_EXITMASK);
            return;
        }
    }
#endif /* SIGTSTP */

    // Unlock output pool.
    sh_offstate(shp, SH_NOTRACK);
    if (!(pool = sfpool(NULL, shp->outpool, SF_WRITE))) pool = shp->outpool;  // can't happen?
    sfclrlock(pool);
#ifdef SIGPIPE
    if (shp->lastsig == SIGPIPE) sfpurge(pool);
#endif  // SIGPIPE
    sfclrlock(sfstdin);
    if (!pp) sh_done(shp, sig);
    shp->intrace = 0;
    shp->prefix = NULL;
    shp->mktype = NULL;
    if (job.in_critical) job_unlock();
    if (pp->mode == SH_JMPSCRIPT && !pp->prev) sh_done(shp, sig);
    if (pp->mode) siglongjmp(pp->buff, pp->mode);
}

static_fn void array_notify(Namval_t *np, void *data) {
    Namarr_t *ap = nv_arrayptr(np);
    UNUSED(data);

    if (ap && ap->fun) (*ap->fun)(np, 0, ASSOC_OP_FREE);
}

//
// This is the exit routine for the shell.
//
__attribute__((noreturn)) void sh_done(void *ptr, int sig) {
    Shell_t *shp = ptr;
    char *t;
    int savxit = shp->exitval;

    shp->trapnote = 0;
    indone = 1;
    if (sig) savxit = SH_EXITSIG | sig;
    if (shp->userinit) (*shp->userinit)(shp, -1);
    if (shp->st.trapcom && (t = shp->st.trapcom[0])) {
        shp->st.trapcom[0] = 0;  // should free but not long
        shp->oldexit = savxit;
        sh_trap(shp, t, 0);
        savxit = shp->exitval;
    } else {
        // Avoid recursive call for set -e.
        sh_offstate(shp, SH_ERREXIT);
        sh_chktrap(shp);
    }
    if (shp->var_tree) nv_scan(shp->var_tree, array_notify, NULL, NV_ARRAY, NV_ARRAY);
    sh_freeup(shp);
    if (mbwide() || sh_isoption(shp, SH_EMACS) || sh_isoption(shp, SH_VI) ||
        sh_isoption(shp, SH_GMACS)) {
        tty_cooked(-1);
    }
#ifdef JOBS
    if ((sh_isoption(shp, SH_INTERACTIVE) && shp->login_sh) ||
        (!sh_isoption(shp, SH_INTERACTIVE) && (sig == SIGHUP))) {
        job_walk(shp, sfstderr, job_terminate, SIGHUP, NULL);
    }
#endif  // JOBS
    job_close(shp);
    sfsync((Sfio_t *)sfstdin);
    sfsync((Sfio_t *)shp->outpool);
    sfsync((Sfio_t *)sfstdout);
    if ((savxit & SH_EXITMASK) == shp->lastsig) sig = savxit & SH_EXITMASK;
    if (sig) {
        // Generate fault termination code.
#if RLIMIT_CORE != RLIMIT_UNKNOWN
        struct rlimit rlp;
        getrlimit(RLIMIT_CORE, &rlp);
        rlp.rlim_cur = 0;
        setrlimit(RLIMIT_CORE, &rlp);
#endif
        sh_signal(sig, (sh_sigfun_t)(SIG_DFL));
        sh_sigaction(sig, SIG_UNBLOCK);
        kill(getpid(), sig);
        pause();
    }
    if (sh_isoption(shp, SH_NOEXEC)) kiaclose(shp->lex_context);
    if (shp->pwdfd >= 0) close(shp->pwdfd);

    // Return POSIX exit code if last process exits due to signal.
    if (savxit & SH_EXITSIG) {
        exit(128 + (savxit & SH_EXITMASK));
    }

    exit(savxit & SH_EXITMASK);
}

//
// Synthesize signal name for sig in buf.
// pfx != 0 prepends SIG to default signal number.
//
static_fn char *sig_name(Shell_t *shp, int sig, char *buf, int pfx) {
    int i;

    i = 0;
    if (sig > shp->gd->sigruntime[SH_SIGRTMIN] && sig < shp->gd->sigruntime[SH_SIGRTMAX]) {
        buf[i++] = 'R';
        buf[i++] = 'T';
        buf[i++] = 'M';
        if (sig > shp->gd->sigruntime[SH_SIGRTMIN] +
                      (shp->gd->sigruntime[SH_SIGRTMAX] - shp->gd->sigruntime[SH_SIGRTMIN]) / 2) {
            buf[i++] = 'A';
            buf[i++] = 'X';
            buf[i++] = '-';
            sig = shp->gd->sigruntime[SH_SIGRTMAX] - sig;
        } else {
            buf[i++] = 'I';
            buf[i++] = 'N';
            buf[i++] = '+';
            sig = sig - shp->gd->sigruntime[SH_SIGRTMIN];
        }
    } else if (pfx) {
        buf[i++] = 'S';
        buf[i++] = 'I';
        buf[i++] = 'G';
    }
    i += sfsprintf(buf + i, 8, "%d", sig);
    buf[i] = 0;
    return buf;
}

static const char trapfmt[] = "trap -- %s %s\n";
//
// If <flag> is positive, then print signal name corresponding to <flag>.
// If <flag> is zero, then print all signal names.
// If <flag> is -1, then print all signal names in menu format.
// If <flag> is <-1, then print all traps.
//
void sh_siglist(Shell_t *shp, Sfio_t *iop, int flag) {
    const struct shtable2 *tp;
    int sig;
    char *sname;
    char name[10];
    const char *names[SH_TRAP];
    const char *traps[SH_DEBUGTRAP + 1];
    tp = shtab_signals;
    if (flag <= 0) {
        // Not all signals may be defined, so initialize.
        for (sig = shp->gd->sigmax - 1; sig >= 0; sig--) names[sig] = 0;
        for (sig = SH_DEBUGTRAP; sig >= 0; sig--) traps[sig] = 0;
    }
    for (; tp->sh_name; tp++) {
        sig = tp->sh_number & ((1 << SH_SIGBITS) - 1);
        if (((tp->sh_number >> SH_SIGBITS) & SH_SIGRUNTIME) &&
            (sig = shp->gd->sigruntime[sig - 1] + 1) == 1) {
            continue;
        }
        if (sig == flag) {
            sfprintf(iop, "%s\n", tp->sh_name);
            return;
        } else if (sig & SH_TRAP) {
            traps[sig & ~SH_TRAP] = (char *)tp->sh_name;
        } else if (sig-- && sig < elementsof(names)) {
            names[sig] = (char *)tp->sh_name;
        }
    }
    if (flag > 0) {
        sfputr(iop, sig_name(shp, flag - 1, name, 0), '\n');
    } else if (flag < -1) {
        // Print the traps.
        char *trap, **trapcom;
        sig = shp->st.trapmax;
        // Use parent traps if otrapcom is set (for $(trap).
        trapcom = (shp->st.otrapcom ? shp->st.otrapcom : shp->st.trapcom);
        while (--sig >= 0) {
            if (!(trap = trapcom[sig])) continue;
            if (sig >= shp->gd->sigmax || !(sname = (char *)names[sig])) {
                sname = sig_name(shp, sig, name, 1);
            }
            sfprintf(iop, trapfmt, sh_fmtq(trap), sname);
        }
        for (sig = SH_DEBUGTRAP; sig >= 0; sig--) {
            if (!(trap = shp->st.otrap ? shp->st.otrap[sig] : shp->st.trap[sig])) continue;
            sfprintf(iop, trapfmt, sh_fmtq(trap), traps[sig]);
        }
    } else {
        // Print all the signal names.
        for (sig = 1; sig < shp->gd->sigmax; sig++) {
            if (!(sname = (char *)names[sig])) {
                sname = sig_name(shp, sig, name, 1);
                if (flag) sname = stkcopy(shp->stk, sname);
            }
            if (flag) {
                names[sig] = sname;
            } else {
                sfputr(iop, sname, '\n');
            }
        }
        if (flag) {
            names[sig] = 0;
            sh_menu(shp, iop, shp->gd->sigmax, (char **)names + 1);
        }
    }
}

//
// Parse and execute the given trap string, stream or tree depending on mode
// mode==0 for string, mode==1 for stream, mode==2 for parse tree.
//
int sh_trap(Shell_t *shp, const char *trap, int mode) {
    int jmpval, savxit = shp->exitval;
    int was_history = sh_isstate(shp, SH_HISTORY);
    int was_verbose = sh_isstate(shp, SH_VERBOSE);
    int staktop = stktell(shp->stk);
    char *savptr = stkfreeze(shp->stk, 0);
    char ifstable[256];
    checkpt_t buff;
    Fcin_t savefc;

    fcsave(&savefc);
    memcpy(ifstable, shp->ifstable, sizeof(ifstable));
    sh_offstate(shp, SH_HISTORY);
    sh_offstate(shp, SH_VERBOSE);
    shp->intrap++;
    sh_pushcontext(shp, &buff, SH_JMPTRAP);
    jmpval = sigsetjmp(buff.buff, 0);
    if (jmpval == 0) {
        if (mode == 2) {
            sh_exec(shp, (Shnode_t *)trap, sh_isstate(shp, SH_ERREXIT));
        } else {
            Sfio_t *sp;
            if (mode) {
                sp = (Sfio_t *)trap;
            } else {
                sp = sfopen(NULL, trap, "s");
            }
            sh_eval(shp, sp, 0);
        }
    } else if (indone) {
        if (jmpval == SH_JMPSCRIPT) {
            indone = 0;
        } else {
            if (jmpval == SH_JMPEXIT) savxit = shp->exitval;
            jmpval = SH_JMPTRAP;
        }
    }
    sh_popcontext(shp, &buff);
    shp->intrap--;
    sfsync(shp->outpool);
    if (!shp->indebug && jmpval != SH_JMPEXIT && jmpval != SH_JMPFUN) shp->exitval = savxit;
    stkset(shp->stk, savptr, staktop);
    fcrestore(&savefc);
    memcpy(shp->ifstable, ifstable, sizeof(ifstable));
    if (was_history) sh_onstate(shp, SH_HISTORY);
    if (was_verbose) sh_onstate(shp, SH_VERBOSE);
    exitset(shp);
    if (jmpval > SH_JMPTRAP && (shp->jmpbuffer->prev || shp->jmpbuffer->mode == SH_JMPSCRIPT)) {
        siglongjmp(shp->jmplist->buff, jmpval);
    }
    return shp->exitval;
}

#undef signal
sh_sigfun_t sh_signal(int sig, sh_sigfun_t func) {
    struct sigaction sigin, sigout;

    memset(&sigin, 0, sizeof(struct sigaction));
    if (func == (sh_sigfun_t)(SIG_DFL)) {
        sigin.sa_handler = SIG_DFL;
    } else if (func == (sh_sigfun_t)(SIG_IGN)) {
        sigin.sa_handler = SIG_IGN;
    } else {
        sigin.sa_sigaction = (void (*)(int, siginfo_t *, void *))func;
        sigin.sa_flags = SA_SIGINFO;
        // Delivering a signal results in malloc() being called. If a different signal is delivered
        // while inside malloc() we can deadlock. So only allow one signal at a time with a couple
        // of exceptions. See issue #490.
        sigfillset(&sigin.sa_mask);
        sigdelset(&sigin.sa_mask, SIGABRT);
        sigdelset(&sigin.sa_mask, SIGCHLD);
        sigdelset(&sigin.sa_mask, SIGSEGV);
    }
    sigaction(sig, &sigin, &sigout);
    sh_sigaction(sig, SIG_UNBLOCK);
    return sigout.sa_sigaction;
}

void sh_sigcheck(Shell_t *shp) {
    if (shp->trapnote & SH_SIGSET) sh_exit((shp), SH_EXITSIG);
}

//
// Given the name or number of a signal (as a string) return the numeric signal number.
//
int sig_number(Shell_t *shp, const char *string) {
    const Shtable_t *tp;
    int n, o, sig = 0;
    char *last, *name;

    if (isdigit(*string)) {
        n = (int)strtol(string, &last, 10);
        if (*last) n = -1;
    } else {
        int c;
        o = stktell(shp->stk);
        do {
            c = *string++;
            if (islower(c)) c = toupper(c);
            sfputc(shp->stk, c);
        } while (c);
        stkseek(shp->stk, o);
        if (strncmp(stkptr(shp->stk, o), "SIG", 3) == 0) {
            sig = 1;
            o += 3;
            if (isdigit(*stkptr(shp->stk, o))) {
                n = (int)strtol(stkptr(shp->stk, o), &last, 10);
                if (!*last) return n;
            }
        }
        char *signame = stkptr(shp->stk, o);
        tp = sh_locate(signame, (const Shtable_t *)shtab_signals, sizeof(*shtab_signals));
        n = tp->sh_number;
        if (sig == 1 && (n >= (SH_TRAP - 1) && n < (1 << SH_SIGBITS))) {
            // Sig prefix cannot match internal traps.
            n = 0;
            tp = (Shtable_t *)((char *)tp + sizeof(*shtab_signals));
            if (strcmp(stkptr(shp->stk, o), tp->sh_name) == 0) n = tp->sh_number;
        }
        if ((n >> SH_SIGBITS) & SH_SIGRUNTIME) {
            n = shp->gd->sigruntime[(n & ((1 << SH_SIGBITS) - 1)) - 1];
        } else {
            n &= (1 << SH_SIGBITS) - 1;
            if (n < SH_TRAP) n--;
        }
        if (n < 0 && shp->gd->sigruntime[1] && (name = stkptr(shp->stk, o)) && *name++ == 'R' &&
            *name++ == 'T') {
            if (name[0] == 'M' && name[1] == 'I' && name[2] == 'N' && name[3] == '+') {
                if ((sig = (int)strtol(name + 4, &name, 10)) >= 0 && !*name) {
                    n = shp->gd->sigruntime[SH_SIGRTMIN] + sig;
                }
            } else if (name[0] == 'M' && name[1] == 'A' && name[2] == 'X' && name[3] == '-') {
                if ((sig = (int)strtol(name + 4, &name, 10)) >= 0 && !*name) {
                    n = shp->gd->sigruntime[SH_SIGRTMAX] - sig;
                }
            } else if ((sig = (int)strtol(name, &name, 10)) > 0 && !*name) {
                n = shp->gd->sigruntime[SH_SIGRTMIN] + sig - 1;
            }
            if (n < shp->gd->sigruntime[SH_SIGRTMIN] || n > shp->gd->sigruntime[SH_SIGRTMAX]) {
                n = -1;
            }
        }
    }
    return n;
}
