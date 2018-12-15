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
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "defs.h"

#include "ast.h"
#include "ast_assert.h"
#include "builtins.h"
#include "cdt.h"
#include "error.h"
#include "fault.h"
#include "name.h"
#include "path.h"
#include "sfio.h"
#include "stk.h"
#include "variables.h"

static_fn void assign(Namval_t *, const void *, int, Namfun_t *);

const Nvdisc_op_t DISC_OP_NOOP = {DISC_OP_NOOP_val};
const Nvdisc_op_t DISC_OP_FIRST = {DISC_OP_FIRST_val};
const Nvdisc_op_t DISC_OP_LAST = {DISC_OP_LAST_val};
const Nvdisc_op_t DISC_OP_POP = {DISC_OP_POP_val};
const Nvdisc_op_t DISC_OP_CLONE = {DISC_OP_CLONE_val};

int nv_compare(Dt_t *dict, void *sp, void *dp, Dtdisc_t *disc) {
    UNUSED(dict);
    UNUSED(disc);

    if (sp == dp) return 0;
    return strcmp((char *)sp, (char *)dp);
}

//
// Call the next getval function in the chain.
//
char *nv_getv(Namval_t *np, Namfun_t *nfp) {
    Shell_t *shp = sh_ptr(np);
    Namfun_t *fp;
    char *cp;

    if ((fp = nfp) != NULL && !nv_local) fp = nfp = nfp->next;
    nv_local = 0;
    for (; fp; fp = fp->next) {
        if (!fp->disc || (!fp->disc->getnum && !fp->disc->getval)) continue;
        if (!nv_isattr(np, NV_NODISC) || fp == (Namfun_t *)nv_arrayptr(np)) break;
    }
    if (fp && fp->disc->getval)
        cp = (*fp->disc->getval)(np, fp);
    else if (fp && fp->disc->getnum) {
        sfprintf(shp->strbuf, "%.*Lg", 12, (*fp->disc->getnum)(np, fp));
        cp = sfstruse(shp->strbuf);
    } else {
        nv_local = 1;
        cp = nv_getval(np);
    }
    return cp;
}

//
// Call the next getnum function in the chain.
//
Sfdouble_t nv_getn(Namval_t *np, Namfun_t *nfp) {
    Namfun_t *fp;
    Sfdouble_t d = 0;
    Shell_t *shp = sh_ptr(np);
    char *str;

    if ((fp = nfp) != NULL && !nv_local) fp = nfp = nfp->next;
    nv_local = 0;
    for (; fp; fp = fp->next) {
        if (!fp->disc || (!fp->disc->getnum && !fp->disc->getval)) continue;
        if (!fp->disc->getnum && nv_isattr(np, NV_INTEGER)) continue;
        if (!nv_isattr(np, NV_NODISC) || fp == (Namfun_t *)nv_arrayptr(np)) break;
    }
    if (fp && fp->disc && fp->disc->getnum) {
        d = (*fp->disc->getnum)(np, fp);
    } else if (nv_isattr(np, NV_INTEGER)) {
        nv_local = 1;
        d = nv_getnum(np);
    } else {
        if (fp && fp->disc && fp->disc->getval) {
            str = (*fp->disc->getval)(np, fp);
        } else {
            str = nv_getv(np, fp ? fp : nfp);
        }
        if (str && *str) {
            if (nv_isattr(np, NV_LJUST | NV_RJUST) ||
                (*str == '0' && !(str[1] == 'x' || str[1] == 'X'))) {
                while (*str == '0') str++;
            }
            d = sh_arith(shp, str);
        }
    }
    return d;
}

//
// Call the next assign function in the chain.
//
void nv_putv(Namval_t *np, const void *value, int flags, Namfun_t *nfp) {
    Namfun_t *fp, *fpnext;
    Namarr_t *ap;

    if ((fp = nfp) != NULL && !nv_local) fp = nfp = nfp->next;
    nv_local = 0;
    if (flags & NV_NODISC) fp = 0;
    for (; fp; fp = fpnext) {
        fpnext = fp->next;
        if (!fp->disc || !fp->disc->putval) {
            if (!value && (!(ap = nv_arrayptr(np)) || ap->nelem == 0)) {
                if (fp->disc || !(fp->nofree & 1)) nv_disc(np, fp, DISC_OP_POP);
                if (!(fp->nofree & 1)) free(fp);
            }
            continue;
        }
        if (!nv_isattr(np, NV_NODISC) || fp == (Namfun_t *)nv_arrayptr(np)) break;
    }
    if (!value && (flags & NV_TYPE) && fp && fp->disc->putval == assign) fp = 0;
    if (fp && fp->disc->putval) {
        (*fp->disc->putval)(np, value, flags, fp);
    } else {
        nv_local = 1;
        if (value) {
            nv_putval(np, value, flags);
        } else {
            _nv_unset(np, flags & (NV_RDONLY | NV_EXPORT));
        }
    }
}

#define LOOKUPS 0
#define ASSIGN 1
#define APPEND 2
#define UNASSIGN 3
#define LOOKUPN 4
#define BLOCKED ((Namval_t *)&nv_local)

struct vardisc {
    Namfun_t fun;
    Namval_t *disc[5];
};

struct blocked {
    struct blocked *next;
    Namval_t *np;
    int flags;
    void *sub;
    int isub;
};

static struct blocked *blist;

#define isblocked(bp, type) ((bp)->flags & (1 << (type)))
#define block(bp, type) ((bp)->flags |= (1 << (type)))
#define unblock(bp, type) ((bp)->flags &= ~(1 << (type)))

//
// Returns pointer to blocking structure.
//
static_fn struct blocked *block_info(Namval_t *np, struct blocked *pp) {
    struct blocked *bp;
    void *sub = 0;
    int isub = 0;

    if (nv_isarray(np) && (isub = nv_aindex(np)) < 0) {
        sub = nv_associative(np, NULL, ASSOC_OP_CURRENT);
    }
    for (bp = blist; bp; bp = bp->next) {
        if (bp->np == np && bp->sub == sub && bp->isub == isub) return bp;
    }
    if (pp) {
        pp->np = np;
        pp->flags = 0;
        pp->isub = isub;
        pp->sub = sub;
        pp->next = blist;
        blist = pp;
    }
    return pp;
}

