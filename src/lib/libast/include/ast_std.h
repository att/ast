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
#define _BLD_sfio 1
#define _BLD_vmalloc 1
#endif

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

#undef getenv
#define getenv _ast_getenv

#undef setenviron
#define setenviron _ast_setenviron

extern char *getenv(const char *);

#undef localeconv
#define localeconv _ast_localeconv

#undef setlocale
#define setlocale _ast_setlocale

extern struct lconv *localeconv(void);
extern char *setenviron(const char *);
extern char *setlocale(int, const char *);
extern char *strerror(int);

#define AST_MESSAGE_SET 3 /* see <mc.h> mcindex()		*/

/*
 * maintain this order when adding categories
 */

#define AST_LC_ALL 0
#define AST_LC_COLLATE 1
#define AST_LC_CTYPE 2
#define AST_LC_MESSAGES 3
#define AST_LC_MONETARY 4
#define AST_LC_NUMERIC 5
#define AST_LC_TIME 6
#define AST_LC_IDENTIFICATION 7
#define AST_LC_ADDRESS 8
#define AST_LC_NAME 9
#define AST_LC_TELEPHONE 10
#define AST_LC_XLITERATE 11
#define AST_LC_MEASUREMENT 12
#define AST_LC_PAPER 13
#define AST_LC_COUNT 14
#define AST_LC_LANG 255

#define AST_LC_internal 1
#define AST_LC_native (1L << 23)
#define AST_LC_unicodeliterals (1L << 24)
#define AST_LC_utf8 (1L << 25)
#define AST_LC_test (1L << 26)
#define AST_LC_setenv (1L << 27)
#define AST_LC_find (1L << 28)
#define AST_LC_debug (1L << 29)
#define AST_LC_setlocale (1L << 30)
#define AST_LC_translate (1L << 31)

#ifndef LC_ALL
#define LC_ALL (-AST_LC_ALL)
#endif
#ifndef LC_COLLATE
#define LC_COLLATE (-AST_LC_COLLATE)
#endif
#ifndef LC_CTYPE
#define LC_CTYPE (-AST_LC_CTYPE)
#endif
#ifndef LC_MESSAGES
#define LC_MESSAGES (-AST_LC_MESSAGES)
#endif
#ifndef LC_MONETARY
#define LC_MONETARY (-AST_LC_MONETARY)
#endif
#ifndef LC_NUMERIC
#define LC_NUMERIC (-AST_LC_NUMERIC)
#endif
#ifndef LC_TIME
#define LC_TIME (-AST_LC_TIME)
#endif
#ifndef LC_ADDRESS
#define LC_ADDRESS (-AST_LC_ADDRESS)
#endif
#ifndef LC_IDENTIFICATION
#define LC_IDENTIFICATION (-AST_LC_IDENTIFICATION)
#endif
#ifndef LC_NAME
#define LC_NAME (-AST_LC_NAME)
#endif
#ifndef LC_TELEPHONE
#define LC_TELEPHONE (-AST_LC_TELEPHONE)
#endif
#ifndef LC_XLITERATE
#define LC_XLITERATE (-AST_LC_XLITERATE)
#endif
#ifndef LC_MEASUREMENT
#define LC_MEASUREMENT (-AST_LC_MEASUREMENT)
#endif
#ifndef LC_PAPER
#define LC_PAPER (-AST_LC_PAPER)
#endif
#ifndef LC_LANG
#define LC_LANG (-AST_LC_LANG)
#endif

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
    } locale;

    long tmp_long;
    size_t tmp_size;
    short tmp_short;
    char tmp_char;
    wchar_t tmp_wchar;

    int (*collate)(const char *, const char *);

    int tmp_int;
    void *tmp_pointer;

    int mb_cur_max;
    int (*mb_len)(const char *, size_t);
    int (*mb_towc)(wchar_t *, const char *, size_t);
    size_t (*mb_xfrm)(char *, const char *, size_t);
    int (*mb_width)(wchar_t);
    int (*mb_conv)(char *, wchar_t);

    uint32_t env_serial;
    uint32_t mb_sync;
    uint32_t version;

    int (*mb_alpha)(wchar_t);

    int pwd;
    int byte_max;

    void *mb_uc2wc;
    void *mb_wc2uc;

    Mbstate_t _ast_mbstate_init;

    size_t (*_ast_mbrlen)(const char *, size_t, mbstate_t *);
    size_t (*_ast_mbrtowc)(wchar_t *, const char *, size_t, mbstate_t *);
    size_t (*_ast_mbsrtowcs)(wchar_t *, const char **, size_t, mbstate_t *);
    size_t (*_ast_wcrtomb)(char *, wchar_t, mbstate_t *);
    size_t (*_ast_wcsrtombs)(char *, const wchar_t **, size_t, mbstate_t *);

    char pad[936 - sizeof(void *) - 2 * sizeof(int) - 8 * sizeof(void *) - sizeof(Mbstate_t)];

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

#endif  // _AST_STD_H
