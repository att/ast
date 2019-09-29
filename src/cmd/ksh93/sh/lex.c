/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2014 AT&T Intellectual Property          *
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
// KornShell  lexical analyzer
//
// Written by David Korn
// AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wctype.h>

#include "argnod.h"
#include "ast.h"
#include "ast_assert.h"
#include "defs.h"
#include "error.h"
#include "fcin.h"
#include "history.h"
#include "lexstates.h"
#include "name.h"
#include "sfio.h"
#include "shlex.h"
#include "shnodes.h"
#include "shtable.h"
#include "stk.h"
#include "test.h"

#define TEST_RE 3
#define SYNBAD 3       // exit value for syntax errors
#define STACK_ARRAY 3  // size of depth match stack growth

#define oldmode(lp) (lp->lexd.lastc >> CHAR_BIT)
#define endchar(lp) (lp->lexd.lastc & 0xff)

static_fn char *fmttoken(Lex_t *, int, char *);
#ifdef SF_BUFCONST
static_fn int alias_exceptf(Sfio_t *, int, void *, Sfdisc_t *);
#else
static_fn int alias_exceptf(Sfio_t *, int, Sfdisc_t *);
#endif
static_fn void setupalias(Lex_t *, const char *, Namval_t *);
static_fn int comsub(Lex_t *, int);
static_fn void nested_here(Lex_t *);
static_fn int here_copy(Lex_t *, struct ionod *);
static_fn void stack_grow(Lex_t *);
static const Sfdisc_t alias_disc = {NULL, NULL, NULL, alias_exceptf, NULL};

static inline void setchar(Lex_t *lp, int c) { lp->lexd.lastc = (lp->lexd.lastc & ~0xff) | c; }

static inline void poplevel(Lex_t *lp) {
    assert(lp->lexd.level > 0);
    lp->lexd.lastc = lp->lexd.lex_match[--lp->lexd.level];
}

static inline void pushlevel(Lex_t *lp, int c, int s) {
    if (lp->lexd.level >= lp->lexd.lex_max) stack_grow(lp);
    lp->lexd.lex_match[lp->lexd.level++] = lp->lexd.lastc;
    lp->lexd.lastc = (s << CHAR_BIT) | c;
}

static_fn void refvar(Lex_t *lp, int type) {
    Shell_t *shp = lp->sh;
    Stk_t *stkp = shp->stk;
    off_t off = 0;
    unsigned long r;

    if (lp->lexd.first) {
        off = (fcseek(0) - (type + 1)) - lp->lexd.first;
        r = kiaentity(lp, lp->lexd.first + lp->lexd.kiaoff + type, off - lp->lexd.kiaoff, 'v', -1,
                      -1, lp->current, 'v', 0, "");
    } else {
        int n, offset = stktell(stkp);
        char *savptr, *begin;
        off = offset + (fcseek(0) - (type + 1)) - fcfirst();
        if (lp->lexd.kiaoff < offset) {
            // Variable starts on stak, copy remainder.
            if (off > offset) sfwrite(stkp, fcfirst() + type, off - offset);
            n = stktell(stkp) - lp->lexd.kiaoff;
            begin = stkptr(stkp, lp->lexd.kiaoff);
        } else {
            // Variable in data buffer.
            begin = fcfirst() + (type + lp->lexd.kiaoff - offset);
            n = off - lp->lexd.kiaoff;
        }
        savptr = stkfreeze(stkp, 0);
        r = kiaentity(lp, begin, n, 'v', -1, -1, lp->current, 'v', 0, "");
        stkset(stkp, savptr, offset);
    }
    sfprintf(lp->kiatmp, "p;%..64d;v;%..64d;%d;%d;r;\n", lp->current, r, shp->inlineno,
             shp->inlineno);
}

//
// This routine gets called when reading across a buffer boundary.
// If lexd.nocopy is off, then current token is saved on the stack.
//
static_fn void lex_advance(Sfio_t *iop, const char *buff, int size, void *context) {
    Lex_t *lp = (Lex_t *)context;
    Shell_t *shp = lp->sh;
    Sfio_t *log = shp->funlog;
    Stk_t *stkp = shp->stk;

    // Write to history file and to stderr if necessary.
    if (!sfstacked(iop)) {
        if (sh_isstate(shp, SH_HISTORY) && shp->gd->hist_ptr) log = shp->gd->hist_ptr->histfp;
        sfwrite(log, buff, size);
        if (sh_isstate(shp, SH_VERBOSE)) sfwrite(sfstderr, buff, size);
    }
    if (lp->lexd.nocopy) return;
    if (lp->lexd.dolparen && lp->lexd.docword && lp->lexd.docend) {
        int n = size - (lp->lexd.docend - (char *)buff);
        sfwrite(shp->strbuf, lp->lexd.docend, n);
        lp->lexd.docextra += n;
        if (sffileno(iop) >= 0) {
            lp->lexd.docend = sfgetbuf(iop);
        } else {
            lp->lexd.docend = fcfirst();
        }
    }
    if (lp->lexd.first) {
        size -= (lp->lexd.first - (char *)buff);
        buff = lp->lexd.first;
        if (!lp->lexd.noarg) lp->arg = stkseek(stkp, ARGVAL);
        lp->lexd.kiaoff += ARGVAL;
    }
    if (size > 0 && (lp->arg || lp->lexd.noarg)) {
        sfwrite(stkp, buff, size);
        lp->lexd.first = NULL;
    }
}

//
// Fill up another input buffer. Preserves lexical state.
//
static_fn int lexfill(Lex_t *lp) {
    int c;
    Lex_t savelex;
    struct argnod *ap;
    int aok, docextra;

    savelex = *lp;
    ap = lp->arg;
    c = fcfill();
    if (ap) lp->arg = ap;
    docextra = lp->lexd.docextra;
    lp->lex = savelex.lex;
    lp->lexd = savelex.lexd;
    if (fcfile() || c) lp->lexd.first = NULL;
    aok = lp->aliasok;
    ap = lp->arg;
    memcpy(lp, &savelex, offsetof(Lex_t, lexd));
    lp->arg = ap;
    lp->aliasok = aok;
    if (lp->lexd.docword && docextra) {
        lp->lexd.docextra = docextra;
        lp->lexd.docend = fcseek(0) - 1;
    }
    return c;
}

//
// Pass mode=1 for reinitialization.
//
Lex_t *sh_lexopen(Lex_t *lp, Shell_t *shp, int mode) {
    if (!lp) {
        lp = calloc(1, sizeof(Lex_t));
        lp->sh = shp;
        shp->lexsize = sizeof(Lex_t);
    }
    fcnotify(lex_advance, lp);
    lp->lex.intest = lp->lex.incase = lp->lex.skipword = lp->lexd.warn = 0;
    lp->comp_assign = 0;
    lp->lex.reservok = 1;
    if (!sh_isoption(shp, SH_DICTIONARY) && sh_isoption(shp, SH_NOEXEC)) lp->lexd.warn = 1;
    if (!mode) {
        lp->lexd.noarg = lp->lexd.level = lp->lexd.dolparen = lp->lexd.balance = 0;
        lp->lexd.nocopy = lp->lexd.docword = lp->lexd.nest = lp->lexd.paren = 0;
        lp->lexd.lex_state = lp->lexd.lastc = 0;
        lp->lexd.docend = NULL;
        lp->lexd.nested_tilde = 0;
    }
    lp->comsub = 0;
    return lp;
}

#ifdef DBUG
extern int lextoken(Lex_t *);
int sh_lex(Lex_t *lp) {
    Shell_t *shp = lp->sh;
    int flag;
    char *quoted, *macro, *split, *expand;
    char tokstr[3];

    int tok = lextoken(lp);
    quoted = macro = split = expand = "";
    if (tok == 0 && (flag = lp->arg->argflag)) {
        if (flag & ARG_MAC) macro = "macro:";
        if (flag & ARG_EXP) expand = "expand:";
        if (flag & ARG_QUOTED) quoted = "quoted:";
    }
    sfprintf(sfstderr, "%d: line %d: %o:%s%s%s%s %s\n", getpid(), shp->inlineno, tok, quoted, macro,
             split, expand, fmttoken(lp, tok, tokstr));
    return tok;
}
#define sh_lex lextoken
#endif  // DBUG

