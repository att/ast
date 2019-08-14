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
// UNIX shell
//
// S. R. Bourne
// Rewritten by David Korn
// AT&T Labs
//
//  This is the parser for the Korn shell language.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <limits.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "argnod.h"
#include "ast.h"
#include "ast_assert.h"
#include "builtins.h"
#include "cdt.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "fcin.h"
#include "history.h"
#include "lexstates.h"
#include "name.h"
#include "path.h"
#include "sfio.h"
#include "shlex.h"
#include "shnodes.h"
#include "stk.h"
#include "test.h"

#define HERE_MEM SF_BUFSIZE  // size of here-docs kept in memory
#define hash nvlink.lh.__hash

// These routines are local to this module.

static_fn Shnode_t *makeparent(Lex_t *, int, Shnode_t *);
static_fn Shnode_t *makelist(Lex_t *, int, Shnode_t *, Shnode_t *);
static_fn struct argnod *qscan(Lex_t *, struct comnod *, int);
static_fn struct ionod *inout(Lex_t *, struct ionod *, int);
static_fn Shnode_t *sh_cmd(Lex_t *, int, int);
static_fn Shnode_t *term(Lex_t *, int);
static_fn Shnode_t *list(Lex_t *, int);
static_fn struct regnod *syncase(Lex_t *, int);
static_fn Shnode_t *item(Lex_t *, int);
static_fn Shnode_t *simple(Lex_t *, int, struct ionod *);
static_fn int skipnl(Lex_t *, int);
static_fn Shnode_t *test_expr(Lex_t *, int);
static_fn Shnode_t *test_and(Lex_t *);
static_fn Shnode_t *test_or(Lex_t *);
static_fn Shnode_t *test_primary(Lex_t *);

#define sh_getlineno(lp) (lp->lastline)
#define CNTL(x) ((x)&037)

static int opt_get;
static int loop_level;
static struct argnod *label_list;
static struct argnod *label_last;

#define getnode(type) stkalloc(stkstd, sizeof(struct type))

//
// Write out entities for each item in the list type=='V' for variable assignment lists. Otherwise
// type is determined by the command.
//
static_fn unsigned long writedefs(Lex_t *lexp, struct argnod *arglist, int line, int type,
                                  struct argnod *cmd) {
    struct argnod *argp = arglist;
    char *cp;
    int eline;
    size_t n;
    int width = 0;
    unsigned long r = 0;
    static char atbuff[20];
    int justify = 0;
    char *attribute = atbuff;
    unsigned long parent = lexp->script;

    if (type == 0) {
        parent = lexp->current;
        type = 'v';
        switch (*argp->argval) {
            case 'a': {
                type = 'p';
                justify = 'a';
                break;
            }
            case 'e': {
                *attribute++ = 'x';
                break;
            }
            case 'r': {
                *attribute++ = 'r';
                break;
            }
            case 'l': {
                break;
            }
            default: { break; }
        }
        while ((argp = argp->argnxt.ap)) {
            if ((n = *(cp = argp->argval)) != '-' && n != '+') break;
            if (cp[1] == n) break;
            while ((n = *++cp)) {
                if (isdigit(n)) {
                    width = 10 * width + n - '0';
                } else if (n == 'L' || n == 'R' || n == 'Z') {
                    justify = n;
                } else {
                    *attribute++ = n;
                }
            }
        }
    } else if (cmd) {
        parent = kiaentity(lexp, sh_argstr(cmd), -1, 'p', -1, -1, lexp->unknown, 'b', 0, "");
    }
    *attribute = 0;
    while (argp) {
        if ((cp = strchr(argp->argval, '=')) || (cp = strchr(argp->argval, '?'))) {
            n = cp - argp->argval;
        } else {
            n = strlen(argp->argval);
        }
        eline = lexp->sh->inlineno - (lexp->token == NL);
        r = kiaentity(lexp, argp->argval, n, type, line, eline, parent, justify, width, atbuff);
        sfprintf(lexp->kiatmp, "p;%..64d;v;%..64d;%d;%d;s;\n", lexp->current, r, line, eline);
        argp = argp->argnxt.ap;
    }
    return r;
}

static_fn void typeset_order(const char *str, int line) {
    int c, n = 0;
    unsigned const char *cp = (unsigned char *)str;
    static unsigned char *table;

    if (*cp != '+' && *cp != '-') return;
    if (!table) {
        table = calloc(1, 256);
        for (cp = (unsigned char *)"bflmnprstuxACHS"; (c = *cp); cp++) table[c] = 1;
        for (cp = (unsigned char *)"aiEFLRXhTZ"; (c = *cp); cp++) table[c] = 2;
        for (c = '0'; c <= '9'; c++) table[c] = 3;
    }
    for (cp = (unsigned char *)str; (c = *cp++); n = table[c]) {
        if (table[c] < n) errormsg(SH_DICT, ERROR_warn(0), e_lextypeset, line, str);
    }
}

//
// a/d type definitions when compiling with -n.
//
static_fn void check_typedef(Lex_t *lp, struct comnod *tp) {
    char *cp = NULL;
    if (tp->comtyp & COMSCAN) {
        struct argnod *ap = tp->comarg;
        while ((ap = ap->argnxt.ap)) {
            if (!(ap->argflag & ARG_RAW) || strncmp(ap->argval, "--", 2)) break;
            if (lp->intypeset == 2) {
                if (*ap->argval == '-') {
                    continue;
                } else {
                    break;
                }
            }
            if (sh_isoption(lp->sh, SH_NOEXEC)) typeset_order(ap->argval, tp->comline);
            if (strncmp(ap->argval, "-T", 2) == 0) {
                if (ap->argval[2]) {
                    cp = ap->argval + 2;
                } else if ((ap->argnxt.ap)->argflag & ARG_RAW) {
                    cp = (ap->argnxt.ap)->argval;
                }
                if (cp) break;
            }
        }
    } else {
        struct dolnod *dp = (struct dolnod *)tp->comarg;
        char **argv = dp->dolval + dp->dolbot + 1;
        while ((cp = *argv++) && strncmp(cp, "--", 2)) {
            if (lp->intypeset == 2) {
                if (*cp == '-') continue;
                break;
            }
            if (sh_isoption(lp->sh, SH_NOEXEC)) typeset_order(cp, tp->comline);
            if (strncmp(cp, "-T", 2) == 0) {
                if (cp[2]) {
                    cp = cp + 2;
                } else {
                    cp = *argv;
                }
                break;
            }
        }
    }
    if (cp) {
        Namval_t *bp = sh_addbuiltin(lp->sh, cp, b_typeset, NULL);
        Namval_t *mp = tp->comnamp;
        nv_onattr(bp, mp->nvflag);
    }
}

//
// Make a parent node for fork() or io-redirection.
//
static_fn Shnode_t *makeparent(Lex_t *lp, int flag, Shnode_t *child) {
    Shnode_t *par = getnode(forknod);
    par->fork.forktyp = flag;
    par->fork.forktre = child;
    par->fork.forkio = NULL;
    par->fork.forkline = sh_getlineno(lp) - 1;
    return par;
}

static_fn bool paramsub(const char *str) {
    int c, sub = 0, lit = 0;

    while ((c = *str++)) {
        if (c == '$' && !lit) {
            if (*str == '(') return false;
            if (sub) continue;
            if (*str == '{') str++;
            if (!isdigit(*str) && strchr("?#@*!$ ", *str) == 0) return true;
        } else if (c == '`') {
            return false;
        } else if (c == '[' && !lit) {
            sub++;
        } else if (c == ']' && !lit) {
            sub--;
        } else if (c == '\'') {
            lit = !lit;
        }
    }
    return false;
}

static_fn Shnode_t *getanode(Lex_t *lp, struct argnod *ap) {
    Shnode_t *t = getnode(arithnod);

    t->ar.artyp = TARITH;
    t->ar.arline = sh_getlineno(lp);
    t->ar.arexpr = ap;
    if (ap->argflag & ARG_RAW) {
        t->ar.arcomp = sh_arithcomp(lp->sh, ap->argval);
    } else {
        if (sh_isoption(lp->sh, SH_NOEXEC) && (ap->argflag & ARG_MAC) && paramsub(ap->argval)) {
            errormsg(SH_DICT, ERROR_warn(0), e_lexwarnvar, lp->sh->inlineno);
        }
        t->ar.arcomp = NULL;
    }
    return t;
}

//
// Make a node corresponding to a command list.
//
static_fn Shnode_t *makelist(Lex_t *lexp, int type, Shnode_t *l, Shnode_t *r) {
    Shnode_t *t = NULL;

    if (!l || !r) {
        sh_syntax(lexp);
    } else {
        if ((type & COMMSK) == TTST) {
            t = getnode(tstnod);
        } else {
            t = getnode(lstnod);
        }
        t->lst.lsttyp = type;
        t->lst.lstlef = l;
        t->lst.lstrit = r;
    }
    return t;
}

