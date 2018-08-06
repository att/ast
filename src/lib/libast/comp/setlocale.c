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
#include "lclib.h"
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

#ifndef AST_LC_CANONICAL
#define AST_LC_CANONICAL LC_abbreviated
#endif

#define native_locale(a, b, c) ((char *)0)

/*
 * LC_COLLATE and LC_CTYPE native support
 */

#if MB_LEN_MAX <= 1
#define mblen 0
#define mbtowc 0
#endif

/*
 * called when LC_COLLATE initialized or changes
 */

static int set_collate(Lc_category_t *cp) {
    if (locales[cp->internal]->flags & LC_default) {
        ast.collate = strcmp;
        ast.mb_xfrm = 0;
    } else {
        ast.collate = strcoll;
        ast.mb_xfrm = strxfrm;
    }
    return 0;
}

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

/*
 * called when LC_CTYPE initialized or changes
 */

static int set_ctype(Lc_category_t *cp) {
    UNUSED(cp);

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

    ast.byte_max = ast.mb_cur_max == 1 && strcmp(nl_langinfo(CODESET), "US-ASCII") ? 0xff : 0x7f;
    return 0;
}

/*
 * called when LC_NUMERIC initialized or changes
 */

static int set_numeric(Lc_category_t *cp) {
    int category = cp->internal;
    struct lconv *lp;
    Lc_numeric_t *dp;

    static Lc_numeric_t default_numeric = {'.', -1};
    static Lc_numeric_t eu_numeric = {',', '.'};
    static Lc_numeric_t us_numeric = {'.', ','};

    if (!LCINFO(category)->data) {
        if (locales[cp->internal]->flags & LC_local)
            dp = locales[cp->internal]->territory == &lc_territories[0]
                     ? &default_numeric
                     : *locales[cp->internal]->territory->code == 'e' ? &eu_numeric : &us_numeric;
        else if ((lp = localeconv()) && (dp = newof(0, Lc_numeric_t, 1, 0))) {
            dp->decimal =
                lp->decimal_point && *lp->decimal_point ? *(unsigned char *)lp->decimal_point : '.';
            dp->thousand =
                lp->thousands_sep && *lp->thousands_sep ? *(unsigned char *)lp->thousands_sep : -1;
        } else
            dp = &default_numeric;
        LCINFO(category)->data = (void *)dp;
    }
    return 0;
}

/*
 * this table is indexed by AST_LC_[A-Z]*
 */

Lc_category_t lc_categories[] = {
    {"LC_ALL", LC_ALL, AST_LC_ALL, NULL, NULL, 0},
    {"LC_COLLATE", LC_COLLATE, AST_LC_COLLATE, set_collate, NULL, 0},
    {"LC_CTYPE", LC_CTYPE, AST_LC_CTYPE, set_ctype, NULL, 0},
    {"LC_MESSAGES", LC_MESSAGES, AST_LC_MESSAGES, NULL, NULL, 0},
    {"LC_MONETARY", LC_MONETARY, AST_LC_MONETARY, NULL, NULL, 0},
    {"LC_NUMERIC", LC_NUMERIC, AST_LC_NUMERIC, set_numeric, NULL, 0},
    {"LC_TIME", LC_TIME, AST_LC_TIME, NULL, NULL, 0},
    {"LC_IDENTIFICATION", LC_IDENTIFICATION, AST_LC_IDENTIFICATION, NULL, NULL, 0},
    {"LC_ADDRESS", LC_ADDRESS, AST_LC_ADDRESS, NULL, NULL, 0},
    {"LC_NAME", LC_NAME, AST_LC_NAME, NULL, NULL, 0},
    {"LC_TELEPHONE", LC_TELEPHONE, AST_LC_TELEPHONE, NULL, NULL, 0},
    {"LC_XLITERATE", LC_XLITERATE, AST_LC_XLITERATE, NULL, NULL, 0},
    {"LC_MEASUREMENT", LC_MEASUREMENT, AST_LC_MEASUREMENT, NULL, NULL, 0},
    {"LC_PAPER", LC_PAPER, AST_LC_PAPER, NULL, NULL, 0},
};

static Lc_t *lang;
static Lc_t *lc_all;

/*
 * set a single AST_LC_* locale category
 * the caller must validate category
 * lc==0 restores the previous state
 */

