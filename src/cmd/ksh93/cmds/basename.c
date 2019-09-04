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

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "error.h"
#include "sfio.h"
#include "shcmd.h"

static const char *short_options = ":as:";
static const struct option long_options[] = {{"help", 0, NULL, 1},  // all builtins supports --help
                                             {"all", 0, NULL, 'a'},
                                             {"suffix", 0, NULL, 's'},
                                             {NULL, 0, NULL, 0}};

static void namebase(Sfio_t *outfile, char *pathname, char *suffix) {
    char *first, *last;
    int n = 0;
    for (first = last = pathname; *last; last++) {
        ;
    }
    if (last > first) {  // back over trailing '/'
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
    int opt;
    char *string;
    char *suffix = NULL;
    bool all = false;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    optind = 1;
    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'a': {
                all = true;
                break;
            }
            case 's': {
                all = true;
                suffix = optarg;
                break;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[optind - 1]);
                return 2;
            }
            case '?': {
                builtin_unknown_option(shp, cmd, argv[optind - 1]);
                return 2;
            }
            default: { abort(); }
        }
    }
    argv += optind;
    argc -= optind;
    if (argc < 1 || (!all && argc > 2)) {
        builtin_print_help(shp, cmd);
        return 2;
    }

    if (!all) {
        namebase(sfstdout, argv[0], argv[1]);
    } else {
        while ((string = *argv++)) namebase(sfstdout, string, suffix);
    }
    return 0;
}
