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
// Return ls -l style file mode string given file mode bits
//
#include "config_ast.h"  // IWYU pragma: keep

#include "ast.h"
#include "modelib.h"

//
// This function always returns same static buffer.
// It should be used with care.
//
char *fmtmode(int mode) {
    char *s;
    struct modeop *p;
    static char strbuf[MODELEN + 1];

    s = strbuf;
    for (p = modetab; p < &modetab[MODELEN]; p++) {
        *s++ = p->name[((mode & p->mask1) >> p->shift1) | ((mode & p->mask2) >> p->shift2)];
    }
    *s = 0;
    return strbuf;
}
