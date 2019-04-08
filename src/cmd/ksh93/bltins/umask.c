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
//
// umask [-pS] [mask]
//
//   David Korn
//   dgkorn@gmail.com
//
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

#include "ast.h"
#include "defs.h"
#include "error.h"
#include "option.h"
#include "sfio.h"

// This has to be included after "shell.h".
#include "builtins.h"

int b_umask(int argc, char *argv[], Shbltin_t *context) {
    char *mask;
    int flag = 0;
    bool sflag = false, pflag = false;
    UNUSED(context);

    while ((argc = optget(argv, sh_optumask))) {
        switch (argc) {
            case 'p': {
                pflag = true;
                break;
            }
            case 'S': {
                sflag = true;
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
            default: { break; }
        }
    }
    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    argv += opt_info.index;
    mask = *argv;
    if (mask) {
        int c;
        if (isdigit(*mask)) {
            while ((c = *mask++)) {
                if (c >= '0' && c <= '7') {
                    flag = (flag << 3) + (c - '0');
                } else {
                    errormsg(SH_DICT, ERROR_exit(1), e_number, *argv);
                    __builtin_unreachable();
                }
            }
        } else {
            char *cp = mask;
            flag = sh_umask(0);
            c = strperm(cp, &cp, ~flag & 0777);
            if (*cp) {
                sh_umask(flag);
                errormsg(SH_DICT, ERROR_exit(1), e_format, mask);
                __builtin_unreachable();
            }
            flag = (~c & 0777);
        }
        sh_umask(flag);
    } else {
        char *prefix = pflag ? "umask " : "";
        flag = sh_umask(0);
        sh_umask(flag);
        if (sflag) {
            sfprintf(sfstdout, "%s%s\n", prefix, fmtperm(~flag & 0777));
        } else {
            sfprintf(sfstdout, "%s%0#4o\n", prefix, flag);
        }
    }
    return 0;
}
