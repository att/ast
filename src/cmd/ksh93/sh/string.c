/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2013 AT&T Intellectual Property          *
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
// String processing routines for Korn shell.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#if _lib_iswprint
#include <wctype.h>
#endif

#include "defs.h"

#include "ast.h"
#include "ast_api.h"
#include "error.h"
#include "fault.h"
#include "lexstates.h"
#include "sfio.h"
#include "shtable.h"
#include "stak.h"
#include "stk.h"

#if !_lib_iswprint
// On some platforms iswprint() may be macro so make sure we don't get a redefinition warning.
#undef iswprint
#define iswprint(c) (((c) & ~0377) || isprint(c))
#endif

#define sep(c) ((c) == '-' || (c) == '_')

//
// Table lookup routine. The <table> is searched for string <sp> and corresponding value is
// returned. This is only used for small tables and is used to save non-sharable memory.
//
const Shtable_t *sh_locate(const char *sp, const Shtable_t *table, int size) {
    int first;
    const Shtable_t *tp;
    int c;
    static const Shtable_t empty = {0, 0};

    if (sp == 0 || (first = *sp) == 0) return (&empty);
    tp = table;
    while ((c = *tp->sh_name) && c <= first) {
        if (first == c && strcmp(sp, tp->sh_name) == 0) return (tp);
        tp = (Shtable_t *)((char *)tp + size);
    }
    return &empty;
}

//
// shtab_options lookup routine.
//
int sh_lookopt(const char *sp, int *invert) {
    int first;
    const Shtable_t *tp;
    int c;
    const char *s, *t, *sw, *tw;
    int amb;
    int hit;
    int inv;
    int no;

    if (sp == 0) return 0;
    if (*sp == 'n' && *(sp + 1) == 'o' && (*(sp + 2) != 't' || *(sp + 3) != 'i')) {
        sp += 2;
        if (sep(*sp)) sp++;
        *invert = !*invert;
    }
    if ((first = *sp) == 0) return 0;
    tp = shtab_options;
    amb = hit = 0;
    for (;;) {
        t = tp->sh_name;
        no = *t == 'n' && *(t + 1) == 'o' && *(t + 2) != 't';
        if (no) t += 2;
        if (!(c = *t)) break;
        if (first == c) {
            if (strcmp(sp, t) == 0) {
                *invert ^= no;
                return tp->sh_number;
            }
            s = sw = sp;
            tw = t;
            for (;;) {
                if (!*s || *s == '=') {
                    if (*s == '=' && !strtol(s + 1, NULL, 0)) no = !no;
                    if (!*t) {
                        *invert ^= no;
                        return tp->sh_number;
                    }
                    if (hit || amb) {
                        hit = 0;
                        amb = 1;
                    } else {
                        hit = tp->sh_number;
                        inv = no;
                    }
                    break;
                } else if (!*t) {
                    break;
                } else if (sep(*s)) {
                    sw = ++s;
                } else if (sep(*t)) {
                    tw = ++t;
                } else if (*s == *t) {
                    s++;
                    t++;
                } else if (s == sw && t == tw) {
                    break;
                } else {
                    if (t != tw) {
                        while (*t && !sep(*t)) t++;
                        if (!*t) break;
                        tw = ++t;
                    }
                    while (s > sw && *s != *t) s--;
                }
            }
        }
        tp = (Shtable_t *)((char *)tp + sizeof(*shtab_options));
    }
    if (hit) *invert ^= inv;
    return hit;
}

//
// Look for the substring <oldsp> in <string> and replace with <newsp>.
// The new string is put on top of the stack.
//
// assume string!=NULL && oldsp!=NULL && newsp!=NULL;
// return x satisfying x==NULL ||
//     strlen(x)==(strlen(in string)+strlen(in newsp)-strlen(in oldsp));
//
char *sh_substitute(Shell_t *shp, const char *string, const char *oldsp, char *newsp) {
    const char *sp = string;
    const char *cp;
    const char *savesp = 0;

    stkseek(shp->stk, 0);
    if (*sp == 0) return NULL;
    if (*(cp = oldsp) == 0) goto found;
    mbinit();
    do {
        // Skip to first character which matches start of oldsp.
        while (*sp && (savesp == sp || *sp != *cp)) {
            // Skip a whole character at a time.
            int c = mbsize(sp);
            if (c < 0) sp++;
            while (c-- > 0) sfputc(shp->stk, *sp++);
        }
        if (*sp == 0) return NULL;
        savesp = sp;
        for (; *cp; cp++) {
            if (*cp != *sp++) break;
        }
        if (*cp == 0) {  // match found
            goto found;
        }
        sp = savesp;
        cp = oldsp;
    } while (*sp);
    return NULL;

found:
    sfputr(shp->stk, newsp, -1);  // copy new
    sfputr(shp->stk, sp, -1);     // copy rest of string
    return stkfreeze(shp->stk, 1);
}

