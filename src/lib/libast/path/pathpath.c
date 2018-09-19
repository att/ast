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
 * return full path to p with mode access using $PATH
 * a!=0 enables related root search
 * a!=0 && a!="" searches a dir first
 * the related root must have a bin subdir
 * p==0 sets the cached relative dir to a
 * full path returned in path buffer
 * if path==0 then the space is malloc'd
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "ast_api.h"
#include "ast_assert.h"
#include "sfio.h"

char *pathpath(const char *p, const char *a, int mode, char *path, size_t size) {
    char *s;
    char *x;

    static char *cmd;

    assert(path && size);
    if (!p) {
        if (cmd) free(cmd);
        cmd = a ? strdup(a) : NULL;
        return NULL;
    }
    if (strlen(p) < size) {
        strcpy(path, p);
        if (pathexists(path, mode)) {
            if (*p != '/' && (mode & PATH_ABSOLUTE)) {
                char buf[PATH_MAX];

                getcwd(buf, sizeof(buf));
                s = buf + strlen(buf);
                sfsprintf(s, sizeof(buf) - (s - buf), "/%s", p);
                strcpy(path, buf);
            }
            return path;
        }
    }
    if (*p == '/') {
        a = 0;
    } else {
        s = (char *)a;
        if (s) {
            if (strchr(p, '/')) {
                a = p;
                p = "..";
            } else {
                a = 0;
            }
            if ((!cmd || *cmd) && (strchr(s, '/') || (s = cmd))) {
                if (!cmd && *s == '/') cmd = strdup(s);
                if (strlen(s) < size - 6) {
                    s = stpcpy(path, s);
                    for (;;) {
                        do {
                            if (s <= path) goto normal;
                        } while (*--s == '/');
                        do {
                            if (s <= path) goto normal;
                        } while (*--s != '/');
                        strcpy(s + 1, "bin");
                        if (pathexists(path, PATH_EXECUTE)) {
                            s = pathaccess(path, p, a, mode, path, size);
                            if (s) return s;
                            goto normal;
                        }
                    }
                normal:;
                }
            }
        }
    }

    x = !a && strchr(p, '/') ? "" : pathbin();
    if (!(s = pathaccess(x, p, a, mode, path, size)) && !*x && (x = getenv("FPATH"))) {
        s = pathaccess(x, p, a, mode, path, size);
    }
    return s;
}
