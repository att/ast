/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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

#include <stdlib.h>

#include "ast_aso.h"
#include "ast_assert.h"

//
// Return small format buffer chunk of size n. Small buffers thread safe but short-lived. This must
// only be used by the code in the libast/string directory. And only for functions that format
// relatively small strings (e.g., less than a kilobyte). Such as might be done when converting a
// binary int to a string.
//
// This API is inherently risky since there is no formal acquire/release semantics. We simply assume
// we'll never be asked to provide more than a small number of psuedo-static buffers that are in use
// at any point in time. This allows uses like
//
//    printf("%s %s %s\n", fmtint(x), fmtint(y), fmtint(z));
//
// This allows users of this API to avoid having to resort to a single static buffer that then
// requires the callers of those functions to `strdup()` the buffer. Or the users of this API having
// to always return a `malloc()` buffer that their caller then has to explicitly free.
//
// See https://github.com/att/ast/issues/962
//

static char buf[16 * 1024];
static char *nxt = buf;

char *fmtbuf(size_t n) {
    char *cur;
    char *tst;

    // This is to ensure a silly abuse of this API doesn't occur. We expect there to be a small
    // number of "in flight" uses of the small buffers we parcel out.
    assert(n <= 1024);

    do {
        cur = nxt;
        if (n > (&buf[sizeof(buf)] - cur)) {
            tst = buf;
        } else {
            tst = cur;
        }
    } while (asocasptr(&nxt, cur, tst + n) != cur);
    return tst;
}