static_fn void block_done(struct blocked *bp) {
    blist = bp = bp->next;
    if (bp && (bp->isub >= 0 || bp->sub)) {
        nv_putsub(bp->np, bp->sub, (bp->isub < 0 ? 0 : bp->isub), ARRAY_SETSUB);
    }
}

//
// Free discipline if no more discipline functions.
//
static_fn void chktfree(Namval_t *np, struct vardisc *vp) {
    int n;

    for (n = 0; n < sizeof(vp->disc) / sizeof(*vp->disc); n++) {
        if (vp->disc[n]) break;
    }
    if (n >= sizeof(vp->disc) / sizeof(*vp->disc)) {
        // No disc left so pop.
        Namfun_t *fp;
        if ((fp = nv_stack(np, NULL)) && !(fp->nofree & 1)) free(fp);
    }
}

//
// This function performs an assignment disc on the given node <np>.
//
static_fn void assign(Namval_t *np, const void *val, int flags, Namfun_t *handle) {
    Shell_t *shp = sh_ptr(np);
    int type = (flags & NV_APPEND) ? APPEND : ASSIGN;
    struct vardisc *vp = (struct vardisc *)handle;
    Namval_t *nq = vp->disc[type];
    struct blocked block, *bp = block_info(np, &block);
    Namval_t node;
    union Value *up = np->nvalue.up;
    Namval_t *tp, *nr;

    if (val && (tp = nv_type(np)) &&
        (nr = nv_open(val, shp->var_tree, NV_VARNAME | NV_ARRAY | NV_NOADD | NV_NOFAIL)) &&
        tp == nv_type(nr)) {
        char *sub = nv_getsub(np);
        _nv_unset(np, 0);
        if (sub) {
            nv_putsub(np, sub, 0, ARRAY_ADD);
            nv_putval(np, nv_getval(nr), 0);
        } else {
            nv_clone(nr, np, 0);
        }
        goto done;
    }
    if (val || isblocked(bp, type)) {
        if (!nq || isblocked(bp, type)) {
            nv_putv(np, val, flags, handle);
            goto done;
        }
        node = *SH_VALNOD;
        if (!nv_isnull(SH_VALNOD)) {
            nv_onattr(SH_VALNOD, NV_NOFREE);
            _nv_unset(SH_VALNOD, 0);
        }
        if (flags & NV_INTEGER) {
            nv_onattr(SH_VALNOD,
                      (flags & (NV_LONG | NV_DOUBLE | NV_EXPNOTE | NV_HEXFLOAT | NV_SHORT)));
        }
        nv_putval(SH_VALNOD, val, (flags & NV_INTEGER) ? flags : NV_NOFREE);
    } else {
        nq = vp->disc[type = UNASSIGN];
    }
    if (nq && !isblocked(bp, type)) {
        int bflag = 0;
        block(bp, type);
        if (type == APPEND && (bflag = !isblocked(bp, LOOKUPS))) block(bp, LOOKUPS);
        sh_fun(shp, nq, np, (char **)0);
        unblock(bp, type);
        if (bflag) unblock(bp, LOOKUPS);
        if (!vp->disc[type]) chktfree(np, vp);
    }
    if (nv_isarray(np)) np->nvalue.up = up;
    if (val) {
        char *cp;
        Sfdouble_t d;
        if (nv_isnull(SH_VALNOD)) {
            cp = 0;
        } else if (flags & NV_INTEGER) {
            d = nv_getnum(SH_VALNOD);
            cp = (char *)(&d);
            flags |= (NV_LONG | NV_DOUBLE);
            flags &= ~NV_SHORT;
        } else {
            cp = nv_getval(SH_VALNOD);
        }
        if (cp) nv_putv(np, cp, flags | NV_RDONLY, handle);
        _nv_unset(SH_VALNOD, 0);
        // Restore everything but the nvlink field.
        memcpy(&SH_VALNOD->nvname, &node.nvname, sizeof(node) - sizeof(node.nvlink));
    } else if (sh_isstate(shp, SH_INIT) || np == SH_FUNNAMENOD) {
        // Don't free functions during reinitialization.
        nv_putv(np, val, flags, handle);
    } else if (!nq || !isblocked(bp, type)) {
        Dt_t *root = sh_subfuntree(shp, 1);
        Namval_t *pp = NULL;
        int n;

        block(bp, type);
        if (!nv_isattr(np, NV_MINIMAL)) pp = (Namval_t *)np->nvenv;
        nv_putv(np, val, flags, handle);
        if (!nv_isarray(np) || array_isempty(np)) nv_disc(np, handle, DISC_OP_POP);
        if (shp->subshell) goto done;
        if (pp && nv_isarray(pp)) goto done;
        if (nv_isarray(np) && !array_isempty(np)) goto done;
        for (n = 0; n < sizeof(vp->disc) / sizeof(*vp->disc); n++) {
            if ((nq = vp->disc[n]) && !nv_isattr(nq, NV_NOFREE)) {
                _nv_unset(nq, 0);
                dtdelete(root, nq);
            }
        }
        unblock(bp, type);
        if (!(handle->nofree & 1)) free(handle);
    }
done:
    if (bp == &block) block_done(bp);
    if (nq && nq->nvalue.rp->running == 1) {
        nq->nvalue.rp->running = 0;
        _nv_unset(nq, 0);
    }
}

