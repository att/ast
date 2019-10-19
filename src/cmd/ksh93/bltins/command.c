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
//
// command [-pvVx] name [arg...]
//
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>
#include "stdlib.h"

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "optget_long.h"
#include "shcmd.h"

static const char *short_options = "pvxV";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
// Builtin `command` command.
//
// The `command` command is called with argc==0 when checking for -V or -v option. In this case
// return 0 when -v or -V or unknown option, otherwise the shift count to the command is returned.
//
int b_command(int argc, char *argv[], Shbltin_t *context) {
    int opt, flags = 0;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    // We need to calculate argc because we might have been invoked with it set to zero. And that
    // doesn't confuse the AST optget() function but does break optget_long().
    int true_argc = argc;
    if (true_argc == 0) {
        for (char **cp = argv; *cp; cp++) true_argc++;
    }

    optget_ind = 0;
    while ((opt = optget_long(true_argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                if (argc != 0) builtin_print_help(shp, cmd);
                return 0;
            }
            case 'p': {
                if (sh_isoption(shp, SH_RESTRICTED)) {
                    errormsg(SH_DICT, ERROR_exit(1), e_restricted, "-p");
                    __builtin_unreachable();
                }
                sh_onstate(shp, SH_DEFPATH);
                break;
            }
            case 'v': {
                flags |= WHENCE_X_FLAG;
                break;
            }
            case 'V': {
                flags |= WHENCE_V_FLAG;
                break;
            }
            case 'x': {
                shp->xargexit = 1;
                break;
            }
            case ':': {
                if (argc == 0) return 0;
                builtin_missing_argument(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            case '?': {
                if (argc == 0) return 0;
                builtin_unknown_option(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            default: { abort(); }
        }
    }
    if (argc == 0) return flags ? 0 : optget_ind;
    argv += optget_ind;
    if (!*argv) {
        builtin_usage_error(shp, cmd, "missing command argument");
        return 2;
    }
    return whence(shp, argv, flags);
}
