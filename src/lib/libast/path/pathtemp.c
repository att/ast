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

#if !has_dev_fd
//
// Construct a unique temp pathname based on a file name prefix, the current pid, and a monotonic
// counter. This should only be used if a psuedo-unique path is required that does not represent an
// actual file. In the latter case the `ast_temp_file()` function should be used.
//
// This is currently predicated on the `has_dev_fd` build time symbol because it is only needed if
// that is false.
//
static unsigned int temp_path_counter = 0;

char *ast_temp_path(const char *prefix) {
    const char *dir = get_pathtemp_dir();
    if (!dir) return NULL;

    int dir_len = strlen(dir);
    int prefix_len = strlen(prefix);

    while (true) {
        char uuid[20];
        int uuid_len = snprintf(uuid, sizeof(uuid), "%d.%d", getpid(), ++temp_path_counter);
        assert(uuid_len != -1 && uuid_len < sizeof(uuid));

        char *template = malloc(dir_len + prefix_len + uuid_len + 2);
        assert(template);
        strcpy(template, dir);
        if (dir_len && dir[dir_len - 1] != '/') strcat(template, "/");
        strcat(template, prefix);
        if (prefix_len && prefix[prefix_len - 1] != '.') strcat(template, ".");
        strcat(template, uuid);

        struct stat statbuf;
        if (stat(template, &statbuf) == -1) return template;
    }
}
#endif  // !has_dev_fd

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
    assert(fd);

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

    *fd = mkostemp(template, open_flags);
    if (*fd == -1) {
        free(template);
        return NULL;
    }

    return template;
}
