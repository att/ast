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
// AT&T Bell Laboratories
//
// Single dir support for pathaccess()
//

#include "config_ast.h"  // IWYU pragma: keep

#include <stddef.h>

#include "ast.h"  // IWYU pragma: keep

// This function:
// - Splits `root` path at first `separator` and adds string before `separator` to `concat_path`.
// - Appends `path1` and `path2` to `concat_path` separated by '/'. Both `path1` and `path2` may be
// 0.
// - A pointer to string after `separator` in `root` is returned, 0 when there are no more
// components.
// `pathcat` is used by `pathaccess`.

char *pathcat(const char *root, int separator, const char *path1, const char *path2,
              char *concat_path, size_t size) {
    char *current;
    char *end;

    current = concat_path;
    end = concat_path + size;

    // Split `root` at first `separator`
    while (*root && *root != separator) {
        if (current >= end) return NULL;
        *current++ = *root++;
    }

    // Append a '/' before concatenating `path1`
    if (current != concat_path) {
        if (current >= end) return NULL;
        *current++ = '/';
    }

    // Append `path1` to `concat_path`
    if (path1) {
        while ((*current = *path1++)) {
            if (++current >= end) return NULL;
        }

        // If `path2` is not NULL, add a `/` before appending it
        if (path2) {
            if (current >= end) return NULL;
            *current++ = '/';
        }
    } else if (!path2) {
        // TODO: Why is a `.` appended here ?
        path2 = ".";
    }

    // Append `path2` to `concat_path`
    if (path2) {
        do {
            if (current >= end) return NULL;
        } while ((*current++ = *path2++));
    }

    // Return pointer to next separated components in `root` after first `separator`.
    return *root ? (char *)++root : NULL;
}
