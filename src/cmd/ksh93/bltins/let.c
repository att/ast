/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2012 AT&T Intellectual Property          *
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
// let expr...
//
//   David Korn
//   AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stdlib.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "option.h"
#include "shcmd.h"

int b_let(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);

    Shell_t *shp = context->shp;
    int n;

    while ((n = optget(argv, sh_optlet))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
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

    argv += opt_info.index;
    if (error_info.errors || !*argv) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    int rv = 0;
    while (*argv) {
        rv = !sh_arith(shp, *argv);
        argv++;
    }
    return rv;
}
