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
#include <string.h>
#include <sys/resource.h>

#include "ast.h"
#include "ast_assert.h"
#include "b_ulimit.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "name.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"

#define HARD 2
#define SOFT 4

static const char *short_options = "abcdefilmnpqrstuvwxHMST";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {"as", 0, NULL, 'M'},
    {"core", 0, NULL, 'c'},
    {"cpu", 0, NULL, 't'},
    {"data", 0, NULL, 'd'},
    {"fsize", 0, NULL, 'f'},
    {"locks", 0, NULL, 'x'},
    {"memlock", 0, NULL, 'l'},
    {"msgqueue", 0, NULL, 'q'},
    {"nice", 0, NULL, 'e'},
    {"nofile", 0, NULL, 'n'},
    {"nproc", 0, NULL, 'u'},
    {"pipe", 0, NULL, 'p'},
    {"rss", 0, NULL, 'm'},
    {"rtprio", 0, NULL, 'r'},
    {"sbsize", 0, NULL, 'b'},
    {"sigpend", 0, NULL, 'i'},
    {"stack", 0, NULL, 's'},
    {"swap", 0, NULL, 'w'},
    {"threads", 0, NULL, 'T'},
    {"vmem", 0, NULL, 'v'},
    {NULL, 0, NULL, 0}};  // keep clang-format from compacting this table

//
// Builtin `ulimit` command.
//
int b_ulimit(int argc, char *argv[], Shbltin_t *context) {
    int mode = 0, n = 0, opt;
    unsigned long hit = 0;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    struct rlimit rlp;
    int label, unit, nosupport;
    rlim_t i;

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'H': {
                mode |= HARD;
                continue;
            }
            case 'S': {
                mode |= SOFT;
                continue;
            }
            case 'a': {
                hit = ~0;
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
            default: {
                // Find the short option in the limits table and set the corresponding bit in the
                // mask of options we've seen.
                const Limit_t *tp;
                for (tp = shtab_limits; tp->option; tp++) {
                    if (tp->option == opt) {
                        hit |= 1L << (tp - shtab_limits);
                        break;
                    }
                }
                assert(tp->option);  // we should always find the short flag in the table
            }
        }
    }

    char *limit = argv[optget_ind];
    // Default to -f.
    if (hit == 0) {
        for (int n = 0; shtab_limits[n].option; n++) {
            if (shtab_limits[n].index == RLIMIT_FSIZE) {
                hit = (1L << n);
                break;
            }
        }
    }
    // Only one option at a time for setting.
    if (argc > optget_ind + 1) {
        builtin_usage_error(shp, cmd, "too many arguments");
        return 2;
    }
    label = (hit & (hit - 1));
    if (limit && label) {
        builtin_usage_error(shp, cmd, "you can only set one option at a time");
        return 2;
    }
    if (mode == 0) mode = (HARD | SOFT);
    for (const Limit_t *tp = shtab_limits; tp->name && hit; tp++, hit >>= 1) {
        if (!(hit & 1)) continue;
        n = tp->index;
        nosupport = n == RLIMIT_UNKNOWN;
        unit = shtab_units[tp->type];
        if (limit) {
            if (shp->subshell && !shp->subshare) sh_subfork();
            if (strcmp(limit, e_unlimited) == 0) {
                i = INFINITY;
            } else {
                char *last;
                // An explicit suffix unit overrides the default.
                if ((i = strtol(limit, &last, 0)) != INFINITY && !*last) {
                    i *= unit;
                } else if ((i = strton64(limit, &last, NULL, 0)) == INFINITY || *last) {
                    if ((i = sh_strnum(shp, limit, &last, 2)) == INFINITY || *last) {
                        errormsg(SH_DICT, ERROR_system(1), e_number, limit);
                        __builtin_unreachable();
                    }
                    i *= unit;
                }
            }
            if (nosupport) {
                errormsg(SH_DICT, ERROR_system(1), e_readonly, tp->name);
                __builtin_unreachable();
            } else {
                if (getrlimit(n, &rlp) < 0) {
                    errormsg(SH_DICT, ERROR_system(1), e_number, limit);
                    __builtin_unreachable();
                }
                if (mode & HARD) rlp.rlim_max = i;
                if (mode & SOFT) rlp.rlim_cur = i;
                if (setrlimit(n, &rlp) < 0) {
                    errormsg(SH_DICT, ERROR_system(1), e_overlimit, limit);
                    __builtin_unreachable();
                }
            }
        } else {
            if (!nosupport) {
                if (getrlimit(n, &rlp) < 0) {
                    errormsg(SH_DICT, ERROR_system(1), e_number, limit);
                    __builtin_unreachable();
                }
                if (mode & HARD) i = rlp.rlim_max;
                if (mode & SOFT) i = rlp.rlim_cur;
            }
            if (label) {
                char tmp[32];
                if (tp->type != LIM_COUNT) {
                    sfsprintf(tmp, sizeof(tmp), "%s (%ss)", tp->description, e_units[tp->type]);
                } else {
                    sfsprintf(tmp, sizeof(tmp), "%s", tp->name);
                }
                sfprintf(sfstdout, "%-30s (-%c)  ", tmp, tp->option);
            }
            if (nosupport) {
                sfputr(sfstdout, e_nosupport, '\n');
            } else if (i != INFINITY) {
                i += (unit - 1);
                sfprintf(sfstdout, "%I*d\n", sizeof(i), i / unit);
            } else {
                sfputr(sfstdout, e_unlimited, '\n');
            }
        }
    }
    return 0;
}
