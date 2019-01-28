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

#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"

/*
 * _sfopen() wrapper to allow user sfopen() intercept
 */

extern Sfio_t *_sfopen(Sfio_t *, const char *, const char *);
extern Sfio_t *_sfopenat(int, Sfio_t *, const char *, const char *);

Sfio_t *sfopen(Sfio_t *f, const char *file, const char *mode) { return _sfopen(f, file, mode); }

Sfio_t *sfopenat(int cwd, Sfio_t *f, const char *file, const char *mode) {
    return _sfopenat(cwd, f, file, mode);
}