//
// TRIM(sp)
// Remove escape characters from characters in <sp> and eliminate quoted nulls.
//
// assume sp!=NULL;
// promise strlen(in sp) <= in strlen(sp);
//
void sh_trim(char *sp) {
    char *dp;
    int c;
    if (sp) {
        dp = sp;
        while ((c = *sp)) {
            int len;
            if (mbwide() && (len = mbsize(sp)) > 1) {
                memmove(dp, sp, len);
                dp += len;
                sp += len;
                continue;
            }
            sp++;
            if (c == '\\') c = *sp++;
            if (c) *dp++ = c;
        }
        *dp = 0;
    }
}

//
// copy <str1> to <str2> changing upper case to lower case
// <str2> must be big enough to hold <str1>
// <str1> and <str2> may point to the same place.
//
// assume str1!=0 && str2!=0
// return x satisfying strlen(in str1)==strlen(in str2);
//
void sh_utol(char const *str1, char *str2) {
    int c;

    for (; (c = *((unsigned char *)str1)); str1++, str2++) {
        if (isupper(c)) {
            *str2 = tolower(c);
        } else {
            *str2 = c;
        }
    }
    *str2 = 0;
}

//
// Format string as a csv field.
//
static_fn char *sh_fmtcsv(const char *string) {
    const char *cp = string;
    int c;
    int offset;

    if (!cp) return NULL;
    offset = staktell();
    while ((c = mbchar(cp)), isaname(c)) {
        ;  // empty loop
    }
    if (c == 0) return ((char *)string);
    stakputc('"');
    stakwrite(string, cp - string);
    if (c == '"') stakputc('"');
    string = cp;
    while ((c = mbchar(cp))) {
        if (c == '"') {
            stakwrite(string, cp - string);
            string = cp;
            stakputc('"');
        }
    }
    if (--cp > string) stakwrite(string, cp - string);
    stakputc('"');
    stakputc(0);
    return stakptr(offset);
}

//
// Print <str> quoting chars so that it can be read by the shell.
// Puts null terminated result on stack, but doesn't freeze it.
//
char *sh_fmtstr(const char *string, int quote) {
    const char *cp = string, *op;
    int c, state, type = quote;
    int offset;
    bool lc_unicodeliterals;

    if (!cp) return NULL;
    offset = staktell();
    mbinit();
    state = ((c = mbchar(cp)) == 0);
    lc_unicodeliterals = quote == 'u' ? 1 : 0;
    if (quote == '"') goto skip;
    quote = '\'';
    if (isaletter(c) && (!lc_unicodeliterals || c <= 0x7f)) {
        while ((c = mbchar(cp)), isaname(c) && (!lc_unicodeliterals || c <= 0x7f)) {
            ;  // empty loop
        }
        if (c == 0) return ((char *)string);
        if (c == '=') {
            if (*cp == 0) return ((char *)string);
            if (*cp == '=') cp++;
            c = cp - string;
            stakwrite(string, c);
            string = cp;
            c = mbchar(cp);
        }
    }
    if (c == 0 || c == '#' || c == '~' || (type == '[' && (c == '@' || c == '!'))) {
    skip:
        state = 1;
    }
    for (; c; c = mbchar(cp)) {
        if (c == quote || c >= 128 || c < 0 || !iswprint(c)) {
            state = 2;
        } else if (c == ']' || c == '=' ||
                   (c != ':' && c <= 0x7f && (c = sh_lexstates[ST_NORM][c]) && c != S_EPAT)) {
            state |= 1;
        }
    }
    if (state < 2) {
        if (state == 1) stakputc(quote);
        c = --cp - string;
        if (c) stakwrite(string, c);
        if (state == 1) stakputc(quote);
    } else {
        int lc_specifier = ast.locale.is_utf8 ? 'u' : 'w';
        bool widebyte;
        if (quote == '"') {
            stakputc('"');
        } else {
            stakwrite("$'", 2);
        }
        cp = string;
        mbinit();
        while (op = cp, c = mbchar(cp)) {
            state = 1;
            switch (c) {
                // Escape character
                case ('\033'): {
                    c = 'E';
                    break;
                }
                case '\n': {
                    c = 'n';
                    break;
                }
                case '\r': {
                    c = 'r';
                    break;
                }
                case '\t': {
                    c = 't';
                    break;
                }
                case '\f': {
                    c = 'f';
                    break;
                }
                case '\b': {
                    c = 'b';
                    break;
                }
                case '\a': {
                    c = 'a';
                    break;
                }
                case '\\': {
                    break;
                }
                case '"':
                case '\'': {
                    if (c == quote) break;
                }
                // FALLTHRU
                default: {
                    if (c < 0) {
                        c = *((unsigned char *)op);
                        cp = op + 1;
                        widebyte = 1;
                    } else {
                        widebyte = 0;
                    }
                    // If we convert the data to Unicode we want to produce portable ASCII-only
                    // output and therefore convert all non-ASCII (e.g. |c > 127|) characters to
                    // \u[] sequences.
                    //
                    // Note that this requires to pass all data through |wcstoutf32s()| to handle
                    // "extended" single-byte locales like "en_US.ISO8859-15" or "ru_RU.koi8r" that
                    // may produce "widebytes".
                    //
                    // If we do not convert to Unicode we only convert the non-printable characters
                    // to locale-specific \w[] sequences.
                    //
                    // Be *VERY* careful with the logic below - some single-byte locale
                    // implementations have wchar_t values > 127 but can return bytes, too.
                    if (!widebyte) {
                        if (lc_unicodeliterals) {
                            wchar_t wc = c;
                            uint32_t uc = 0;

                            // Posix doesn't play the iswrune() game for utf8 locales. Also, empty
                            // strings on error with no diagnostic just isn't right so source wchars
                            // that have no utf32 counterpart are emitted as \\w[HEX] => and that's
                            // a detectable error under lc_unicodeliterals.
                            if (lc_specifier == 'u') {
                                uc = c;
                            } else if (wcstoutf32s(&uc, &wc, 1) < 0) {
                                sfprintf(staksp, "\\\\w[%lx]", (unsigned long)c);
                                continue;
                            }

                            // We assume that all locales have ASCII as their base character set.
                            if (!iswprint(c) || uc > 127) {
                                sfprintf(staksp, "\\u[%lx]", (unsigned long)uc);
                                continue;
                            }
                        } else if (mbwide() && !iswprint(c)) {
                            sfprintf(staksp, "\\%c[%x]", lc_specifier, c);
                            continue;
                        }
                    }
                    if (widebyte || !iswprint(c)) {
                        sfprintf(staksp, "\\x%.2x", c);
                        continue;
                    }
                    state = 0;
                    break;
                }
            }
            if (state) {
                stakputc('\\');
                stakputc(c);
            } else {
                stakwrite(op, cp - op);
            }
        }
        stakputc(quote);
    }
    stakputc(0);
    return stakptr(offset);
}

