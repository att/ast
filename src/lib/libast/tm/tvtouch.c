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
 * Tv_t conversion support
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>  // IWYU pragma: keep
#include <time.h>      // IWYU pragma: keep
#include <unistd.h>

#include "tv.h"

#ifndef ALLPERMS
#ifdef S_ISTXT
#define ALLPERMS (S_ISTXT | S_ISUID | S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO)
#else
#define ALLPERMS (S_ISVTX | S_ISUID | S_ISGID | S_IRWXU | S_IRWXG | S_IRWXO)
#endif
#endif

// NOTE: These symbols aren't actually available to any code calling tvtouch() since they are not
// defined in any header that such code could include. See the NOTE below.
//
// TODO: Consider removing these and the code predicated on them since the capabilities controlled
// by them cannot be used.
#define TV_TOUCH_CREATE 1
#define TV_TOUCH_PHYSICAL 2

#define NS(n) (((uint32_t)(n)) < 1000000000L ? (n) : 0)

/*
 * touch path <atime,mtime,ctime>
 * Tv_t==0 uses current time
 * Tv_t==TV_TOUCH_RETAIN retains path value if it exists, current time otherwise
 * otherwise it is exact time
 * file created if it doesn't exist and (flags&TV_TOUCH_CREATE)
 * symlink not followed if (flags&TV_TOUCH_PHYSICAL)
 * cv most likely ignored on most implementations
 *
 * NOTE: when *at() calls are integrated TV_TOUCH_* should be advertized!
 */

// There are two implmentations of tvtouch() below. Which one is used depends on whether the build
// time feature tests finds a usable utimensat(). The implementations are listed in order of
// preference.

#if _lib_utimensat

// The system has a usable utimensat() function so use that.
int tvtouch(const char *path, const Tv_t *av, const Tv_t *mv, const Tv_t *cv, int flags) {
    struct timespec ts[2];
    int oerrno = errno;

    if (!cv && av == TV_TOUCH_RETAIN && mv == TV_TOUCH_RETAIN) {
        struct stat st;
        if (!stat(path, &st) && !chmod(path, st.st_mode & ALLPERMS)) {
            // We were asked to retain existing timestamps but the file doesn't seem to exist. So do
            // nothing and report success.
            //
            // TODO: Figure out if this really the right return code. It seems like -1 to indicate
            // an error is more appropriate since we couldn't do what was requested.
            return 0;
        }
    }

    if (!av) {
        ts[0].tv_sec = 0;
        ts[0].tv_nsec = UTIME_NOW;
    } else if (av == TV_TOUCH_RETAIN) {
        ts[0].tv_sec = 0;
        ts[0].tv_nsec = UTIME_OMIT;
    } else {
        ts[0].tv_sec = av->tv_sec;
        ts[0].tv_nsec = NS(av->tv_nsec);
    }

    if (!mv) {
        ts[1].tv_sec = 0;
        ts[1].tv_nsec = UTIME_NOW;
    } else if (mv == TV_TOUCH_RETAIN) {
        ts[1].tv_sec = 0;
        ts[1].tv_nsec = UTIME_OMIT;
    } else {
        ts[1].tv_sec = mv->tv_sec;
        ts[1].tv_nsec = NS(mv->tv_nsec);
    }

    struct timespec *tsp = (ts[0].tv_nsec == UTIME_NOW && ts[1].tv_nsec == UTIME_NOW) ? NULL : ts;
    int utimensat_flags = (flags & TV_TOUCH_PHYSICAL) ? AT_SYMLINK_NOFOLLOW : 0;
    if (!utimensat(AT_FDCWD, path, tsp, utimensat_flags)) return 0;
    if (errno != ENOENT || !(flags & TV_TOUCH_CREATE)) return -1;

    // Create the file with the requested timestamps.
    int mode = umask(0);
    umask(mode);
    mode = (~mode) & (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, mode);
    if (fd == -1) return -1;
    close(fd);
    errno = oerrno;
    if (ts[0].tv_nsec == UTIME_NOW && ts[1].tv_nsec == UTIME_NOW) return 0;
    return utimensat(AT_FDCWD, path, ts, utimensat_flags);
}

#else  // _lib_utimensat

// The system doesn't appear to have a usable utimensat() so use utimes().
int tvtouch(const char *path, const Tv_t *av, const Tv_t *mv, const Tv_t *cv, int flags) {
    UNUSED(cv);
    struct stat st;
    Tv_t now;
    struct timeval am[2];
    int oerrno = errno;

    memset(&st, 0, sizeof(st));
    if ((av == TV_TOUCH_RETAIN || mv == TV_TOUCH_RETAIN) && stat(path, &st)) {
        errno = oerrno;
        if (av == TV_TOUCH_RETAIN) av = NULL;
        if (mv == TV_TOUCH_RETAIN) mv = NULL;
    }
    if (!av || !mv) {
        tvgettime(&now);
        if (!av) av = (const Tv_t *)&now;
        if (!mv) mv = (const Tv_t *)&now;
    }

    if (av == TV_TOUCH_RETAIN) {
        am[0].tv_sec = st.st_atime;
        am[0].tv_usec = ST_ATIME_NSEC_GET(&st) / 1000;
    } else {
        am[0].tv_sec = av->tv_sec;
        am[0].tv_usec = NS(av->tv_nsec) / 1000;
    }
    if (mv == TV_TOUCH_RETAIN) {
        am[1].tv_sec = st.st_mtime;
        am[1].tv_usec = ST_MTIME_NSEC_GET(&st) / 1000;
    } else {
        am[1].tv_sec = mv->tv_sec;
        am[1].tv_usec = NS(mv->tv_nsec) / 1000;
    }

    if (!utimes(path, am)) return 0;
    if (errno != ENOENT && av == (const Tv_t *)&now && mv == (const Tv_t *)&now &&
        !utimes(path, NULL)) {
        errno = oerrno;
        return 0;
    }

    if (!access(path, F_OK)) {
        if (av != (const Tv_t *)&now || mv != (const Tv_t *)&now) {
            errno = EINVAL;
            return -1;
        }

        int fd = open(path, O_RDWR | O_CLOEXEC);
        if (fd >= 0) {
            int c;

            if (read(fd, &c, 1) == 1) {
                c = lseek(fd, 0, 0) == 0 && write(fd, &c, 1);
                if (c) errno = oerrno;
                close(fd);
                if (c) return 0;
            } else {
                close(fd);
            }
        }
    }
    if (errno != ENOENT || !(flags & TV_TOUCH_CREATE)) return -1;

    int mode = umask(0);
    umask(mode);
    mode = (~mode) & (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, mode);
    if (fd == -1) return -1;
    close(fd);
    errno = oerrno;
    if (av == (const Tv_t *)&now && mv == (const Tv_t *)&now) return 0;
    return utimes(path, am);
}

#endif  // _lib_utimensat
