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
// David Korn
// AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "argnod.h"
#include "ast.h"
#include "ast_assert.h"
#include "cdt.h"
#include "defs.h"
#include "dlldefs.h"
#include "error.h"
#include "fault.h"
#include "fcin.h"
#include "history.h"
#include "io.h"
#include "jobs.h"
#include "name.h"
#include "path.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"
#include "test.h"
#include "variables.h"

#if USE_SPAWN
#include "spawnvex.h"
#endif

#define LIBCMD "cmd"

#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif

static_fn int can_execute(Shell_t *, char *, bool);
static_fn void funload(Shell_t *, int, const char *);
static_fn void exscript(Shell_t *, char *, char *[], char *const *);
static_fn bool path_chkpaths(Shell_t *, Pathcomp_t *, Pathcomp_t *, Pathcomp_t *, int);
static_fn void path_checkdup(Shell_t *shp, Pathcomp_t *);
static_fn Pathcomp_t *defpath_init(Shell_t *shp);

static const char *std_path = NULL;

static_fn bool onstdpath(Shell_t *shp, const char *name) {
    if (!std_path) defpath_init(shp);
    const char *cp = std_path, *sp;
    if (cp) {
        while (*cp) {
            for (sp = name; *sp && (*cp == *sp); sp++, cp++) {
                ;  // empty loop
            }
            if (*sp == 0 && (*cp == 0 || *cp == ':')) return true;
            while (*cp && *cp++ != ':') {
                ;  // empty loop
            }
        }
    }
    return false;
}

#if __CYGWIN__

// On Cygwin execve() can fail with errno == ENOEXEC when the real problem is that the file does
// not have an appropriate execute bit. In that case the correct errno is EACCES and the caller
// of path_pfexecve() requires that specific errno in that situation.
//
// Warning: This is a potential security hole since we're checking the permissions after the
// execve() has failed. In between the two operations the permissions might have been changed.
// But there isn't much we can do about that.
static_fn void fixup_execve_errno(const char *path) {
    if (errno != ENOEXEC) return;
    if (access(path, X_OK) == 0) return;
    errno = EACCES;
    return;
}

#else  // __CYGWIN__

static_fn void fixup_execve_errno(const char *path) { UNUSED(path); }

#endif  // __CYGWIN__

static_fn pid_t path_pfexecve(Shell_t *shp, const char *path, char *argv[], char *const envp[],
                              int spawn) {
    UNUSED(spawn);
#if USE_SPAWN
    if (shp->vex->cur) {
        spawnvex_apply(shp->vex, 0, 0);
        spawnvex_apply(shp->vexp, 0, SPAWN_RESET);
    }
#else   // USE_SPAWN
    UNUSED(shp);

    // Ensure stdin, stdout, stderr are open in the child process.
    // See https://github.com/att/ast/issues/1117.
    for (int fd = 0; fd < 3; ++fd) {
        errno = 0;
        if (fcntl(fd, F_GETFD, NULL) == -1 || errno == EBADF) {
            int tmp_fd = sh_open("/dev/null", O_RDWR);
            assert(tmp_fd == fd);
        }
    }
#endif  // USE_SPAWN

    int rv = execve(path, argv, envp);
    fixup_execve_errno(path);
    return rv;
}

#if !USE_SPAWN
static_fn pid_t _spawnveg(Shell_t *shp, const char *path, char *const argv[], char *const envp[],
                          pid_t pgid) {
    UNUSED(shp);
    UNUSED(path);
    UNUSED(argv);
    UNUSED(envp);
    UNUSED(pgid);

    abort();
}

#else  // USE_SPAWN

static_fn pid_t _spawnveg(Shell_t *shp, const char *path, char *const argv[], char *const envp[],
                          pid_t pgid) {
    UNUSED(pgid);
    pid_t pid;

#ifdef SIGTSTP
    if (job.jobcontrol) {
        sh_signal(SIGTTIN, (sh_sigfun_t)(SIG_DFL));
        sh_signal(SIGTTOU, (sh_sigfun_t)(SIG_DFL));
    }
#endif  // SIGTSTP

    while (1) {
        char *arg0 = argv[0], **av0 = (char **)&argv[0];
        int fd;

        sh_stats(STAT_SPAWN);
        pid = spawnvex(path, argv, envp, shp->vex);
        *av0 = arg0;
        if (pid > 0 && shp->comsub && (fd = sffileno(sfstdout)) != 1 && fd >= 0) {
            spawnvex_add(shp->vex, fd, 1, 0, 0);
        }
        if (pid >= 0 || errno != EAGAIN) break;
    }

#ifdef SIGTSTP
    if (job.jobcontrol) {
        sh_signal(SIGTTIN, (sh_sigfun_t)(SIG_IGN));
        sh_signal(SIGTTOU, (sh_sigfun_t)(SIG_IGN));
    }
#endif  // SIGTSTP

    return pid;
}
#endif  // USE_SPAWN

//
// Used with command -x to run the command in multiple passes. Spawn is non-zero when invoked via
// spawn. The exitval is set to the maximum for each execution.
//
static_fn pid_t path_xargs(Shell_t *shp, const char *path, char *argv[], char *const envp[],
                           int spawn) {
    char **av, **xv;
    char **avlast = &argv[shp->xargmax], **saveargs = NULL;
    char *const *ev;
    size_t size;
    ssize_t left;
    int nlast = 1, n, exitval = 0;
    pid_t pid;

    if (shp->xargmin < 0) return (pid_t)-1;
    size = shp->gd->lim.arg_max - 1024;
    for (ev = envp; *ev; ev++) size -= strlen(*ev) - 1;
    for (av = argv; *av && av < &argv[shp->xargmin]; av++) size -= strlen(*av) - 1;
    for (av = avlast; *av; av++, nlast++) size -= strlen(*av) - 1;
    av = &argv[shp->xargmin];
    if (!spawn) job_clear(shp);
    shp->exitval = 0;
    while (av < avlast) {
        for (xv = av, left = size; left > 0 && av < avlast;) left -= strlen(*av++) + 1;
        // Leave at least two for last.
        if (left < 0 && (avlast - av) < 2) av--;
        if (xv == &argv[shp->xargmin]) {
            n = nlast * sizeof(char *);
            saveargs = malloc(n);
            memcpy(saveargs, av, n);
            memcpy(av, avlast, n);
        } else {
            for (n = shp->xargmin; xv < av; xv++) argv[n++] = *xv;
            for (xv = avlast; *xv; xv++) argv[n++] = *xv;
            argv[n] = 0;
        }
        if (saveargs || av < avlast || (exitval && !spawn)) {
            if ((pid = _spawnveg(shp, path, argv, envp, 0)) < 0) return -1;
            job_post(shp, pid, 0);
            job_wait(pid);
            if (shp->exitval > exitval) exitval = shp->exitval;
            if (saveargs) {
                memcpy(av, saveargs, n);
                free(saveargs);
                saveargs = NULL;
            }
        } else if (spawn /*&& !sh_isoption(shp,SH_PFSH)*/) {
            shp->xargexit = exitval;
            if (saveargs) free(saveargs);
            return _spawnveg(shp, path, argv, envp, spawn >> 1);
        } else {
            if (saveargs) free(saveargs);
            return path_pfexecve(shp, path, argv, envp, spawn);
        }
    }
    if (!spawn) exit(exitval);
    return (pid_t)-1;
}

