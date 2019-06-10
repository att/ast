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
// Return path to file path1/path2 with access mode using `:` separated dirs
// both path1 and path2 may be NULL
// If path1==".." then relative paths in dirs are ignored
// If (mode&PATH_REGULAR) then path must not be a directory
// If (mode&PATH_ABSOLUTE) then path must be rooted
// path returned in path buffer
//

#include "config_ast.h"  // IWYU pragma: keep

#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"

char *pathaccess(const char *dirs, const char *path1, const char *path2, int mode, char *path,
                 size_t size) {
    bool is_path1_parentdir = path1 && !strcmp(path1, "..");
    int separator = ':';
    char cwd[PATH_MAX];

    do {
        dirs = pathcat(dirs, separator, path1, path2, path, size);
        // Remove extra `.` and `/`. Resolve `..`.
        pathcanon(path, size, 0);
        if ((!is_path1_parentdir || *path == '/') && pathexists(path, mode)) {
            if (*path == '/' || !(mode & PATH_ABSOLUTE)) return path;
            // If path is relative, try to resolve it with current directory.
            dirs = getcwd(cwd, sizeof(cwd));
            separator = 0;
        }
    } while (dirs);

    return NULL;
}
