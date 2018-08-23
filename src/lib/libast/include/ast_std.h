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
 * Advanced Software Technology Library
 * AT&T Research
 *
 * a union of standard headers that works
 * with local extensions enabled
 * and local omission compensation
 */
#ifndef _AST_STD_H
#define _AST_STD_H 1

#define _AST_STD_I 1

#if _BLD_ast
#define _BLD_aso 1
#define _BLD_cdt 1
#define _BLD_vmalloc 1
#endif

#include <iconv.h>
#include <stdbool.h>

#include "ast_fcntl.h"
#include "ast_getopt.h" /* <stdlib.h> does this */
#include "ast_lib.h"
#include "ast_sys.h"

/*
 * <stdio.h> and <wchar.h> are entangled on some systems
 * and this make <stdio.h> pop up in weird places
 * this dance works around those places
 */

#if _AST_H
#include "sfio.h" /* moved from <ast.h> because mbstate_t entangled with <stdio.h> on some systems */
#endif
#include <wchar.h>

#undef setlocale
#define setlocale _ast_setlocale

extern char *setlocale(int, const char *);
extern char *strerror(int);

#define AST_MESSAGE_SET 3 /* see <mc.h> mcindex()		*/

#undef strcoll
#define strcoll _ast_info.collate

typedef struct Mbstate_s {
    mbstate_t mb_state;
    int mb_errno;
} Mbstate_t;

typedef struct {
    char *id;

    struct {
        uint32_t serial;
        uint32_t set;
        bool is_utf8;  // true if current locale uses UTF-8 for its encoding
    } locale;

    long tmp_long;
    size_t tmp_size;
    short tmp_short;
    char tmp_char;
    wchar_t tmp_wchar;

    int (*collate)(const char *, const char *);

    int tmp_int;
    void *tmp_pointer;

    int (*mb_towc)(wchar_t *, const char *, size_t);
    size_t (*mb_xfrm)(char *, const char *, size_t);
    int (*mb_width)(wchar_t);

    uint32_t env_serial;
    uint32_t mb_sync;
    uint32_t version;

    int (*mb_alpha)(wchar_t);

    int pwd;
    int byte_max;

    iconv_t mb_uc2wc;
    iconv_t mb_wc2uc;

    Mbstate_t _ast_mbstate_init;

    size_t (*_ast_mbrlen)(const char *, size_t, mbstate_t *);
    size_t (*_ast_mbrtowc)(wchar_t *, const char *, size_t, mbstate_t *);
    size_t (*_ast_mbsrtowcs)(wchar_t *, const char **, size_t, mbstate_t *);
    size_t (*_ast_wcrtomb)(char *, wchar_t, mbstate_t *);
    size_t (*_ast_wcsrtombs)(char *, const wchar_t **, size_t, mbstate_t *);
} _Ast_info_t;

extern _Ast_info_t _ast_info;

typedef int (*Qsortcmp_f)(const void *, const void *);
typedef int (*Qsortcmp_r_f)(const void *, const void *, void *);

#if !defined(remove)
extern int remove(const char *);
#endif

#if !defined(rename)
extern int rename(const char *, const char *);
#endif

/* and now introducing prototypes botched by the standard(s) */

/*
 * and finally, standard interfaces hijacked by ast
 * _AST_STD_I delays headers that require <ast_map.h>
 */

#undef _AST_STD_I

#if _AST_GETOPT_H < 0
#undef _AST_GETOPT_H
#include "ast_getopt.h"
#endif

#if _GETOPT_H < 0
#undef _GETOPT_H
#include <getopt.h>
#endif

#if _REGEX_H < 0
#undef _REGEX_H
#include "regex.h"
#endif

extern char *translate(const char *, const char *, const char *, const char *);

#endif  // _AST_STD_H
