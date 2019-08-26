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
 * David Korn
 * AT&T Research
 *
 * logname
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <stdio.h>
#include <unistd.h>

#include "builtins.h"
#include "error.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"

int b_logname(int argc, char **argv, Shbltin_t *context) {
    char *logname;
    char buf[12];
    int n;

    if (cmdinit(argc, argv, context, 0)) return -1;
    while ((n = optget(argv, sh_optlogname))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case ':':
                error(2, "%s", opt_info.arg);
                break;
            case '?':
                error(ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
        }
    }
    if (error_info.errors) {
        error(ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    logname = getlogin();
    if (!logname) {
        (void)snprintf(buf, sizeof(buf), "%u", getuid());
        logname = buf;
    }
    sfputr(sfstdout, logname, '\n');
    return 0;
}
