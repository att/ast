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
// Return base representation for number
// If prefix!=0 then base prefix is included
// Otherwise if n==0 or b==0 then output is signed base 10
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stdint.h>

#include "ast.h"
#include "sfio.h"

char *fmtbase(int64_t number, int base, int prefix) {
    char *buf;
    int bufsize = 72;

    if (!prefix) {
        if (!number) return "0";
        if (!base) return fmtint(number, 0);
        if (base == 10) return fmtint(number, 1);
    }

    // This function allocates memory through `fmtbuf()` function
    // so it's returned value should be duped if it is to be kept persistent
    buf = fmtbuf(bufsize);
    sfsprintf(buf, bufsize, prefix ? "%#..*I*u" : "%..*I*u", base, sizeof(number), number);
    return buf;
}
