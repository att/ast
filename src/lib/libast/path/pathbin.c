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

#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"

#define CS_PATH_LEN 1024

static const char *def_cs_path = "/bin:/usr/bin:/usr/local/bin";

//
//  Return default PATH to find all system utilities.
//
char *cs_path() {
    static bool path_ok = false;
    static char saved_cs_path[CS_PATH_LEN] = {0};

    if (path_ok) return saved_cs_path;
    path_ok = true;

    size_t rv = confstr(_CS_PATH, saved_cs_path, CS_PATH_LEN);
    if (rv == 0) strlcpy(saved_cs_path, def_cs_path, CS_PATH_LEN);
    // Make sure the string is null terminated in case the buf was too small for confstr().
    saved_cs_path[CS_PATH_LEN - 1] = '\0';
    return saved_cs_path;
}

//
//  Return current PATH
//
char *pathbin(void) {
    char *path = getenv("PATH");
    if (!path || !*path) path = cs_path();
    return path;
}