//
// Returns the present working directory (PWD). Attempts several different locations before
// falling back to getcwd(). Sets shp->pwd and $PWD to the first valid value found.
//
char *path_pwd(Shell_t *shp) {
    char *cp;
    int count = 0;

    if (shp->pwd) return (char *)shp->pwd;
    while (1) {
        // Try from lowest to highest.
        switch (count++) {
            case 0: {
                cp = nv_getval(VAR_PWD);
                break;
            }
            case 1: {
                cp = nv_getval(VAR_HOME);
                break;
            }
            case 2: {
                cp = "/";
                break;
            }
            case 3: {
                cp = getcwd(NULL, 0);
                if (cp) {
                    // FWIW: This creates a memory leak since the dynamically allocated buffer is
                    // never freed. Hopefully this is only executed in rare circumstances where the
                    // leak doesn't matter. This is also never true on SVr4 systems like Solaris.
                    nv_offattr(VAR_PWD, NV_NOFREE);
                    _nv_unset(VAR_PWD, 0);
                    STORE_VT(VAR_PWD->nvalue, const_cp, cp);
                    goto skip;
                }
                break;
            }
            default: { return (char *)e_dot; }
        }
        if (cp && *cp == '/' && test_inode(cp, e_dot)) break;
    }

    if (count > 1) {
        nv_offattr(VAR_PWD, NV_NOFREE);
        nv_putval(VAR_PWD, cp, NV_RDONLY);
    }
skip:
    nv_onattr(VAR_PWD, NV_NOFREE | NV_EXPORT);
    shp->pwd = FETCH_VT(VAR_PWD->nvalue, const_cp);
    return cp;
}

//
// Delete current Pathcomp_t structure.
//
void path_delete(Pathcomp_t *first) {
    Pathcomp_t *pp = first;
    Pathcomp_t *old = NULL;
    Pathcomp_t *ppnext;

    while (pp) {
        ppnext = pp->next;
        if (--pp->refcount <= 0) {
            if (pp->lib) free(pp->lib);
            if (pp->bbuf) free(pp->bbuf);
            free(pp);
            if (old) old->next = ppnext;
        } else {
            old = pp;
        }
        pp = ppnext;
    }
}

//
// Returns library variable from .paths. The value might be returned on the stack overwriting path.
//
static_fn char *path_lib(Shell_t *shp, Pathcomp_t *pp, char *path) {
    int r;
    struct stat statb;
    char *last = strrchr(path, '/');

    if (last) {
        *last = 0;
    } else {
        path = ".";
    }
    r = sh_stat(path, &statb);
    if (last) *last = '/';
    if (r < 0) return NULL;

    for (; pp; pp = pp->next) {
        if (!pp->dev && !pp->ino) path_checkdup(shp, pp);
        if (pp->ino == statb.st_ino && pp->dev == statb.st_dev && pp->mtime == statb.st_mtime) {
            return pp->lib;
        }
    }

    Pathcomp_t pcomp;
    memset(&pcomp, 0, sizeof(pcomp));
    if (last) pcomp.len = last - path;

    char save[8];
    memcpy(save, stkptr(shp->stk, PATH_OFFSET + pcomp.len), sizeof(save));
    if (path_chkpaths(shp, NULL, NULL, &pcomp, PATH_OFFSET)) return stkfreeze(shp->stk, 1);
    memcpy(stkptr(shp->stk, PATH_OFFSET + pcomp.len), save, sizeof(save));
    return NULL;
}

//
// Check for duplicate directories on PATH.
//
static_fn void path_checkdup(Shell_t *shp, Pathcomp_t *pp) {
    char *name = pp->name;
    Pathcomp_t *oldpp, *first;
    int flag = 0;
    struct stat statb;

    if (sh_stat(name, &statb) < 0 || !S_ISDIR(statb.st_mode)) {
        pp->flags |= PATH_SKIP;
        pp->dev = *name == '/';
        return;
    }
    pp->mtime = statb.st_mtime;
    pp->ino = statb.st_ino;
    pp->dev = statb.st_dev;
    if (*name == '/' && onstdpath(shp, name)) flag = PATH_STD_DIR;
    first = (pp->flags & PATH_CDPATH) ? shp->cdpathlist : path_get(shp, "");
    for (oldpp = first; oldpp && oldpp != pp; oldpp = oldpp->next) {
        if (pp->ino == oldpp->ino && pp->dev == oldpp->dev && pp->mtime == oldpp->mtime) {
            flag |= PATH_SKIP;
            break;
        }
    }
    pp->flags |= flag;
    if (((pp->flags & (PATH_PATH | PATH_SKIP)) == PATH_PATH)) {
        int offset = stktell(shp->stk);
        sfputr(shp->stk, name, -1);
        path_chkpaths(shp, first, 0, pp, offset);
        stkseek(shp->stk, offset);
    }
}

//
// Write the next path to search on the current stack. If last is given, all paths that come before
// <last> are skipped. The next pathcomp is returned.
//
Pathcomp_t *path_nextcomp(Shell_t *shp, Pathcomp_t *pp, const char *name, Pathcomp_t *last) {
    Pathcomp_t *ppnext;
    stkseek(shp->stk, PATH_OFFSET);

    if (*name == '/') {
        pp = NULL;
    } else {
        for (; pp && pp != last; pp = ppnext) {
            ppnext = pp->next;
            if (!pp->dev && !pp->ino) path_checkdup(shp, pp);
            if (pp->flags & PATH_SKIP) return ppnext;
            if (!last || *pp->name != '/') break;
        }
        if (!pp) {  // this should not happen
            pp = last;
        }
    }
    if (pp && (pp->name[0] != '.' || pp->name[1])) {
        if (*pp->name != '/') {
            sfputr(shp->stk, path_pwd(shp), -1);
            if (*stkptr(shp->stk, stktell(shp->stk) - 1) != '/') sfputc(shp->stk, '/');
        }
        sfwrite(shp->stk, pp->name, pp->len);
        if (pp->name[pp->len - 1] != '/') sfputc(shp->stk, '/');
    }
    sfputr(shp->stk, name, 0);
    while (pp && pp != last && (pp = pp->next)) {
        if (!(pp->flags & PATH_SKIP)) return pp;
    }
    return NULL;
}

static_fn Pathcomp_t *defpath_init(Shell_t *shp) {
    if (!std_path) {
        std_path = cs_path();
        if (std_path) {
            // Value returned by cs_path() is short lived, duplicate the string.
            // https://github.com/att/ast/issues/959
            std_path = strdup(std_path);
        } else {
            std_path = e_defpath;
        }
    }
    return path_addpath(shp, NULL, std_path, PATH_PATH);
}

static_fn void path_init(Shell_t *shp) {
    const char *val = FETCH_VT(sh_scoped(shp, VAR_PATH)->nvalue, const_cp);
    if (val) {
        shp->pathlist = path_addpath(shp, shp->pathlist, val, PATH_PATH);
    } else {
        Pathcomp_t *pp = shp->defpathlist;
        if (!pp) pp = defpath_init(shp);
        shp->pathlist = path_dup(pp);
    }
    val = FETCH_VT(sh_scoped(shp, VAR_FPATH)->nvalue, const_cp);
    if (val) (void)path_addpath(shp, shp->pathlist, val, PATH_FPATH);
}

//
// Returns that pathlist to search.
//
Pathcomp_t *path_get(Shell_t *shp, const char *name) {
    Pathcomp_t *pp = NULL;

    if (*name && strchr(name, '/')) return 0;
    if (!sh_isstate(shp, SH_DEFPATH)) {
        if (!shp->pathlist) path_init(shp);
        pp = shp->pathlist;
    }
    if ((!pp && !(FETCH_VT(sh_scoped(shp, VAR_PATH)->nvalue, const_cp))) ||
        sh_isstate(shp, SH_DEFPATH)) {
        pp = shp->defpathlist;
        if (!pp) pp = defpath_init(shp);
    }
    return pp;
}

