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
//
// setlocale() intercept.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <iconv.h>
#include <langinfo.h>
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wchar.h>

#include "ast.h"

#undef setlocale
#undef mbsrtowcs
#undef wcsrtombs
#undef strcoll
#undef strxfrm

//
// Grab one mb char from s max n bytes to w.
//
// Returns # chars read from s.
//
size_t ast_mbrchar(wchar_t *w, const char *s, size_t n, Mbstate_t *q) {
    size_t m;

    m = mbrtowc(w, s, n, (mbstate_t *)q);
    if (m == (size_t)(-1) || m == (size_t)(-2)) {
        q->mb_errno = m == (size_t)(-2) ? E2BIG : EILSEQ;
        m = 1;
        if (w) *w = n ? *(unsigned char *)s : 0;
    } else {
        q->mb_errno = 0;
    }
    return m;
}

typedef int (*Isw_f)(wchar_t);

static_fn bool is_us_ascii_codeset() {
    char *codeset = nl_langinfo(CODESET);
    return strcasecmp(codeset, "US-ASCII") || strcasecmp(codeset, "USASCII");
}

//
// Called when the locale subsystem is configured. This inits the `ast` structure members
// related to the locale subsystem.
//
static_fn void init_ast_struct() {
    ast.locale.serial++;

    if (ast.mb_uc2wc != (iconv_t)-1) {
        if (ast.mb_uc2wc) iconv_close(ast.mb_uc2wc);
        ast.mb_uc2wc = (iconv_t)-1;
    }

    if (ast.mb_wc2uc != (iconv_t)-1) {
        if (ast.mb_wc2uc) iconv_close(ast.mb_wc2uc);
        ast.mb_wc2uc = (iconv_t)-1;
    }

    const char *codeset = nl_langinfo(CODESET);
    ast.locale.is_utf8 = strcasecmp(codeset, "utf-8") == 0 || strcasecmp(codeset, "utf8") == 0;

    ast.byte_max = MB_CUR_MAX == 1 && !is_us_ascii_codeset() ? 0xff : 0x7f;
}

//
// Ensure the system's locale subsystem is initialized based on the locale env vars and initialize
// some internal state. Most notably increment a locale generation counter. This makes it cheap for
// any code that caches info based on the current locale to know when its cache is out of date.
//
char *ast_setlocale(int category, const char *locale) {
    char *rv = setlocale(category, locale);
    if (locale) init_ast_struct();
    return rv;
}