//
// Entry to shell parser. Flag can be the union of SH_EOF|SH_NL.
//
Shnode_t *sh_parse(Shell_t *shp, Sfio_t *iop, int flag) {
    Shnode_t *t;
    Lex_t *lexp = shp->lex_context;
    Fcin_t sav_input;
    struct argnod *sav_arg = lexp->arg;
    int sav_prompt = shp->nextprompt;

    if (shp->binscript && (sffileno(iop) == shp->infd || (flag & SH_FUNEVAL))) {
        return sh_trestore(shp, iop);
    }
    fcsave(&sav_input);
    shp->st.staklist = NULL;
    lexp->assignlevel = 0;
    lexp->noreserv = 0;
    lexp->heredoc = NULL;
    lexp->inlineno = shp->inlineno;
    lexp->firstline = shp->st.firstline;
    lexp->fundepth = 0;
    shp->nextprompt = 1;
    loop_level = 0;
    label_list = label_last = 0;
    if (sh_isoption(shp, SH_INTERACTIVE)) sh_onstate(shp, SH_INTERACTIVE);
    if (sh_isoption(shp, SH_VERBOSE)) sh_onstate(shp, SH_VERBOSE);
    sh_lexopen(lexp, shp, 0);
    if (fcfopen(iop) < 0) return NULL;
    if (fcfile()) {
        char *cp = fcfirst();
        if (cp[0] == CNTL('k') && cp[1] == CNTL('s') && cp[2] == CNTL('h') && cp[3] == 0) {
            int version;
            fcseek(4);
            version = fcgetc();
            fcclose();
            fcrestore(&sav_input);
            lexp->arg = sav_arg;
            if (version > 3) {
                errormsg(SH_DICT, ERROR_exit(1), e_lexversion);
                __builtin_unreachable();
            }
            if (sffileno(iop) == shp->infd || (flag & SH_FUNEVAL)) shp->binscript = 1;
            sfgetc(iop);
            t = sh_trestore(shp, iop);
            if (flag & SH_NL) {
                Shnode_t *tt;
                while (1) {
                    if (!(tt = sh_trestore(shp, iop))) break;
                    t = makelist(lexp, TLST, t, tt);
                }
            }
            return t;
        }
    }
    flag &= ~SH_FUNEVAL;
    if ((flag & SH_NL) && (shp->inlineno = error_info.line + shp->st.firstline) == 0) {
        shp->inlineno = 1;
    }
    shp->nextprompt = 2;
    t = sh_cmd(lexp, (flag & SH_EOF) ? EOFSYM : '\n', SH_SEMI | SH_EMPTY | (flag & SH_NL));
    fcclose();
    fcrestore(&sav_input);
    lexp->arg = sav_arg;
    // Unstack any completed alias expansions.
    if ((sfset(iop, 0, 0) & SF_STRING) && !sfreserve(iop, 0, -1)) {
        Sfio_t *sp = sfstack(iop, NULL);
        if (sp) sfclose(sp);
    }
    shp->nextprompt = sav_prompt;
    if (flag & SH_NL) {
        shp->st.firstline = lexp->firstline;
        shp->inlineno = lexp->inlineno;
    }
    stkseek(shp->stk, 0);
    return t;
}

//
// This routine parses up the matching right parenthesis and returns the parse tree.
//
Shnode_t *sh_dolparen(Lex_t *lp) {
    Shnode_t *t = NULL;
    Sfio_t *sp = fcfile();
    int line = lp->sh->inlineno;

    lp->sh->inlineno = error_info.line + lp->sh->st.firstline;
    sh_lexopen(lp, lp->sh, 1);
    lp->comsub = 1;
    switch (sh_lex(lp)) {
        // ((...)) arithmetic expression.
        case EXPRSYM: {
            t = getanode(lp, lp->arg);
            break;
        }
        case LPAREN: {
            t = sh_cmd(lp, RPAREN, SH_NL | SH_EMPTY);
            break;
        }
        case LBRACE: {
            t = sh_cmd(lp, RBRACE, SH_NL | SH_EMPTY);
            break;
        }
        default: { break; }
    }
    lp->comsub = 0;
    if (!sp && (sp = fcfile())) {
        // This code handles the case where string has been converted to a file by an alias setup.
        int c;
        char *cp;
        c = fcgetc();
        if (c > 0) fcseek(-1);
        cp = fcseek(0);
        fcclose();
        fcsopen(cp);
        sfclose(sp);
    }
    lp->sh->inlineno = line;
    return t;
}

//
// Remove temporary files and stacks.
//
void sh_freeup(Shell_t *shp) {
    if (shp->st.staklist) sh_funstaks(shp->st.staklist, -1);
    shp->st.staklist = NULL;
}

//
// Increase reference count for each stack in function list when flag>0.
// Decrease reference count for each stack in function list when flag<=0.
// Stack is freed when reference count is zero.
//
void sh_funstaks(struct slnod *slp, int flag) {
    struct slnod *slpold;

    while ((slpold = slp)) {
        if (slp->slchild) sh_funstaks(slp->slchild, flag);
        slp = slp->slnext;
        if (flag <= 0) {
            stkclose(slpold->slptr);
        } else {
            stklink(slpold->slptr);
        }
    }
}

//
// cmd
//      empty
//      list
//      list & [ cmd ]
//      list [ ; cmd ]
//
static_fn Shnode_t *sh_cmd(Lex_t *lexp, int sym, int flag) {
    Shnode_t *left, *right;
    int type = FINT | FAMP;

    if (sym == NL) lexp->lasttok = 0;
    left = list(lexp, flag);
    if (lexp->token == NL) {
        if (flag & SH_NL) lexp->token = ';';
    } else if (!left && !(flag & SH_EMPTY)) {
        sh_syntax(lexp);
    }
    switch (lexp->token) {
        case COOPSYM: {  // set up a cooperating process
            type |= (FPIN | FPOU | FPCL | FCOOP);
        }
        // FALLTHRU
        case '&': {
            if (left) {
                // (...)& -> {...;} &
                if (left->tre.tretyp == TPAR) left = left->par.partre;
                left = makeparent(lexp, TFORK | type, left);
            }
        }
        // FALLTHRU
        case ';': {
            if (!left) sh_syntax(lexp);
            right = sh_cmd(lexp, sym, flag | SH_EMPTY);
            if (right) {
                left = makelist(lexp, TLST, left, right);
            }
            break;
        }
        case EOFSYM: {
            if (sym == NL) break;
        }
        // FALLTHRU
        default: {
            if (sym && sym != lexp->token) {
                if (sym != ELSESYM || (lexp->token != ELIFSYM && lexp->token != FISYM)) {
                    sh_syntax(lexp);
                }
            }
        }
    }
    return left;
}

//
// list
//      term
//      list && term
//      list || term
//      unfortunately, these are equal precedence
//
static_fn Shnode_t *list(Lex_t *lexp, int flag) {
    Shnode_t *t = term(lexp, flag);
    int token;

    while (t && ((token = lexp->token) == ANDFSYM || token == ORFSYM)) {
        t = makelist(lexp, (token == ANDFSYM ? TAND : TORF), t, term(lexp, SH_NL | SH_SEMI));
    }
    return t;
}

//
// term
//      item
//      item | term
//
static_fn Shnode_t *term(Lex_t *lexp, int flag) {
    Shnode_t *t;
    int token;

    if (flag & SH_NL) {
        token = skipnl(lexp, flag);
    } else {
        token = sh_lex(lexp);
    }
    // Check to see if pipeline is to be timed.
    if (token == TIMESYM || token == NOTSYM) {
        t = getnode(parnod);
        t->par.partyp = TTIME;
        if (lexp->token == NOTSYM) t->par.partyp |= COMSCAN;
        t->par.partre = term(lexp, 0);
    } else if ((t = item(lexp, SH_NL | SH_EMPTY | (flag & SH_SEMI))) && lexp->token == '|') {
        Shnode_t *tt;
        int showme = t->tre.tretyp & FSHOWME;
        t = makeparent(lexp, TFORK | FPOU, t);
        tt = term(lexp, SH_NL);
        if (tt) {
            switch (tt->tre.tretyp & COMMSK) {
                case TFORK: {
                    tt->tre.tretyp |= FPIN | FPCL;
                    break;
                }
                case TFIL: {
                    tt->lst.lstlef->tre.tretyp |= FPIN | FPCL;
                    break;
                }
                default: { tt = makeparent(lexp, TSETIO | FPIN | FPCL, tt); }
            }
            t = makelist(lexp, TFIL, t, tt);
            t->tre.tretyp |= showme;
        } else if (lexp->token) {
            sh_syntax(lexp);
        }
    }
    return t;
}

//
// Case statement.
//
static_fn struct regnod *syncase(Lex_t *lexp, int esym) {
    int tok = skipnl(lexp, 0);

    if (tok == esym) return NULL;

    struct regnod *r = stkalloc(stkstd, sizeof(struct regnod));
    r->regptr = NULL;
    r->regflag = 0;
    if (tok == LPAREN) skipnl(lexp, 0);
    while (1) {
        if (!lexp->arg) sh_syntax(lexp);
        lexp->arg->argnxt.ap = r->regptr;
        r->regptr = lexp->arg;
        if ((tok = sh_lex(lexp)) == RPAREN) {
            break;
        } else if (tok == '|') {
            sh_lex(lexp);
        } else {
            sh_syntax(lexp);
        }
    }
    r->regcom = sh_cmd(lexp, 0, SH_NL | SH_EMPTY | SH_SEMI);
    if ((tok = lexp->token) == BREAKCASESYM) {
        r->regnxt = syncase(lexp, esym);
    } else if (tok == FALLTHRUSYM) {
        r->regflag++;
        r->regnxt = syncase(lexp, esym);
    } else {
        if (tok != esym && tok != EOFSYM) sh_syntax(lexp);
        r->regnxt = NULL;
    }
    if (lexp->token == EOFSYM) return NULL;
    return r;
}

