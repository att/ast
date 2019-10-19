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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"

static const char *short_options = "alp";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
//  The `trap` special builtin.
//
int b_trap(int argc, char *argv[], Shbltin_t *context) {
    char *arg = NULL;
    int opt, clear;
    bool pflag = false, dflag = false, aflag = false, lflag = false;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'a': {
                aflag = true;
                break;
            }
            case 'p': {
                pflag = true;
                break;
            }
            case 'l': {
                lflag = true;
                break;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            case '?': {
                builtin_unknown_option(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            default: { abort(); }
        }
    }
    argv += optget_ind;

    if (pflag && aflag) {
        errormsg(SH_DICT, ERROR_usage(2), "-a and -p are mutually exclusive");
        __builtin_unreachable();
    }
    if (lflag) {
        sh_siglist(shp, sfstdout, -1);
        return 0;
    }
    arg = *argv;
    if (arg) {
        char *action = arg;
        if (!dflag && !pflag) {
            // First argument all digits or - means clear.
            while (isdigit(*arg)) arg++;
            clear = (arg != action && *arg == 0);
            if (!clear) {
                ++argv;
                if (*action == '-' && action[1] == 0) {
                    clear++;
                    //
                    // NOTE: 2007-11-26: workaround for tests/signal.sh. If function semantics can
                    // be worked out then it may merit a -d,--default option.
                    //
                } else if (*action == '+' && action[1] == 0 && shp->st.self == &shp->global) {
                    clear++;
                    dflag = true;
                }
            }
            if (!argv[0]) {
                errormsg(SH_DICT, ERROR_exit(1), e_condition);
                __builtin_unreachable();
            }
        }
        while ((arg = *argv++)) {
            int sig = sig_number(shp, arg);
            if (sig < 0) {
                errormsg(SH_DICT, 2, e_trap, arg);
                return 1;
            }
            // Internal traps.
            if (sig & SH_TRAP) {
                char **trap = (shp->st.otrap ? shp->st.otrap : shp->st.trap);
                sig &= ~SH_TRAP;
                if (sig > SH_DEBUGTRAP) {
                    errormsg(SH_DICT, 2, e_trap, arg);
                    return 1;
                }
                if (pflag) {
                    arg = trap[sig];
                    if (arg) sfputr(sfstdout, arg, '\n');
                    continue;
                }
                shp->st.otrap = NULL;
                arg = shp->st.trap[sig];
                shp->st.trap[sig] = 0;
                if (!clear && *action) {
                    char *cp = action;
                    if (aflag) {
                        size_t off = stktell(shp->stk);
                        sfprintf(shp->stk, "%s;%s%c", cp, arg, 0);
                        cp = stkptr(shp->stk, off);
                        stkseek(shp->stk, off);
                    }
                    shp->st.trap[sig] = strdup(cp);
                }
                if (sig == SH_DEBUGTRAP) {
                    if (shp->st.trap[sig]) {
                        shp->trapnote |= SH_SIGTRAP;
                    } else {
                        shp->trapnote = 0;
                    }
                }
                if (sig == SH_ERRTRAP) {
                    if (clear) {
                        shp->errtrap = 0;
                    } else {
                        if (!shp->fn_depth || shp->end_fn) shp->errtrap = 1;
                    }
                }
                if (arg) free(arg);
                continue;
            }
            if (sig >= shp->gd->sigmax) {
                errormsg(SH_DICT, 2, e_trap, arg);
                return 1;
            } else if (pflag) {
                char **trapcom = (shp->st.otrapcom ? shp->st.otrapcom : shp->st.trapcom);
                arg = trapcom[sig];
                if (arg) sfputr(sfstdout, arg, '\n');
            } else if (clear) {
                sh_sigclear(shp, sig);
                if (sig == 0) shp->exittrap = 0;
                if (dflag) sh_signal(sig, (sh_sigfun_t)SIG_DFL);
            } else {
                if (sig >= shp->st.trapmax) shp->st.trapmax = sig + 1;
                arg = shp->st.trapcom[sig];
                // Empty trap handler
                shp->st.trapcom[sig] = strdup("");
                sh_sigtrap(shp, sig);
                if (!(shp->sigflag[sig] & SH_SIGOFF)) {
                    char *cp = action;
                    if (aflag && arg && arg != Empty) {
                        size_t off = stktell(shp->stk);
                        sfprintf(shp->stk, "%s;%s%c", cp, arg, 0);
                        cp = stkptr(shp->stk, off);
                        stkseek(shp->stk, off);
                    }
                    shp->st.trapcom[sig] = strdup(cp);
                }
                if (arg && arg != Empty) free(arg);
                if (sig == 0 && (!shp->fn_depth || shp->end_fn)) shp->exittrap = 1;
            }
        }
    } else {  // print out current traps
        sh_siglist(shp, sfstdout, -2);
    }
    return 0;
}