//
// This function executes a lookup disc and then performs the lookup on the given node <np>.
//
static_fn char *lookup(Namval_t *np, int type, Sfdouble_t *dp, Namfun_t *handle) {
    Shell_t *shp = sh_ptr(np);
    struct vardisc *vp = (struct vardisc *)handle;
    struct blocked block, *bp = block_info(np, &block);
    Namval_t *nq = vp->disc[type];
    char *cp = 0;
    Namval_t node;
    union Value *up = np->nvalue.up;

    if (nq && !isblocked(bp, type)) {
        node = *SH_VALNOD;
        if (!nv_isnull(SH_VALNOD)) {
            nv_onattr(SH_VALNOD, NV_NOFREE);
            _nv_unset(SH_VALNOD, 0);
        }
        if (type == LOOKUPN) {
            nv_onattr(SH_VALNOD, NV_DOUBLE | NV_INTEGER);
            nv_setsize(SH_VALNOD, 10);
        }
        block(bp, type);
        sh_fun(shp, nq, np, (char **)0);
        unblock(bp, type);
        if (!vp->disc[type]) chktfree(np, vp);
        if (type == LOOKUPN) {
            cp = (char *)(SH_VALNOD->nvalue.cp);
            *dp = nv_getnum(SH_VALNOD);
        } else if ((cp = nv_getval(SH_VALNOD))) {
            cp = stkcopy(stkstd, cp);
        }
        _nv_unset(SH_VALNOD, NV_RDONLY);
        if (!nv_isnull(&node)) {
            // Restore everything but the nvlink field.
            memcpy(&SH_VALNOD->nvname, &node.nvname, sizeof(node) - sizeof(node.nvlink));
        }
    }
    if (nv_isarray(np)) np->nvalue.up = up;
    if (!cp) {
        if (type == LOOKUPS) {
            cp = nv_getv(np, handle);
        } else {
            *dp = nv_getn(np, handle);
        }
    }
    if (bp == &block) block_done(bp);
    if (nq && nq->nvalue.rp && nq->nvalue.rp->running == 1) {
        nq->nvalue.rp->running = 0;
        _nv_unset(nq, 0);
    }
    return cp;
}

static_fn char *lookups(Namval_t *np, Namfun_t *handle) {
    return lookup(np, LOOKUPS, NULL, handle);
}

static_fn Sfdouble_t lookupn(Namval_t *np, Namfun_t *handle) {
    Sfdouble_t d;
    lookup(np, LOOKUPN, &d, handle);
    return d;
}

//
// Set disc on given <event> to <action>.
// If action==np, the current disc is returned.
// A null return value indicates that no <event> is known for <np>.
// If <event> is NULL, then return the event name after <action>.
// If <event> is NULL, and <action> is NULL, return the first event.
//
char *nv_setdisc(Namval_t *np, const void *event, Namval_t *action, Namfun_t *fp) {
    struct vardisc *vp = (struct vardisc *)np->nvfun;
    int type;
    char *empty = "";

    while (vp) {
        if (vp->fun.disc && (vp->fun.disc->setdisc || vp->fun.disc->putval == assign)) break;
        vp = (struct vardisc *)vp->fun.next;
    }
    if (vp && !vp->fun.disc) vp = 0;
    if (np == (Namval_t *)fp) {
        const char *name;
        int getname = 0;
        // Top level call, check for get/set.
        if (!event) {
            if (!action) return (char *)nv_discnames[0];
            getname = 1;
            event = (char *)action;
        }
        for (type = 0; (name = nv_discnames[type]); type++) {
            if (strcmp(event, name) == 0) break;
        }
        if (getname) {
            event = 0;
            if (name && !(name = nv_discnames[++type])) action = 0;
        }
        if (!name) {
            for (fp = (Namfun_t *)vp; fp; fp = fp->next) {
                if (fp->disc && fp->disc->setdisc) {
                    return (*fp->disc->setdisc)(np, event, action, fp);
                }
            }
        } else if (getname) {
            return (char *)name;
        }
    }
    if (!fp) return NULL;
    if (np != (Namval_t *)fp) {
        // Not the top level.
        while ((fp = fp->next)) {
            if (fp->disc && fp->disc->setdisc) return (*fp->disc->setdisc)(np, event, action, fp);
        }
        return NULL;
    }
    // Handle GET/SET/APPEND/UNSET disc.
    if (vp && vp->fun.disc->putval != assign) vp = NULL;
    if (!vp) {
        Namdisc_t *dp;
        if (action == np) return (char *)action;
        vp = calloc(1, sizeof(struct vardisc) + sizeof(Namdisc_t));
        if (!vp) return NULL;
        dp = (Namdisc_t *)(vp + 1);
        vp->fun.disc = dp;
        memset(dp, 0, sizeof(*dp));
        dp->dsize = sizeof(struct vardisc);
        dp->putval = assign;
        if (nv_isarray(np) && !nv_arrayptr(np)) nv_putsub(np, NULL, 1, 0);
        nv_stack(np, (Namfun_t *)vp);
    }
    if (action == np) {
        action = vp->disc[type];
        empty = 0;
    } else if (action) {
        Namdisc_t *dp = (Namdisc_t *)vp->fun.disc;
        if (type == LOOKUPS) {
            dp->getval = lookups;
        } else if (type == LOOKUPN) {
            dp->getnum = lookupn;
        }
        vp->disc[type] = action;
    } else {
        struct blocked *bp;
        action = vp->disc[type];
        vp->disc[type] = 0;
        if (!(bp = block_info(np, NULL)) || !isblocked(bp, UNASSIGN)) {
            chktfree(np, vp);
        }
    }
    return action ? (char *)action : empty;
}

//
// Set disc on given <event> to <action>.
// If action==np, the current disc is returned.
// A null return value indicates that no <event> is known for <np>.
// If <event> is NULL, then return the event name after <action>.
// If <event> is NULL, and <action> is NULL, return the first event.
//
static_fn char *setdisc(Namval_t *np, const void *vp, Namval_t *action, Namfun_t *fp) {
    const char *event = vp;
    Nambfun_t *bp = (Nambfun_t *)fp;
    int type, getname = 0;
    const char *name;
    const char **discnames = bp->bnames;

    // Top level call, check for discipline match.
    if (!event) {
        if (!action) return (char *)discnames[0];
        getname = 1;
        event = (char *)action;
    }
    for (type = 0; (name = discnames[type]); type++) {
        if (strcmp(event, name) == 0) break;
    }
    if (getname) {
        event = 0;
        if (name && !(name = discnames[++type])) action = 0;
    }
    if (!name) {
        return nv_setdisc(np, event, action, fp);
    } else if (getname) {
        return (char *)name;
    }
    // Handle the disciplines.
    if (action == np) {
        action = bp->bltins[type];
    } else if (action) {
        Namval_t *tp = nv_type(np);
        if (tp && (np = (Namval_t *)bp->bltins[type]) && nv_isattr(np, NV_STATICF)) {
            errormsg(SH_DICT, ERROR_exit(1), e_staticfun, name, tp->nvname);
        }
        bp->bltins[type] = action;
    } else {
        action = bp->bltins[type];
        bp->bltins[type] = 0;
    }
    return (char *)action;
}

