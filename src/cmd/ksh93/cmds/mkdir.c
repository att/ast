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
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ast.h"
#include "builtins.h"
#include "error.h"
#include "shcmd.h"

#define DIRMODE (S_IRWXU | S_IRWXG | S_IRWXO)

static const char *short_options = "+:m:pv";
static const struct option long_options[] = {
    {"help", no_argument, NULL, 1},  // all builtins support --help
    {"mode", required_argument, NULL, 'm'},
    {"verbose", no_argument, NULL, 'v'},
    {NULL, 0, NULL, 0}};

int b_mkdir(int argc, char **argv, Shbltin_t *context) {
    char *path;
    int n, opt;
    mode_t mode = DIRMODE;
    mode_t mask = 0;
    int mflag = 0;
    int pflag = 0;
    int vflag = 0;
    int made;
    char *part;
    mode_t dmode;
    struct stat st;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    if (cmdinit(argc, argv, context, 0)) return -1;
    optind = opterr = 0;
    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'm':
                mflag = 1;
                mode = strperm(optarg, &part, mode);
                if (*part) error(ERROR_exit(0), "%s: invalid mode", optarg);
                break;
            case 'p':
                pflag = 1;
                break;
            case 'v':
                vflag = 1;
                break;
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

    if (!*argv) {
        builtin_usage_error(shp, cmd, "you must provide at least one directory");
        return 2;
    }

    mask = umask(0);
    if (mflag || pflag) {
        dmode = DIRMODE & ~mask;
        if (!mflag) mode = dmode;
        dmode |= S_IWUSR | S_IXUSR;
    } else {
        mode &= ~mask;
        umask(mask);
        mask = 0;
    }
    while (*argv) {
        path = *argv++;
        if (!mkdir(path, mode)) {
            if (vflag) error(0, "%s: directory created", path);
            made = 1;
        } else if (!pflag || !(errno == ENOENT || errno == EEXIST || errno == ENOTDIR)) {
            error(ERROR_system(0), "%s:", path);
            continue;
        } else if (errno == EEXIST) {
            continue;
        } else {
            /*
             * -p option, preserve intermediates
             * first eliminate trailing /'s
             */

            made = 0;
            n = strlen(path);
            while (n > 0 && path[--n] == '/') {
                ;  // empty loop
            }
            path[n + 1] = 0;
            for (part = path, n = *part; n;) {
                /* skip over slashes */
                while (*part == '/') part++;
                /* skip to next component */
                while ((n = *part) && n != '/') part++;
                *part = 0;
                if (mkdir(path, n ? dmode : mode) < 0 && errno != EEXIST &&
                    access(path, F_OK) < 0) {
                    error(ERROR_system(0), "%s: cannot create intermediate directory", path);
                    *part = n;
                    break;
                }
                if (vflag) error(0, "%s: directory created", path);
                *part = n;
                if (!(*part)) {
                    made = 1;
                    break;
                }
            }
        }
        if (made && (mode & (S_ISVTX | S_ISUID | S_ISGID))) {
            if (stat(path, &st)) {
                error(ERROR_system(0), "%s: cannot stat", path);
                break;
            }
            if ((st.st_mode & (S_ISVTX | S_ISUID | S_ISGID)) !=
                    (mode & (S_ISVTX | S_ISUID | S_ISGID)) &&
                chmod(path, mode)) {
                char *st_mode_str = strdup(fmtperm(st.st_mode & (S_ISVTX | S_ISUID | S_ISGID)));
                error(ERROR_system(0), "%s: cannot change mode from %s to %s", path, st_mode_str,
                      fmtperm(mode));
                free(st_mode_str);
                break;
            }
        }
    }
    if (mask) umask(mask);
    return error_info.errors != 0;
}
