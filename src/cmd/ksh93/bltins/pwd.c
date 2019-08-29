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
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "option.h"
#include "path.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"

int b_pwd(int argc, char *argv[], Shbltin_t *context) {
    char *cp;
    Shell_t *shp = context->shp;
    bool pflag = false;
    int n, ffd = -1;
    UNUSED(argc);

    while ((n = optget(argv, sh_optpwd))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'f': {
                ffd = opt_info.num;
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
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
        }
    }

    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    if (ffd != -1) {
        cp = fgetcwd(ffd, 0, 0);
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
