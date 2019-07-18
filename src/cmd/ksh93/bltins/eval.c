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
#include "config_ast.h"  // IWYU pragma: keep

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "argnod.h"
#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "io.h"
#include "jobs.h"
#include "name.h"
#include "option.h"
#include "path.h"
#include "sfio.h"
#include "shcmd.h"
#include "shnodes.h"
#include "stk.h"
#include "variables.h"

int b_eval(int argc, char *argv[], Shbltin_t *context) {
    int n;
    Shell_t *shp = context->shp;
    UNUSED(argc);

    while ((n = optget(argv, sh_opteval))) {
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
    if (*argv && **argv) {
        sh_offstate(shp, SH_MONITOR);
        sh_eval(shp, sh_sfeval((const char **)argv), 0);
    }
    return shp->exitval;
}
