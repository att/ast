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
 * posix regex compiler
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "ast.h"
#include "ast_regex.h"
#include "cdt.h"
#include "reglib.h"
#include "stk.h"

#define serialize re_serialize /* hp.ia64 <unistd.h>! */

#define C_ESC (-1)
#define C_MB (-2)

static Dt_t *lc_collate_data = NULL;
static unsigned char *lc_ctype_data = NULL;
static uint32_t lc_collate_serial = UINT_MAX;
static uint32_t lc_ctype_serial = UINT_MAX;

#if _AST_REGEX_DEBUG

#define DEBUG_TEST(f, y, n) ((debug & (debug_flag = (f))) ? (y) : (n))
#define DEBUG_CODE(f, y)                  \
    do                                    \
        if (debug & (debug_flag = (f))) { \
            y                             \
        }                                 \
    while (0)
#define DEBUG_INIT()                                                               \
    do {                                                                           \
        char *t;                                                                   \
        if (!debug) {                                                              \
            debug = 0x80000000;                                                    \
            if (t = getenv("_AST_regex_comp_debug")) debug |= strtoul(t, NULL, 0); \
        }                                                                          \
    } while (0)

static unsigned long debug;
static unsigned long debug_flag;

#else

#define DEBUG_INIT()
#define DEBUG_TEST(f, y, n) (n)
#define DEBUG_CODE(f, y) \
    do {                 \
        ;                \
    } while (0)

#endif

typedef struct Cchr_s {
    Dtlink_t lnk;
    unsigned char nam[2];
    Ckey_t key;
} Cchr_t;

#define eat(p)                             \
    do {                                   \
        if ((p)->token.push)               \
            (p)->token.push = 0;           \
        else                               \
            (p)->cursor += (p)->token.len; \
    } while (0)

/*
 * determine whether greedy matching will work, i.e. produce
 * the best match first.  such expressions are "easy", and
 * need no backtracking once a complete match is found.
 * if an expression has backreferences or alts it's hard
 * else if it has only one closure it's easy
 * else if all closures are simple (i.e. one-character) it's easy
 * else it's hard.
 */

typedef struct Stats_s {
    unsigned long l;  /* min length to left of x                */
    unsigned long k;  /* min length to left of y                */
    unsigned long m;  /* min length                             */
    unsigned long n;  /* max length                             */
    unsigned short a; /* number of alternations         */
    unsigned short b; /* number of backrefs                     */
    unsigned short c; /* number of closures                     */
    unsigned short e; /* $                                      */
    unsigned short i; /* number of negations                    */
    unsigned short p; /* number of named subexpressions */
    unsigned short s; /* number of simple closures              */
    unsigned short t; /* number of tries                        */
    unsigned short u; /* number of unnamed subexpressions       */
    Rex_t *x;         /* max length REX_STRING          */
    Rex_t *y;         /* max length REX_TRIE                    */
} Stats_t;

typedef struct Token_s {
    unsigned long min;
    unsigned long max;
    short lex;
    short len;
    short esc;
    short att;
    short push;
} Token_t;

typedef struct Cenv_s {
    int delimiter;          /* pattern delimiter                */
    int error;              /* last error                       */
    int explicit;           /* explicit match on this char      */
    int mappeddot;          /* inverse mapped '.'               */
    int mappednewline;      /* inverse mapped '\n'              */
    int mappedslash;        /* inverse mapped '/'               */
    regflags_t flags;       /* flags arg to regcomp             */
    int type;               /* BRE,ERE,ARE,SRE,KRE              */
    unsigned char *cursor;  /* curent point in re               */
    unsigned char *pattern; /* the original pattern             */
    unsigned char *literal; /* literal restart pattern  */
    int parno;              /* number of last open paren        */
    int parnest;            /* paren nest count         */
    int posixkludge;        /* to make * nonspecial             */
    Token_t token;          /* token lookahead          */
    Stats_t stats;          /* RE statistics            */
    int terminator;         /* pattern terminator               */
    Rex_t *paren[2 * (BACK_REF_MAX + 2)];
    /* paren[i]!=0 if \i defined        */
    regex_t *regex;     /* user handle                  */
    regdisc_t *disc;    /* user discipline              */
    unsigned char *map; /* external to native ccode map */
    unsigned char *MAP; /* fold and/or map              */
    Mbstate_t q;        /* pattern mb state             */
    int i;              /* macro tmp int                */
} Cenv_t;

/*
 * allocate a new Rex_t node
 */

#if _BLD_DEBUG
#define regcomp_node(a, b, c, d, e) regcomp_node(a, b, c, d, e, const char *file, unsigned int line)
#endif

static_fn Rex_t *regcomp_node(Cenv_t *env, int type, int lo, int hi, size_t extra) {
    Rex_t *e;

    DEBUG_CODE(0x0800, sfprintf(sfstdout, "%s:%d regcomp_node(%d,%d,%d,%u)\n", file, line, type, lo,
                                hi, sizeof(Rex_t) + extra));
    e = regalloc(env->disc, 0, sizeof(Rex_t) + extra);
    if (e) {
        memset(e, 0, sizeof(Rex_t) + extra);
        e->type = type;
        e->marked = 0;
        e->lo = lo;
        e->hi = hi;
        e->flags = env->flags;
        e->map = (e->flags & REG_ICASE) ? env->MAP : env->map;
        e->explicit = env->explicit;
        if (extra) e->re.data = (char *)e + sizeof(Rex_t);
    }
    return e;
}

#if _BLD_DEBUG
#undef node
#define regcomp_node(a, b, c, d, e) regcomp_node(a, b, c, d, e, __FILE__, __LINE__)
#endif

/*
 * free a Trie_node_t node
 */

static_fn void regcomp_triedrop(regdisc_t *disc, Trie_node_t *e) {
    if (e) {
        regcomp_triedrop(disc, e->son);
        regcomp_triedrop(disc, e->sib);
        (void)regalloc(disc, e, 0);
    }
}

/*
 * free a Rex_t node
 */

void regdrop(regdisc_t *disc, Rex_t *e) {
    int i;
    Rex_t *x;

    if (e && !(disc->re_flags & REG_NOFREE)) {
        do {
            switch (e->type) {
                case REX_ALT:
                case REX_CONJ:
                    regdrop(disc, e->re.group.expr.binary.left);
                    regdrop(disc, e->re.group.expr.binary.right);
                    break;
                case REX_GROUP:
                case REX_GROUP_AHEAD:
                case REX_GROUP_AHEAD_NOT:
                case REX_GROUP_BEHIND:
                case REX_GROUP_BEHIND_NOT:
                case REX_GROUP_CUT:
                case REX_NEG:
                case REX_REP:
                    regdrop(disc, e->re.group.expr.rex);
                    break;
                case REX_TRIE:
                    for (i = 0; i <= UCHAR_MAX; i++) regcomp_triedrop(disc, e->re.trie.root[i]);
                    break;
            }
            x = e->next;
            (void)regalloc(disc, e, 0);
            e = x;
        } while (e);
    }
}

/*
 * mark e and descendants minimal
 */

static_fn void regcomp_mark(Rex_t *e, int set) {
    if (e && !e->marked) {
        do {
            e->marked = 1;
            if (set) {
                e->flags |= REG_MINIMAL;
            } else {
                e->flags &= ~REG_MINIMAL;
            }
            switch (e->type) {
                case REX_ALT:
                case REX_CONJ:
                case REX_GROUP_COND:
                    if (e->re.group.expr.binary.left) {
                        regcomp_mark(e->re.group.expr.binary.left, set);
                    }
                    if (e->re.group.expr.binary.right) {
                        regcomp_mark(e->re.group.expr.binary.right, set);
                    }
                    break;
                case REX_GROUP:
                case REX_GROUP_AHEAD:
                case REX_GROUP_AHEAD_NOT:
                case REX_GROUP_BEHIND:
                case REX_GROUP_BEHIND_NOT:
                case REX_GROUP_CUT:
                case REX_NEG:
                case REX_REP:
                case REX_TRIE:
                    regcomp_mark(e->re.group.expr.rex, set);
                    break;
            }
            e = e->next;
        } while (e);
    }
}

/*
 * assign subexpression numbers by a preorder tree walk
 */

static_fn int regcomp_serialize(Cenv_t *env, Rex_t *e, int n) {
    do {
        e->serial = n++;
        switch (e->type) {
            case REX_ALT:
            case REX_GROUP_COND:
                if (e->re.group.expr.binary.left) {
                    n = regcomp_serialize(env, e->re.group.expr.binary.left, n);
                }
                e->re.group.expr.binary.serial = n++;
                if (e->re.group.expr.binary.right) {
                    n = regcomp_serialize(env, e->re.group.expr.binary.right, n);
                }
                break;
            case REX_CONJ:
                n = regcomp_serialize(env, e->re.group.expr.binary.left, n);
                n = regcomp_serialize(env, e->re.group.expr.binary.right, n);
                break;
            case REX_GROUP:
            case REX_GROUP_AHEAD:
            case REX_GROUP_AHEAD_NOT:
            case REX_GROUP_BEHIND:
            case REX_GROUP_BEHIND_NOT:
            case REX_GROUP_CUT:
            case REX_NEG:
            case REX_REP:
                n = regcomp_serialize(env, e->re.group.expr.rex, n);
                break;
        }
        e = e->next;
    } while (e);
    return n;
}

/*
 * catenate e and f into a sequence, collapsing them if possible
 */

static_fn Rex_t *regcomp_cat(Cenv_t *env, Rex_t *e, Rex_t *f) {
    Rex_t *g;

    if (!f) {
        regdrop(env->disc, e);
        return 0;
    }
    if (e->type == REX_NULL) {
        regdrop(env->disc, e);
        return f;
    }
    if (f->type == REX_NULL) {
        g = f->next;
        f->next = NULL;
        regdrop(env->disc, f);
        f = g;
    } else if (e->type == REX_DOT && f->type == REX_DOT) {
        unsigned int m = e->lo + f->lo;
        unsigned int n = e->hi + f->hi;

        if (m <= RE_DUP_MAX) {
            if (e->hi > RE_DUP_MAX || f->hi > RE_DUP_MAX) {
                n = RE_DUP_INF;
                goto combine;
            } else if (n <= RE_DUP_MAX) {
            combine:
                e->lo = m;
                e->hi = n;
                g = f->next;
                f->next = NULL;
                regdrop(env->disc, f);
                f = g;
            }
        }
    }
    e->next = f;
    return e;
}

/*
 * collect re statistics
 */

