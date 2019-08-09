// This is not built at this time as it is unused. It used to be used by the COSHELL facility
// which has been removed. We're retaining this module because it is potentially useful.

/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2011 AT&T Intellectual Property          *
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
// David Korn
// AT&T Labs
//
// Shell deparser.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "argnod.h"
#include "ast_assert.h"
#include "defs.h"
#include "sfio.h"
#include "shnodes.h"
#include "shtable.h"

#define BEGIN 0
#define MIDDLE 1
#define END 2
#define PRE 1
#define POST 2

// Flags that can be specified with p_tree().
#define NO_NEWLINE 1
#define NEED_BRACE 2
#define NO_BRACKET 4

static_fn void p_comlist(const struct dolnod *, int);
static_fn void p_arg(const struct argnod *, int endchar, int opts);
static_fn void p_comarg(const struct comnod *);
static_fn void p_keyword(const char *, int);
static_fn void p_redirect(const struct ionod *);
static_fn void p_switch(const struct regnod *);
static_fn void here_body(const struct ionod *);
static_fn void p_tree(const Shnode_t *, int);

static int level;
static int begin_line;
static int end_line;
static char io_op[7];
static char un_op[3] = "-?";
static const struct ionod *here_doc = NULL;
static Sfio_t *outfile = NULL;
static const char *forinit = "";

extern void sh_deparse(Sfio_t *, const Shnode_t *, int);

void sh_deparse(Sfio_t *out, const Shnode_t *t, int tflags) {
    outfile = out;
    p_tree(t, tflags);
}