//
// Get the next word and put it on the top of the stak.
// A pointer to the current word is stored in lp->arg.
// Returns the token type.
//
int sh_lex(Lex_t *lp) {
    Shell_t *shp = lp->sh;
    const char *state;
    int n, c, mode = ST_BEGIN, wordflags = 0;
    Stk_t *stkp = shp->stk;
    int inlevel = lp->lexd.level, assignment = 0, ingrave = 0;
    int epatchar = 0;
    Sfio_t *sp;

    LEN = 1;
    if (lp->lexd.paren) {
        lp->lexd.paren = 0;
        lp->token = LPAREN;
        return lp->token;
    }
    if (lp->noreserv) {
        lp->lex.reservok = 0;
        c = fcgetc();
        while (c == ' ' || c == '\t' || c == '\n') {
            c = fcgetc();
        }
        fcseek(-LEN);
        if (c == '[') lp->assignok = SH_ASSIGN;
    }
    if (lp->lex.incase) {
        lp->assignok = 0;
    } else {
        lp->assignok |= lp->lex.reservok;
    }
    if (lp->comp_assign == 2) lp->comp_assign = lp->lex.reservok = 0;
    lp->lexd.arith = (lp->lexd.nest == 1);
    if (lp->lexd.nest) {
        pushlevel(lp, lp->lexd.nest, ST_NONE);
        lp->lexd.nest = 0;
        mode = lp->lexd.lex_state;
    } else if (lp->lexd.docword) {
        if ((c = fcgetc()) == '-' || c == '#') {
            lp->lexd.docword++;
            lp->digits = (c == '#' ? 3 : 1);
        } else if (c == '<') {
            lp->digits = 2;
            lp->lexd.docword = 0;
        } else if (c > 0) {
            fcseek(-LEN);
        }
    }
    if (!lp->lexd.dolparen) {
        lp->arg = NULL;
        if (mode != ST_BEGIN) {
            lp->lexd.first = fcseek(0);
        } else {
            lp->lexd.first = NULL;
        }
    }
    lp->lastline = lp->sh->inlineno;
    lp->typed = 0;

    while (1) {
        // Skip over characters in the current state.
        state = sh_lexstates[mode];
        while ((n = STATE(state, c)) == 0) {
            ;  // empty loop
        }

        switch (n) {
            case S_BREAK: {
                if (lp->lex.incase > TEST_RE && mode == ST_NORM && c == LPAREN) {
                    pushlevel(lp, RPAREN, mode);
                    mode = ST_NESTED;
                    continue;
                }
                fcseek(-LEN);
                goto breakloop;
            }
            case S_EOF: {
                sp = fcfile();
                if ((n = lexfill(lp)) > 0) {
                    fcseek(-1);
                    continue;
                }
                // Check for zero byte in file.
                if (n == 0 && fcfile()) {
                    if (shp->readscript) {
                        char *cp = error_info.id;
                        errno = ENOEXEC;
                        error_info.id = shp->readscript;
                        errormsg(SH_DICT, ERROR_system(ERROR_NOEXEC), e_exec, cp);
                        __builtin_unreachable();
                    } else {
                        lp->token = -1;
                        sh_syntax(lp);
                    }
                }
                // End-of-file.
                if (mode == ST_BEGIN) {
                    lp->token = EOFSYM;
                    return EOFSYM;
                }
                if (mode <= ST_NORM) goto breakloop;
                if (lp->lexd.level <= 0) goto breakloop;
                switch (c = endchar(lp)) {
                    case '$': {
                        if (mode == ST_LIT) {
                            c = '\'';
                            break;
                        }
                        mode = oldmode(lp);
                        poplevel(lp);
                        continue;
                    }
                    case RBRACT: {
                        c = LBRACT;
                        break;
                    }
                    case 1:  // for ((...))
                    case RPAREN: {
                        c = LPAREN;
                        break;
                    }
                    case '"':
                    case '`':
                    case '\'': {
                        lp->lexd.balance = c;
                        break;
                    }
                    default: {
                        c = LBRACE;
                        break;
                    }
                }
                if (sp && !(sfset(sp, 0, 0) & SF_STRING)) {
                    lp->lasttok = c;
                    lp->token = EOFSYM;
                    sh_syntax(lp);
                }
                lp->lexd.balance = c;
                goto breakloop;
            }
            case S_COM: {
                // Skip one or more comment line(s).
                lp->lex.reservok = !lp->lex.intest;
                if ((n = lp->lexd.nocopy) && lp->lexd.dolparen) lp->lexd.nocopy--;
                do {
                    while ((c = fcgetc()) > 0 && c != '\n') {
                        ;  // empty loop
                    }
                    if (c <= 0 || lp->heredoc) {
                        shp->inlineno++;
                        break;
                    }
                    while (shp->inlineno++, fcpeek(0) == '\n') fcseek(1);
                    while (state[c = fcpeek(0)] == 0) fcseek(1);
                } while (c == '#');
                lp->lexd.nocopy = n;
                if (c < 0) {
                    lp->token = EOFSYM;
                    return EOFSYM;
                }
                n = S_NLTOK;
                shp->inlineno--;
            }
            // FALLTHRU
            case S_NLTOK: {
                // Check for here-document.
                if (lp->heredoc) {
                    if (!lp->lexd.dolparen) lp->lexd.nocopy++;
                    c = shp->inlineno;
                    if (here_copy(lp, lp->heredoc) <= 0 && lp->lasttok) {
                        lp->lasttok = IODOCSYM;
                        lp->token = EOFSYM;
                        lp->lastline = c;
                        sh_syntax(lp);
                    }
                    if (!lp->lexd.dolparen) lp->lexd.nocopy--;
                    lp->heredoc = NULL;
                }
                lp->lex.reservok = !lp->lex.intest;
                lp->lex.skipword = 0;
            }
            // FALLTHRU
            case S_NL: {
                // Skip over new-lines.
                lp->lex.last_quote = 0;
                while (shp->inlineno++, fcget() == '\n') {
                    ;  // empty loop
                }
                fcseek(-LEN);
                if (n == S_NLTOK) {
                    lp->comp_assign = 0;
                    lp->token = '\n';
                    return lp->token;
                }
            }
            // FALLTHRU
            case S_BLNK: {
                if (lp->lex.incase <= TEST_RE) continue;
                // Implicit RPAREN for =~ test operator.
                if (inlevel + 1 == lp->lexd.level) {
                    if (lp->lex.intest) fcseek(-LEN);
                    c = RPAREN;
                    goto do_pop;
                }
                continue;
            }
            case S_OP: {
                // Return operator token.
                if (c == '<' || c == '>') {
                    if (lp->lex.testop2) {
                        lp->lex.testop2 = 0;
                    } else {
                        lp->digits = (c == '>');
                        lp->lex.skipword = 1;
                        lp->aliasok = lp->lex.reservok;
                        if (lp->lex.incase < 2) lp->lex.reservok = 0;
                    }
                } else {
                    lp->lex.reservok = !lp->lex.intest;
                    if (c == RPAREN) {
                        if (!lp->lexd.dolparen) lp->lex.incase = 0;
                        lp->token = c;
                        return lp->token;
                    }
                    lp->lex.testop1 = lp->lex.intest;
                }
                n = fcgetc();
                if (n > 0) fcseek(-LEN);
                if (state[n] == S_OP || n == '#') {
                    if (n == c) {
                        if (c == '<') {
                            lp->lexd.docword = 1;
                        } else if (n == LPAREN) {
                            if (lp->lex.intest || lp->comp_assign) return c;
                            lp->lexd.nest = 1;
                            lp->lastline = shp->inlineno;
                            lp->lexd.lex_state = ST_NESTED;
                            fcseek(1);
                            return sh_lex(lp);
                        }
                        c |= SYMREP;
                    } else if (c == '(' || c == ')') {
                        lp->token = c;
                        return c;
                    } else if (c == '&') {
                        if (!sh_isoption(lp->sh, SH_POSIX) && n == '>' &&
                            (sh_isoption(lp->sh, SH_BASH) || sh_isstate(lp->sh, SH_PROFILE))) {
                            if (!sh_isoption(lp->sh, SH_BASH) && !lp->nonstandard) {
                                lp->nonstandard = 1;
                                errormsg(SH_DICT, ERROR_warn(0), e_lexnonstandard, shp->inlineno);
                            }
                            lp->digits = -1;
                            c = '>';
                        } else if (n == '|') {
                            c |= SYMPIPE;
                        } else {
                            n = 0;
                        }
                    } else if (n == '&') {
                        c |= SYMAMP;
                    } else if (c != '<' && c != '>') {
                        n = 0;
                    } else if (n == LPAREN) {
                        c |= SYMLPAR;
                        lp->lex.reservok = 1;
                        lp->lex.skipword = 0;
                    } else if (n == '|') {
                        c |= SYMPIPE;
                    } else if (c == '<' && n == '>') {
                        lp->digits = 0;
                        c = IORDWRSYM;
                        (void)fcgetc();
                        n = fcgetc();  // yes, we call fcgetc() twice (no idea why)
                        if (n == ';') {
                            lp->token = c = IORDWRSYMT;
                            if (lp->inexec) sh_syntax(lp);
                        } else if (n > 0) {
                            fcseek(-LEN);
                        }
                        n = 0;
                    } else if (n == '#' && (c == '<' || c == '>')) {
                        c |= SYMSHARP;
                    } else if (n == ';' && c == '>') {
                        c |= SYMSEMI;
                        if (lp->inexec) {
                            lp->token = c;
                            sh_syntax(lp);
                        }
                    } else {
                        n = 0;
                    }
                    if (n) {
                        fcseek(1);
                        lp->lex.incase = (c == BREAKCASESYM || c == FALLTHRUSYM);
                    } else {
                        if (lp->lexd.warn && (n = fcpeek(0)) != RPAREN && n != ' ' && n != '\t') {
                            errormsg(SH_DICT, ERROR_warn(0), e_lexspace, shp->inlineno, c, n);
                        }
                    }
                }
                if (c == LPAREN && lp->comp_assign && !lp->lex.intest && !lp->lex.incase) {
                    lp->comp_assign = 2;
                } else {
                    lp->comp_assign = 0;
                }
                lp->token = c;
                return lp->token;
            }
            case S_ESC: {
                // Check for \<new-line>.
                n = fcgetc();
                c = 2;
                if (n == '\n') {
                    Sfio_t *sp;
                    struct argnod *ap;
                    shp->inlineno++;
                    // Synchronize.
                    if (!(sp = fcfile())) state = fcseek(0);
                    fcclose();
                    ap = lp->arg;
                    if (sp) {
                        fcfopen(sp);
                    } else {
                        fcsopen((char *)state);
                    }
                    // Remove \new-line.
                    n = stktell(stkp) - c;
                    stkseek(stkp, n);
                    lp->arg = ap;
                    if (n <= ARGVAL) {
                        mode = 0;
                        lp->lexd.first = NULL;
                    }
                    continue;
                }
                wordflags |= ARG_QUOTED;
                if (mode == ST_DOL) goto err;
#ifndef STR_MAXIMAL
                else if (mode == ST_NESTED && lp->lexd.warn && endchar(lp) == RBRACE &&
                         sh_lexstates[ST_DOL][n] == S_DIG) {
                    errormsg(SH_DICT, ERROR_warn(0), e_lexfuture, shp->inlineno, n);
                }
#endif /* STR_MAXIMAL */
                break;
            }
            case S_NAME: {
                if (!lp->lex.skipword) lp->lex.reservok *= 2;
            }
            // FALLTHRU
            case S_TILDE: {
                if (c == '~' && mode == ST_NESTED) {
                    if (endchar(lp) == RBRACE) {
                        lp->lexd.nested_tilde++;
                        goto tilde;
                    }
                    continue;
                }
            }
            // FALLTHRU
            case S_RES: {
                if (!lp->lexd.dolparen) {
                    lp->lexd.first = fcseek(0) - LEN;
                } else if (lp->lexd.docword) {
                    lp->lexd.docend = fcseek(0) - LEN;
                }
                mode = ST_NAME;
                if (c == '.') fcseek(-LEN);
                if (n != S_TILDE) continue;
            tilde:
                n = fcgetc();
                if (n > 0) {
                    if (c == '~' && n == LPAREN) {
                        if (lp->lexd.nested_tilde) {
                            lp->lexd.nested_tilde++;
                        } else if (lp->lex.incase) {
                            lp->lex.incase = TEST_RE;
                        }
                    }
                    fcseek(-LEN);
                    if (lp->lexd.nested_tilde) {
                        lp->lexd.nested_tilde--;
                        continue;
                    }
                }
                if (n == LPAREN) goto epat;
                wordflags = ARG_MAC;
                mode = ST_NORM;
                continue;
            }
            case S_REG: {
                if (mode == ST_BEGIN) {
                do_reg:
                    // skip new-line joining.
                    if (c == '\\' && fcpeek(0) == '\n') {
                        shp->inlineno++;
                        fcseek(1);
                        continue;
                    }
                    fcseek(-LEN);
                    if (!lp->lexd.dolparen) {
                        lp->lexd.first = fcseek(0);
                    } else if (lp->lexd.docword) {
                        lp->lexd.docend = fcseek(0);
                    }
                    if (c == '[' && lp->assignok >= SH_ASSIGN) {
                        mode = ST_NAME;
                        continue;
                    }
                }
                mode = ST_NORM;
                continue;
            }
            case S_LIT: {
                if (oldmode(lp) == ST_NONE && !lp->lexd.noarg) {  //  in ((...))
                    if ((c = fcpeek(0)) == LPAREN || c == RPAREN || c == '$' || c == LBRACE ||
                        c == RBRACE || c == '[' || c == ']') {
                        if (fcpeek(1) == '\'') fcseek(2);
                    }
                    continue;
                }
                wordflags |= ARG_QUOTED;
                if (mode == ST_DOL) {
                    if (endchar(lp) != '$') goto err;
                    if (oldmode(lp) == ST_QUOTE) {  // $' within "" or ``
                        if (lp->lexd.warn) {
                            errormsg(SH_DICT, ERROR_warn(0), e_lexslash, shp->inlineno);
                        }
                        mode = ST_LIT;
                    }
                }
                if (mode != ST_LIT) {
                    if (lp->lexd.warn && lp->lex.last_quote && shp->inlineno > lp->lastline &&
                        fcpeek(-2) != '$') {
                        errormsg(SH_DICT, ERROR_warn(0), e_lexlongquote, lp->lastline,
                                 lp->lex.last_quote);
                    }
                    lp->lex.last_quote = 0;
                    lp->lastline = shp->inlineno;
                    if (mode != ST_DOL) pushlevel(lp, '\'', mode);
                    mode = ST_LIT;
                    continue;
                } else if (shp->inlineno >
                           lp->lastline) {  // check for multi-line single-quoted str
                    lp->lex.last_quote = '\'';
                }
                mode = oldmode(lp);
                poplevel(lp);
                break;
            }
            case S_ESC2: {
                // Inside ''.
                if (endchar(lp) == '$') {
                    n = fcgetc();
                    if (n == '\n') shp->inlineno++;
                }
                continue;
            }
            case S_GRAVE: {
                if (lp->lexd.warn && (mode != ST_QUOTE || endchar(lp) != '`')) {
                    errormsg(SH_DICT, ERROR_warn(0), e_lexobsolete1, shp->inlineno);
                }
                wordflags |= (ARG_MAC | ARG_EXP);
                if (mode == ST_QUOTE) ingrave = !ingrave;
            }
            // FALLTHRU
            case S_QUOTE: {
                if (oldmode(lp) == ST_NONE && lp->lexd.arith) {  // in ((...))
                    if (n != S_GRAVE || fcpeek(0) == '\'') continue;
                }
                if (n == S_QUOTE) wordflags |= ARG_QUOTED;
                if (mode != ST_QUOTE) {
                    if (c != '"' || mode != ST_QNEST) {
                        if (lp->lexd.warn && lp->lex.last_quote && shp->inlineno > lp->lastline) {
                            errormsg(SH_DICT, ERROR_warn(0), e_lexlongquote, lp->lastline,
                                     lp->lex.last_quote);
                        }
                        lp->lex.last_quote = 0;
                        lp->lastline = shp->inlineno;
                        pushlevel(lp, c, mode);
                    }
                    ingrave ^= (c == '`');
                    mode = ST_QUOTE;
                    continue;
                } else if ((n = endchar(lp)) == c) {
                    if (shp->inlineno > lp->lastline) lp->lex.last_quote = c;
                    mode = oldmode(lp);
                    poplevel(lp);
                } else if (c == '"' && n == RBRACE) {
                    mode = ST_QNEST;
                }
                break;
            }
            case S_DOL: {
                // Don't check syntax inside ``.
                if (mode == ST_QUOTE && ingrave) continue;
                if (lp->lexd.first) {
                    lp->lexd.kiaoff = fcseek(0) - lp->lexd.first;
                } else {
                    lp->lexd.kiaoff = stktell(stkp) + fcseek(0) - fcfirst();
                }
                pushlevel(lp, '$', mode);
                mode = ST_DOL;
                continue;
            }
            case S_PAR: {
            do_comsub:
                wordflags |= ARG_MAC;
                mode = oldmode(lp);
                poplevel(lp);
                fcseek(-LEN);
                n = lp->digits;
                wordflags |= comsub(lp, c);
                lp->digits = n;
                continue;
            }
            case S_RBRA: {
                if ((n = endchar(lp)) == '$') goto err;
                if (mode != ST_QUOTE || n == RBRACE) {
                    mode = oldmode(lp);
                    poplevel(lp);
                }
                break;
            }
            case S_EDOL: {
                // End $identifier.
                if (lp->kiafile) refvar(lp, 0);
                if (lp->lexd.warn && c == LBRACT && !lp->lex.intest && !lp->lexd.arith &&
                    oldmode(lp) != ST_NESTED) {
                    errormsg(SH_DICT, ERROR_warn(0), e_lexusebrace, shp->inlineno);
                }
                fcseek(-LEN);
                mode = oldmode(lp);
                poplevel(lp);
                break;
            }
            case S_DOT: {
                // Make sure next character is alpha.
                n = fcgetc();
                if (n > 0) {
                    if (n == '.') n = fcgetc();
                    if (n > 0) fcseek(-LEN);
                }
                if (isaletter(n) || n == LBRACT) continue;
                if (mode == ST_NAME) {
                    if (n == '=') continue;
                    break;
                } else if (n == RBRACE) {
                    continue;
                }
                if (isastchar(n)) continue;
                goto err;
            }
            case S_SPC1: {
                wordflags |= ARG_MAC;
                if (endchar(lp) == RBRACE) {
                    setchar(lp, c);
                    continue;
                }
            }
            // FALLTHRU
            case S_ALP: {
                if (c == '.' && endchar(lp) == '$') goto err;
            }
            // FALLTHRU
            case S_SPC2: {
#if SHOPT_BASH
                if (c == '=' && (lp->lexd.warn || !sh_isoption(shp, SH_BASH))) {
                    lp->token = c;
                    sh_syntax(lp);
                }
#endif
            }
            // FALLTHRU
            case S_DIG: {
                wordflags |= ARG_MAC;
                switch (endchar(lp)) {
                    case '$': {
                        if (n == S_ALP) {  // $identifier
                            mode = ST_DOLNAME;
                        } else {
                            mode = oldmode(lp);
                            poplevel(lp);
                        }
                        break;
                    }
                    case '@':
                    case '!': {
                        if (n != S_ALP && n != S_DIG) goto dolerr;
                    }
                    // FALLTHRU
                    case '#': {
                        if (c == '#') n = S_ALP;
                    }
                    // FALLTHRU
                    case RBRACE: {
                        if (n == S_ALP) {
                            setchar(lp, RBRACE);
                            if (c == '.') fcseek(-LEN);
                            mode = ST_BRACE;
                        } else {
                            c = fcgetc();
                            if (c > 0) fcseek(-LEN);
                            if (state[c] == S_ALP) goto err;
                            if (n == S_DIG) {
                                setchar(lp, '0');
                            } else {
                                setchar(lp, '!');
                            }
                        }
                        break;
                    }
                    case '0': {
                        if (n == S_DIG) break;
                        goto dolerr;
                    }
                    default: { goto dolerr; }
                }
                break;
            }
            dolerr:
            case S_ERR: {
                if ((n = endchar(lp)) == '$') goto err;
                if (c == '*' || ((n = sh_lexstates[ST_BRACE][c]) != S_MOD1 && n != S_MOD2)) {
                    n = endchar(lp);
                    if (n != '`') goto err;  // not inside `...`
                    mode = oldmode(lp);
                    poplevel(lp);
                    pushlevel(lp, RBRACE, mode);
                } else {
                    setchar(lp, RBRACE);
                }
                mode = ST_NESTED;
                continue;
            }
            case S_MOD1: {
                if (oldmode(lp) == ST_QUOTE || oldmode(lp) == ST_NONE) {
                    // Allow ' inside "${...}".
                    if (c == ':' && (n = fcgetc()) > 0) {
                        n = state[n];
                        fcseek(-LEN);
                    }
                    if (n == S_MOD1) {
                        mode = ST_QUOTE;
                        continue;
                    }
                }
            }
            // FALLTHRU
            case S_MOD2: {
                if (lp->kiafile) refvar(lp, 1);
                // Correctly handle issue #475 cases by placing the lexer in
                // the correct mode when analyzing a modifier with parameter
                // expansions using `:', `-', `+', and `='
                if (c == ':' || c == '-' || c == '+' || c == '=') {
                    mode = ST_QUOTE;
                } else {
                    if (c != ':' && (n = fcgetc()) > 0) {
                        if (n != c) c = 0;
                        if (!c || (n = fcgetc()) > 0) {
                            fcseek(-LEN);
                            if (n == LPAREN) {
                                if (c != '%') {
                                    lp->token = n;
                                    sh_syntax(lp);
                                } else if (lp->lexd.warn) {
                                    errormsg(SH_DICT, ERROR_warn(0), e_lexquote, shp->inlineno,
                                             '%');
                                }
                            }
                        }
                    }
                    lp->lex.nestedbrace = 0;
                    mode = ST_NESTED;
                }
                continue;
            }
            case S_LBRA: {
                if ((c = endchar(lp)) == '$') {
                    c = fcgetc();
                    if (c > 0) fcseek(-LEN);
                    setchar(lp, RBRACE);
                    if (state[c] != S_ERR && c != RBRACE) continue;
                    if ((n = sh_lexstates[ST_BEGIN][c]) == 0 || n == S_OP || n == S_NLTOK) {
                        c = LBRACE;
                        goto do_comsub;
                    }
                }
            err:
                if (iswalpha(c)) continue;
                n = endchar(lp);
                mode = oldmode(lp);
                poplevel(lp);
                if (n != '$') {
                    lp->token = c;
                    sh_syntax(lp);
                } else {
                    if (lp->lexd.warn && c != '/' && sh_lexstates[ST_NORM][c] != S_BREAK &&
                        (c != '"' || mode == ST_QUOTE)) {
                        errormsg(SH_DICT, ERROR_warn(0), e_lexslash, shp->inlineno);
                    } else if (c == '"' && mode != ST_QUOTE && !ingrave) {
                        wordflags |= ARG_MESSAGE;
                    }
                    fcseek(-LEN);
                }
                continue;
            }
            case S_META: {
                if (lp->lexd.warn && endchar(lp) == RBRACE && !lp->lexd.nested_tilde) {
                    errormsg(SH_DICT, ERROR_warn(0), e_lexusequote, shp->inlineno, c);
                }
                continue;
            }
            case S_PUSH: {
                n = fcgetc();
                if (n == RPAREN) {
                    continue;
                } else {
                    fcseek(-LEN);
                }
                pushlevel(lp, RPAREN, mode);
                mode = ST_NESTED;
                continue;
            }
            case S_POP: {
            do_pop:
                if (c == RBRACE && mode == ST_NESTED && lp->lex.nestedbrace) {
                    lp->lex.nestedbrace--;
                    continue;
                }
                if (lp->lexd.level <= inlevel) break;
                if (lp->lexd.level == inlevel + 1 && lp->lex.incase >= TEST_RE && !lp->lex.intest) {
                    fcseek(-LEN);
                    goto breakloop;
                }
                n = endchar(lp);
                if (c == RBRACT && !(n == RBRACT || n == RPAREN)) continue;
                if ((c == RBRACE || c == RPAREN) && n == RPAREN) {
                    n = fcgetc();
                    if (n == LPAREN) {
                        if (c != RPAREN) fcseek(-LEN);
                        continue;
                    }
                    if (n > 0) fcseek(-LEN);
                    n = RPAREN;
                }
                if (c == RBRACE) lp->lexd.nested_tilde = 0;
                if (c == ';' && n != ';') {
                    if (lp->lexd.warn && n == RBRACE) {
                        errormsg(SH_DICT, ERROR_warn(0), e_lexusequote, shp->inlineno, c);
                    }
                    continue;
                }
                if (mode == ST_QNEST) {
                    if (lp->lexd.warn) {
                        errormsg(SH_DICT, ERROR_warn(0), e_lexescape, shp->inlineno, c);
                    }
                    continue;
                }
                mode = oldmode(lp);
                poplevel(lp);
                if (epatchar != '~') epatchar = '@';
                // Quotes in subscript need expansion.
                if (mode == ST_NAME && (wordflags & ARG_QUOTED)) wordflags |= ARG_MAC;
                // Check for ((...)).
                if (n == 1 && c == RPAREN) {
                    n = fcgetc();
                    if (n == RPAREN) {
                        if (mode == ST_NONE && !lp->lexd.dolparen) goto breakloop;
                        lp->lex.reservok = 1;
                        lp->lex.skipword = 0;
                        lp->token = EXPRSYM;
                        return lp->token;
                    }
                    // Backward compatibility.
                    {
                        if (lp->lexd.warn) {
                            errormsg(SH_DICT, ERROR_warn(0), e_lexnested, shp->inlineno);
                        }
                        state = lp->lexd.first;
                        if (state) {
                            n = state - fcseek(0);
                            fcseek(n);
                        }
                        lp->lexd.paren = 1;
                    }
                    lp->token = LPAREN;
                    return lp->token;
                }
                if (mode == ST_NONE) return 0;
                if (c != n && lp->lex.incase < TEST_RE) {
                    lp->token = c;
                    sh_syntax(lp);
                }
                if (c == RBRACE && (mode == ST_NAME || mode == ST_NORM)) goto epat;
                continue;
            }
            case S_EQ: {
                assignment = lp->assignok;
                if (!assignment) {
                    c = fcgetc();
                    fcseek(-LEN);
                    if (c == '(') lp->typed = assignment = 1;
                }
            }
            // FALLTHRU
            case S_COLON: {
                if (assignment) {
                    c = fcgetc();
                    if (c == '~') {
                        wordflags |= ARG_MAC;
                    } else if (c != LPAREN && assignment == SH_COMPASSIGN) {
                        assignment = 0;
                    }
                    if (c != EOF) fcseek(-LEN);
                }
                break;
            }
            case S_LABEL: {
                if (lp->lex.reservok && !lp->lex.incase) {
                    c = fcget();
                    fcseek(-LEN);
                    if (state[c] == S_BREAK) {
                        assignment = -1;
                        goto breakloop;
                    }
                }
                break;
            }
            case S_BRACT: {
                // Check for possible subscript.
                if ((n = endchar(lp)) == RBRACT || n == RPAREN || (mode == ST_BRACE) ||
                    (oldmode(lp) == ST_NONE) ||
                    (mode == ST_NAME && (lp->assignok || lp->lexd.level))) {
                    n = fcgetc();
                    if (n > 0 && n == ']') {
                        if (mode == ST_NAME) {
                            errormsg(SH_DICT, ERROR_exit(SYNBAD), e_lexsyntax1, shp->inlineno, "[]",
                                     "empty subscript");
                            __builtin_unreachable();
                        }
                        if (!epatchar || epatchar == '%') continue;
                    } else {
                        fcseek(-LEN);
                    }
                    pushlevel(lp, RBRACT, mode);
                    wordflags |= ARG_QUOTED;
                    mode = ST_NESTED;
                    continue;
                }
                wordflags |= ARG_EXP;
                break;
            }
            case S_BRACE: {
                if (lp->lexd.dolparen) {
                    if (mode != ST_BEGIN || !(lp->lex.reservok || lp->comsub)) break;
                    if (lp->comsub) {
                        lp->token = c;
                        return c;
                    }
                    n = fcgetc();
                    if (n > 0) {
                        fcseek(-LEN);
                    } else {
                        n = '\n';
                    }
                    if (n == RBRACT || sh_lexstates[ST_NORM][n]) {
                        lp->token = c;
                        return c;
                    }
                    break;
                } else if (mode == ST_NESTED && endchar(lp) == RBRACE) {
                    lp->lex.nestedbrace++;
                    continue;
                } else if (mode == ST_BEGIN) {
                    if (lp->comsub && c == RBRACE) {
                        lp->token = c;
                        return c;
                    }
                    goto do_reg;
                }

                bool isfirst = lp->lexd.first && fcseek(0) == lp->lexd.first + 1;
                n = fcgetc();
                if (n < 0) break;
                // Check for {}.
                if (c == LBRACE && n == RBRACE) break;
                fcseek(-LEN);
                // Check for reserved word { or }.
                if (lp->lex.reservok && state[n] == S_BREAK && isfirst) break;
                if (sh_isoption(lp->sh, SH_BRACEEXPAND) && c == LBRACE && !assignment &&
                    state[n] != S_BREAK && !lp->lex.incase && !lp->lex.intest &&
                    !lp->lex.skipword) {
                    wordflags |= ARG_EXP;
                }
                if (c == RBRACE && n == LPAREN) goto epat;
                break;
            }
            case S_PAT: {
                wordflags |= ARG_EXP;
            }
            // FALLTHRU
            case S_EPAT: {
            epat:
                n = fcgetc();
                if (n == LPAREN && c != '[') {
                    epatchar = c;
                    if (lp->lex.incase == TEST_RE) {
                        lp->lex.incase++;
                        pushlevel(lp, RPAREN, ST_NORM);
                        mode = ST_NESTED;
                    }
                    wordflags |= ARG_EXP;
                    pushlevel(lp, RPAREN, mode);
                    mode = ST_NESTED;
                    continue;
                }
                if (lp->lexd.warn && c == '[' && n == '^') {
                    errormsg(SH_DICT, ERROR_warn(0), e_lexcharclass, shp->inlineno);
                }
                if (n > 0) fcseek(-LEN);
                if (n == '=' && c == '+' && mode == ST_NAME) continue;
                break;
            }
            default: { break; }
        }
        lp->comp_assign = 0;
        if (mode == ST_NAME) {
            mode = ST_NORM;
        } else if (mode == ST_NONE) {
            return 0;
        }
    }

breakloop:
    if (lp->lexd.nocopy) {
        lp->lexd.balance = 0;
        return 0;
    }
    if (lp->lexd.dolparen) {
        lp->lexd.balance = 0;
        if (lp->lexd.docword) nested_here(lp);
        lp->lexd.message = (wordflags & ARG_MESSAGE);
        lp->token = 0;
        return lp->token;
    }
    if (!(state = lp->lexd.first)) state = fcfirst();
    n = fcseek(0) - (char *)state;
    if (!lp->arg) lp->arg = stkseek(stkp, ARGVAL);
    if (n > 0) sfwrite(stkp, state, n);
    // Add balancing character if necessary.
    if (lp->lexd.balance) {
        sfputc(stkp, lp->lexd.balance);
        lp->lexd.balance = 0;
    }
    sfputc(stkp, 0);
    stkseek(stkp, stktell(stkp) - 1);
    state = stkptr(stkp, ARGVAL);
    n = stktell(stkp) - ARGVAL;
    lp->lexd.first = NULL;
    if (n == 1) {
        // Check for numbered redirection.
        n = state[0];
        if (!lp->lex.intest && (c == '<' || c == '>') && isadigit(n)) {
            c = sh_lex(lp);
            lp->digits = (n - '0');
            return c;
        }
        if (n == LBRACT) {
            c = 0;
        } else if (n == RBRACE && lp->comsub) {
            lp->token = n;
            return n;
        } else if (n == '~') {
            c = ARG_MAC;
        } else {
            c = (wordflags & ARG_EXP);
        }
        n = 1;
    } else if (n > 2 && state[0] == '{' && state[n - 1] == '}' && !lp->lex.intest &&
               !lp->lex.incase && (c == '<' || c == '>') && sh_isoption(lp->sh, SH_BRACEEXPAND)) {
        if (!strchr(state, ',')) {
            stkseek(stkp, stktell(stkp) - 1);
            lp->arg = (struct argnod *)stkfreeze(stkp, 1);
            lp->token = IOVNAME;
            return lp->token;
        }
        c = wordflags;
    } else {
        c = wordflags;
    }
    if (assignment < 0) {
        stkseek(stkp, stktell(stkp) - 1);
        lp->arg = (struct argnod *)stkfreeze(stkp, 1);
        lp->lex.reservok = 1;
        lp->token = LABLSYM;
        return lp->token;
    }
    if (assignment || (lp->lex.intest && !lp->lex.incase) || mode == ST_NONE) c &= ~ARG_EXP;
    if ((c & ARG_EXP) && (c & ARG_QUOTED)) c |= ARG_MAC;
    if (mode == ST_NONE) {  // eliminate trailing ))
        stkseek(stkp, stktell(stkp) - 2);
    }
    if (c & ARG_MESSAGE) {
        if (sh_isoption(lp->sh, SH_DICTIONARY)) lp->arg = sh_endword(shp, 2);
        c |= ARG_MAC;
    }
    if (c == 0 || (c & (ARG_MAC | ARG_EXP | ARG_MESSAGE))) {
        lp->arg = (struct argnod *)stkfreeze(stkp, 1);
        lp->arg->argflag = (c ? c : ARG_RAW);
    } else if (mode == ST_NONE) {
        lp->arg = sh_endword(shp, -1);
    } else {
        lp->arg = sh_endword(shp, 0);
    }
    state = lp->arg->argval;
    lp->comp_assign = assignment;
    if (assignment) {
        lp->arg->argflag |= ARG_ASSIGN;
        if (sh_isoption(shp, SH_NOEXEC)) {
            char *cp = strchr(state, '=');
            if (cp && strncmp(++cp, "$((", 3) == 0) {
                errormsg(SH_DICT, ERROR_warn(0), e_lexarithwarn, shp->inlineno, cp - state, state,
                         cp + 3, state);
            }
        }
    } else if (!lp->lex.skipword) {
        lp->assignok = 0;
    }
    lp->arg->argchn.cp = NULL;
    lp->arg->argnxt.ap = NULL;
    if (mode == ST_NONE) {
        lp->token = EXPRSYM;
        return EXPRSYM;
    }
    if (lp->lex.intest) {
        if (lp->lex.testop1) {
            lp->lex.testop1 = 0;
            if (n == 2 && state[0] == '-' && state[2] == 0 && strchr(test_opchars, state[1])) {
                if (lp->lexd.warn && state[1] == 'a') {
                    errormsg(SH_DICT, ERROR_warn(0), e_lexobsolete2, shp->inlineno);
                }
                lp->digits = state[1];
                lp->token = TESTUNOP;
            } else if (n == 1 && state[0] == '!' && state[1] == 0) {
                lp->lex.testop1 = 1;
                lp->token = '!';
            } else {
                lp->lex.testop2 = 1;
                lp->token = 0;
            }
            return lp->token;
        }
        lp->lex.incase = 0;
        c = sh_lookup(state, shtab_testops);
        switch (c) {
            case 0:
            case TEST_OR:
            case TEST_AND: {
                lp->token = 0;
                return lp->token;
            }
            case TEST_END: {
                lp->lex.testop2 = lp->lex.intest = 0;
                lp->lex.reservok = 1;
                lp->token = ETESTSYM;
                return lp->token;
            }
            case TEST_SEQ: {
                if (lp->lexd.warn && state[1] == 0) {
                    errormsg(SH_DICT, ERROR_warn(0), e_lexobsolete3, shp->inlineno);
                }
            }
            // FALLTHRU
            default: {
                if (lp->lex.testop2) {
                    if (lp->lexd.warn && (c & TEST_ARITH)) {
                        errormsg(SH_DICT, ERROR_warn(0), e_lexobsolete4, shp->inlineno, state);
                    }
                    if (c & TEST_PATTERN) {
                        lp->lex.incase = 1;
                    } else if (c == TEST_REP) {
                        lp->lex.incase = TEST_RE;
                    }
                    lp->lex.testop2 = 0;
                    lp->digits = c;
                    lp->token = TESTBINOP;
                    return lp->token;
                }
                lp->token = 0;
                return lp->token;
            }
        }
    }
    if (lp->lex.reservok /* && !lp->lex.incase*/ && n <= 2) {
        // Check for {, }, !.
        c = state[0];
        if (n == 1 && (c == '{' || c == '}' || c == '!')) {
            if (lp->lexd.warn && c == '{' && lp->lex.incase == 2) {
                errormsg(SH_DICT, ERROR_warn(0), e_lexobsolete6, shp->inlineno);
            }
            if (lp->lex.incase == 1 && c == RBRACE) lp->lex.incase = 0;
            lp->token = c;
            return lp->token;
        } else if (!lp->lex.incase && c == LBRACT && state[1] == LBRACT) {
            lp->lex.intest = lp->lex.testop1 = 1;
            lp->lex.testop2 = lp->lex.reservok = 0;
            lp->token = BTESTSYM;
            return lp->token;
        }
    }
    c = 0;
    if (!lp->lex.skipword) {
        if (n > 1 && lp->lex.reservok == 1 && mode == ST_NAME &&
            (c = sh_lookup(state, shtab_reserved))) {
            if (lp->lex.incase) {
                if (lp->lex.incase > 1) {
                    lp->lex.incase = 1;
                } else if (c == ESACSYM) {
                    lp->lex.incase = 0;
                } else {
                    c = 0;
                }
            } else if (c == FORSYM || c == CASESYM || c == SELECTSYM || c == FUNCTSYM ||
                       c == NSPACESYM) {
                lp->lex.skipword = 1;
                lp->lex.incase = 2 * (c == CASESYM);
            } else {
                lp->lex.skipword = 0;
            }
            if (c == INSYM) {
                lp->lex.reservok = 0;
            } else if (c == TIMESYM) {
                // Yech - POSIX requires time -p.
                while ((n = fcgetc()) == ' ' || n == '\t') {
                    ;  // empty loop
                }
                if (n > 0) fcseek(-LEN);
                if (n == '-') c = 0;
            }
            lp->token = c;
            return lp->token;
        }
        if (!(wordflags & ARG_QUOTED) && (lp->lex.reservok || lp->aliasok)) {
            // Check for aliases.
            Namval_t *np;
            if (!lp->lex.incase && !assignment && fcpeek(0) != LPAREN &&
                (np = nv_search(state, shp->alias_tree, 0)) && !nv_isattr(np, NV_NOEXPAND) &&
                (lp->aliasok != 2 || nv_isattr(np, BLT_DCL)) &&
                (!sh_isstate(lp->sh, SH_NOALIAS) || nv_isattr(np, NV_NOFREE)) &&
                (state = nv_getval(np))) {
                setupalias(lp, state, np);
                nv_onattr(np, NV_NOEXPAND);
                lp->lex.reservok = 1;
                lp->assignok |= lp->lex.reservok;
                return sh_lex(lp);
            }
        }
        lp->lex.reservok = 0;
    }
    lp->lex.skipword = lp->lexd.docword = 0;
    lp->token = c;
    return lp->token;
}

