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
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "ast.h"
#include "builtins.h"
#include "error.h"
#include "optget_long.h"
#include "sfio.h"
#include "shcmd.h"

static const char *short_options = "fs:S:X";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {"sfsync", optget_no_arg, NULL, 'f'},
    {"fsync", optget_required_arg, NULL, 's'},
    {"syncfs", optget_required_arg, NULL, 'S'},
    {"sync", optget_no_arg, NULL, 'X'},
    {"all", optget_no_arg, NULL, 'X'},
    {NULL, 0, NULL, 0}};  // keep clang-format from compacting this table

#if !_lib_syncfs
int syncfs(int fd) {
    if (fcntl(fd, F_GETFL) >= 0) errno = ENOSYS;
    return -1;
}
#endif

int b_sync(int argc, char **argv, Shbltin_t *context) {
    int fsync_fd = -1;
    int syncfs_fd = -1;
    bool do_sfsync = 0;
    bool do_sync = 0;
    int opt;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    if (cmdinit(argc, argv, context, 0)) return -1;

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'f': {
                do_sfsync = 1;
                break;
            }
            case 's': {
                char *cp;
                int64_t n = strton64(optget_arg, &cp, NULL, 0);
                if (*cp || n < 0 || n > INT_MAX) {
                    builtin_usage_error(shp, cmd, "%s: invalid -N value", optget_arg);
                    return 2;
                }
                fsync_fd = n;
                break;
            }
            case 'S': {
                char *cp;
                int64_t n = strton64(optget_arg, &cp, NULL, 0);
                if (*cp || n < 0 || n > INT_MAX) {
                    builtin_usage_error(shp, cmd, "%s: invalid -N value", optget_arg);
                    return 2;
                }
                syncfs_fd = n;
                do_sfsync = 1;
                break;
            }
            case 'X': {
                do_sync = 1;
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

    if (*argv) {
        builtin_usage_error(shp, cmd, "unexpected arguments");
        return 2;
    }
    if (fsync_fd == -1 && syncfs_fd == -1) do_sync = do_sfsync = 1;
    if (do_sfsync && sfsync(NULL) < 0) error(ERROR_system(0), "sfsync(0) failed");
    if (fsync_fd >= 0 && fsync(fsync_fd) < 0) error(ERROR_system(0), "fsync(%d) failed", fsync_fd);
    if (syncfs_fd >= 0 && syncfs(syncfs_fd) < 0) {
        error(ERROR_system(0), "syncfs(%d) failed", syncfs_fd);
    }
    if (do_sync) sync();
    return error_info.errors != 0;
}
