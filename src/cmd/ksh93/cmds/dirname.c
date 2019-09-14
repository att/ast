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
 * dirname path [suffix]
 *
 * print the dirname of a pathname
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <getopt.h>
#include <limits.h>
#include <stdlib.h>

#include "ast.h"
#include "builtins.h"
#include "error.h"
#include "sfio.h"
#include "shcmd.h"

static const char *short_options = "+:frx";
static const struct option long_options[] = {
    {"help", no_argument, NULL, 1},  // all builtins support --help
    {"file", no_argument, NULL, 'f'},
    {"relative", no_argument, NULL, 'r'},
    {"executable", no_argument, NULL, 'x'},
    {NULL, 0, NULL, 0}};

static void l_dirname(Sfio_t *outfile, const char *pathname) {
    const char *last;
    /* go to end of path */
    for (last = pathname; *last; last++) {
        ;
    }
    /* back over trailing '/' */
    while (last > pathname && *--last == '/') {
        ;
    }
    /* back over non-slash chars */
    for (; last > pathname && *last != '/'; last--) {
        ;
    }
    if (last == pathname) {
        /* all '/' or "" */
        if (*pathname != '/') last = pathname = ".";
    } else {
        /* back over trailing '/' */
        for (; *last == '/' && last > pathname; last--) {
            ;
        }
    }
    /* preserve // */
    if (last != pathname && pathname[0] == '/' && pathname[1] == '/') {
        while (pathname[2] == '/' && pathname < last) pathname++;
        if (last != pathname && pathname[0] == '/' && pathname[1] == '/') {
            pathname++;
        }
    }
    sfwrite(outfile, pathname, last + 1 - pathname);
    sfputc(outfile, '\n');
}

int b_dirname(int argc, char **argv, Shbltin_t *context) {
    int mode = 0;
    int opt;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    if (cmdinit(argc, argv, context, 0)) return -1;

    optind = 0;
    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'f': {
                mode |= PATH_REGULAR;
                break;
            }
            case 'r': {
                mode &= ~PATH_REGULAR;
                mode |= PATH_READ;
                break;
            }
            case 'x': {
                mode |= PATH_EXECUTE;
                break;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[opterr]);
                return 2;
            }
            case '?': {
                builtin_unknown_option(shp, cmd, argv[opterr]);
                return 2;
            }
            default: { abort(); }
        }
    }
    argv += optind;
    argc -= optind;
    if (argc != 1) {
        builtin_usage_error(shp, cmd, "expected exactly one argument, got %d", argc);
        return 2;
    }

    if (!mode) {
        l_dirname(sfstdout, argv[0]);
        return 0;
    }

    char path[PATH_MAX];
    char *p = pathpath(argv[0], "", mode, path, sizeof(path));
    if (p) {
        sfputr(sfstdout, path, '\n');
    } else {
        // How is this possible? A filesystem problem of some sort? Such as the CWD no longer being
        // valid?
        error(1 | ERROR_WARNING, "%s: relative path not found", argv[0]);
    }
    return 0;
}
