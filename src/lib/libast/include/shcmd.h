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
/*
 * ksh builtin command api
 */
#ifndef _SHCMD_H
#define _SHCMD_H 1

#define SH_PLUGIN_VERSION 20111111L

#define SHLIB(m) \
    unsigned long plugin_version(void) { return SH_PLUGIN_VERSION; }

#ifndef SH_VERSION
#define Shell_t void
#endif
#ifndef _NVAL_H
#define Namval_t void
#endif

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
    char *data;
    int flags;
    int invariant;
    int pwdfd;
};

typedef struct Shbltin_s Shbltin_t;
typedef int (*Shbltin_f)(int, char **, Shbltin_t *);

#if defined(SH_VERSION) || defined(_SH_PRIVATE)
#undef Shell_t
#undef Namval_t
#else  // defined(SH_VERSION) || defined(_SH_PRIVATE)
#define sh_context(c) ((Shbltin_t *)(c))
#define sh_run(c, ac, av) ((c) ? (*sh_context(c)->shrun)(sh_context(c)->shp, ac, av) : -1)
#define sh_exit(c, n) ((c) ? (*sh_context(c)->shexit)(sh_context(c)->shp, n) : exit(n))
#define sh_checksig(c) ((c) && sh_context(c)->sigset)
#if defined(SFIO_VERSION) || defined(_AST_H)
#endif  // defined(SFIO_VERSION) || defined(_AST_H)
#endif  // defined(SH_VERSION) || defined(_SH_PRIVATE)

extern int cmdinit(int, char **, Shbltin_t *, int);

#endif  // _SHCMD_H
