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
//
// Glenn Fowler
// AT&T Research
//
// path name canonicalization -- preserves the logical view
//
//      Remove redundant `.`'s and `/`'s
//      Move `..`'s to the front
//      /.. preserved (for pdu and newcastle hacks)
//      if (flags&PATH_PHYSICAL) then symlinks resolved at each component
//      if (flags&(PATH_DOTDOT|PATH_PHYSICAL)) then each .. checked for access
//      if (flags&PATH_EXISTS) then path must exist at each component
//
// Longer pathname possible if (flags&PATH_PHYSICAL) involved
// 0 returned on error and if (flags&(PATH_DOTDOT|PATH_EXISTS)) then canon
// will contain the components following the failure point
//
// pathcanon() return pointer to trailing 0 in canon
//
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>

#include "ast.h"
#include "ast_assert.h"

#ifndef ELOOP
#define ELOOP EINVAL
#endif

char *pathcanon(char *path, size_t size, int flags) {
    char *p;
    char *next_char_after_triple_dot;
    // Next character to read from input path
    char *next;
    // This points to end of canonical path
    char *canonical_path;
    // Points to valid physical canonical path. This path is verfied to exist with `stat()`.
    char *physical_canonical_path;
    // Points to physical path
    char *physical_path = path;
    int dots = 0;
    int loop = 0;
    int oerrno = errno;

    physical_canonical_path = path + ((flags >> 5) & 01777);
    if (!size) size = strlen(path) + 1;
    p = next_char_after_triple_dot = next = canonical_path = path;
    for (;;) {
        switch (*canonical_path++ = *next++) {
            case '.': {
                dots++;
                break;
            }
            case 0: {
                // If we hit end of path, fall through to next block to canonicalize path
                next--;
            }
            // FALLTHRU
            case '/': {
                while (*next == '/') next++;
                switch (dots) {
                    case 1: {
                        // Path contains single dot
                        // Single dot refers to current directory, move canonical_path pointer to 2
                        // characters back i.e. `/foo/bar/.` becomes `/foo/bar`
                        canonical_path -= 2;
                        break;
                    }
                    case 2: {
                        // Path contains double dot
                        if ((flags & (PATH_DOTDOT | PATH_EXISTS)) == PATH_DOTDOT &&
                            (canonical_path - 2) >= physical_canonical_path) {
                            struct stat st;

                            // If `PATH_DOTDOT` is set, try to verify if path actually exists.
                            *(canonical_path - 2) = 0;
                            if (stat(physical_path, &st)) {
                                assert(size > strlen(next));
                                strcpy(path, next);
                                return 0;
                            }
                            *(canonical_path - 2) = '.';
                        }
                        if (canonical_path - 5 < next_char_after_triple_dot) {
                            if (canonical_path - 4 == next_char_after_triple_dot) {
                                canonical_path = next_char_after_triple_dot + 1;
                            } else {
                                next_char_after_triple_dot = canonical_path;
                            }
                        } else {
                            for (canonical_path -= 5; canonical_path > next_char_after_triple_dot &&
                                                      *(canonical_path - 1) != '/';
                                 canonical_path--) {
                                ;  // empty loop
                            }
                        }
                        break;
                    }
                    case 3: {
                        // Path contains triple dot
                        next_char_after_triple_dot = canonical_path;
                        break;
                    }
                    default: {
                        // Path does not contain any dots. Dots from previous paths should be
                        // resolved by this point.
                        if ((flags & PATH_PHYSICAL) && loop < 32 && (canonical_path - 1) > path) {
                            int c;
                            char buf[PATH_MAX];

                            c = *(canonical_path - 1);
                            *(canonical_path - 1) = 0;
                            // Try to resolve symbolic link
                            dots = pathgetlink(physical_path, buf, sizeof(buf));
                            *(canonical_path - 1) = c;
                            if (dots > 0) {
                                // Symbolic link refers to valid physcal path
                                loop++;
                                strlcpy(buf + dots, next - (*next != 0), sizeof(buf) - dots);
                                if (*buf == '/') p = next_char_after_triple_dot = path;
                                physical_canonical_path = next = canonical_path = p;
                                strcpy(p, buf);
                            } else if (dots < 0 && errno == ENOENT) {
                                // Symbolic link is broken
                                if (flags & PATH_EXISTS) {
                                    assert(size > strlen(next));
                                    strcpy(path, next);
                                    return 0;
                                }
                                flags &= ~(PATH_PHYSICAL | PATH_DOTDOT);
                            }
                            // All dots have been resolved
                            dots = 4;
                        }
                    }
                }
                if (dots >= 4 && (flags & PATH_EXISTS) &&
                    (canonical_path - 1) >= physical_canonical_path &&
                    (canonical_path > path + 1 || (canonical_path > path && *(canonical_path - 1) &&
                                                   *(canonical_path - 1) != '/'))) {
                    // Check if a path exists
                    struct stat st;

                    *(canonical_path - 1) = 0;
                    if (stat(physical_path, &st)) {
                        assert(size >= strlen(next));
                        strcpy(path, next);
                        return 0;
                    }
                    physical_canonical_path = canonical_path;
                    if (*next) *(canonical_path - 1) = '/';
                }
                if (!*next) {
                    if (canonical_path > path && !*(canonical_path - 1)) canonical_path--;
                    if (canonical_path == path) {
                        *canonical_path++ = '.';
                    } else if ((next <= path || *(next - 1) != '/') && canonical_path > path + 1 &&
                               *(canonical_path - 1) == '/') {
                        canonical_path--;
                    }
                    *canonical_path = 0;
                    errno = oerrno;
                    return canonical_path;
                }
                dots = 0;
                p = canonical_path;
                break;
            }
            default: {
                // Character is not `/`, `.` or null character
                dots = 4;
                break;
            }
        }
    }
}
