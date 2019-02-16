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
 * convert wide character string to native utf-32 string
 * Roland Mainz <roland.mainz@nrubsig.org>
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <iconv.h>
#include <langinfo.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wchar.h>

#include "ast.h"

ssize_t wcstoutf32s(uint32_t *utf32, wchar_t *wchar, size_t n) {
    size_t i;
    ssize_t res;
    Mbstate_t q;

    if (ast.locale.is_utf8) {
        char tmp[UTF8_LEN_MAX + 1];

        mbinit(&q);
        for (i = 0; i < n; i++) {
            if (mbconv(tmp, wchar[i], &q) == (size_t)-1) break;
            utf32[i] = wchar[i];
        }
        res = (ssize_t)i;
    } else {
        char *inbuf;
        char *inbuf_start;
        char *outbuf;
        char *outbuf_start;
        size_t inbytesleft;
        size_t outbytesleft;
        int oerrno;

        if (ast.mb_wc2uc == (iconv_t)-1) {
            ast.mb_wc2uc = iconv_open("UTF-8", nl_langinfo(CODESET));
            if (ast.mb_wc2uc == (iconv_t)-1) ast.mb_wc2uc = NULL;
        }
        if (!ast.mb_wc2uc) return -1;
        inbytesleft = n * MB_CUR_MAX;
        outbytesleft = n * sizeof(uint32_t);
        inbuf_start = malloc(inbytesleft + 2 + outbytesleft);
        if (!inbuf_start) return -1;
        outbuf_start = inbuf_start + inbytesleft + 2;
        if (mbwide()) {
            ssize_t len;

            mbinit(&q);
            for (inbuf = inbuf_start, i = 0; i < n; i++, inbuf += len) {
                if ((len = mbconv(inbuf, wchar[i], &q)) < 0) {
                    inbuf[i] = 0;
                    break;
                }
            }
        } else {
            /*
             * We need this because Linux's |wcrtomb()| doesn't
             * handle single-byte locales like ISO8859-15
             * correctly
             */

            for (inbuf = inbuf_start, i = 0; i < n; i++) *inbuf++ = wchar[i];
        }
        inbytesleft = inbuf - inbuf_start;
        inbuf = inbuf_start;
        outbuf = outbuf_start;
        (void)iconv(ast.mb_wc2uc, NULL, NULL, NULL, NULL);
        if ((res = iconv(ast.mb_wc2uc, &inbuf, &inbytesleft, &outbuf, &outbytesleft)) >= 0) {
            const char *s;

            for (s = outbuf_start, i = 0; i < n && s < (const char *)outbuf; i++) {
                s += utf8toutf32v(&utf32[i], s);
            }
            if (i < n) utf32[i] = 0;
        }
        oerrno = errno;
        free(inbuf_start);
        errno = oerrno;
    }
    return res;
}