//
// Open file corresponding to name using path give by <pp>.
//
static_fn int path_opentype(Shell_t *shp, const char *name, Pathcomp_t *pp, int fun) {
    int fd = -1;
    struct stat statb;
    Pathcomp_t *oldpp;

    if (!pp && !shp->pathlist) path_init(shp);
    if (!fun && strchr(name, '/') && sh_isoption(shp, SH_RESTRICTED)) {
        errormsg(SH_DICT, ERROR_exit(1), e_restricted, name);
        __builtin_unreachable();
    }
    do {
        pp = path_nextcomp(shp, oldpp = pp, name, 0);
        while (oldpp && (oldpp->flags & PATH_SKIP)) oldpp = oldpp->next;
        if (fun && (!oldpp || !(oldpp->flags & PATH_FPATH))) continue;
        fd = sh_open(path_relative(shp, stkptr(shp->stk, PATH_OFFSET)), O_RDONLY | O_CLOEXEC, 0);
        if (fd >= 0 && (fstat(fd, &statb) < 0 || S_ISDIR(statb.st_mode))) {
            errno = EISDIR;
            sh_close(fd);
            fd = -1;
        }
    } while (fd < 0 && pp);

    assert(fd < 0 || sh_iovalidfd(shp, fd));

    if (fd >= 0 && (fd = sh_iomovefd(shp, fd)) > 0) {
        (void)fcntl(fd, F_SETFD, FD_CLOEXEC);
        shp->fdstatus[fd] |= IOCLEX;
    }
    return fd;
}

//
// Open file corresponding to name using path give by <pp>.
//
int path_open(Shell_t *shp, const char *name, Pathcomp_t *pp) {
    return path_opentype(shp, name, pp, 0);
}

//
// Given a pathname return the base name.
//
char *path_basename(const char *name) {
    const char *start = name;

    while (*name) {
        if ((*name++ == '/') && *name) {  // don't trim trailing /
            start = name;
        }
    }
    return (char *)start;
}

char *path_fullname(Shell_t *shp, const char *name) {
    size_t len = strlen(name) + 1, dirlen = 0;
    char *path, *pwd;
    if (*name != '/') {
        pwd = path_pwd(shp);
        dirlen = strlen(pwd) + 1;
    }
    path = malloc(len + dirlen);
    if (dirlen) {
        memcpy(path, pwd, dirlen);
        path[dirlen - 1] = '/';
    }
    memcpy(&path[dirlen], name, len);
    pathcanon(path, len + dirlen, 0);
    return path;
}

//
// Load functions from file <fno>.
//
static_fn void funload(Shell_t *shp, int fno, const char *name) {
    char *pname, *oldname = shp->st.filename, buff[IOBSIZE + 1];
    Namval_t *np;
    struct Ufunction *rp, *rpfirst;
    int savestates = sh_getstate(shp), oldload = shp->funload;

    pname = path_fullname(shp, stkptr(shp->stk, PATH_OFFSET));
    if (shp->fpathdict && (rp = dtmatch(shp->fpathdict, pname))) {
        Dt_t *funtree = sh_subfuntree(shp, true);
        while (1) {
            rpfirst = dtprev(shp->fpathdict, rp);
            if (!rpfirst || strcmp(pname, rpfirst->fname)) break;
            rp = rpfirst;
        }
        do {
            if ((np = dtsearch(funtree, rp->np)) && is_afunction(np)) {
                if (FETCH_VT(np->nvalue, rp)) FETCH_VT(np->nvalue, rp)->fdict = NULL;
                nv_delete(np, funtree, NV_NOFREE);
            }
            dtinsert(funtree, rp->np);
            rp->fdict = funtree;
        } while ((rp = dtnext(shp->fpathdict, rp)) && strcmp(pname, rp->fname) == 0);
        sh_close(fno);
        free(pname);
        return;
    }
    sh_onstate(shp, SH_NOLOG);
    sh_onstate(shp, SH_NOALIAS);
    shp->readscript = (char *)name;
    shp->st.filename = pname;
    shp->funload = 1;
    error_info.line = 0;
    sh_eval(shp, sfnew(NULL, buff, IOBSIZE, fno, SF_READ), SH_FUNEVAL);
    sh_close(fno);
    shp->readscript = NULL;
    if (shp->namespace) {
        np = sh_fsearch(shp, name, 0);
    } else {
        np = nv_search(name, shp->fun_tree, 0);
    }
    if (!np || !FETCH_VT(np->nvalue, ip)) {
        pname = stkcopy(shp->stk, shp->st.filename);
    } else {
        pname = 0;
    }
    shp->funload = oldload;
    free(shp->st.filename);
    shp->st.filename = oldname;
    sh_setstate(shp, savestates);
    if (pname) errormsg(SH_DICT, ERROR_exit(ERROR_NOEXEC), e_funload, name, pname);
}

//
// Do a path search and track alias if requested.
// If flag is 0, or if name not found, then try autoloading function.
// If flag==2 or 3, returns 1 if name found on FPATH.
// If flag==3 no tracked alias will be set.
// Returns 1, if function was autoloaded.
// If oldpp is not NULL, it will contain a pointer to the path component where it was found.
//
bool path_search(Shell_t *shp, const char *name, Pathcomp_t **oldpp, int flag) {
    Namval_t *np;
    int fno;
    Pathcomp_t *pp = NULL;

    assert(name);
    if (strchr(name, '/')) {
        stkseek(shp->stk, PATH_OFFSET);
        sfputr(shp->stk, name, -1);
        if (can_execute(shp, stkptr(shp->stk, PATH_OFFSET), false) < 0) {
            *stkptr(shp->stk, PATH_OFFSET) = 0;
            return false;
        }
        if (*name == '/') return true;
        stkseek(shp->stk, PATH_OFFSET);
        sfputr(shp->stk, path_pwd(shp), '/');
        sfputr(shp->stk, name, 0);
        return false;
    }
    if (sh_isstate(shp, SH_DEFPATH)) {
        if (!shp->defpathlist) defpath_init(shp);
    } else if (!shp->pathlist) {
        path_init(shp);
    }
    if (flag) {
        if (!(flag & 1) && (np = nv_search(name, shp->track_tree, 0)) &&
            !nv_isattr(np, NV_NOALIAS) && (pp = (Pathcomp_t *)FETCH_VT(np->nvalue, const_cp))) {
            stkseek(shp->stk, PATH_OFFSET);
            path_nextcomp(shp, pp, name, pp);
            if (oldpp) *oldpp = pp;
            sfputc(shp->stk, 0);
            return false;
        }
        pp = path_absolute(shp, name, oldpp ? *oldpp : NULL);
        if (oldpp) *oldpp = pp;
        if (!pp && (np = nv_search(name, shp->fun_tree, 0)) && FETCH_VT(np->nvalue, ip)) {
            return true;
        }
        if (!pp) *stkptr(shp->stk, PATH_OFFSET) = 0;
    }
    if (flag == 0 || !pp || (pp->flags & PATH_FPATH)) {
        if (!pp) pp = sh_isstate(shp, SH_DEFPATH) ? shp->defpathlist : shp->pathlist;
        if (pp && strmatch(name, e_alphanum) && (fno = path_opentype(shp, name, pp, 1)) >= 0) {
            if (flag == 2) {
                sh_close(fno);
                return true;
            }
            funload(shp, fno, name);
            return true;
        }
        *stkptr(shp->stk, PATH_OFFSET) = 0;
        return false;
    } else if (pp && !sh_isstate(shp, SH_DEFPATH) && *name != '/' && flag < 3) {
        np = nv_search(name, shp->track_tree, NV_ADD);
        if (np) path_alias(np, pp);
    }
    return false;
}