char *sh_fmtq(const char *string) { return sh_fmtstr(string, '\''); }

char *sh_fmtj(const char *string) { return sh_fmtstr(string, '"'); }

//
// Print <str> quoting chars so that it can be read by the shell. Puts null terminated result on
// stack, but doesn't freeze it. Flags is a bitmask of SFFMT_* flags. Fold>0 prints raw newlines and
// inserts appropriately escaped newlines every (fold-x) chars.
//
char *sh_fmtqf(const char *string, int flags, int fold) {
    const char *cp = string;
    const char *bp;
    const char *vp;
    int c;
    int n;
    int q;
    int a;
    int single;
    int offset;

    if (flags & SFFMT_ALTER) return sh_fmtcsv(cp);
    if (--fold < 8) fold = 0;
    if (!cp || !*cp || !fold || (fold && strlen(string) < fold)) {
        return sh_fmtstr(cp, (flags & SFFMT_ZERO) ? 'U' : (flags & SFFMT_SIGN) ? 'u' : '\'');
    }
    offset = staktell();
    single = 3;
    c = mbchar(string);
    a = isaletter(c) ? '=' : 0;
    vp = cp + 1;
    do {
        q = 0;
        n = fold;
        bp = cp;
        while ((!n || n-- > 0) && (c = mbchar(cp))) {
            if (a && !isaname(c)) a = 0;
            if (c >= 0x200) continue;
            if (c == '\'' || !iswprint(c)) {
                q = single;
                break;
            }
            if (c == '\n') {
                q = 1;
            } else if (c == a) {
                stakwrite(bp, cp - bp);
                bp = cp;
                vp = cp + 1;
                a = 0;
            } else if ((c == '#' || c == '~') && cp == vp) {
                q = 1;
            } else if (c == ']') {
                q = 1;
            } else if (c != ':' && (c = sh_lexstates[ST_NORM][c]) && c != S_EPAT) {
                q = 1;
            }
        }
        if (q & 2) {
            stakputc('$');
            stakputc('\'');
            cp = bp;
            n = fold - 3;
            q = 1;
            while ((c = mbchar(cp))) {
                switch (c) {
                    case ('a' == 97 ? '\033' : 39): {
                            c = 'E';
                            break;
                        }
                    case '\n': {
                        q = 0;
                        n = fold - 1;
                        break;
                    }
                    case '\r': {
                        c = 'r';
                        break;
                    }
                    case '\t': {
                        c = 't';
                        break;
                    }
                    case '\f': {
                        c = 'f';
                        break;
                    }
                    case '\b': {
                        c = 'b';
                        break;
                    }
                    case '\a': {
                        c = 'a';
                        break;
                    }
                    case '\\': {
                        if (*cp == 'n') {
                            c = '\n';
                            q = 0;
                            n = fold - 1;
                        }
                        break;
                    }
                    case '\'': {
                        break;
                    }
                    default: {
                        if (!iswprint(c)) {
                            if ((n -= 4) <= 0) {
                                stakwrite("'\\\n$'", 5);
                                n = fold - 7;
                            }
                            sfprintf(staksp, "\\%03o", c);
                            continue;
                        }
                        q = 0;
                        break;
                    }
                }
                if ((n -= q + 1) <= 0) {
                    if (!q) {
                        stakputc('\'');
                        cp = bp;
                        break;
                    }
                    stakwrite("'\\\n$'", 5);
                    n = fold - 5;
                }
                if (q) {
                    stakputc('\\');
                } else {
                    q = 1;
                }
                stakputc(c);
                bp = cp;
            }
            if (!c) stakputc('\'');
        } else if (q & 1) {
            stakputc('\'');
            cp = bp;
            n = fold ? (fold - 2) : 0;
            while ((c = mbchar(cp))) {
                if (c == '\n') {
                    n = fold - 1;
                } else if (n && --n <= 0) {
                    n = fold - 2;
                    stakwrite(bp, --cp - bp);
                    bp = cp;
                    stakwrite("'\\\n'", 4);
                } else if (n == 1 && *cp == '\'') {
                    n = fold - 5;
                    stakwrite(bp, --cp - bp);
                    bp = cp;
                    stakwrite("'\\\n\\''", 6);
                } else if (c == '\'') {
                    stakwrite(bp, cp - bp - 1);
                    bp = cp;
                    if (n && (n -= 4) <= 0) {
                        n = fold - 5;
                        stakwrite("'\\\n\\''", 6);
                    } else {
                        stakwrite("'\\''", 4);
                    }
                }
            }
            stakwrite(bp, cp - bp - 1);
            stakputc('\'');
        } else if ((n = fold)) {
            cp = bp;
            while ((c = mbchar(cp))) {
                if (--n <= 0) {
                    n = fold;
                    stakwrite(bp, --cp - bp);
                    bp = cp;
                    stakwrite("\\\n", 2);
                }
            }
            stakwrite(bp, cp - bp - 1);
        } else {
            stakwrite(bp, cp - bp);
        }
        if (c) {
            stakputc('\\');
            stakputc('\n');
        }
    } while (c);
    stakputc(0);
    return stakptr(offset);
}

