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
#include "config_ast.h"  // IWYU pragma: keep

#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"

/*      Print data with a given format
**
**      Written by Kiem-Phong Vo.
*/

int sfprintf(Sfio_t *f, const char *form, ...) {
    va_list args;
    int rv;

    va_start(args, form);
    rv = sfvprintf(f, form, args);

    va_end(args);
    return rv;
}

ssize_t sfvsprintf(char *s, size_t n, const char *form, va_list args) {
    Sfio_t *f;
    ssize_t rv;
    Sfnotify_f notify = _Sfnotify;

    /* make a temp stream */
    _Sfnotify = 0;
    f = sfnew(NULL, NULL, (size_t)SF_UNBOUND, -1, SF_WRITE | SF_STRING);
    _Sfnotify = notify;
    if (!f) return -1;

    if ((rv = sfvprintf(f, form, args)) < 0) return -1;
    if (s && n > 0) {
        if ((rv + 1) >= n) {
            n--;
        } else {
            n = rv;
        }
        memcpy(s, f->data, n);
        s[n] = 0;
    }

    sfclose(f);

    _Sfi = rv;

    return rv;
}

ssize_t sfsprintf(char *s, size_t n, const char *form, ...) {
    va_list args;
    ssize_t rv;

    va_start(args, form);

    rv = sfvsprintf(s, n, form, args);
    va_end(args);

    return rv;
}
