/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2012 AT&T Intellectual Property          *
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
// Shell intermediate code reader.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

#include "argnod.h"
#include "defs.h"
#include "name.h"
#include "sfio.h"
#include "shnodes.h"
#include "stk.h"

static_fn struct dolnod *r_comlist(Shell_t *);
static_fn struct argnod *r_arg(Shell_t *);
static_fn struct ionod *r_redirect(Shell_t *);
static_fn struct regnod *r_switch(Shell_t *);
static_fn Shnode_t *r_tree(Shell_t *);
static_fn char *r_string(Stk_t *);
static_fn void r_comarg(Shell_t *, struct comnod *);

static Sfio_t *infile;

#define getnode(s, type) (stkalloc((s), sizeof(struct type)))

Shnode_t *sh_trestore(Shell_t *shp, Sfio_t *in) {
    Shnode_t *t;
    infile = in;
    t = r_tree(shp);
    return t;
}

//
// Read in a shell tree.
//
static_fn Shnode_t *r_tree(Shell_t *shp) {
    long l = sfgetl(infile);
    int type;
    Shnode_t *t = NULL;

    if (l < 0) return t;
    type = l;
    switch (type & COMMSK) {
        case TTIME:
        case TPAR: {
            t = getnode(shp->stk, parnod);
            t->par.partre = r_tree(shp);
            break;
        }
        case TCOM: {
            t = getnode(shp->stk, comnod);
            t->tre.tretyp = type;
            r_comarg(shp, (struct comnod *)t);
            break;
        }
        case TSETIO:
        case TFORK: {
            t = getnode(shp->stk, forknod);
            t->fork.forkline = sfgetu(infile);
            t->fork.forktre = r_tree(shp);
            t->fork.forkio = r_redirect(shp);
            break;
        }
        case TIF: {
            t = getnode(shp->stk, ifnod);
            t->if_.iftre = r_tree(shp);
            t->if_.thtre = r_tree(shp);
            t->if_.eltre = r_tree(shp);
            break;
        }
        case TWH: {
            t = getnode(shp->stk, whnod);
            t->wh.whinc = (struct arithnod *)r_tree(shp);
            t->wh.whtre = r_tree(shp);
            t->wh.dotre = r_tree(shp);
            break;
        }
        case TLST:
        case TAND:
        case TORF:
        case TFIL: {
            t = getnode(shp->stk, lstnod);
            t->lst.lstlef = r_tree(shp);
            t->lst.lstrit = r_tree(shp);
            break;
        }
        case TARITH: {
            t = getnode(shp->stk, arithnod);
            t->ar.arline = sfgetu(infile);
            t->ar.arexpr = r_arg(shp);
            t->ar.arcomp = NULL;
            if ((t->ar.arexpr)->argflag & ARG_RAW) {
                t->ar.arcomp = sh_arithcomp(shp, (t->ar.arexpr)->argval);
            }
            break;
        }
        case TFOR: {
            t = getnode(shp->stk, fornod);
            t->for_.forline = 0;
            if (type & FLINENO) t->for_.forline = sfgetu(infile);
            t->for_.fortre = r_tree(shp);
            t->for_.fornam = r_string(shp->stk);
            t->for_.forlst = (struct comnod *)r_tree(shp);
            break;
        }
        case TSW: {
            t = getnode(shp->stk, swnod);
            t->sw.swline = 0;
            if (type & FLINENO) t->sw.swline = sfgetu(infile);
            t->sw.swarg = r_arg(shp);
            if (type & COMSCAN) {
                t->sw.swio = r_redirect(shp);
            } else {
                t->sw.swio = NULL;
            }
            t->sw.swlst = r_switch(shp);
            break;
        }
        case TFUN: {
            Sfio_t *savstk;
            struct slnod *slp;
            struct functnod *fp;
            t = getnode(shp->stk, functnod);
            t->funct.functloc = -1;
            t->funct.functline = sfgetu(infile);
            t->funct.functnam = r_string(shp->stk);
            savstk = stkopen(STK_SMALL);
            savstk = stkinstall(savstk, 0);
            slp = stkalloc(shp->stk, sizeof(struct slnod) + sizeof(struct functnod));
            slp->slchild = NULL;
            slp->slnext = shp->st.staklist;
            shp->st.staklist = NULL;
            fp = (struct functnod *)(slp + 1);
            memset(fp, 0, sizeof(*fp));
            fp->functtyp = TFUN | FAMP;
            if (shp->st.filename) fp->functnam = stkcopy(shp->stk, shp->st.filename);
            t->funct.functtre = r_tree(shp);
            t->funct.functstak = slp;
            t->funct.functargs = (struct comnod *)r_tree(shp);
            slp->slptr = stkinstall(savstk, 0);
            slp->slchild = shp->st.staklist;
            break;
        }
        case TTST: {
            t = getnode(shp->stk, tstnod);
            t->tst.tstline = sfgetu(infile);
            if ((type & TPAREN) == TPAREN) {
                t->lst.lstlef = r_tree(shp);
            } else {
                t->lst.lstlef = (Shnode_t *)r_arg(shp);
                if ((type & TBINARY)) t->lst.lstrit = (Shnode_t *)r_arg(shp);
            }
        }
        default: { break; }
    }
    if (t) t->tre.tretyp = type;
    return t;
}

