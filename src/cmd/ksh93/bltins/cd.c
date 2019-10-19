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
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "argnod.h"
#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "name.h"
#include "optget_long.h"
#include "path.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"
#include "variables.h"

static const char *short_options = "f:LP@";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
// Invalidate path name bindings to relative paths.
//
static_fn void invalidate(Namval_t *np, void *data) {
    Pathcomp_t *pp = (Pathcomp_t *)FETCH_VT(np->nvalue, const_cp);
    UNUSED(data);

    if (pp && *pp->name != '/') _nv_unset(np, 0);
}

//
//  The `cd` special builtin.
//
int b_cd(int argc, char *argv[], Shbltin_t *context) {
    int opt;
    char *dir;
    Pathcomp_t *cdpath = NULL;
    const char *dp;
    const char *scoped_dir;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    int saverrno = 0;
    int rval, i, j;
    bool fflag = false, pflag = false;
    char *oldpwd;
    int dirfd = shp->pwdfd;
    int newdirfd;
    Namval_t *opwdnod, *pwdnod;

    if (sh_isoption(shp, SH_RESTRICTED)) {
        errormsg(SH_DICT, ERROR_exit(1), e_restricted + 4);
        __builtin_unreachable();
    }

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'f': {
                char *cp;
                fflag = true;
                int64_t n = strton64(optget_arg, &cp, NULL, 0);
                if (*cp || n < 0 || n > INT_MAX) {
                    errormsg(SH_DICT, ERROR_exit(0), "Invalid dirfd value: %s", optget_arg);
                    return 2;
                }
                dirfd = n;
                break;
            }
            case 'L': {
                pflag = false;
                break;
            }
            case 'P': {
                pflag = true;
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
    dir = argv[0];
    if (argc > 2) {
        builtin_usage_error(shp, cmd, "Too many arguments (expected at most two args)");
        return 2;
    }
    shp->pwd = path_pwd(shp);
    oldpwd = (char *)shp->pwd;
    opwdnod = (shp->subshell ? sh_assignok(VAR_OLDPWD, 1) : VAR_OLDPWD);
    pwdnod = (shp->subshell ? sh_assignok(VAR_PWD, 1) : VAR_PWD);
    if (dirfd != shp->pwdfd && dir == 0) dir = (char *)e_dot;
    if (argc == 2) {
        dir = sh_substitute(shp, oldpwd, dir, argv[1]);
    } else if (!dir) {
        struct passwd *pw;
        dir = nv_getval(VAR_HOME);
        if (!dir && (pw = getpwuid(geteuid()))) dir = pw->pw_dir;
    } else if (*dir == '-' && dir[1] == 0) {
        dir = FETCH_VT(sh_scoped(shp, opwdnod)->nvalue, cp);
    }

    if (!dir || *dir == 0) {
        errormsg(SH_DICT, ERROR_exit(1), argc == 2 ? e_subst + 4 : e_direct);
        __builtin_unreachable();
    }
#if __CYGWIN__
    if (*dir != '/' && (dir[1] != ':'))
#else
    if (dirfd == shp->pwdfd && *dir != '/')
#endif  // __CYGWIN__
    {
        cdpath = shp->cdpathlist;
        if (!cdpath) {
            dp = FETCH_VT(sh_scoped(shp, VAR_CDPATH)->nvalue, const_cp);
            if (dp) {
                cdpath = path_addpath(shp, NULL, dp, PATH_CDPATH);
                if (cdpath) {
                    shp->cdpathlist = cdpath;
                    cdpath->shp = shp;
                }
            }
        }
        if (!oldpwd) oldpwd = path_pwd(shp);
    }
    if (dirfd == shp->pwdfd && *dir != '/') {
        // Check for leading ..
        char *cp;

        j = sfprintf(shp->strbuf, "%s", dir);
        cp = sfstruse(shp->strbuf);
        pathcanon(cp, j + 1, 0);
        if (cp[0] == '.' && cp[1] == '.' && (cp[2] == '/' || cp[2] == 0)) {
            if (!shp->strbuf2) shp->strbuf2 = sfstropen();
            j = sfprintf(shp->strbuf2, "%s/%s", oldpwd, cp);
            dir = sfstruse(shp->strbuf2);
            pathcanon(dir, j + 1, 0);
        }
    }
    rval = -1;
    do {
        dp = cdpath ? cdpath->name : "";
        cdpath = path_nextcomp(shp, cdpath, dir, 0);
#if __CYGWIN__
        if (*stkptr(stkstd, PATH_OFFSET + 1) == ':' && isalpha(*stkptr(stkstd, PATH_OFFSET))) {
            *stkptr(stkstd, PATH_OFFSET + 1) = *stkptr(stkstd, PATH_OFFSET);
            *stkptr(stkstd, PATH_OFFSET) = '/';
        }
#endif  // __CYGWIN__
        if (*stkptr(stkstd, PATH_OFFSET) != '/' && dirfd == shp->pwdfd) {
            char *last = (char *)stkfreeze(stkstd, 1);
            stkseek(stkstd, PATH_OFFSET);
            sfputr(stkstd, oldpwd, 0);
            --stkstd->next;
            // Don't add '/' of oldpwd is / itself.
            if (*oldpwd != '/' || oldpwd[1]) sfputc(stkstd, '/');
            sfputr(stkstd, last + PATH_OFFSET, 0);
            --stkstd->next;
            sfputc(stkstd, 0);
        }
        if (!fflag && !pflag) {
            char *cp;
            stkseek(stkstd, PATH_MAX + PATH_OFFSET);
            cp = stkptr(stkstd, PATH_OFFSET);
            if (*cp == '/' && !pathcanon(cp, PATH_MAX, PATH_ABSOLUTE | PATH_DOTDOT)) continue;
        }

        newdirfd = sh_diropenat(shp, dirfd, path_relative(shp, stkptr(stkstd, PATH_OFFSET)));
        if (newdirfd >= 0) {
            // chdir for directories on HSM/tapeworms may take minutes.
            rval = sh_fchdir(newdirfd);
            if (rval >= 0) {
                if (shp->pwdfd >= 0) sh_close(shp->pwdfd);
                shp->pwdfd = newdirfd;
                goto success;
            }
            sh_close(newdirfd);
        } else {
#if O_SEARCH
            rval = newdirfd;
#else
            rval = sh_chdir(path_relative(shp, stkptr(stkstd, PATH_OFFSET)));
            if (rval >= 0 && shp->pwdfd >= 0) {
                sh_close(shp->pwdfd);
                shp->pwdfd = AT_FDCWD;
            }
#endif
        }
        if (saverrno == 0) saverrno = errno;
    } while (cdpath);

    if (rval < 0 && *dir == '/' && *(path_relative(shp, stkptr(stkstd, PATH_OFFSET))) != '/') {
        rval = newdirfd = sh_diropenat(shp, shp->pwdfd, dir);
        if (newdirfd >= 0) {
            // chdir for directories on HSM/tapeworms may take minutes.
            rval = sh_fchdir(newdirfd);
            if (rval >= 0) {
                if (shp->pwdfd >= 0) sh_close(shp->pwdfd);
                shp->pwdfd = newdirfd;
                goto success;
            }
            sh_close(newdirfd);
        }
#if !O_SEARCH
        else if (sh_chdir(dir) >= 0 && shp->pwdfd >= 0) {
            sh_close(shp->pwdfd);
            shp->pwdfd = AT_FDCWD;
        }
#endif
    }
    // Use absolute chdir() if relative chdir() fails.
    if (rval < 0) {
        if (saverrno) errno = saverrno;
        errormsg(SH_DICT, ERROR_system(1), "%s:", dir);
        __builtin_unreachable();
    }

success:
    scoped_dir = FETCH_VT(sh_scoped(shp, opwdnod)->nvalue, cp);
    if (dir == scoped_dir || argc == 2) {
        dp = dir;  // print out directory for cd -
    }
    if (pflag) {
        dir = stkptr(stkstd, PATH_OFFSET);
        dir = pathcanon(dir, PATH_MAX, PATH_ABSOLUTE | PATH_PHYSICAL);
        if (!dp) {
            dir = stkptr(stkstd, PATH_OFFSET);
            errormsg(SH_DICT, ERROR_system(1), "%s:", dir);
            __builtin_unreachable();
        }
        stkseek(stkstd, dir - stkptr(stkstd, 0));
    }
    dir = (char *)stkfreeze(stkstd, 1) + PATH_OFFSET;
    if (*dp && (*dp != '.' || dp[1]) && strchr(dir, '/')) sfputr(sfstdout, dir, '\n');
    if (*dir != '/') {
        if (!fflag) return 0;
        dir = fgetcwd(newdirfd, 0, 0);
    }
    nv_putval(opwdnod, oldpwd, NV_RDONLY);
    i = j = (int)strlen(dir);
    // Delete trailing '/'.
    while (--i > 0 && dir[i] == '/') {
        dir[i] = 0;
    }
    nv_putval(pwdnod, dir, NV_RDONLY);
    nv_onattr(pwdnod, NV_NOFREE | NV_EXPORT);
    shp->pwd = FETCH_VT(pwdnod->nvalue, const_cp);
    nv_scan(shp->track_tree, invalidate, NULL, NV_TAGGED, NV_TAGGED);
    path_newdir(shp, shp->pathlist);
    path_newdir(shp, shp->cdpathlist);
    if (oldpwd && (oldpwd != e_dot)) free(oldpwd);
    return 0;
}
