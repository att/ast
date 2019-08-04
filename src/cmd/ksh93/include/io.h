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
//
// UNIX shell
// David Korn
//
#ifndef _IO_H
#define _IO_H 1

#include "shell.h"

#define IOBSIZE (SF_BUFSIZE * _ast_sizeof_pointer)
#define IOMAXTRY 20

#define IOREAD 001
#define IOWRITE 002
#define IODUP 004
#define IOSEEK 010
#define IONOSEEK 020
#define IOTTY 040
#define IOCLEX 0100
#define IOCLOSE (IOSEEK | IONOSEEK)

#define IOSUBSHELL 0x8000     // must be larger than any file descriptor
#define IOPICKFD 0x10000      // file descriptor number was selected automatically
#define IOHERESTRING 0x20000  // allow here documents to be string streams
#define IOSAVESTRING 0x40000  // string file was saved
#define IOUSEVEX 0x80000      // use spawnvex to save and restore

//
// The remainder of this file is only used when compiled with shell.
//
#ifndef ARG_RAW
struct ionod;
#endif  // !ARG_RAW

extern int sh_iocheckfd(Shell_t *, int, int);
extern void sh_ioinit(Shell_t *);
extern int sh_iomovefd(Shell_t *, int);
extern int sh_iorenumber(Shell_t *, int, int);
extern void sh_pclose(int[]);
extern void sh_rpipe(int[]);
extern void sh_iorestore(Shell_t *, int, int);
#if USE_SPAWN
extern void sh_vexrestore(Shell_t *, int);
extern void sh_vexsave(Shell_t *, int, int, Spawnvex_f, void *);
#endif  // USE_SPAWN
extern Sfio_t *sh_iostream(Shell_t *, int, int);
extern int sh_redirect(Shell_t *, struct ionod *, int);
extern void sh_iosave(Shell_t *, int, int, char *);
extern int sh_get_unused_fd(Shell_t *shp, int min_fd);
extern bool sh_iovalidfd(Shell_t *, int);
extern bool sh_inuse(Shell_t *, int);
extern void sh_iounsave(Shell_t *);
extern void sh_iounpipe(Shell_t *);
extern int sh_chkopen(const char *);
extern int sh_ioaccess(int, int);
extern bool sh_isdevfd(const char *);
extern bool sh_source(Shell_t *, Sfio_t *, const char *);

// The following are readonly.
extern const char e_copexists[];
extern const char e_query[];
extern const char e_history[];
extern const char e_argtype[];
extern const char e_create[];
extern const char e_tmpcreate[];
extern const char e_exists[];
extern const char e_file[];
extern const char e_redirect[];
extern const char e_io[];
extern const char e_formspec[];
extern const char e_badregexp[];
extern const char e_open[];
extern const char e_notseek[];
extern const char e_noread[];
extern const char e_badseek[];
extern const char e_badwrite[];
extern const char e_badpattern[];
extern const char e_toomany[];
extern const char e_pipe[];
extern const char e_unknown[];
extern const char e_profile[];
extern const char e_sysprofile[];
extern const char e_sysrc[];
#if SHOPT_BASH
extern const char e_bash_sysrc[];
extern const char e_bash_rc[];
extern const char e_bash_login[];
extern const char e_bash_logout[];
extern const char e_bash_profile[];
#endif  // SHOPT_BASH
extern const char e_stdprompt[];
extern const char e_supprompt[];
extern const char e_ambiguous[];

#endif  // _IO_H