static_fn bool pwdinfpath(void) {
    const char *pwd = nv_getval(VAR_PWD);
    const char *fpath = nv_getval(VAR_FPATH);
    int n;

    if (!pwd || !fpath) return false;
    while (*fpath) {
        for (n = 0; pwd[n] && pwd[n] == fpath[n]; n++) {
            ;  // empty loop
        }
        if (fpath[n] == ':' || fpath[n] == 0) return true;
        fpath += n;
        while (*fpath) fpath++;
    }
    return false;
}

//
// Do a path search and find the full pathname of file name.
//
Pathcomp_t *path_absolute(Shell_t *shp, const char *name, Pathcomp_t *pp) {
    int isfun;
    int fd = -1;
    int noexec = 0;
    Pathcomp_t *oldpp;
    Namval_t *np;
    char *cp;

    shp->path_err = ENOENT;
    if (!pp && !(pp = path_get(shp, ""))) return 0;
    shp->path_err = 0;
    while (true) {
        sh_sigcheck(shp);
        shp->bltin_dir = NULL;
        // In this loop, oldpp is the current pointer
        // pp is the next pointer
        while ((oldpp = pp)) {
            pp = path_nextcomp(shp, pp, name, 0);
            if (!(oldpp->flags & PATH_SKIP)) break;
        }
        if (!oldpp) {
            shp->path_err = ENOENT;
            return NULL;
        }
        isfun = (oldpp->flags & PATH_FPATH);
        if (!isfun && *oldpp->name == '.' && oldpp->name[1] == 0 && pwdinfpath()) isfun = true;
        if (!isfun && !sh_isoption(shp, SH_RESTRICTED)) {
            char *bp;
            Shbltin_f addr;
            int n;

            if (*stkptr(shp->stk, PATH_OFFSET) == '/' &&
                (np = nv_search(stkptr(shp->stk, PATH_OFFSET), shp->bltin_tree, 0)) &&
                !nv_isattr(np, BLT_DISABLE)) {
                return oldpp;
            }
            if ((oldpp->flags & PATH_BIN) && (bp = strrchr(oldpp->name, '/'))) {
                bp = stkptr(shp->stk, PATH_OFFSET + bp - oldpp->name);
                if (!(np = nv_search(bp, shp->bltin_tree, 0))) {
                    char save[4];
                    memcpy(save, bp -= 4, 4);
                    memcpy(bp, "/usr", 4);
                    np = nv_search(bp, shp->bltin_tree, 0);
                    memcpy(bp, save, 4);
                }

                if (np) {
                    addr = FETCH_VT(np->nvalue, shbltinp);
                    np = sh_addbuiltin(shp, stkptr(shp->stk, PATH_OFFSET), addr, NULL);
                    if (np) return oldpp;
                }
            }

            n = stktell(shp->stk);
            sfputr(shp->stk, "b_", -1);
            sfputr(shp->stk, name, 0);
            if ((addr = sh_getlib(shp, stkptr(shp->stk, n), oldpp)) &&
                (np = sh_addbuiltin(shp, stkptr(shp->stk, PATH_OFFSET), addr, NULL)) &&
                nv_isattr(np, NV_BLTINOPT)) {
                shp->bltin_dir = NULL;
                return oldpp;
            }
            stkseek(shp->stk, n);
            while (oldpp->blib) {
                char *fp;
                void *dll;
                int m;

                bp = oldpp->blib;
                fp = strchr(bp, ':');
                if (fp) {
                    *fp++ = 0;
                    oldpp->blib = fp;
                    fp = NULL;
                } else {
                    fp = oldpp->bbuf;
                    oldpp->blib = oldpp->bbuf = NULL;
                }
                n = stktell(shp->stk);
                sfputr(shp->stk, "b_", 0);
                sfputr(shp->stk, name, 0);
                m = stktell(shp->stk);
                shp->bltin_dir = oldpp->name;
                if (*bp != '/') sfputr(shp->stk, oldpp->name, '/');
                sfputr(shp->stk, bp, 0);
                cp = strrchr(stkptr(shp->stk, m), '/');
                if (cp) {
                    cp++;
                } else {
                    cp = stkptr(shp->stk, m);
                }
                if (!strcmp(cp, LIBCMD) && (addr = (Shbltin_f)dlllook(NULL, stkptr(shp->stk, n))) &&
                    (np = sh_addbuiltin(shp, stkptr(shp->stk, PATH_OFFSET), addr, NULL)) &&
                    nv_isattr(np, NV_BLTINOPT)) {
                found:
                    if (fp) free(fp);
                    shp->bltin_dir = NULL;
                    return oldpp;
                }
                dll = dllplugin(SH_ID, stkptr(shp->stk, m), NULL, SH_PLUGIN_VERSION, NULL,
                                RTLD_LAZY, NULL, 0);
                if (dll) sh_addlib(shp, dll, stkptr(shp->stk, m), oldpp);
                if (dll && (addr = (Shbltin_f)dlllook(dll, stkptr(shp->stk, n))) &&
                    (!(np = sh_addbuiltin(shp, stkptr(shp->stk, PATH_OFFSET), NULL, NULL)) ||
                     FETCH_VT(np->nvalue, shbltinp) != addr) &&
                    (np = sh_addbuiltin(shp, stkptr(shp->stk, PATH_OFFSET), addr, NULL))) {
                    np->nvenv = dll;
                    goto found;
                }
                if (*stkptr(shp->stk, PATH_OFFSET) == '/' &&
                    nv_search(stkptr(shp->stk, PATH_OFFSET), shp->bltin_tree, 0)) {
                    goto found;
                }
                if (fp) free(fp);
                stkseek(shp->stk, n);
            }
        }
        shp->bltin_dir = NULL;
        sh_stats(STAT_PATHS);
        fd = can_execute(shp, stkptr(shp->stk, PATH_OFFSET), isfun);
        if (isfun && fd >= 0 && (cp = strrchr(name, '.'))) {
            *cp = 0;
            if (nv_open(name, sh_subfuntree(shp, true), NV_NOARRAY | NV_IDENT | NV_NOSCOPE)) {
                sh_close(fd);
                fd = -1;
            }
            *cp = '.';
        }
        if (isfun && fd >= 0) {
            nv_onattr(nv_open(name, sh_subfuntree(shp, true), NV_NOARRAY | NV_IDENT | NV_NOSCOPE),
                      NV_LTOU | NV_FUNCTION);
            funload(shp, fd, name);
            return NULL;
        } else if (fd >= 0 && (oldpp->flags & PATH_STD_DIR)) {
            int n = stktell(shp->stk);
            sfputr(shp->stk, "/bin/", -1);
            sfputr(shp->stk, name, 0);
            np = nv_search(stkptr(shp->stk, n), shp->bltin_tree, 0);
            stkseek(shp->stk, n);
            if (np) {
                nvflag_t nvflags = np->nvflag;
                np = sh_addbuiltin(shp, stkptr(shp->stk, PATH_OFFSET),
                                   FETCH_VT(np->nvalue, shbltinp), nv_context(np));
                nv_setattr(np, nvflags);
            }
        }
        if (!pp || fd >= 0) break;
        if (errno != ENOENT) noexec = errno;
        // It's not clear this can actually happen but Coverity Scan says it is possible.
        // No unit test causes this condition to be true. We don't bother to set fd = -1 because
        // it is set by the `fd = can_execute()` assignment above before being used again.
        if (fd != -1) sh_close(fd);
    }

    if (fd == -1) {
        shp->path_err = (noexec ? noexec : ENOENT);
        return NULL;
    }

    // If we reach this point fd must be stdin (i.e., zero) since we intend to read the file.
    assert(fd == 0);
    sfputc(shp->stk, 0);
    return oldpp;
}