//
// Read to end of command substitution.
//
static_fn int comsub(Lex_t *lp, int endtok) {
    int n, c, count = 1;
    int line = lp->sh->inlineno;
    struct ionod *inheredoc = lp->heredoc;
    char *first, *cp = fcseek(0), word[5];
    int off, messages = 0, assignok = lp->assignok, csub;
    struct lexstate save;

    save = lp->lex;
    csub = lp->comsub;
    sh_lexopen(lp, lp->sh, 1);
    lp->lexd.dolparen++;
    lp->lex.incase = 0;
    pushlevel(lp, 0, 0);
    lp->comsub = (endtok == LBRACE);
    first = lp->lexd.first;
    off = cp - (first ? first : fcfirst());
    if (off < 0) {
        c = *cp;
        *cp = 0;
    }
    n = sh_lex(lp);
    if (off < 0) *cp = c;
    if (n == endtok || off < 0) {
        if (endtok == LPAREN && lp->lexd.paren) {
            if (first == lp->lexd.first) {
                n = cp + 1 - (char *)fcseek(0);
                fcseek(n);
            }
            count++;
            lp->lexd.paren = 0;
            (void)fcgetc();  // it's not obvious why we're throwing away this char
        }
        while (1) {
            // Look for case and esac.
            n = 0;
            while (1) {
                c = fcgetc();
                // Skip leading white space.
                if (n == 0 && !sh_lexstates[ST_BEGIN][c]) continue;
                if (n == 4) break;
                if (sh_lexstates[ST_NAME][c]) {
                    if (c == ' ' || c == '\t') {
                        n = 0;
                        continue;
                    }
                    goto skip;
                }
                word[n++] = c;
            }
            if (sh_lexstates[ST_NAME][c] == S_BREAK) {
                if (strncmp(word, "case", 4) == 0) {
                    lp->lex.incase = 1;
                } else if (strncmp(word, "esac", 4) == 0) {
                    lp->lex.incase = 0;
                }
            }
        skip:
            if (c && (c != '#' || n == 0)) fcseek(-LEN);
            if (c == RBRACE && lp->lex.incase) lp->lex.incase = 0;
            c = sh_lex(lp);
            switch (c) {
                case LBRACE: {
                    if (endtok == LBRACE && !lp->lex.incase) {
                        lp->comsub = 0;
                        count++;
                    }
                    break;
                }
                case RBRACE: {
                rbrace:
                    if (endtok == LBRACE && --count <= 0) goto done;
                    if (count == 1) lp->comsub = endtok == LBRACE;
                    break;
                }
                case IPROCSYM:
                case OPROCSYM:
                case LPAREN: {
                    if (endtok == LPAREN && !lp->lex.incase) count++;
                    break;
                }
                case RPAREN: {
                    if (lp->lex.incase) {
                        lp->lex.incase = 0;
                    } else if (endtok == LPAREN && --count <= 0) {
                        goto done;
                    }
                    break;
                }
                case EOFSYM: {
                    lp->lastline = line;
                    lp->lasttok = endtok;
                    sh_syntax(lp);
                }
                case IOSEEKSYM: {
                    c = fcgetc();
                    if (c != '#' && c > 0) fcseek(-LEN);
                    break;
                }
                case IODOCSYM: {
                    lp->lexd.docextra = 0;
                    sh_lex(lp);
                    break;
                }
                case 0: {
                    lp->lex.reservok = 0;
                    messages |= lp->lexd.message;
                    break;
                }
                case ';': {
                    do {
                        c = fcgetc();
                    } while (!sh_lexstates[ST_BEGIN][c]);
                    if (c == RBRACE && endtok == LBRACE) goto rbrace;
                    if (c > 0) fcseek(-LEN);
                }
                // FALLTHRU
                default: { lp->lex.reservok = 1; }
            }
        }
    }
done:
    poplevel(lp);
    lp->comsub = csub;
    lp->lastline = line;
    lp->lexd.dolparen--;
    lp->lex = save;
    lp->assignok = (endchar(lp) == RBRACT ? assignok : 0);
    if (lp->heredoc && !inheredoc) {
        errormsg(SH_DICT, ERROR_exit(SYNBAD), e_lexsyntax5, lp->sh->inlineno, lp->heredoc->ioname);
        __builtin_unreachable();
    }
    return messages;
}