static_fn int regcomp_stats(Cenv_t *env, Rex_t *e) {
    unsigned long n;
    unsigned long m;
    unsigned long cm;
    unsigned long nm;
    unsigned long cn;
    unsigned long nn;
    unsigned long l;
    unsigned long k;
    unsigned long t;
    Rex_t *q;
    Rex_t *x;
    Rex_t *y;
    unsigned char c;
    unsigned char b;

    do {
        switch (e->type) {
            case REX_ALT:
                x = env->stats.x;
                l = env->stats.l;
                y = env->stats.y;
                k = env->stats.k;
                t = env->stats.t;
                if (++env->stats.a <= 0) return 1;
                cm = env->stats.m;
                env->stats.m = 0;
                cn = env->stats.n;
                env->stats.n = 0;
                if (regcomp_stats(env, e->re.group.expr.binary.left)) return 1;
                m = env->stats.m;
                env->stats.m = 0;
                n = env->stats.n;
                env->stats.n = 0;
                if (e->re.group.expr.binary.right &&
                    regcomp_stats(env, e->re.group.expr.binary.right)) {
                    return 1;
                }
                if (env->stats.m > m) {
                    env->stats.m = m;
                } else {
                    m = env->stats.m;
                }
                if ((env->stats.m += cm) < m) return 1;
                if (env->stats.n < n) {
                    env->stats.n = n;
                } else {
                    n = env->stats.n;
                }
                if ((env->stats.n += cn) < n) return 1;
                env->stats.x = x;
                env->stats.l = l;
                env->stats.y = y;
                env->stats.k = k;
                env->stats.t = t;
                break;
            case REX_BACK:
                if (++env->stats.b <= 0) return 1;
                break;
            case REX_CLASS:
            case REX_COLL_CLASS:
            case REX_DOT:
            case REX_ONECHAR:
                n = env->stats.m;
                if ((env->stats.m += e->lo) < n) return 1;
                if (e->hi != RE_DUP_INF) {
                    n = env->stats.n;
                    if ((env->stats.n += e->hi) < n) return 1;
                }
                if (e->lo != e->hi) {
                    if (++env->stats.c <= 0) return 1;
                    if (++env->stats.s <= 0) return 1;
                }
                break;
            case REX_CONJ:
                cm = env->stats.m;
                env->stats.m = 0;
                cn = env->stats.n;
                env->stats.n = 0;
                if (regcomp_stats(env, e->re.group.expr.binary.left)) return 1;
                nm = env->stats.m;
                env->stats.m = 0;
                nn = env->stats.n;
                env->stats.n = 0;
                if (regcomp_stats(env, e->re.group.expr.binary.right)) return 1;
                if (env->stats.m < nm) {
                    env->stats.m = nm;
                } else {
                    nm = env->stats.m;
                }
                if ((env->stats.m += cm) < nm) return 1;
                if (env->stats.n < nn) {
                    env->stats.n = nn;
                } else {
                    nn = env->stats.n;
                }
                if ((env->stats.n += cn) < nn) return 1;
                break;
            case REX_END:
                env->stats.e = 1;
                break;
            case REX_GROUP:
                if ((e->re.group.number && ++env->stats.p <= 0) ||
                    (!e->re.group.number && ++env->stats.u <= 0)) {
                    return 1;
                }
                if (regcomp_stats(env, e->re.group.expr.rex)) return 1;
                break;
            case REX_GROUP_AHEAD:
            case REX_GROUP_AHEAD_NOT:
            case REX_GROUP_BEHIND:
            case REX_GROUP_BEHIND_NOT:
                m = env->stats.m;
                n = env->stats.n;
                x = env->stats.x;
                y = env->stats.y;
                if (regcomp_stats(env, e->re.group.expr.rex)) return 1;
                env->stats.m = m;
                env->stats.n = n;
                env->stats.x = x;
                env->stats.y = y;
                switch (e->type) {
                    case REX_GROUP_AHEAD:
                    case REX_GROUP_BEHIND:
                        if (++env->stats.u <= 0) return 1;
                        break;
                }
                break;
            case REX_GROUP_COND:
                if (++env->stats.u <= 0) return 1;
                m = env->stats.m;
                n = env->stats.n;
                x = env->stats.x;
                y = env->stats.y;
                if (e->re.group.size > 0 && ++env->stats.b <= 0) return 1;
                if (e->re.group.expr.binary.left &&
                    regcomp_stats(env, e->re.group.expr.binary.left)) {
                    return 1;
                }
                q = e->re.group.expr.binary.right;
                if (q) {
                    if (q->re.group.expr.binary.left &&
                        regcomp_stats(env, q->re.group.expr.binary.left)) {
                        return 1;
                    }
                    if (q->re.group.expr.binary.right &&
                        regcomp_stats(env, q->re.group.expr.binary.right)) {
                        return 1;
                    }
                }
                env->stats.m = m;
                env->stats.n = n;
                env->stats.x = x;
                env->stats.y = y;
                break;
            case REX_GROUP_CUT:
                if (++env->stats.u <= 0) return 1;
                m = env->stats.m;
                n = env->stats.n;
                x = env->stats.x;
                y = env->stats.y;
                if (regcomp_stats(env, e->re.group.expr.rex)) return 1;
                env->stats.m = m;
                env->stats.n = n;
                env->stats.x = x;
                env->stats.y = y;
                break;
            case REX_NEG:
                env->stats.i++;
                x = env->stats.x;
                l = env->stats.l;
                y = env->stats.y;
                k = env->stats.k;
                t = env->stats.t;
                cm = env->stats.m;
                env->stats.m = 0;
                if (regcomp_stats(env, e->re.group.expr.rex)) return 1;
                env->stats.m = !env->stats.m;
                if ((env->stats.m += cm) < cm) return 1;
                env->stats.x = x;
                env->stats.l = l;
                env->stats.y = y;
                env->stats.k = k;
                env->stats.t = t;
                break;
            case REX_REP:
                x = env->stats.x;
                l = env->stats.l;
                y = env->stats.y;
                k = env->stats.k;
                t = env->stats.t;
                if (++env->stats.c <= 0) return 1;
                b = env->stats.b;
                c = env->stats.c;
                cm = env->stats.m;
                env->stats.m = 0;
                if (regcomp_stats(env, e->re.group.expr.rex)) return 1;
                if (env->stats.m == 1 && b == env->stats.b && c == env->stats.c &&
                    ++env->stats.s <= 0) {
                    return 1;
                }
                if (e->lo < 1) {
                    env->stats.x = x;
                    env->stats.l = l;
                    env->stats.y = y;
                    env->stats.k = k;
                    env->stats.t = t;
                    env->stats.m = cm;
                } else {
                    m = env->stats.m;
                    if ((env->stats.m *= e->lo) > 0 && env->stats.m < m) return 1;
                    m = env->stats.m;
                    if ((env->stats.m += cm) < m) return 1;
                    if (env->stats.x != x) env->stats.l = cm;
                    if (env->stats.y != y) env->stats.k = cm;
                }
                break;
            case REX_STRING:
                if (!e->map) {
                    cm = env->stats.m;
                    if ((env->stats.m += e->re.string.size) < cm) return 1;
                    cn = env->stats.n;
                    if ((env->stats.n += e->re.string.size) < cn) return 1;
                    if (!env->stats.x || env->stats.x->re.string.size < e->re.string.size) {
                        env->stats.x = e;
                        env->stats.l = cm;
                    }
                }
                break;
            case REX_TRIE:
                if (++env->stats.s <= 0) return 1;
                cm = env->stats.m;
                if ((env->stats.m += e->re.trie.min) < cm) return 1;
                cn = env->stats.n;
                if ((env->stats.n += e->re.trie.max) < cn) return 1;
                env->stats.t++;
                if (!env->stats.y || env->stats.y->re.trie.min < e->re.trie.min) {
                    env->stats.y = e;
                    env->stats.k = cm;
                }
                break;
        }
        e = e->next;
    } while (e);
    return 0;
}

static_fn int regcomp_token(Cenv_t *);

static_fn int regcomp_magic(Cenv_t *env, int c, int escaped) {
    char *sp;
    int n;
    int o = c;
    int e = env->error;
    int l = env->token.len;
    short *mp;
    char *ep;

    mp = regstate.magic[c];
    if (mp) {
        c = mp[env->type + escaped];
        if (c >= T_META) {
            sp = (char *)env->cursor + env->token.len;
            switch (c) {
                case T_LEFT:
                    n = 0;
                    ep = sp;
                    while (*sp >= '0' && *sp <= '9') {
                        if (n > (INT_MAX / 10)) {
                            env->error = REG_BADBR;
                            goto bad;
                        }
                        n = n * 10 + *sp++ - '0';
                    }
                    if (sp == ep) {
                        if (env->type < SRE || *sp != ',') {
                            env->error = *sp ? REG_BADBR : REG_EBRACE;
                            goto bad;
                        }
                    } else if (n > RE_DUP_MAX) {
                        env->error = REG_BADBR;
                        goto bad;
                    }
                    env->token.min = n;
                    if (*sp == ',') {
                        n = 0;
                        ep = ++sp;
                        while (*sp >= '0' && *sp <= '9') {
                            if (n > (INT_MAX / 10)) {
                                env->error = REG_BADBR;
                                goto bad;
                            }
                            n = n * 10 + *sp++ - '0';
                        }
                        if (sp == ep) {
                            n = RE_DUP_INF;
                        } else if (n < env->token.min) {
                            env->error = REG_BADBR;
                            goto bad;
                        }
                    }
                    env->token.max = n;
                    switch (*sp) {
                        case 0:
                            env->error = REG_EBRACE;
                            goto bad;
                        case '\\':
                            if (!escaped) {
                                env->error = REG_BADBR;
                                goto bad;
                            }
                            sp++;
                            break;
                        default:
                            if (escaped) {
                                env->error = REG_BADBR;
                                goto bad;
                            }
                            break;
                    }
                    switch (*sp++) {
                        case 0:
                            env->error = REG_EBRACE;
                            goto bad;
                        case '}':
                            break;
                        default:
                            env->error = REG_BADBR;
                            goto bad;
                    }
                    env->token.len = sp - (char *)env->cursor;
                    break;
                case T_RIGHT:
                    env->error = REG_EBRACE;
                    goto bad;
                case T_OPEN:
                    if (env->type < SRE && *sp == '?') {
                        env->token.len++;
                        env->token.lex = 0;
                        goto group;
                    }
                    break;
                case T_ESCAPE:
                    c = chresc(sp - 2, &ep);
                    if (ep < sp) goto bad;
                    env->token.len += ep - sp;
                    if (c >= T_META) {
                        env->token.lex = c;
                        c = C_ESC;
                    }
                    return c;
                case T_BACK + 0:
                case T_BACK + 1:
                case T_BACK + 2:
                case T_BACK + 3:
                case T_BACK + 4:
                case T_BACK + 5:
                case T_BACK + 6:
                case T_BACK + 7:
                    n = chresc(sp - 2, &ep);
                    if (ep > sp + 1) {
                        env->token.len += ep - sp;
                        return n;
                    }
                    /*FALLTHROUGH*/
                case T_BACK + 8:
                case T_BACK + 9:
                    if (env->type == SRE ||
                        (c == T_BACK && !(env->flags & (REG_LENIENT | REG_REGEXP)))) {
                        env->error = REG_BADESC;
                        goto bad;
                    }
                    if ((env->flags & REG_MULTIREF) && isdigit(*sp)) {
                        c = (c - T_BACK) * 10 + (*sp - '0');
                        if (c > 0 && c <= env->parno && env->paren[c]) {
                            c += T_BACK;
                        } else {
                            c = chresc(sp - 2, &ep);
                        }
                        env->token.len++;
                    }
                    if (c == T_BACK) c = 0;
                    break;
                case T_BAD:
                    if (escaped == 1 && (env->flags & (REG_LENIENT | REG_REGEXP)) &&
                        (c = mp[env->type + escaped + 2]) >= T_META) {
                        return c;
                    }
                    goto bad;
            }
            if (env->type >= SRE) {
                if (c == T_DOT) {
                    c = '.';
                } else if (c < T_OPEN) {
                    if (env->type == KRE && *(env->cursor + env->token.len) == '-' &&
                        *(env->cursor + env->token.len + 1) == '(') {
                        env->token.len++;
                        env->token.att = 1;
                    }
                    if (env->type == KRE && *(env->cursor + env->token.len) == '(') {
                        env->token.len++;
                        switch (c) {
                            case T_AT:
                                break;
                            case T_PERCENT:
                                env->token.lex = c;
                                goto group;
                            case T_TILDE:
                                env->token.lex = 0;
                                goto group;
                            default:
                                env->token.lex = c;
                                break;
                        }
                        c = T_OPEN;
                    } else if (c == T_STAR) {
                        c = T_DOTSTAR;
                    } else if (c == T_QUES) {
                        c = T_DOT;
                    } else {
                        c = o;
                        env->token.len = l;
                    }
                } else if (c > T_BACK) {
                    c = (c - T_BACK) * 2 - 1;
                    c = (c > env->parno || !env->paren[c]) ? o : T_BACK + c;
                } else if (env->type == KRE && !env->parnest && (env->flags & REG_SHELL_GROUP)) {
                    if (c == T_AND) {
                        c = '&';
                    } else if (c == T_BAR) {
                        c = '|';
                    } else if (c == T_OPEN) {
                        c = '(';
                    }
                }
            }
        }
    } else if (escaped == 2) {
        if ((env->type >= SRE && !(env->flags & REG_SHELL_ESCAPED)) ||
            ((env->flags & REG_ESCAPE) &&
             (c == '[' || c == '-' || c == ']' || (env->delimiter && c == env->delimiter)))) {
            ;  // okay
        } else {
            env->error = REG_BADESC;
            goto bad;
        }
    } else if (escaped && !(env->flags & (REG_LENIENT | REG_REGEXP)) && c != ']') {
        env->error = REG_BADESC;
        goto bad;
    }
    return c;
group:
    sp = (char *)env->cursor + env->token.len;
    switch (*sp++) {
        case ')': {
            break;
        }
        case '#': {
            bool done = false;
            while (!done) {
                switch (*sp++) {
                    case 0:
                        env->error = REG_EPAREN;
                        return T_BAD;
                    case ')':
                        done = true;
                        break;
                    default:
                        break;
                }
            }
            break;
        }
        default: { return T_GROUP; }
    }
    env->cursor = (unsigned char *)sp;
    return regcomp_token(env);
bad:
    if (escaped == 2) {
        env->error = e;
    } else if (env->flags & (REG_LENIENT | REG_REGEXP)) {
        return o;
    } else if (escaped == 1 && !env->error) {
        if (mp || o == ']') return o;
        env->error = REG_BADESC;
    }
    return T_BAD;
}