//
// Returns 0 if path can execute. Sets exec_err if file is found but can't be executable.
//
#undef S_IXALL
#ifdef S_IXUSR
#define S_IXALL (S_IXUSR | S_IXGRP | S_IXOTH)
#else  // S_IXUSR
#ifdef S_IEXEC
#define S_IXALL (S_IEXEC | (S_IEXEC >> 3) | (S_IEXEC >> 6))
#else  // S_IEXEC
#define S_IXALL 0111
#endif  // S_IEXEC
#endif  // S_IXUSR

static_fn int can_execute(Shell_t *shp, char *path, bool isfun) {
    struct stat statb;
    int fd = 0;

    path = path_relative(shp, path);
    if (isfun) {
        fd = sh_open(path, O_RDONLY | O_CLOEXEC, 0);
        if (fd < 0 || fstat(fd, &statb) < 0) goto err;
    } else if (sh_stat(path, &statb) < 0) {
#if __CYGWIN__
        // Check for .exe or .bat suffix.
        char *cp;
        if (errno == ENOENT && (!(cp = strrchr(path, '.')) || strlen(cp) > 4 || strchr(cp, '/'))) {
            int offset = stktell(shp->stk) - 1;
            stkseek(shp->stk, offset);
            sfputr(shp->stk, ".bat", -1);
            path = stkptr(shp->stk, PATH_OFFSET);
            if (stat(path, &statb) < 0) {
                if (errno != ENOENT) goto err;
                memcpy(stkptr(shp->stk, offset), ".sh", 4);
                if (stat(path, &statb) < 0) goto err;
            }
        } else
#endif  // __CYGWIN__
            goto err;
    }
    errno = EPERM;
    if (S_ISDIR(statb.st_mode)) {
        errno = EISDIR;
    } else if (isfun || (statb.st_mode & S_IXALL) == S_IXALL || sh_access(path, X_OK) >= 0) {
        return fd;
    }

err:
    if (isfun && fd >= 0) sh_close(fd);
    return -1;
}

//
// Return path relative to present working directory.
//
char *path_relative(Shell_t *shp, const char *file) {
    const char *pwd;
    const char *fp = file;

    // Can't relpath when shp->pwd not set.
    if (!(pwd = shp->pwd)) return (char *)fp;
    while (*pwd == *fp) {
        if (*pwd++ == 0) return (char *)e_dot;
        fp++;
    }
    if (*pwd == 0 && *fp == '/') {
        // //@// exposed here and in b_pwd() -- rats.
        do {
            if (fp[0] == '/' && fp[1] == '/' && fp[2] == '@' && fp[3] == '/' && fp[4] == '/') {
                return (char *)file;
            }
        } while (*++fp == '/');
        if (*fp) return (char *)fp;
        return (char *)e_dot;
    }
    return (char *)file;
}

void path_exec(Shell_t *shp, const char *arg0, char *argv[], struct argnod *local) {
    char **envp;
    const char *opath;
    Pathcomp_t *libpath;
    Pathcomp_t *pp = NULL;
    int slash = 0;
    int not_executable = 0;

    sh_setlist(shp, local, NV_EXPORT | NV_IDENT | NV_ASSIGN, 0);
    envp = sh_envgen(shp);
    if (strchr(arg0, '/')) {
        slash = 1;
        // Name containing / not allowed for restricted shell.
        if (sh_isoption(shp, SH_RESTRICTED)) {
            errormsg(SH_DICT, ERROR_exit(1), e_restricted, arg0);
            __builtin_unreachable();
        }
    } else {
        pp = path_get(shp, arg0);
    }
    shp->path_err = ENOENT;
    sfsync(NULL);
    timerdel(NULL);
    // Find first path that has a library component.
    while (pp && (pp->flags & PATH_SKIP)) pp = pp->next;
    if (pp || slash) {
        do {
            sh_sigcheck(shp);
            libpath = pp;
            if (libpath) {
                pp = path_nextcomp(shp, pp, arg0, 0);
                opath = stkfreeze(shp->stk, 1) + PATH_OFFSET;
            } else {
                opath = arg0;
            }
            path_spawn(shp, opath, argv, envp, libpath, 0);
            if ((shp->path_err != ENOENT) && (shp->path_err != EACCES) &&
                (shp->path_err != EISDIR)) {
                // An executable command was found, but failed to execute it
                errormsg(SH_DICT, ERROR_system(ERROR_NOEXEC), e_exec, arg0);
                __builtin_unreachable();
            } else if (shp->path_err == EACCES) {
                // A command was found but it was not executable.
                // POSIX specifies that shell should continue to search for command in PATH
                // and return 126 only when it can not find executable file in other elements
                // of PATH.
                not_executable = 1;
            }
            while (pp && (pp->flags & PATH_FPATH)) pp = path_nextcomp(shp, pp, arg0, 0);
        } while (pp);
    }
    // Force an exit.
    shp->jmplist->mode = SH_JMPEXIT;
    if (not_executable) {
        // This will return with status 126.
        errormsg(SH_DICT, ERROR_system(ERROR_NOEXEC), e_exec, arg0);
        __builtin_unreachable();
    } else {
        // This will return with status 127.
        errormsg(SH_DICT, ERROR_exit(ERROR_NOENT), e_found, arg0);
        __builtin_unreachable();
    }
}

#if USE_SPAWN
static_fn int vexexec(void *ptr, uint64_t fd1, uint64_t fd2) {
    UNUSED(fd1);
    char *devfd;
    int fd = -1;
    Spawnvex_noexec_t *ep = (Spawnvex_noexec_t *)ptr;
    Shell_t *shp = ep->handle;
    char **argv = (char **)ep->argv;

    if (fd2 != ENOEXEC) return (int)fd2;
    if (!(ep->flags & SPAWN_EXEC)) return ENOEXEC;

    if (ep->msgfd >= 0) close(ep->msgfd);
    spawnvex_apply(ep->vex, 0, SPAWN_RESET);
    if (!shp->subshell) {
        exscript(ep->handle, (char *)ep->path, argv, ep->envv);
        return ENOEXEC;
    }

    fd = open(ep->path, O_RDONLY);
    if (fd >= 0) {
        struct stat statb;
        sfprintf(shp->strbuf, "/dev/fd/%d", fd);
        if (stat(devfd = sfstruse(shp->strbuf), &statb) >= 0) argv[0] = devfd;
    }
    argv[-1] = argv[0];
    argv[0] = (char *)ep->path;
    if (!shp->gd->shpath) shp->gd->shpath = pathshell();
    execve(shp->gd->shpath, &argv[-1], ep->envv);
    return errno;
}
#endif  // USE_SPAWN

