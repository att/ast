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
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>

#if _hdr_unistd
#include <unistd.h>
#endif

/*
 * return external representation for symbolic link text of name in buf
 * the link text string length is returned
 */

int pathgetlink(const char *name, char *buf, int siz) {
    int n;

    if ((n = readlink(name, buf, siz)) < 0) return -1;
    if (n >= siz) {
        errno = EINVAL;
        return -1;
    }
    buf[n] = 0;
    return n;
}
