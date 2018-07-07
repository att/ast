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
 *
 * st_dev formatter
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#if _hdr_sys_sysmacros
#include <sys/sysmacros.h>
#endif

#include "ast.h"
#include "ls.h"
#include "sfio.h"

char *fmtdev(struct stat *st) {
    char *buf;
    unsigned long mm;
    unsigned int ma;
    unsigned int mi;
    int z;

    mm = (S_ISBLK(st->st_mode) || S_ISCHR(st->st_mode)) ? idevice(st) : st->st_dev;
    ma = major(mm);
    mi = minor(mm);
    buf = fmtbuf(z = 17);
    if (ma == '#' && isalnum(mi)) {
        /*
         * Plan? Nein!
         */

        buf[0] = ma;
        buf[1] = mi;
        buf[2] = 0;
    } else
        sfsprintf(buf, z, "%03d,%03d", ma, mi);
    return buf;
}
