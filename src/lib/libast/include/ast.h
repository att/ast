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
 * std + posix + ast
 */
#ifndef _AST_H
#define _AST_H 1

#ifndef _AST_STD_H
#include "ast_std.h"
#endif

#ifndef _SFIO_H
#include "sfio.h"
#endif

#ifndef ast
#define ast _ast_info
#endif

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

/*
 * exit() support -- this matches shell exit codes
 */

#define EXIT_BITS 8 /* # exit status bits	*/

#define EXIT_USAGE 2      /* usage exit code	*/
#define EXIT_QUIT 255     /* parent should quit	*/
#define EXIT_NOTFOUND 127 /* command not found	*/
#define EXIT_NOEXEC 126   /* other exec error	*/

#define EXIT_CODE(x) ((x) & ((1 << EXIT_BITS) - 1))
#define EXIT_CORE(x) (EXIT_CODE(x) | (1 << EXIT_BITS) | (1 << (EXIT_BITS - 1)))
#define EXIT_TERM(x) (EXIT_CODE(x) | (1 << EXIT_BITS))

/*
 * NOTE: for compatibility the following work for EXIT_BITS={7,8}
 */

#define EXIT_STATUS(x) (((x) & ((1 << (EXIT_BITS - 2)) - 1)) ? (x) : EXIT_CODE((x) >> EXIT_BITS))

#define EXITED_CORE(x)                                            \
    (((x) & ((1 << EXIT_BITS) | (1 << (EXIT_BITS - 1)))) ==       \
         ((1 << EXIT_BITS) | (1 << (EXIT_BITS - 1))) ||           \
     ((x) & ((1 << (EXIT_BITS - 1)) | (1 << (EXIT_BITS - 2)))) == \
         ((1 << (EXIT_BITS - 1)) | (1 << (EXIT_BITS - 2))))
#define EXITED_TERM(x) ((x) & ((1 << EXIT_BITS) | (1 << (EXIT_BITS - 1))))

/*
 * astconflist() flags
 */

#define ASTCONF_parse 0x0001
#define ASTCONF_write 0x0002
#define ASTCONF_read 0x0004
#define ASTCONF_lower 0x0008
#define ASTCONF_base 0x0010
#define ASTCONF_defined 0x0020
#define ASTCONF_quote 0x0040
#define ASTCONF_table 0x0080
#define ASTCONF_matchcall 0x0100
#define ASTCONF_matchname 0x0200
#define ASTCONF_matchstandard 0x0400
#define ASTCONF_error 0x0800
#define ASTCONF_system 0x1000
#define ASTCONF_AST 0x2000

/*
 * pathcanon() flags and info
 */

#define PATH_PHYSICAL 0x001
#define PATH_DOTDOT 0x002
#define PATH_EXISTS 0x004
#define PATH_DEV 0x008
#define PATH_ABSOLUTE 0x010 /* NOTE: shared with pathaccess() below */
#define PATH_CANON 0x020
#define PATH_DROP_HEAD_SLASH2 0x040
#define PATH_DROP_TAIL_SLASH 0x080
#define PATH_EXCEPT_LAST 0x100
#define PATH_VERIFIED(n) (((n)&0xfffff) << 12)
#define PATH_GET_VERIFIED(n) (((n) >> 12) & 0xfffff)

/*
 * pathaccess() flags
 */

#define PATH_READ 0x04
#define PATH_WRITE 0x02
#define PATH_EXECUTE 0x01
#define PATH_REGULAR 0x08
/* NOTE: PATH_ABSOLUTE shared with pathcanon() above */

/*
 * touch() flags
 */

#define PATH_TOUCH_CREATE 01
#define PATH_TOUCH_VERBATIM 02

/*
 * strgrpmatch() flags
 */

#define STR_MAXIMAL 0x01 /* maximal match		*/
#define STR_LEFT 0x02    /* implicit left anchor		*/
#define STR_RIGHT 0x04   /* implicit right anchor	*/
#define STR_ICASE 0x08   /* ignore case			*/
#define STR_GROUP 0x10   /* (|&) inside [@|&](...) only	*/
#define STR_INT 0x20     /* deprecated int* match array	*/

/*
 * fmtquote() flags
 */

#define FMT_ALWAYS 0x01  /* always quote			*/
#define FMT_ESCAPED 0x02 /* already escaped		*/
#define FMT_SHELL 0x04   /* escape $ ` too		*/
#define FMT_WIDE 0x08    /* don't escape 8 bit chars	*/
#define FMT_PARAM 0x10   /* disable FMT_SHELL ${$( quote	*/