//
// Here-doc nested in $(...).
// Allocate ionode with delimiter filled in without disturbing stak.
//
static_fn void nested_here(Lex_t *lp) {
    struct ionod *iop;
    int n = 0, offset;
    struct argnod *arg = lp->arg;
    Stk_t *stkp = lp->sh->stk;
    char *base;

    offset = stktell(stkp);
    if (offset) base = stkfreeze(stkp, 0);
    if (lp->lexd.docend) n = fcseek(0) - lp->lexd.docend;
    iop = calloc(1, sizeof(struct ionod) + lp->lexd.docextra + n + ARGVAL);
    iop->iolst = lp->heredoc;
    stkseek(stkp, ARGVAL);
    if (lp->lexd.docextra) {
        sfseek(lp->sh->strbuf, (Sfoff_t)0, SEEK_SET);
        sfmove(lp->sh->strbuf, stkp, lp->lexd.docextra, -1);
        sfseek(lp->sh->strbuf, (Sfoff_t)0, SEEK_SET);
    }
    sfwrite(stkp, lp->lexd.docend, n);
    lp->arg = sh_endword(lp->sh, 0);
    iop->ioname = (char *)(iop + 1);
    strcpy(iop->ioname, lp->arg->argval);
    iop->iofile = (IODOC | IORAW);
    if (lp->lexd.docword > 1) iop->iofile |= IOSTRIP;
    lp->heredoc = iop;
    lp->arg = arg;
    lp->lexd.docword = 0;
    if (offset) {
        stkset(stkp, base, offset);
    } else {
        stkseek(stkp, 0);
    }
}