//
// This routine creates the parse tree for the arithmetic for what? When called, shlex.arg contains
// the string inside ((...)). When the first argument is missing, a while node is returned. Otherise
// a list containing an arithmetic command and a while is returned.
//
static_fn Shnode_t *arithfor(Lex_t *lexp, Shnode_t *tf) {
    Shnode_t *t, *tw = tf;
    int offset;
    struct argnod *argp;
    int n;
    Stk_t *stkp = lexp->sh->stk;
    int argflag = lexp->arg->argflag;

    // Save current input.
    Fcin_t sav_input;
    fcsave(&sav_input);
    fcsopen(lexp->arg->argval);
    // Split ((...)) into three expressions.
    for (n = 0;; n++) {
        int c;
        argp = stkseek(stkp, ARGVAL);
        argp->argnxt.ap = NULL;
        argp->argchn.cp = NULL;
        argp->argflag = argflag;
        if (n == 2) break;
        // Copy up to ; onto the stack.
        sh_lexskip(lexp, ';', 1, ST_NESTED);
        offset = stktell(stkp) - 1;
        if ((c = fcpeek(-1)) != ';') break;
        // Remove trailing white space.
        while (offset > ARGVAL && ((c = *stkptr(stkp, offset - 1)), isspace(c))) offset--;
        // Check for empty initialization expression.
        if (offset == ARGVAL && n == 0) continue;
        stkseek(stkp, offset);
        // Check for empty condition and treat as while((1)).
        if (offset == ARGVAL) sfputc(stkp, '1');
        argp = (struct argnod *)stkfreeze(stkp, 1);
        t = getanode(lexp, argp);
        if (n == 0) {
            tf = makelist(lexp, TLST, t, tw);
        } else {
            tw->wh.whtre = t;
        }
    }
    while ((offset = fcpeek(0)) && isspace(offset)) fcseek(1);
    sfputr(stkstd, fcseek(0), 0);
    --stkstd->next;
    argp = (struct argnod *)stkfreeze(stkstd, 1);
    fcrestore(&sav_input);
    if (n < 2) {
        lexp->token = RPAREN | SYMREP;
        sh_syntax(lexp);
    }
    // Check whether the increment is present.
    if (*argp->argval) {
        t = getanode(lexp, argp);
        tw->wh.whinc = (struct arithnod *)t;
    } else {
        tw->wh.whinc = NULL;
    }
    sh_lexopen(lexp, lexp->sh, 1);
    if ((n = sh_lex(lexp)) == NL) {
        n = skipnl(lexp, 0);
    } else if (n == ';') {
        n = sh_lex(lexp);
    }
    if (n != DOSYM && n != LBRACE) sh_syntax(lexp);
    tw->wh.dotre = sh_cmd(lexp, n == DOSYM ? DONESYM : RBRACE, SH_NL | SH_SEMI);
    tw->wh.whtyp = TWH;
    return tf;
}

static_fn Shnode_t *funct(Lex_t *lexp) {
    Shell_t *shp = lexp->sh;
    Shnode_t *t;
    int flag;
    struct slnod *volatile slp = NULL;
    Sfio_t *volatile savstak = NULL;
    Sfoff_t first, last;
    struct functnod *volatile fp = NULL;
    Sfio_t *iop;
    unsigned long current = lexp->current;
    int nargs = 0, size = 0, jmpval, saveloop = loop_level;
    struct argnod *savelabel = label_last;
    checkpt_t buff;
    int save_optget = opt_get;
    void *in_mktype = shp->mktype;

    shp->mktype = NULL;
    opt_get = 0;
    t = getnode(functnod);
    t->funct.functline = shp->inlineno;
    t->funct.functtyp = TFUN;
    t->funct.functargs = NULL;
    if (!(flag = (lexp->token == FUNCTSYM))) {
        t->funct.functtyp |= FPOSIX;
    } else if (sh_lex(lexp)) {
        sh_syntax(lexp);
    }
    lexp->fundepth++;
    if (!(iop = fcfile())) {
        iop = sfopen(NULL, fcseek(0), "s");
        fcclose();
        fcfopen(iop);
    }
    t->funct.functloc = first = fctell();
    if (!shp->st.filename || sffileno(iop) < 0) {
        if (fcfill() >= 0) fcseek(-1);
        if (sh_isstate(shp, SH_HISTORY) && shp->gd->hist_ptr) {
            t->funct.functloc = sfseek(shp->gd->hist_ptr->histfp, (off_t)0, SEEK_CUR);
        } else {
            // Copy source to temporary file.
            t->funct.functloc = 0;
            if (lexp->sh->heredocs) {
                t->funct.functloc = sfseek(lexp->sh->heredocs, (Sfoff_t)0, SEEK_END);
            } else {
                lexp->sh->heredocs = sftmp(HERE_MEM);
            }
            lexp->sh->funlog = lexp->sh->heredocs;
            t->funct.functtyp |= FPIN;
        }
    }
    t->funct.functnam = (char *)lexp->arg->argval;
    if (lexp->kiafile) {
        lexp->current =
            kiaentity(lexp, t->funct.functnam, -1, 'p', -1, -1, lexp->script, 'p', 0, "");
    }
    if (flag) {
        lexp->token = sh_lex(lexp);
#if SHOPT_BASH
        if (lexp->token == LPAREN) {
            if ((lexp->token = sh_lex(lexp)) == RPAREN) {
                t->funct.functtyp |= FPOSIX;
            } else {
                sh_syntax(lexp);
            }
        }
#endif
    }
    if (t->funct.functtyp & FPOSIX) {
        skipnl(lexp, 0);
    } else {
        if (lexp->token == 0) {
            struct comnod *ac;
            char *cp, **argv, **argv0;
            int c = 0;
            t->funct.functargs = ac = (struct comnod *)simple(lexp, SH_NOIO | SH_FUNDEF, NULL);
            if (ac->comset || (ac->comtyp & COMSCAN)) {
                errormsg(SH_DICT, ERROR_exit(3), e_lexsyntax4, lexp->sh->inlineno);
                __builtin_unreachable();
            }
            argv0 = argv = ((struct dolnod *)ac->comarg)->dolval + ARG_SPARE;
            while ((cp = *argv++)) {
                size += strlen(cp) + 1;
                c = mb1char(&cp);
                if (isaletter(c)) {
                    while (c = mb1char(&cp), isaname(c)) {
                        ;  // empty body
                    }
                }
            }
            if (c) {
                errormsg(SH_DICT, ERROR_exit(3), e_lexsyntax4, lexp->sh->inlineno);
                __builtin_unreachable();
            }
            nargs = argv - argv0;
            size += sizeof(struct dolnod) + (nargs + ARG_SPARE) * sizeof(char *);
            if (shp->shcomp && strncmp(".sh.math.", t->funct.functnam, 9) == 0) {
                Namval_t *np = nv_open(t->funct.functnam, shp->fun_tree, NV_ADD | NV_VARNAME);
                STORE_VT(
                    np->nvalue, rp,
                    calloc(1, sizeof(struct Ufunction) + (shp->funload ? sizeof(Dtlink_t) : 0)));
                memset(FETCH_VT(np->nvalue, rp), 0, sizeof(struct Ufunction));
                FETCH_VT(np->nvalue, rp)->argc = ((struct dolnod *)ac->comarg)->dolnum;
            }
        }
        while (lexp->token == NL) lexp->token = sh_lex(lexp);
    }
    if ((flag && lexp->token != LBRACE) || lexp->token == EOFSYM) sh_syntax(lexp);
    sh_pushcontext(shp, &buff, 1);
    jmpval = sigsetjmp(buff.buff, 0);
    if (jmpval == 0) {
        // Create a new stak frame to compile the command.
        savstak = stkopen(STK_SMALL);
        savstak = stkinstall(savstak, 0);
        slp = stkalloc(stkstd, sizeof(struct slnod) + sizeof(struct functnod));
        slp->slchild = NULL;
        slp->slnext = shp->st.staklist;
        shp->st.staklist = NULL;
        t->funct.functstak = (struct slnod *)slp;
        // Store the pathname of function definition file on stack in name field of fake for node.
        fp = (struct functnod *)(slp + 1);
        fp->functtyp = TFUN | FAMP;
        fp->functnam = NULL;
        fp->functargs = NULL;
        fp->functline = t->funct.functline;
        if (shp->st.filename) fp->functnam = stkcopy(stkstd, shp->st.filename);
        loop_level = 0;
        label_last = label_list;
        if (size) {
            struct dolnod *dp = stkalloc(stkstd, size);
            char *cp, *sp, **argv;
            char **old = ((struct dolnod *)t->funct.functargs->comarg)->dolval + 1;
            argv = dp->dolval + 1;
            dp->dolnum = ((struct dolnod *)t->funct.functargs->comarg)->dolnum;
            t->funct.functargs->comarg = (struct argnod *)dp;
            for (cp = (char *)&argv[nargs]; (sp = *old++); cp++) {
                *argv++ = cp;
                cp = stpcpy(cp, sp);
            }
            *argv = 0;
        }
        if (!flag && lexp->token == 0) {
            // Copy current word token to current stak frame.
            struct argnod *ap;
            flag = ARGVAL + strlen(lexp->arg->argval);
            ap = stkalloc(stkstd, flag);
            memcpy(ap, lexp->arg, flag);
            lexp->arg = ap;
        }
        t->funct.functtre = item(lexp, SH_NOIO);
    } else if (shp->shcomp) {
        exit(1);
    } else {
        // Since `fp` is volatile simply evaluating it for not NULL can have a side-effect. To avoid
        // differences in behavior between debug/non-debug builds we can't use assert() since it may
        // or may not be a no-op.
        if (!fp) abort();
    }
    sh_popcontext(shp, &buff);
    loop_level = saveloop;
    label_last = savelabel;
    // Restore the old stack.
    if (slp) {
        slp->slptr = stkinstall(savstak, 0);
        slp->slchild = shp->st.staklist;
    }
    lexp->current = current;
    if (jmpval) {
        if (slp && slp->slptr) {
            shp->st.staklist = slp->slnext;
            stkclose(slp->slptr);
        }
        siglongjmp(shp->jmplist->buff, jmpval);
    }
    shp->st.staklist = (struct slnod *)slp;
    last = fctell();
    // It should be impossible for `fp` to be NULL since a longjmp() should not occur before it has
    // been assigned a non-NULL value in the `if (jmpval == 0) {...}` block above.
    fp->functline = last - first;
    fp->functtre = t;
    shp->mktype = in_mktype;
    if (lexp->sh->funlog) {
        if (fcfill() > 0) fcseek(-1);
        lexp->sh->funlog = NULL;
    }
    if (lexp->kiafile) {
        kiaentity(lexp, t->funct.functnam, -1, 'p', t->funct.functline, shp->inlineno - 1,
                  lexp->current, 'p', 0, "");
    }
    t->funct.functtyp |= opt_get;
    opt_get = save_optget;
    lexp->fundepth--;
    return t;
}