// Print script corresponding to shell tree <t>.
static_fn void p_tree(const Shnode_t *t, int tflags) {
    char *cp = NULL;
    int save = end_line;
    int needbrace = (tflags & NEED_BRACE);
    int bracket = 0;

    tflags &= ~NEED_BRACE;
    if (tflags & NO_NEWLINE) {
        end_line = ' ';
    } else {
        end_line = '\n';
    }
    int cmd_type = t->tre.tretyp & COMMSK;
    switch (cmd_type) {
        case TTIME: {
            if (t->tre.tretyp & COMSCAN) {
                p_keyword("!", BEGIN);
            } else {
                p_keyword("time", BEGIN);
            }
            if (t->par.partre) p_tree(t->par.partre, tflags);
            level--;
            break;
        }
        case TCOM: {
            if (begin_line && level > 0) sfnputc(outfile, '\t', level);
            begin_line = 0;
            p_comarg((struct comnod *)t);
            break;
        }
        case TSETIO: {
            if (t->tre.tretyp & FPCL) {
                tflags |= NEED_BRACE;
            } else {
                tflags = NO_NEWLINE | NEED_BRACE;
            }
            p_tree(t->fork.forktre, tflags);
            p_redirect(t->fork.forkio);
            break;
        }
        case TFORK: {
            if (needbrace) tflags |= NEED_BRACE;
            if (t->tre.tretyp & (FAMP | FCOOP)) {
                tflags = NEED_BRACE | NO_NEWLINE;
                end_line = ' ';
            } else if (t->fork.forkio) {
                tflags = NO_NEWLINE;
            }
            p_tree(t->fork.forktre, tflags);
            if (t->fork.forkio) p_redirect(t->fork.forkio);
            if (t->tre.tretyp & FCOOP) {
                sfputr(outfile, "|&", '\n');
                begin_line = 1;
            } else if (t->tre.tretyp & FAMP) {
                sfputr(outfile, "&", '\n');
                begin_line = 1;
            }
            break;
        }
        case TIF: {
            p_keyword("if", BEGIN);
            p_tree(t->if_.iftre, 0);
            p_keyword("then", MIDDLE);
            p_tree(t->if_.thtre, 0);
            if (t->if_.eltre) {
                p_keyword("else", MIDDLE);
                p_tree(t->if_.eltre, 0);
            }
            p_keyword("fi", END);
            break;
        }
        case TWH: {
            if (t->wh.whinc) {
                cp = "for";
            } else if (t->tre.tretyp & COMSCAN) {
                cp = "until";
            } else {
                cp = "while";
            }
            p_keyword(cp, BEGIN);
            if (t->wh.whinc) {
                struct argnod *arg = (t->wh.whtre)->ar.arexpr;
                sfprintf(outfile, "(( %s; ", forinit);
                forinit = "";
                sfputr(outfile, arg->argval, ';');
                arg = (t->wh.whinc)->arexpr;
                sfprintf(outfile, " %s))\n", arg->argval);
            } else {
                p_tree(t->wh.whtre, 0);
            }
            t = t->wh.dotre;
            p_keyword("do", MIDDLE);
            p_tree(t, 0);
            p_keyword("done", END);
            break;
        }
        case TLST: {
            Shnode_t *tr = t->lst.lstrit;
            if (tr->tre.tretyp == TWH && tr->wh.whinc && t->lst.lstlef->tre.tretyp == TARITH) {
                // Arithmetic for statement.
                struct argnod *init = (t->lst.lstlef)->ar.arexpr;
                forinit = init->argval;
                p_tree(t->lst.lstrit, tflags);
                break;
            }
            if (needbrace) p_keyword("{", BEGIN);
            p_tree(t->lst.lstlef, 0);
            if (needbrace) tflags = 0;
            p_tree(t->lst.lstrit, tflags);
            if (needbrace) p_keyword("}", END);
            break;
        }
        case TAND:
        case TORF:
        case TFIL: {
            if (cmd_type == TAND) {
                cp = "&&";
            } else if (cmd_type == TORF) {
                cp = "||";
            } else {
                cp = "|";
            }
            if (t->tre.tretyp & TTEST) {
                tflags |= NO_NEWLINE;
                if (!(tflags & NO_BRACKET)) {
                    p_keyword("[[", BEGIN);
                    tflags |= NO_BRACKET;
                    bracket = 1;
                }
            }
            p_tree(t->lst.lstlef, NEED_BRACE | NO_NEWLINE | (tflags & NO_BRACKET));
            if (tflags & FALTPIPE) {
                Shnode_t *tt = t->lst.lstrit;
                if (tt->tre.tretyp != TFIL || !(tt->lst.lstlef->tre.tretyp & FALTPIPE)) {
                    sfputc(outfile, '\n');
                    return;
                }
            }
            sfputr(outfile, cp, here_doc ? '\n' : ' ');
            if (here_doc) {
                here_body(here_doc);
                here_doc = 0;
            }
            level++;
            p_tree(t->lst.lstrit, tflags | NEED_BRACE);
            if (bracket) p_keyword("]]", END);
            level--;
            break;
        }
        case TPAR: {
            p_keyword("(", BEGIN);
            p_tree(t->par.partre, 0);
            p_keyword(")", END);
            break;
        }
        case TARITH: {
            struct argnod *ap = t->ar.arexpr;
            if (begin_line && level) sfnputc(outfile, '\t', level);
            sfprintf(outfile, "(( %s ))%c", ap->argval, end_line);
            if (!(tflags & NO_NEWLINE)) begin_line = 1;
            break;
        }
        case TFOR: {
            cp = ((t->tre.tretyp & COMSCAN) ? "select" : "for");
            p_keyword(cp, BEGIN);
            sfputr(outfile, t->for_.fornam, ' ');
            if (t->for_.forlst) {
                sfputr(outfile, "in", ' ');
                tflags = end_line;
                end_line = '\n';
                p_comarg(t->for_.forlst);
                end_line = tflags;
            } else {
                sfputc(outfile, '\n');
            }
            begin_line = 1;
            t = t->for_.fortre;
            p_keyword("do", MIDDLE);
            p_tree(t, 0);
            p_keyword("done", END);
            break;
        }
        case TSW: {
            p_keyword("case", BEGIN);
            p_arg(t->sw.swarg, ' ', 0);
            if (t->sw.swlst) {
                begin_line = 1;
                sfputr(outfile, "in", '\n');
                tflags = end_line;
                end_line = '\n';
                p_switch(t->sw.swlst);
                end_line = tflags;
            }
            p_keyword("esac", END);
            break;
        }
        case TFUN: {
            if (t->tre.tretyp & FPOSIX) {
                sfprintf(outfile, "%s", t->funct.functnam);
                p_keyword("()\n", BEGIN);
            } else {
                p_keyword("function", BEGIN);
                tflags = (t->funct.functargs ? ' ' : '\n');
                sfputr(outfile, t->funct.functnam, tflags);
                if (t->funct.functargs) {
                    tflags = end_line;
                    end_line = '\n';
                    p_comarg(t->funct.functargs);
                    end_line = tflags;
                }
            }
            begin_line = 1;
            p_keyword("{\n", MIDDLE);
            begin_line = 1;
            p_tree(t->funct.functtre, 0);
            p_keyword("}", END);
            break;
        }
        case TTST: {  // new test compound command
            if (!(tflags & NO_BRACKET)) p_keyword("[[", BEGIN);
            if ((t->tre.tretyp & TPAREN) == TPAREN) {
                p_keyword("(", BEGIN);
                p_tree(t->lst.lstlef, NO_BRACKET | NO_NEWLINE);
                p_keyword(")", END);
            } else {
                int flags = (t->tre.tretyp) >> TSHIFT;
                if (t->tre.tretyp & TNEGATE) sfputr(outfile, "!", ' ');
                if (t->tre.tretyp & TUNARY) {
                    un_op[1] = flags;
                    sfputr(outfile, un_op, ' ');
                } else {
                    cp = ((char *)(shtab_testops + (flags & 037) - 1)->sh_name);
                }
                p_arg(&(t->lst.lstlef->arg), ' ', 0);
                if (t->tre.tretyp & TBINARY) {
                    assert(cp);
                    sfputr(outfile, cp, ' ');
                    p_arg(&(t->lst.lstrit->arg), ' ', 0);
                }
            }
            if (!(tflags & NO_BRACKET)) p_keyword("]]", END);
        }
        default: { break; }
    }

    while (begin_line && here_doc) {
        here_body(here_doc);
        here_doc = 0;
    }
    end_line = save;
}

