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
#include <string.h>
#include <stdlib.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "name.h"
#include "shcmd.h"

static const char *short_options = "+:v:";
static const struct option long_options[] = {
    {"help", no_argument, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
// Builtin `printf` command.
//
int b_printf(int argc, char *argv[], Shbltin_t *context) {
    int opt;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    struct print prdata;

    memset(&prdata, 0, sizeof(prdata));
    prdata.fd = 1;

    optind = opterr = 0;
    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'v': {
                prdata.var_name = nv_open(optarg, shp->var_tree, NV_VARNAME | NV_NOARRAY);
                if (!prdata.var_name) {
                    errormsg(SH_DICT, 2, "Cannot create variable %s", optarg);
                    return 2;
                }
                break;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[optind - 1]);
                return 2;
            }
            case '?': {
                builtin_unknown_option(shp, cmd, argv[optind - 1]);
                return 2;
            }
            default: { abort(); }
        }
    }

    argv += optind;
    if (!*argv) {
        builtin_usage_error(shp, cmd, "at least one argument (the format) is required");
        return 2;
    }
    prdata.format = *argv++;
    return sh_print(argv, shp, &prdata);
}
