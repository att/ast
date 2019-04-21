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
#include <stdlib.h>
#include <string.h>

#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"

/*      Construct a string with the given format and data.
**      These functions allocate space as necessary to store the string.
**      This avoids overflow problems typical with sprintf() in stdio.
**
**      Written by Kiem-Phong Vo.
*/

char *sfvprints(const char *form, va_list args) {
    int rv;
    Sfnotify_f notify = _Sfnotify;
    static Sfio_t *f;

    if (!f) /* make a string stream to write into */
    {
        _Sfnotify = 0;
        f = sfnew(NULL, NULL, (size_t)SF_UNBOUND, -1, SF_WRITE | SF_STRING);
        _Sfnotify = notify;
        if (!f) return NULL;
    }

    sfseek(f, (Sfoff_t)0, SEEK_SET);
    rv = sfvprintf(f, form, args);

    if (rv < 0 || sfputc(f, '\0') < 0) return NULL;

    _Sfi = (f->next - f->data) - 1;
    return (char *)f->data;
}

char *sfprints(const char *form, ...) {
    char *s;
    va_list args;

    va_start(args, form);
    s = sfvprints(form, args);
    va_end(args);

    return s;
}

ssize_t sfvaprints(char **sp, const char *form, va_list args) {
    char *s;
    ssize_t n;

    if (!sp || !(s = sfvprints(form, args))) return -1;
    *sp = malloc(n = strlen(s) + 1);
    if (!*sp) return -1;
    memcpy(*sp, s, n);
    return n - 1;
}

ssize_t sfaprints(char **sp, const char *form, ...) {
    ssize_t n;
    va_list args;

    va_start(args, form);
    n = sfvaprints(sp, form, args);
    va_end(args);

    return n;
}