static_fn void putdisc(Namval_t *np, const void *val, int flag, Namfun_t *fp) {
    Shell_t *shp = sh_ptr(np);
    nv_putv(np, val, flag, fp);
    if (!val && !(flag & NV_NOFREE)) {
        Nambfun_t *vp = (Nambfun_t *)fp;
        int i;
        for (i = 0; vp->bnames[i]; i++) {
            Namval_t *mp;
            mp = vp->bltins[i];
            if (mp && !nv_isattr(mp, NV_NOFREE) && is_abuiltin(mp)) {
                if (mp->nvfun && !nv_isattr(mp, NV_NOFREE)) free(mp->nvfun);
                dtdelete(shp->bltin_tree, mp);
                free(mp);
            }
        }
        nv_disc(np, fp, DISC_OP_POP);
        if (!(fp->nofree & 1)) free(fp);
    }
}

static const Namdisc_t Nv_bdisc = {.dsize = 0, .putval = putdisc, .setdisc = setdisc};

Namfun_t *nv_clone_disc(Namfun_t *fp, int flags) {
    Namfun_t *nfp;
    int size;

    if (!fp->disc && !fp->next && (fp->nofree & 1)) return fp;
    if (!(size = fp->dsize) && (!fp->disc || !(size = fp->disc->dsize))) size = sizeof(Namfun_t);
    nfp = calloc(1, size);
    if (!nfp) return NULL;
    memcpy(nfp, fp, size);
    nfp->nofree &= ~1;
    nfp->nofree |= (flags & NV_RDONLY) ? 1 : 0;
    return nfp;
}

bool nv_adddisc(Namval_t *np, const char **names, Namval_t **funs) {
    Nambfun_t *vp;
    int n = 0;
    const char **av = names;

    if (av) {
        while (*av++) n++;
    }
    vp = calloc(1, sizeof(Nambfun_t) + n * sizeof(Namval_t *));
    if (!vp) return false;
    vp->fun.dsize = sizeof(Nambfun_t) + n * sizeof(Namval_t *);
    vp->fun.nofree |= 2;
    vp->num = n;
    if (funs) {
        memcpy((void *)vp->bltins, (void *)funs, n * sizeof(Namval_t *));
    } else {
        while (n >= 0) vp->bltins[n--] = 0;
    }
    vp->fun.disc = &Nv_bdisc;
    vp->bnames = names;
    nv_stack(np, &vp->fun);
    return true;
}

//
// Push, pop, clone, or reorder disciplines onto node <np>.
// <op> can be one of
//    DISC_OP_NOOP:   ???
//    DISC_OP_FIRST:  Move or push <fp> to top of the stack or delete top.
//    DISC_OP_LAST:   Move or push <fp> to bottom of stack or delete last.
//    DISC_OP_POP:    Delete <fp> from top of the stack.
//    DISC_OP_CLONE:  Replace <fp> with a copy created my malloc() and return it.
//
Namfun_t *nv_disc(Namval_t *np, Namfun_t *fp, Nvdisc_op_t op) {
    Shell_t *shp = sh_ptr(np);
    Namfun_t *lp, **lpp;

    if (nv_isref(np)) return NULL;
    if (op.val == DISC_OP_CLONE_val && !fp) return NULL;

    if (fp) {
        fp->subshell = shp->subshell;
        if ((lp = np->nvfun) == fp) {
            if (op.val == DISC_OP_CLONE_val) {
                lp = nv_clone_disc(fp, 0);
                np->nvfun = lp;
                return lp;
            }
            if (op.val == DISC_OP_FIRST_val || op.val == DISC_OP_NOOP_val) return fp;
            np->nvfun = lp->next;
            if (op.val == DISC_OP_POP_val) return fp;
            if (op.val == DISC_OP_LAST_val && (lp->next == 0 || lp->next->disc == 0)) return fp;
        }
        // See if <fp> is on the list already.
        lpp = &np->nvfun;
        if (lp) {
            while (lp->next && lp->next->disc) {
                if (lp->next == fp) {
                    if (op.val == DISC_OP_LAST_val && fp->next == 0) return fp;
                    if (op.val == DISC_OP_CLONE_val) {
                        fp = nv_clone_disc(fp, 0);
                        lp->next = fp;
                        return fp;
                    }
                    lp->next = fp->next;
                    if (op.val == DISC_OP_POP_val) return fp;
                    if (op.val != DISC_OP_LAST_val) break;
                }
                lp = lp->next;
            }
            if (op.val == DISC_OP_LAST_val && lp->disc) lpp = &lp->next;
        }
        if (op.val == DISC_OP_POP_val) return NULL;
        // Push.
        nv_offattr(np, NV_NODISC);
        if (op.val == DISC_OP_LAST_val) {
            if (lp && !lp->disc) {
                fp->next = lp;
            } else {
                fp->next = NULL;
            }
        } else {
            if ((fp->nofree & 1) && *lpp) fp = nv_clone_disc(fp, 0);
            fp->next = *lpp;
        }
        *lpp = fp;
    } else {
        if (op.val == DISC_OP_FIRST_val) {
            return np->nvfun;
        } else if (op.val == DISC_OP_LAST_val) {
            for (lp = np->nvfun; lp; fp = lp, lp = lp->next) {
                ;  // empty loop
            }
        } else if ((fp = np->nvfun)) {
            np->nvfun = fp->next;
        }
    }
    return fp;
}