/*
 * chrexp() flags
 */

#define FMT_EXP_CHAR 0x020 /* expand single byte chars	*/
#define FMT_EXP_LINE 0x040 /* expand \n and \r		*/
#define FMT_EXP_WIDE 0x080 /* expand \u \U \x wide chars	*/
#define FMT_EXP_NOCR 0x100 /* skip \r			*/
#define FMT_EXP_NONL 0x200 /* skip \n			*/

/*
 * multibyte macros
 */
#define mbwide() (MB_CUR_MAX > 1)
#define mbwidth(w) (ast.mb_width ? (*ast.mb_width)(w) : 1)
#define mbxfrm(t, f, n) strxfrm((char *)(t), (char *)(f), n)
#define mbalpha(w) (ast.mb_alpha ? (*ast.mb_alpha)(w) : isalpha((w)&0xff))

#define mbsrtowcs(w, s, n, q) (*ast._ast_mbsrtowcs)((s), (w), (n), (mbstate_t *)(q))
#define wcsrtombs(s, w, n, q) (*ast._ast_wcsrtombs)((s), (w), (n), (mbstate_t *)(q))

/* the converse does not always hold! */
#define utf32invalid(u)                                              \
    ((u) > 0x0010FFFF || ((u) >= 0x0000D800 && (u) <= 0x0000DFFF) || \
     ((u) >= 0xFFFE && (u) <= 0xFFFF))

#define UTF8_LEN_MAX 6 /* UTF-8 only uses 5 */

/*
 * common macros
 */

#define elementsof(x) (sizeof(x) / sizeof(x[0]))
#define getconf(x) strtol(astconf((x), NULL, NULL), NULL, 0)
#define roundof(x, y) (((x) + ((y)-1)) & ~((y)-1))

typedef int (*Ast_confdisc_f)(const char *, const char *, const char *);
typedef int (*Strcmp_context_f)(const char *, const char *, void *);
typedef int (*Strcmp_f)(const char *, const char *);

#include "ast_errorf.h"

#ifndef _VMALLOC_H
struct Vmdisc_s;
#endif

extern char *astgetconf(const char *, const char *, const char *, int, Error_f);
extern char *astconf(const char *, const char *, const char *);
extern Ast_confdisc_f astconfdisc(Ast_confdisc_f);
extern void astconflist(Sfio_t *, const char *, int, const char *);
extern void astwinsize(int, int *, int *);

extern ssize_t base64encode(const void *, size_t, void **, void *, size_t, void **);
extern ssize_t base64decode(const void *, size_t, void **, void *, size_t, void **);
extern int chresc(const char *, char **);
extern int chrexp(const char *, char **, int *, int);
extern char *conformance(const char *, size_t);
extern ssize_t debug_printf(int, const char *, ...);
extern void dump_backtrace(int max_frames, int skip_levels);
extern char *fmtbase(intmax_t, int, int);
#define fmtbasell(a, b, c) fmtbase(a, b, c) /* until 2014-01-01 */
extern char *fmtbuf(size_t);
extern char *fmtelapsed(unsigned long, int);
extern char *fmterror(int);
extern char *fmtesc(const char *);
extern char *fmtesq(const char *, const char *);
extern char *fmtident(const char *);
extern char *fmtfmt(const char *);
extern char *fmtgid(int);
extern char *fmtint(intmax_t, int);
extern char *fmtmatch(const char *);
extern char *fmtmode(int, int);
extern char *fmtnesq(const char *, const char *, size_t);
extern char *fmtperm(int);
extern char *fmtquote(const char *, const char *, const char *, size_t, int);
extern char *fmtre(const char *);
extern char *fmtscale(Sfulong_t, int);
extern char *fmttime(const char *, time_t);
extern char *fmtuid(int);
extern void *memdup(const void *, size_t);
extern char *pathaccess(char *, const char *, const char *, const char *, int);
extern char *pathaccess_20100601(const char *, const char *, const char *, int, char *, size_t);
extern char *pathbin(void);
extern char *pathcanon(char *, size_t, int);
extern char *pathcat(char *, const char *, int, const char *, const char *);
extern char *pathcat_20100601(const char *, int, const char *, const char *, char *, size_t);
extern int pathexists(char *, int);
extern int pathgetlink(const char *, char *, int);
extern int pathinclude(const char *);
extern char *pathkey(char *, char *, const char *, const char *, const char *);
extern char *pathkey_20100601(const char *, const char *, const char *, char *, size_t, char *,
                              size_t);
