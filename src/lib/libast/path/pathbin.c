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
#include <unistd.h>

#include "ast.h"

static const char *def_cs_path = "/bin:/usr/bin:/usr/local/bin";
static char *saved_cs_path = NULL;
#define CS_PATH_LEN 1024

//
//  Return default PATH to find all system utilities.
//
const char *cs_path() {
    if (!saved_cs_path) {
        saved_cs_path = malloc(CS_PATH_LEN);
        size_t rv = confstr(_CS_PATH, saved_cs_path, CS_PATH_LEN);
        if (rv == 0) strlcpy(saved_cs_path, def_cs_path, CS_PATH_LEN);
        // Make sure the string is null terminated in case the buf was too small.
        saved_cs_path[CS_PATH_LEN - 1] = '\0';
    }
    return saved_cs_path;
}

//
//  Return current PATH
//
char *pathbin(void) {
    char *path = getenv("PATH");
    if (!path || !*path) path = strdup(cs_path());
    return path;
}