//
// Returns discipline pointer if discipline with specified functions is on the discipline stack.
//
Namfun_t *nv_hasdisc(Namval_t *np, const Namdisc_t *dp) {
    Namfun_t *fp;

    for (fp = np->nvfun; fp; fp = fp->next) {
        if (fp->disc == dp) return fp;
    }
    return 0;
}

struct notify {
    Namfun_t hdr;
    char **ptr;
};

static_fn void put_notify(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    struct notify *pp = (struct notify *)fp;

    nv_putv(np, val, flags, fp);
    nv_stack(np, fp);
    nv_stack(np, NULL);
    *pp->ptr = NULL;
    if (!(fp->nofree & 1)) free(fp);
}

static const Namdisc_t notify_disc = {.dsize = 0, .putval = put_notify};

bool nv_unsetnotify(Namval_t *np, char **addr) {
    Namfun_t *fp;

    for (fp = np->nvfun; fp; fp = fp->next) {
        if (fp->disc->putval == put_notify && ((struct notify *)fp)->ptr == addr) {
            nv_stack(np, fp);
            nv_stack(np, NULL);
            if (!(fp->nofree & 1)) free(fp);
            return true;
        }
    }
    return false;
}

bool nv_setnotify(Namval_t *np, char **addr) {
    struct notify *pp = calloc(1, sizeof(struct notify));

    if (!pp) return false;
    pp->ptr = addr;
    pp->hdr.disc = &notify_disc;
    nv_stack(np, &pp->hdr);
    return true;
}

static_fn void *newnode(const char *name) {
    size_t s = strlen(name);
    Namval_t *np = calloc(1, sizeof(Namval_t) + s + 1);

    if (!np) return NULL;
    np->nvname = (char *)np + sizeof(Namval_t);
    memcpy(np->nvname, name, s);
    return np;
}

//
// Clone a numeric value.
//
static_fn void *num_clone(Namval_t *np, void *val) {
    int size;
    void *nval;

    if (!val) return 0;
    if (nv_isattr(np, NV_DOUBLE) == NV_DOUBLE) {
        if (nv_isattr(np, NV_LONG)) {
            size = sizeof(Sfdouble_t);
        } else if (nv_isattr(np, NV_SHORT)) {
            size = sizeof(float);
        } else {
            size = sizeof(double);
        }
    } else {
        if (nv_isattr(np, NV_LONG)) {
            size = sizeof(Sflong_t);
        } else if (nv_isattr(np, NV_SHORT)) {
            if (nv_isattr(np, NV_INT16P | NV_DOUBLE) == NV_INT16P) {
                size = sizeof(short);
            } else {
                return (void *)np->nvalue.ip;
            }
        } else {
            size = sizeof(int32_t);
        }
    }
    if (!(nval = malloc(size))) return 0;
    memcpy(nval, val, size);
    return nval;
}

void clone_all_disc(Namval_t *np, Namval_t *mp, int flags) {
    Namfun_t *fp, **mfp = &mp->nvfun, *nfp, *fpnext;

    for (fp = np->nvfun; fp; fp = fpnext) {
        fpnext = fp->next;
        if (!fpnext && (flags & NV_COMVAR) && fp->disc && fp->disc->namef) return;
        if ((fp->nofree & 2) && (flags & NV_NODISC)) nfp = 0;
        if (fp->disc && fp->disc->clonef) {
            nfp = (*fp->disc->clonef)(np, mp, flags, fp);
        } else if (flags & NV_MOVE) {
            nfp = fp;
        } else {
            nfp = nv_clone_disc(fp, flags);
        }
        if (!nfp) continue;
        nfp->next = 0;
        *mfp = nfp;
        mfp = &nfp->next;
    }
}

//
// Clone <mp> from <np> flags can be one of the following:
// NV_APPEND - append <np> onto <mp>
// NV_MOVE - move <np> to <mp>
// NV_NOFREE - mark the new node as nofree
// NV_NODISC - discplines with funs non-zero will not be copied
// NV_COMVAR - cloning a compound variable
//
int nv_clone(Namval_t *np, Namval_t *mp, int flags) {
    Namfun_t *fp, *fpnext;
    const char *val = mp->nvalue.cp;
    unsigned short flag = mp->nvflag;
    size_t size = nv_size(mp);

    mp->nvshell = np->nvshell;
    for (fp = mp->nvfun; fp; fp = fpnext) {
        fpnext = fp->next;
        if (!fpnext && (flags & NV_COMVAR) && fp->disc && fp->disc->namef) break;
        if (!(fp->nofree & 1)) free(fp);
    }
    mp->nvfun = fp;
    fp = np->nvfun;
    if (fp) {
        Shell_t *shp = (Shell_t *)np->nvshell;
        Namval_t *last_table = shp->last_table;
        if (nv_isattr(mp, NV_EXPORT | NV_MINIMAL) == (NV_EXPORT | NV_MINIMAL)) {
            mp->nvenv = 0;
            nv_offattr(mp, NV_MINIMAL);
        }
        if (!(flags & NV_COMVAR) && !nv_isattr(np, NV_MINIMAL) && np->nvenv &&
            !(nv_isattr(mp, NV_MINIMAL))) {
            mp->nvenv = np->nvenv;
        }
        mp->nvflag &= NV_MINIMAL;
        mp->nvflag |= np->nvflag & ~(NV_ARRAY | NV_MINIMAL | NV_NOFREE);
        flag = mp->nvflag;
        clone_all_disc(np, mp, flags);
        shp->last_table = last_table;
    }
    if (flags & NV_APPEND) return 1;
    if (nv_size(mp) == size) nv_setsize(mp, nv_size(np));
    if (mp->nvflag == flag) {
        nv_setattr(mp, (np->nvflag & ~(NV_MINIMAL)) | (mp->nvflag & NV_MINIMAL));
    }
    if (nv_isattr(np, NV_EXPORT)) mp->nvflag |= (np->nvflag & NV_MINIMAL);
    if (mp->nvalue.cp == val && !nv_isattr(np, NV_INTEGER)) {
        if (np->nvalue.cp && np->nvalue.cp != Empty && (flags & NV_COMVAR) && !(flags & NV_MOVE)) {
            if (size) {
                mp->nvalue.cp = (char *)memdup(np->nvalue.cp, size);
            } else {
                mp->nvalue.cp = strdup(np->nvalue.cp);
            }
            nv_offattr(mp, NV_NOFREE);
        } else if ((np->nvfun || !nv_isattr(np, NV_ARRAY)) && !(mp->nvalue.cp = np->nvalue.cp)) {
            nv_offattr(mp, NV_NOFREE);
        }
    }
    mp->nvshell = np->nvshell;
    if (flags & NV_MOVE) {
        if (nv_isattr(np, NV_INTEGER)) mp->nvalue.ip = np->nvalue.ip;
        np->nvfun = 0;
        np->nvalue.cp = 0;
        if (!nv_isattr(np, NV_MINIMAL) || nv_isattr(mp, NV_EXPORT)) {
            mp->nvenv = np->nvenv;
            if (nv_isattr(np, NV_MINIMAL)) {
                np->nvenv = 0;
                nv_setattr(np, NV_EXPORT);
            } else {
                nv_setattr(np, 0);
            }
        } else {
            np->nvflag &= NV_MINIMAL;
        }
        return 1;
    } else if ((flags & NV_ARRAY) && !nv_isattr(np, NV_MINIMAL)) {
        mp->nvenv = np->nvenv;
    }
    if (nv_isattr(np, NV_INTEGER) && mp->nvalue.ip != np->nvalue.ip && np->nvalue.cp != Empty) {
        mp->nvalue.ip = (int *)num_clone(np, (void *)np->nvalue.ip);
        nv_offattr(mp, NV_NOFREE);
    } else if ((flags & NV_NOFREE) && !nv_arrayptr(np)) {
        nv_onattr(np, NV_NOFREE);
    }
    return 1;
}

