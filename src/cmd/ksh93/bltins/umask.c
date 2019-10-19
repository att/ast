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

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ast.h"
#include "defs.h"
#include "error.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"

// This has to be included after "shell.h".
#include "builtins.h"

static const char *short_options = "+:pS";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

int b_umask(int argc, char *argv[], Shbltin_t *context) {
    int flag = 0;
    bool sflag = false, pflag = false;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    int opt;

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'p': {
                pflag = true;
                break;
            }
            case 'S': {
                sflag = true;
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

    char *mask = *argv;
    if (!mask) {  // display the current umask
        char *prefix = pflag ? "umask " : "";
        flag = sh_umask(0);
        sh_umask(flag);
        if (sflag) {
            sfprintf(sfstdout, "%s%s\n", prefix, fmtperm(~flag & 0777));
        } else {
            sfprintf(sfstdout, "%s%0#4o\n", prefix, flag);
        }
        return 0;
    }

    // Set a new umask.
    int c;
    if (isdigit(*mask)) {
        while ((c = *mask++)) {
            if (c >= '0' && c <= '7') {
                flag = (flag << 3) + (c - '0');
            } else {
                errormsg(SH_DICT, ERROR_exit(1), e_number, *argv);
                __builtin_unreachable();
            }
        }
    } else {
        char *cp = mask;
        flag = sh_umask(0);
        c = strperm(cp, &cp, ~flag & 0777);
        if (*cp) {
            sh_umask(flag);
            errormsg(SH_DICT, ERROR_exit(1), e_format, mask);
            __builtin_unreachable();
        }
        flag = (~c & 0777);
    }
    sh_umask(flag);
    return 0;
}
