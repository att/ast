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

#include <stdlib.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "optget_long.h"
#include "shcmd.h"

static const char *short_options = "";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins supports --help
    {NULL, 0, NULL, 0}};

//
// Builtin `break` command. See also the continue.c module.
//
int b_break(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);
    int opt;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
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

    long n = 1;
    char *arg = *argv;
    if (arg) {
        n = strtol(arg, &arg, 10);
        if (n <= 0 || *arg) {
            errormsg(SH_DICT, ERROR_exit(1), e_nolabels, *argv);
            __builtin_unreachable();
        }
    }

    if (shp->st.loopcnt) {
        shp->st.execbrk = shp->st.breakcnt = n;
        if (shp->st.breakcnt > shp->st.loopcnt) shp->st.breakcnt = shp->st.loopcnt;
    }

    return 0;
}