//
// The following discipline is for copy-on-write semantics.
//
static_fn char *clone_getv(Namval_t *np, Namfun_t *handle) {
    UNUSED(handle);

    return np->nvalue.np ? nv_getval(np->nvalue.np) : 0;
}

static_fn Sfdouble_t clone_getn(Namval_t *np, Namfun_t *handle) {
    UNUSED(handle);

    return np->nvalue.np ? nv_getnum(np->nvalue.np) : 0;
}

static_fn void clone_putv(Namval_t *np, const void *val, int flags, Namfun_t *handle) {
    UNUSED(handle);
    Shell_t *shp = sh_ptr(np);
    Namfun_t *dp = nv_stack(np, NULL);
    Namval_t *mp = np->nvalue.np;
    if (!shp->subshell) free(dp);
    if (val) nv_clone(mp, np, NV_NOFREE);
    np->nvalue.cp = 0;
    nv_putval(np, val, flags);
}

static const Namdisc_t clone_disc = {
    .dsize = 0, .putval = clone_putv, .getval = clone_getv, .getnum = clone_getn};

Namval_t *nv_mkclone(Namval_t *mp) {
    Shell_t *shp = sh_ptr(mp);
    Namval_t *np;
    Namfun_t *dp;
    np = calloc(1, sizeof(Namval_t));
    nv_setattr(np, mp->nvflag);
    np->nvsize = mp->nvsize;
    np->nvname = mp->nvname;
    np->nvshell = mp->nvshell;
    np->nvalue.np = mp;
    nv_setattr(np, mp->nvflag);
    dp = calloc(1, sizeof(Namfun_t));
    dp->disc = &clone_disc;
    nv_stack(np, dp);
    dtinsert(nv_dict(shp->namespace), np);
    return np;
}

Namval_t *nv_search(const char *name, Dt_t *root, int mode) {
    Shell_t *shp = sh_getinterp();

    Namval_t *np;
    Dt_t *dp = 0;
    if (mode & NV_NOSCOPE) dp = dtview(root, 0);
    if (mode & HASH_BUCKET) {
        Namval_t *mp = (void *)name;
        if (!(np = dtsearch(root, mp)) && (mode & NV_ADD)) name = nv_name(mp);
    } else {
        if (*name == '.' && root == shp->var_tree && !dp) root = shp->var_base;
        np = dtmatch(root, (void *)name);
    }
#if SHOPT_COSHELL
    if (shp->inpool) mode |= NV_NOSCOPE;
#endif  // SHOPT_COSHELL
    if (!np && (mode & NV_ADD)) {
        if (shp->namespace && !(mode & NV_NOSCOPE) && root == shp->var_tree) {
            root = nv_dict(shp->namespace);
        } else if (!dp && !(mode & NV_NOSCOPE)) {
            Dt_t *next;
            while ((next = dtvnext(root))) root = next;
        }
        np = (Namval_t *)dtinsert(root, newnode(name));
        np->nvshell = sh_getinterp();
    }
    if (dp) dtview(root, dp);
    return np;
}

