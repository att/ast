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
// break [n]
// continue [n]
// return [n]
// exit [n]
//
//   David Korn
//   dgkorn@gmail.com
//
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
// Builtins `return` and `exit`.
//
int b_return(int n, char *argv[], Shbltin_t *context) {
    char *arg;
    Shell_t *shp = context->shp;
    struct checkpt *pp = (struct checkpt *)shp->jmplist;
    const char *options = (**argv == 'r' ? sh_optreturn : sh_optexit);
    while ((n = optget(argv, options))) {
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

    pp->mode = (**argv == 'e' ? SH_JMPEXIT : SH_JMPFUN);
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

    abort();
}

//
// Builtins `break` and `continue`.
//
int b_break(int n, char *argv[], Shbltin_t *context) {
    char *arg;
    int cont = **argv == 'c';
    Shell_t *shp = context->shp;
    while ((n = optget(argv, cont ? sh_optcont : sh_optbreak))) {
        switch (n) {
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(0), "%s", opt_info.arg);
                return 2;
            }
            default: { break; }
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
        if (cont) shp->st.breakcnt = -shp->st.breakcnt;
    }

    return 0;
}
