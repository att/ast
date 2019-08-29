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

#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "option.h"
#include "shcmd.h"

//
// Builtin `return` command. See also the exit.c module which is similar to this module.
//
int b_return(int n, char *argv[], Shbltin_t *context) {
    char *arg;
    Shell_t *shp = context->shp;
    checkpt_t *pp = shp->jmplist;
    while ((n = optget(argv, sh_optreturn))) {
        switch (n) {
            case ':': {
                if (!argv[opt_info.index] || !strmatch(argv[opt_info.index], "[+-]+([0-9])")) {
                    errormsg(SH_DICT, 2, "%s", opt_info.arg);
                }
                goto done;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(0), "%s", opt_info.arg);
                return 2;
            }
            default: { break; }
        }
    }

done:
    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    pp->mode = SH_JMPFUN;
    argv += opt_info.index;
    n = (((arg = *argv) ? (int)strtol(arg, NULL, 10) : shp->oldexit));
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