extern size_t pathnative(const char *, char *, size_t);
extern char *pathpath(const char *, const char *, int, char *, size_t);
extern size_t pathposix(const char *, char *, size_t);
extern size_t pathprog(const char *, char *, size_t);
extern char *pathrepl(char *, const char *, const char *);
extern char *pathrepl_20100601(char *, size_t, const char *, const char *);
extern char *pathshell(void);
extern char *pathtemp(char *, size_t, const char *, const char *, int *);
extern char *pathtmp(char *, const char *, const char *, int *);
extern char *sh_setenviron(const char *);
extern int stracmp(const char *, const char *);
extern char *strcopy(char *, const char *);
extern unsigned long strelapsed(const char *, char **, int);
extern int stresc(char *);
extern int strexp(char *, int);
extern int strgrpmatch(const char *, const char *, int *, int, int);
extern int strgrpmatch_20120528(const char *, const char *, ssize_t *, int, int);
extern int strngrpmatch(const char *, size_t, const char *, ssize_t *, int, int);
extern int strmatch(const char *, const char *);
extern int stropt(const char *, const void *, int, int (*)(void *, const void *, int, const char *),
                  void *);
extern int strperm(const char *, char **, int);
extern void strsort(char **, int, Strcmp_f);
extern void strsort_r(char **, size_t, Strcmp_context_f, void *);
extern char *strsubmatch(const char *, const char *, int);
extern long strton(const char *, char **, char *, int);
extern intmax_t strtonll(const char *, char **, char *, int);
extern int struniq(char **, int);

extern size_t utf32toutf8(char *, uint32_t);
extern size_t utf8toutf32(uint32_t *, const char *, size_t);
extern size_t utf8toutf32v(uint32_t *, const char *);
extern int utf8towc(wchar_t *, const char *, size_t);

extern ssize_t utf32stowcs(wchar_t *, uint32_t *, size_t);
extern ssize_t wcstoutf32s(uint32_t *, wchar_t *, size_t);

extern size_t ast_mbrchar(wchar_t *, const char *, size_t, Mbstate_t *);

/*
 * C library global data symbols not prototyped by <unistd.h>
 */
extern char **environ;

#include "ast_api.h"

#define mbinit(q) (void)memset(q, 0, sizeof(*q))
#define mberrno(q) ((q)->mb_errno)
#define mbchar(w, s, n, q) (((s) += (ast_mbrchar)((wchar_t *)(w), (char *)(s), (n), (q))), (*(w)))
#define mbconv(s, w, q) (*ast._ast_wcrtomb)((s), (w), (mbstate_t *)(q))

// This is the pre 2013-09-13 implementation of the `mbchar()` macro. We retain it, under a new
// name, solely to facilitate removing the `ASTAPI()` macro. This allows us to avoid changing every
// place in the ksh code that uses this legacy macro in a single change. With this in place we can
// slowly convert the legacy API uses to the new one above.
#define mb1char(p)                                                                   \
    (mbwide() ? ((ast.tmp_int = mbtowc(&ast.tmp_wchar, (char *)(p), MB_CUR_MAX)) > 0 \
                     ? ((p += ast.tmp_int), ast.tmp_wchar)                           \
                     : (p += 1, ast.tmp_int))                                        \
              : (*(unsigned char *)(p++)))

/* generic plugin version support */

extern unsigned long plugin_version(void);

/* This is a workaround for issue discussed in https://github.com/att/ast/pull/107 */
#include <wctype.h>
#undef iswalpha
#define iswalpha(w) (_ast_info.mb_alpha ? (*_ast_info.mb_alpha)(w) : isalpha(w))

#define CC_bel 0007  // BEL character
#define CC_esc 0033  // ESC character
#define CC_sub 0032  // SUB character
#define CC_vt 0013   // VT character

// If running under the oclint tool try to suppress some platform specific warnings.
// For example, on macOS many commonly used functions are defined in terms of macros
// that contain expressions which always evalute true or false.
#if _OCLINT_

#undef strcat
#undef strcpy
#undef strncpy
#undef strlcat
#undef strlcpy
#undef memcpy
#undef memmove

#endif

#endif  // _AST_H
