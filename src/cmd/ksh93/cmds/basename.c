/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1992-2012 AT&T Intellectual Property          *
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
 * namebase pathname [suffix]
 *
 * print the namebase of a pathname
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "builtins.h"
#include "error.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"

static void namebase(Sfio_t *outfile, char *pathname, char *suffix) {
    char *first, *last;
    int n = 0;
    for (first = last = pathname; *last; last++) {
        ;
    }
    /* back over trailing '/' */
    if (last > first) {
        while (*--last == '/' && last > first) {
            ;
        }
    }
    if (last == first && *last == '/') {
        /* all '/' or "" */
        if (*first == '/') {
            if (*++last == '/') { /* keep leading // */
                last++;
            }
        }
    } else {
        for (first = last++; first > pathname && *first != '/'; first--) {
            ;
        }
        if (*first == '/') first++;
        /* check for trailing suffix */
        if (suffix && (n = strlen(suffix)) && n < (last - first)) {
            if (memcmp(last - n, suffix, n) == 0) last -= n;
        }
    }
    if (last > first) sfwrite(outfile, first, last - first);
    sfputc(outfile, '\n');
}

int b_basename(int argc, char **argv, Shbltin_t *context) {
    char *string;
    char *suffix = NULL;
    int all = 0;
    int n;

    if (cmdinit(argc, argv, context, 0)) return -1;
    while ((n = optget(argv, sh_optbasename))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'a':
                all = 1;
                break;
            case 's':
                all = 1;
                suffix = opt_info.arg;
                break;
            case ':':
                error(2, "%s", opt_info.arg);
                break;
            case '?':
                error(ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
        }
    }
    argv += opt_info.index;
    argc -= opt_info.index;
    if (error_info.errors || argc < 1 || (!all && argc > 2)) {
        error(ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    if (!all) {
        namebase(sfstdout, argv[0], argv[1]);
    } else {
        while ((string = *argv++)) namebase(sfstdout, string, suffix);
    }
    return 0;
}