pid_t path_spawn(Shell_t *shp, const char *opath, char **argv, char **envp, Pathcomp_t *libpath,
                 int spawn) {
    char *path;
    char **xp = 0, *xval, *libenv = (libpath ? libpath->lib : 0);
    Namval_t *np;
    char *s, *v;
    int n, pidsize;
    size_t r;
    pid_t pid = -1;

#if USE_SPAWN
    if (spawn > 1) {
        spawnvex_add(shp->vex, SPAWN_pgrp, spawn >> 1, 0, 0);
    }
    spawnvex_add(shp->vex, SPAWN_noexec, 0, vexexec, shp);
#endif  // USE_SPAWN
    // Leave room for inserting _= pathname in environment.
    envp--;
    // Save original pathname.
    stkseek(shp->stk, PATH_OFFSET);
    pidsize = sfprintf(stkstd, "*%d*", spawn ? getpid() : getppid());
    sfputr(shp->stk, opath, -1);
    opath = stkfreeze(shp->stk, 1) + PATH_OFFSET + pidsize;
    np = nv_search(argv[0], shp->track_tree, 0);
    while (libpath && !libpath->lib) libpath = libpath->next;
    if (libpath && (!np || nv_size(np) > 0)) {
        // Check for symlink and use symlink name.
        char buff[PATH_MAX + 1];
        char save[PATH_MAX + 1];
        stkseek(shp->stk, PATH_OFFSET);
        sfputr(shp->stk, opath, -1);
        path = stkptr(shp->stk, PATH_OFFSET);
        while ((n = readlink(path, buff, PATH_MAX)) > 0) {
            buff[n] = 0;
            n = PATH_OFFSET;
            r = 0;
            if ((v = strrchr(path, '/')) && *buff != '/') {
                if (buff[0] == '.' && buff[1] == '.' && (r = strlen(path) + 1) <= PATH_MAX) {
                    memcpy(save, path, r);
                } else {
                    r = 0;
                }
                n += (v + 1 - path);
            }
            stkseek(shp->stk, n);
            sfputr(shp->stk, buff, 0);
            n = stktell(shp->stk);
            path = stkptr(shp->stk, PATH_OFFSET);
            if (v && buff[0] == '.' && buff[1] == '.') {
                pathcanon(path, n - PATH_OFFSET, 0);
                if (r && sh_access(path, X_OK)) {
                    memcpy(path, save, r);
                    break;
                }
            }
            libenv = path_lib(shp, libpath, path);
            if (libenv) break;
        }
        stkseek(shp->stk, 0);
    }
    if (libenv && (v = strchr(libenv, '='))) {
        n = v - libenv;
        *v = 0;
        np = nv_open(libenv, shp->var_tree, 0);
        *v = '=';
        s = nv_getval(np);
        sfputr(shp->stk, libenv, -1);
        if (s) {
            sfputc(shp->stk, ':');
            sfputr(shp->stk, s, -1);
        }
        v = stkfreeze(shp->stk, 1);
        r = 1;
        xp = envp + 1;
        while ((s = *xp++)) {
            if (!strncmp(s, v, n) && s[n] == '=') {
                xval = *--xp;
                *xp = v;
                r = 0;
                break;
            }
        }
        if (r) {
            *envp-- = v;
            xp = 0;
        }
    }

    envp[0] = (char *)opath - (PATH_OFFSET + pidsize);
    envp[0][0] = '_';
    envp[0][1] = '=';
    sfsync(sfstderr);
    sh_sigcheck(shp);
    path = path_relative(shp, opath);
    if (spawn /* && !sh_isoption(shp,SH_PFSH) */) {
        pid = _spawnveg(shp, opath, &argv[0], envp, spawn >> 1);
    } else {
        pid = path_pfexecve(shp, opath, &argv[0], envp, spawn);
    }
    if (xp) *xp = xval;
    if (pid > 0) return pid;
retry:
    shp->path_err = errno;
    switch (shp->path_err) {
        case EISDIR: {
            return -1;
        }
        case ENOEXEC:
        case EPERM: {
            // Some systems return EPERM if setuid bit is on.
            errno = ENOEXEC;
            if (spawn) {
                if (shp->subshell) return -1;
                do {
                    pid = fork();
                    if (pid > 0) return pid;
                } while (_sh_fork(shp, pid, 0, NULL) < 0);
#if USE_SPAWN
                if (shp->vex) spawnvex_apply(shp->vex, 0, 0);
#endif  // USE_SPAWN
                shp->jmplist->mode = SH_JMPEXIT;
            }
            exscript(shp, path, argv, envp);
            // TODO: is this supposed to FALL THRU or it is unreachable?
        }
        // FALLTHRU
        case EACCES: {
            struct stat statb;
            if (sh_stat(path, &statb) >= 0) {
                if (S_ISDIR(statb.st_mode)) errno = EISDIR;
#ifdef S_ISSOCK
                if (S_ISSOCK(statb.st_mode)) exscript(shp, path, argv, envp);
#endif
            }
            return -1;
        }
#ifdef ENAMETOOLONG
        case ENAMETOOLONG: {
            return -1;
        }
#endif  // ENAMETOOLONG
#ifdef EMLINK
        case EMLINK:
#endif  // EMLINK
        case ENOTDIR:
        case ENOENT:
        case EINTR: {
            return -1;
        }
        case E2BIG: {
            if (shp->xargmin) {
                pid = path_xargs(shp, opath, &argv[0], envp, spawn);
                if (pid < 0) goto retry;
                return pid;
            }
        }
        // FALLTHRU
        default: {
            errormsg(SH_DICT, ERROR_system(ERROR_NOEXEC), e_exec, path);
            __builtin_unreachable();
        }
    }

    return 0;
}

//
// File is executable but not machine code. Assume file is a Shell script and execute it.
//
static_fn void exscript(Shell_t *shp, char *path, char *argv[], char *const *envp) {
    UNUSED(envp);
    Sfio_t *sp;

    path = path_relative(shp, path);
    shp->comdiv = NULL;
    shp->bckpid = 0;
    shp->coshell = NULL;
    shp->st.ioset = 0;
    // Clean up any cooperating processes.
    if (shp->cpipe[0] > 0) sh_pclose(shp->cpipe);
    if (shp->cpid && shp->outpipe) sh_close(*shp->outpipe);
    shp->cpid = 0;
    sp = fcfile();
    if (sp) {
        while (sfstack(sp, SF_POPSTACK)) {
            ;  // empty loop
        }
    }
    job_clear(shp);
    if (shp->infd > 0 && (shp->fdstatus[shp->infd] & IOCLEX)) sh_close(shp->infd);
    sh_setstate(shp, sh_state(SH_FORKED));
    sfsync(sfstderr);

    // Check if file cannot open for read or script is setuid/setgid.
    static char name[] = "/tmp/euidXXXXXXXXXX";
    int n;
    uid_t euserid;
    char *savet = NULL;
    struct stat statb;
    int err = 0;

    if ((n = sh_open(path, O_RDONLY | O_CLOEXEC, 0)) >= 0) {
        // Move <n> if n=0,1,2.
        n = sh_iomovefd(shp, n);
        if (fstat(n, &statb) >= 0 && !(statb.st_mode & (S_ISUID | S_ISGID))) goto openok;
        sh_close(n);
    } else {
        err = errno;
    }
    if ((euserid = geteuid()) != shp->gd->userid) {
        n = strlcpy(name + 9, fmtbase(getpid(), 10, 0), sizeof(name) - 9);
        if (n >= sizeof(name) - 9) abort();  // this can't happen
        // Create a suid open file with owner equal effective uid.
        n = sh_open(name, O_CREAT | O_TRUNC | O_WRONLY | O_CLOEXEC, S_ISUID | S_IXUSR);
        if (n < 0) goto fail;
        unlink(name);
        // Make sure that file has right owner.
        if (fstat(n, &statb) < 0 || statb.st_uid != euserid) {
            sh_close(n);
            goto fail;
        }
        if (n != 10) {
            sh_close(10);
            (void)fcntl(n, F_DUPFD_CLOEXEC, 10);
            sh_close(n);
        }
    }
    savet = *--argv;
    *argv = path;
#if 1
    if (err == EACCES) {
        errno = EACCES;
        return;
    }
#else
    // This block is commented out because we no longer intend to support the suid_exec program.
    // See issue #366. I'm leaving the code here to provide context for the surrounding code.
    //
    // TODO: Rewrite this function. The code above that creates a temp file if a suid/sgid file is
    // found appears to be pointless since the temp file is never used. Furthermore, if the file is
    // suid/sgid but can be opened (and the suidexec program isn't found) then it is still executed
    // but without the proper credentials!
    if (err == EACCES && sh_access(e_suidexec, X_OK) < 0) {
        errno = EACCES;
        return;
    }
    path_pfexecve(shp, e_suidexec, argv, envp, 0);
#endif

fail:
    // The following code is just for compatibility.
    n = sh_open(path, O_RDONLY | O_CLOEXEC, 0);
    if (n == -1) {
        errormsg(SH_DICT, ERROR_system(ERROR_NOEXEC), e_exec, path);
        __builtin_unreachable();
    }
    if (savet) *argv++ = savet;

openok:
    shp->infd = n;

    shp->infd = sh_iomovefd(shp, shp->infd);
    shp->arglist = sh_argcreate(argv);
    shp->lastarg = strdup(path);
    // Save name of calling command.
    shp->readscript = error_info.id;
    // Close history file if name has changed.
    if (shp->gd->hist_ptr && (path = nv_getval(VAR_HISTFILE)) &&
        strcmp(path, shp->gd->hist_ptr->histname)) {
        hist_close(shp->gd->hist_ptr);
        STORE_VT((VAR_HISTCMD)->nvalue, i32p, NULL);
    }
    sh_offstate(shp, SH_FORKED);
    if (shp->sigflag[SIGCHLD] == SH_SIGOFF) shp->sigflag[SIGCHLD] = SH_SIGFAULT;
    siglongjmp(shp->jmplist->buff, SH_JMPSCRIPT);
}

