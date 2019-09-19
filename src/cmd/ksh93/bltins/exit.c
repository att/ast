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

#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "fault.h"
#include "shcmd.h"

static const char *short_options = "+:X";
static const struct option long_options[] = {
    {"help", no_argument, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
// Builtin `exit` command. See also the return.c module which is similar to this module.
//
int b_exit(int argc, char *argv[], Shbltin_t *context) {
    int n, opt;
    char *arg;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    checkpt_t *pp = shp->jmplist;

    // We use `getopt_long_only()` rather than `getopt_long()` to facilitate handling negative
    // integers that might otherwise look like a flag.
    optind = opterr = 0;
    bool done = false;
    while (!done && (opt = getopt_long_only(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[optind - 1]);
                return 2;
            }
            case '?': {
                if (!strmatch(argv[optind - 1], "[+-]+([0-9])")) {
                    builtin_unknown_option(shp, cmd, argv[optind - 1]);
                    return 2;
                }
                optind--;
                done = true;
                break;
            }
            default: { abort(); }
        }
    }

    pp->mode = SH_JMPEXIT;
    argv += optind;
    arg = *argv;
    n = arg ? (int)strtol(arg, NULL, 10) : shp->oldexit;
    if (n < 0 || n == 256 || n > SH_EXITMASK + shp->gd->sigmax) {
        n &= ((unsigned int)n) & SH_EXITMASK;
    }

    // Return outside of function, dotscript and profile is exit.
    if (shp->fn_depth == 0 && shp->dot_depth == 0 && !sh_isstate(shp, SH_PROFILE)) {
        pp->mode = SH_JMPEXIT;
    }

    shp->savexit = n;
    sh_exit(shp, shp->savexit);
    __builtin_unreachable();
}