int sh_strchr(const char *string, const char *dp, size_t size) {
    wchar_t c, d;
    const char *cp = string;

    // This used to use the obsolete `mbnchar()` macro. Then and now the code does not correctly
    // handle a conversion error. In the old `mbnchar()` using code it would decrement the pointer
    // by one. Which, at least in the context of this function was pointless and probably wrong
    // regardless.
    mbtowc(&d, dp, size);
    mbinit();
    while ((c = mbchar(cp))) {
        if (c == d) return cp - string;
    }
    if (d == 0) return cp - string;
    return -1;
}

const char *_sh_translate(const char *message) {
#if ERROR_VERSION >= 20000317L
    return (ERROR_translate(0, 0, e_dict, message));
#else
#if ERROR_VERSION >= 20000101L
    return (ERROR_translate(e_dict, message));
#else
    return (ERROR_translate(message, 1));
#endif
#endif
}

//
// Change '['identifier']' to identifier. Character before <str> must be a '['. returns pointer to
// last character.
//
char *sh_checkid(char *str, char *last) {
    unsigned char *cp = (unsigned char *)str;
    unsigned char *v = cp;
    int c;

    if (c = mbchar(cp), isaletter(c)) {
        while (c = mbchar(cp), isaname(c)) {
            ;  // empty loop
        }
    }
    if (c == ']' && (!last || ((char *)cp == last))) {
        // Eliminate [ and ]
        while (v < cp) {
            v[-1] = *v;
            v++;
        }
        if (last) {
            last -= 2;
        } else {
            while (*v) {
                v[-2] = *v;
                v++;
            }
            v[-2] = 0;
            last = (char *)v;
        }
    }
    return last;
}

#if _AST_VERSION <= 20000317L
char *fmtident(const char *string) { return (char *)string; }
#endif
