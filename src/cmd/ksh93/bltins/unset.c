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
#include "cdt.h"
#include "defs.h"
#include "name.h"
#include "optget_long.h"
#include "shcmd.h"

static const char *short_options = "+:fnv";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
// The `unset` builtin.
//
int b_unset(int argc, char *argv[], Shbltin_t *context) {
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    Dt_t *troot = shp->var_tree;
    nvflag_t nvflags = 0;
    int opt;

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'f': {
                troot = sh_subfuntree(shp, true);
                nvflags |= NV_NOSCOPE;
                break;
            }
            case 'n': {
                nvflags |= NV_NOREF;
                troot = shp->var_tree;
                break;
            }
            case 'v': {
                troot = shp->var_tree;
                break;
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

    if (!*argv) {
        builtin_usage_error(shp, cmd, "expected at least one variable or function name");
        return 2;
    }

    if (!troot) return 1;
    if (troot == shp->var_tree) nvflags |= NV_VARNAME;
    return nv_unall(argv, false, nvflags, troot, shp);
}