static_fn int regcomp_token(Cenv_t *env) {
    int c;
    int posixkludge;

    if (env->token.push) return env->token.lex;
    env->token.att = env->token.esc = 0;
    if ((env->token.len = MBSIZE(env, env->cursor)) > 1) return env->token.lex = C_MB;
    env->token.lex = 0;
    for (;;) {
        c = *env->cursor;
        if (c == 0 || c == env->delimiter || c == env->terminator) return T_END;
        if (!(env->flags & REG_COMMENT)) break;
        if (c == '#') {
            do {
                c = *++env->cursor;
                if (c == 0 || c == env->delimiter) return T_END;
            } while (c != '\n');
        } else if (!isspace(c)) {
            break;
        }
        env->cursor++;
    }
    if (c == '\n' && (env->flags & REG_MULTIPLE) && !env->delimiter) {
        if (env->parnest) {
            env->error = REG_EPAREN;
            return T_BAD;
        }
        env->parno = 0;
        env->pattern = env->cursor + 1;
        return T_BAR;
    }
    if (env->flags & REG_LITERAL) return c;
    posixkludge = env->posixkludge;
    if (posixkludge) {
        env->posixkludge = 0;
        if (c == '*') return c;
    }
    if (c == '\\') {
        if (env->flags & REG_SHELL_ESCAPED) return c;
        if (!(c = *(env->cursor + 1)) || c == env->terminator) {
            if (env->flags & (REG_LENIENT | REG_REGEXP)) {
                if (c) {
                    env->token.esc = env->token.len;
                    env->token.len += MBSIZE(env, env->cursor + 1);
                    return c;
                }
                return '\\';
            }
            env->error = REG_EESCAPE;
            return T_BAD;
        }
        env->token.esc = env->token.len;
        env->token.len += MBSIZE(env, env->cursor + 1);
        if (env->delimiter && c == 'n') {
            return '\n';
        } else if (c == env->delimiter) {
            return regcomp_magic(env, c, 0);
        } else if (c == '(' && env->type == BRE) {
            env->posixkludge = 1;
        } else if (c == ')' && env->type == BRE && env->parnest <= 0) {
            env->error = REG_EPAREN;
            return T_BAD;
        } else if (isspace(c) && (env->flags & REG_COMMENT)) {
            return c;
        }
        return regcomp_magic(env, c, 1);
    } else if (c == '$') {
        if ((env->type == BRE && (*(env->cursor + 1) == 0 || *(env->cursor + 1) == env->delimiter ||
                                  *(env->cursor + 1) == env->terminator ||
                                  (*(env->cursor + 1) == '\\' && *(env->cursor + 2) == ')'))) ||
            ((env->flags & REG_MULTIPLE) && *(env->cursor + 1) == '\n')) {
            return T_DOLL;
        }
    } else if (c == '^') {
        if (env->type == BRE && (env->cursor == env->pattern || posixkludge == 1)) {
            env->posixkludge = 2;
            return T_CFLX;
        }
    } else if (c == ')') {
        if (env->type != BRE && env->parnest <= 0) return c;
    } else if (c == '/' && env->explicit == env->mappedslash) {
        while (*(env->cursor + env->token.len) == c) env->token.len++;
        return T_SLASHPLUS;
    }
    return regcomp_magic(env, c, 0);
}

static_fn Celt_t *regcomp_col(Cenv_t *env, Celt_t *ce, int ic, unsigned char *bp, int bw, int bc,
                              unsigned char *ep, int ew, int ec) {
    char *s;
    unsigned char *k;
    unsigned char *e;
    int c;
    int cc;
    int bt;
    int et;
    wchar_t w;
    Mbstate_t q;
    Ckey_t key;

    cc = 0;
    for (;;) {
        k = key;
        if (bw == 1) {
            c = bc;
            if (ic) {
                if (isupper(c)) {
                    c = tolower(c);
                    cc = -1;
                } else if (islower(c)) {
                    c = toupper(c);
                    cc = -1;
                }
            }
            *k++ = c;
        } else if (bw < COLL_KEY_MAX) {
            s = (char *)bp;
            if (ic) {
                c = mbchar(&w, &s, MB_LEN_MAX, &env->q);
                if (iswupper(c)) {
                    c = towlower(c);
                    cc = 1;
                } else if (iswlower(c)) {
                    c = towupper(c);
                    cc = 1;
                }
            }
            if (cc > 0) {
                cc = -1;
                mbinit(&q);
                k += mbconv((char *)k, c, &q);
            } else {
                for (e = k + bw; k < e; *k++ = *s++) {
                    ;
                }
            }
        }
        *k = 0;
        mbxfrm(ce->beg, key, COLL_KEY_MAX);
        if (ep) {
            k = key;
            mbinit(&env->q);
            c = mbchar(&w, (char **)&k, MB_LEN_MAX, &env->q);
            if (iswupper(c)) {
                bt = COLL_range_uc;
            } else if (iswlower(c)) {
                bt = COLL_range_lc;
            } else {
                bt = COLL_range;
            }
            k = key;
            if (ew == 1) {
                c = ec;
                if (ic) {
                    if (isupper(c)) {
                        c = tolower(c);
                        cc = -1;
                    } else if (islower(c)) {
                        c = toupper(c);
                        cc = -1;
                    }
                }
                *k++ = c;
            } else if (ew < COLL_KEY_MAX) {
                s = (char *)ep;
                if (ic) {
                    mbinit(&env->q);
                    c = mbchar(&w, &s, MB_LEN_MAX, &env->q);
                    if (iswupper(c)) {
                        c = towlower(c);
                        cc = 1;
                    } else if (iswlower(c)) {
                        c = towupper(c);
                        cc = 1;
                    }
                }
                if (cc > 0) {
                    cc = -1;
                    mbinit(&q);
                    k += mbconv((char *)k, c, &q);
                } else {
                    for (e = k + ew; k < e; *k++ = *s++) {
                        ;
                    }
                }
            }
            *k = 0;
            mbxfrm(ce->end, key, COLL_KEY_MAX);
            k = key;
            c = mbchar(&w, (char **)&k, MB_LEN_MAX, &env->q);
            if (iswupper(c)) {
                et = COLL_range_uc;
            } else if (iswlower(c)) {
                et = COLL_range_lc;
            } else {
                et = COLL_range;
            }
            ce->typ = bt == et ? bt : COLL_range;
        } else {
            ce->typ = COLL_char;
        }
        ce++;
        if (!ic || !cc) break;
        ic = 0;
    }
    return ce;
}