// Print a keyword.
// Increment indent level for flag==BEGIN.
// Decrement indent level for flag==END.
static_fn void p_keyword(const char *word, int flag) {
    int sep;

    if (flag == END) {
        sep = end_line;
    } else if (*word == '[' || *word == '(') {
        sep = ' ';
    } else {
        sep = '\t';
    }
    if (flag != BEGIN) level--;
    if (begin_line && level) sfnputc(outfile, '\t', level);
    sfputr(outfile, word, sep);
    if (sep == '\n') {
        begin_line = 1;
    } else {
        begin_line = 0;
    }
    if (flag != END) level++;
}

static_fn void p_arg(const struct argnod *arg, int endchar, int opts) {
    int flag = -1;

    do {
        if (!arg->argnxt.ap) {
            flag = endchar;
        } else if (opts & PRE) {  // case alternation lists in reverse order
            p_arg(arg->argnxt.ap, '|', opts);
            flag = endchar;
        } else if (opts) {
            flag = ' ';
        }
        const char *cp = arg->argval;
        if (*cp == 0 && (arg->argflag & ARG_EXP) && arg->argchn.ap) {
            int c = (arg->argflag & ARG_RAW) ? '>' : '<';
            sfputc(outfile, c);
            sfputc(outfile, '(');
            p_tree((Shnode_t *)arg->argchn.ap, 0);
            sfputc(outfile, ')');
        } else if (*cp == 0 && opts == POST && arg->argchn.ap) {  // compound assignment
            struct fornod *fp = (struct fornod *)arg->argchn.ap;
            sfprintf(outfile, "%s=(\n", fp->fornam);
            sfnputc(outfile, '\t', ++level);
            p_tree(fp->fortre, 0);
            if (--level) sfnputc(outfile, '\t', level);
            sfputc(outfile, ')');
        } else if ((arg->argflag & ARG_RAW) && (cp[1] || (*cp != '[' && *cp != ']'))) {
            cp = sh_fmtq(cp);
        }
        sfputr(outfile, cp, flag);
        if (flag == '\n') begin_line = 1;
        arg = arg->argnxt.ap;
    } while ((opts & POST) && arg);
}

