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
/*
 * setlocale() intercept
 * maintains a bitmask of non-default categories
 * and a permanent locale namespace for pointer comparison
 * and persistent private data for locale related functions
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <langinfo.h>
#include <locale.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "ast.h"
#include "ast_iconv.h"
#include "sfio.h"

#undef mbsrtowcs
#undef wcsrtombs
#undef wcwidth
#undef wctomb
#ifdef mblen
#undef mblen
extern int mblen(const char *, size_t);
#endif

#undef mbtowc
#undef setlocale
#undef strcmp
#undef strcoll
#undef strxfrm
#undef valid

/*
 * LC_COLLATE and LC_CTYPE native support
 */

#if MB_LEN_MAX <= 1
#define mblen 0
#define mbtowc 0
#endif

/*
 * workaround the interesting sjis that translates unshifted 7 bit ascii!
 */

#define mb_state ((mbstate_t *)&ast.pad[sizeof(ast.pad) - sizeof(mbstate_t)])

static size_t sjis_mbrtowc(wchar_t *p, const char *s, size_t n, mbstate_t *q) {
    if (n && p && s && (*s == '\\' || *s == '~') &&
        !memcmp(q, &ast._ast_mbstate_init, sizeof(ast._ast_mbstate_init))) {
        *p = *s;
        return 1;
    }
    return mbrtowc(p, s, n, q);
}

static int sjis_mbtowc(wchar_t *p, const char *s, size_t n) {
    return (int)sjis_mbrtowc(p, s, n, mb_state);
}

/*
 * grab one mb char from s max n bytes to w
 * # chars read from s returned
 */

size_t ast_mbrchar(wchar_t *w, const char *s, size_t n, Mbstate_t *q) {
    size_t m;

    m = (*ast._ast_mbrtowc)(w, s, n, (mbstate_t *)q);
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

static_fn size_t utf8towc_wrapper(wchar_t *wc, const char *s, size_t n, mbstate_t *state) {
    UNUSED(state);
    return utf8towc(wc, s, n);
}

static_fn size_t utf32toutf8_wrapper(char *s, wchar_t wc, mbstate_t *state) {
    UNUSED(state);
    return utf32toutf8(s, wc);
}

static bool is_us_ascii_codeset() {
    char *codeset = nl_langinfo(CODESET);
    return strcasecmp(codeset, "US-ASCII") || strcasecmp(codeset, "USASCII");
}

/*
 * called when LC_CTYPE initialized or changes
 */

static int set_ctype() {
    ast.locale.serial++;
    ast.collate = strcoll;
    ast.mb_xfrm = strxfrm;
    ast.mb_sync = 0;
    ast.mb_alpha = (Isw_f)iswalpha;
    if (ast.mb_uc2wc != (void *)(-1)) {
        if (ast.mb_uc2wc) iconv_close((iconv_t)ast.mb_uc2wc);
        ast.mb_uc2wc = (void *)(-1);
    }
    if (ast.mb_wc2uc != (void *)(-1)) {
        if (ast.mb_wc2uc) iconv_close((iconv_t)ast.mb_wc2uc);
        ast.mb_wc2uc = (void *)(-1);
    }

    const char *codeset = nl_langinfo(CODESET);
    ast.locale.is_utf8 = strcasecmp(codeset, "utf-8") == 0 || strcasecmp(codeset, "utf8") == 0;
    ast.mb_width = wcwidth;
    ast.mb_cur_max = MB_CUR_MAX;
    ast.mb_len = mblen;
    ast.mb_towc = mbtowc;
    ast.mb_conv = wctomb;
    ast._ast_mbrlen = mbrlen;
    ast._ast_mbrtowc = mbrtowc;
    ast._ast_mbsrtowcs = mbsrtowcs;
    ast._ast_wcrtomb = wcrtomb;
    ast._ast_wcsrtombs = wcsrtombs;
#ifdef mb_state
    {
        /*
            * check for sjis that translates unshifted 7 bit ascii!
            */

        char *s;
        wchar_t w;
        Mbstate_t q;
        char c;

        mbinit(&q);
        c = '\\';
        s = &c;
        if (mbchar(&w, s, 1, &q) != c) {
            ast.mb_towc = sjis_mbtowc;
            ast._ast_mbrtowc = sjis_mbrtowc;
        }
    }
#endif
        ast.mb_conv = wctomb;

    ast.byte_max = ast.mb_cur_max == 1 && !is_us_ascii_codeset() ? 0xff : 0x7f;
    return 0;
}

//
// setlocale() intercept.
//
char *_ast_setlocale(int category, const char *locale) {
    // Ensure the system's locale subsystem is initialized based on the locale env vars.
    char *rv = setlocale(category, locale);
    if (locale) set_ctype();
    return rv;
}