static_fn Rex_t *regcomp_bra(Cenv_t *env) {
    Rex_t *e;
    int c;
    int i;
    int w;
    int neg;
    int last;
    int inrange;
    int complicated;
    int collate;
    int elements;
    unsigned char *first;
    unsigned char *start;
    unsigned char *begin;
    unsigned char *s;
    regclass_t f;
    Mbstate_t q;
    unsigned char buf[4 * (COLL_KEY_MAX + 1)];
    int ic;
    char mbc[COLL_KEY_MAX + 1];
    Cchr_t *cc_keys = 0;

    if (!(e = regcomp_node(env, REX_CLASS, 1, 1, sizeof(Set_t)))) return 0;
    collate = complicated = elements = 0;
    if (*env->cursor == '^' || (env->type >= SRE && *env->cursor == '!')) {
        env->cursor++;
        complicated = neg = 1;
    } else {
        neg = 0;
    }
    first = env->cursor;
    start = first + MBSIZE(env, first);
    if (*env->cursor == 0 || *(env->cursor + 1) == 0 || *env->cursor == env->terminator ||
        *(env->cursor + 1) == env->terminator ||
        ((env->flags & REG_ESCAPE) &&
         (*env->cursor == env->delimiter ||
          (*env->cursor != '\\' && *(env->cursor + 1) == env->delimiter)))) {
        goto error;
    }
    begin = env->cursor + MBSIZE(env, env->cursor);

    /*
     * inrange: 0=no, 1=possibly, 2=definitely
     */

    inrange = 0;
    for (;;) {
        if (!(c = *env->cursor) || c == env->terminator ||
            (c == env->delimiter && (env->flags & REG_ESCAPE))) {
            goto error;
        }
        env->cursor += (w = MBSIZE(env, env->cursor));
        if (c == '\\' && ((env->flags & REG_CLASS_ESCAPE) ||
                          (*env->cursor == env->delimiter && (env->flags & REG_ESCAPE)))) {
            if (*env->cursor) {
                if (*env->cursor == 'n') {
                    env->cursor++;
                    c = '\n';
                } else if (env->type < SRE || !(env->flags & REG_SHELL_ESCAPED)) {
                    env->token.len = 1;
                    w = regcomp_magic(env, *env->cursor, 2);
                    if (env->token.len > 1 || w != T_BAD) {
                        if (env->token.len == 1 && (f = regclassfun(w))) {
                            if (inrange > 1) {
                                if (env->type < SRE && !(env->flags & (REG_LENIENT | REG_REGEXP))) {
                                    goto erange;
                                }
                                inrange = 0;
                            }
                            env->cursor++;
                            for (c = 0; c <= UCHAR_MAX; c++) {
                                if ((*f)(c)) setadd(e->re.charclass, c);
                            }
                            complicated++;
                            elements++;
                            continue;
                        }
                        if (env->token.len > 1 || (w >= 0 && w < T_META)) {
                            c = w;
                            if (c > UCHAR_MAX) {
                                if (env->type < SRE && !(env->flags & (REG_LENIENT | REG_REGEXP)) &&
                                    !mbwide()) {
                                    goto erange;
                                }
                                c = UCHAR_MAX;
                            }
                            env->cursor += env->token.len;
                        }
                    }
                }
            }
        } else if (c == ']') {
            if (env->cursor == begin) {
                last = c;
                inrange = 1;
                continue;
            }
            if (inrange != 0) {
                setadd(e->re.charclass, last);
                elements++;
                if (inrange == 2) {
                    setadd(e->re.charclass, '-');
                    elements++;
                }
            }
            break;
        } else if (c == '-') {
            if (!inrange && env->cursor != begin && *env->cursor != ']') {
                if (env->type < SRE && !(env->flags & (REG_LENIENT | REG_REGEXP))) goto erange;
                continue;
            } else if (inrange == 1) {
                inrange = 2;
                complicated++;
                continue;
            }
        } else if (c == '[') {
            switch (*env->cursor) {
                case 0:
                    goto error;
                case ':':
                    if (env->flags & REG_REGEXP) goto normal;
                    if (inrange == 1) {
                        setadd(e->re.charclass, last);
                        elements++;
                    }
                    if (!(f = regclass((char *)env->cursor, (char **)&env->cursor))) {
                        if (env->cursor == start && (c = *(env->cursor + 1))) {
                            s = start = env->cursor + 1;
                            while (*++s && *s != ':') {
                                ;
                            }
                            if (*s == ':' && *(s + 1) == ']' && *(s + 2) == ']') {
                                if ((i = (s - start)) == 1) {
                                    switch (c) {
                                        case '<':
                                            i = REX_WBEG;
                                            break;
                                        case '>':
                                            i = REX_WEND;
                                            break;
                                        default:
                                            i = 0;
                                            break;
                                    }
                                    if (i) {
                                        env->cursor = s + 3;
                                        regdrop(env->disc, e);
                                        return regcomp_node(env, i, 0, 0, 0);
                                    }
                                }
                            }
                        }
                        env->error = REG_ECTYPE;
                        goto error;
                    }
                    for (c = 0; c <= UCHAR_MAX; c++) {
                        if ((*f)(c)) setadd(e->re.charclass, c);
                    }
                    inrange = 0;
                    complicated++;
                    elements++;
                    continue;
                case '=':
                    if (env->flags & REG_REGEXP) goto normal;
                    if (inrange == 2) goto erange;
                    if (inrange == 1) {
                        setadd(e->re.charclass, last);
                        elements++;
                    }
                    if ((c = regcollate((char *)env->cursor, (char **)&env->cursor, (char *)buf,
                                        sizeof(buf), NULL)) < 0) {
                        goto ecollate;
                    }
                    if (c > 1) {
                        collate++;
                    } else {
                        setadd(e->re.charclass, buf[0]);
                    }
                    inrange = 0;
                    complicated++;
                    elements++;
                    continue;
                case '.':
                    if (env->flags & REG_REGEXP) goto normal;
                    if ((c = regcollate((char *)env->cursor, (char **)&env->cursor, (char *)buf,
                                        sizeof(buf), NULL)) < 0) {
                        goto ecollate;
                    }
                    if (c > 1) collate++;
                    c = buf[0];
                    complicated++;
                    break;
                default:
                normal:
                    if (*env->cursor == env->terminator ||
                        (*env->cursor == env->delimiter && (env->flags & REG_ESCAPE))) {
                        goto error;
                    }
                    break;
            }
        } else if (w > 1) {
            complicated++;
        }
        if (inrange == 2) {
            if (last <= c) {
                for (i = last; i <= c; i++) setadd(e->re.charclass, i);
                inrange = env->type >= SRE || (env->flags & (REG_LENIENT | REG_REGEXP));
                elements += 2;
            } else if (env->type >= SRE) {
                setadd(e->re.charclass, last);
                setadd(e->re.charclass, c);
                elements += 2;
                inrange = 1;
            } else if (!(env->flags & (REG_LENIENT | REG_REGEXP))) {
                goto erange;
            } else {
                inrange = 0;
            }
        } else if (inrange == 1) {
            setadd(e->re.charclass, last);
            elements++;
        } else {
            inrange = 1;
        }
        last = c;
    }

    if (complicated) {
        Dt_t *dt;
        Cchr_t *cc;
        Cchr_t *tc;
        Cchr_t *xc;
        Celt_t *ce;
        Cchr_t key;
        int rw;
        int rc;
        wchar_t wc;
        unsigned char *rp;
        unsigned char *pp;
        char cb[2][COLL_KEY_MAX + 1];

        static Dtdisc_t disc;

        static const char primary[] =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

        dt = lc_collate_data;
        if (ast.locale.serial != lc_collate_serial || !dt) {
            disc.key = offsetof(Cchr_t, key);
            cc_keys = cc = calloc(1, elementsof(primary) * sizeof(Cchr_t));
            if (cc && (dt = dtopen(&disc, Dtoset))) {
                for (i = 0; i < elementsof(primary) - 1; i++, cc++) {
                    cc->nam[0] = primary[i];
                    mbxfrm(cc->key, cc->nam, COLL_KEY_MAX);
                    dtinsert(dt, cc);
                }
                for (i = 0; i < elementsof(cc->key); i++) cc->key[i] = ~0;
                dtinsert(dt, cc);
                lc_collate_data = dt;
                lc_collate_serial = ast.locale.serial;
            } else {
                if (cc) free(cc);
                regdrop(env->disc, e);
                return 0;
            }
        }

        if (dt) {
            regdrop(env->disc, e);
            ic = env->flags & REG_ICASE;
            if (ic) elements *= 2;
            e = regcomp_node(env, REX_COLL_CLASS, 1, 1, (elements + 3) * sizeof(Celt_t));
            if (!e) {
                free(cc_keys);
                return 0;
            }
            ce = (Celt_t *)e->re.data;
            e->re.collate.invert = neg;
            e->re.collate.elements = ce;
            env->cursor = first;
            inrange = 0;
            for (;;) {
                if ((c = *env->cursor) == 0 || c == env->terminator ||
                    ((env->flags & REG_ESCAPE) && c == env->delimiter)) {
                    goto error;
                }
                pp = env->cursor;
                env->cursor += (w = MBSIZE(env, env->cursor));
                if (c == '\\' && ((env->flags & REG_CLASS_ESCAPE) ||
                                  (*env->cursor == env->delimiter && (env->flags & REG_ESCAPE)))) {
                    if (*env->cursor) {
                        if (*env->cursor == 'n') {
                            pp = env->cursor++;
                            c = '\n';
                        } else if (env->type < SRE || !(env->flags & REG_SHELL_ESCAPED)) {
                            env->token.len = 1;
                            w = regcomp_magic(env, *env->cursor, 2);
                            if (env->token.len > 1 || w != T_BAD) {
                                if (env->token.len == 1 && (f = regclassfun(w))) {
                                    if (inrange > 1) {
                                        if (env->type < SRE &&
                                            !(env->flags & (REG_LENIENT | REG_REGEXP))) {
                                            goto erange;
                                        }
                                        inrange = 0;
                                    }
                                    env->cursor++;
                                    ce->fun = f;
                                    ce->typ = COLL_call;
                                    ce++;
                                    continue;
                                }
                                if (env->token.len > 1 || (w >= 0 && w < T_META)) {
                                    c = w;
                                    mbinit(&q);
                                    w = mbconv(mbc, c, &q);
                                    pp = (unsigned char *)mbc;
                                    env->cursor += env->token.len;
                                }
                            }
                        }
                    }
                } else if (c == ']') {
                    if (env->cursor == begin) {
                        rp = pp;
                        rw = w;
                        inrange = 1;
                        continue;
                    }
                    if (inrange != 0) {
                        ce = regcomp_col(env, ce, ic, rp, rw, rc, NULL, 0, 0);
                        if (inrange == 2) ce = regcomp_col(env, ce, ic, NULL, 1, '-', NULL, 0, 0);
                    }
                    break;
                } else if (c == '-') {
                    if (!inrange && env->cursor != begin && *env->cursor != ']') {
                        if (env->type < SRE && !(env->flags & (REG_LENIENT | REG_REGEXP))) {
                            goto erange;
                        }
                        continue;
                    } else if (inrange == 1) {
                        inrange = 2;
                        continue;
                    }
                } else if (c == '[') {
                    switch (*env->cursor) {
                        case 0:
                            goto error;
                        case ':':
                            if (env->flags & REG_REGEXP) {
                                goto complicated_normal;
                            }
                            if (inrange == 1) ce = regcomp_col(env, ce, ic, rp, rw, rc, NULL, 0, 0);
                            if (!(f = regclass((char *)env->cursor, (char **)&env->cursor))) {
                                if (env->cursor == start && (c = *(env->cursor + 1)) &&
                                    *(env->cursor + 2) == ':' && *(env->cursor + 3) == ']' &&
                                    *(env->cursor + 4) == ']') {
                                    switch (c) {
                                        case '<':
                                            i = REX_WBEG;
                                            break;
                                        case '>':
                                            i = REX_WEND;
                                            break;
                                        default:
                                            i = 0;
                                            break;
                                    }
                                    if (i) {
                                        env->cursor += 5;
                                        regdrop(env->disc, e);
                                        free(cc_keys);
                                        return regcomp_node(env, i, 0, 0, 0);
                                    }
                                }
                                env->error = REG_ECTYPE;
                                goto error;
                            }
                            ce->fun = f;
                            ce->typ = COLL_call;
                            ce++;
                            inrange = 0;
                            continue;
                        case '=':
                            if (env->flags & REG_REGEXP) {
                                goto complicated_normal;
                            }
                            if (inrange == 2) {
                                goto erange;
                            }
                            if (inrange == 1) ce = regcomp_col(env, ce, ic, rp, rw, rc, NULL, 0, 0);
                            pp = (unsigned char *)cb[inrange];
                            rp = env->cursor + 1;
                            if ((rw = regcollate((char *)env->cursor, (char **)&env->cursor,
                                                 (char *)pp, COLL_KEY_MAX, &wc)) < 0) {
                                goto ecollate;
                            }
                            c = 0;
                            if (ic) {
                                if (iswupper(wc)) {
                                    wc = towlower(wc);
                                    mbinit(&q);
                                    rw = mbconv((char *)pp, wc, &q);
                                    c = 'u';
                                } else if (iswlower(wc)) {
                                    c = 'l';
                                }
                            }
                            i = 1;
                            for (;;) {
                                mbxfrm(key.key, (char *)pp, COLL_KEY_MAX);
                                if (!(cc = (Cchr_t *)dtsearch(dt, &key)) &&
                                    !(cc = (Cchr_t *)dtprev(dt, &key))) {
                                    if (i) {
                                        c = *pp;
                                        goto singleton;
                                    }
                                    goto ecollate;
                                }
                                xc = (tc = (Cchr_t *)dtprev(dt, cc)) &&
                                             !strcasecmp((char *)tc->nam, (char *)cc->nam)
                                         ? tc
                                         : cc;
                                if (c == 'l' || c == 'L') {
                                    if (c == 'L') c = 0;
                                    ce->typ = COLL_range_lc;
                                } else if (c == 'u' || c == 'U') {
                                    if (c == 'U') c = 0;
                                    ce->typ = COLL_range_uc;
                                } else {
                                    ce->typ = COLL_range;
                                }
                                strcpy((char *)ce->beg, (char *)xc->key);
                                if (!(cc = (Cchr_t *)dtnext(dt, cc))) {
                                    if (i) {
                                        c = *pp;
                                        goto singleton;
                                    }
                                    goto ecollate;
                                }
                                if (!strcasecmp((char *)xc->nam, (char *)cc->nam) &&
                                    (tc = (Cchr_t *)dtnext(dt, cc))) {
                                    cc = tc;
                                }
                                strcpy((char *)ce->end, (char *)cc->key);
                                ce->max = -1;
                                ce++;
                                if (!c) break;
                                if (c == 'u') {
                                    wc = towlower(wc);
                                    c = 'L';
                                } else {
                                    wc = towupper(wc);
                                    c = 'U';
                                }
                                mbinit(&q);
                                rw = mbconv((char *)pp, wc, &q);
                                i = 0;
                            }
                            inrange = 0;
                            continue;
                        case '.':
                            if (env->flags & REG_REGEXP) {
                                goto complicated_normal;
                            }
                            pp = (unsigned char *)cb[inrange];
                            if ((w = regcollate((char *)env->cursor, (char **)&env->cursor,
                                                (char *)pp, COLL_KEY_MAX, NULL)) < 0) {
                                goto ecollate;
                            }
                            c = *pp;
                            break;
                        default:
                        complicated_normal:
                            if (*env->cursor == env->terminator ||
                                (*env->cursor == env->delimiter && (env->flags & REG_ESCAPE))) {
                                goto error;
                            }
                            break;
                    }
                }
            singleton:
                if (inrange == 2) {
                    ce = regcomp_col(env, ce, ic, rp, rw, rc, pp, w, c);
                    if (strcmp((char *)ce->beg, (char *)ce->end) > 0) {
                        if (env->type < SRE && !(env->flags & (REG_LENIENT | REG_REGEXP))) {
                            goto erange;
                        }
                        (ce - 1)->typ = COLL_char;
                        strcpy((char *)ce->beg, (char *)(ce - 1)->end);
                        ce->typ = COLL_char;
                        ce++;
                    }
                    inrange = env->type >= SRE || (env->flags & (REG_LENIENT | REG_REGEXP));
                } else if (inrange == 1) {
                    ce = regcomp_col(env, ce, ic, rp, rw, rc, NULL, 0, 0);
                } else {
                    inrange = 1;
                }
                rp = pp;
                rw = w;
                rc = c;
            }
            ce->typ = COLL_end;
            free(cc_keys);
            return e;
        }

        free(cc_keys);
    }

    if (collate) goto ecollate;
    if (env->flags & REG_ICASE) {
        for (i = 0; i <= UCHAR_MAX; i++) {
            if (settst(e->re.charclass, i)) {
                if (isupper(i)) {
                    c = tolower(i);
                } else if (islower(i)) {
                    c = toupper(i);
                } else {
                    continue;
                }
                setadd(e->re.charclass, c);
            }
        }
    }
    if (neg) {
        for (i = 0; i < elementsof(e->re.charclass->bits); i++) e->re.charclass->bits[i] ^= ~0;
        if (env->explicit >= 0) setclr(e->re.charclass, env->explicit);
    }

    return e;
ecollate:
    env->error = REG_ECOLLATE;
    goto error;
erange:
    env->error = REG_ERANGE;
error:
    free(cc_keys);
    regdrop(env->disc, e);
    if (!env->error) env->error = REG_EBRACK;
    return 0;
}

static_fn Rex_t *regcomp_ccl(Cenv_t *env, int type) {
    Rex_t *e;
    Celt_t *ce;
    regclass_t f;

    f = regclassfun(type);
    if (!f) {
        env->error = REG_BADESC;
        return NULL;
    }

    e = regcomp_node(env, REX_COLL_CLASS, 1, 1, 2 * sizeof(Celt_t));
    if (!e) return NULL;
    ce = (Celt_t *)e->re.data;
    e->re.collate.invert = 0;
    e->re.collate.elements = ce;
    ce->fun = f;
    ce->typ = COLL_call;
    ce++;
    ce->typ = COLL_end;
    return e;
}