static_fn void p_redirect(const struct ionod *iop) {
    char *cp;
    int iof, iof2;

    for (; iop; iop = iop->ionxt) {
        iof = iop->iofile;
        cp = io_op;
        if (iop->iovname) {
            sfwrite(outfile, "(;", 2);
            sfputr(outfile, iop->iovname, ')');
            cp++;
        } else {
            *cp = '0' + (iof & IOUFD);
        }
        if (iof & IOPUT) {
            if (*cp == '1' && !iop->iovname) cp++;
            io_op[1] = '>';
        } else {
            if (*cp == '0' && !iop->iovname) cp++;
            io_op[1] = '<';
        }
        io_op[2] = 0;
        io_op[3] = 0;
        if (iof & IOLSEEK) {
            io_op[1] = '#';
            if (iof & IOARITH) strcpy(&io_op[3], " ((");
        } else if (iof & IOMOV) {
            io_op[2] = '&';
        } else if (iof & (IORDW | IOAPP)) {
            io_op[2] = '>';
        } else if (iof & IOCLOB) {
            io_op[2] = '|';
        }
        if (iop->iodelim) {  // here document
            here_doc = iop;
            io_op[2] = '<';
        }
        sfputr(outfile, cp, ' ');
        if (iop->ionxt) {
            iof = ' ';
        } else {
            iof = end_line;
            if (iof == '\n') begin_line = 1;
        }
        if ((iof & IOLSEEK) && (iof & IOARITH)) iof2 = iof, iof = ' ';
        if (iop->iodelim) {
            if (!(iop->iofile & IODOC)) sfwrite(outfile, "''", 2);
            sfputr(outfile, sh_fmtq(iop->iodelim), iof);
        } else if (iop->iofile & IORAW) {
            sfputr(outfile, sh_fmtq(iop->ioname), iof);
        } else {
            sfputr(outfile, iop->ioname, iof);
        }
        if ((iof & IOLSEEK) && (iof & IOARITH)) sfputr(outfile, "))", iof2);
    }
}

static_fn void p_comarg(const struct comnod *com) {
    int flag = end_line;

    if (com->comtyp & FAMP) sfwrite(outfile, "& ", 2);
    if (com->comarg || com->comio) flag = ' ';
    if (com->comset) p_arg(com->comset, flag, POST);
    if (com->comarg) {
        if (!com->comio) flag = end_line;
        if (com->comtyp & COMSCAN) {
            p_arg(com->comarg, flag, POST);
        } else {
            p_comlist((struct dolnod *)com->comarg, flag);
        }
    }
    if (com->comio) p_redirect(com->comio);
}

static_fn void p_comlist(const struct dolnod *dol, int endchar) {
    char *cp, *const *argv;
    int flag = ' ', special;

    argv = dol->dolval + ARG_SPARE;
    cp = *argv++;
    special = strcmp(cp, "[") == 0;
    do {
        if (*argv == 0) {
            flag = endchar;
            if (flag == '\n') begin_line = 1;
            special = (*cp == ']' && cp[1] == 0);
        }
        sfputr(outfile, special ? cp : sh_fmtq(cp), flag);
        special = 0;
        cp = *argv++;
    } while (cp);
}

static_fn void p_switch(const struct regnod *reg) {
    if (level > 1) sfnputc(outfile, '\t', level - 1);
    p_arg(reg->regptr, ')', PRE);
    begin_line = 0;
    sfputc(outfile, '\t');
    if (reg->regcom) p_tree(reg->regcom, 0);
    level++;
    if (reg->regflag) {
        p_keyword(";&", END);
    } else {
        p_keyword(";;", END);
    }
    if (reg->regnxt) p_switch(reg->regnxt);
}

// Output `here` documents.
static_fn void here_body(const struct ionod *iop) {
    Sfio_t *infile;
    if (iop->iofile & IOSTRG) {
        infile = sfnew(NULL, iop->ioname, iop->iosize, -1, SF_STRING | SF_READ);
    } else {
        sfseek(infile = sh.heredocs, iop->iooffset, SEEK_SET);
    }
    sfmove(infile, outfile, iop->iosize, -1);
    if (iop->iofile & IOSTRG) sfclose(infile);
    sfputr(outfile, iop->iodelim, '\n');
}