static char *single(int category, Lc_t *lc, unsigned int flags) {
    const char *sys;
    int i;

    if (flags & (LC_setenv | LC_setlocale)) {
        if (!(ast.locale.set & AST_LC_internal)) lc_categories[category].prev = lc;
        if ((flags & LC_setenv) && lc_all && locales[category]) {
            if (lc_categories[category].setf)
                (*lc_categories[category].setf)(&lc_categories[category]);
            return (char *)locales[category]->name;
        }
    }
    if (!lc &&
        (!(lc_categories[category].flags & LC_setlocale) || !(lc = lc_categories[category].prev)) &&
        !(lc = lc_all) && !(lc = lc_categories[category].prev) && !(lc = lang))
        lc = lcmake(NULL);
    sys = 0;
    if (locales[category] != lc) {
        if (lc_categories[category].external == -lc_categories[category].internal) {
            for (i = 1; i < AST_LC_COUNT; i++)
                if (locales[i] == lc) {
                    sys = (char *)lc->name;
                    break;
                }
        } else if (lc->flags & LC_local)
            sys = setlocale(lc_categories[category].external, lcmake(NULL)->name);
        else if (!(sys = setlocale(lc_categories[category].external, lc->name)) &&
                 (streq(lc->name, lc->code) ||
                  !(sys = setlocale(lc_categories[category].external, lc->code))) &&
                 !streq(lc->code, lc->language->code))
            sys = setlocale(lc_categories[category].external, lc->language->code);
        if (sys)
            lc->flags |= LC_checked;
        else {
            /*
             * check for local override
             * currently this means an LC_MESSAGES dir exists
             */

            if (!(lc->flags & LC_checked)) {
#if 0
                // mcfind() has been removed as part of the transition from the AST locale subsystem
                // to the platform implementation.
                char path[PATH_MAX];

                if (mcfind(lc->code, NULL, LC_MESSAGES, 0, path, sizeof(path)))
                    lc->flags |= LC_local;
#endif
                lc->flags |= LC_checked;
            }
            if (!(lc->flags & LC_local)) return 0;
            if (lc_categories[category].external != -lc_categories[category].internal)
                setlocale(lc_categories[category].external, lcmake(NULL)->name);
        }
        locales[category] = lc;
        if (lc_categories[category].setf &&
            (*lc_categories[category].setf)(&lc_categories[category])) {
            locales[category] = lc_categories[category].prev;
            return 0;
        }
        if (lc->flags & LC_default) {
            ast.locale.set &= ~(1 << category);
        } else if (category == AST_LC_MESSAGES && lc->name[0] == 'e' && lc->name[1] == 'n' &&
                   (lc->name[2] == 0 || (lc->name[2] == '_' && lc->name[3] == 'U'))) {
            ast.locale.set &= ~(1 << category);
        } else {
            ast.locale.set |= (1 << category);
        }

    } else if (lc_categories[category].flags ^ flags) {
        lc_categories[category].flags &= ~(LC_setenv | LC_setlocale);
        lc_categories[category].flags |= flags;
    } else {
        if (lc_categories[category].setf) (*lc_categories[category].setf)(&lc_categories[category]);
        return (char *)lc->name;
    }
    return (char *)lc->name;
}

/*
 * set composite AST_LC_ALL locale categories
 * return <0:composite-error 0:not-composite >0:composite-ok
 */

static int composite(const char *s, int initialize) {
    const char *t;
    int i;
    int j;
    int k;
    int n;
    int m;
    const char *w;
    Lc_t *p;
    int cat[AST_LC_COUNT];
    int stk[AST_LC_COUNT];
    char buf[PATH_MAX / 2];

    k = n = 0;
    while (s[0] == 'L' && s[1] == 'C' && s[2] == '_') {
        n++;
        j = 0;
        w = s;
        for (i = 1; i < AST_LC_COUNT; i++) {
            s = w;
            t = lc_categories[i].name;
            while (*t && *s++ == *t++)
                ;
            if (!*t && *s++ == '=') {
                cat[j++] = i;
                if (s[0] != 'L' || s[1] != 'C' || s[2] != '_') break;
                w = s;
                i = -1;
            }
        }
        for (s = w; *s && *s != '='; s++)
            ;
        if (!*s) {
            for (i = 0; i < k; i++) single(stk[i], NULL, 0);
            return -1;
        }
        w = ++s;
        for (;;) {
            if (!*s) {
                p = lcmake(w);
                break;
            } else if (*s++ == ';') {
                if ((m = s - w - 1) >= sizeof(buf)) m = sizeof(buf) - 1;
                memcpy(buf, w, m);
                buf[m] = 0;
                p = lcmake(buf);
                break;
            }
        }
        for (i = 0; i < j; i++)
            if (!initialize) {
                if (!single(cat[i], p, 0)) {
                    for (i = 0; i < k; i++) single(stk[i], NULL, 0);
                    return -1;
                }
                stk[k++] = cat[i];
            } else if (!lc_categories[cat[i]].prev && !(ast.locale.set & AST_LC_internal))
                lc_categories[cat[i]].prev = p;
    }
    while (s[0] == '/' && s[1] && n < (AST_LC_COUNT - 1)) {
        n++;
        for (w = ++s; *s && *s != '/'; s++)
            ;
        if (!*s)
            p = lcmake(w);
        else {
            if ((j = s - w - 1) >= sizeof(buf)) j = sizeof(buf) - 1;
            memcpy(buf, w, j);
            buf[j] = 0;
            p = lcmake(buf);
        }
        if (!initialize) {
            if (!single(n, p, 0)) {
                for (i = 1; i < n; i++) single(i, NULL, 0);
                return -1;
            }
        } else if (!lc_categories[n].prev && !(ast.locale.set & AST_LC_internal))
            lc_categories[n].prev = p;
    }
    return n;
}