static_fn bool check_array(Lex_t *lexp) {
    if (lexp->token != 0) return false;

    assert(lexp->arg);
    if (strcmp(lexp->arg->argval, "typeset") != 0) return false;

    int c;
    while ((c = fcgetc()) == ' ' || c == '\t') {
        ;  // empty loop
    }
    if (c == '-') {
        if (fcgetc() == 'a') {
            lexp->assignok = SH_ASSIGN;
            lexp->noreserv = 1;
            sh_lex(lexp);
            return true;
        } else {
            fcseek(-2);
        }
    } else {
        fcseek(-1);
    }
    return false;
}

//
// Compound assignment.
//
static_fn struct argnod *parse_assign(Lex_t *lexp, struct argnod *ap, nvflag_t type) {
    int n;
    Shnode_t *t, **tp;
    struct comnod *ac;
    Stk_t *stkp = lexp->sh->stk;
    int array = 0, index = 0;
    Namval_t *np;

    lexp->assignlevel++;
    n = strlen(ap->argval) - 1;
    if (ap->argval[n] != '=') sh_syntax(lexp);
    if (ap->argval[n - 1] == '+') {
        ap->argval[n--] = 0;
        array = ARG_APPEND;
        type |= NV_APPEND;
    }
    // Shift right.
    while (n > 0) {
        ap->argval[n] = ap->argval[n - 1];
        n--;
    }
    *ap->argval = 0;
    t = getnode(fornod);
    t->for_.fornam = (char *)(ap->argval + 1);
    t->for_.fortyp = sh_getlineno(lexp);
    tp = &t->for_.fortre;
    ap->argchn.ap = (struct argnod *)t;
    ap->argflag &= ARG_QUOTED;
    ap->argflag |= array;
    lexp->assignok = SH_ASSIGN;
    if (type & NV_ARRAY) {
        lexp->noreserv = 1;
        lexp->assignok = 0;
    } else {
        lexp->aliasok = 2;
    }
    array = (type & NV_ARRAY) ? SH_ARRAY : 0;
    if ((n = skipnl(lexp, 0)) == RPAREN || n == LPAREN) {
        struct argnod *ar, *aq, **settail;
        ac = (struct comnod *)getnode(comnod);
        memset(ac, 0, sizeof(*ac));
    comarray:
        settail = &ac->comset;
        ac->comline = sh_getlineno(lexp);
        while (n == LPAREN) {
            ar = stkseek(stkp, ARGVAL);
            ar->argflag = ARG_ASSIGN;
            sfprintf(stkp, "[%d]=", index++);
            aq = ac->comarg;
            if (aq) {
                ac->comarg = aq->argnxt.ap;
                sfprintf(stkp, "%s", aq->argval);
                ar->argflag |= aq->argflag;
            }
            ar = (struct argnod *)stkfreeze(stkp, 1);
            ar->argnxt.ap = NULL;
            if (!aq) ar = parse_assign(lexp, ar, 0);
            ar->argflag |= ARG_MESSAGE;
            *settail = ar;
            settail = &(ar->argnxt.ap);
            if (aq) continue;
            while ((n = skipnl(lexp, 0)) == 0) {
                ar = stkseek(stkp, ARGVAL);
                ar->argflag = ARG_ASSIGN;
                sfprintf(stkp, "[%d]=", index++);
                sfputr(stkstd, lexp->arg->argval, 0);
                --stkstd->next;
                ar = (struct argnod *)stkfreeze(stkp, 1);
                ar->argnxt.ap = NULL;
                ar->argflag = lexp->arg->argflag;
                *settail = ar;
                settail = &(ar->argnxt.ap);
            }
        }
    } else if (n && n != FUNCTSYM) {
        sh_syntax(lexp);
    } else if (type != NV_ARRAY && n != FUNCTSYM && !(lexp->arg->argflag & ARG_ASSIGN) &&
               !((np = nv_search(lexp->arg->argval, lexp->sh->fun_tree, 0)) &&
                 (nv_isattr(np, BLT_DCL) || np == SYSDOT))) {
        array = SH_ARRAY;
        n = fcgetc();
        if (n == LPAREN) {
            int c = fcgetc();
            if (c == RPAREN) {
                lexp->token = SYMRES;
                array = 0;
            } else {
                fcseek(-2);
            }
        } else if (n > 0) {
            fcseek(-1);
        }
        if (array && type == NV_TYPE) {
            struct argnod *arg = lexp->arg;
            n = lexp->token;
            if (path_search(lexp->sh, lexp->arg->argval, NULL, 1) &&
                (np = nv_search(lexp->arg->argval, lexp->sh->fun_tree, 0)) &&
                nv_isattr(np, BLT_DCL)) {
                lexp->token = n;
                lexp->arg = arg;
                array = 0;
            } else {
                sh_syntax(lexp);
            }
        }
    }
    lexp->noreserv = 0;
    while (1) {
        if ((n = lexp->token) == RPAREN) break;
        if (n == FUNCTSYM || n == SYMRES) {
            ac = (struct comnod *)funct(lexp);
        } else {
            ac = (struct comnod *)simple(lexp, SH_NOIO | SH_ASSIGN | type | array, NULL);
        }
        if ((n = lexp->token) == RPAREN) break;
        if (n != NL && n != ';') {
            if (array && n == LPAREN) goto comarray;
            sh_syntax(lexp);
        }
        lexp->assignok = SH_ASSIGN;
        if ((n = skipnl(lexp, 0)) || array) {
            if (n == RPAREN) {
                n = fcgetc();
                if (n != ';' && n > 0) fcseek(-LEN);
                break;
            }
            if (array || n != FUNCTSYM) sh_syntax(lexp);
        }
        if ((n != FUNCTSYM) && !(lexp->arg->argflag & ARG_ASSIGN) &&
            !((np = nv_search(lexp->arg->argval, lexp->sh->fun_tree, 0)) &&
              (nv_isattr(np, BLT_DCL) || np == SYSDOT))) {
            struct argnod *arg = lexp->arg;
            if (n != 0) sh_syntax(lexp);
            // Check for sys5 style function.
            if (sh_lex(lexp) != LPAREN || sh_lex(lexp) != RPAREN) {
                lexp->arg = arg;
                lexp->token = 0;
                sh_syntax(lexp);
            }
            lexp->arg = arg;
            lexp->token = SYMRES;
        }
        t = makelist(lexp, TLST, (Shnode_t *)ac, t);
        *tp = t;
        tp = &t->lst.lstrit;
    }
    *tp = (Shnode_t *)ac;
    lexp->assignok = 0;
    lexp->assignlevel--;
    return ap;
}