static_fn Rex_t *regcomp_rep(Cenv_t *env, Rex_t *e, int number, int last) {
    Rex_t *f;
    unsigned long m = 0;
    unsigned long n = RE_DUP_INF;
    int minimal = -1;

    if (!e) return 0;
    switch (regcomp_token(env)) {
        case T_BANG:
            eat(env);
            if (!(f = regcomp_node(env, REX_NEG, m, n, 0))) {
                regdrop(env->disc, e);
                return 0;
            }
            f->re.group.expr.rex = e;
            return f;
        case T_QUES:
            eat(env);
            n = 1;
            break;
        case T_STAR:
            eat(env);
            break;
        case T_PLUS:
            eat(env);
            m = 1;
            break;
        case T_LEFT:
            eat(env);
            m = env->token.min;
            n = env->token.max;
            break;
        default:
            return e;
    }
    if (env->token.att) {
        minimal = 1;
    } else if (env->type < SRE) {
        switch (regcomp_token(env)) {
            case T_QUES:
                eat(env);
                minimal = (env->flags & REG_MINIMAL) == 0;
                break;
            case T_STAR: /*AST*/
                eat(env);
                minimal = (env->flags & REG_MINIMAL) != 0;
                break;
        }
    }
    switch (e->type) {
        case REX_DOT:
        case REX_CLASS:
        case REX_COLL_CLASS:
        case REX_ONECHAR:
            e->lo = m;
            e->hi = n;
            if (minimal >= 0) regcomp_mark(e, minimal);
            return e;
        case REX_BEG_STR:
        case REX_END_STR:
        case REX_FIN_STR:
        case REX_WBEG:
        case REX_WEND:
        case REX_WORD:
        case REX_WORD_NOT:
            env->error = REG_BADRPT;
            regdrop(env->disc, e);
            return 0;
    }
    if (m == 1 && n == 1) {
        if (minimal >= 0) regcomp_mark(e, minimal);
        return e;
    }
    if (!(f = regcomp_node(env, REX_REP, m, n, 0))) {
        regdrop(env->disc, e);
        return 0;
    }
    f->re.group.expr.rex = e;
    f->re.group.number = number;
    f->re.group.last = last;
    if (minimal >= 0) regcomp_mark(f, minimal);
    if (m <= n && n) {
        for (; e && e->type >= REX_GROUP && e->type <= REX_GROUP_CUT; e = e->re.group.expr.rex) {
            ;
        }
        if (e && e->type == REX_NEG) f->type = REX_GROUP;
    }
    return f;
}

static_fn int regcomp_isstring(Rex_t *e) {
    switch (e->type) {
        case REX_ONECHAR:
            return e->lo == 1 && e->hi == 1;
        case REX_STRING:
            return 1;
    }
    return 0;
}

static_fn Trie_node_t *regcomp_trienode(Cenv_t *env, int c) {
    Trie_node_t *t;

    t = regalloc(env->disc, 0, sizeof(Trie_node_t));
    if (t) {
        memset(t, 0, sizeof(Trie_node_t));
        t->c = c;
    }
    return t;
}

static_fn int regcomp_insert(Cenv_t *env, Rex_t *f, Rex_t *g) {
    unsigned char *s;
    unsigned char *e;
    Trie_node_t *t;
    int len;
    unsigned char tmp[2];

    switch (f->type) {
        case REX_ONECHAR:
            *(s = tmp) = f->re.onechar;
            e = s + 1;
            break;
        case REX_STRING:
            s = f->re.string.base;
            e = s + f->re.string.size;
            break;
        default:
            return 1;
    }
    if (!(t = g->re.trie.root[*s]) && !(t = g->re.trie.root[*s] = regcomp_trienode(env, *s))) {
        return 1;
    }
    for (len = 1;;) {
        if (t->c == *s) {
            if (++s >= e) break;
            if (!t->son && !(t->son = regcomp_trienode(env, *s))) return 1;
            t = t->son;
            len++;
        } else {
            if (!t->sib && !(t->sib = regcomp_trienode(env, *s))) return 1;
            t = t->sib;
        }
    }
    if (g->re.trie.min > len) g->re.trie.min = len;
    if (g->re.trie.max < len) g->re.trie.max = len;
    t->end = 1;
    return 0;
}

/*
 * trie() tries to combine nontrivial e and f into a REX_TRIE
 * unless 0 is returned, e and f are deleted as far as possible.
 */

static_fn Rex_t *regcomp_trie(Cenv_t *env, Rex_t *e, Rex_t *f) {
    Rex_t *g;

    if (e->next || f->next || !regcomp_isstring(e) || e->flags != f->flags) return 0;
    if (regcomp_isstring(f)) {
        if (!(g = regcomp_node(env, REX_TRIE, 0, 0, (UCHAR_MAX + 1) * sizeof(Trie_node_t *)))) {
            return 0;
        }
        g->re.trie.min = INT_MAX;
        if (regcomp_insert(env, f, g)) goto nospace;
        regdrop(env->disc, f);
    } else if (f->type != REX_TRIE) {
        return 0;
    } else {
        g = f;
    }
    if (regcomp_insert(env, e, g)) goto nospace;
    regdrop(env->disc, e);
    return g;
nospace:
    if (g != f) regdrop(env->disc, g);
    return 0;
}

static_fn Rex_t *regcomp_alt(Cenv_t *, int, int);

static_fn int regcomp_chr(Cenv_t *env, int *escaped) {
    unsigned char *p;
    int c;

    *escaped = 0;
    if (!(c = *env->cursor)) return -1;
    env->cursor++;
    if (c == '\\') {
        if (env->flags & REG_SHELL_ESCAPED) return c;
        if (!(c = *(env->cursor + 1)) || c == env->terminator) {
            if (env->flags & (REG_LENIENT | REG_REGEXP)) return c ? c : '\\';
            env->error = REG_EESCAPE;
            return -1;
        }
        p = env->cursor;
        c = chresc((char *)env->cursor - 1, (char **)&env->cursor);
        *escaped = env->cursor - p;
    }
    return c;
}

/*
 * open the perly gates
 */