/*
 * setlocale() intercept
 *
 * locale:
 *	0	query
 *	""	initialize from environment (if LC_ALL)
 *	""	AST_LC_setenv: value unset (defer to LANG)
 *	"*"	AST_LC_setenv: value set (defer to LC_ALL)
 *	*	set (override LC_ALL)
 */

char *_ast_setlocale(int category, const char *locale) {
    char *s;
    int i;
    int j;
    int k;
    int f;
    Lc_t *p;
    int cat[AST_LC_COUNT];

    // Ensure the system's locale subsystem is initialized based on the locale env vars.
    setlocale(LC_ALL, "");

    static Sfio_t *sp;
    static int initialized;
    static const char local[] = "local";

    if ((category = lcindex(category, 0)) < 0) return 0;
    if (!locale) {
        /*
         * return the current state
         */

    compose:
        if (category != AST_LC_ALL && category != AST_LC_LANG)
            return (char *)locales[category]->name;
        if (!sp && !(sp = sfstropen())) return 0;
        for (i = 1; i < AST_LC_COUNT; i++) cat[i] = -1;
        for (i = 1, k = 0; i < AST_LC_COUNT; i++)
            if (cat[i] < 0) {
                k++;
                cat[i] = i;
                for (j = i + 1; j < AST_LC_COUNT; j++)
                    if (locales[j] == locales[i]) cat[j] = i;
            }
        if (k == 1) return (char *)locales[1]->name;
        for (i = 1; i < AST_LC_COUNT; i++)
            if (cat[i] >= 0 && !(locales[i]->flags & LC_default)) {
                if (sfstrtell(sp)) sfprintf(sp, ";");
                for (j = i, k = cat[i]; j < AST_LC_COUNT; j++)
                    if (cat[j] == k) {
                        cat[j] = -1;
                        sfprintf(sp, "%s=", lc_categories[j].name);
                    }
                sfprintf(sp, "%s", locales[i]->name);
            }
        if (!sfstrtell(sp)) return (char *)locales[0]->name;
        return sfstruse(sp);
    }
    if (!ast.locale.serial++) {
        initialized = 0;
    }
    if (ast.locale.set & AST_LC_setenv) {
        f = LC_setenv;
        p = *locale ? lcmake(locale) : (Lc_t *)0;
    } else if (*locale) {
        f = LC_setlocale;
        p = lcmake(locale);
    } else if (category == AST_LC_ALL) {
        if (!initialized) {
            char *u;

            /*
             * initialize from the environment
             * precedence determined by X/Open
             */

            u = 0;
            if ((s = getenv("LANG")) && *s) {
                if (streq(s, local) && (u || (u = native_locale(locale, tmp, sizeof(tmp))))) s = u;
                lang = lcmake(s);
            } else
                lang = 0;
            if ((s = getenv("LC_ALL")) && *s) {
                if (streq(s, local) && (u || (u = native_locale(locale, tmp, sizeof(tmp))))) s = u;
                lc_all = lcmake(s);
            } else
                lc_all = 0;
            for (i = 1; i < AST_LC_COUNT; i++)
                if (lc_categories[i].flags & LC_setlocale)
                    /* explicitly set by setlocale() */;
                else if ((s = getenv(lc_categories[i].name)) && *s) {
                    if (streq(s, local) && (u || (u = native_locale(locale, tmp, sizeof(tmp)))))
                        s = u;
                    lc_categories[i].prev = lcmake(s);
                } else
                    lc_categories[i].prev = 0;
            for (i = 1; i < AST_LC_COUNT; i++)
                if (!single(i,
                            lc_all && !(lc_categories[i].flags & LC_setlocale)
                                ? lc_all
                                : lc_categories[i].prev,
                            0)) {
                    while (i--) single(i, NULL, 0);
                    return 0;
                }
            initialized = 1;
        }
        goto compose;
    } else if (category == AST_LC_LANG || !(p = lc_categories[category].prev)) {
        f = 0;
        p = lcmake("C");
    } else
        f = 0;
    if (category == AST_LC_LANG) {
        if (lang != p) {
            lang = p;
            if (!lc_all)
                for (i = 1; i < AST_LC_COUNT; i++)
                    if (!single(i, lc_categories[i].prev, 0)) {
                        while (i--) single(i, NULL, 0);
                        return 0;
                    }
        }
    } else if (category != AST_LC_ALL) {
        if (f || !lc_all) return single(category, p, f);
        if (p && !(ast.locale.set & AST_LC_internal)) lc_categories[category].prev = p;
        return (char *)locales[category]->name;
    } else if (composite(locale, 0) < 0)
        return 0;
    else if (lc_all != p) {
        lc_all = p;
        for (i = 1; i < AST_LC_COUNT; i++)
            if (!single(i,
                        lc_all && !(lc_categories[i].flags & LC_setlocale) ? lc_all
                                                                           : lc_categories[i].prev,
                        0)) {
                while (i--) single(i, NULL, 0);
                return 0;
            }
    }
    goto compose;
}
