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
#include <sys/types.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "jobs.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"

#ifdef JOBS
//
// Builtin `jobs`.
//
int b_jobs(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);
    int n;
    int flag = 0;
    Shell_t *shp = context->shp;
    while ((n = optget(argv, sh_optjobs))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'l': {
                flag = JOB_LFLAG;
                break;
            }
            case 'n': {
                flag = JOB_NFLAG;
                break;
            }
            case 'p': {
                flag = JOB_PFLAG;
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

    argv += opt_info.index;
    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    if (*argv == 0) argv = NULL;
    if (job_walk(shp, sfstdout, job_list, flag, argv)) {
        errormsg(SH_DICT, ERROR_exit(1), e_no_job);
        __builtin_unreachable();
    }
    job_wait((pid_t)0);
    return shp->exitval;
}
#endif  // JOBS
