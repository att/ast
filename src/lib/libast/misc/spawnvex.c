/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2014 AT&T Intellectual Property          *
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
 *                     Phong Vo <phongvo@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
/*
 * son of spawnveg()
 * more cooperative than posix_spawn()
 *
 * this code looks worse than it is because of the DEBUG_* emulations
 * the DEBUG_* emulations allow { cygwin ibm } debugging on linux
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast.h"
#include "error.h"
#include "sfio.h"
#include "sig.h"
#include "spawnvex.h"

#ifndef ENOSYS
#define ENOSYS EINVAL
#endif

#if !_lib_spawnvex

#define VEXCHUNK 8
#define VEXFLAG(x) (1 << (-(x)))
#define VEXINIT(p) ((p)->cur = (p)->frame, (p)->set = 0)

union _Spawnvex_u {
    int64_t number;
    void *handle;
    Spawnvex_f callback;
};

#if _lib_spawn_mode

#ifndef P_NOWAIT

#include <process.h>

#ifndef P_NOWAIT
#define P_NOWAIT _P_NOWAIT
#endif
#ifndef P_DETACH
#define P_DETACH _P_DETACH
#endif

#endif

#define SPAWN_cloexec (-99)

#else

#include <spawn.h>

#endif

#if _lib_spawn_mode || _lib_spawn && _mem_pgroup_inheritance

/*
 * slide fd out of the way so native spawn child process doesn't see it
 */

static int save(int fd, int *pf, int md) {
    int td;

    if ((*pf = fcntl(fd, F_GETFD)) < 0) return -1;
    if (md) {
#ifdef F_DUPFD_CLOEXEC
        if ((td = fcntl(fd, F_DUPFD_CLOEXEC, md)) < 0)
#else
        if ((td = fcntl(fd, F_DUPFD, md)) < 0 ||
            fcntl(td, F_SETFD, FD_CLOEXEC) < 0 && (close(td), 1))
#endif
            return -2;
        close(fd);
        fd = td;
    }
    return fd;
}

/*
 * add save()'d fd restore ops to vex
 */

static int restore(Spawnvex_t *vex, int i, int j, int fl) {
    if (j >= 0) {
        if (spawnvex_add(vex, i, j, 0, 0) < 0) return -1;
        if (fl == 0 && spawnvex_add(vex, j, j, 0, 0) < 0) return -1;
    }
    if (spawnvex_add(vex, i, -1, 0, 0) < 0) return -1;
    return 0;
}

#endif

Spawnvex_t *spawnvex_open(unsigned int flags) {
    Spawnvex_t *vex;

    vex = calloc(1, sizeof(Spawnvex_t));
    if (vex) {
        VEXINIT(vex);
        vex->flags = flags;
#ifdef F_DUPFD_CLOEXEC
        vex->debug = (flags & SPAWN_DEBUG) ? fcntl(2, F_DUPFD_CLOEXEC, 60) : -1;
#else
        if ((vex->debug = (flags & SPAWN_DEBUG) ? fcntl(2, F_DUPFD, 60) : -1) >= 0)
            (void)fcntl(vex->debug, F_SETFD, FD_CLOEXEC);
#endif
    }
    return vex;
}

int spawnvex_add(Spawnvex_t *vex, int64_t op, int64_t arg, Spawnvex_f callback, void *handle) {
    if ((vex->cur + (callback ? 4 : 2)) >= vex->max) {
        vex->max += VEXCHUNK;
        vex->op = realloc(vex->op, vex->max * sizeof(Spawnvex_u));
        if (!vex->op) {
            vex->max = 0;
            VEXINIT(vex);
            return -1;
        }
    }
    switch ((int)op) {
        case SPAWN_frame:
            arg = vex->frame;
            vex->frame = vex->cur;
            break;
        case SPAWN_pgrp:
#if OBSOLETE < 20150101
            if (arg == 1) arg = 0;
#endif
            break;
    }
    if (op < 0) vex->set |= VEXFLAG(op);
    op *= 2;
    if (callback) {
        if (op < 0) {
            op--;
        } else {
            op++;
        }
    }
    if (vex->debug > 0) {
        error(ERROR_OUTPUT, vex->debug, "spawnvex add %4d %8d %p %4d %4I*d %4I*d %p %p", __LINE__,
              getpid(), vex, vex->cur, sizeof(op), op / 2, sizeof(arg), arg, callback,
              callback ? handle : NULL);
    }
    vex->op[vex->cur++].number = op;
    vex->op[vex->cur++].number = arg;
    if (callback) {
        vex->op[vex->cur++].callback = callback;
        vex->op[vex->cur++].handle = handle;
    }
    return vex->cur;
}

