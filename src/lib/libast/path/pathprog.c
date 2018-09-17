/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2012 AT&T Intellectual Property          *
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
 * return the full path of the current program in path
 * command!=0 is used as a default
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#if __CYGWIN__
#include <ctype.h>
#endif

#include "ast.h"

#if __CYGWIN__
#include "ast_windows.h"
#endif

#if _hdr_mach_o_dyld
#include <mach-o/dyld.h>
#endif

static_fn size_t path_prog(const char *command, char *path, size_t size) {
    ssize_t n;
    char *s;
#if __CYGWIN__
    char *t;
    char *e;
    int c;
    int q;
#endif
#if _hdr_mach_o_dyld
    uint32_t z;
#endif

#ifdef _PROC_PROG
    n = readlink(_PROC_PROG, path, size);
    if (n > 0 && *path == '/') {
        // readlink() doesn't null terminate the path so do so now.
        path[n < size ? n : size - 1] = 0;
        return n;
    }
#endif
#if _lib_getexecname
    if ((s = (char *)getexecname()) && *s == '/') goto found;
#endif
#if _hdr_mach_o_dyld
    z = size;
    if (!_NSGetExecutablePath(path, &z) && *path == '/') return strlen(path);
#endif
#if __CYGWIN__
    s = GetCommandLine();
    if (s) {
        n = 0;
        q = 0;
        t = path;
        e = path + size - 1;
        while ((c = *s++)) {
            if (c == q) {
                q = 0;
            } else if (!q && c == '"') {
                q = c;
            } else if (!q && isspace(c)) {
                break;
            } else if (t < e) {
                *t++ = c == '\\' ? '/' : c;
            } else {
                n++;
            }
        }
        if (t < e) *t = 0;
        return (t - path) + n;
    }
#endif
    if (command) {
        s = (char *)command;
        goto found;
    }
    return 0;
found:
    n = strlen(s);
    if (n < size) memcpy(path, s, n + 1);
    return n;
}

size_t pathprog(const char *command, char *path, size_t size) {
    char *rel;
    ssize_t n;

    n = path_prog(command, path, size);
    if (n > 0 && n < size && *path != '/') {
        rel = strdup(path);
        if (rel) {
            n = pathpath(rel, NULL, PATH_REGULAR | PATH_EXECUTE, path, size) ? strlen(path) : 0;
            free(rel);
        }
    }
    return n;
}
