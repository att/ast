/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2011 AT&T Intellectual Property          *
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
 *                    David Korn <dgkorn@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
//
// Routines to implement fast character input
//
// David Korn
// AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "fcin.h"
#include "sfio.h"

Fcin_t _Fcin = {.fcbuff = NULL};

//
// Open stream <f> for fast character input.
//
int fcfopen(Sfio_t *f) {
    int n;
    char *buff;
    Fcin_t save;

    errno = 0;
    _Fcin.fcbuff = _Fcin.fcptr;
    _Fcin._fcfile = f;
    fcsave(&save);
    if (!(buff = (char *)sfreserve(f, SF_UNBOUND, SF_LOCKR))) {
        fcrestore(&save);
        _Fcin.fcchar = 0;
        _Fcin.fcptr = _Fcin.fcbuff = &_Fcin.fcchar;
        _Fcin.fclast = NULL;
        _Fcin._fcfile = NULL;
        return EOF;
    }
    n = sfvalue(f);
    fcrestore(&save);
    sfread(f, buff, 0);
    _Fcin.fcoff = sftell(f);
    buff = (char *)sfreserve(f, SF_UNBOUND, SF_LOCKR);
    _Fcin.fclast = (_Fcin.fcptr = _Fcin.fcbuff = (unsigned char *)buff) + n;
    if (sffileno(f) >= 0) *_Fcin.fclast = 0;
    return n;
}

//
// This was originally implemented as a macro:
//   #define fcgetc(c) (((c = fcget()) || (c = fcfill())), c)
//
// However, that is an ugly API that causes lots of lint warnings.
//
int fcgetc() {
    int c = fcget();
    if (!c) c = fcfill();
    return c;
}

//
// With _Fcin.fcptr>_Fcin.fcbuff, the stream pointer is advanced.
// If _Fcin.fclast!=0, performs an sfreserve() for the next buffer.
// If a notify function has been set, it is called.
// If last is non-zero, and the stream is a file, 0 is returned when
// the previous character is a 0 byte.
//
int fcfill(void) {
    int n;
    Sfio_t *f;
    unsigned char *last = _Fcin.fclast, *ptr = _Fcin.fcptr;
    f = fcfile();
    if (!f) {
        // See whether pointer has passed null byte.
        if (ptr > _Fcin.fcbuff && *--ptr == 0) {
            _Fcin.fcptr = ptr;
        } else {
            _Fcin.fcoff = 0;
        }
        return 0;
    }
    if (last) {
        if (ptr < last && ptr > _Fcin.fcbuff && *(ptr - 1) == 0) return 0;
        if (_Fcin.fcchar) *last = _Fcin.fcchar;
        if (ptr > last) _Fcin.fcptr = ptr = last;
    }
    n = ptr - _Fcin.fcbuff;
    if (n && _Fcin.fcfun) (*_Fcin.fcfun)(f, (const char *)_Fcin.fcbuff, n, _Fcin.context);
    sfread(f, (char *)_Fcin.fcbuff, n);
    _Fcin.fcoff += n;
    _Fcin._fcfile = NULL;
    if (!last) {
        return 0;
    } else if (fcfopen(f) < 0) {
        return EOF;
    }
    return *_Fcin.fcptr++;
}

//
// Synchronize and close the current stream.
//
int fcclose(void) {
    unsigned char *ptr;

    if (_Fcin.fclast == 0) return 0;
    if ((ptr = _Fcin.fcptr) > _Fcin.fcbuff && *(ptr - 1) == 0) _Fcin.fcptr--;
    if (_Fcin.fcchar) *_Fcin.fclast = _Fcin.fcchar;
    _Fcin.fclast = NULL;
    _Fcin.fcleft = 0;
    return fcfill();
}

//
// Set the notify function that is called for each fcfill().
//
void fcnotify(void (*fun)(Sfio_t *, const char *, int, void *), void *context) {
    _Fcin.fcfun = fun;
    _Fcin.context = context;
}

#undef fcsave
void fcsave(Fcin_t *fp) { *fp = _Fcin; }

#undef fcrestore
void fcrestore(Fcin_t *fp) { _Fcin = *fp; }

int _fcmbget(short *len) {
    int c;
    *len = mblen((char *)_Fcin.fcptr, MB_CUR_MAX);
    switch (*len) {
        case -1: {
            *len = 1;
        }
        // FALLTHRU
        case 0:
        case 1: {
            c = fcget();
            break;
        }
        default: { c = mb1char((char **)&_Fcin.fcptr); }
    }
    return c;
}
