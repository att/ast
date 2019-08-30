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

#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "jobs.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"

#define L_FLAG 1
#define S_FLAG 2
#define Q_FLAG JOB_QFLAG

//
//  The `kill` special builtin.
//
int b_kill(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);
    char *signame = NULL;
    int sig = SIGTERM;
    int flag = 0;
    int n;
    Shell_t *shp = context->shp;
    int usemenu = 0;

    while ((n = optget(argv, sh_optkill))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'n': {
                sig = (int)opt_info.num;
                goto endopts;
            }
            case 's': {
                flag |= S_FLAG;
                signame = opt_info.arg;
                goto endopts;
            }
            case 'L': {
                usemenu = -1;
            }
            // FALLTHRU
            case 'l': {
                flag |= L_FLAG;
                break;
            }
            case 'q': {
                flag |= Q_FLAG;
                shp->sigval = opt_info.num;
                if ((int)shp->sigval != shp->sigval) {
                    errormsg(SH_DICT, ERROR_exit(1), "%lld - too large for sizeof(integer)",
                             shp->sigval);
                    __builtin_unreachable();
                }
                break;
            }
            case ':': {
                if ((signame = argv[opt_info.index++]) &&
                    (sig = sig_number(shp, signame + 1)) >= 0) {
                    goto endopts;
                }
                opt_info.index--;
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                shp->sigval = 0;
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
        }
    }
endopts:
    argv += opt_info.index;
    if (*argv && strcmp(*argv, "--") == 0 && strcmp(*(argv - 1), "--") != 0) argv++;
    if (error_info.errors || flag == (L_FLAG | S_FLAG) || (!(*argv) && !(flag & L_FLAG))) {
        shp->sigval = 0;
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    // Just in case we send a kill -9 $$.
    sfsync(sfstderr);
    if (flag & L_FLAG) {
        if (!(*argv)) {
            sh_siglist(shp, sfstdout, usemenu);
        } else {
            while ((signame = *argv++)) {
                if (isdigit(*signame)) {
                    sh_siglist(shp, sfstdout, ((int)strtol(signame, NULL, 10) & 0177) + 1);
                } else {
                    if ((sig = sig_number(shp, signame)) < 0) {
                        shp->exitval = 2;
                        shp->sigval = 0;
                        errormsg(SH_DICT, ERROR_exit(1), e_nosignal, signame);
                        __builtin_unreachable();
                    }
                    sfprintf(sfstdout, "%d\n", sig);
                }
            }
        }
        return shp->exitval;
    }
    if (flag & S_FLAG) {
        sig = sig_number(shp, signame);
        if (sig < 0 || sig >= shp->gd->sigmax) {
            shp->exitval = 2;
            errormsg(SH_DICT, ERROR_exit(1), e_nosignal, signame);
            __builtin_unreachable();
        }
    }
    if (job_walk(shp, sfstdout, job_kill, sig | (flag & Q_FLAG), argv)) {
        shp->exitval = 1;
    }
    shp->sigval = 0;
    return shp->exitval;
}