static_fn struct argnod *r_arg(Shell_t *shp) {
    struct argnod *ap = NULL;
    struct argnod *aptop = NULL;
    struct argnod *apold;
    long l;
    Stk_t *stkp = shp->stk;

    while ((l = sfgetu(infile)) > 0) {
        ap = stkseek(stkp, (unsigned)l + ARGVAL);
        if (!aptop) {
            aptop = ap;
        } else {
            apold->argnxt.ap = ap;
        }
        if (--l > 0) {
            sfread(infile, ap->argval, (size_t)l);
        }
        ap->argval[l] = 0;
        ap->argchn.cp = NULL;
        ap->argflag = sfgetc(infile);
        ap = (struct argnod *)stkfreeze(stkp, 0);
        if (*ap->argval == 0 && (ap->argflag & ARG_EXP)) {
            ap->argchn.ap = (struct argnod *)r_tree(shp);
        } else if (*ap->argval == 0 &&
                   (ap->argflag & ~(ARG_APPEND | ARG_MESSAGE | ARG_QUOTED | ARG_ARRAY)) == 0) {
            struct fornod *fp = (struct fornod *)getnode(shp->stk, fornod);
            fp->fortyp = sfgetu(infile);
            fp->fortre = r_tree(shp);
            fp->fornam = ap->argval + 1;
            ap->argchn.ap = (struct argnod *)fp;
        }
        apold = ap;
    }
    if (ap) ap->argnxt.ap = NULL;
    return aptop;
}

static_fn struct ionod *r_redirect(Shell_t *shp) {
    long l;
    struct ionod *iop = NULL;
    struct ionod *ioptop = NULL;
    struct ionod *iopold;

    while ((l = sfgetl(infile)) >= 0) {
        iop = (struct ionod *)getnode(shp->stk, ionod);
        if (!ioptop) {
            ioptop = iop;
        } else {
            iopold->ionxt = iop;
        }
        iop->iofile = l;
        iop->ioname = r_string(shp->stk);
        iop->iodelim = r_string(shp->stk);
        if (iop->iodelim) {
            iop->iosize = sfgetl(infile);
            if (shp->heredocs) {
                iop->iooffset = sfseek(shp->heredocs, (off_t)0, SEEK_END);
            } else {
                shp->heredocs = sftmp(512);
                iop->iooffset = 0;
            }
            sfmove(infile, shp->heredocs, iop->iosize, -1);
        }
        iopold = iop;
        if (iop->iofile & IOVNM) {
            iop->iovname = r_string(shp->stk);
        } else {
            iop->iovname = NULL;
        }
        iop->iofile &= ~IOVNM;
    }
    if (iop) iop->ionxt = NULL;
    return ioptop;
}

static_fn void r_comarg(Shell_t *shp, struct comnod *com) {
    char *cmdname = NULL;

    com->comio = r_redirect(shp);
    com->comset = r_arg(shp);
    com->comstate = NULL;
    if (com->comtyp & COMSCAN) {
        com->comarg = r_arg(shp);
        if (com->comarg->argflag == ARG_RAW) cmdname = com->comarg->argval;
    } else if ((com->comarg = (struct argnod *)r_comlist(shp))) {
        cmdname = ((struct dolnod *)(com->comarg))->dolval[ARG_SPARE];
    }
    com->comline = sfgetu(infile);
    com->comnamq = NULL;
    if (cmdname) {
        char *cp;
        com->comnamp = nv_search(cmdname, shp->fun_tree, 0);
        if (com->comnamp && (cp = strrchr(cmdname + 1, '.'))) {
            *cp = 0;
            com->comnamp = nv_open(cmdname, shp->var_tree, NV_VARNAME | NV_NOADD | NV_NOARRAY);
            *cp = '.';
        }
    } else {
        com->comnamp = NULL;
    }
}

static_fn struct dolnod *r_comlist(Shell_t *shp) {
    struct dolnod *dol = NULL;
    long l;
    char **argv;

    if ((l = sfgetl(infile)) > 0) {
        dol = stkalloc(shp->stk, sizeof(struct dolnod) + sizeof(char *) * (l + ARG_SPARE));
        dol->dolnum = l;
        dol->dolbot = ARG_SPARE;
        argv = dol->dolval + ARG_SPARE;
        while (true) {
            *argv = r_string(shp->stk);
            if (!*argv) break;
            argv++;
        }
    }
    return dol;
}

static_fn struct regnod *r_switch(Shell_t *shp) {
    long l;
    struct regnod *reg = NULL;
    struct regnod *regtop = NULL;
    struct regnod *regold;

    while ((l = sfgetl(infile)) >= 0) {
        reg = (struct regnod *)getnode(shp->stk, regnod);
        if (!regtop) {
            regtop = reg;
        } else {
            regold->regnxt = reg;
        }
        reg->regflag = l;
        reg->regptr = r_arg(shp);
        reg->regcom = r_tree(shp);
        regold = reg;
    }
    if (reg) reg->regnxt = NULL;
    return regtop;
}

static_fn char *r_string(Stk_t *stkp) {
    Sfio_t *in = infile;
    unsigned long l = sfgetu(in);
    char *ptr;

    if (l == 0) return NULL;
    ptr = stkalloc(stkp, (unsigned)l);
    if (--l > 0 && sfread(in, ptr, (size_t)l) != (size_t)l) return NULL;
    ptr[l] = 0;
    return ptr;
}
