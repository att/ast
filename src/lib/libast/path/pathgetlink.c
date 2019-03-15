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
// Glenn Fowler
// AT&T Bell Laboratories
//
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <unistd.h>

//
// Return external representation for symbolic link text of name in `buff`
// the link text string length is returned
//
int pathgetlink(const char *name, char *buff, int buffsize) {
    int n;

    if ((n = readlink(name, buff, buffsize)) < 0) return -1;
    if (n >= buffsize) {
        errno = EINVAL;
        return -1;
    }

    buff[n] = 0;
    return n;
}
