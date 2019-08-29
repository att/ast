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

#include <stdlib.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "jobs.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"

#ifdef JOBS
//
// Builtin `disown` command.
//
int b_disown(int n, char *argv[], Shbltin_t *context) {
    Shell_t *shp = context->shp;

    while ((n = optget(argv, sh_optdisown))) {
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
    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    argv += opt_info.index;
    if (!sh_isoption(shp, SH_MONITOR) || !job.jobcontrol) {
        if (sh_isstate(shp, SH_INTERACTIVE)) {
            errormsg(SH_DICT, ERROR_exit(1), e_no_jctl);
            __builtin_unreachable();
        }
        return 1;
    }
    if (*argv == 0) argv = NULL;
    if (job_walk(shp, sfstdout, job_switch, 'd', argv)) {
        errormsg(SH_DICT, ERROR_exit(1), e_no_job);
        __builtin_unreachable();
    }
    return shp->exitval;
}
#endif  // JOBS