//
// Skip to <close> character.
// If <copy> is non,zero, then the characters are copied to the stack.
// <state> is the initial lexical state.
//
void sh_lexskip(Lex_t *lp, int close, int copy, int state) {
    char *cp;

    lp->lexd.nest = close;
    lp->lexd.lex_state = state;
    lp->lexd.noarg = 1;
    if (copy) {
        fcnotify(lex_advance, lp);
    } else {
        lp->lexd.nocopy++;
    }
    sh_lex(lp);
    lp->lexd.noarg = 0;
    if (copy) {
        fcnotify(0, lp);
        if (!(cp = lp->lexd.first)) cp = fcfirst();
        if ((copy = fcseek(0) - cp) > 0) sfwrite(lp->sh->stk, cp, copy);
    } else {
        lp->lexd.nocopy--;
    }
}

//
// Read in here-document from script. Quoted here documents, and here-documents
// without special chars are noted with the IOQUOTE flag.
//
// Returns 1 for complete here-doc, 0 for EOF.
//
static_fn int here_copy(Lex_t *lp, struct ionod *iop) {
    const char *state;
    int c, n;
    char *bufp, *cp;
    Sfio_t *sp = lp->sh->heredocs, *funlog;
    int stripcol = 0, stripflg, nsave, special = 0;

    funlog = lp->sh->funlog;
    if (funlog) {
        if (fcfill() > 0) fcseek(-LEN);
        lp->sh->funlog = NULL;
    }
    if (iop->iolst) here_copy(lp, iop->iolst);
    iop->iooffset = sfseek(sp, (off_t)0, SEEK_END);
    iop->iosize = 0;
    iop->iodelim = iop->ioname;
    // Check for and strip quoted characters in delimiter string.
    stripflg = iop->iofile & IOSTRIP;
    if (stripflg) {
        while (*iop->iodelim == '\t') iop->iodelim++;
        // Skip over leading tabs in document.
        if (iop->iofile & IOLSEEK) {
            iop->iofile &= ~IOLSEEK;
            while ((c = fcgetc()) == '\t' || c == ' ') {
                if (c == ' ') {
                    stripcol++;
                } else {
                    stripcol += 8 - stripcol % 8;
                }
            }
        } else {
            while ((c = fcgetc()) == '\t') {
                ;  // empty loop
            }
        }
        if (c > 0) fcseek(-LEN);
    }
    if (iop->iofile & IOQUOTE) {
        state = sh_lexstates[ST_LIT];
    } else {
        state = sh_lexstates[ST_QUOTE];
    }
    bufp = fcseek(0);
    n = S_NL;
    while (1) {
        if (n != S_NL) {
            // Skip over regular characters.
            do {
                if (fcleft() < MB_LEN_MAX && mblen(fcseek(0), MB_CUR_MAX) < 0) {
                    n = S_EOF;
                    LEN = -fcleft();
                    break;
                }
            } while ((n = STATE(state, c)) == 0);
        }
        if (n == S_EOF || !(c = fcget())) {
            if (LEN < 0) {
                c = fclast() - bufp;
            } else {
                c = (fcseek(0) - 1) - bufp;
            }
            if (!lp->lexd.dolparen && c) {
                if (n == S_ESC) c--;
                if (!lp->lexd.dolparen && (c = sfwrite(sp, bufp, c)) > 0) iop->iosize += c;
            }
            if (LEN == 0) LEN = 1;
            if (LEN < 0) {
                n = LEN;
                c = fcmbget(&LEN);
                LEN += n;
            } else {
                c = lexfill(lp);
            }
            if (c < 0) break;
            if (n == S_ESC) {
                if (c == NL) {
                    fcseek(1);
                } else if (!lp->lexd.dolparen) {
                    iop->iosize++;
                    sfputc(sp, '\\');
                }
            }
            bufp = fcseek(-LEN);
        } else {
            fcseek(-LEN);
        }
        switch (n) {
            case S_NL: {
                lp->sh->inlineno++;
                if ((stripcol && c == ' ') || (stripflg && c == '\t')) {
                    if (!lp->lexd.dolparen) {  // write out line
                        n = fcseek(0) - bufp;
                        if ((n = sfwrite(sp, bufp, n)) > 0) iop->iosize += n;
                    }
                    // Skip over tabs.
                    if (stripcol) {
                        int col = 0;
                        do {
                            c = fcgetc();
                            if (c == ' ') {
                                col++;
                            } else {
                                col += 8 - col % 8;
                            }
                            if (col > stripcol) break;
                        } while (c == ' ' || c == '\t');
                    } else {
                        while (c == '\t') c = fcgetc();
                    }
                    if (c <= 0) goto done;
                    bufp = fcseek(-LEN);
                }
                if (c != iop->iodelim[0]) break;
                cp = fcseek(0);
                nsave = n = 0;
                while (1) {
                    if (!(c = fcget())) {
                        if (!lp->lexd.dolparen && (c = cp - bufp)) {
                            if ((c = sfwrite(sp, cp = bufp, c)) > 0) iop->iosize += c;
                        }
                        nsave = n;
                        if ((c = lexfill(lp)) <= 0) {
                            c = iop->iodelim[n] == 0;
                            goto done;
                        }
                    }
                    if (c == NL) lp->sh->inlineno++;
                    if (iop->iodelim[n] == 0 && (c == NL || c == RPAREN)) {
                        if (!lp->lexd.dolparen && (n = cp - bufp)) {
                            if ((n = sfwrite(sp, bufp, n)) > 0) iop->iosize += n;
                        }
                        lp->sh->inlineno--;
                        if (c == RPAREN) fcseek(-LEN);
                        goto done;
                    }
                    if (iop->iodelim[n++] != c) {
                        // The match for delimiter failed. nsave>0 only when a
                        // buffer boundary was crossed while checking the
                        // delimiter.
                        if (!lp->lexd.dolparen && nsave > 0) {
                            if ((n = sfwrite(sp, iop->iodelim, nsave)) > 0) iop->iosize += n;
                            bufp = fcfirst();
                        }
                        if (c == NL) fcseek(-LEN);
                        break;
                    }
                }
                break;
            }
            case S_ESC: {
                n = 1;
                if (c == NL) {
                    /* new-line joining */
                    lp->sh->inlineno++;
                    if (!lp->lexd.dolparen && (n = (fcseek(0) - bufp) - n) >= 0) {
                        if (n && (n = sfwrite(sp, bufp, n)) > 0) iop->iosize += n;
                        bufp = fcseek(0) + 1;
                    }
                } else {
                    special++;
                }
                fcnxt();
                break;
            }
            case S_GRAVE:
            case S_DOL: {
                special++;
                break;
            }
            default: { break; }
        }
        n = 0;
    }
done:
    lp->sh->funlog = funlog;
    if (lp->lexd.dolparen) {
        free(iop);
    } else if (!special) {
        iop->iofile |= IOQUOTE;
    }
    return c;
}

