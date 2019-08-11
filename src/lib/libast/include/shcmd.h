/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
//
// Ksh builtin command API header.
//
#ifndef _SHCMD_H
#define _SHCMD_H 1

#define SH_PLUGIN_VERSION 20111111L

// #define SHLIB(m)
//     unsigned long plugin_version(void) { return SH_PLUGIN_VERSION; }

struct Shbltin_s {
    Shell_t *shp;
    void *ptr;
    int version;
    int (*shrun)(Shell_t *, int, char **);
    void (*shexit)(Shell_t *, int);
    unsigned char notify;
    unsigned char sigset;
    unsigned char nosfio;
    Namval_t *bnode;
    Namval_t *vnode;
    void *data;
    int flags;
    int invariant;
    int pwdfd;
};

// The following symbols used to have a `sh_` prefix and were meant to mask the functions of the
// same name when used in a builtin (e.g., code in src/lib/libcmd). That has been changed because
// that sort of redirection obfuscates what is actually happening and makes reasoning about the
// code harder.
#define bltin_checksig(c) (c && c->sigset)

extern int cmdinit(int, char **, Shbltin_t *, int);

#endif  // _SHCMD_H