static_fn Rex_t *regcomp_grp(Cenv_t *env, int parno) {
    Rex_t *e;
    Rex_t *f;
    int c;
    int g;
    int i;
    int n;
    int x;
    int esc;
    int typ;
    int beg;
    unsigned char *p;

    g = env->flags;
    beg = env->pattern == env->cursor - env->token.len;
    if (!(c = env->token.lex) && (c = *env->cursor)) env->cursor++;
    env->token.len = 0;
    env->parnest++;
    typ = -1;
    switch (c) {
        case '-':
        case '+':
        case 'a':
        case 'g':
        case 'i':
        case 'l':
        case 'm':
        case 'p':
        case 'r':
        case 's':
        case 'x':
        case 'A':
        case 'B':
        case 'E':
        case 'F':
        case 'G':
        case 'I':
        case 'K':
        case 'L':
        case 'M': /* glob(3) */
        case 'N': /* glob(3) */
        case 'O': /* glob(3) */
        case 'P':
        case 'R': /* pcre */
        case 'S':
        case 'U': /* pcre */
        case 'V':
        case 'X': /* pcre */
            x = REX_GROUP;
            i = 1;
            env->token.push = 1;
            for (;;) {
                switch (c) {
                    case ')':
                        if (!(env->flags & (REG_LITERAL | REG_SHELL))) {
                            env->error = REG_BADRPT;
                            return 0;
                        }
                        /*FALLTHROUGH*/
                    case 0:
                    case T_CLOSE:
                        x = 0;
                        goto done;
                    case ':':
                        eat(env);
                        if (regcomp_token(env) == T_CLOSE) x = 0;
                        goto done;
                    case '-':
                        i = 0;
                        break;
                    case '+':
                        i = 1;
                        break;
                    case 'a':
                        if (i) {
                            env->flags |= (REG_LEFT | REG_RIGHT);
                        } else {
                            env->flags &= ~(REG_LEFT | REG_RIGHT);
                        }
                        break;
                    case 'g':
                        if (i) {
                            env->flags &= ~REG_MINIMAL;
                        } else {
                            env->flags |= REG_MINIMAL;
                        }
                        break;
                    case 'i':
                        if (i) {
                            env->flags |= REG_ICASE;
                        } else {
                            env->flags &= ~REG_ICASE;
                        }
                        break;
                    case 'l':
                        if (i) {
                            env->flags |= REG_LEFT;
                        } else {
                            env->flags &= ~REG_LEFT;
                        }
                        break;
                    case 'm':
                        if (i) {
                            env->flags |= REG_NEWLINE;
                        } else {
                            env->flags &= ~REG_NEWLINE;
                        }
                        env->explicit = (env->flags & (REG_NEWLINE | REG_SPAN)) == REG_NEWLINE
                                            ? env->mappednewline
                                            : -1;
                        break;
                    case 'p':
                        if (i) {
                            env->flags &= ~REG_LENIENT;
                        } else {
                            env->flags |= REG_LENIENT;
                        }
                        break;
                    case 'r':
                        if (i) {
                            env->flags |= REG_RIGHT;
                        } else {
                            env->flags &= ~REG_RIGHT;
                        }
                        break;
                    case 's':
                        if (i) {
                            env->flags |= REG_SPAN;
                        } else {
                            env->flags &= ~REG_SPAN;
                        }
                        env->explicit = (env->flags & (REG_NEWLINE | REG_SPAN)) == REG_NEWLINE
                                            ? env->mappednewline
                                            : -1;
                        break;
                    case 'x':
                        if (i) {
                            env->flags |= REG_COMMENT;
                        } else {
                            env->flags &= ~REG_COMMENT;
                        }
                        break;
                    case 'X':
                        if (typ >= 0 || (env->type == ERE && (env->flags & REG_CLASS_ESCAPE))) {
                            break;  // PCRE_EXTRA
                        }
                        // FALLTHROUGH
                    case 'A':
                        env->flags &= ~(REG_AUGMENTED | REG_EXTENDED | REG_LITERAL | REG_REGEXP |
                                        REG_SHELL | REG_LEFT | REG_RIGHT);
                        env->flags |= REG_AUGMENTED | REG_EXTENDED;
                        typ = ARE;
                        break;
                    case 'B':
                    case 'G':
                        env->flags &= ~(REG_AUGMENTED | REG_EXTENDED | REG_LITERAL | REG_REGEXP |
                                        REG_SHELL | REG_LEFT | REG_RIGHT);
                        typ = BRE;
                        break;
                    case 'E':
                        env->flags &= ~(REG_AUGMENTED | REG_EXTENDED | REG_LITERAL | REG_REGEXP |
                                        REG_SHELL | REG_LEFT | REG_RIGHT);
                        env->flags |= REG_EXTENDED;
                        typ = ERE;
                        break;
                    case 'F':
                    case 'L':
                        env->flags &= ~(REG_AUGMENTED | REG_EXTENDED | REG_LITERAL | REG_REGEXP |
                                        REG_SHELL | REG_LEFT | REG_RIGHT);
                        env->flags |= REG_LITERAL;
                        typ = ERE;
                        break;
                    case 'K':
                        env->flags &= ~(REG_AUGMENTED | REG_EXTENDED | REG_LITERAL | REG_REGEXP |
                                        REG_SHELL | REG_LEFT | REG_RIGHT);
                        env->flags |= REG_AUGMENTED | REG_SHELL | REG_LEFT | REG_RIGHT;
                        typ = KRE;
                        break;
                    case 'M':
                        /* used by caller to disable glob(3) GLOB_BRACE */
                        break;
                    case 'N':
                        /* used by caller to disable glob(3) GLOB_NOCHECK */
                        break;
                    case 'O':
                        /* used by caller to disable glob(3) GLOB_STARSTAR */
                        break;
                    case 'P':
                        env->flags &= ~(REG_AUGMENTED | REG_EXTENDED | REG_LITERAL | REG_REGEXP |
                                        REG_SHELL | REG_LEFT | REG_RIGHT);
                        env->flags |= REG_EXTENDED | REG_CLASS_ESCAPE;
                        typ = ERE;
                        break;
                    case 'S':
                        env->flags &= ~(REG_AUGMENTED | REG_EXTENDED | REG_LITERAL | REG_REGEXP |
                                        REG_SHELL | REG_LEFT | REG_RIGHT);
                        env->flags |= REG_SHELL | REG_LEFT | REG_RIGHT;
                        typ = SRE;
                        break;
                    case 'U': /* PCRE_UNGREEDY */
                        if (typ >= 0 || (env->type == ERE && (env->flags & REG_CLASS_ESCAPE))) {
                            if (i) {
                                env->flags |= REG_MINIMAL;
                            } else {
                                env->flags &= ~REG_MINIMAL;
                            }
                        }
                        break;
                    case 'V':
                        env->flags &= ~(REG_AUGMENTED | REG_EXTENDED | REG_LITERAL | REG_REGEXP |
                                        REG_SHELL | REG_LEFT | REG_RIGHT);
                        env->flags |= REG_REGEXP;
                        typ = BRE;
                        break;
                    default:
                        env->error = REG_BADRPT;
                        return 0;
                }
                eat(env);
                c = regcomp_token(env);
            }
        done:
            break;
        case ':':
            x = REX_GROUP;
            break;
        case '=':
            x = REX_GROUP_AHEAD;
            break;
        case '!':
            x = REX_GROUP_AHEAD_NOT;
            break;
        case '<':
            switch (regcomp_token(env)) {
                case '=':
                    x = REX_GROUP_BEHIND;
                    break;
                case '!':
                case T_BANG:
                    x = REX_GROUP_BEHIND_NOT;
                    break;
                default:
                    env->error = REG_BADRPT;
                    return 0;
            }
            eat(env);
            break;
        case '>':
            x = REX_GROUP_CUT;
            break;
        case '%':
        case T_PERCENT:
            e = regcomp_node(env, REX_NEST, 0, 0, (UCHAR_MAX + 1) * sizeof(unsigned short));
            e->re.nest.primary = isalnum(*env->cursor) ? -1 : *env->cursor;
            n = 1;
            for (;;) {
                switch (i = regcomp_chr(env, &esc)) {
                    case -1:
                    case 0:
                    invalid:
                        env->cursor -= esc + 1;
                        env->error = REG_EPAREN;
                        return 0;
                    case 'D':
                        x = REX_NEST_delimiter;
                        /*FALLTHROUGH*/
                    delimiter:
                        if ((i = regcomp_chr(env, &esc)) < 0) goto invalid;
                        if (e->re.nest.type[i] & ~x) goto invalid;
                        e->re.nest.type[i] = x;
                        continue;
                    case 'E':
                        x = REX_NEST_escape;
                        goto delimiter;
                    case 'L':
                        x = REX_NEST_literal;
                        goto quote;
                    case 'O':
                        switch (i = regcomp_chr(env, &esc)) {
                            case 'T':
                                e->re.nest.type[UCHAR_MAX + 1] |= REX_NEST_terminator;
                                break;
                            default:
                                goto invalid;
                        }
                        continue;
                    case 'Q':
                        x = REX_NEST_quote;
                        /*FALLTHROUGH*/
                    quote:
                        if ((i = regcomp_chr(env, &esc)) < 0) goto invalid;
                        if (e->re.nest.type[i] & ~x) goto invalid;
                        e->re.nest.type[i] =
                            x | REX_NEST_open | REX_NEST_close | (i << REX_NEST_SHIFT);
                        continue;
                    case 'S':
                        x = REX_NEST_separator;
                        goto delimiter;
                    case 'T':
                        x = REX_NEST_terminator;
                        goto delimiter;
                    case '|':
                    case '&':
                        if (!esc) goto invalid;
                        goto nesting;
                    case '(':
                        if (!esc) n++;
                        goto nesting;
                    case ')':
                        if (!esc && !--n) break;
                        goto nesting;
                    default:
                    nesting:
                        if (isalnum(i) ||
                            (e->re.nest.type[i] &
                             (REX_NEST_close | REX_NEST_escape | REX_NEST_literal | REX_NEST_quote |
                              REX_NEST_delimiter | REX_NEST_separator | REX_NEST_terminator))) {
                            goto invalid;
                        }
                        e->re.nest.type[i] = REX_NEST_open;
                        if ((x = regcomp_chr(env, &esc)) < 0 ||
                            (e->re.nest.type[x] &
                             (REX_NEST_close | REX_NEST_escape | REX_NEST_delimiter |
                              REX_NEST_separator | REX_NEST_terminator))) {
                            goto invalid;
                        }
                        if (!esc) {
                            if (x == ')' && !--n) {
                                goto invalid;
                            } else if (x == '(') {
                                n++;
                            }
                        }
                        e->re.nest.type[x] |= REX_NEST_close;
                        e->re.nest.type[i] |= x << REX_NEST_SHIFT;
                        continue;
                }
                break;
            }
            env->parnest--;
            if (c == T_PERCENT) {
                for (n = 0; n < 2; n++) {
                    parno = ++env->parno;
                    if (!(f = regcomp_node(env, REX_GROUP, 0, 0, 0))) {
                        regdrop(env->disc, e);
                        return 0;
                    }
                    if (parno < elementsof(env->paren)) env->paren[parno] = f;
                    f->re.group.back = 0;
                    f->re.group.number = parno;
                    f->re.group.expr.rex = e;
                    e = f;
                }
            }
            return e;
        case '(':
            c = 0;
            if (isdigit(*env->cursor)) {
                f = 0;
                do {
                    if (c > (INT_MAX / 10)) {
                        env->error = REG_BADRPT;
                        return NULL;
                    }
                    c = c * 10 + (*env->cursor++ - '0');
                } while (isdigit(*env->cursor));
                if (*env->cursor++ != ')') {
                    env->error = REG_BADRPT;
                    return NULL;
                }
                if (c && env->type >= SRE) c = c * 2 - 1;
                if (!c || c > env->parno || !env->paren[c]) {
                    if (!(env->flags & (REG_LENIENT | REG_REGEXP))) {
                        env->error = REG_ESUBREG;
                        return NULL;
                    }
                    if (c) c = -1;
                }
            } else {
                if (env->type < SRE && *env->cursor++ != '?') {
                    env->error = REG_BADRPT;
                    return NULL;
                }
                f = regcomp_grp(env, parno + 1);
                if (!f && env->error) return NULL;
            }
            e = regcomp_node(env, REX_GROUP_COND, 0, 0, 0);
            if (!e) {
                regdrop(env->disc, f);
                return NULL;
            }
            e->re.group.size = c;
            e->re.group.expr.binary.left = f;
            e->re.group.expr.binary.right = regcomp_alt(env, parno, 1);
            if (!e->re.group.expr.binary.right) {
                regdrop(env->disc, e);
                free(e);
                return NULL;
            }
            if (regcomp_token(env) != T_CLOSE) {
                env->error = REG_EPAREN;
                free(e);
                return NULL;
            }
            eat(env);
            env->parnest--;
            Rex_t *rv = regcomp_rep(env, e, parno, parno);
            if (!rv) free(e);
            return rv;
        case '{':
            p = env->cursor;
            n = 1;
            while ((c = *env->cursor)) {
                if (c == '\\' && *(env->cursor + 1)) {
                    env->cursor++;
                } else if (c == '{') {
                    n++;
                } else if (c == '}' && !--n) {
                    break;
                } else if (c == env->delimiter || c == env->terminator) {
                    break;
                }
                env->cursor++;
            }
            if (c != '}') {
                env->error = REG_EBRACE;
                return 0;
            }
            if (*++env->cursor != ')') {
                env->error = REG_EPAREN;
                return 0;
            }
            env->cursor++;
            env->parnest--;
            if (env->disc->re_version < REG_VERSION_EXEC) {
                env->error = REG_BADRPT;
                return 0;
            }
            if (!env->disc->re_execf) return 0;
            if (!(e = regcomp_node(env, REX_EXEC, 0, 0, 0))) return 0;
            e->re.exec.text = (const char *)p;
            e->re.exec.size = env->cursor - p - 2;
            if (!env->disc->re_compf) {
                e->re.exec.data = 0;
            } else {
                e->re.exec.data =
                    (*env->disc->re_compf)(env->regex, e->re.exec.text, e->re.exec.size, env->disc);
            }
            return e;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            c -= '0';
            while (isdigit(*env->cursor)) {
                if (c > (INT_MAX / 10)) {
                    env->error = REG_ESUBREG;
                    return 0;
                }
                c = c * 10 + *env->cursor++ - '0';
            }
            if (*env->cursor == ')') {
                env->cursor++;
                env->parnest--;
                env->token.len = 1;
                if (c > env->parno || !env->paren[c]) {
                    env->error = REG_ESUBREG;
                    return 0;
                }
                env->paren[c]->re.group.back = 1;
                return regcomp_rep(env, regcomp_node(env, REX_BACK, c, 0, 0), 0, 0);
            }
            /*FALLTHROUGH*/
        default:
            env->error = REG_BADRPT;
            return 0;
    }
    p = env->pattern;
    i = env->type;
    if (x) {
        if (typ >= 0) env->type = typ;
        if (!(e = regcomp_alt(env, parno, 0))) goto nope;
        env->flags = g;
        env->type = i;
    }
    c = regcomp_token(env);
    env->parnest--;
    if (c != T_CLOSE && (c != ')' || !(env->flags & (REG_LITERAL | REG_SHELL)))) {
        env->error = REG_EPAREN;
        goto nope;
    }
    eat(env);
    if (typ >= 0 && beg) env->pattern = env->cursor;
    if (!x) {
        if (typ >= 0) env->type = typ;
        return 0;
    }
    if (!(f = regcomp_node(env, x, 0, 0, 0))) {
        regdrop(env->disc, e);
        goto nope;
    }
    f->re.group.expr.rex = e;
    if (x == REX_GROUP_BEHIND || x == REX_GROUP_BEHIND_NOT) {
        if (regcomp_stats(env, e)) {
            regdrop(env->disc, f);
            if (!env->error) env->error = REG_ECOUNT;
            goto nope;
        }
        f->re.group.size = env->stats.m;
        memset(&env->stats, 0, sizeof(env->stats));
    }
    switch (x) {
        case REX_GROUP:
        case REX_GROUP_CUT:
            f = regcomp_rep(env, f, parno, env->parno);
            break;
    }
    if (f) return f;
nope:
    env->flags = g;
    env->pattern = p;
    env->type = i;
    return 0;
}

