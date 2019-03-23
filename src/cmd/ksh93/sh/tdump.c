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
// Shell parse tree dump.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>
#include <sys/types.h>

#include "argnod.h"
#include "defs.h"
#include "sfio.h"
#include "shnodes.h"

static_fn int dump_p_comlist(const struct dolnod *);
static_fn int dump_p_arg(const struct argnod *);
static_fn int dump_p_comarg(const struct comnod *);
static_fn int dump_p_redirect(const struct ionod *);
static_fn int dump_p_switch(const struct regnod *);
static_fn int dump_p_tree(const Shnode_t *);
static_fn int dump_p_string(const char *);

static Sfio_t *outfile;

int sh_tdump(Sfio_t *out, const Shnode_t *t) {
    outfile = out;
    return dump_p_tree(t);
}

//
// Print script corresponding to shell tree <t>.
//
static_fn int dump_p_tree(const Shnode_t *t) {
    if (!t) return sfputl(outfile, -1);
    if (sfputl(outfile, t->tre.tretyp) < 0) return -1;
    switch (t->tre.tretyp & COMMSK) {
        case TTIME:
        case TPAR: {
            return dump_p_tree(t->par.partre);
        }
        case TCOM: {
            return dump_p_comarg((struct comnod *)t);
        }
        case TSETIO:
        case TFORK: {
            if (sfputu(outfile, t->fork.forkline) < 0) return -1;
            if (dump_p_tree(t->fork.forktre) < 0) return -1;
            return dump_p_redirect(t->fork.forkio);
        }
        case TIF: {
            if (dump_p_tree(t->if_.iftre) < 0) return -1;
            if (dump_p_tree(t->if_.thtre) < 0) return -1;
            return dump_p_tree(t->if_.eltre);
        }
        case TWH: {
            if (t->wh.whinc) {
                if (dump_p_tree((Shnode_t *)(t->wh.whinc)) < 0) return -1;
            } else {
                if (sfputl(outfile, -1) < 0) return -1;
            }
            if (dump_p_tree(t->wh.whtre) < 0) return -1;
            return dump_p_tree(t->wh.dotre);
        }
        case TLST:
        case TAND:
        case TORF:
        case TFIL: {
            if (dump_p_tree(t->lst.lstlef) < 0) return -1;
            return dump_p_tree(t->lst.lstrit);
        }
        case TARITH: {
            if (sfputu(outfile, t->ar.arline) < 0) return -1;
            return dump_p_arg(t->ar.arexpr);
        }
        case TFOR: {
            if (sfputu(outfile, t->for_.forline) < 0) return -1;
            if (dump_p_tree(t->for_.fortre) < 0) return -1;
            if (dump_p_string(t->for_.fornam) < 0) return -1;
            return dump_p_tree((Shnode_t *)t->for_.forlst);
        }
        case TSW: {
            if (sfputu(outfile, t->sw.swline) < 0) return -1;
            if (dump_p_arg(t->sw.swarg) < 0) return -1;
            return dump_p_switch(t->sw.swlst);
        }
        case TFUN: {
            if (sfputu(outfile, t->funct.functline) < 0) return -1;
            if (dump_p_string(t->funct.functnam) < 0) return -1;
            if (dump_p_tree(t->funct.functtre) < 0) return -1;
            return dump_p_tree((Shnode_t *)t->funct.functargs);
        }
        case TTST: {
            if (sfputu(outfile, t->tst.tstline) < 0) return -1;
            if ((t->tre.tretyp & TPAREN) == TPAREN) return dump_p_tree(t->lst.lstlef);
            if (dump_p_arg(&(t->lst.lstlef->arg)) < 0) return -1;
            if ((t->tre.tretyp & TBINARY)) return dump_p_arg(&(t->lst.lstrit->arg));
            return 0;
        }
        default: { return -1; }
    }
}

static_fn int dump_p_arg(const struct argnod *arg) {
    ssize_t n;
    struct fornod *fp;

    while (arg) {
        if ((n = strlen(arg->argval)) ||
            (arg->argflag & ~(ARG_APPEND | ARG_MESSAGE | ARG_QUOTED | ARG_ARRAY))) {
            fp = NULL;
        } else {
            fp = (struct fornod *)arg->argchn.ap;
            n = strlen(fp->fornam) + 1;
        }
        sfputu(outfile, n + 1);
        if (fp) {
            sfputc(outfile, 0);
            sfwrite(outfile, fp->fornam, n - 1);
        } else {
            sfwrite(outfile, arg->argval, n);
        }
        sfputc(outfile, arg->argflag);
        if (fp) {
            sfputu(outfile, fp->fortyp);
            dump_p_tree(fp->fortre);
        } else if (n == 0 && (arg->argflag & ARG_EXP) && arg->argchn.ap) {
            dump_p_tree((Shnode_t *)arg->argchn.ap);
        }
        arg = arg->argnxt.ap;
    }
    return sfputu(outfile, 0);
}

static_fn int dump_p_redirect(const struct ionod *iop) {
    while (iop) {
        if (iop->iovname) {
            sfputl(outfile, iop->iofile | IOVNM);
        } else {
            sfputl(outfile, iop->iofile);
        }
        dump_p_string(iop->ioname);
        if (iop->iodelim) {
            dump_p_string(iop->iodelim);
            sfputl(outfile, iop->iosize);
            sfseek(sh.heredocs, iop->iooffset, SEEK_SET);
            sfmove(sh.heredocs, outfile, iop->iosize, -1);
        } else {
            sfputu(outfile, 0);
        }
        if (iop->iovname) dump_p_string(iop->iovname);
        iop = iop->ionxt;
    }
    return sfputl(outfile, -1);
}

static_fn int dump_p_comarg(const struct comnod *com) {
    dump_p_redirect(com->comio);
    dump_p_arg(com->comset);
    if (!com->comarg) {
        sfputl(outfile, -1);
    } else if (com->comtyp & COMSCAN) {
        dump_p_arg(com->comarg);
    } else {
        dump_p_comlist((struct dolnod *)com->comarg);
    }
    return sfputu(outfile, com->comline);
}

static_fn int dump_p_comlist(const struct dolnod *dol) {
    char *const *argv;
    int n;

    argv = dol->dolval + ARG_SPARE;
    while (*argv) argv++;
    n = argv - (dol->dolval + 1);
    sfputl(outfile, n);

    argv = dol->dolval + ARG_SPARE;
    while (*argv) dump_p_string(*argv++);

    return sfputu(outfile, 0);
}

static_fn int dump_p_switch(const struct regnod *reg) {
    while (reg) {
        sfputl(outfile, reg->regflag);
        dump_p_arg(reg->regptr);
        dump_p_tree(reg->regcom);
        reg = reg->regnxt;
    }
    return sfputl(outfile, -1);
}

static_fn int dump_p_string(const char *string) {
    size_t n = strlen(string);
    if (sfputu(outfile, n + 1) < 0) return -1;
    return sfwrite(outfile, string, n);
}
