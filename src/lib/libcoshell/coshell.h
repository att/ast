/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
//
// Glenn Fowler
// AT&T Research
//
// Coshell library interface
//
#ifndef _COSHELL_H
#define _COSHELL_H 1

#include "ast.h"

#if !_BLD_coshell

#undef procrun
#define procrun(a, b, c) coprocrun(a, b, c)
#undef system
#define system(a) cosystem(a)

#endif

struct Coshell_s;
typedef struct Coshell_s Coshell_t;
struct Cojob_s;
typedef struct Cojob_s Cojob_t;

//
// DEPRECATED names for compatibility
//

#define COSHELL Coshell_t
#define COJOB Cojob_t

#define CO_ID "coshell"  // Library/command id

#define CO_ENV_ATTRIBUTES "COATTRIBUTES"  // Coshell attributes env var
#define CO_ENV_EXPORT "COEXPORT"          // Coshell env var export list
#define CO_ENV_HOST "HOSTNAME"            // Coshell host name env var
#define CO_ENV_MSGFD "_COSHELL_msgfd"     // Msg fd
#define CO_ENV_OPTIONS "COSHELL_OPTIONS"  // Options environment var
#define CO_ENV_PROC "NPROC"               // Concurrency environment var
#define CO_ENV_SHELL "COSHELL"            // Coshell path environment var
#define CO_ENV_TEMP "COTEMP"              // 10 char temp file base
#define CO_ENV_TYPE "HOSTTYPE"            // Coshell host type env var

#define CO_OPT_ACK "ack"            // Wait for server coexec() ack
#define CO_OPT_INDIRECT "indirect"  // Indirect server connection
#define CO_OPT_SERVER "server"      // Server connection

#define CO_QUANT 100  // Time quanta per sec

#define CO_ANY 0x000001       // Return any open coshell
#define CO_DEBUG 0x000002     // Library debug trace
#define CO_EXPORT 0x000004    // Export everything
#define CO_IGNORE 0x000008    // Ignore command errors
#define CO_LOCAL 0x000010     // Local affinity
#define CO_NONBLOCK 0x000020  // Don't block coexec if Q full
#define CO_SHELL 0x000040     // Shell using coshell!
#define CO_SILENT 0x000080    // Don't trace commands

#define CO_KSH 0x000100     // Coshell is ksh (readonly)
#define CO_SERVER 0x000200  // Coshell is server (readonly)
#define CO_OSH 0x000400     // Coshell is OLD (readonly)

#define CO_CROSS 0x000800  // Don't prepend local dirs
#define CO_DEVFD 0x001000  // Coshell handles /dev/fd/#

#define CO_SERIALIZE 0x002000  // Serialize stdout and stderr
#define CO_SERVICE 0x004000    // Service callouts

#define CO_APPEND 0x008000    // Append coexec() out/err
#define CO_SEPARATE 0x010000  // 1 shell+wait per coexec()
#define CO_ORPHAN 0x020000    // PROC_ORPHAN

#define CO_USER 0x100000  // First user flag

struct Cojob_s  // Coshell job info
{
    Coshell_t *coshell;  // Running in this coshell
    int id;              // Job id
    int status;          // Exit status
    int flags;           // CO_* flags
    void *local;         // Local info
    unsigned long user;  // User time in 1/CO_QUANT secs
    unsigned long sys;   // Sys time in 1/CO_QUANT secs
#ifdef _CO_JOB_PRIVATE_
    _CO_JOB_PRIVATE_  // Library private additions
#endif
};

struct Coshell_s  // coshell connection info
{
    void *data;          // User data, initially 0
    int flags;           // Flags
    int outstanding;     // Number of outstanding jobs
    int running;         // Number of running jobs
    int total;           // Number of coexec() jobs
    unsigned long user;  // User time in 1/CO_QUANT secs
    unsigned long sys;   // Sys time in 1/CO_QUANT secs
    Sfio_t *msgfp;       // Message stream for sfpoll()
#ifdef _CO_SHELL_PRIVATE_
    _CO_SHELL_PRIVATE_  // Library private additions
#endif
};

extern int coclose(Coshell_t *);
extern Cojob_t *coexec(Coshell_t *, const char *, int, const char *, const char *, const char *);
extern char *coinit(int);
extern int coexport(Coshell_t *, const char *, const char *);
extern int cokill(Coshell_t *, Cojob_t *, int);
extern Coshell_t *coopen(const char *, int, const char *);
extern void coquote(Sfio_t *, const char *, int);
extern int cosync(Coshell_t *, const char *, int, int);
extern Cojob_t *cowait(Coshell_t *, Cojob_t *, int);

extern int cojobs(Coshell_t *);
extern int copending(Coshell_t *);
extern int cozombie(Coshell_t *);

extern int coattr(Coshell_t *, const char *);

extern int coprocrun(const char *, char **, int);
extern int cosystem(const char *);

#endif  // _COSHELL_H