//
// Add a pathcomponent to the path search list and eliminate duplicates and non-existing absolute
// paths.
//
static_fn Pathcomp_t *path_addcomp(Shell_t *shp, Pathcomp_t *first, Pathcomp_t *old,
                                   const char *name, int flag) {
    Pathcomp_t *pp, *oldpp;
    int offset = stktell(shp->stk);
    size_t len;

    if (!(flag & PATH_BFPATH)) {
        const char *cp = name;
        while (*cp && *cp != ':') sfputc(shp->stk, *cp++);
        len = stktell(shp->stk) - offset;
        sfputc(shp->stk, 0);
        stkseek(shp->stk, offset);
        name = (const char *)stkptr(shp->stk, offset);
    } else {
        len = strlen(name);
    }
    for (pp = first; pp; pp = pp->next) {
        if (len == pp->len && strncmp(name, pp->name, len) == 0) {
            pp->flags |= flag;
            return first;
        }
    }
    for (pp = first, oldpp = NULL; pp; oldpp = pp, pp = pp->next) {
        ;  // empty loop
    }
    pp = calloc(1, sizeof(Pathcomp_t) + len + 1);
    pp->shp = shp;
    pp->refcount = 1;
    memcpy(pp + 1, name, len + 1);
    pp->name = (char *)(pp + 1);
    pp->len = len;
    if (oldpp) {
        oldpp->next = pp;
    } else {
        first = pp;
    }
    pp->flags = flag;
    if (strcmp(name, SH_CMDLIB_DIR) == 0) {
        pp->dev = 1;
        pp->flags |= PATH_BUILTIN_LIB;
        pp->blib = pp->bbuf = malloc(sizeof(LIBCMD));
        strcpy(pp->blib, LIBCMD);
        return first;
    }
    if ((old || shp->pathinit) && ((flag & (PATH_PATH | PATH_SKIP)) == PATH_PATH)) {
        path_chkpaths(shp, first, old, pp, offset);
    }
    return first;
}

bool path_cmdlib(Shell_t *shp, const char *dir, bool on) {
    Pathcomp_t *pp;
    for (pp = shp->pathlist; pp; pp = pp->next) {
        if (strcmp(pp->name, dir)) continue;
        if (on) {
            pp->flags &= ~PATH_SKIP;
        } else if (pp->dev == 1 && pp->ino == 0) {
            pp->flags |= PATH_SKIP;
        }
        break;
    }
    return pp != 0;
}

//
// This function checks for the .paths file in directory in <pp>. It assumes that the directory is
// on the stack at <offset>.
//
static_fn bool path_chkpaths(Shell_t *shp, Pathcomp_t *first, Pathcomp_t *old, Pathcomp_t *pp,
                             int offset) {
    struct stat statb;
    int k, m, n, fd;
    char *sp, *cp, *ep;

    stkseek(shp->stk, offset + pp->len);
    if (pp->len == 1 && *stkptr(shp->stk, offset) == '/') stkseek(shp->stk, offset);
    sfputr(shp->stk, "/.paths", -1);
    fd = sh_open(stkptr(shp->stk, offset), O_RDONLY | O_CLOEXEC);
    if (fd >= 0) {
        if (fstat(fd, &statb) == -1) abort();  // it should be impossible for this to fail
        n = statb.st_size;
        stkseek(shp->stk, offset + pp->len + n + 2);
        sp = stkptr(shp->stk, offset + pp->len);
        *sp++ = '/';
        n = read(fd, cp = sp, n);
        sp[n] = 0;
        close(fd);
        for (ep = NULL; n--; cp++) {
            if (*cp == '=') {
                ep = cp + 1;
                continue;
            } else if (*cp != '\r' && *cp != '\n') {
                continue;
            }
            if (*sp == '#' || sp == cp) {
                sp = cp + 1;
                continue;
            }
            *cp = 0;
            m = ep ? ep - sp : 0;
            if (m == 0 || (m == 6 && strncmp(sp, "FPATH=", m) == 0)) {
                if (first) {
                    char *ptr = stkptr(shp->stk, offset + pp->len + 1);
                    // This must be memmove() because the buffers will typically overlap. For
                    // example, if ptr points to "FPATH=../xxfun" then ep will point to the start of
                    // the path (e.g., the first period) and will copy that path over the "FPATH="
                    // portion of the buffer to elide that prefix.
                    if (ep) memmove(ptr, ep, strlen(ep) + 1);
                    path_addcomp(shp, first, old, stkptr(shp->stk, offset),
                                 PATH_FPATH | PATH_BFPATH);
                }
            } else if (m == 11 && strncmp(sp, "PLUGIN_LIB=", m) == 0) {
                if (pp->bbuf) free(pp->bbuf);
                assert(ep);
                pp->blib = pp->bbuf = strdup(ep);
            } else if (m == 4 && strncmp(sp, "BIN=1", m) == 0) {
                pp->flags |= PATH_BIN;
            } else if (m) {
                size_t z;
                pp->lib = malloc(z = cp - sp + pp->len + 2);
                memcpy(pp->lib, sp, m);
                memcpy(&pp->lib[m], stkptr(shp->stk, offset), pp->len);
                pp->lib[k = m + pp->len] = '/';
                strcpy(&pp->lib[k + 1], ep);
                pathcanon(&pp->lib[m], z, 0);
                if (!first) {
                    stkseek(shp->stk, 0);
                    sfputr(shp->stk, pp->lib, -1);
                    free(pp->lib);
                    return true;
                }
            }
            sp = cp + 1;
            ep = NULL;
        }
    }
    return false;
}

