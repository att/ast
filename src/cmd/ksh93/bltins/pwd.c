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

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "optget_long.h"
#include "path.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"

static const char *short_options = "f:LP";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
//  The `pwd` special builtin.
//
int b_pwd(int argc, char *argv[], Shbltin_t *context) {
    char *cp;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    bool pflag = false;
    int opt, fd = -1;

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'f': {
                char *cp;
                int64_t n = strton64(optget_arg, &cp, NULL, 0);
                if (*cp || n < 0 || n > INT_MAX) {
                    builtin_usage_error(shp, cmd, "%s: invalid -f value", optget_arg);
                    return 2;
                }
                fd = n;
                break;
            }
            case 'L': {
                pflag = false;
                break;
            }
            case 'P': {
                pflag = true;
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

    if (fd != -1) {
        cp = fgetcwd(fd, 0, 0);
        if (!cp) {
            errormsg(SH_DICT, ERROR_system(1), e_pwd);
            __builtin_unreachable();
        }
        sfputr(sfstdout, cp, '\n');
        free(cp);
        return 0;
    }
    if (pflag) {
        cp = path_pwd(shp);
        cp = strcpy(stkseek(stkstd, strlen(cp) + PATH_MAX), cp);
        pathcanon(cp, PATH_MAX, PATH_ABSOLUTE | PATH_PHYSICAL);
    } else {
        cp = path_pwd(shp);
    }

    if (*cp != '/') {
        errormsg(SH_DICT, ERROR_system(1), e_pwd);
        __builtin_unreachable();
    }

    sfputr(sfstdout, cp, '\n');
    return 0;
}
