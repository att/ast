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
// This module is named `pathtemp.c` because it originally exported a public function of that name.
// That function was replaced by ast_temp_file() which is considerably simpler and avoids
// reinventing the wheel. It also more accurately mimics the `mktemp` user command with respect to
// the absolute pathname. See https://github.com/att/ast/issues/854.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"  // IWYU pragma: keep
#include "ast_assert.h"

#define TMP1 "/tmp"
#define TMP2 "/usr/tmp"
#define TEMPLATE "XXXXXXXX"

static_fn char *get_pathtemp_dir() {
    char *tmpdir = getenv("TMPDIR");
    if (tmpdir && eaccess(tmpdir, W_OK | X_OK) == 0) return tmpdir;

#ifdef P_tmpdir
    if (eaccess(P_tmpdir, W_OK | X_OK) == 0) return P_tmpdir;
#endif

    if (eaccess(TMP1, W_OK | X_OK) == 0) return TMP1;
    if (eaccess(TMP2, W_OK | X_OK) == 0) return TMP2;
    return NULL;
}

//
// Create a temp file, open it, and return the dynamically constructed pathname and open fd.
//
// Args:
//   dir: NULL to use the preferred temp dir else the directory in which to create the file.
//   prefix: NULL to use the default "ast" prefix else the desired file name prefix. Can be the
//     empty string.
//   fd: Pointer to the opened file. If NULL we only construct and return a temp file name without
//     creating and opening a file.
//   open_flags: O_CLOEXEC and/or O_APPEND.
//
// Returns:
//   Pointer to the dynamically constructed path name which must be freed by the caller.
//   This will be NULL if anything went wrong.
//
//   The `fd` parameter will be updated to the file descriptor open on the file.
//
char *ast_temp_file(const char *dir, const char *prefix, int *fd, int open_flags) {
    if (!dir) dir = get_pathtemp_dir();
    if (!dir) return NULL;
    int dir_len = strlen(dir);

    if (!prefix) prefix = "ast";
    int prefix_len = strlen(prefix);

    // The first "+ 1" is for the slash between the dir and prefix.
    // The second "+ 1" is for the period between the prefix and template.
    // The sizeof(TEMPLATE) takes care of including space for the terminating null.
    char *template = malloc(dir_len + prefix_len + sizeof(TEMPLATE) + 2);
    assert(template);
    strcpy(template, dir);
    if (dir_len && dir[dir_len - 1] != '/') strcat(template, "/");
    strcat(template, prefix);
    if (prefix_len && prefix[prefix_len - 1] != '.') strcat(template, ".");
    strcat(template, TEMPLATE);

    if (fd) {
        *fd = mkostemp(template, open_flags);
        if (*fd == -1) {
            free(template);
            return NULL;
        }
    } else {
    // Only construct a unique file name from the template. Don't create and open a file.
    // This should only be used if the caller is going to use the name to create something
    // other than a file; e.g., a named fifo.
#ifndef __clang_analyzer__
        // cppcheck-suppress  mktempCalled
        char *tp = mktemp(template);
        assert(tp);
#endif
    }

    return template;
}
