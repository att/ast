/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2011 AT&T Intellectual Property          *
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
//  Glenn Fowler
//  AT&T Bell Laboratories
//
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "ast.h"

//  Return current PATH
//
char *pathbin(void) {
    char *path;
    // Used to restore value of saved astconf path from earlier function call
    static char *astconf_path = NULL;

    path = getenv("PATH");
    if (!path || !*path) path = astconf_path;

    if (!path) {
        path = astconf("PATH", NULL, NULL);
        if (path && *path) path = strdup(path);
        if (!path) {
            path = "/bin:/usr/bin:/usr/local/bin";
        }
        astconf_path = path;
    }

    return path;
}
