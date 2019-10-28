/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1992-2013 AT&T Intellectual Property          *
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
 *               Glenn Fowler <glenn.s.fowler@gmail.com>                *
 *                    David Korn <dgkorn@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
/*
 * David Korn
 * AT&T Bell Laboratories
 *
 * output the beginning portion of one or more files
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"
#include "stdlib.h"

static const char *short_options = "#n:c:qs:v";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {"lines", optget_required_arg, NULL, 'n'},
    {"bytes", optget_required_arg, NULL, 'c'},
    {"skip", optget_required_arg, NULL, 's'},
    {"quiet", optget_no_arg, NULL, 'q'},
    {"silent", optget_no_arg, NULL, 'q'},
    {"verbose", optget_no_arg, NULL, 'v'},
    {NULL, 0, NULL, 0}};

//
// Builtin `head` command.
//
int b_head(int argc, char **argv, Shbltin_t *context) {
    static const char header_fmt[] = "\n==> %s <==\n";

    Sfio_t *fp;
    int opt;
    char *cp;
    off_t keep = 10;
    off_t skip = 0;
    off_t moved;
    int delim = '\n';
    int header = 1;
    char *format = (char *)header_fmt + 1;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    if (cmdinit(argc, argv, context, 0)) return -1;

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case -2: {
                // Test against `OFF_MAX - 1` rather than `OFF_MAX` to avoid lint about a constant
                // expression never being false if `off_t` is the same as `int64_t`. In practice we
                // don't care that the user can't specify the max offset since that will never
                // occur. We do want the test in case `off_t` is only 32 bits.
                if (optget_num < 0 || optget_num > OFF_MAX - 1) {
                    errormsg(SH_DICT, ERROR_exit(0), "%s: invalid value for -n", optget_arg);
                    return 2;
                }
                keep = optget_num;
                delim = '\n';
                break;
            }
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'c': {
                char *cp;
                int64_t n = strton64(optget_arg, &cp, NULL, 0);
                if (*cp || n < 0 || n > OFF_MAX) {
                    errormsg(SH_DICT, ERROR_exit(0), "%s: invalid value for -c", optget_arg);
                    return 2;
                }
                keep = n;
                delim = -1;
                break;
            }
            case 'n': {
                char *cp;
                int64_t n = strton64(optget_arg, &cp, NULL, 0);
                if (*cp || n < 0 || n > OFF_MAX) {
                    errormsg(SH_DICT, ERROR_exit(0), "%s: invalid value for -n", optget_arg);
                    return 2;
                }
                keep = n;
                delim = '\n';
                break;
            }
            case 'q': {
                header = argc;
                break;
            }
            case 'v': {
                header = 0;
                break;
            }
            case 's': {
                char *cp;
                int64_t n = strton64(optget_arg, &cp, NULL, 0);
                if (*cp || n < 0 || n > OFF_MAX) {
                    errormsg(SH_DICT, ERROR_exit(0), "%s: invalid value for -s", optget_arg);
                    return 2;
                }
                skip = n;
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
    argc -= optget_ind;

    cp = *argv;
    if (cp) argv++;
    do {
        if (!cp || !strcmp(cp, "-")) {
            cp = "/dev/stdin";
            fp = sfstdin;
            sfset(fp, SF_SHARE, 1);
        } else if (!(fp = sfopen(NULL, cp, "r"))) {
            error(ERROR_system(0), "%s: cannot open", cp);
            continue;
        }
        if (argc > header) sfprintf(sfstdout, format, cp);
        format = (char *)header_fmt;
        if (skip > 0) {
            if ((moved = sfmove(fp, NULL, skip, delim)) < 0 && !ERROR_PIPE(errno) &&
                errno != EINTR) {
                error(ERROR_system(0), "%s: skip error", cp);
            }
            if (delim >= 0 && moved < skip) goto next;
        }
        moved = sfmove(fp, sfstdout, keep, delim);
        if ((moved < 0 && !ERROR_PIPE(errno) && errno != EINTR) ||
            (delim >= 0 && moved < keep && sfmove(fp, sfstdout, SF_UNBOUND, -1) < 0 &&
             !ERROR_PIPE(errno) && errno != EINTR)) {
            error(ERROR_system(0), "%s: read error", cp);
        }
    next:
        if (fp != sfstdin) sfclose(fp);
        cp = *argv++;
    } while (cp);
    if (sfsync(sfstdout)) error(ERROR_system(0), "write error");
    return error_info.errors != 0;
}