static_fn Rex_t *regcomp_seq(Cenv_t *env) {
    Rex_t *e;
    Rex_t *f;
    Token_t tok;
    int c;
    int i;
    int n;
    int x;
    int parno;
    int type;
    wchar_t w;
    regflags_t flags;
    Mbstate_t q;
    unsigned char *s;
    unsigned char *p;
    unsigned char *t;
    unsigned char *u;
    unsigned char buf[256];

    for (;;) {
        s = buf;
        while ((c = regcomp_token(env)) < T_META && s < &buf[sizeof(buf) - env->token.len]) {
            x = c;
            p = env->cursor;
            if (c >= 0) {
                n = 1;
                *s++ = (env->flags & REG_ICASE) ? toupper(c) : c;
            } else if (c == C_ESC || (env->flags & REG_ICASE)) {
                c = (c == C_ESC) ? env->token.lex : mbchar(&w, (char **)&p, MB_LEN_MAX, &env->q);
                if (env->flags & REG_ICASE) c = towupper(c);
                if ((&buf[sizeof(buf)] - s) < MB_CUR_MAX) break;
                mbinit(&q);
                n = mbconv((char *)s, c, &q);
                if (n < 0) {
                    *s++ = c;
                } else if (n) {
                    s += n;
                } else {
                    n = 1;
                    *s++ = 0;
                }
            } else {
                n = env->token.len - env->token.esc;
                for (t = p, u = s + n; s < u; *s++ = *t++) {
                    ;
                }
            }
            eat(env);
        }
        if (c == T_BAD) return NULL;
        if (s > buf) {
            switch (c) {
                case T_STAR:
                case T_PLUS:
                case T_LEFT:
                case T_QUES:
                case T_BANG:
                    if ((s -= n) == buf) {
                        e = NULL;
                    } else {
                        i = s - buf;
                        e = regcomp_node(env, REX_STRING, 0, 0, i);
                        if (!e) return NULL;
                        e->re.string.base = (unsigned char *)e->re.data;
                        memcpy(e->re.string.base, buf, i);
                        e->re.string.size = i;
                    }
                    if (x >= 0) {
                        if (!(f = regcomp_node(env, REX_ONECHAR, 1, 1, 0))) {
                            regdrop(env->disc, e);
                            return NULL;
                        }
                        f->re.onechar = (env->flags & REG_ICASE) ? toupper(x) : x;
                    } else {
                        f = regcomp_node(env, REX_STRING, 0, 0, n);
                        if (!f) {
                            free(e);
                            return NULL;
                        }
                        f->re.string.base = (unsigned char *)f->re.data;
                        memcpy(f->re.string.base, p, n);
                        f->re.string.size = n;
                    }
                    if (!(f = regcomp_rep(env, f, 0, 0)) ||
                        !(f = regcomp_cat(env, f, regcomp_seq(env)))) {
                        regdrop(env->disc, e);
                        return NULL;
                    }
                    if (e) f = regcomp_cat(env, e, f);
                    return f;
                default:
                    c = s - buf;
                    e = regcomp_node(env, REX_STRING, 0, 0, c);
                    if (!e) return NULL;
                    e->re.string.base = (unsigned char *)e->re.data;
                    memcpy(e->re.string.base, buf, c);
                    e->re.string.size = c;
                    return regcomp_cat(env, e, regcomp_seq(env));
            }
        } else if (c > T_BACK) {
            eat(env);
            c -= T_BACK;
            if (c > env->parno || !env->paren[c]) {
                env->error = REG_ESUBREG;
                return NULL;
            }
            env->paren[c]->re.group.back = 1;
            e = regcomp_rep(env, regcomp_node(env, REX_BACK, c, 0, 0), 0, 0);
        } else {
            switch (c) {
                case T_AND:
                case T_CLOSE:
                case T_BAR:
                case T_END:
                    return regcomp_node(env, REX_NULL, 0, 0, 0);
                case T_DOLL:
                    eat(env);
                    e = regcomp_rep(env, regcomp_node(env, REX_END, 0, 0, 0), 0, 0);
                    break;
                case T_CFLX:
                    eat(env);
                    if ((e = regcomp_node(env, REX_BEG, 0, 0, 0)) && (env->flags & REG_EXTENDED)) {
                        e = regcomp_rep(env, e, 0, 0);
                    }
                    break;
                case T_OPEN:
                    tok = env->token;
                    eat(env);
                    flags = env->flags;
                    type = env->type;
                    if (env->token.att) env->flags |= REG_MINIMAL;
                    env->parnest++;
                    if (env->type == KRE) ++env->parno;
                    parno = ++env->parno;
                    if (!(e = regcomp_alt(env, parno + 1, 0))) break;
                    if (e->type == REX_NULL && env->type == ERE &&
                        !(env->flags & (REG_NULL | REG_REGEXP))) {
                        regdrop(env->disc, e);
                        env->error = (*env->cursor == 0 || *env->cursor == env->delimiter ||
                                      *env->cursor == env->terminator)
                                         ? REG_EPAREN
                                         : REG_ENULL;
                        return NULL;
                    }
                    if (regcomp_token(env) != T_CLOSE) {
                        regdrop(env->disc, e);
                        env->error = REG_EPAREN;
                        return NULL;
                    }
                    env->parnest--;
                    eat(env);
                    if (!(f = regcomp_node(env, REX_GROUP, 0, 0, 0))) {
                        regdrop(env->disc, e);
                        return NULL;
                    }
                    if (parno < elementsof(env->paren)) env->paren[parno] = f;
                    f->re.group.back = 0;
                    f->re.group.number = parno;
                    f->re.group.expr.rex = e;
                    if (tok.lex) {
                        tok.push = 1;
                        env->token = tok;
                    }
                    if (!(e = regcomp_rep(env, f, parno, env->parno))) return NULL;
                    if (env->type == KRE) {
                        if (!(f = regcomp_node(env, REX_GROUP, 0, 0, 0))) {
                            regdrop(env->disc, e);
                            return NULL;
                        }
                        if (--parno < elementsof(env->paren)) env->paren[parno] = f;
                        f->re.group.back = 0;
                        f->re.group.number = parno;
                        f->re.group.expr.rex = e;
                        e = f;
                    }
                    env->flags = flags;
                    env->type = type;
                    break;
                case T_GROUP:
                    p = env->cursor;
                    eat(env);
                    flags = env->flags;
                    type = env->type;
                    e = regcomp_grp(env, env->parno + 1);
                    if (!e) {
                        if (env->error) return NULL;
                        if (env->literal == env->pattern && env->literal == p) {
                            env->literal = env->cursor;
                        }
                        continue;
                    }
                    env->flags = flags;
                    env->type = type;
                    break;
                case T_BRA:
                    eat(env);
                    e = regcomp_bra(env);
                    if (e) e = regcomp_rep(env, e, 0, 0);
                    break;
                case T_ALNUM:
                case T_ALNUM_NOT:
                case T_DIGIT:
                case T_DIGIT_NOT:
                case T_SPACE:
                case T_SPACE_NOT:
                    eat(env);
                    e = regcomp_ccl(env, c);
                    if (e) e = regcomp_rep(env, e, 0, 0);
                    break;
                case T_LT:
                    eat(env);
                    e = regcomp_rep(env, regcomp_node(env, REX_WBEG, 0, 0, 0), 0, 0);
                    break;
                case T_GT:
                    eat(env);
                    e = regcomp_rep(env, regcomp_node(env, REX_WEND, 0, 0, 0), 0, 0);
                    break;
                case T_DOT:
                    eat(env);
                    e = regcomp_rep(env, regcomp_node(env, REX_DOT, 1, 1, 0), 0, 0);
                    break;
                case T_DOTSTAR:
                    eat(env);
                    env->token.lex = T_STAR;
                    env->token.push = 1;
                    e = regcomp_rep(env, regcomp_node(env, REX_DOT, 1, 1, 0), 0, 0);
                    break;
                case T_SLASHPLUS:
                    eat(env);
                    env->token.lex = T_PLUS;
                    env->token.push = 1;
                    e = regcomp_node(env, REX_ONECHAR, 1, 1, 0);
                    if (e) {
                        e->re.onechar = '/';
                        e = regcomp_rep(env, e, 0, 0);
                    }
                    break;
                case T_WORD:
                    eat(env);
                    e = regcomp_rep(env, regcomp_node(env, REX_WORD, 0, 0, 0), 0, 0);
                    break;
                case T_WORD_NOT:
                    eat(env);
                    e = regcomp_rep(env, regcomp_node(env, REX_WORD_NOT, 0, 0, 0), 0, 0);
                    break;
                case T_BEG_STR:
                    eat(env);
                    e = regcomp_rep(env, regcomp_node(env, REX_BEG_STR, 0, 0, 0), 0, 0);
                    break;
                case T_END_STR:
                    eat(env);
                    e = regcomp_rep(env, regcomp_node(env, REX_END_STR, 0, 0, 0), 0, 0);
                    break;
                case T_FIN_STR:
                    eat(env);
                    e = regcomp_rep(env, regcomp_node(env, REX_FIN_STR, 0, 0, 0), 0, 0);
                    break;
                default:
                    env->error = REG_BADRPT;
                    return NULL;
            }
        }
        if (e && *env->cursor != 0 && *env->cursor != env->delimiter &&
            *env->cursor != env->terminator) {
            e = regcomp_cat(env, e, regcomp_seq(env));
        }
        return e;
    }
}

static_fn Rex_t *regcomp_con(Cenv_t *env) {
    Rex_t *e;
    Rex_t *f;
    Rex_t *g;

    if (!(e = regcomp_seq(env)) || !(env->flags & REG_AUGMENTED) || regcomp_token(env) != T_AND) {
        return e;
    }
    eat(env);
    if (!(f = regcomp_con(env))) {
        regdrop(env->disc, e);
        return NULL;
    }
    if (!(g = regcomp_node(env, REX_CONJ, 0, 0, 0))) {
        regdrop(env->disc, e);
        regdrop(env->disc, f);
        return NULL;
    }
    g->re.group.expr.binary.left = e;
    g->re.group.expr.binary.right = f;
    return g;
}

static_fn Rex_t *regcomp_alt(Cenv_t *env, int number, int cond) {
    Rex_t *e;
    Rex_t *f;
    Rex_t *g;

    if (!(e = regcomp_con(env))) {
        return 0;
    } else if (regcomp_token(env) != T_BAR) {
        if (!cond) return e;
        f = 0;
        if (e->type == REX_NULL) goto bad;
    } else {
        eat(env);
        if (!(f = regcomp_alt(env, number, 0))) {
            regdrop(env->disc, e);
            return 0;
        }
        if ((e->type == REX_NULL || f->type == REX_NULL) &&
            !(env->flags & (REG_NULL | REG_REGEXP))) {
            goto bad;
        }
        if (!cond && (g = regcomp_trie(env, e, f))) return g;
    }
    if (!(g = regcomp_node(env, REX_ALT, 0, 0, 0))) {
        env->error = REG_ESPACE;
        goto bad;
    }
    g->re.group.number = number;
    g->re.group.last = env->parno;
    g->re.group.expr.binary.left = e;
    g->re.group.expr.binary.right = f;
    return g;
bad:
    regdrop(env->disc, e);
    regdrop(env->disc, f);
    if (!env->error) env->error = REG_ENULL;
    return 0;
}

/*
 * add v to REX_BM tables
 */

static_fn void bmstr(Cenv_t *env, Rex_t *a, unsigned char *v, int n, Bm_mask_t b) {
    UNUSED(env);
    int c;
    int m;
    size_t z;

    for (m = 0; m < n; m++) {
        if (!(z = n - m - 1)) z = HIT;
        c = v[m];
        a->re.bm.mask[m][c] |= b;
        if (z == HIT || !a->re.bm.skip[c] || (a->re.bm.skip[c] > z && a->re.bm.skip[c] < HIT)) {
            a->re.bm.skip[c] = z;
        }
        if (a->flags & REG_ICASE) {
            if (isupper(c)) {
                c = tolower(c);
            } else if (islower(c)) {
                c = toupper(c);
            } else {
                continue;
            }
            a->re.bm.mask[m][c] |= b;
            if (z == HIT || !a->re.bm.skip[c] || (a->re.bm.skip[c] > z && a->re.bm.skip[c] < HIT)) {
                a->re.bm.skip[c] = z;
            }
        }
    }
}

/*
 * set up BM table from trie
 */

static_fn int bmtrie(Cenv_t *env, Rex_t *a, unsigned char *v, Trie_node_t *x, int n, int m,
                     Bm_mask_t b) {
    do {
        v[m] = x->c;
        if (m >= (n - 1)) {
            bmstr(env, a, v, n, b);
            if (!(b <<= 1)) {
                b = 1;
                a->re.bm.complete = 0;
            } else if (x->son) {
                a->re.bm.complete = 0;
            }
        } else if (x->son) {
            b = bmtrie(env, a, v, x->son, n, m + 1, b);
        }
        x = x->sib;
    } while (x);
    return b;
}

/*
 * rewrite the expression tree for some special cases
 * 1. it is a null expression - illegal
 * 2. max length fixed string found -- use BM algorithm
 * 3. it begins with an unanchored string - use KMP algorithm
 * 0 returned on success
 */

