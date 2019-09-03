/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2012 AT&T Intellectual Property          *
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
// UNIX shell
// S. R. Bourne
// Rewritten by David Korn
//
#ifndef _FAULT_H
#define _FAULT_H 1

#ifdef _AST_SIG_H
#error You cannot include fault.h after sig.h
#endif

#include <setjmp.h>
#include <signal.h>

#include "error.h"

#ifndef SIGWINCH
#ifdef SIGWIND
#define SIGWINCH SIGWIND
#else  // SIGWIND
#ifdef SIGWINDOW
#define SIGWINCH SIGWINDOW
#endif  // SIGWINDOW
#endif  // SIGWIND
#endif  // SIGWINCH

typedef void (*SH_SIGTYPE)(int, void (*)(int));

#define SH_FORKLIM 16  // fork timeout interval

#define SH_TRAP 0200    // bit for internal traps
#define SH_ERRTRAP 0    // trap for non-zero exit status
#define SH_KEYTRAP 1    // trap for keyboard event
#define SH_DEBUGTRAP 4  // must be last internal trap

#define SH_SIGBITS 8
#define SH_SIGFAULT 1           // signal handler is sh_fault
#define SH_SIGOFF 2             // signal handler is SIG_IGN
#define SH_SIGSET 4             // pending signal
#define SH_SIGTRAP 010          // pending trap
#define SH_SIGDONE 020          // default is exit
#define SH_SIGIGNORE 040        // default is ignore signal
#define SH_SIGINTERACTIVE 0100  // handle interactive specially
#define SH_SIGTSTP 0200         // tstp signal received
#define SH_SIGTERM SH_SIGOFF    // term signal received
#define SH_SIGRUNTIME 0400      // runtime value

#define SH_SIGRTMIN 0  // sh.sigruntime[] index
#define SH_SIGRTMAX 1  // sh.sigruntime[] index

//
// These are longjmp values.
//
#define SH_JMPDOT 2
#define SH_JMPEVAL 3
#define SH_JMPTRAP 4
#define SH_JMPIO 5
#define SH_JMPCMD 6
#define SH_JMPFUN 7
#define SH_JMPERRFN 8
#define SH_JMPSUB 9
#define SH_JMPERREXIT 10
#define SH_JMPEXIT 11
#define SH_JMPSCRIPT 12

struct openlist {
    Sfio_t *strm;
    struct openlist *next;
};

struct checkpt {
    sigjmp_buf buff;
    struct checkpt *prev;
    int topfd;
    int mode;
    int vexi;
    struct openlist *olist;
    Error_context_t err;
};

typedef struct checkpt checkpt_t;

struct siginfo_ll {
    siginfo_t info;
    struct siginfo_ll *next;
    struct siginfo_ll *last;
};
typedef struct siginfo_ll siginfo_ll_t;

#if USE_SPAWN
#define sh_pushcontext(shp, bp, n)                                                              \
    ((bp)->mode = (n), (bp)->olist = NULL, (bp)->topfd = shp->topfd, (bp)->prev = shp->jmplist, \
     (bp)->vexi = shp->vexp->cur, (bp)->err = *ERROR_CONTEXT_BASE, shp->jmplist = bp)

#else  // USE_SPAWN

#define sh_pushcontext(shp, bp, n)                                                              \
    ((bp)->mode = (n), (bp)->olist = NULL, (bp)->topfd = shp->topfd, (bp)->prev = shp->jmplist, \
     (bp)->err = *ERROR_CONTEXT_BASE, shp->jmplist = bp)
#endif  // USE_SPAWN

#define sh_popcontext(shp, bp) (shp->jmplist = (bp)->prev, errorpop(&((bp)->err)))

typedef void (*sh_sigfun_t)(int, siginfo_t *, void *);
extern sh_sigfun_t sh_signal(int, sh_sigfun_t);
extern void sh_fault(int, siginfo_t *, void *);
extern void sh_setsiginfo(siginfo_t *);
extern void set_trapinfo(Shell_t *shp, int sig, siginfo_t *info);
extern int sig_number(Shell_t *shp, const char *string);
#undef signal
#define signal(a, b) ERROR("use sh_signal() not signal()")

extern __attribute__((noreturn)) void sh_done(void *, int);
extern void sh_siginit(Shell_t *shp);
extern Timer_t *sh_timeradd(unsigned long, int, void (*)(void *), void *);
extern void timerdel(void *);

extern const char e_alarm[];

#endif  // _FAULT_H
