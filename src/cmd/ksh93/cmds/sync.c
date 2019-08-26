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
#include <stdbool.h>
#include <unistd.h>

#include "builtins.h"
#include "error.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"

#if !_lib_syncfs
int syncfs(int fd) {
    if (fcntl(fd, F_GETFL) >= 0) errno = ENOSYS;
    return -1;
}
#endif

int b_sync(int argc, char **argv, Shbltin_t *context) {
    UNUSED(argc);
    int fsync_fd = -1;
    int syncfs_fd = -1;
    bool do_sfsync = 0;
    bool do_sync = 0;
    int n;

    if (cmdinit(argc, argv, context, 0)) return -1;
    while ((n = optget(argv, sh_optsync))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'f':
                do_sfsync = 1;
                break;
            case 's':
                fsync_fd = opt_info.num;
                break;
            case 'S':
                syncfs_fd = opt_info.num;
                do_sfsync = 1;
                break;
            case 'X':
                do_sync = 1;
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
    if (error_info.errors || *argv) {
        error(ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
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