//
// item
//
//    ( cmd ) [ < in ] [ > out ]
//    word word* [ < in ] [ > out ]
//    if ... then ... else ... fi
//    for ... while ... do ... done
//    case ... in ... esac
//    begin ... end
//
static_fn Shnode_t *item(Lex_t *lexp, int flag) {
    Shnode_t *t;
    struct ionod *io;
    int tok = (lexp->token & 0xff);
    int savwdval = lexp->lasttok;
    int savline = lexp->lastline;
    int showme = 0, comsub;

    if (!(flag & SH_NOIO) && (tok == '<' || tok == '>' || lexp->token == IOVNAME)) {
        io = inout(lexp, NULL, 1);
    } else {
        io = 0;
    }
    if ((tok = lexp->token) && tok != EOFSYM && tok != FUNCTSYM) {
        lexp->lastline = sh_getlineno(lexp);
        lexp->lasttok = lexp->token;
    }
    switch (tok) {
        case BTESTSYM: {  // [[ ... ]] test expression
            t = test_expr(lexp, ETESTSYM);
            t->tre.tretyp &= ~TTEST;
            break;
        }
        case EXPRSYM: {  // ((...)) arithmetic expression
            t = getanode(lexp, lexp->arg);
            sh_lex(lexp);
            goto done;
        }
        case CASESYM: {  // case statement
            int savetok = lexp->lasttok;
            int saveline = lexp->lastline;
            t = getnode(swnod);
            if (sh_lex(lexp)) sh_syntax(lexp);
            t->sw.swarg = lexp->arg;
            t->sw.swtyp = TSW;
            t->sw.swio = NULL;
            t->sw.swtyp |= FLINENO;
            t->sw.swline = lexp->sh->inlineno;
            if ((tok = skipnl(lexp, 0)) != INSYM && tok != LBRACE) sh_syntax(lexp);
            if (!(t->sw.swlst = syncase(lexp, tok == INSYM ? ESACSYM : RBRACE)) &&
                lexp->token == EOFSYM) {
                lexp->lasttok = savetok;
                lexp->lastline = saveline;
                sh_syntax(lexp);
            }
            break;
        }
        case IFSYM: {  // if statement
            Shnode_t *tt = NULL;
            t = getnode(ifnod);
            t->if_.iftyp = TIF;
            t->if_.iftre = sh_cmd(lexp, THENSYM, SH_NL);
            t->if_.thtre = sh_cmd(lexp, ELSESYM, SH_NL | SH_SEMI);
            tok = lexp->token;
            t->if_.eltre =
                (tok == ELSESYM
                     ? sh_cmd(lexp, FISYM, SH_NL | SH_SEMI)
                     : (tok == ELIFSYM ? (lexp->token = IFSYM, tt = item(lexp, SH_NOIO)) : 0));
            if (tok == ELIFSYM) {
                if (!tt || tt->tre.tretyp != TSETIO) goto done;
                t->if_.eltre = tt->fork.forktre;
                tt->fork.forktre = t;
                t = tt;
                goto done;
            }
            break;
        }
        case FORSYM:
        case SELECTSYM: {  // for and select statement
            t = getnode(fornod);
            t->for_.fortyp = (lexp->token == FORSYM ? TFOR : TSELECT);
            t->for_.forlst = NULL;
            t->for_.forline = lexp->sh->inlineno;
            if (sh_lex(lexp)) {
                if (lexp->token != EXPRSYM || t->for_.fortyp != TFOR) sh_syntax(lexp);
                // arithmetic for.
                t = arithfor(lexp, t);
                break;
            }
            t->for_.fornam = (char *)lexp->arg->argval;
            t->for_.fortyp |= FLINENO;
            if (lexp->kiafile) writedefs(lexp, lexp->arg, lexp->sh->inlineno, 'v', NULL);
            while ((tok = sh_lex(lexp)) == NL) {
                ;  // empty body
            }
            if (tok == INSYM) {
                if (sh_lex(lexp)) {
                    if (lexp->token != NL && lexp->token != ';') sh_syntax(lexp);
                    // Some Linux scripts assume this.
                    if (sh_isoption(lexp->sh, SH_NOEXEC)) {
                        errormsg(SH_DICT, ERROR_warn(0), e_lexemptyfor,
                                 lexp->sh->inlineno - (lexp->token == '\n'));
                    }
                    t->for_.forlst = (struct comnod *)getnode(comnod);
                    (t->for_.forlst)->comarg = NULL;
                    (t->for_.forlst)->comset = NULL;
                    (t->for_.forlst)->comnamp = NULL;
                    (t->for_.forlst)->comnamq = NULL;
                    (t->for_.forlst)->comstate = NULL;
                    (t->for_.forlst)->comio = NULL;
                    (t->for_.forlst)->comtyp = 0;
                } else {
                    t->for_.forlst = (struct comnod *)simple(lexp, SH_NOIO, NULL);
                }
                if (lexp->token != NL && lexp->token != ';') sh_syntax(lexp);
                tok = skipnl(lexp, 0);
            } else if (tok == ';') {  // 'for i;do cmd' is valid syntax
                while ((tok = sh_lex(lexp)) == NL) {
                    ;  // empty loop
                }
            }
            if (tok != DOSYM && tok != LBRACE) sh_syntax(lexp);
            loop_level++;
            t->for_.fortre = sh_cmd(lexp, tok == DOSYM ? DONESYM : RBRACE, SH_NL | SH_SEMI);
            if (--loop_level == 0) label_last = label_list;
            break;
        }
        case FUNCTSYM: {  // this is the code for parsing function definitions
            return funct(lexp);
        }
        case NSPACESYM: {
            if (lexp->fundepth) sh_syntax(lexp);
            t = getnode(functnod);
            t->funct.functtyp = TNSPACE;
            t->funct.functargs = NULL;
            t->funct.functloc = 0;
            if (sh_lex(lexp)) sh_syntax(lexp);
            t->funct.functnam = (char *)lexp->arg->argval;
            while ((tok = sh_lex(lexp)) == NL) {
                ;  // empty loop
            }
            if (tok != LBRACE) sh_syntax(lexp);
            t->funct.functtre = sh_cmd(lexp, RBRACE, SH_NL);
            break;
        }
        case WHILESYM:
        case UNTILSYM: {  // while and until
            t = getnode(whnod);
            t->wh.whtyp = (lexp->token == WHILESYM ? TWH : TUN);
            loop_level++;
            t->wh.whtre = sh_cmd(lexp, DOSYM, SH_NL);
            t->wh.dotre = sh_cmd(lexp, DONESYM, SH_NL | SH_SEMI);
            if (--loop_level == 0) label_last = label_list;
            t->wh.whinc = NULL;
            break;
        }
        case LABLSYM: {
            struct argnod *argp = label_list;
            while (argp) {
                if (strcmp(argp->argval, lexp->arg->argval) == 0) {
                    errormsg(SH_DICT, ERROR_exit(3), e_lexsyntax3, lexp->sh->inlineno,
                             argp->argval);
                    __builtin_unreachable();
                }
                argp = argp->argnxt.ap;
            }
            lexp->arg->argnxt.ap = label_list;
            label_list = lexp->arg;
            label_list->argchn.len = sh_getlineno(lexp);
            label_list->argflag = loop_level;
            skipnl(lexp, flag);
            if (!(t = item(lexp, SH_NL))) sh_syntax(lexp);
            tok = (t->tre.tretyp & (COMSCAN | COMMSK));
            if (sh_isoption(lexp->sh, SH_NOEXEC) && tok != TWH && tok != TUN && tok != TFOR &&
                tok != TSELECT) {
                errormsg(SH_DICT, ERROR_warn(0), e_lexlabignore, label_list->argchn.len,
                         label_list->argval);
            }
            return t;
        }
        case LBRACE: {  // command group with {...}
            comsub = lexp->comsub;
            lexp->comsub = 0;
            t = sh_cmd(lexp, RBRACE, SH_NL | SH_SEMI);
            lexp->comsub = comsub;
            break;
        }
        case LPAREN: {
            t = getnode(parnod);
            t->par.partre = sh_cmd(lexp, RPAREN, SH_NL | SH_SEMI);
            t->par.partyp = TPAR;
            break;
        }
        default: {
            if (io == 0) return NULL;
        }
        // FALLTHRU
        case ';': {
            if (io == 0) {
                if (!(flag & SH_SEMI)) return NULL;
                if (sh_lex(lexp) == ';') sh_syntax(lexp);
                showme = FSHOWME;
            }
        }
        // FALLTHRU
        case 0: {  // simple command
            t = simple(lexp, flag, io);
            if (t->com.comarg && lexp->intypeset) check_typedef(lexp, &t->com);
            lexp->intypeset = 0;
            lexp->inexec = 0;
            t->tre.tretyp |= showme;
            return t;
        }
    }
    sh_lex(lexp);
done:
    io = inout(lexp, io, 0);
    if (io) {
        if ((tok = t->tre.tretyp & COMMSK) != TFORK) tok = TSETIO;
        t = makeparent(lexp, tok, t);
        t->tre.treio = io;
    }
    lexp->lasttok = savwdval;
    lexp->lastline = savline;
    return t;
}

static_fn struct argnod *process_sub(Lex_t *lexp, int tok) {
    struct argnod *argp;
    Shnode_t *t;
    int mode = (tok == OPROCSYM);

    t = sh_cmd(lexp, RPAREN, SH_NL);
    argp = stkalloc(lexp->sh->stk, sizeof(struct argnod));
    *argp->argval = 0;
    argp->argchn.ap =
        (struct argnod *)makeparent(lexp, mode ? TFORK | FPIN | FAMP | FPCL : TFORK | FPOU, t);
    argp->argflag = (ARG_EXP | mode);
    return argp;
}

