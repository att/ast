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
#include "config_ast.h"  // IWYU pragma: keep

#include <stdarg.h>
#include <string.h>

#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"

/*      Read formated data from a stream
**
**      Written by Kiem-Phong Vo.
*/

int sfscanf(Sfio_t *f, const char *form, ...) {
    va_list args;
    int rv;

    va_start(args, form);

    rv = (f && form) ? sfvscanf(f, form, args) : -1;
    va_end(args);
    return rv;
}

int sfvsscanf(const char *s, const char *form, va_list args) {
    Sfio_t f;

    if (!s || !form) return -1;

    /* make a fake stream */
    SFCLEAR(&f, NULL);
    f.flags = SF_STRING | SF_READ;
    f.bits = SF_PRIVATE;
    f.mode = SF_READ;
    f.size = strlen((char *)s);
    f.data = f.next = f.endw = (uchar *)s;
    f.endb = f.endr = f.data + f.size;

    return sfvscanf(&f, form, args);
}

int sfsscanf(const char *s, const char *form, ...) {
    va_list args;
    int rv;
    va_start(args, form);

    rv = (s && form) ? sfvsscanf(s, form, args) : -1;
    va_end(args);
    return rv;
}
