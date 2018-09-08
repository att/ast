/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1992-2012 AT&T Intellectual Property          *
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
/*
 * AT&T Research
 *
 * builtin cmd definitions
 */
#ifndef _CMD_H
#define _CMD_H 1

#include "ast.h"
#include "error.h"
#include "shcmd.h"

extern int b_stty(int argc, char **argv, Shbltin_t *context);

#define ERROR_CALLBACK ERROR_SET

#include "cmdext.h"

#define cmdinit(a, b, c, d, e)                   \
    do {                                         \
        if (_cmd_init(a, b, c, d, e)) return -1; \
    } while (0)

extern int _cmd_init(int, char **, Shbltin_t *, const char *, int);

#endif  // _CMD_H
