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
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "jobs.h"
#include "sfio.h"
#include "shcmd.h"

#define L_FLAG 1
#define S_FLAG 2
#define Q_FLAG JOB_QFLAG

static const char *short_options = "+:ln:q:s:L";
static const struct option long_options[] = {
    {"help", no_argument, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
//  The `kill` special builtin.
//
int b_kill(int argc, char *argv[], Shbltin_t *context) {
    int opt;
    char *signame = NULL;
    int sig = SIGTERM;
    int flag = 0;
    int usemenu = 0;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    // We use `getopt_long_only()` rather than `getopt_long()` to facilitate handling integers that
    // might otherwise look like a flag. Or signal names which we want to recognize with a single
    // leading hyphen.
    optind = opterr = 0;
    bool done = false;
    while (!done && (opt = getopt_long_only(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'n': {
                char *cp;
                int64_t n = strton64(optarg, &cp, NULL, 0);
                if (*cp || n < 0 || n > INT_MAX) {
                    errormsg(SH_DICT, ERROR_exit(0), "%s: invalid value for -n", optarg);
                    return 2;
                }
                sig = n;
                done = true;
                break;
            }
            case 's': {
                flag |= S_FLAG;
                signame = optarg;
                done = true;
                break;
            }
            case 'L': {
                usemenu = -1;
                flag |= L_FLAG;
                break;
            }
            case 'l': {
                flag |= L_FLAG;
                break;
            }
            case 'q': {
                char *cp;
                int64_t n = strton64(optarg, &cp, NULL, 0);
                if (*cp || n < 0 || n > INT_MAX) {
                    errormsg(SH_DICT, ERROR_exit(0), "%s: invalid value for -q", optarg);
                    return 2;
                }
                flag |= Q_FLAG;
                shp->sigval = n;
                break;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[optind - 1]);
                return 2;
            }
            case '?': {
                // See if the flag is an integer that should be treated as a signal number.
                char *cp;
                int64_t n = strton64(argv[optind - 1] + 1, &cp, NULL, 0);
                if (!*cp && n >= 0 && n <= MAX_SIGNUM) {
                    sig = n;
                    done = true;
                    break;
                }
                // See if the flag is a signal name.
                sig = sig_number(shp, argv[optind - 1] + 1);
                if (sig >= 0) {
                    done = true;
                    break;
                }
                // Nope. It looks like an invalid flag.
                builtin_unknown_option(shp, cmd, argv[optind - 1]);
                return 2;
            }
            default: { abort(); }
        }
    }
    argv += optind;

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
