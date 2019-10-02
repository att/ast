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

#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "jobs.h"
#include "sfio.h"
#include "shcmd.h"

#ifdef JOBS

static const char *short_options = "+:lnp";
static const struct option long_options[] = {
    {"help", no_argument, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
// Builtin `jobs`.
//
int b_jobs(int argc, char *argv[], Shbltin_t *context) {
    int opt;
    int flag = 0;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    // We use `getopt_long_only()` rather than `getopt_long()` to facilitate handling negative
    // integers that might otherwise look like a flag.
    optind = opterr = 0;
    bool done = false;
    while (!done && (opt = getopt_long_only(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
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
                builtin_missing_argument(shp, cmd, argv[optind - 1]);
                return 2;
            }
            case '?': {
                if (!strmatch(argv[optind - 1], "[+-]+([0-9])")) {
                    builtin_unknown_option(shp, cmd, argv[optind - 1]);
                    return 2;
                }
                optind--;
                done = true;
                break;
            }
            default: { abort(); }
        }
    }

    argv += optind;
    if (!*argv) argv = NULL;
    if (job_walk(shp, sfstdout, job_list, flag, argv)) {
        errormsg(SH_DICT, ERROR_exit(1), e_no_job);
        __builtin_unreachable();
    }
    job_wait((pid_t)0);
    return shp->exitval;
}
#endif  // JOBS