//
// This is for a simple command, for list, or compound assignment.
//
static_fn Shnode_t *simple(Lex_t *lexp, int flag, struct ionod *io) {
    struct comnod *t;
    struct argnod *argp;
    int tok;
    Stk_t *stkp = lexp->sh->stk;
    struct argnod **argtail;
    struct argnod **settail;
    bool typed = false;
    int cmdarg = 0;
    int argno = 0;
    nvflag_t type = 0;
    int was_assign = 0;
    int assignment = 0;
    int key_on = (!(flag & SH_NOIO) && sh_isoption(lexp->sh, SH_KEYWORD));
    int procsub = 0, associative = 0;

    Namval_t *np = NULL;
    argp = lexp->arg;
    if (argp && (argp->argflag & ARG_ASSIGN) && argp->argval[0] == '[') {
        flag |= SH_ARRAY;
        associative = 1;
    }
    t = (struct comnod *)getnode(comnod);
    t->comio = io;                    // initial io chain
    t->comline = sh_getlineno(lexp);  // set command line number for error messages
    argtail = &(t->comarg);
    t->comset = NULL;
    t->comnamp = NULL;
    t->comnamq = NULL;
    t->comstate = NULL;
    settail = &(t->comset);
    if (lexp->assignlevel && (flag & SH_ARRAY) && check_array(lexp)) type |= NV_ARRAY;
    while (lexp->token == 0) {
        was_assign = 0;
        argp = lexp->arg;
        assert(argp);
        if (*argp->argval == LBRACE && (flag & SH_FUNDEF) && argp->argval[1] == 0) {
            lexp->token = LBRACE;
            break;
        }
        if (associative && argp->argval[0] != '[') sh_syntax(lexp);
        // Check for assignment argument.
        if ((argp->argflag & ARG_ASSIGN) && assignment != 2) {
            if (lexp->typed && assignment == 0) {
                typed = true;
                assignment = 1;
            }
            *settail = argp;
            settail = &(argp->argnxt.ap);
            lexp->assignok = (flag & SH_ASSIGN) ? SH_ASSIGN : 1;
            if (strncmp(argp->argval, ".sh.value", 9) == 0) opt_get |= FSHVALUE;
            if (assignment) {
                struct argnod *ap = argp;
                char *last, *cp;
                if (assignment == 1) {
                    last = strchr(argp->argval, '=');
                    assert(last);
                    if (last[-1] == ']' || (last[-1] == '+' && last[-2] == ']')) {
                        cp = strchr(argp->argval, '[');
                        if (cp && cp < last && cp[-1] != '.') last = cp;
                    }
                    stkseek(stkp, ARGVAL);
                    sfwrite(stkp, argp->argval, last - argp->argval);
                    sfputc(stkp, 0);
                    sfseek(stkp, (Sfoff_t)-1, SEEK_CUR);

                    if (np && FETCH_VT(np->nvalue, shbltinp) != b_alias &&
                        strchr(stkptr(stkp, ARGVAL), '[')) {
                        sfputc(stkp, '@');
                    }
                    ap = (struct argnod *)stkfreeze(stkp, 1);
                    ap->argflag = ARG_RAW;
                    ap->argchn.ap = NULL;
                }
                *argtail = ap;
                argtail = &(ap->argnxt.ap);
                if (argno >= 0) argno++;
            } else {  // alias substitutions allowed
                lexp->aliasok = 1;
            }
        } else {
            if (!(argp->argflag & ARG_RAW)) argno = -1;
            if (argno >= 0 && argno++ == cmdarg && !(flag & SH_ARRAY) && *argp->argval != '/') {
                // Check for builtin command.
                if (*argp->argval != '_' || argp->argval[1] != '.') {
                    np = nv_bfsearch(argp->argval, lexp->sh->fun_tree, (Namval_t **)&t->comnamq,
                                     NULL);
                }
                if (cmdarg == 0) t->comnamp = np;
                if (np && is_abuiltin(np)) {
                    if (nv_isattr(np, BLT_DCL)) {
                        assignment = 1 + !strcmp(argp->argval, "alias");
                        if (np == SYSTYPESET) {
                            lexp->intypeset = 1;
                        } else if (np == SYSENUM) {
                            lexp->intypeset = 2;
                        }
                        key_on = 1;
                    } else if (np == SYSCOMMAND) {
                        cmdarg++;
                    } else if (np == SYSEXEC) {
                        lexp->inexec = 1;
                    } else if (FETCH_VT(np->nvalue, shbltinp) == b_getopts) {
                        opt_get |= FOPTGET;
                    }
                }
            }
            if ((flag & NV_COMVAR) && !assignment) sh_syntax(lexp);
            *argtail = argp;
            argtail = &(argp->argnxt.ap);
            if (!(lexp->assignok = key_on) && !(flag & SH_NOIO) &&
                sh_isoption(lexp->sh, SH_NOEXEC)) {
                lexp->assignok = SH_COMPASSIGN;
            }
            lexp->aliasok = 0;
        }
    retry:
        tok = sh_lex(lexp);
        if (tok == ';' && was_assign) tok = '\n';
        if (was_assign && check_array(lexp)) type = NV_ARRAY;
        if (tok == 0 && procsub && (lexp->arg->argflag & ARG_ASSIGN)) {
            lexp->arg->argflag &= ~ARG_ASSIGN;
        }
        procsub = 0;
        if (tok == LABLSYM && (flag & SH_ASSIGN)) lexp->token = tok = 0;
        if ((tok == IPROCSYM || tok == OPROCSYM)) {
            argp = process_sub(lexp, tok);
            argno = -1;
            *argtail = argp;
            argtail = &(argp->argnxt.ap);
            procsub = 1;
            goto retry;
        }
        if (tok == LPAREN) {
            if (argp->argflag & ARG_ASSIGN) {
                int intypeset = lexp->intypeset;
                lexp->intypeset = 0;
                if (t->comnamp == SYSTYPESET) {
                    struct argnod *ap;
                    for (ap = t->comarg->argnxt.ap; ap; ap = ap->argnxt.ap) {
                        if (*ap->argval != '-') break;
                        if (strchr(ap->argval, 'T')) {
                            type |= NV_TYPE;
                        } else if (strchr(ap->argval, 'a')) {
                            type |= NV_ARRAY;
                        } else if (strchr(ap->argval, 'C')) {
                            type |= NV_COMVAR;
                        } else {
                            continue;
                        }
                    }
                }
                argp = parse_assign(lexp, argp, type);
                if (type == NV_ARRAY) argp->argflag |= ARG_ARRAY;
                lexp->intypeset = intypeset;
                if (lexp->assignlevel) was_assign = 1;
                if (associative) lexp->assignok |= SH_ASSIGN;
                goto retry;
            } else if (argno == 1 && !t->comset) {
                // SVR2 style function.
                if (!(flag & SH_ARRAY) && sh_lex(lexp) == RPAREN) {
                    lexp->arg = argp;
                    return funct(lexp);
                }
                lexp->token = LPAREN;
            }
        } else if (flag & SH_ASSIGN) {
            if (tok == RPAREN) {
                break;
            } else if (tok == NL && (flag & SH_ARRAY)) {
                lexp->comp_assign = 2;
                goto retry;
            }
        }
        if (!(flag & SH_NOIO)) {
            if (io) {
                while (io->ionxt) io = io->ionxt;
                io->ionxt = inout(lexp, NULL, 0);
            } else {
                t->comio = io = inout(lexp, NULL, 0);
            }
        }
    }
    *argtail = 0;
    t->comtyp = TCOM;
    if (typed) t->comtyp |= COMFIXED;
    if (lexp->kiafile && !(flag & SH_NOIO)) {
        Namval_t *np = (Namval_t *)t->comnamp;
        unsigned long r = 0;
        int line = t->comline;
        argp = t->comarg;
        if (np) {
            r = kiaentity(lexp, nv_name(np), -1, 'p', -1, 0, lexp->unknown, 'b', 0, "");
        } else if (argp) {
            r = kiaentity(lexp, sh_argstr(argp), -1, 'p', -1, 0, lexp->unknown, 'c', 0, "");
        }
        if (r > 0) {
            sfprintf(lexp->kiatmp, "p;%..64d;p;%..64d;%d;%d;c;\n", lexp->current, r, line, line);
        }
        if (t->comset && argno == 0) {
            writedefs(lexp, t->comset, line, 'v', t->comarg);
        } else if (np && nv_isattr(np, BLT_DCL)) {
            writedefs(lexp, argp, line, 0, NULL);
        } else if (argp && strcmp(argp->argval, "read") == 0) {
            writedefs(lexp, argp, line, 0, NULL);
        } else if (argp && *argp->argval == '.' && argp->argval[1] == 0 &&
                   (argp = argp->argnxt.ap)) {
            r = kiaentity(lexp, sh_argstr(argp), -1, 'p', 0, 0, lexp->script, 'd', 0, "");
            sfprintf(lexp->kiatmp, "p;%..64d;p;%..64d;%d;%d;d;\n", lexp->current, r, line, line);
        }
    }
    if (t->comnamp && (argp = t->comarg->argnxt.ap)) {
        Namval_t *np = (Namval_t *)t->comnamp;
        if ((np == SYSBREAK || np == SYSCONT) && (argp->argflag & ARG_RAW) &&
            !isdigit(*argp->argval)) {
            char *cp = argp->argval;
            // Convert break/continue labels to numbers.
            tok = 0;
            for (argp = label_list; argp != label_last; argp = argp->argnxt.ap) {
                if (!strcmp(cp, argp->argval)) {
                    tok = loop_level - argp->argflag;
                    if (tok >= 1) {
                        argp = t->comarg->argnxt.ap;
                        if (tok > 9) {
                            argp->argval[1] = '0' + tok % 10;
                            argp->argval[2] = 0;
                            tok /= 10;
                        } else {
                            argp->argval[1] = 0;
                        }
                        *argp->argval = '0' + tok;
                    }
                    break;
                }
            }
            if (sh_isoption(lexp->sh, SH_NOEXEC) && tok == 0) {
                errormsg(SH_DICT, ERROR_warn(0), e_lexlabunknown,
                         lexp->sh->inlineno - (lexp->token == '\n'), cp);
            }
        } else if (sh_isoption(lexp->sh, SH_NOEXEC) && np == SYSSET &&
                   ((tok = *argp->argval) == '-' || tok == '+') &&
                   (argp->argval[1] == 0 || strchr(argp->argval, 'k'))) {
            errormsg(SH_DICT, ERROR_warn(0), e_lexobsolete5,
                     lexp->sh->inlineno - (lexp->token == '\n'), argp->argval);
        }
    }
    // Expand argument list if possible.
    if (argno > 0 && !(flag & (SH_ARRAY | NV_APPEND))) {
        t->comarg = qscan(lexp, t, argno);
    } else if (t->comarg) {
        t->comtyp |= COMSCAN;
    }
    lexp->aliasok = 0;
    return (Shnode_t *)t;
}

