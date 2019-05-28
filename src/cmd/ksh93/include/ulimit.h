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
// This is for the ulimit built-in command.
//
#ifndef _ULIMIT_H
#define _ULIMIT_H 1

#include <sys/resource.h>

#ifdef RLIM_INFINITY
#define INFINITY RLIM_INFINITY
#else
#ifndef INFINITY
#define INFINITY ((rlim_t)-1L)
#endif  // INFINITY
#endif  // RLIM_INFINITY

#ifndef RLIMIT_VMEM
#ifdef RLIMIT_AS
#define RLIMIT_VMEM RLIMIT_AS
#endif
#endif  // !RLIMIT_VMEM

#if !defined(RLIMIT_NOFILE) && defined(RLIMIT_OFILE)
#define RLIMIT_NOFILE RLIMIT_OFILE
#endif

#ifndef RLIMIT_UNKNOWN
#define RLIMIT_UNKNOWN (-9999)
#endif
#ifndef RLIMIT_AS
#define RLIMIT_AS RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_CORE
#define RLIMIT_CORE RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_CPU
#define RLIMIT_CPU RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_DATA
#define RLIMIT_DATA RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_FSIZE
#define RLIMIT_FSIZE RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_LOCKS
#define RLIMIT_LOCKS RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_MEMLOCK
#define RLIMIT_MEMLOCK RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_MSGQUEUE
#define RLIMIT_MSGQUEUE RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_NOFILE
#define RLIMIT_NOFILE RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_NICE
#define RLIMIT_NICE RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_NPROC
#define RLIMIT_NPROC RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_PIPE
#define RLIMIT_PIPE RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_PTHREAD
#define RLIMIT_PTHREAD RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_RSS
#define RLIMIT_RSS RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_RTPRIO
#define RLIMIT_RTPRIO RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_SBSIZE
#define RLIMIT_SBSIZE RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_SIGPENDING
#define RLIMIT_SIGPENDING RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_STACK
#define RLIMIT_STACK RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_SWAP
#define RLIMIT_SWAP RLIMIT_UNKNOWN
#endif
#ifndef RLIMIT_VMEM
#define RLIMIT_VMEM RLIMIT_UNKNOWN
#endif

#define LIM_COUNT 0
#define LIM_BLOCK 1
#define LIM_BYTE 2
#define LIM_KBYTE 3
#define LIM_SECOND 4

typedef struct Limit_s {
    const char *name;
    const char *description;
    int index;
    unsigned char option;
    unsigned char type;
} Limit_t;

extern const Limit_t shtab_limits[];
extern const int shtab_units[];

extern const char e_unlimited[];
extern const char *e_units[];

#endif  // _ULIMIT_H
