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
// coshell library private definitions
//
#ifndef _COLIB_H
#define _COLIB_H 1

#include <sys/wait.h>

#include "ast.h"
#include "dt.h"
#include "error.h"
#include "sig.h"

#define _CO_JOB_PRIVATE_  /* Cojob_t private additions	*/ \
    Cojob_t *next;        /* Next in list			*/            \
    Coservice_t *service; /* Service 			*/                \
    int pid;              /* pid				*/                    \
    char *out;            /* Serialized stdout file	*/    \
    char *err;            /* Serialized stderr file	*/    \
                          /* End of private additions	*/

#define _CO_SHELL_PRIVATE_ /* Coshell_t private additions	*/  \
    Coshell_t *next;       /* Next in list			*/               \
    Cojob_t *jobs;         /* Job list			*/                   \
    Coservice_t *service;  /* Service 			*/                   \
    Dt_t *export;          /* coexport() dictionary	*/        \
    Dtdisc_t *exdisc;      /* coexport() discipline	*/        \
    struct Coinit_s        /* Initialization script state	*/  \
    {                                                         \
        char *script;  /* Initialization script	*/            \
        dev_t pwd_dev; /* Previous pwd dev		*/                \
        ino_t pwd_ino; /* Previous pwd inode number	*/        \
        int mask;      /* Previous umask		*/                  \
        int sync;      /* Sync script			*/                    \
    } init;                                                   \
    int cmdfd;           /* Command pipe fd		*/               \
    int gsmfd;           /* msgfp child write side	*/         \
    int mask;            /* CO_* flags to clear		*/           \
    int mode;            /* Connection modes		*/              \
    int svc_outstanding; /* Outstanding service intercepts */ \
    int svc_running;     /* Running service intercepts	*/     \
    int pid;             /* pid				*/                         \
    int index;           /* coshell index		*/                 \
    int slots;           /* Number of job slots		*/           \
                         /* End of private additions	*/

typedef struct Coexport_s {
    Dtlink_t link;
    char *value;
    char name[1];
} Coexport_t;

struct Coservice_s;
typedef struct Coservice_s Coservice_t;

// Service info.
struct Coservice_s {
    Coservice_t *next;  // Next in list
    char *name;         // Instance name
    char *path;         // coexec() command path
    char *db;           // State/db path
    int fd;             // Command pipe
    int pid;            // Pid
    char *argv[16];     // coexec() command argv[]
};

#include "coshell.h"

#define state _coshell_info_  // Hide external symbol

#define CO_MODE_ACK (1 << 0)       // Wait for coexec() ack
#define CO_MODE_INDIRECT (1 << 1)  // Indirect CO_SERVER
#define CO_MODE_SEPARATE (1 << 2)  // 1 shell+wait per action

#define CO_INIT (CO_USER >> 1)  // Initial command

#define CO_PID_FREE (-3)    // Free job slot
#define CO_PID_WARPED (-2)  // Exit before start message
#define CO_PID_ZOMBIE (-1)  // Ready for wait

#define CO_BUFSIZ (PATH_MAX / 2)   // Temporary buffer size
#define CO_MAXEVAL (PATH_MAX * 8)  // Max eval'd action size

typedef struct Costate_s  // Global coshell state
{
    const char *lib;      // Library id
    Coshell_t *coshells;  // List of all coshells
    Coshell_t *current;   // Current coshell
    Coshell_t *generic;   // Generic coshell for coinit()
    char *pwd;            // pwd
    char *sh;             // sh from first coopen()
    char *type;           // CO_ENV_TYPE value
    int init;             // 0 if first coopen()
    int index;            // Last coshell index
} Costate_t;

extern char coident[];     // Coshell ident script
extern char cobinit[];     // bsh initialition script
extern char cokinit[];     // ksh initialition script
extern char *co_export[];  // Default export var list

extern Costate_t state;  // Global coshell info

extern char *costash(Sfio_t *);
extern char *coinitialize(Coshell_t *, int);

#endif  // _COLIB_H