Pathcomp_t *path_addpath(Shell_t *shp, Pathcomp_t *first, const char *path, int type) {
    const char *cp;
    Pathcomp_t *old = NULL;
    int offset = stktell(shp->stk);
    char *savptr;

    if (!path && type != PATH_PATH) return first;
    if (type != PATH_FPATH) {
        old = first;
        first = 0;
    }
    if (offset) savptr = stkfreeze(shp->stk, 0);
    if (path) {
        while (*(cp = path)) {
            if (*cp == ':') {
                if (type != PATH_FPATH) first = path_addcomp(shp, first, old, ".", type);
                while (*++path == ':') {
                    ;  // empty loop
                }
            } else {
                int c;
                while (*path && *path != ':') path++;
                c = *path++;
                first = path_addcomp(shp, first, old, cp, type);
                if (c == 0) break;
                if (*path == 0) path--;
            }
        }
    }
    if (old) {
        if (!first && !path) {
            Pathcomp_t *pp = shp->defpathlist;
            if (!pp) pp = defpath_init(shp);
            first = path_dup(pp);
        }
        cp = FETCH_VT(sh_scoped(shp, VAR_FPATH)->nvalue, const_cp);
        if (cp) {
            first = path_addpath(shp, first, cp, PATH_FPATH);
        }
        path_delete(old);
    }
    if (offset) {
        stkset(shp->stk, savptr, offset);
    } else {
        stkseek(shp->stk, 0);
    }
    return first;
}

//
// Duplicate the path give by <first> by incremented reference counts.
//
Pathcomp_t *path_dup(Pathcomp_t *first) {
    Pathcomp_t *pp = first;

    while (pp) {
        pp->refcount++;
        pp = pp->next;
    }
    return first;
}

//
// Called whenever the directory is changed.
//
void path_newdir(Shell_t *shp, Pathcomp_t *first) {
    Pathcomp_t *pp = first, *next, *pq;
    struct stat statb;

    for (pp = first; pp; pp = pp->next) {
        pp->flags &= ~PATH_SKIP;
        if (*pp->name == '/') continue;
        // Delete .paths component.
        if ((next = pp->next) && (next->flags & PATH_BFPATH)) {
            pp->next = next->next;
            if (--next->refcount <= 0) free(next);
        }
        if (sh_stat(pp->name, &statb) < 0 || !S_ISDIR(statb.st_mode)) {
            pp->dev = 0;
            pp->ino = 0;
            continue;
        }
        pp->dev = statb.st_dev;
        pp->ino = statb.st_ino;
        pp->mtime = statb.st_mtime;
        for (pq = first; pq != pp; pq = pq->next) {
            if (pp->ino == pq->ino && pp->dev == pq->dev) pp->flags |= PATH_SKIP;
        }
        for (pq = pp; pq->next;) {
            pq = pq->next;
            if (pp->ino == pq->ino && pp->dev == pq->dev) pq->flags |= PATH_SKIP;
        }
        if ((pp->flags & (PATH_PATH | PATH_SKIP)) == PATH_PATH) {
            // Try to insert .paths component.
            int offset = stktell(shp->stk);
            sfputr(shp->stk, pp->name, -1);
            stkseek(shp->stk, offset);
            next = pp->next;
            pp->next = NULL;
            path_chkpaths(shp, first, NULL, pp, offset);
            if (pp->next) pp = pp->next;
            pp->next = next;
        }
    }
}

Pathcomp_t *path_unsetfpath(Shell_t *shp) {
    Pathcomp_t *first = shp->pathlist;
    Pathcomp_t *pp = first;
    Pathcomp_t *old = NULL;

    if (shp->fpathdict) {
        struct Ufunction *rp, *rpnext;
        for (rp = (struct Ufunction *)dtfirst(shp->fpathdict); rp; rp = rpnext) {
            rpnext = (struct Ufunction *)dtnext(shp->fpathdict, rp);
            if (rp->fdict) nv_delete(rp->np, rp->fdict, NV_NOFREE);
            rp->fdict = NULL;
        }
    }
    while (pp) {
        if ((pp->flags & PATH_FPATH) && !(pp->flags & PATH_BFPATH)) {
            if (pp->flags & PATH_PATH) {
                pp->flags &= ~PATH_FPATH;
            } else {
                Pathcomp_t *ppsave = pp;
                if (old) {
                    old->next = pp->next;
                } else {
                    first = pp->next;
                }
                pp = pp->next;
                if (--ppsave->refcount <= 0) {
                    if (ppsave->lib) free(ppsave->lib);
                    free(ppsave);
                }
                continue;
            }
        }
        old = pp;
        pp = pp->next;
    }
    return first;
}

Pathcomp_t *path_dirfind(Pathcomp_t *first, const char *name, int c) {
    Pathcomp_t *pp = first;

    while (pp) {
        if (strncmp(name, pp->name, pp->len) == 0 && name[pp->len] == c) return pp;
        pp = pp->next;
    }
    return NULL;
}

//
// Get discipline for tracked alias.
//
static_fn char *talias_get(Namval_t *np, Namfun_t *nvp) {
    UNUSED(nvp);
    Shell_t *shp = sh_ptr(np);
    Pathcomp_t *pp = (Pathcomp_t *)FETCH_VT(np->nvalue, const_cp);
    char *ptr;

    if (!pp) return NULL;
    pp->shp->last_table = NULL;
    path_nextcomp(pp->shp, pp, nv_name(np), pp);
    ptr = stkfreeze(shp->stk, 0);
    return ptr + PATH_OFFSET;
}

static_fn void talias_put(Namval_t *np, const void *val, nvflag_t flags, Namfun_t *fp) {
    if (!val && FETCH_VT(np->nvalue, const_cp)) {
        Pathcomp_t *pp = (Pathcomp_t *)FETCH_VT(np->nvalue, const_cp);
        if (--pp->refcount <= 0) free(pp);
    }
    nv_putv(np, val, flags, fp);
}

static const Namdisc_t talias_disc = {.dsize = 0, .putval = talias_put, .getval = talias_get};
static Namfun_t talias_init = {.disc = &talias_disc, .nofree = 1, .subshell = 0, .dsize = 0};

//
// Set tracked alias node <np> to value <pp>.
//
void path_alias(Namval_t *np, Pathcomp_t *pp) {
    if (pp) {
        Shell_t *shp = sh_ptr(np);
        struct stat statb;
        char *sp;
        Pathcomp_t *old;
        nv_offattr(np, NV_NOPRINT);
        nv_stack(np, &talias_init);
        old = FETCH_VT(np->nvalue, pathcomp);
        if (old && (--old->refcount <= 0)) {
            free(old);
        }
        STORE_VT(np->nvalue, pathcomp, pp);
        pp->refcount++;
        nv_setattr(np, NV_TAGGED | NV_NOFREE);
        path_nextcomp(pp->shp, pp, nv_name(np), pp);
        sp = stkptr(shp->stk, PATH_OFFSET);
        if (sp && lstat(sp, &statb) >= 0 && S_ISLNK(statb.st_mode)) {
            nv_setsize(np, statb.st_size + 1);
        } else {
            nv_setsize(np, 0);
        }
    } else {
        _nv_unset(np, 0);
    }
}

//
// Obtain a file handle to the directory "path" relative to directory "dir".
//
int sh_diropenat(Shell_t *shp, int dir, const char *path) {
    UNUSED(shp);
    int fd, shfd;
    fd = openat(dir, path, O_DIRECTORY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
#if O_SEARCH
        if (errno != EACCES ||
            (fd = openat(dir, path, O_SEARCH | O_DIRECTORY | O_NONBLOCK | O_CLOEXEC)) < 0) {
#endif
            return fd;
#if O_SEARCH
        }
#endif
    }

    // Move fd to a number > 10 and register the fd number with the shell.
    shfd = sh_fcntl(fd, F_DUPFD_CLOEXEC, 10);
    close(fd);
    return shfd;
}
