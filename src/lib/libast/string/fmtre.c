/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2011 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Research
 *
 * Returns:
 *   pointer to a short lived ERE pattern for a valid strmatch() pattern
 *   NULL for invalid strmatch() pattern or one that cannot be convert to a ERE
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <string.h>

#include "ast.h"

typedef struct Stack_s {
    char *beg;
    short len;
    short min;
} Stack_t;

char *fmtre(const char *as) {
    char *s = (char *)as;
    int c;
    char *t;
    Stack_t *p;
    char *x;
    int n;
    bool end = true;
    char *buf;
    Stack_t stack[32];

    int slen = strlen(s);
    c = 2 * slen + 1;
    t = buf = fmtbuf(c);
    if (slen == 0) {
        *buf = 0;
        return buf;
    }
    p = stack;
    if (s[0] != '*') {
        *t++ = '^';
    } else if (slen >= 2 && s[1] == '(') {
        *t++ = '^';
    } else if (slen >= 3 && s[1] == '-' && s[2] == '(') {
        *t++ = '^';
    } else {
        s++;
    }

    while (*s) {
        c = *s++;
        if (c == '*' && !*s) {
            end = false;
            break;
        }

        switch (c) {
            case '\\': {
                c = *s++;
                if (!c || c == '{' || c == '}') return NULL;
                *t++ = '\\';
                *t++ = c;
                if (c == '(' && *s == '|') {
                    *t++ = *s++;
                    if (!*s || *s == ')') return NULL;
                    *t++ = c;
                }
                break;
            }
            case '[': {
                *t++ = c;
                n = 0;
                c = *s++;
                if (c == '!') {
                    *t++ = '^';
                    c = *s++;
                } else if (c == '^') {
                    c = *s++;
                    if (c == ']') {
                        *(t - 1) = '\\';
                        *t++ = '^';
                        continue;
                    }
                    n = '^';
                }
                for (;;) {
                    *t++ = c;
                    if (!c) return NULL;
                    c = *s++;
                    if (c == ']') {
                        if (n) *t++ = n;
                        *t++ = c;
                        break;
                    }
                }
                break;
            }
            case '{': {
                for (x = s; *x && *x != '}'; x++) {
                    ;
                }
                if (*x++ && (*x == '(' || (*x == '-' && *(x + 1) == '('))) {
                    if (p >= &stack[elementsof(stack)]) return NULL;
                    p->beg = s - 1;
                    s = x;
                    p->len = s - p->beg;
                    p->min = *s == '-';
                    if (p->min) s++;
                    p++;
                    *t++ = *s++;
                } else {
                    *t++ = c;
                }
                break;
            }
            case '*': {
                // FALLTHRU
            }
            case '?': {
                // FALLTHRU
            }
            case '+': {
                // FALLTHRU
            }
            case '@': {
                // FALLTHRU
            }
            case '!': {
                // FALLTHRU
            }
            case '~': {
                if (*s == '(' || (c != '~' && *s == '-' && *(s + 1) == '(')) {
                    if (p >= &stack[elementsof(stack)]) return NULL;
                    p->beg = s - 1;
                    if (c == '~') {
                        if (*(s + 1) == 'E' && *(s + 2) == ')') {
                            for (s += 3; (*t = *s); t++, s++) {
                                ;  // empty loop
                            }
                            continue;
                        }
                        p->len = 0;
                        p->min = 0;
                        *t++ = *s++;
                        *t++ = '?';
                    } else {
                        p->len = c != '@';
                        p->min = (*s == '-');
                        if (p->min) s++;
                        *t++ = *s++;
                    }
                    p++;
                } else {
                    switch (c) {
                        case '*':
                            *t++ = '.';
                            break;
                        case '?':
                            c = '.';
                            break;
                        case '+':
                            *t++ = '\\';
                            break;
                        case '!':
                            *t++ = '\\';
                            break;
                        default:
                            break;
                    }
                    *t++ = c;
                }
                break;
            }
            case '(': {
                if (p >= &stack[elementsof(stack)]) return NULL;
                p->beg = s - 1;
                p->len = 0;
                p->min = 0;
                p++;
                *t++ = c;
                break;
            }
            case ')': {
                if (p == stack) return NULL;
                *t++ = c;
                p--;
                for (c = 0; c < p->len; c++) *t++ = p->beg[c];
                if (p->min) *t++ = '?';
                break;
            }
            case '^': {
                *t++ = '\\';
                *t++ = c;
                break;
            }
            case '.': {
                *t++ = '\\';
                *t++ = c;
                break;
            }
            case '$': {
                *t++ = '\\';
                *t++ = c;
                break;
            }
            case '|': {
                if (t == buf || *(t - 1) == '(') return NULL;
                if (!*s || *s == ')') return NULL;
                *t++ = c;
                break;
            }
            default: {
                *t++ = c;
                break;
            }
        }
    }

    if (p != stack) return NULL;
    if (end) *t++ = '$';
    *t = 0;
    return buf;
}
