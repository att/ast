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
// Full path to `path_to_search` is searched in `$PATH` and `$FPATH` with `mode` access
// path_to_search2!=0 enables related root search - ????
// path_to_search2!=0 && path_to_search2!="" searches a dir first - ????
// the related root must have a bin subdir - ????
// path_to_search==0 sets the cached relative dir to a
// full path returned in path buffer - ????
//

#include "config_ast.h"  // IWYU pragma: keep

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "ast_assert.h"
#include "sfio.h"

char *pathpath(const char *path_to_search, const char *path_to_search2, int mode, char *path,
               size_t size) {
    char *result_path;

    // Colon separated list of paths to search
    char *search_path;

    // Saved command from previous invocation
    static char *cached_path;

    assert(path && size);

    // If path to search is empty, save path from `path_to_search2` into `cached_path`.
    // I am not sure what this cached path is supposed to do.
    if (!path_to_search) {
        if (cached_path) free(cached_path);
        cached_path = path_to_search2 ? strdup(path_to_search2) : NULL;
        return NULL;
    }

    if (strlen(path_to_search) < size) {
        strcpy(path, path_to_search);
        if (pathexists(path, mode)) {
            // If mode is `PATH_ABSOLUTE` expand path without beginning `/` to current directory
            if (*path_to_search != '/' && (mode & PATH_ABSOLUTE)) {
                char buf[PATH_MAX];

                getcwd(buf, sizeof(buf));
                result_path = buf + strlen(buf);
                sfsprintf(result_path, sizeof(buf) - (result_path - buf), "/%s", path_to_search);
                strcpy(path, buf);
            }
            return path;
        }
    }

    if (*path_to_search == '/') {
        path_to_search2 = 0;
    } else {
        // This code block seems a bit out of this world
        result_path = (char *)path_to_search2;
        if (result_path) {
            if (strchr(path_to_search, '/')) {
                path_to_search2 = path_to_search;
                path_to_search = "..";
            } else {
                path_to_search2 = 0;
            }
            if ((!cached_path || *cached_path) &&
                (strchr(result_path, '/') || (result_path = cached_path))) {
                if (!cached_path && *result_path == '/') cached_path = strdup(result_path);
                size_t slen = strlen(result_path);
                if (slen < size - 6) {
                    (void)strlcpy(path, result_path, slen + 1);
                    result_path += slen;
                    for (;;) {
                        do {
                            // TODO: In what situation this condition will be false ?
                            if (result_path <= path) goto normal;
                        } while (*--result_path == '/');
                        do {
                            if (result_path <= path) goto normal;
                        } while (*--result_path != '/');
                        strcpy(result_path + 1, "bin");
                        if (pathexists(path, PATH_EXECUTE)) {
                            result_path =
                                pathaccess(path, path_to_search, path_to_search2, mode, path, size);
                            if (result_path) return result_path;
                            goto normal;
                        }
                    }
                normal:;
                }
            }
        }
    }

    search_path = !path_to_search2 && strchr(path_to_search, '/') ? "" : pathbin();
    result_path = pathaccess(search_path, path_to_search, path_to_search2, mode, path, size);

    // If search path was empty, search for it in `$FPATH`
    if (!result_path && !*search_path && (search_path = getenv("FPATH"))) {
        result_path = pathaccess(search_path, path_to_search, path_to_search2, mode, path, size);
    }

    return result_path;
}