int spawnvex_apply(Spawnvex_t *vex, int cur, int flags) {
    int i;
    int j;
    int k;
    int op;
    int arg;
    int err;
    int ret;
    off_t off;
    void *handle;
    Spawnvex_f callback;
    unsigned char siz[512];

    if (cur < 0 || cur > vex->max) return EINVAL;
    ret = 0;
    if (!(flags & SPAWN_RESET)) {
        vex->noexec = -1;
        vex->pgrp = -1;
        i = cur;
        if (flags & SPAWN_UNDO) {
            for (j = 0; i < vex->cur && j < elementsof(siz) - 1; j++) {
                i += (siz[j] = (vex->op[i].number & 1) ? 4 : 2);
            }
            siz[j] = 0;
        }
        for (;;) {
            k = i;
            if (flags & SPAWN_UNDO) {
                if (j < 1) break;
                i -= siz[j];
                i -= siz[--j];
            } else if (i >= vex->cur || !vex->op) {
                break;
            }
            op = vex->op[i++].number;
            arg = vex->op[i++].number;
            if (!(op & 1)) {
                callback = 0;
            } else if (flags & SPAWN_NOCALL) {
                i += 2;
                callback = 0;
            } else {
                callback = vex->op[i++].callback;
                handle = vex->op[i++].handle;
            }
            op /= 2;
            if (vex->debug >= 0) {
                error(ERROR_OUTPUT, vex->debug, "spawnvex app %4d %8d %p %4d %4I*d %4I*d %p %p",
                      __LINE__, getpid(), vex, k, sizeof(op), op, sizeof(arg), arg, callback,
                      callback ? handle : NULL);
            }
            if (!(flags & SPAWN_CLEANUP)) {
                err = 0;
                switch (op) {
                    case SPAWN_noop:
                        break;
                    case SPAWN_frame:
                        break;
                    case SPAWN_cwd:
                        if (fchdir(arg)) err = errno;
                        break;
#ifdef SPAWN_cloexec
                    case SPAWN_cloexec:
                        if (fcntl(arg, F_SETFD, FD_CLOEXEC) < 0) err = errno;
                        break;
#endif
                    case SPAWN_noexec:
                        callback = 0;
                        vex->noexec = k;
                        break;
                    case SPAWN_pgrp:
                        /* parent may succeed and cause setpigid() to fail but that's ok */
                        if (setpgid(0, arg) < 0 && arg && errno == EPERM) setpgid(arg, 0);
                        vex->pgrp = (pid_t)arg;
                        break;
                    case SPAWN_resetids:
                        if (arg == 1) {
                            if (geteuid() == 0 &&
                                (setuid(geteuid()) < 0 || setgid(getegid()) < 0)) {
                                err = errno;
                            }
                        } else if (setuid(getuid()) < 0 || setgid(getgid()) < 0) {
                            err = errno;
                        }
                        break;
                    case SPAWN_sid:
                        if (setsid() < 0) err = errno;
                        break;
                    case SPAWN_sigdef:
                        err = ENOSYS;
                        break;
                    case SPAWN_sigmask:
                        err = ENOSYS;
                        break;
                    case SPAWN_truncate:
                        if (callback) {
                            err = (*callback)(handle, op, arg);
                            if (err < 0) continue;
                            callback = 0;
                            if (err) break;
                        }
                        if ((off = lseek(arg, 0, SEEK_CUR)) < 0 || ftruncate(arg, off) < 0) {
                            err = errno;
                        }
                        break;
                    case SPAWN_umask:
                        umask(arg);
                        break;
                    default:
                        if (op < 0) {
                            err = EINVAL;
                        } else if (arg < 0) {
                            if (callback) {
                                err = (*callback)(handle, op, arg);
                                if (err < 0) continue;
                                callback = 0;
                                if (err) break;
                            }
#if 0
                                                if (close(op))
                                                        err = errno;
#else
                            close(op);
#endif
                        } else if (op == arg) {
                            if (fcntl(op, F_SETFD, 0) < 0) err = errno;
                        } else {
                            close(arg);
                            if (fcntl(op, F_DUPFD, arg) < 0) err = errno;
                        }
                        break;
                }
                if (err) {
                    if (!(flags & SPAWN_FLUSH)) return err;
                    ret = err;
                } else if (callback) {
                    err = (*callback)(handle, op, arg);
                    if (err > 0) {
                        if (!(flags & SPAWN_FLUSH)) return err;
                        ret = err;
                    }
                }
            } else if (op >= 0 && arg >= 0 && op != arg) {
                close(op);
            }
        }
    }
    if (!(flags & SPAWN_NOCALL)) {
        if (!(flags & SPAWN_FRAME)) {
            vex->frame = 0;
        } else if (vex->op && (vex->op[vex->frame].number / 2) == SPAWN_frame) {
            cur = vex->frame;
            vex->frame = (unsigned int)vex->op[vex->frame + 1].number;
        }
        if (!(vex->cur = cur)) VEXINIT(vex);
    }
    return ret;
}

