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
 * command initialization
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "error.h"
#include "option.h"
#include "shcmd.h"

int cmdinit(int argc, char **argv, Shbltin_t *context, int flags) {
    char *cp;

    if (argc <= 0) return -1;
    if (context) {
        if (flags & ERROR_NOTIFY) {
            context->notify = 1;
            flags &= ~ERROR_NOTIFY;
        }
        error_info.flags |= flags;
    }
    cp = strrchr(argv[0], '/');
    if (cp) {
        cp++;
    } else {
        cp = argv[0];
    }
    error_info.id = cp;
    opt_info.index = 0;
    return 0;
}