//
// Generates string for given token.
//
static_fn char *fmttoken(Lex_t *lp, int sym, char *tok) {
    int n = 1;

    if (sym < 0) return (char *)sh_translate(e_lexzerobyte);
    if (sym == 0) return lp->arg ? lp->arg->argval : "?";
    if (lp->lex.intest && lp->arg && *lp->arg->argval) return lp->arg->argval;
    if (sym & SYMRES) {
        const Shtable_t *tp = shtab_reserved;
        while (tp->sh_number && tp->sh_number != sym) tp++;
        return (char *)tp->sh_name;
    }
    if (sym == EOFSYM) return (char *)sh_translate(e_endoffile);
    if (sym == NL) return (char *)sh_translate(e_newline);
    tok[0] = sym;
    if (sym & SYMREP) {
        tok[n++] = sym;
    } else {
        switch (sym & SYMMASK) {
            case SYMAMP: {
                sym = '&';
                break;
            }
            case SYMPIPE: {
                sym = '|';
                break;
            }
            case SYMGT: {
                sym = '>';
                break;
            }
            case SYMLPAR: {
                sym = LPAREN;
                break;
            }
            case SYMSHARP: {
                sym = '#';
                break;
            }
            case SYMSEMI: {
                if (tok[0] == '<') tok[n++] = '>';
                sym = ';';
                break;
            }
            default: { sym = 0; }
        }
        tok[n++] = sym;
    }
    tok[n] = 0;
    return tok;
}