//
// Skip past newlines but issue prompt if interactive.
//
static_fn int skipnl(Lex_t *lexp, int flag) {
    int token;

    while ((token = sh_lex(lexp)) == NL) {
        ;  // empty loop
    }
    if (token == ';' && !(flag & SH_SEMI)) sh_syntax(lexp);
    return token;
}

//
// Check for and process and i/o redirections.
// If flag>0 then an alias can be in the next word.
// If flag<0 only one redirection will be processed.
//
static_fn struct ionod *inout(Lex_t *lexp, struct ionod *lastio, int flag) {
    int iof = lexp->digits, token = lexp->token;
    struct ionod *iop;
    Stk_t *stkp = lexp->sh->stk;
    char *iovname = NULL;
    int errout = 0;

    if (token == IOVNAME) {
        iovname = lexp->arg->argval + 1;
        token = sh_lex(lexp);
        iof = 0;
    }
    switch (token & 0xff) {
        case '<': {
            if (token == IODOCSYM) {
                iof |= (IODOC | IORAW);
            } else if (token == IOMOV0SYM) {
                iof |= IOMOV;
            } else if (token == IORDWRSYMT) {
                iof |= IORDW | IOREWRITE;
            } else if (token == IORDWRSYM) {
                iof |= IORDW;
            } else if ((token & SYMSHARP) == SYMSHARP) {
                int n;
                iof |= IOLSEEK;
                n = fcgetc();
                if (n == '#') {
                    iof |= IOCOPY;
                } else if (n > 0) {
                    fcseek(-1);
                }
            }
            break;
        }
        case '>': {
            if (iof < 0) {
                errout = 1;
                iof = 1;
            }
            iof |= IOPUT;
            if (token == IOAPPSYM) {
                iof |= IOAPP;
            } else if (token == IOMOV1SYM) {
                iof |= IOMOV;
            } else if (token == IOCLOBSYM) {
                iof |= IOCLOB;
            } else if ((token & SYMSHARP) == SYMSHARP) {
                iof |= IOLSEEK;
            } else if ((token & SYMSEMI) == SYMSEMI) {
                iof |= IOREWRITE;
            }
            break;
        }
        default: { return lastio; }
    }
    lexp->digits = 0;
    iop = stkalloc(stkp, sizeof(struct ionod));
    iop->iodelim = NULL;
    iop->iosize = 0;
    token = sh_lex(lexp);
    if (token) {
        if (token == RPAREN && (iof & IOLSEEK) && lexp->comsub) {
            lexp->arg = stkalloc(stkp, sizeof(struct argnod) + 3);
            strcpy(lexp->arg->argval, "CUR");
            lexp->arg->argflag = ARG_RAW;
            iof |= IOARITH;
            fcseek(-1);
        } else if (token == EXPRSYM && (iof & IOLSEEK)) {
            iof |= IOARITH;
        } else if (((token == IPROCSYM && !(iof & IOPUT)) ||
                    (token == OPROCSYM && (iof & IOPUT))) &&
                   !(iof & (IOLSEEK | IOREWRITE | IOMOV | IODOC))) {
            lexp->arg = process_sub(lexp, token);
            iof |= IOPROCSUB;
        } else {
            sh_syntax(lexp);
        }
    }
    if ((iof & IOPROCSUB) && !(iof & IOLSEEK)) {
        iop->ioname = (char *)lexp->arg->argchn.ap;
    } else {
        iop->ioname = lexp->arg->argval;
    }
    iop->iovname = iovname;
    if (iof & IODOC) {
        if (lexp->digits == 2) {
            iof |= IOSTRG;
            if (!(lexp->arg->argflag & ARG_RAW)) iof &= ~IORAW;
        } else {
            if (!lexp->sh->heredocs) lexp->sh->heredocs = sftmp(HERE_MEM);
            iop->iolst = lexp->heredoc;
            lexp->heredoc = iop;
            if (lexp->arg->argflag & ARG_QUOTED) iof |= IOQUOTE;
            if (lexp->digits == 3) iof |= IOLSEEK;
            if (lexp->digits) iof |= IOSTRIP;
        }
    } else {
        iop->iolst = NULL;
        if (lexp->arg->argflag & ARG_RAW) iof |= IORAW;
    }
    iop->iofile = iof;
    if (flag > 0) {  // allow alias substitutions and parameter assignments
        lexp->aliasok = lexp->assignok = 1;
    }
    if (lexp->kiafile) {
        int n = lexp->sh->inlineno - (lexp->token == '\n');
        if (!(iof & IOMOV)) {
            unsigned long r = kiaentity(lexp, (iof & IORAW) ? sh_fmtq(iop->ioname) : iop->ioname,
                                        -1, 'f', 0, 0, lexp->script, 'f', 0, "");
            sfprintf(lexp->kiatmp, "p;%..64d;f;%..64d;%d;%d;%c;%d\n", lexp->current, r, n, n,
                     (iof & IOPUT) ? ((iof & IOAPP) ? 'a' : 'w') : ((iof & IODOC) ? 'h' : 'r'),
                     iof & IOUFD);
        }
    }
    if (flag >= 0) {
        struct ionod *ioq = iop;
        sh_lex(lexp);
        if (errout) {
            // Redirect standard output to standard error.
            ioq = stkalloc(stkp, sizeof(struct ionod));
            memset(ioq, 0, sizeof(*ioq));
            ioq->ioname = "1";
            ioq->iolst = NULL;
            ioq->iodelim = NULL;
            ioq->iofile = IORAW | IOPUT | IOMOV | 2;
            iop->ionxt = ioq;
        }
        ioq->ionxt = inout(lexp, lastio, flag);
    } else {
        iop->ionxt = NULL;
    }
    return iop;
}

//
// Convert argument chain to argument list when no special arguments.
//
static_fn struct argnod *qscan(Lex_t *lp, struct comnod *ac, int argn) {
    char **cp;
    struct argnod *ap;
    struct dolnod *dp;
    int special = 0;

    // Special hack for test -t compatibility.
    if ((Namval_t *)ac->comnamp == SYSTEST) {
        special = 2;
    } else if (*(ac->comarg->argval) == '[' && ac->comarg->argval[1] == 0) {
        special = 3;
    }
    if (special) {
        ap = ac->comarg->argnxt.ap;
        if (argn == (special + 1) && ap->argval[1] == 0 && *ap->argval == '!') {
            ap = ap->argnxt.ap;
        } else if (argn != special) {
            special = 0;
        }
    }
    if (special) {
        const char *message;
        if (strcmp(ap->argval, "-t")) {
            message = "line %d: Invariant test";
            special = 0;
        } else {
            message = "line %d: -t requires argument";
            argn++;
        }
        if (sh_isoption(lp->sh, SH_NOEXEC)) errormsg(SH_DICT, ERROR_warn(0), message, ac->comline);
    }
    // Leave space for an extra argument at the front.
    dp = stkalloc(stkstd,
                  sizeof(struct dolnod) + ARG_SPARE * sizeof(char *) + argn * sizeof(char *));
    cp = dp->dolval + ARG_SPARE;
    dp->dolnum = argn;
    dp->dolbot = ARG_SPARE;
    ap = ac->comarg;
    while (ap) {
        *cp++ = ap->argval;
        ap = ap->argnxt.ap;
    }
    if (special == 3) {
        cp[0] = cp[-1];
        cp[-1] = "1";
        cp++;
    } else if (special) {
        *cp++ = "1";
    }
    *cp = 0;
    return (struct argnod *)dp;
}

static_fn Shnode_t *test_expr(Lex_t *lp, int sym) {
    Shnode_t *t = test_or(lp);
    if (lp->token != sym) sh_syntax(lp);
    return t;
}

