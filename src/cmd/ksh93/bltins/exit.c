/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2014 AT&T Intellectual Property          *
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
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <stdlib.h>

#include "builtins.h"
#include "defs.h"
#include "fault.h"
#include "optget_long.h"
#include "shcmd.h"

static const char *short_options = "#";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
// Builtin `exit` command. See also the return.c module which is similar to this module.
//
int b_exit(int argc, char *argv[], Shbltin_t *context) {
    int opt;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    bool done = false;
    optget_ind = 0;
    while (!done && (opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case -2: {
                done = true;
                optget_ind--;
                break;
            }
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            case '?': {
                builtin_unknown_option(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            default: { abort(); }
        }
    }
    argv += optget_ind;

    int exit_val = *argv ? strtol(*argv, NULL, 10) : shp->oldexit;
    if (exit_val < 0 || exit_val == 256 || exit_val > SH_EXITMASK + shp->gd->sigmax) {
        exit_val &= ((unsigned int)exit_val) & SH_EXITMASK;
    }

    shp->jmplist->mode = SH_JMPEXIT;
    shp->savexit = exit_val;
    sh_exit(shp, shp->savexit);
    __builtin_unreachable();
}