//
// Print a bad syntax message.
//
__attribute__((noreturn)) void sh_syntax(Lex_t *lp) {
    Shell_t *shp = lp->sh;
    const char *cp = sh_translate(e_unexpected);
    char *tokstr;
    int tok = lp->token;
    char tokbuf[3];
    Sfio_t *sp;

    if ((tok == EOFSYM) && lp->lasttok) {
        tok = lp->lasttok;
        cp = sh_translate(e_unmatched);
    } else {
        lp->lastline = shp->inlineno;
    }
    tokstr = fmttoken(lp, tok, tokbuf);
    if ((sp = fcfile()) || (shp->infd >= 0 && (sp = shp->sftable[shp->infd]))) {
        // Clear out any pending input.
        Sfio_t *top;
        while (fcget() > 0) {
            ;  // empty loop
        }
        fcclose();
        while ((top = sfstack(sp, SF_POPSTACK))) sfclose(top);
    } else {
        fcclose();
    }
    shp->inlineno = lp->inlineno;
    shp->st.firstline = lp->firstline;
    if (!sh_isstate(shp, SH_INTERACTIVE) && !sh_isstate(shp, SH_PROFILE)) {
        errormsg(SH_DICT, ERROR_exit(SYNBAD), e_lexsyntax1, lp->lastline, tokstr, cp);
        __builtin_unreachable();
    } else {
        errormsg(SH_DICT, ERROR_exit(SYNBAD), e_lexsyntax2, tokstr, cp);
        __builtin_unreachable();
    }
}

static_fn char *stack_shift(Stk_t *stkp, char *sp, char *dp) {
    char *ep;
    int offset = stktell(stkp);
    int left = offset - (sp - stkptr(stkp, 0));
    int shift = (dp + 1 - sp);

    offset += shift;
    stkseek(stkp, offset);
    sp = stkptr(stkp, offset);
    ep = sp - shift;
    while (left--) *--sp = *--ep;
    return sp;
}