/*
 * return the fd that is redirected to fd
 * otherwise -1
 *
 * what is the fd that is redirected to 2? spawnvex_get(vex, 2, 0)
 */

int64_t spawnvex_get(Spawnvex_t *vex, int fd, int i) {
    int od;
    int op;
    int arg;
    int k;
    int m;

    if (i >= 0 && i < vex->max) {
        od = fd;
        m = vex->cur;
        while (i < m) {
            k = i;
            op = vex->op[i++].number;
            arg = vex->op[i++].number;
            if (op & 1) i += 2;
            op /= 2;
            if (op >= 0 && arg == fd) {
                fd = op;
                m = k;
                i = 0;
            }
        }
        if (fd != od) return fd;
    }
    return -1;
}

int spawnvex_close(Spawnvex_t *vex) {
    if (!vex) return -1;
    if (vex->op) free(vex->op);
    if (vex->debug >= 0) close(vex->debug);
    free(vex);
    return 0;
}

pid_t spawnvex(const char *path, char *const argv[], char *const envv[], Spawnvex_t *vex) {
    int i;
    int op;
    unsigned int flags = 0;
    pid_t pid;
    int arg;
    int err;
    int fd;
    posix_spawnattr_t ax;
    posix_spawn_file_actions_t fx;
    Spawnvex_t *xev = NULL;
#if _lib_spawn_mode || _lib_spawn && _mem_pgroup_inheritance
    pid_t pgid;
    int arg;
    int j;
    int k;
    int m;
    int ic;
    int jc;
    Spawnvex_t *xev;
#if !_lib_spawn_mode
    int *map;
    int a;
    struct inheritance inherit;
#endif
#endif

    if (vex && vex->debug >= 0) {
        error(ERROR_OUTPUT, vex->debug, "spawnvex exe %4d %8d %p %4d \"%s\"", __LINE__, getpid(),
              vex, vex->cur, path);
    }
#if _lib_spawn_mode || _lib_spawn && _mem_pgroup_inheritance
    if (!envv) envv = environ;
    pid = -1;
    m = 0;
    if (vex) {
        vex->noexec = -1;
        vex->pgrp = -1;
        flags = vex->flags;
        if (!(xev = spawnvex_open(0))) goto bad;
        j = -1;
        for (i = 0; i < vex->cur;) {
            op = vex->op[i++].number;
            arg = vex->op[i++].number;
            if (op & 1) i += 2;
            op /= 2;
            if (op >= 0) {
                if (m < op) m = op + 1;
                if (m < arg) m = arg + 1;
            } else if (op == SPAWN_cwd)
                j = arg;
        }
        if (j >= 0) {
            if ((i = open(".", O_RDONLY)) < 0) goto bad;
            if ((i = save(i, &ic, m)) < 0) goto bad;
            if (spawnvex_add(xev, SPAWN_cwd, i, 0, 0) < 0 || restore(xev, i, -1, 0) < 0) {
                close(i);
                goto bad;
            }
            if (fchdir(j) < 0) goto bad;
            if ((i = save(j, &jc, m)) < 0) goto bad;
            if (restore(xev, i, j, jc) < 0) {
                close(i);
                goto bad;
            }
        }
    } else {
        flags = 0;
        xev = NULL;
    }
#if _lib_spawn_mode
    if (vex)
        for (i = 0; i < vex->cur;) {
            op = vex->op[i++].number;
            arg = vex->op[i++].number;
            if (op & 1) i += 2;
            switch (op /= 2) {
                case SPAWN_frame:
                    vex->frame = (unsigned int)arg;
                    break;
                case SPAWN_pgrp:
                    vex->pgrp = (pid_t)arg;
                    break;
                default:
                    if (op >= 0) {
                        if (arg < 0) {
                            if ((i = save(op, &ic, m)) < 0) {
                                if (i < -1) goto bad;
                            } else if (restore(xev, i, op, ic) < 0) {
                                close(i);
                                goto bad;
                            } else
                                close(op);
                        } else if (arg == op) {
                            if (spawnvex_add(xev, SPAWN_cloexec, arg, 0, 0) < 0) goto bad;
                            if (fcntl(arg, F_SETFD, 0) < 0) goto bad;
                        } else if ((j = save(arg, &jc, m)) < -1)
                            goto bad;
                        else {
                            close(arg);
                            if (fcntl(op, F_DUPFD, arg) >= 0) {
                                if ((i = save(op, &ic, m)) >= 0) {
                                    if (restore(xev, i, op, ic) >= 0) {
                                        close(op);
                                        if (j < 0 || restore(xev, j, arg, jc) >= 0) continue;
                                    }
                                    close(i);
                                }
                            }
                            if (j >= 0) {
                                fcntl(j, F_DUPFD, arg);
                                close(j);
                            }
                            goto bad;
                        }
                    }
                    break;
            }
        }
    pid = spawnve(vex && vex->pgrp >= 0 ? P_DETACH : P_NOWAIT, path, argv, envv);
#else
    inherit.flags = 0;
    map = 0;
    if (vex) {
        if (m) {
            map = calloc(1, m * sizeof(int));
            if (!map) goto bad;
            for (i = 0; i < m; i++) map[i] = i;
        }
        for (i = 0; i < vex->cur;) {
            op = vex->op[i++].number;
            a = i;
            arg = vex->op[i++].number;
            if (op & 1) i += 2;
            switch (op /= 2) {
                case SPAWN_noop:
                    break;
                case SPAWN_noexec:
                    break;
                case SPAWN_frame:
                    vex->frame = (unsigned int)arg;
                    break;
                case SPAWN_pgrp:
                    inherit.flags |= SPAWN_SETGROUP;
                    inherit.pgroup = arg ? arg : SPAWN_NEWPGROUP;
                    break;
                case SPAWN_sigdef:
                    inherit.flags |= SPAWN_SETSIGDEF;
                    sigemptyset(&inherit.sigdefault);
                    for (j = 1; j < 8 * sizeof(vex->op[a].number); j++)
                        if (vex->op[a].number & (1 << j)) sigaddset(&inherit.sigdefault, j);
                    break;
                case SPAWN_sigmask:
                    inherit.flags |= SPAWN_SETSIGMASK;
                    sigemptyset(&inherit.sigmask);
                    for (j = 1; j < 8 * sizeof(vex->op[a].number); j++)
                        if (vex->op[a].number & (1 << j)) sigaddset(&inherit.sigmask, j);
                    break;
                default:
                    if (op < 0) {
                        errno = EINVAL;
                        goto bad;
                    } else if (arg < 0)
                        map[op] = SPAWN_FDCLOSED;
                    else
                        map[op] = arg;
                    break;
            }
        }
    }
    pid = spawn(path, m, map, &inherit, (const char **)argv, (const char **)envv);
#endif
    if (pid >= 0 && vex) VEXINIT(vex);
bad:
    if (xev) {
        spawnvex_apply(xev, 0, SPAWN_FLUSH | SPAWN_NOCALL);
        spawnvex_close(xev);
    }
#if !_lib_spawn_mode
    if (map) free(map);
#endif
    return pid;

#else

#if _lib_spawnve
    if (!vex || !vex->cur && !vex->flags) return spawnve(path, argv, envv);
#endif
    if (vex && ((vex->set & (0
#if !_lib_posix_spawnattr_setfchdir
                             | VEXFLAG(SPAWN_cwd)
#endif
#if !_lib_posix_spawnattr_setsid
                             | VEXFLAG(SPAWN_sid)
#endif
#if !_lib_posix_spawnattr_setumask
                             | VEXFLAG(SPAWN_umask)
#endif
                                 ))
#if !_lib_posix_spawn
                || !(vex->flags & SPAWN_EXEC)
#endif
                    )) {
        int n;
        int m;
        Spawnvex_noexec_t nx;
        int msg[2];

        if (!envv) envv = environ;
        n = errno;
        if (pipe(msg) < 0) {
            msg[0] = msg[1] = -1;
        } else {
            (void)fcntl(msg[0], F_SETFD, FD_CLOEXEC);
            (void)fcntl(msg[1], F_SETFD, FD_CLOEXEC);
        }
        if (!(flags & SPAWN_FOREGROUND)) sigcritical(SIG_REG_EXEC | SIG_REG_PROC);
        pid = fork();
        if (pid == -1) {
            n = errno;
        } else if (!pid) {
            if (!(flags & SPAWN_FOREGROUND)) sigcritical(SIG_REG_POP);
            if (vex && (n = spawnvex_apply(vex, 0, SPAWN_FRAME | SPAWN_NOCALL))) {
                errno = n;
            } else {
                if (vex && vex->debug >= 0) {
                    error(ERROR_OUTPUT, vex->debug, "spawnvex exe %4d %8d %p %4d \"%s\"", __LINE__,
                          getpid(), vex, vex->cur, path);
                }
                execve(path, argv, envv);
                if (vex && vex->debug >= 0) {
                    error(ERROR_OUTPUT, vex->debug, "spawnvex exe %4d %8d %p %4d \"%s\" FAILED",
                          __LINE__, getpid(), vex, vex->cur, path);
                }
                if (vex && (i = vex->noexec) >= 0) {
                    nx.vex = vex;
                    nx.handle = vex->op[i + 3].handle;
                    nx.path = path;
                    nx.argv = argv;
                    nx.envv = envv;
#if _use_spawn_exec
                    /*
                     * setting SPAWN_EXEC here means that it is more efficient to
                     * exec(interpreter) on script than to fork() initialize and
                     * read script -- highly subjective, based on some ksh
                     * implementaions, and probably won't be set unless its a
                     * noticable win
                     */

                    nx.flags |= SPAWN_EXEC;
#endif
                    nx.msgfd = msg[1];
                    errno = (*vex->op[i + 2].callback)(&nx, SPAWN_noexec, errno);
                }
            }
            if (msg[1] != -1) {
                m = errno;
                write(msg[1], &m, sizeof(m));
            }
            _exit(errno == ENOENT ? EXIT_NOTFOUND : EXIT_NOEXEC);
        }
        if (msg[0] != -1) {
            close(msg[1]);
            if (pid != -1) {
                m = 0;
                while (read(msg[0], &m, sizeof(m)) == -1) {
                    if (errno != EINTR) {
                        m = errno;
                        break;
                    }
                }
                if (m) {
                    while (waitpid(pid, &n, 0) && errno == EINTR) {
                        ;
                    }
                    pid = -1;
                    n = m;
                }
            }
            close(msg[0]);
        }
        if (!(flags & SPAWN_FOREGROUND)) sigcritical(SIG_REG_POP);
        if (pid != -1 && vex) VEXINIT(vex);
        errno = n;
        return pid;
    }
    if (vex) {
        err = posix_spawnattr_init(&ax);
        if (err) goto nope;
        err = posix_spawn_file_actions_init(&fx);
        if (err) {
            posix_spawnattr_destroy(&ax);
            goto nope;
        }
        for (i = 0; i < vex->cur;) {
            op = vex->op[i++].number;
            arg = vex->op[i++].number;
            if (op & 1) i += 2;
            switch (op /= 2) {
                case SPAWN_noop:
                    break;
                case SPAWN_noexec:
                    break;
                case SPAWN_frame:
                    break;
#if _lib_posix_spawnattr_setfchdir
                case SPAWN_cwd:
                    err = posix_spawnattr_setfchdir(&ax, arg);
                    if (err) goto bad;
                    break;
#endif
                case SPAWN_pgrp:
                    err = posix_spawnattr_setpgroup(&ax, arg);
                    if (err) goto bad;
                    err = posix_spawnattr_setflags(&ax, POSIX_SPAWN_SETPGROUP);
                    if (err) goto bad;
                    break;
                case SPAWN_resetids:
                    err = posix_spawnattr_setflags(&ax, POSIX_SPAWN_RESETIDS);
                    if (err) goto bad;
                    break;
#if _lib_posix_spawnattr_setsid
                case SPAWN_sid:
                    err = posix_spawnattr_setsid(&ax, arg);
                    if (err) goto bad;
                    break;
#endif
                case SPAWN_sigdef:
                    break;
                case SPAWN_sigmask:
                    break;
#if _lib_posix_spawnattr_setumask
                case SPAWN_umask:
                    if (err = posix_spawnattr_setumask(&ax, arg)) goto bad;
                    break;
#endif
                default:
                    if (op < 0) {
                        err = EINVAL;
                        goto bad;
                    } else if (arg < 0) {
                        err = posix_spawn_file_actions_addclose(&fx, op);
                        if (err) goto bad;
                    } else if (arg == op) {
#ifdef F_DUPFD_CLOEXEC
                        if ((fd = fcntl(op, F_DUPFD_CLOEXEC, 0)) < 0)
#else
                        if ((fd = fcntl(op, F_DUPFD, 0)) < 0 ||
                            fcntl(fd, F_SETFD, FD_CLOEXEC) < 0 && (close(fd), 1))
#endif
                        {
                            err = errno;
                            goto bad;
                        }
                        if (!xev && !(xev = spawnvex_open(0))) goto bad;
                        spawnvex_add(xev, fd, -1, 0, 0);
                        err = posix_spawn_file_actions_adddup2(&fx, fd, op);
                        if (err) goto bad;
                    } else {
                        err = posix_spawn_file_actions_adddup2(&fx, op, arg);
                        if (err) goto bad;
                    }
                    break;
            }
        }

        // Ensure stdin, stdout, stderr are open in the child process.
        // See https://github.com/att/ast/issues/1117.
        for (int fd = 0; fd < 3; ++fd) {
            errno = 0;
            if (fcntl(fd, F_GETFD, NULL) == -1 || errno == EBADF) {
                err = posix_spawn_file_actions_addopen(&fx, fd, "/dev/null", O_RDWR, 0);
                if (err) goto bad;
            }
        }

        err = posix_spawn(&pid, path, &fx, &ax, argv, envv ? envv : environ);
        if (err) goto bad;
        posix_spawnattr_destroy(&ax);
        posix_spawn_file_actions_destroy(&fx);
        if (xev) {
            spawnvex_apply(xev, 0, SPAWN_NOCALL);
            spawnvex_close(xev);
        }
        if (vex->flags & SPAWN_CLEANUP) spawnvex_apply(vex, 0, SPAWN_FRAME | SPAWN_CLEANUP);
        VEXINIT(vex);
    } else {
        err = posix_spawn(&pid, path, NULL, NULL, argv, envv ? envv : environ);
        if (err) goto nope;
    }
    if (vex && vex->debug >= 0) {
        error(ERROR_OUTPUT, vex->debug, "spawnvex exe %4d %8d %p %4d \"%s\" %8d posix_spawn",
              __LINE__, getpid(), vex, vex->cur, path, pid);
    }
    return pid;
bad:
    posix_spawnattr_destroy(&ax);
    posix_spawn_file_actions_destroy(&fx);
    if (xev) {
        spawnvex_apply(xev, 0, SPAWN_NOCALL);
        spawnvex_close(xev);
    }
nope:
    errno = err;
    if (vex && vex->debug >= 0) {
        error(ERROR_OUTPUT, vex->debug, "spawnvex exe %4d %8d %p %4d \"%s\" %8d posix_spawn FAILED",
              __LINE__, getpid(), vex, vex->cur, path, -1);
    }
    return -1;
#endif
}

#endif  // !_lib_spawnvex
