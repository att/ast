/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2013 AT&T Intellectual Property          *
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
// sleep delay
//
//   David Korn
//   AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"
#include "tmx.h"
#include "tv.h"

int b_sleep(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);
    char *cp;
    long double d = 0.0;
    Shell_t *shp = context->shp;
    int sflag = 0;
    time_t tloc = 0;
    char *last;
    int n;

    if (shp->subshell) sh_subfork();
    if (!(shp->sigflag[SIGALRM] & (SH_SIGFAULT | SH_SIGOFF))) sh_sigtrap(shp, SIGALRM);
    while ((n = optget(argv, sh_optsleep))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 's': {
                sflag = 1;
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
    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    argv += opt_info.index;
    cp = *argv;
    if (cp) {
        d = strtold(cp, &last);
        if (*last) {
            Time_t now = TMX_NOW, ns = 0;
            char *pp;
            if (*cp == 'P' || *cp == 'p') {
                ns = tmxdate(cp, &last, now);
            } else if (*last == '.' && shp->decomma && d == truncl(d)) {
                *(pp = last) = ',';
                if (!strchr(cp, '.')) d = strtold(cp, &last);
                *pp = '.';
                if (*last == 0) goto skip;
            } else if (*last != '.' && *last != ',') {
                if (*last && last > cp && last[1] == 0) {
                    switch (*last) {
                        case 'd': {
                            d *= 24;
                        }
                        // FALLTHRU
                        case 'h': {
                            d *= 60;
                        }
                        // FALLTHRU
                        case 'm': {
                            d *= 60;
                        }
                        // FALLTHRU
                        case 's': {
                            goto skip;
                        }
                        default: { break; }
                    }
                } else {
                    pp = sfprints("exact %s", cp);
                    if (pp) ns = tmxdate(pp, &last, now);
                    if (*last && (pp = sfprints("p%s", cp))) ns = tmxdate(pp, &last, now);
                }
            }
            if (*last) {
                errormsg(SH_DICT, ERROR_exit(1), e_number, *argv);
                __builtin_unreachable();
            }
            d = ns - now;
            d /= TMX_RESOLUTION;
        }
    skip:
        if (argv[1]) {
            errormsg(SH_DICT, ERROR_exit(1), e_oneoperand);
            __builtin_unreachable();
        }
    } else if (!sflag) {
        errormsg(SH_DICT, ERROR_exit(1), e_oneoperand);
        __builtin_unreachable();
    }
    if (d > .10) {
        time(&tloc);
        tloc += (time_t)(d + .5);
    }
    if (sflag && d == 0.0) {
        pause();
    } else {
        while (1) {
            time_t now;
            errno = 0;
            shp->lastsig = 0;
            sh_delay(d);
            if (sflag || tloc == 0 || errno != EINTR || shp->lastsig) break;
            sh_sigcheck(shp);
            if (tloc < (now = time(NULL))) break;
            d = tloc - now;
        }
    }
    return 0;
}

//
// Delay execution for time <t>.
//
void sh_delay(double t) {
    Shell_t *shp = sh_getinterp();
    int n = (int)t;
    Tv_t ts, tx;

    ts.tv_sec = n;
    ts.tv_nsec = 1000000000 * (t - (double)n);
    while (tvsleep(&ts, &tx) < 0 && errno == EINTR) {
        if (shp->trapnote & (SH_SIGSET | SH_SIGTRAP)) return;
        ts = tx;
    }
}
