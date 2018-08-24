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
 * path name canonicalization -- preserves the logical view
 *
 *	remove redundant .'s and /'s
 *	move ..'s to the front
 *	/.. preserved (for pdu and newcastle hacks)
 *	if (flags&PATH_ABSOLUTE) then pwd prepended to relative paths
 *	if (flags&PATH_PHYSICAL) then symlinks resolved at each component
 *	if (flags&(PATH_DOTDOT|PATH_PHYSICAL)) then each .. checked for access
 *	if (flags&PATH_EXISTS) then path must exist at each component
 *	if (flags&PATH_VERIFIED(n)) then first n chars of path exist
 *
 * longer pathname possible if (flags&PATH_PHYSICAL) involved
 * 0 returned on error and if (flags&(PATH_DOTDOT|PATH_EXISTS)) then canon
 * will contain the components following the failure point
 *
 * pathcanon() return pointer to trailing 0 in canon
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "ast.h"

#ifndef ELOOP
#define ELOOP EINVAL
#endif

char *pathcanon(char *path, size_t size, int flags) {
    char *p;
    char *r;
    char *s;
    char *t;
    char *v;
    char *phys = path;
    int dots = 0;
    int loop = 0;
    int oerrno = errno;

    v = path + ((flags >> 5) & 01777);
    if (!size) size = strlen(path) + 1;
    if (*path == '/') {
        if (*(path + 1) == '/' && *astconf("PATH_LEADING_SLASHES", NULL, NULL) == '1') {
            do {
                path++;
            } while (*path == '/' && *(path + 1) == '/');
        }
        if (!*(path + 1)) return path + 1;
    }
    p = r = s = t = path;
    for (;;) {
        switch (*t++ = *s++) {
            case '.': {
                dots++;
                break;
            }
            case 0: {
                s--;
            }
            // FALLTHRU
            case '/': {
                while (*s == '/') s++;
                switch (dots) {
                    case 1: {
                        t -= 2;
                        break;
                    }
                    case 2: {
                        if ((flags & (PATH_DOTDOT | PATH_EXISTS)) == PATH_DOTDOT && (t - 2) >= v) {
                            struct stat st;

                            *(t - 2) = 0;
                            if (stat(phys, &st)) {
                                strcpy(path, s);
                                return 0;
                            }
                            *(t - 2) = '.';
                        }
                        if (t - 5 < r) {
                            if (t - 4 == r) {
                                t = r + 1;
                            } else {
                                r = t;
                            }
                        } else {
                            for (t -= 5; t > r && *(t - 1) != '/'; t--) {
                                ;  // empty loop
                            }
                        }
                        break;
                    }
                    case 3: {
                        r = t;
                        break;
                    }
                    default: {
                        if ((flags & PATH_PHYSICAL) && loop < 32 && (t - 1) > path) {
                            int c;
                            char buf[PATH_MAX];

                            c = *(t - 1);
                            *(t - 1) = 0;
                            dots = pathgetlink(phys, buf, sizeof(buf));
                            *(t - 1) = c;
                            if (dots > 0) {
                                loop++;
                                strcpy(buf + dots, s - (*s != 0));
                                if (*buf == '/') p = r = path;
                                v = s = t = p;
                                strcpy(p, buf);
                            } else if (dots < 0 && errno == ENOENT) {
                                if (flags & PATH_EXISTS) {
                                    strcpy(path, s);
                                    return 0;
                                }
                                flags &= ~(PATH_PHYSICAL | PATH_DOTDOT);
                            }
                            dots = 4;
                        }
                    }
                }
                if (dots >= 4 && (flags & PATH_EXISTS) && (t - 1) >= v &&
                    (t > path + 1 || (t > path && *(t - 1) && *(t - 1) != '/'))) {
                    struct stat st;

                    *(t - 1) = 0;
                    if (stat(phys, &st)) {
                        strcpy(path, s);
                        return 0;
                    }
                    v = t;
                    if (*s) *(t - 1) = '/';
                }
                if (!*s) {
                    if (t > path && !*(t - 1)) t--;
                    if (t == path) {
                        *t++ = '.';
                    } else if ((s <= path || *(s - 1) != '/') && t > path + 1 && *(t - 1) == '/') {
                        t--;
                    }
                    *t = 0;
                    errno = oerrno;
                    return t;
                }
                dots = 0;
                p = t;
                break;
            }
            default: {
                dots = 4;
                break;
            }
        }
    }
}
