/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Research
 *
 * pwd library support
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ast.h"

#include "ast_dir.h"

#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif

#ifndef ERANGE
#define ERANGE E2BIG
#endif

#define ERROR(e)    \
    {               \
        errno = e;  \
        goto error; \
    }

/*
 * return a pointer to the absolute path name of fd
 * fd must be an fd to a directory open for read
 * the resulting path may be longer than PATH_MAX
 *
 * a few environment variables are checked before the search algorithm
 * return value is placed in buf of len chars
 * if buf is 0 then space is allocated via malloc() with
 * len extra chars after the path name
 * 0 is returned on error with errno set as appropriate
 *
 * NOTE: the !_lib_fdopendir version is neither thread nor longjump safe
 */

char *fgetcwd(int fd, char *buf, size_t len) {
    char *p;
    char *s;
    DIR *dirp = NULL;
    int dd = 0;
    bool dd_closed = false;
    int n;
    int x;
    size_t namlen;
    ssize_t extra = -1;
    struct dirent *entry;
    struct stat *cur;
    struct stat *par;
    struct stat *tmp;
    struct stat curst;
    struct stat parst;
    struct stat tstst;

    static struct {
        char *name;
        char *path;
        dev_t dev;
        ino_t ino;
    } env[] = {
        {NULL, NULL, 0, 0},  // previous
        {"PWD", NULL, 0, 0},
        {"HOME", NULL, 0, 0},
    };

    if (buf && !len) ERROR(EINVAL)
    cur = &curst;
    par = &parst;
    if (fstatat(fd, ".", par, 0)) ERROR(errno)

    for (n = 0; n < elementsof(env); n++) {
        p = env[n].name ? getenv(env[n].name) : NULL;
        if (!p) p = env[n].path;
        if (!p || *p != '/') continue;
        if (stat(p, cur) != 0) continue;

        env[n].path = p;
        env[n].dev = cur->st_dev;
        env[n].ino = cur->st_ino;
        if (cur->st_ino == par->st_ino && cur->st_dev == par->st_dev) {
            namlen = strlen(p) + 1;
            if (buf) {
                if (len < namlen) ERROR(ERANGE)
            } else {
                buf = calloc(1, namlen + len);
                if (!buf) ERROR(ENOMEM)
            }
            return memcpy(buf, p, namlen);
        }
    }

    if (!buf) {
        extra = len;
        len = PATH_MAX;
        buf = calloc(1, len + extra);
        if (!buf) ERROR(ENOMEM)
    }
    p = buf + len - 1;
    *p = 0;
    n = elementsof(env);
    dd = fd;
    if (dd != AT_FDCWD && fchdir(dd)) ERROR(errno)
    for (;;) {
        tmp = cur;
        cur = par;
        par = tmp;
        dd = openat(fd, "..", O_RDONLY | O_NONBLOCK | O_DIRECTORY | O_CLOEXEC);
        if (dd < 0) {
            ERROR(errno)
        }
        if (fstat(dd, par)) ERROR(errno)
        if (par->st_dev == cur->st_dev && par->st_ino == cur->st_ino) {
            dd_closed = true;
            close(dd);
            dd = INT_MIN;
            *--p = '/';
            break;
        }
        if (dirp) closedir(dirp);
        if (!(dirp = fdopendir(dd))) ERROR(errno)
        fd = dd;
#ifdef D_FILENO
        if (par->st_dev == cur->st_dev) {
            while ((entry = readdir(dirp))) {
                if (D_FILENO(entry) == cur->st_ino) {
                    namlen = D_NAMLEN(entry);
                    goto part;
                }
            }

            // This fallthrough handles logical naming.
            rewinddir(dirp);
        }
#endif
        do {
            if (!(entry = readdir(dirp))) ERROR(ENOENT)
        } while (fstatat(fd, entry->d_name, &tstst, AT_SYMLINK_NOFOLLOW) ||
                 tstst.st_ino != cur->st_ino || tstst.st_dev != cur->st_dev);
        namlen = D_NAMLEN(entry);
    part:
        if (*p) *--p = '/';
        while ((p -= namlen) <= (buf + 1)) {
            x = (buf + len - 1) - (p + namlen);
            s = buf + len;
            if (extra < 0) ERROR(ERANGE)
            len += PATH_MAX;
            buf = realloc(buf, len + extra);
            if (!buf) ERROR(ERANGE)
            p = buf + len;
            while (p > buf + len - 1 - x) *--p = *--s;
        }
        if (n < elementsof(env)) {
            memcpy(p, env[n].path, namlen);
            break;
        }
        if (namlen == 1 && entry->d_name[0] == '.') {
            p = buf + len - 1;
            *p = 0;
            *--p = '/';
            break;
        }
        memcpy(p, entry->d_name, namlen);
        for (n = 0; n < elementsof(env); n++) {
            if (env[n].ino == par->st_ino && env[n].dev == par->st_dev) {
                namlen = strlen(env[n].path);
                goto part;
            }
        }
    }
    if (p != buf) {
        s = buf;
        while ((*s++ = *p++)) {
            ;  // empty loop
        }
        len = s - buf;
        if (extra >= 0) {
            buf = realloc(buf, len + extra);
            if (!buf) ERROR(ENOMEM)
        }
    }
    if (env[0].path) free(env[0].path);
    env[0].path = strdup(buf);
    if (dd != AT_FDCWD && !dd_closed) fchdir(dd);
    if (dirp) closedir(dirp);
    return buf;

error:
    if (buf && extra >= 0) free(buf);
    if (dirp) closedir(dirp);
    if (dd != AT_FDCWD) fchdir(dd);
    return NULL;
}
