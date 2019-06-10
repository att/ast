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
// ulimit [-HSacdfmnstuv] [limit]
//
//   David Korn
//   AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

#include "ast.h"
#include "b_ulimit.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "name.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"

#define HARD 2
#define SOFT 4

static_fn int ulimit_infof(Opt_t *op, Sfio_t *sp, const char *s, Optdisc_t *dp) {
    UNUSED(op);
    UNUSED(s);
    UNUSED(dp);
    const Limit_t *tp;

    for (tp = shtab_limits; tp->name; tp++) {
        sfprintf(sp, "[%c=%d:%s?The %s", tp->option, tp - shtab_limits + 1, tp->name,
                 tp->description);
        if (tp->type != LIM_COUNT) sfprintf(sp, " in %ss", e_units[tp->type]);
        sfprintf(sp, ".]");
    }
    return 1;
}

int b_ulimit(int argc, char *argv[], Shbltin_t *context) {
    char *limit;
    int mode = 0, n;
    unsigned long hit = 0;
    Shell_t *shp = context->shp;
    struct rlimit rlp;
    const Limit_t *tp;
    int label, unit, nosupport;
    rlim_t i;
    Optdisc_t disc;

    memset(&disc, 0, sizeof(disc));
    disc.version = OPT_VERSION;
    disc.infof = ulimit_infof;
    opt_info.disc = &disc;
    while ((n = optget(argv, sh_optulimit))) {
        switch (n) {
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
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
            default: {
                if (n < 0) {
                    hit |= (1L << (-(n + 1)));
                } else {
                    errormsg(SH_DICT, 2, e_notimp, opt_info.name);
                }
                break;
            }
        }
    }
    opt_info.disc = NULL;
    // Default to -f.
    limit = argv[opt_info.index];
    if (hit == 0) {
        for (n = 0; shtab_limits[n].option; n++) {
            if (shtab_limits[n].index == RLIMIT_FSIZE) {
                hit |= (1L << n);
                break;
            }
        }
    }
    // Only one option at a time for setting.
    label = (hit & (hit - 1));
    if (error_info.errors || (limit && label) || argc > opt_info.index + 1) {
        errormsg(SH_DICT, ERROR_usage(2), optusage(NULL));
        __builtin_unreachable();
    }
    if (mode == 0) mode = (HARD | SOFT);
    for (tp = shtab_limits; tp->name && hit; tp++, hit >>= 1) {
        if (!(hit & 1)) continue;
        nosupport = (n = tp->index) == RLIMIT_UNKNOWN;
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
