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
 * process library interface
 */
#ifndef _PROC_H
#define _PROC_H 1

#include <signal.h>
#include "ast.h"

#define PROC_ARGMOD (1 << 0)       // argv[-1],argv[0] can be modified
#define PROC_BACKGROUND (1 << 1)   // shell background (&) setup
#define PROC_CLEANUP (1 << 2)      // close parent redirect fds on error
#define PROC_DAEMON (1 << 3)       // daemon setup
#define PROC_ENVCLEAR (1 << 4)     // clear environment
#define PROC_GID (1 << 5)          // setgid(getgid())
#define PROC_IGNORE (1 << 6)       // ignore parent pipe errors
#define PROC_OVERLAY (1 << 7)      // overlay current process if possible
#define PROC_PRIVELEGED (1 << 9)   // setuid(0), setgid(getegid())
#define PROC_READ (1 << 10)        // proc pipe fd 1 returned
#define PROC_SESSION (1 << 11)     // session leader
#define PROC_UID (1 << 12)         // setuid(getuid())
#define PROC_WRITE (1 << 13)       // proc pipe fd 0 returned
#define PROC_FOREGROUND (1 << 14)  // system(3) setup
#define PROC_ZOMBIE (1 << 15)      // proc may leave a zombie behind
#define PROC_ORPHAN (1 << 18)      // create orphaned process
// #define PROC_PARANOID (1 << 8)     // restrict everything
// #define PROC_IGNOREPATH (1 << 16)  // procrun() intercept to ignore path
// #define PROC_CHECK (1 << 17)       // check that command exists

#define PROC_ARG_BIT 14  // bits per op arg
#define PROC_OP_BIT 4    // bits per op

#define PROC_ARG_NULL ((1 << PROC_ARG_BIT) - 1)

#define PROC_FD_CHILD 0x1
#define PROC_FD_PARENT 0x2

#define PROC_fd_dup 0x4
#define PROC_sig_dfl 0x8
#define PROC_sig_ign 0x9

#define PROC_sys_pgrp 0xa
#define PROC_sys_umask 0xb

#define PROC_fd_ctty 0xc

#define PROC_OP(x) (((x) >> (2 * PROC_ARG_BIT)) & ((1 << PROC_OP_BIT) - 1))
//
// This is the original definition of PROC_ARG. It's hard to read and results in lint warnings since
// at the moment it is always called with n!=0.
//
// #define PROC_ARG(x, n)
//     ((n) ? (((x) >> (((n)-1) * PROC_ARG_BIT)) & PROC_ARG_NULL)
//          : (((x) & ~((1 << (2 * PROC_ARG_BIT)) - 1)) == ~((1 << (2 * PROC_ARG_BIT)) - 1))
//                ? (-1)
//                : ((x) & ~((1 << (2 * PROC_ARG_BIT)) - 1)))
//
static inline int PROC_ARG(long x, long n) {
    if (n) return (x >> ((n - 1) * PROC_ARG_BIT)) & PROC_ARG_NULL;
    if ((x & ~((1 << (2 * PROC_ARG_BIT)) - 1)) == ~((1 << (2 * PROC_ARG_BIT)) - 1)) return -1;
    return x & ~((1 << (2 * PROC_ARG_BIT)) - 1);
}

struct Mods_s;

typedef struct {
    pid_t pid;             // process id
    pid_t pgrp;            // process group id
    int rfd;               // read fd if applicable
    int wfd;               // write fd if applicable
    struct Mod_s *mods;    // process modification state
    long flags;            // original PROC_* flags
    sigset_t mask;         // original blocked sig mask
    sighandler_t sigchld;  // PROC_FOREGROUND SIG_DFL
    sighandler_t sigint;   // PROC_FOREGROUND SIG_IGN
    sighandler_t sigquit;  // PROC_FOREGROUND SIG_IGN
} Proc_t;

extern Proc_t proc_default;  // first proc
extern int procclose(Proc_t *);
extern int procfree(Proc_t *);
extern Proc_t *procopen(const char *, char **, char **, long *, int);

#endif  // _PROC_H