//
// Finds function or builtin for given name and the discipline variable. if var!=0 the variable
// pointer is returned and the built-in name is put onto the stack at the current offset. Otherwise,
// a pointer to the builtin (variable or type) is returned and var contains the poiner to the
// variable. If last==0 and first component of name is a reference, nv_bfsearch() will return 0.
//
Namval_t *nv_bfsearch(const char *name, Dt_t *root, Namval_t **var, char **last) {
    Shell_t *shp = sh_getinterp();
    int c, offset = stktell(shp->stk);
    char *sp, *cp = 0;
    Namval_t *np, *nq;
    char *dname = 0;

    if (var) *var = 0;
    // Check for . in the name before =.
    for (sp = (char *)name + 1; *sp; sp++) {
        if (*sp == '=') return 0;
        if (*sp == '[') {
            while (*sp == '[') {
                sp = nv_endsubscript(NULL, (char *)sp, 0, (void *)shp);
                if (sp[-1] != ']') return 0;
            }
            if (*sp == 0) break;
            if (*sp != '.') return 0;
            cp = sp;
        } else if (*sp == '.') {
            cp = sp;
        }
    }
    if (!cp) {
        if (!var) return 0;
        if (shp->namespace) {
            sfprintf(shp->strbuf, "%s.%s%c", nv_name(shp->namespace), name, 0);
            np = nv_search(sfstruse(shp->strbuf), root, 0);
            if (np) return np;
        }
        return nv_search(name, root, 0);
    }
    sfputr(shp->stk, name, 0);
    dname = cp + 1;
    cp = stkptr(shp->stk, offset) + (cp - name);
    if (last) *last = cp;
    c = *cp;
    *cp = 0;
    nq = nv_open(stkptr(shp->stk, offset), shp->var_tree, NV_VARNAME | NV_NOADD | NV_NOFAIL);
    *cp = c;
    if (!nq) {
        np = 0;
        goto done;
    }
    if (!var) {
        np = nq;
        goto done;
    }
    *var = nq;
    if (c == '[') nv_endsubscript(nq, cp, NV_NOADD, nq->nvshell);
    stkseek(shp->stk, offset);
    if (nv_istable(nq)) {
        Namval_t *nsp = shp->namespace;
        if (last == 0) return nv_search(name, root, 0);
        shp->namespace = 0;
        sfputr(shp->stk, nv_name(nq), -1);
        shp->namespace = nsp;
        sfputr(shp->stk, dname - 1, 0);
        np = nv_search(stkptr(shp->stk, offset), root, 0);
        stkseek(shp->stk, offset);
        return np;
    }
    while (nv_isarray(nq) && !nv_isattr(nq, NV_MINIMAL | NV_EXPORT) && nq->nvenv &&
           nv_isarray((Namval_t *)nq->nvenv)) {
        nq = (Namval_t *)nq->nvenv;
    }
    return (Namval_t *)nv_setdisc(nq, dname, nq, (Namfun_t *)nq);
done:
    stkseek(shp->stk, offset);
    return np;
}

//
// Add or replace built-in version of command corresponding to <path>. The <bltin> argument is a
// pointer to the built-in. If <extra>==builtin_delete, the built-in will be deleted. If
// <extra>==builtin_disable, the built-in will be disabled. Special builtins cannot be
// added or deleted return failure. The return value for adding builtins is a pointer to the node or
// NULL on failure.  For delete NULL means success and the node that cannot be deleted is returned
// on failure.
//
Namval_t *sh_addbuiltin(Shell_t *shp, const char *path, Shbltin_f bltin, void *extra) {
    const char *name;
    char *cp;
    Namval_t *np, *nq = NULL;
    int offset = stktell(shp->stk);

    if (extra == builtin_delete) {
        name = path;
    } else if ((name = path_basename(path)) == path && bltin != (Shbltin_f)SYSTYPESET->nvalue.bfp &&
               (nq = nv_bfsearch(name, shp->bltin_tree, (Namval_t **)0, &cp))) {
        path = name = stkptr(shp->stk, offset);
    } else if (shp->bltin_dir && extra != builtin_delete) {
        sfputr(shp->stk, shp->bltin_dir, '/');
        sfputr(shp->stk, name, 0);
        path = stkptr(shp->stk, offset);
    }
    np = nv_search(name, shp->bltin_tree, 0);
    if (np) {
        // Exists without a path.
        stkseek(shp->stk, offset);
        if (extra == builtin_delete) {
            if (nv_isattr(np, BLT_SPC)) {
                errormsg(SH_DICT, ERROR_exit(1), "Cannot delete: %s%s", name, is_spcbuiltin);
            }
            if (np->nvfun && !nv_isattr(np, NV_NOFREE)) free(np->nvfun);
            dtdelete(shp->bltin_tree, np);
            return NULL;
        } else if (extra == builtin_disable) {
            nv_onattr(np, BLT_DISABLE);
            return NULL;
        }
        if (!bltin) return np;
    } else {
        for (np = (Namval_t *)dtfirst(shp->bltin_tree); np;
             np = (Namval_t *)dtnext(shp->bltin_tree, np)) {
            if (strcmp(name, path_basename(nv_name(np)))) continue;
            // Exists probably with different path so delete it.
            if (strcmp(path, nv_name(np))) {
                if (nv_isattr(np, BLT_SPC)) return np;
                if (!bltin) bltin = (Shbltin_f)np->nvalue.bfp;
                if (extra == builtin_delete) {
                    dtdelete(shp->bltin_tree, np);
                    return NULL;
                }
                np = NULL;
            }
            break;
        }
    }
    if (!np && !(np = nv_search(path, shp->bltin_tree, bltin ? NV_ADD : 0))) return NULL;
    stkseek(shp->stk, offset);
    if (nv_isattr(np, BLT_SPC)) {
        if (extra) np->nvfun = extra;
        return np;
    }
    np->nvenv = 0;
    np->nvfun = 0;
    if (bltin) {
        np->nvalue.bfp = (Nambfp_f)bltin;
        nv_onattr(np, NV_BLTIN | NV_NOFREE);
        np->nvfun = extra;
    }
    if (nq) {
        cp = nv_setdisc(nq, cp + 1, np, (Namfun_t *)nq);
        nv_close(nq);
        if (!cp) errormsg(SH_DICT, ERROR_exit(1), e_baddisc, name);
    }
    if (extra == builtin_delete) return NULL;
    return np;
}

struct table {
    Namfun_t fun;
    Namval_t *parent;
    Shell_t *shp;
    Dt_t *dict;
};

static_fn Namval_t *next_table(Namval_t *np, Dt_t *root, Namfun_t *fp) {
    struct table *tp = (struct table *)fp;
    if (root) return (Namval_t *)dtnext(root, np);
    return (Namval_t *)dtfirst(tp->dict);
}

static_fn Namval_t *create_table(Namval_t *np, const void *name, int flags, Namfun_t *fp) {
    struct table *tp = (struct table *)fp;
    tp->shp->last_table = np;
    return nv_create(name, tp->dict, flags, fp);
}

