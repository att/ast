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
#include "option.h"
#include "shcmd.h"

//
// Builtin `break`.
// See also the continue.c module.
//
int b_break(int n, char *argv[], Shbltin_t *context) {
    char *arg;
    Shell_t *shp = context->shp;
    while ((n = optget(argv, sh_optbreak))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(0), "%s", opt_info.arg);
                return 2;
            }
        }
    }

    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    argv += opt_info.index;
    n = 1;
    arg = *argv;

    if (arg) {
        n = (int)strtol(arg, &arg, 10);
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
