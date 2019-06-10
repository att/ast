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

static const char usage[] =
    "[-?\n@(#)$Id: chmod (AT&T Research) 2012-04-20 $\n]" USAGE_LICENSE
    "[+NAME?chmod - change the access permissions of files]"
    "[+DESCRIPTION?\bchmod\b changes the permission of each file "
    "according to mode, which can be either a symbolic representation "
    "of changes to make, or an octal number representing the bit "
    "pattern for the new permissions.]"
    "[+?Symbolic mode strings consist of one or more comma separated list "
    "of operations that can be perfomed on the mode. Each operation is of "
    "the form \auser\a \aop\a \aperm\a where \auser\a is zero or more of "
    "the following letters:]{"
    "[+u?User permission bits.]"
    "[+g?Group permission bits.]"
    "[+o?Other permission bits.]"
    "[+a?All permission bits. This is the default if none are specified.]"
    "}"
    "[+?The \aperm\a portion consists of zero or more of the following letters:]{"
    "[+r?Read permission.]"
    "[+s?Setuid when \bu\b is selected for \awho\a and setgid when \bg\b "
    "is selected for \awho\a.]"
    "[+w?Write permission.]"
    "[+x?Execute permission for files, search permission for directories.]"
    "[+X?Same as \bx\b except that it is ignored for files that do not "
    "already have at least one \bx\b bit set.]"
    "[+l?Exclusive lock bit on systems that support it. Group execute "
    "must be off.]"
    "[+t?Sticky bit on systems that support it.]"
    "}"
    "[+?The \aop\a portion consists of one or more of the following characters:]{"
    "[++?Cause the permission selected to be added to the existing "
    "permissions. | is equivalent to +.]"
    "[+-?Cause the permission selected to be removed to the existing "
    "permissions.]"
    "[+=?Cause the permission to be set to the given permissions.]"
    "[+&?Cause the permission selected to be \aand\aed with the existing "
    "permissions.]"
    "[+^?Cause the permission selected to be propagated to more "
    "restrictive groups.]"
    "}"
    "[+?Symbolic modes with the \auser\a portion omitted are subject to "
    "\bumask\b(2) settings unless the \b=\b \aop\a or the "
    "\b--ignore-umask\b option is specified.]"
    "[+?A numeric mode is from one to four octal digits (0-7), "
    "derived by adding up the bits with values 4, 2, and 1. "
    "Any omitted digits are assumed to be leading zeros. The "
    "first digit selects the set user ID (4) and set group ID "
    "(2) and save text image (1) attributes. The second digit "
    "selects permissions for the user who owns the file: read "
    "(4), write (2), and execute (1); the third selects permissions"
    "for other users in the file's group, with the same values; "
    "and the fourth for other users not in the file's group, with "
    "the same values.]"

    "[+?For symbolic links, by default, \bchmod\b changes the mode on the file "
    "referenced by the symbolic link, not on the symbolic link itself. "
    "The \b-h\b options can be specified to change the mode of the link. "
    "When traversing directories with \b-R\b, \bchmod\b either follows "
    "symbolic links or does not follow symbolic links, based on the "
    "options \b-H\b, \b-L\b, and \b-P\b.]"

    "[+?When the \b-c\b or \b-v\b options are specified, change notifications "
    "are written to standard output using the format, "
    "\b%s: mode changed to %0.4o (%s)\b, with arguments of the "
    "pathname, the numeric mode, and the resulting permission bits as "
    "would be displayed by the \bls\b command.]"

    "[+?For backwards compatibility, if an invalid option is given that is a valid "
    "symbolic mode specification, \bchmod\b treats this as a mode "
    "specification rather than as an option specification.]"

    "[H:metaphysical?Follow symbolic links for command arguments; otherwise don't "
    "follow symbolic links when traversing directories.]"
    "[L:logical|follow?Follow symbolic links when traversing directories.]"
    "[P:physical|nofollow?Don't follow symbolic links when traversing directories.]"
    "[R:recursive?Change the mode for files in subdirectories recursively.]"
    "[c:changes?Describe only files whose permission actually change.]"
    "[f:quiet|silent?Do not report files whose permissioins fail to change.]"
    "[h|l:symlink?Change the mode of symbolic links on systems that "
    "support \blchmod\b(2). Implies \b--physical\b.]"
    "[i:ignore-umask?Ignore the \bumask\b(2) value in symbolic mode "
    "expressions. This is probably how you expect \bchmod\b to work.]"
    "[n:show?Show actions but do not change any file modes.]"
    "[F:reference?Omit the \amode\a operand and use the mode of \afile\a "
    "instead.]:[file]"
    "[v:verbose?Describe changed permissions of all files.]"
    "\n"
    "\nmode file ...\n"
    "\n"
    "[+EXIT STATUS?]{"
    "[+0?All files changed successfully.]"
    "[+>0?Unable to change mode of one or more files.]"
    "}"
    "[+SEE ALSO?\bchgrp\b(1), \bchown\b(1), \blchmod\b(1), \btw\b(1), \bgetconf\b(1), "
    "\bls\b(1), \bumask\b(2)]";

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
    while (!optget_done && (flag = optget(argv, usage))) {
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
                                 fmtmode(mode, 1) + 1);
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
