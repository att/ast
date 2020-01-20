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
// UNIX shell path handling interface.
// Written by David Korn.
// These are the definitions for the lexical analyzer.
//
#ifndef _PATH_H
#define _PATH_H 1

#include "name.h"
#include "shell.h"

#define PATH_PATH (1 << 0)
#define PATH_FPATH (1 << 1)  // do not change this value; see can_execute() and path_absolute()
#define PATH_CDPATH (1 << 2)
#define PATH_BFPATH (1 << 3)
#define PATH_SKIP (1 << 4)
#define PATH_BUILTIN_LIB (1 << 5)
#define PATH_STD_DIR (1 << 6)  // directory is on $(getconf PATH)
#define PATH_BIN (1 << 7)      // path behaves like /bin for builtins

#define PATH_OFFSET 2  // path offset for path_join
#define MAXDEPTH 1024  // maximum recursion depth

//
// Path component structure for path searching.
//
struct pathcomp {
    struct pathcomp *next;
    int refcount;
    dev_t dev;
    ino_t ino;
    time_t mtime;
    char *name;
    char *lib;
    char *bbuf;
    char *blib;
    unsigned short len;
    unsigned short flags;
    Shell_t *shp;
};

#ifndef ARG_RAW
struct argnod;
#endif  // !ARG_RAW

// Pathname handling routines.
extern void path_newdir(Shell_t *, Pathcomp_t *);
extern Pathcomp_t *path_dirfind(Pathcomp_t *, const char *, int);
extern Pathcomp_t *path_unsetfpath(Shell_t *);
extern Pathcomp_t *path_addpath(Shell_t *, Pathcomp_t *, const char *, int);
extern bool path_cmdlib(Shell_t *, const char *, bool);
extern Pathcomp_t *path_dup(Pathcomp_t *);
extern void path_delete(Pathcomp_t *);
extern void path_alias(Namval_t *, Pathcomp_t *);
extern Pathcomp_t *path_absolute(Shell_t *, const char *, Pathcomp_t *);
extern char *path_basename(const char *);
extern char *path_fullname(Shell_t *, const char *);
extern int path_expand(Shell_t *, const char *, struct argnod **);
extern void path_exec(Shell_t *, const char *, char *[], struct argnod *);
extern pid_t path_spawn(Shell_t *, const char *, char *[], char *[], Pathcomp_t *, int);
extern int path_open(Shell_t *, const char *, Pathcomp_t *);
extern Pathcomp_t *path_get(Shell_t *, const char *);
extern char *path_pwd(Shell_t *);
extern Pathcomp_t *path_nextcomp(Shell_t *, Pathcomp_t *, const char *, Pathcomp_t *);
extern bool path_search(Shell_t *, const char *, Pathcomp_t **, int);
extern char *path_relative(Shell_t *, const char *);
extern int path_complete(Shell_t *, const char *, const char *, struct argnod **);
extern int path_generate(Shell_t *, struct argnod *, struct argnod **);

// Builtin/plugin routines.
extern int sh_addlib(Shell_t *, void *, char *, Pathcomp_t *);
extern Shbltin_f sh_getlib(Shell_t *, char *, Pathcomp_t *);

// Constant strings needed for whence.
extern const char e_timeformat[];
extern const char e_badtformat[];
extern const char e_dot[];
extern const char e_funload[];
extern const char e_pfsh[];
extern const char e_pwd[];
extern const char e_logout[];
extern const char e_alphanum[];
extern const char e_mailmsg[];
extern const char e_suidprofile[];
extern const char e_sysprofile[];
extern const char e_traceprompt[];
extern const char is_alias[];
extern const char is_builtin[];
extern const char is_spcbuiltin[];
extern const char is_builtver[];
extern const char is_reserved[];
extern const char is_talias[];
extern const char is_xalias[];
extern const char is_function[];
extern const char is_ufunction[];
extern const char e_prohibited[];

#endif  // _PATH_H
