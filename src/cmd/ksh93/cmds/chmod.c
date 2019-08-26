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
 * Glenn Fowler
 * AT&T Research
 *
 * chmod
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include <fts.h>  // OpenBSD and possibly others require the above includes first

#include "ast.h"
#include "ast_assert.h"
#include "builtins.h"
#include "error.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"

#ifndef ALLPERMS
#ifdef S_ISTXT
#define ALLPERMS (S_ISTXT | S_ISUID | S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO)
#else
#define ALLPERMS (S_ISVTX | S_ISUID | S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO)
#endif
#endif

#ifndef ENOSYS
#define ENOSYS EINVAL
#endif

int b_chmod(int argc, char **argv, Shbltin_t *context) {
    int (*chmodf)(const char *, mode_t);
    int mode;
    int flag;
    char *amode = NULL;
    struct stat st;
    char *last;
    int notify = 0;
    int ignore = 0;
    int show = 0;
    bool quiet = false;
    bool recursive = false;
    bool chlink = false;
    bool flag_L = false;
    bool flag_H = false;
    bool flag_P = false;

    if (cmdinit(argc, argv, context, ERROR_NOTIFY)) return -1;

    //
    // NOTE: we diverge from the normal optget boilerplate to allow `chmod -x path`, and similar
    // invocations, to terminate the loop even though they look like unrecognized short flags.
    //
    bool optget_done = false;
    while (!optget_done && (flag = optget(argv, sh_optchmod))) {
        switch (flag) {
            case 'c':
                notify = 1;
                break;
            case 'f':
                quiet = true;
                break;
            case 'h':
                chlink = true;
                break;
            case 'i':
                ignore = 1;
                break;
            case 'n':
                show = 1;
                break;
            case 'v':
                notify = 2;
                break;
            case 'F':
                if (stat(opt_info.arg, &st)) {
                    error(ERROR_exit(1), "%s: cannot stat", opt_info.arg);
                    __builtin_unreachable();
                }
                mode = st.st_mode;
                amode = "";
                break;
            case 'H':
                flag_H = true;
                break;
            case 'L':
                flag_L = true;
                break;
            case 'P':
                flag_P = true;
                break;
            case 'R':
                recursive = true;
                break;
            case ':':
                optget_done = true;  // probably a negative permission like `-x` or similar
                break;
            case '?':
                error(ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            default: { break; }
        }
    }
    argv += opt_info.index;

    if (error_info.errors || !*argv || (!amode && !*(argv + 1))) {
        error(ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    int fts_opts = FTS_PHYSICAL;
    if (recursive) {
        if (chlink) {
            error(ERROR_exit(1), "the -R and -h options may not be used together");
            __builtin_unreachable();
        }

        if (flag_H) {
            fts_opts = FTS_PHYSICAL | FTS_COMFOLLOW;
        } else if (flag_L) {
            fts_opts = FTS_LOGICAL;
        }
    } else {
        if (flag_H || flag_L || flag_P) {
            error(ERROR_exit(1), "options -H, -L, -P only useful with -R");
            __builtin_unreachable();
        }

        if (!chlink) fts_opts = FTS_PHYSICAL | FTS_COMFOLLOW;
    }

    if (ignore) ignore = umask(0);
    if (amode) {
        amode = NULL;
    } else {
        amode = *argv++;
        mode = strperm(amode, &last, 0);
        if (*last) {
            if (ignore) umask(ignore);
            error(ERROR_exit(1), "%s: invalid mode", amode);
            __builtin_unreachable();
        }
    }

    // One of FTS_LOGICAL or FTS_PHYSICAL must be set but not both.
    assert(fts_opts & FTS_LOGICAL || fts_opts & FTS_PHYSICAL);
    assert(!(fts_opts & FTS_LOGICAL && fts_opts & FTS_PHYSICAL));
    FTS *fts = fts_open(argv, fts_opts, NULL);
    if (!fts) {
        if (ignore) umask(ignore);
        error(ERROR_system(1), "%s: not found", *argv);
        __builtin_unreachable();
    }

    while (!bltin_checksig(context)) {
        FTSENT *ent = fts_read(fts);
        if (!ent) break;
        switch (ent->fts_info) {
            case FTS_SL:
            case FTS_SLNONE:
#if _lib_lchmod
                if (!chlink) break;
#else
                if (chlink && !quiet) {
                    errno = ENOSYS;
                    error(ERROR_system(0), "%s: cannot change symlink mode", ent->fts_path);
                }
                break;
#endif
                // FALLTHRU
            case FTS_F:
            case FTS_D:
#if _lib_lchmod
                chmodf = chlink ? lchmod : chmod;
#else
                assert(!chlink);
                chmodf = chmod;
#endif
                if (amode) mode = strperm(amode, &last, ent->fts_statp->st_mode);
                if (show || (*chmodf)(ent->fts_accpath, mode) >= 0) {
                    if (notify == 2 || (notify == 1 && (mode & ALLPERMS) !=
                                                           (ent->fts_statp->st_mode & ALLPERMS))) {
                        sfprintf(sfstdout, "%s: mode changed to %0.4o (%s)\n", ent->fts_path, mode,
                                 fmtmode(mode) + 1);
                    }
                } else if (!quiet) {
                    error(ERROR_system(0), "%s: cannot change mode", ent->fts_path);
                }
                // If this is a directory and `-R` wasn't used then don't descend into the dir.
                if (ent->fts_info == FTS_D && !recursive) fts_set(fts, ent, FTS_SKIP);
                break;
            case FTS_DC:
                if (!quiet) error(ERROR_warn(0), "%s: directory causes cycle", ent->fts_path);
                break;
            case FTS_DNR:
                if (!quiet) error(ERROR_system(0), "%s: cannot read directory", ent->fts_path);
                break;
            case FTS_ERR:
                if (!quiet) {
                    error(ERROR_system(0), "%s: %s", ent->fts_path, strerror(ent->fts_errno));
                }
                break;
            case FTS_NS:
                if (!quiet) error(ERROR_system(0), "%s: not found", ent->fts_path);
                break;
            default:
                break;
        }
    }
    fts_close(fts);
    if (ignore) umask(ignore);
    return error_info.errors != 0;
}
