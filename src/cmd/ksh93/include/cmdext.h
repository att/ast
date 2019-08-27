/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1992-2018 AT&T Intellectual Property          *
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
 *                                                                      *
 ***********************************************************************/
#ifndef _CMDEXT_H
#define _CMDEXT_H 1

#include "shcmd.h"

extern int b_basename(int, char **, Shbltin_t *);
extern int b_cat(int, char **, Shbltin_t *);
extern int b_chmod(int, char **, Shbltin_t *);
extern int b_cmp(int, char **, Shbltin_t *);
extern int b_cut(int, char **, Shbltin_t *);
extern int b_dirname(int, char **, Shbltin_t *);
extern int b_head(int, char **, Shbltin_t *);
extern int b_logname(int, char **, Shbltin_t *);
extern int b_mkdir(int, char **, Shbltin_t *);
extern int b_sync(int, char **, Shbltin_t *);
extern int b_uname(int, char **, Shbltin_t *);
extern int b_wc(int, char **, Shbltin_t *);

#endif  // _CMDEXT_H