static_fn Namfun_t *clone_table(Namval_t *np, Namval_t *mp, int flags, Namfun_t *fp) {
    struct table *tp = (struct table *)fp;
    struct table *ntp = (struct table *)nv_clone_disc(fp, 0);
    Dt_t *oroot = tp->dict;
    Dt_t *nroot = dtopen(&_Nvdisc, Dtoset);
    assert(nroot);

    dtuserdata(nroot, dtuserdata(oroot, 0, 0), 1);
    memcpy(ntp, fp, sizeof(struct table));
    ntp->dict = nroot;
    ntp->parent = nv_lastdict(mp->nvshell);
    for (np = (Namval_t *)dtfirst(oroot); np; np = (Namval_t *)dtnext(oroot, np)) {
        mp = (Namval_t *)dtinsert(nroot, newnode(np->nvname));
        mp->nvshell = dtuserdata(nroot, 0, 0);
        nv_clone(np, mp, flags);
    }
    return &ntp->fun;
}

struct adata {
    Shell_t *sh;
    Namval_t *tp;
    char *mapname;
    char **argnam;
    int attsize;
    char *attval;
};

static_fn void delete_fun(Namval_t *np, void *data) {
    Shell_t *shp = ((struct adata *)data)->sh;
    nv_delete(np, shp->fun_tree, NV_NOFREE);
}

static_fn void put_table(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    Dt_t *root = ((struct table *)fp)->dict;
    Namval_t *nq, *mp;
    Namarr_t *ap;
    struct adata data;

    if (val) {
        nv_putv(np, val, flags, fp);
        return;
    }
    if (nv_isarray(np) && (ap = nv_arrayptr(np)) && array_elem(ap)) return;
    memset(&data, 0, sizeof(data));
    data.mapname = nv_name(np);
    data.sh = ((struct table *)fp)->shp;
    nv_scan(data.sh->fun_tree, delete_fun, (void *)&data, NV_FUNCTION, NV_FUNCTION | NV_NOSCOPE);
    dtview(root, 0);
    for (mp = (Namval_t *)dtfirst(root); mp; mp = nq) {
        _nv_unset(mp, flags);
        nq = (Namval_t *)dtnext(root, mp);
        dtdelete(root, mp);
        free(mp);
    }
    dtclose(root);
    if (!(fp->nofree & 1)) free(fp);
    np->nvfun = 0;
}

//
// Return space separated list of names of variables in given tree.
//
static_fn char *get_table(Namval_t *np, Namfun_t *fp) {
    Dt_t *root = ((struct table *)fp)->dict;
    static Sfio_t *out;
    int first = 1;
    Dt_t *base = dtview(root, 0);

    if (out) {
        sfseek(out, (Sfoff_t)0, SEEK_SET);
    } else {
        out = sfnew(NULL, NULL, -1, -1, SF_WRITE | SF_STRING);
    }
    for (np = (Namval_t *)dtfirst(root); np; np = (Namval_t *)dtnext(root, np)) {
        if (!nv_isnull(np) || np->nvfun || nv_isattr(np, ~NV_NOFREE)) {
            if (!first) {
                sfputc(out, ' ');
            } else {
                first = 0;
            }
            sfputr(out, np->nvname, -1);
        }
    }
    sfputc(out, 0);
    if (base) dtview(root, base);
    return (char *)out->_data;
}

static const Namdisc_t table_disc = {.dsize = sizeof(struct table),
                                     .putval = put_table,
                                     .getval = get_table,
                                     .createf = create_table,
                                     .clonef = clone_table,
                                     .nextf = next_table};

Namval_t *nv_parent(Namval_t *np) {
    struct table *tp = (struct table *)nv_hasdisc(np, &table_disc);
    if (tp) return tp->parent;
    return 0;
}

Dt_t *nv_dict(Namval_t *np) {
    Shell_t *shp = sh_ptr(np);
    struct table *tp = (struct table *)nv_hasdisc(np, &table_disc);
    if (tp) return tp->dict;
    np = shp->last_table;
    while (np) {
        tp = (struct table *)nv_hasdisc(np, &table_disc);
        if (tp) return tp->dict;
#if 0
		np = nv_create(np,(const char*)0, NV_FIRST, (Namfun_t*)0);
#else
        break;
#endif
    }
    return shp->var_tree;
}

bool nv_istable(Namval_t *np) { return nv_hasdisc(np, &table_disc) != 0; }

//
// Create a mountable name-value pair tree.
//
Namval_t *nv_mount(Namval_t *np, const char *name, Dt_t *dict) {
    Namval_t *mp, *pp;
    struct table *tp;

    dtuserdata(dict, sh_ptr(np), 1);
    if (nv_hasdisc(np, &table_disc)) {
        pp = np;
    } else {
        pp = nv_lastdict(np->nvshell);
    }
    tp = calloc(1, sizeof(struct table));
    if (!tp) return NULL;
    if (name) {
        Namfun_t *fp = pp->nvfun;
        mp = (*fp->disc->createf)(pp, name, 0, fp);
    } else {
        mp = np;
    }
    nv_offattr(mp, NV_TABLE);
    if (!nv_isnull(mp)) _nv_unset(mp, NV_RDONLY);
    tp->shp = sh_ptr(np);
    tp->dict = dict;
    tp->parent = pp;
    tp->fun.disc = &table_disc;
    nv_disc(mp, &tp->fun, DISC_OP_FIRST);
    return mp;
}

const Namdisc_t *nv_discfun(Nvdiscfun_op_t op) {
    if (op == DISCFUN_ADD) return &Nv_bdisc;
    if (op == DISCFUN_RESTRICT) return &RESTRICTED_disc;
    abort();
}

bool nv_hasget(Namval_t *np) {
    Namfun_t *fp;

    for (fp = np->nvfun; fp; fp = fp->next) {
        if (!fp->disc || (!fp->disc->getnum && !fp->disc->getval)) continue;
        return true;
    }
    return false;
}

Namval_t *sh_fsearch(Shell_t *shp, const char *fname, int add) {
    Stk_t *stkp = shp->stk;
    int offset = stktell(stkp);

    sfputr(stkp, nv_name(shp->namespace), '.');
    sfputr(stkp, fname, 0);
    fname = stkptr(stkp, offset);
    return nv_search(fname, sh_subfuntree(shp, add & NV_ADD), add);
}