//
// Assumes that current word is unfrozen on top of the stak.
// If <mode> is zero, gets rid of quoting and consider argument as string
//    and returns pointer to frozen arg.
// If mode==1, just replace $"..." strings with international strings
//    The result is left on the stak.
// If mode==2, the each $"" string is printed on standard output.
//
struct argnod *sh_endword(Shell_t *shp, int mode) {
    const char *state = sh_lexstates[ST_NESTED];
    size_t n;
    char *sp, *dp;
    int inquote = 0, inlit = 0; /* set within quoted strings */
    struct argnod *argp = NULL;
    char *ep = NULL;
    char *xp = NULL;
    int bracket = 0;
    Stk_t *stkp = shp->stk;

    sfputc(stkp, 0);
    sp = stkptr(stkp, ARGVAL);
    if (mbwide()) {
        do {
            int len;
            switch (len = mblen(sp, MB_CUR_MAX)) {
                case -1: /* illegal multi-byte char */
                case 0:
                case 1: {
                    unsigned int idx = (unsigned char)*sp++;
                    n = state[idx];
                    break;
                }
                default: {
                    //
                    // None of the state tables contain entries for multibyte
                    // characters, however, they should be treated the same as
                    // any other alph character.  Therefore, we'll use the
                    // state of the 'a' character.
                    //
                    n = state['a'];
                    sp += len;
                }
            }
        } while (n == 0);
    } else {
        unsigned int idx = (unsigned char)*sp++;
        while ((n = state[idx]) == 0) {
            idx = (unsigned char)*sp++;
        }
    }
    dp = sp;
    if (mode < 0) inquote = 1;
    while (1) {
        switch (n) {
            case S_EOF: {
                stkseek(stkp, dp - stkptr(stkp, 0));
                if (mode <= 0) {
                    argp = (struct argnod *)stkfreeze(stkp, 0);
                    argp->argflag = ARG_RAW | ARG_QUOTED;
                }
                return argp;
            }
            case S_LIT: {
                if (!(inquote & 1)) {
                    inlit = !inlit;
                    if (mode == 0 || (mode < 0 && bracket)) {
                        dp--;
                        if (ep) {
                            *dp = 0;
                            stresc(ep);
                            dp = ep + strlen(ep);
                        }
                        ep = 0;
                    }
                }
                break;
            }
            case S_QUOTE: {
                if (mode < 0 && !bracket) break;
                if (inlit) break;

                if (mode <= 0) dp--;
                inquote = inquote ^ 1;
                if (ep) {
                    char *msg;
                    if (mode == 2) {
                        sfprintf(sfstdout, "%.*s\n", dp - ep, ep);
                        ep = 0;
                        break;
                    }
                    *--dp = 0;
                    msg = ERROR_translate(0, error_info.id, 0, ep);
                    n = strlen(msg);
                    dp = ep + n;
                    if (sp - dp <= 1) {
                        sp = stack_shift(stkp, sp, dp);
                        dp = sp - 1;
                        ep = dp - n;
                    }
                    memmove(ep, msg, n);
                    *dp++ = '"';
                }
                ep = 0;
                break;
            }
            case S_DOL: {  // check for $'...'  and $"..."
                if (inlit) break;
                if (*sp == LPAREN || *sp == LBRACE) {
                    inquote <<= 1;
                    break;
                }
                if (inquote & 1) break;
                if (*sp != '\'' && *sp != '"') break;

                if (*sp == '"') {
                    inquote |= 1;
                } else {
                    inlit = 1;
                }
                sp++;
                if ((mode == 0 || (mode < 0 && bracket)) || (inquote & 1)) {
                    if (mode == 2) {
                        ep = dp++;
                    } else if (mode == 1) {
                        (ep = dp)[-1] = '"';
                    } else {
                        ep = --dp;
                    }
                }
                break;
            }
            case S_ESC: {
                if (inlit || mode > 0) {
                    if (mode < 0) {
                        if (dp >= sp) {
                            sp = stack_shift(stkp, sp, dp + 1);
                            dp = sp - 2;
                        }
                        *dp++ = '\\';
                    }
                    if (ep) *dp++ = *sp++;
                    break;
                }
                n = (unsigned char)*sp;
                if (!(inquote & 1) || (sh_lexstates[ST_QUOTE][n] && n != RBRACE)) {
                    if (n == '\n') {
                        dp--;
                    } else {
                        dp[-1] = n;
                    }
                    sp++;
                }
                break;
            }
            case S_POP: {
                if (sp[-1] != RBRACT) break;
                if (!inlit && !(inquote & 1)) {
                    inquote >>= 1;
                    if (xp) dp = sh_checkid(xp, dp);
                    xp = 0;
                    if (--bracket <= 0 && mode < 0) inquote = 1;
                } else if ((inlit || inquote) && mode < 0) {
                    dp[-1] = '\\';
                    if (dp >= sp) {
                        sp = stack_shift(stkp, sp, dp);
                        dp = sp - 1;
                    }
                    *dp++ = ']';
                }
                break;
            }
            case S_BRACT: {
                if (dp[-2] == '.') xp = dp;
                if (mode < 0) {
                    if (inlit || (bracket && inquote)) {
                        dp[-1] = '\\';
                        if (dp >= sp) {
                            sp = stack_shift(stkp, sp, dp);
                            dp = sp - 1;
                        }
                        *dp++ = '[';
                    } else if (bracket++ == 0) {
                        inquote = 0;
                    }
                }
                break;
            }
            default: { break; }
        }
        if (mbwide()) {
            do {
                int len;
                switch (len = mblen(sp, MB_CUR_MAX)) {
                    case -1:  // illegal multi-byte char
                    case 0:
                    case 1: {
                        unsigned int idx = (unsigned char)*sp;
                        *dp++ = *sp++;
                        n = state[idx];
                        break;
                    }
                    default: {
                        //
                        // None of the state tables contain entries for
                        // multibyte characters, however, they should be
                        // treated the same as any other alph character.
                        // Therefore, we'll use the state of the 'a' character.
                        //
                        while (len--) *dp++ = *sp++;
                        n = state['a'];
                    }
                }
            } while (n == 0);
        } else {
            unsigned int idx = (unsigned char)*sp;
            *dp++ = *sp++;
            while ((n = state[idx]) == 0) {
                idx = (unsigned char)*sp;
                *dp++ = *sp++;
            }
        }
    }
}

struct alias {
    Sfdisc_t disc;
    Namval_t *np;
    int nextc;
    int line;
    char buf[2];
    Lex_t *lp;
};

//
// This code gets called whenever an end of string is found with alias.
//
#ifdef SF_BUFCONST
static_fn int alias_exceptf(Sfio_t *iop, int type, void *data, Sfdisc_t *handle)
#else
static_fn int alias_exceptf(Sfio_t *iop, int type, Sfdisc_t *handle)
#endif
{
    UNUSED(data);
    struct alias *ap = (struct alias *)handle;
    Namval_t *np;
    Lex_t *lp;

    if (type == 0 || type == SF_ATEXIT || !ap) return 0;
    lp = ap->lp;
    np = ap->np;
    if (type != SF_READ) {
        if (type == SF_CLOSING) {
            Sfdisc_t *dp = sfdisc(iop, SF_POPDISC);
            if (dp != handle) sfdisc(iop, dp);
        } else if (type == SF_DPOP || type == SF_FINAL) {
            free(ap);
        }
        goto done;
    }
    if (ap->nextc) {
        // If last character is a blank, then next work can be alias.
        int c = fcpeek(-1);
        if (iswblank(c)) lp->aliasok = 1;
        *ap->buf = ap->nextc;
        ap->nextc = 0;
        sfsetbuf(iop, ap->buf, 1);
        return 1;
    }
done:
    if (np) nv_offattr(np, NV_NOEXPAND);
    return 0;
}

static_fn void setupalias(Lex_t *lp, const char *string, Namval_t *np) {
    Sfio_t *iop, *base;
    struct alias *ap = malloc(sizeof(struct alias));

    ap->disc = alias_disc;
    ap->lp = lp;
    ap->buf[1] = 0;
    ap->np = np;
    if (ap->np) {
        if (lp->kiafile) {
            unsigned long r;
            r = kiaentity(lp, nv_name(np), -1, 'p', 0, 0, lp->current, 'a', 0, "");
            sfprintf(lp->kiatmp, "p;%..64d;p;%..64d;%d;%d;e;\n", lp->current, r, lp->sh->inlineno,
                     lp->sh->inlineno);
        }
        if ((ap->nextc = fcget()) == 0) ap->nextc = ' ';
    } else {
        ap->nextc = 0;
    }
    iop = sfopen(NULL, (char *)string, "s");
    sfdisc(iop, &ap->disc);
    lp->lexd.nocopy++;
    if (!(base = fcfile())) base = sfopen(NULL, fcseek(0), "s");
    fcclose();
    sfstack(base, iop);
    fcfopen(base);
    lp->lexd.nocopy--;
}

//
// Grow storage stack for nested constructs by STACK_ARRAY.
//
static_fn void stack_grow(Lex_t *lp) {
    lp->lexd.lex_max += STACK_ARRAY;
    if (lp->lexd.lex_match) {
        lp->lexd.lex_match = realloc(lp->lexd.lex_match, sizeof(int) * lp->lexd.lex_max);
    } else {
        lp->lexd.lex_match = malloc(sizeof(int) * STACK_ARRAY);
    }
}