static_fn int regcomp_special(Cenv_t *env, regex_t *p) {
    Rex_t *a;
    Rex_t *e;
    Rex_t *t;
    Rex_t *x;
    Rex_t *y;
    unsigned char *s;
    int *f;
    int n;
    int m;
    int k;

    DEBUG_INIT();
    e = p->re_info->rex;
    if (e) {
        x = env->stats.x;
        if (x && x->re.string.size < 3) x = 0;
        t = env->stats.y;
        if (t && t->re.trie.min < 3) t = 0;
        if (x && t) {
            if (x->re.string.size >= t->re.trie.min) {
                t = 0;
            } else {
                x = 0;
            }
        }
        if (x || t) {
            Bm_mask_t **mask;
            Bm_mask_t *h;
            unsigned char *v;
            size_t *q;
            size_t l;
            int i;
            int j;

            if (x) {
                y = x;
                n = m = x->re.string.size;
                l = env->stats.l;
            } else {
                y = t;
                n = t->re.trie.min;
                m = t->re.trie.max;
                l = env->stats.k;
            }
            if (!(q = regalloc(env->disc, 0, (n + 1) * sizeof(size_t)))) return 1;
            if (!(a = regcomp_node(env, REX_BM, 0, 0,
                                   n * (sizeof(Bm_mask_t *) + (UCHAR_MAX + 1) * sizeof(Bm_mask_t)) +
                                       (UCHAR_MAX + n + 2) * sizeof(size_t)))) {
                (void)regalloc(env->disc, q, 0);
                return 1;
            }
            a->flags = y->flags;
            a->map = y->map;
            a->re.bm.size = n;
            a->re.bm.back = (y == e || y == e->re.group.expr.rex) ? (m - n) : -1;
            a->re.bm.left = l - 1;
            a->re.bm.right = env->stats.m - l - n;
            a->re.bm.complete =
                (env->stats.e || (y != e && (e->type != REX_GROUP || y != e->re.group.expr.rex)) ||
                 e->next || ((a->re.bm.left + a->re.bm.right) >= 0))
                    ? 0
                    : n;
            h = (Bm_mask_t *)&a->re.bm.mask[n];
            a->re.bm.skip = (size_t *)(h + n * (UCHAR_MAX + 1));
            a->re.bm.fail = &a->re.bm.skip[UCHAR_MAX + 1];
            for (m = 0; m <= UCHAR_MAX; m++) a->re.bm.skip[m] = n;
            a->re.bm.skip[0] = a->re.bm.skip[env->mappednewline] =
                (y->next && y->next->type == REX_END) ? HIT : (n + a->re.bm.left);
            for (i = 1; i <= n; i++) a->re.bm.fail[i] = 2 * n - i;
            mask = a->re.bm.mask;
            for (m = 0; m < n; m++) {
                mask[m] = h;
                h += UCHAR_MAX + 1;
            }
            if (x) {
                bmstr(env, a, x->re.string.base, n, 1);
            } else {
                v = (unsigned char *)q;
                memset(v, 0, n);
                m = 1;
                for (i = 0; i <= UCHAR_MAX; i++) {
                    if (t->re.trie.root[i]) m = bmtrie(env, a, v, t->re.trie.root[i], n, 0, m);
                }
            }
            mask--;
            memset(q, 0, n * sizeof(*q));
            for (k = (j = n) + 1; j > 0; j--, k--) {
                DEBUG_CODE(0x0010, sfprintf(sfstderr, "BM#0: k=%d j=%d\n", k, j));
                for (q[j] = k; k <= n; k = q[k]) {
                    for (m = 0; m <= UCHAR_MAX; m++) {
                        if (mask[k][m] == mask[j][m]) {
                            DEBUG_CODE(0x0010,
                                       sfprintf(sfstderr, "CUT1: mask[%d][%c]=mask[%d][%c]\n", k, m,
                                                j, m));
                            goto cut;
                        }
                    }
                    DEBUG_CODE(0x0010,
                               sfprintf(sfstderr, "BM#2: fail[%d]=%d => %d\n", k, a->re.bm.fail[k],
                                        (a->re.bm.fail[k] > n - j) ? (n - j) : a->re.bm.fail[k]));
                    if (a->re.bm.fail[k] > n - j) a->re.bm.fail[k] = n - j;
                }
            cut:;
            }
            for (i = 1; i <= n; i++) {
                if (a->re.bm.fail[i] > n + k - i) {
                    DEBUG_CODE(0x0010, sfprintf(sfstderr, "BM#4: fail[%d]=%d => %d\n", i,
                                                a->re.bm.fail[i], n + k - i));
                    a->re.bm.fail[i] = n + k - i;
                }
            }
#if _AST_REGEX_DEBUG
            if (DEBUG_TEST(0x0020, 1, 0)) {
                sfprintf(sfstderr, "STAT: complete=%d n=%d k=%d l=%d r=%d y=%d:%d e=%d:%d\n",
                         a->re.bm.complete, n, k, a->re.bm.left, a->re.bm.right, y->type,
                         y->next ? y->next->type : 0, e->type, e->next ? e->next->type : 0);
                for (m = 0; m < n; m++)
                    for (i = 1; i <= UCHAR_MAX; i++)
                        if (a->re.bm.mask[m][i])
                            sfprintf(sfstderr, "MASK: [%d]['%c'] = %032..2u\n", m, i,
                                     a->re.bm.mask[m][i]);
                for (i = ' '; i <= UCHAR_MAX; i++)
                    if (a->re.bm.skip[i] >= HIT)
                        sfprintf(sfstderr, "SKIP: ['%c'] =   *\n", i);
                    else if (a->re.bm.skip[i] > 0 && a->re.bm.skip[i] < n)
                        sfprintf(sfstderr, "SKIP: ['%c'] = %3d\n", i, a->re.bm.skip[i]);
                for (j = 31; j >= 0; j--) {
                    sfprintf(sfstderr, "      ");
                next:
                    for (m = 0; m < n; m++) {
                        for (i = 0040; i < 0177; i++)
                            if (a->re.bm.mask[m][i] & (1 << j)) {
                                sfprintf(sfstderr, "  %c", i);
                                break;
                            }
                        if (i >= 0177) {
                            if (j > 0) {
                                j--;
                                goto next;
                            }
                            sfprintf(sfstderr, "  ?");
                        }
                    }
                    sfprintf(sfstderr, "\n");
                }
                sfprintf(sfstderr, "FAIL: ");
                for (m = 1; m <= n; m++) sfprintf(sfstderr, "%3d", a->re.bm.fail[m]);
                sfprintf(sfstderr, "\n");
            }
#endif
            (void)regalloc(env->disc, q, 0);
            a->next = e;
            p->re_info->rex = a;
            return 0;
        }
        switch (e->type) {
            case REX_BEG:
                if (env->flags & REG_NEWLINE) return 0;
                break;
            case REX_GROUP:
                if (env->stats.b) return 0;
                e = e->re.group.expr.rex;
                if (e->type != REX_DOT) return 0;
                /*FALLTHROUGH*/
            case REX_DOT:
                if (e->lo == 0 && e->hi == RE_DUP_INF) break;
                return 0;
            case REX_NULL:
                if (env->flags & (REG_NULL | REG_REGEXP)) break;
                env->error = REG_ENULL;
                return 1;
            case REX_STRING:
                if ((env->flags & (REG_LEFT | REG_LITERAL | REG_RIGHT)) || e->map) return 0;
                s = e->re.string.base;
                n = e->re.string.size;
                if (!(a = regcomp_node(env, REX_KMP, 0, 0, n * (sizeof(int *) + 1)))) return 1;
                a->flags = e->flags;
                a->map = e->map;
                f = a->re.string.fail;
                memcpy((char *)(a->re.string.base = (unsigned char *)&f[n]), (char *)s, n);
                s = a->re.string.base;
                a->re.string.size = n;
                f[0] = m = -1;
                for (k = 1; k < n; k++) {
                    while (m >= 0 && s[m + 1] != s[k]) m = f[m];
                    if (s[m + 1] == s[k]) m++;
                    f[k] = m;
                }
                a->next = e->next;
                p->re_info->rex = a;
                e->next = NULL;
                regdrop(env->disc, e);
                break;
            default:
                return 0;
        }
    }
    p->re_info->once = 1;
    return 0;
}

int regcomp(regex_t *p, const char *pattern, regflags_t flags) {
    Rex_t *e;
    Rex_t *f;
    regdisc_t *disc;
    unsigned char *fold;
    int i;
    Cenv_t env;

    if (!p) return REG_BADPAT;
    if (flags & REG_DISCIPLINE) {
        flags &= ~REG_DISCIPLINE;
        disc = p->re_disc;
    } else {
        disc = &regstate.disc;
    }
    if (!disc->re_errorlevel) disc->re_errorlevel = 2;
    p->re_info = NULL;
    if (!pattern) return regfatal(disc, REG_BADPAT, pattern);
    if (!regstate.initialized) {
        regstate.initialized = 1;
        for (i = 0; i < elementsof(regstate.escape); i++) {
            regstate.magic[regstate.escape[i].key] = regstate.escape[i].val;
        }
    }
    fold = lc_ctype_data;
    if (ast.locale.serial != lc_ctype_serial || !fold) {
        fold = calloc(1, UCHAR_MAX + 1);
        if (!fold) return regfatal(disc, REG_ESPACE, pattern);
        for (i = 0; i <= UCHAR_MAX; i++) fold[i] = toupper(i);
        lc_ctype_data = fold;
        lc_ctype_serial = ast.locale.serial;
    }
again:
    if (!(p->re_info = regalloc(disc, 0, sizeof(Env_t)))) {
        return regfatal(disc, REG_ESPACE, pattern);
    }
    memset(p->re_info, 0, sizeof(*p->re_info));
    if (!(p->re_info->mst = stkopen(STK_SMALL | STK_NULL))) {
        return regfatal(disc, REG_ESPACE, pattern);
    }
    memset(&env, 0, sizeof(env));
    env.regex = p;
    env.flags = flags;
    env.disc = p->re_info->disc = disc;
    if (env.flags & REG_AUGMENTED) env.flags |= REG_EXTENDED;
    env.mappeddot = '.';
    env.mappednewline = '\n';
    env.mappedslash = '/';
    if (disc->re_version >= REG_VERSION_MAP && disc->re_map) {
        env.map = disc->re_map;
        env.MAP = p->re_info->fold;
        for (i = 0; i <= UCHAR_MAX; i++) {
            env.MAP[i] = fold[env.map[i]];
            if (env.map[i] == '.') env.mappeddot = i;
            if (env.map[i] == '\n') env.mappednewline = i;
            if (env.map[i] == '/') env.mappedslash = i;
        }
    } else {
        env.MAP = fold;
    }
    env.type = (env.flags & REG_AUGMENTED) ? ARE : (env.flags & REG_EXTENDED) ? ERE : BRE;
    env.explicit = -1;
    if (env.flags & REG_SHELL) {
        if (env.flags & REG_SHELL_PATH) env.explicit = env.mappedslash;
        if (!(env.flags & REG_SHELL_ESCAPED)) env.flags |= REG_CLASS_ESCAPE;
        env.flags |= REG_LENIENT | REG_NULL;
        env.type = env.type == BRE ? SRE : KRE;
    } else {
        env.flags &= ~(REG_SHELL_DOT | REG_SHELL_ESCAPED | REG_SHELL_GROUP | REG_SHELL_PATH);
    }
    if ((env.flags & (REG_NEWLINE | REG_SPAN)) == REG_NEWLINE) env.explicit = env.mappednewline;
    p->re_info->leading = (env.flags & REG_SHELL_DOT) ? env.mappeddot : -1;
    env.posixkludge = !(env.flags & (REG_EXTENDED | REG_SHELL));
    env.token.lex = 0;
    env.token.push = 0;
    if (env.flags & REG_DELIMITED) {
        switch (env.delimiter = *pattern++) {
            case 0:
            case '\\':
            case '\n':
            case '\r':
                env.error = REG_EDELIM;
                goto bad;
        }
        env.terminator = '\n';
    }
    env.literal = env.pattern = env.cursor = (unsigned char *)pattern;
    if (!(p->re_info->rex = regcomp_alt(&env, 1, 0))) goto bad;
    if (env.parnest) {
        env.error = REG_EPAREN;
        goto bad;
    }
    if ((env.flags & REG_LEFT) && p->re_info->rex->type != REX_BEG) {
        if (p->re_info->rex->type == REX_ALT) env.flags &= ~REG_FIRST;
        if (!(e = regcomp_node(&env, REX_BEG, 0, 0, 0))) {
            regfree(p);
            return regfatal(disc, REG_ESPACE, pattern);
        }
        e->next = p->re_info->rex;
        p->re_info->rex = e;
        p->re_info->once = 1;
    }
    for (e = p->re_info->rex; e->next; e = e->next) {
        ;
    }
    p->re_info->done.type = REX_DONE;
    p->re_info->done.flags = e->flags;
    if ((env.flags & REG_RIGHT) && e->type != REX_END) {
        if (p->re_info->rex->type == REX_ALT) env.flags &= ~REG_FIRST;
        if (!(f = regcomp_node(&env, REX_END, 0, 0, 0))) {
            regfree(p);
            return regfatal(disc, REG_ESPACE, pattern);
        }
        f->flags = e->flags;
        f->map = e->map;
        e->next = f;
    }
    if (regcomp_stats(&env, p->re_info->rex)) {
        if (!env.error) env.error = REG_ECOUNT;
        goto bad;
    }
    if (env.stats.b) {
        p->re_info->hard = p->re_info->separate = 1;
    } else if (!(env.flags & REG_FIRST) &&
               (env.stats.a || (env.stats.c > 1 && env.stats.c != env.stats.s) ||
                (env.stats.t && (env.stats.t > 1 || env.stats.a || env.stats.c)))) {
        p->re_info->hard = 1;
    }
    if (p->re_info->hard || env.stats.c || env.stats.i) {
        p->re_info->stats.re_min = p->re_info->stats.re_max = -1;
    } else {
        if (!(p->re_info->stats.re_min = env.stats.m)) p->re_info->stats.re_min = -1;
        if (!(p->re_info->stats.re_max = env.stats.n)) p->re_info->stats.re_max = -1;
    }
    if (regcomp_special(&env, p)) goto bad;
    regcomp_serialize(&env, p->re_info->rex, 1);
    p->re_nsub = env.stats.p;
    if (env.type == KRE) p->re_nsub /= 2;
    if (env.flags & REG_DELIMITED) {
        p->re_npat = env.cursor - (unsigned char *)pattern + 1;
        if (*env.cursor == env.delimiter) {
            p->re_npat++;
        } else if (env.flags & REG_MUSTDELIM) {
            env.error = REG_EDELIM;
            goto bad;
        } else {
            env.flags &= ~REG_DELIMITED;
        }
    }
    p->re_info->explicit = env.explicit;
    p->re_info->flags = env.flags & REG_COMP;
    p->re_info->min = env.stats.m;
    p->re_info->nsub = env.stats.p + env.stats.u;
    return 0;

bad:
    regfree(p);
    if (!env.error) env.error = REG_ESPACE;
    if (env.type >= SRE && env.error != REG_ESPACE && !(flags & REG_LITERAL)) {
        flags |= REG_LITERAL;
        pattern = (const char *)env.literal;
        goto again;
    }
    return regfatal(disc, env.error, pattern);
}