static_fn Shnode_t *test_or(Lex_t *lp) {
    Shnode_t *t = test_and(lp);
    while (lp->token == ORFSYM) t = makelist(lp, TORF | TTEST, t, test_and(lp));
    return t;
}

static_fn Shnode_t *test_and(Lex_t *lp) {
    Shnode_t *t = test_primary(lp);
    while (lp->token == ANDFSYM) t = makelist(lp, TAND | TTEST, t, test_primary(lp));
    return t;
}

//
// Convert =~ into == ~(E).
//
static_fn void ere_match(void) {
    Sfio_t *base, *iop = sfopen(NULL, " ~(E)", "s");
    int c;

    while ((c = fcgetc()) == ' ' || c == '\t') {
        ;  // empty loop
    }
    if (c) fcseek(-1);
    if (!(base = fcfile())) base = sfopen(NULL, fcseek(0), "s");
    fcclose();
    sfstack(base, iop);
    fcfopen(base);
}

static_fn Shnode_t *test_primary(Lex_t *lexp) {
    struct argnod *arg;
    Shnode_t *t;
    int num, token;
    static Shnode_t dummy_node;

    token = skipnl(lexp, 0);
    num = lexp->digits;
    switch (token) {
        case '(': {
            t = test_expr(lexp, ')');
            t = makelist(lexp, TTST | TTEST | TPAREN, t, &dummy_node);
            t->tst.tstline = lexp->sh->inlineno;
            break;
        }
        case '!': {
            t = test_primary(lexp);
            if (!t) sh_syntax(lexp);
            t->tre.tretyp |= TNEGATE;
            return t;
        }
        case TESTUNOP: {
            if (sh_lex(lexp)) sh_syntax(lexp);
            if (lexp->kiafile && !strchr("sntzoOG", num)) {
                int line = lexp->sh->inlineno - (lexp->token == NL);
                unsigned long r;
                r = kiaentity(lexp, sh_argstr(lexp->arg), -1, 'f', 0, 0, lexp->script, 't', 0, "");
                sfprintf(lexp->kiatmp, "p;%..64d;f;%..64d;%d;%d;t;\n", lexp->current, r, line,
                         line);
            }
            t = makelist(lexp, TTST | TTEST | TUNARY | (num << TSHIFT), (Shnode_t *)lexp->arg,
                         (Shnode_t *)lexp->arg);
            t->tst.tstline = lexp->sh->inlineno;
            break;
        }
        case 0: {  // binary test operators
            arg = lexp->arg;
            if ((token = sh_lex(lexp)) == TESTBINOP) {
                num = lexp->digits;
                if (num == TEST_REP) {
                    ere_match();
                    num = TEST_PEQ;
                }
            } else if (token == '<') {
                num = TEST_SLT;
            } else if (token == '>') {
                num = TEST_SGT;
            } else if (token == ANDFSYM || token == ORFSYM || token == ETESTSYM ||
                       token == RPAREN) {
                t = makelist(lexp, TTST | TTEST | TUNARY | ('n' << TSHIFT), (Shnode_t *)arg,
                             (Shnode_t *)arg);
                t->tst.tstline = lexp->sh->inlineno;
                return t;
            } else {
                sh_syntax(lexp);
            }
            if (lexp->kiafile && (num == TEST_EF || num == TEST_NT || num == TEST_OT)) {
                int line = lexp->sh->inlineno - (lexp->token == NL);
                unsigned long r;
                r = kiaentity(lexp, sh_argstr(lexp->arg), -1, 'f', 0, 0, lexp->current, 't', 0, "");
                sfprintf(lexp->kiatmp, "p;%..64d;f;%..64d;%d;%d;t;\n", lexp->current, r, line,
                         line);
            }
            if (sh_lex(lexp)) sh_syntax(lexp);
            if (num & TEST_PATTERN) {
                if (lexp->arg->argflag & (ARG_EXP | ARG_MAC)) num &= ~TEST_PATTERN;
            }
            t = getnode(tstnod);
            t->lst.lsttyp = TTST | TTEST | TBINARY | (num << TSHIFT);
            t->lst.lstlef = (Shnode_t *)arg;
            t->lst.lstrit = (Shnode_t *)lexp->arg;
            t->tst.tstline = lexp->sh->inlineno;
            if (lexp->kiafile && (num == TEST_EF || num == TEST_NT || num == TEST_OT)) {
                int line = lexp->sh->inlineno - (lexp->token == NL);
                unsigned long r;
                r = kiaentity(lexp, sh_argstr(lexp->arg), -1, 'f', 0, 0, lexp->current, 't', 0, "");
                sfprintf(lexp->kiatmp, "p;%..64d;f;%..64d;%d;%d;t;\n", lexp->current, r, line,
                         line);
            }
            break;
        }
        default: { return 0; }
    }
    skipnl(lexp, 0);
    return t;
}

//
// Return an entity checksum. The entity is created if it doesn't exist.
//
unsigned long kiaentity(Lex_t *lexp, const char *name, int len, int type, int first, int last,
                        unsigned long parent, int pkind, int width, const char *attr) {
    Stk_t *stkp = lexp->sh->stk;
    Namval_t *np;
    long offset = stktell(stkp);

    sfputc(stkp, type);
    if (len > 0) {
        sfwrite(stkp, name, len);
    } else {
        if (type == 'p') {
            sfputr(stkp, path_basename(name), 0);
        } else {
            sfputr(stkp, name, 0);
        }
    }
    sfputc(stkp, '\0');
    np = nv_search(stkptr(stkstd, offset), lexp->entity_tree, NV_ADD);
    assert(np);
    stkseek(stkp, offset);
    STORE_VT(np->nvalue, i, pkind);
    nv_setsize(np, width);
    if (!nv_isattr(np, NV_TAGGED) && first >= 0) {
        nv_onattr(np, NV_TAGGED);
        if (!pkind) pkind = '0';
        if (len > 0) {
            sfprintf(lexp->kiafile, "%..64d;%c;%.*s;%d;%d;%..64d;%..64d;%c;%d;%s\n", np->hash, type,
                     len, name, first, last, parent, lexp->fscript, pkind, width, attr);
        } else {
            sfprintf(lexp->kiafile, "%..64d;%c;%s;%d;%d;%..64d;%..64d;%c;%d;%s\n", np->hash, type,
                     name, first, last, parent, lexp->fscript, pkind, width, attr);
        }
    }
    return np->hash;
}

static_fn void kia_add(Namval_t *np, void *data) {
    char *name = nv_name(np);
    Lex_t *lp = data;
    UNUSED(data);

    kiaentity(lp, name + 1, -1, *name, 0, -1, (*name == 'p' ? lp->unknown : lp->script),
              FETCH_VT(np->nvalue, i), nv_size(np), "");
}

int kiaclose(Lex_t *lexp) {
    if (!lexp->kiafile) return sfclose(lexp->kiafile);

    off_t off1, off2;
    int n;
    unsigned long r =
        kiaentity(lexp, lexp->scriptname, -1, 'p', -1, lexp->sh->inlineno - 1, 0, 's', 0, "");
    kiaentity(lexp, lexp->scriptname, -1, 'p', 1, lexp->sh->inlineno - 1, r, 's', 0, "");
    kiaentity(lexp, lexp->scriptname, -1, 'f', 1, lexp->sh->inlineno - 1, r, 's', 0, "");
    nv_scan(lexp->entity_tree, kia_add, lexp, NV_TAGGED, 0);
    off1 = sfseek(lexp->kiafile, (off_t)0, SEEK_END);
    sfseek(lexp->kiatmp, (off_t)0, SEEK_SET);
    sfmove(lexp->kiatmp, lexp->kiafile, SF_UNBOUND, -1);
    off2 = sfseek(lexp->kiafile, (off_t)0, SEEK_END);
#ifdef SF_BUFCONST
    if (off2 == off1) {
        n = sfprintf(lexp->kiafile, "DIRECTORY\nENTITY;%lld;%d\nDIRECTORY;",
                     (Sflong_t)lexp->kiabegin, (size_t)(off1 - lexp->kiabegin));
    } else {
        n = sfprintf(lexp->kiafile, "DIRECTORY\nENTITY;%lld;%d\nRELATIONSHIP;%lld;%d\nDIRECTORY;",
                     (Sflong_t)lexp->kiabegin, (size_t)(off1 - lexp->kiabegin), (Sflong_t)off1,
                     (size_t)(off2 - off1));
    }
    if (off2 >= INT_MAX) off2 = -(n + 12);
    sfprintf(lexp->kiafile, "%010.10lld;%010d\n", (Sflong_t)off2 + 10, n + 12);
#else
    if (off2 == off1) {
        n = sfprintf(lexp->kiafile, "DIRECTORY\nENTITY;%d;%d\nDIRECTORY;", lexp->kiabegin,
                     off1 - lexp->kiabegin);
    } else {
        n = sfprintf(lexp->kiafile, "DIRECTORY\nENTITY;%d;%d\nRELATIONSHIP;%d;%d\nDIRECTORY;",
                     lexp->kiabegin, off1 - lexp->kiabegin, off1, off2 - off1);
    }
    sfprintf(lexp->kiafile, "%010d;%010d\n", off2 + 10, n + 12);
#endif
    return sfclose(lexp->kiafile);
}
