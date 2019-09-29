/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2013 AT&T Intellectual Property          *
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
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "defs.h"
#include "error.h"
#include "fault.h"

struct _timer {
    double wakeup;
    double incr;
    struct _timer *next;
    void (*action)(void *);
    void *handle;
};

#define IN_ADDTIMEOUT 1
#define IN_SIGALRM 2
#define DEFER_SIGALRM 4
#define SIGALRM_CALL 8

static Timer_t *tptop, *tpmin, *tpfree;
static char time_state;

static_fn double getnow(void) {
    double now;
    struct timeval tp;
    timeofday(&tp);
    now = tp.tv_sec + 1.e-6 * tp.tv_usec;
    return now + .001;
}

//
// Set an alarm for <t> seconds.
//
static_fn double setalarm(double t) {
    struct itimerval tnew, told;
    tnew.it_value.tv_sec = t;
    tnew.it_value.tv_usec = 1.e6 * (t - (double)tnew.it_value.tv_sec);
    if (t && tnew.it_value.tv_sec == 0 && tnew.it_value.tv_usec < 1000) {
        tnew.it_value.tv_usec = 1000;
    }
    tnew.it_interval.tv_sec = 0;
    tnew.it_interval.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &tnew, &told) < 0) {
        errormsg(SH_DICT, ERROR_system(1), e_alarm);
        __builtin_unreachable();
    }
    t = told.it_value.tv_sec + 1.e-6 * told.it_value.tv_usec;
    return t;
}

//
// Signal handler for alarm call.
//
// TODO: There is a bug in this code that may cause alarm callback to never get invoked due
// to timing issues. See https://github.com/att/ast/issues/633
static_fn void sigalrm(int sig, siginfo_t *info, void *context) {
    UNUSED(context);
    Timer_t *tp, *tplast, *tpold, *tpnext;
    double now;
    static double left = 0;
    Shell_t *shp = sh_getinterp();

    set_trapinfo(shp, sig, info);

    if (time_state & SIGALRM_CALL) {
        time_state &= ~SIGALRM_CALL;
    } else if (alarm(0)) {
        kill(getpid(), SIGALRM | SH_TRAP);
    }
    if (time_state) {
        if (time_state & IN_ADDTIMEOUT) time_state |= DEFER_SIGALRM;
        errno = EINTR;
        return;
    }
    time_state |= IN_SIGALRM;
    sh_sigaction(SIGALRM, SIG_UNBLOCK);
    while (1) {
        now = getnow();
        tpold = tpmin = 0;
        for (tplast = 0, tp = tptop; tp; tp = tpnext) {
            tpnext = tp->next;
            if (tp->action) {
                if (tp->wakeup <= now) {
                    if (!tpold || tpold->wakeup > tp->wakeup) tpold = tp;
                } else {
                    if (!tpmin || tpmin->wakeup > tp->wakeup) tpmin = tp;
                }
                tplast = tp;
            } else {
                if (tplast) {
                    tplast->next = tp->next;
                } else {
                    tptop = tp->next;
                }
                tp->next = tpfree;
                tpfree = tp;
            }
        }
        if ((tp = tpold) && tp->incr) {
            while ((tp->wakeup += tp->incr) <= now) {
                ;  // empty loop
            }
            if (!tpmin || tpmin->wakeup > tp->wakeup) tpmin = tp;
        }
        if (tpmin && (left == 0 || (tp && tpmin->wakeup < (now + left)))) {
            if (left == 0) sh_signal(SIGALRM, sigalrm);
            left = setalarm(tpmin->wakeup - now);
            if (left && (now + left) < tpmin->wakeup) {
                setalarm(left);
            } else {
                left = tpmin->wakeup - now;
            }
        }
        if (tp) {
            void (*action)(void *);
            action = tp->action;
            if (!tp->incr) tp->action = 0;
            errno = EINTR;
            time_state &= ~IN_SIGALRM;
            (*action)(tp->handle);
            time_state |= IN_SIGALRM;
        } else {
            break;
        }
    }
    if (!tpmin) {
        sh_signal(SIGALRM, (sh.sigflag[SIGALRM] & SH_SIGFAULT) ? sh_fault : (sh_sigfun_t)(SIG_DFL));
    }
    time_state &= ~IN_SIGALRM;
    errno = EINTR;
}

static_fn void oldalrm(void *handle) {
    sh_sigfun_t fn = *(sh_sigfun_t *)handle;
    free(handle);
    (*fn)(SIGALRM, NULL, NULL);
}

Timer_t *sh_timeradd(unsigned long msec, int flags, void (*action)(void *), void *handle) {
    Timer_t *tp;
    double t;
    sh_sigfun_t fn;

    t = ((double)msec) / 1000.;
    if (t <= 0 || !action) return NULL;
    tp = tpfree;
    if (tp) {
        tpfree = tp->next;
    } else {
        tp = malloc(sizeof(Timer_t));
    }
    tp->wakeup = getnow() + t;
    tp->incr = (flags ? t : 0);
    tp->action = action;
    tp->handle = handle;
    time_state |= IN_ADDTIMEOUT;
    tp->next = tptop;
    tptop = tp;
    if (!tpmin || tp->wakeup < tpmin->wakeup) {
        tpmin = tp;
        fn = sh_signal(SIGALRM, sigalrm);
        if ((t = setalarm(t)) > 0 && fn && fn != sigalrm) {
            sh_sigfun_t *hp = malloc(sizeof(sh_sigfun_t));
            *hp = fn;
            sh_timeradd((long)(1000 * t), 0, oldalrm, hp);
        }
        tp = tptop;
    } else if (tpmin && !tpmin->action) {
        time_state |= DEFER_SIGALRM;
    }
    time_state &= ~IN_ADDTIMEOUT;
    if (time_state & DEFER_SIGALRM) {
        time_state = SIGALRM_CALL;
        kill(getpid(), SIGALRM);
        if (tp != tptop) tp = 0;
    }
    return tp;
}

//
// Delete timer <tp>.  If <tp> is NULL, all timers are deleted.
//
void timerdel(void *handle) {
    Timer_t *tp = (Timer_t *)handle;
    if (tp) {
        tp->action = 0;
    } else {
        for (tp = tptop; tp; tp = tp->next) tp->action = 0;
        if (tpmin) {
            tpmin = 0;
            setalarm((double)0);
        }
        sh_signal(SIGALRM, (sh.sigflag[SIGALRM] & SH_SIGFAULT) ? sh_fault : (sh_sigfun_t)(SIG_DFL));
    }
}
