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
// Array processing routines
//
//   David Korn
//   dgkorn@gmail.com
//
#include "config_ast.h"  // IWYU pragma: keep

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"

#include "ast.h"
#include "ast_assert.h"
#include "cdt.h"
#include "error.h"
#include "fault.h"
#include "name.h"
#include "sfio.h"
#include "stk.h"

#define NUMSIZE 11
#define array_setbit(cp, n, b) (cp[n] |= (b))
#define array_clrbit(cp, n, b) (cp[n] &= ~(b))
#define array_isbit(cp, n, b) (cp[n] & (b))
#define NV_CHILD NV_EXPORT
#define ARRAY_CHILD 1
#define ARRAY_NOFREE 2
#define ARRAY_UNSET 4

struct index_array {
    Namarr_t namarr;
    void *xp;             // if set, subscripts will be converted
    int cur;              // index of current element
    int last;             // index of highest assigned element
    int maxi;             // maximum index for array
    unsigned char *bits;  // bit array for child subscripts
    union Value val[1];   // array of value holders
};

struct assoc_array {
    Namarr_t namarr;
    Namval_t *pos;
    Namval_t *nextpos;
    Namval_t *cur;
};

// Clone the index_array pointed to by `aq` and do what? What does the "scope" in the function name
// imply?
static_fn struct index_array *array_scope(Namval_t *np, struct index_array *aq, int flags) {
    Shell_t *shp = sh_ptr(np);
    struct index_array *ar;
    size_t size = aq->namarr.hdr.dsize;

    if (size == 0) size = aq->namarr.hdr.disc->dsize;
    ar = calloc(1, size);
    assert(ar);
    memcpy(ar, aq, size);
    if (flags & NV_RDONLY) {
        ar->namarr.hdr.nofree |= 1;
    } else {
        ar->namarr.hdr.nofree &= ~1;
    }
    if (is_associative(&ar->namarr)) {
        ar->namarr.scope = (void *)dtopen(&_Nvdisc, Dtoset);
        dtuserdata(ar->namarr.scope, shp, 1);
        dtview((Dt_t *)ar->namarr.scope, ar->namarr.table);
        ar->namarr.table = (Dt_t *)ar->namarr.scope;
        return ar;
    }
    ar->namarr.scope = aq;
    memset(ar->val, 0, ar->maxi * sizeof(char *));
    ar->bits = (unsigned char *)&ar->val[ar->maxi];
    return ar;
}

static_fn bool array_unscope(Namval_t *np, Namarr_t *ap) {
    Namfun_t *fp;

    if (!ap->scope) return false;
    if (is_associative(ap)) (*ap->fun)(np, NULL, NV_AFREE);
    fp = nv_disc(np, (Namfun_t *)ap, NV_POP);
    if (fp && !(fp->nofree & 1)) free(fp);
    nv_delete(np, NULL, 0);
    return true;
}

static_fn void array_syncsub(Namarr_t *ap, Namarr_t *aq) {
    ((struct index_array *)ap)->cur = ((struct index_array *)aq)->cur;
}

static_fn bool array_covered(Namval_t *np, struct index_array *ap) {
    UNUSED(np);
    struct index_array *aq = (struct index_array *)ap->namarr.scope;
    if (!ap->namarr.fun && aq) {
        return (ap->cur < aq->maxi) && aq->val[ap->cur].cp;
    }
    return false;
}

//
// Replace discipline with new one.
//
static_fn void array_setptr(Namval_t *np, struct index_array *old, struct index_array *new) {
    Namfun_t **fp = &np->nvfun;

    while (*fp && *fp != &old->namarr.hdr) fp = &((*fp)->next);
    if (*fp) {
        new->namarr.hdr.next = (*fp)->next;
        *fp = &new->namarr.hdr;
    } else {
        sfprintf(sfstderr, "discipline not replaced\n");
    }
}

//
// Calculate the amount of space to be allocated to hold an indexed array into
// which <maxi> is a legal index.  The number of elements that will actually
// fit into the array (> <maxi> but <= ARRAY_MAX) is returned.
//
static_fn int arsize(struct index_array *ap, int maxi) {
    if (ap && maxi < 2 * ap->maxi) maxi = 2 * ap->maxi;
    // cppcheck-suppress integerOverflowCond
    maxi = roundof(maxi, ARRAY_INCR);
    return maxi > ARRAY_MAX ? ARRAY_MAX : maxi;
}

static_fn struct index_array *array_grow(Namval_t *, struct index_array *, int);

// Return next index after the highest element in an array.
int array_maxindex(Namval_t *np) {
    struct index_array *ap = (struct index_array *)nv_arrayptr(np);
    int i = ap->maxi;
    if (is_associative(&ap->namarr)) return -1;
    while (--i >= 0 && ap->val[i].cp == 0) {
        ;  // empty loop
    }
    return i + 1;
}

// Check if array is empty
int array_isempty(Namval_t *np) { return array_maxindex(np) <= 0; }

static_fn union Value *array_getup(Namval_t *np, Namarr_t *arp, int update) {
    struct index_array *ap = (struct index_array *)arp;
    union Value *up;
    int nofree = 0;

    if (!arp) return &np->nvalue;
    if (is_associative(&ap->namarr)) {
        Namval_t *mp;
        mp = (Namval_t *)((*arp->fun)(np, NULL, NV_ACURRENT));
        if (mp) {
            nofree = nv_isattr(mp, NV_NOFREE);
            up = &(mp->nvalue);  // parens are to silence false positive from cppcheck
        } else {
            return (union Value *)((*arp->fun)(np, NULL, 0));
        }
    } else {
        if (ap->cur >= ap->maxi) {
            errormsg(SH_DICT, ERROR_exit(1), e_subscript, nv_name(np));
            __builtin_unreachable();
        }
        up = &(ap->val[ap->cur]);
        nofree = array_isbit(ap->bits, ap->cur, ARRAY_NOFREE);
    }
    if (update) {
        if (nofree) {
            nv_onattr(np, NV_NOFREE);
        } else {
            nv_offattr(np, NV_NOFREE);
        }
    }
    return up;
}

bool nv_arrayisset(Namval_t *np, Namarr_t *arp) {
    struct index_array *ap = (struct index_array *)arp;
    union Value *up;

    if (is_associative(&ap->namarr)) {
        np = nv_opensub(np);
        return np && !nv_isnull(np);
    }
    if (ap->cur >= ap->maxi) return false;
    up = &(ap->val[ap->cur]);
    if (up->cp == Empty) {
        Namfun_t *fp = &arp->hdr;
        for (fp = fp->next; fp; fp = fp->next) {
            if (fp->disc && (fp->disc->getnum || fp->disc->getval)) return true;
        }
    }
    return up->cp && up->cp != Empty;
}

//
// Get the Value pointer for an array.
// Delete space as necessary if flag is ARRAY_DELETE.
// After the lookup is done the last @ or * subscript is incremented.
//
static_fn Namval_t *array_find(Namval_t *np, Namarr_t *arp, int flag) {
    Shell_t *shp = sh_ptr(np);
    struct index_array *ap = (struct index_array *)arp;
    union Value *up;
    Namval_t *mp;
    int wasundef;

    if (flag & ARRAY_LOOKUP) {
        ap->namarr.flags &= ~ARRAY_NOSCOPE;
    } else {
        ap->namarr.flags |= ARRAY_NOSCOPE;
    }
    wasundef = ap->namarr.flags & ARRAY_UNDEF;
    if (wasundef) {
        ap->namarr.flags &= ~ARRAY_UNDEF;
        // Delete array is the same as delete array[@].
        if (flag & ARRAY_DELETE) {
            nv_putsub(np, NULL, 0, ARRAY_SCAN | ARRAY_NOSCOPE);
            ap->namarr.flags |= ARRAY_SCAN;
        } else {  // same as array[0]
            if (is_associative(&ap->namarr)) {
                (*ap->namarr.fun)(np, "0", flag == ARRAY_ASSIGN ? NV_AADD : 0);
            } else {
                ap->cur = 0;
            }
        }
    }
    if (nv_isattr(np, NV_NOTSET) == NV_NOTSET) nv_offattr(np, NV_BINARY);
    if (is_associative(&ap->namarr)) {
        mp = (Namval_t *)((*arp->fun)(np, NULL, NV_ACURRENT));
        if (!mp) {
            up = (union Value *)&mp;
        } else if (nv_isarray(mp)) {
            if (wasundef) nv_putsub(mp, NULL, 0, ARRAY_UNDEF);
            return mp;
        } else {
            up = &mp->nvalue;
            if (nv_isvtree(mp)) {
                if (!up->cp && flag == ARRAY_ASSIGN) {
                    nv_arraychild(np, mp, 0);
                    ap->namarr.nelem++;
                }
                return mp;
            } else if (!up->cp) {
                if (flag == ARRAY_ASSIGN) {
                    mp->nvsize &= ~1;
                } else if ((mp->nvsize & 1) && nv_isattr(np, NV_INTEGER)) {
                    nv_onattr(np, NV_BINARY);
                }
            }
        }
    } else {
        if (!(ap->namarr.flags & ARRAY_SCAN) && ap->cur >= ap->maxi) {
            ap = array_grow(np, ap, (int)ap->cur);
        }
        if (ap->cur >= ap->maxi) {
            errormsg(SH_DICT, ERROR_exit(1), e_subscript, nv_name(np));
            __builtin_unreachable();
        }
        up = &(ap->val[ap->cur]);
        if ((!up->cp || up->cp == Empty) && nv_type(np) && nv_isvtree(np)) {
            char *cp;
            if (!ap->namarr.table) {
                ap->namarr.table = dtopen(&_Nvdisc, Dtoset);
                dtuserdata(ap->namarr.table, shp, 1);
            }
            sfprintf(shp->strbuf, "%d", ap->cur);
            cp = sfstruse(shp->strbuf);
            mp = nv_search(cp, ap->namarr.table, NV_ADD);
            mp->nvenv = (char *)np;
            nv_arraychild(np, mp, 0);
        }
        if (up->np && array_isbit(ap->bits, ap->cur, ARRAY_CHILD)) {
            if (wasundef && nv_isarray(up->np)) nv_putsub(up->np, NULL, 0, ARRAY_UNDEF);
            return up->np;
        }
        if (flag & ARRAY_ASSIGN) {
            array_clrbit(ap->bits, ap->cur, ARRAY_UNSET);
        } else if (!up->cp && nv_isattr(np, NV_INTEGER) &&
                   array_isbit(ap->bits, ap->cur, ARRAY_UNSET)) {
            nv_onattr(np, NV_BINARY);
        }
    }
    np->nvalue.cp = up->cp;
    if (!up->cp && nv_isattr(np, NV_NOTSET) != NV_NOTSET) {
        char *xp = nv_setdisc(np, "get", np, (Namfun_t *)np);
        if (flag != ARRAY_ASSIGN) {
            struct index_array *aq = (arp->fun ? 0 : (struct index_array *)arp->scope);
            if (xp == (char *)np && aq && aq->maxi < ap->cur) return np;
            return xp && xp != (char *)np ? np : 0;
        }
        if (!array_covered(np, ap)) ap->namarr.nelem++;
    }
    return np;
}

bool nv_arraysettype(Namval_t *np, Namval_t *tp, const char *sub, int flags) {
    Shell_t *shp = sh_ptr(np);
    Namval_t *nq;
    int rdonly = nv_isattr(np, NV_RDONLY);
    int xtrace = sh_isoption(shp, SH_XTRACE);
    Namarr_t *ap = nv_arrayptr(np);

    shp->last_table = NULL;
    if (!tp->nvfun) return true;
    if (!ap->table) {
        ap->table = dtopen(&_Nvdisc, Dtoset);
        dtuserdata(ap->table, shp, 1);
    }
    nq = nv_search(sub, ap->table, NV_ADD);
    if (nq) {
        char *av[2] = {NULL, NULL};

        if (!nq->nvfun && nq->nvalue.cp && *nq->nvalue.cp == 0) _nv_unset(nq, NV_RDONLY);
        nv_arraychild(np, nq, 0);
        if (!nv_isattr(tp, NV_BINARY)) {
            sfprintf(shp->strbuf, "%s=%s", nv_name(nq), nv_getval(np));
            char *p = sfstruse(shp->strbuf);
            assert(p);
            av[0] = strdup(p);
        }
        if (!nv_clone(tp, nq, flags | NV_NOFREE)) {
            if (av[0]) free(av[0]);
            return false;
        }
        ap->flags |= ARRAY_SCAN;
        if (!rdonly) nv_offattr(nq, NV_RDONLY);
        if (!nv_isattr(tp, NV_BINARY)) {
            char *prefix = shp->prefix;
            if (xtrace) sh_offoption(shp, SH_XTRACE);
            ap->flags &= ~ARRAY_SCAN;
            shp->prefix = 0;
            sh_eval(shp, sh_sfeval(av), 0);
            shp->prefix = prefix;
            ap->flags |= ARRAY_SCAN;
            if (xtrace) sh_onoption(shp, SH_XTRACE);
        }
        if (av[0]) free(av[0]);
        return true;
    }
    return false;
}

static_fn Namfun_t *array_clone(Namval_t *np, Namval_t *mp, int flags, Namfun_t *fp) {
    Namarr_t *ap = (Namarr_t *)fp;
    Namval_t *nq, *mq;
    char *name, *sub = 0;
    int flg, skipped = 0;
    Dt_t *otable = ap->table;
    struct index_array *aq = (struct index_array *)ap, *ar;
    Shell_t *shp = sh_ptr(np);

    if (flags & NV_MOVE) {
        if ((flags & NV_COMVAR) && nv_putsub(np, NULL, 0, ARRAY_SCAN)) {
            do {
                nq = nv_opensub(np);
                if (nq) nq->nvenv = (void *)mp;
            } while (nv_nextsub(np));
        }
        return fp;
    }
    flg = ap->flags;
    if (flg & ARRAY_NOCLONE) return 0;
    if ((flags & NV_TYPE) && !ap->scope) {
        aq = array_scope(np, aq, flags);
        return &aq->namarr.hdr;
    }
    ap = (Namarr_t *)nv_clone_disc(fp, 0);
    if (flags & NV_COMVAR) {
        ap->scope = 0;
        ap->flags = 0;
        ap->nelem = 0;
        shp->prev_table = shp->last_table;
        shp->prev_root = shp->last_root;
    }
    if (ap->table) {
        ap->table = dtopen(&_Nvdisc, Dtoset);
        dtuserdata(ap->table, shp, 1);
        if (ap->scope && !(flags & NV_COMVAR)) {
            ap->scope = ap->table;
            dtview(ap->table, otable->view);
        }
    }
    mp->nvfun = (Namfun_t *)ap;
    mp->nvflag &= NV_MINIMAL;
    mp->nvflag |= (np->nvflag & ~(NV_MINIMAL | NV_NOFREE));
    if (!(flg & (ARRAY_SCAN | ARRAY_UNDEF)) && (sub = nv_getsub(np))) sub = strdup(sub);
    ar = (struct index_array *)ap;
    if (!is_associative(ap)) ar->bits = (unsigned char *)&ar->val[ar->maxi];
    if (!nv_putsub(np, NULL, 0, ARRAY_SCAN | ((flags & NV_COMVAR) ? 0 : ARRAY_NOSCOPE))) {
        if (ap->fun) (*ap->fun)(np, (char *)np, 0);
        skipped = 1;
        goto skip;
    }
    do {
        name = nv_getsub(np);
        nv_putsub(mp, name, 0, ARRAY_ADD | ARRAY_NOSCOPE);
        mq = 0;
        nq = nv_opensub(np);
        if (nq) {
            mq = nv_search(name, ap->table, NV_ADD);
            if (flags & NV_COMVAR) mq->nvenv = (char *)mp;
        }
        if (nq && (((flags & NV_COMVAR) && nv_isvtree(nq)) || nv_isarray(nq))) {
            mq->nvalue.cp = 0;
            if (!is_associative(ap)) ar->val[ar->cur].np = mq;
            nv_clone(nq, mq, flags);
        } else if (flags & NV_ARRAY) {
            if ((flags & NV_NOFREE) && !is_associative(ap)) {
                array_setbit(aq->bits, aq->cur, ARRAY_NOFREE);
            } else if (nq && (flags & NV_NOFREE)) {
                mq->nvalue = nq->nvalue;
                nv_onattr(nq, NV_NOFREE);
            }
        } else if (nv_isattr(np, NV_INTEGER)) {
            Sfdouble_t d = nv_getnum(np);
            if (!is_associative(ap)) ar->val[ar->cur].cp = 0;
            nv_putval(mp, (char *)&d, NV_LDOUBLE);
        } else {
            if (!is_associative(ap)) ar->val[ar->cur].cp = 0;
            nv_putval(mp, nv_getval(np), NV_RDONLY);
        }
        aq->namarr.flags |= ARRAY_NOSCOPE;
    } while (nv_nextsub(np));
skip:
    if (sub) {
        if (!skipped) nv_putsub(np, sub, 0L, 0);
        free(sub);
    }
    aq->namarr.flags = ap->flags = flg;
    ap->nelem = aq->namarr.nelem;
    return &ap->hdr;
}

static_fn char *array_getval(Namval_t *np, Namfun_t *disc) {
    Namarr_t *aq, *ap = (Namarr_t *)disc;
    Namval_t *mp;
    char *cp = NULL;

    mp = array_find(np, ap, ARRAY_LOOKUP);
    if (mp != np) {
        if (!mp && !is_associative(ap) && (aq = (Namarr_t *)ap->scope)) {
            array_syncsub(aq, ap);
            if ((mp = array_find(np, aq, ARRAY_LOOKUP)) == np) return nv_getv(np, &aq->hdr);
        }
        if (mp) {
            cp = nv_getval(mp);
            nv_offattr(mp, NV_EXPORT);
        }
        return cp;
    }
    return nv_getv(np, &ap->hdr);
}

static_fn Sfdouble_t array_getnum(Namval_t *np, Namfun_t *disc) {
    Namarr_t *aq, *ap = (Namarr_t *)disc;
    Namval_t *mp;

    mp = array_find(np, ap, ARRAY_LOOKUP);
    if (mp != np) {
        if (!mp && !is_associative(ap) && (aq = (Namarr_t *)ap->scope)) {
            array_syncsub(aq, ap);
            if ((mp = array_find(np, aq, ARRAY_LOOKUP)) == np) return nv_getn(np, &aq->hdr);
        }
        return mp ? nv_getnum(mp) : 0;
    }
    return nv_getn(np, &ap->hdr);
}

static_fn void array_putval(Namval_t *np, const void *string, int flags, Namfun_t *dp) {
    Namarr_t *ap = (Namarr_t *)dp;
    union Value *up;
    Namval_t *mp;
    struct index_array *aq = (struct index_array *)ap;
    int scan, nofree = nv_isattr(np, NV_NOFREE);

    do {
        int xfree =
            (ap->fixed || is_associative(ap)) ? 0 : array_isbit(aq->bits, aq->cur, ARRAY_NOFREE);
        mp = array_find(np, ap, string ? ARRAY_ASSIGN : ARRAY_DELETE);
        scan = ap->flags & ARRAY_SCAN;
        if (mp && mp != np) {
            if (!is_associative(ap) && string && !(flags & NV_APPEND) && !nv_type(np) &&
                nv_isvtree(mp) && !(ap->flags & ARRAY_TREE)) {
                if (!nv_isattr(np, NV_NOFREE)) _nv_unset(mp, flags & NV_RDONLY);
                array_clrbit(aq->bits, aq->cur, ARRAY_CHILD);
                aq->val[aq->cur].cp = 0;
                if (!nv_isattr(mp, NV_NOFREE)) nv_delete(mp, ap->table, 0);
                goto skip;
            }
            if (!xfree) nv_putval(mp, string, flags);
            if (string) {
                if (ap->hdr.type && ap->hdr.type != nv_type(mp)) {
                    nv_arraysettype(np, ap->hdr.type, nv_getsub(np), 0);
                }
                continue;
            }
            ap->flags |= scan;
        }
        if (!string) {
            if (mp) {
                if (is_associative(ap)) {
                    (*ap->fun)(np, NULL, NV_ADELETE);
                    np->nvalue.cp = 0;
                } else {
                    if (mp != np) {
                        array_clrbit(aq->bits, aq->cur, ARRAY_CHILD);
                        aq->val[aq->cur].cp = 0;
                        // TODO: NV_NOFREE was added here as a workaround for a use after free bug
                        // If it creates a big memory leak we should investigate another solution
                        // for it https://github.com/att/ast/issues/398
                        if (!xfree) nv_delete(mp, ap->table, NV_NOFREE);
                    }
                    if (!array_covered(np, (struct index_array *)ap) && array_elem(ap)) {
                        ap->nelem--;
                    }
                }
            }
            if (array_elem(ap) == 0 && (ap->flags & ARRAY_SCAN)) {
                if (is_associative(ap)) {
                    (*ap->fun)(np, NULL, NV_AFREE);
                } else if (ap->table) {
                    dtclose(ap->table);
                    ap->table = 0;
                }
                nv_offattr(np, NV_ARRAY);
            }
            if (!mp || mp != np || is_associative(ap)) continue;
        }
    skip:
        // Prevent empty string from being deleted.
        up = array_getup(np, ap, !nofree);
        if (up->cp == Empty && nv_isattr(np, NV_INTEGER)) {
            nv_offattr(np, NV_BINARY);
            up->cp = 0;
        }
        if (nv_isarray(np)) np->nvalue.up = up;
        nv_putv(np, string, flags, &ap->hdr);
        if (nofree && !up->cp) up->cp = Empty;
        if (!is_associative(ap)) {
            if (string) {
                array_clrbit(aq->bits, aq->cur, ARRAY_NOFREE);
            } else if (mp == np) {
                aq->val[aq->cur].cp = 0;
            }
        }
        if (string && ap->hdr.type && nv_isvtree(np)) {
            nv_arraysettype(np, ap->hdr.type, nv_getsub(np), 0);
        }
    } while (!string && nv_nextsub(np));
    if (ap) ap->flags &= ~ARRAY_NOSCOPE;
    if (nofree) {
        nv_onattr(np, NV_NOFREE);
    } else {
        nv_offattr(np, NV_NOFREE);
    }
    if (!string && !nv_isattr(np, NV_ARRAY)) {
        Namfun_t *nfp;
        if (!is_associative(ap) && aq->xp) {
            _nv_unset(nv_namptr(aq->xp, 0), NV_RDONLY);
            free(aq->xp);
        }
        if ((nfp = nv_disc(np, (Namfun_t *)ap, NV_POP)) && !(nfp->nofree & 1)) {
            ap = 0;
            free(nfp);
        }
        if (!nv_isnull(np)) {
            if (!np->nvfun) nv_onattr(np, NV_NOFREE);
            _nv_unset(np, flags);
        } else {
            nv_offattr(np, NV_NOFREE);
        }
        if (np->nvalue.cp == Empty) np->nvalue.cp = 0;
    }
    if (!string && (flags & NV_TYPE) && ap) array_unscope(np, ap);
}

static const Namdisc_t array_disc = {.dsize = sizeof(Namarr_t),
                                     .putval = array_putval,
                                     .getval = array_getval,
                                     .getnum = array_getnum,
                                     .clonef = array_clone};

static_fn void array_copytree(Namval_t *np, Namval_t *mp) {
    Namfun_t *fp = nv_disc(np, NULL, NV_POP);
    nv_offattr(np, NV_ARRAY);
    nv_clone(np, mp, 0);
    if (np->nvalue.cp && !nv_isattr(np, NV_NOFREE)) free(np->nvalue.sp);
    np->nvalue.cp = 0;
    np->nvalue.up = &mp->nvalue;
    fp->nofree &= ~1;
    nv_disc(np, (Namfun_t *)fp, NV_FIRST);
    fp->nofree |= 1;
    nv_onattr(np, NV_ARRAY);
    mp->nvenv = (char *)np;
}

//
// Increase the size of the indexed array of elements in <arp> so that <maxi>
// is a legal index.  If <arp> is 0, an array of the required size is
// allocated.  A pointer to the allocated Namarr_t structure is returned.
// <maxi> becomes the current index of the array.
//
static_fn struct index_array *array_grow(Namval_t *np, struct index_array *arp, int maxi) {
    struct index_array *ap;
    int i;
    // cppcheck-suppress integerOverflowCond
    int newsize = arsize(arp, maxi + 1);
    size_t size;

    if (maxi >= ARRAY_MAX) {
        errormsg(SH_DICT, ERROR_exit(1), e_subscript, fmtbase((long)maxi, 10, 0));
        __builtin_unreachable();
    }
    size = (newsize - 1) * sizeof(union Value *) + newsize;
    ap = calloc(1, sizeof(struct index_array) + size);
    memset((void *)ap, 0, sizeof(*ap) + size);
    ap->maxi = newsize;
    ap->cur = maxi;
    ap->bits = (unsigned char *)&ap->val[newsize];
    memset(ap->bits, ARRAY_UNSET, newsize);
    if (arp) {
        ap->namarr = arp->namarr;
        ap->namarr.hdr.dsize = sizeof(*ap) + size;
        ap->last = arp->last;
        for (i = 0; i < arp->maxi; i++) {
            ap->bits[i] = arp->bits[i];
            ap->val[i].cp = arp->val[i].cp;
        }
        memcpy(ap->bits, arp->bits, arp->maxi);
        array_setptr(np, arp, ap);
        free(arp);
    } else {
        int flags = 0;
        Namval_t *mp = 0;
        ap->namarr.hdr.dsize = sizeof(*ap) + size;
        i = 0;
        ap->namarr.fun = 0;
        if ((nv_isnull(np) || np->nvalue.cp == Empty) && nv_isattr(np, NV_NOFREE)) {
            flags = ARRAY_TREE;
            nv_offattr(np, NV_NOFREE);
        }
        if (np->nvalue.cp == Empty) np->nvalue.cp = 0;
        if (nv_hasdisc(np, &array_disc) || (nv_type(np) && nv_isvtree(np))) {
            Shell_t *shp = sh_ptr(np);
            ap->namarr.table = dtopen(&_Nvdisc, Dtoset);
            dtuserdata(ap->namarr.table, shp, 1);
            mp = nv_search("0", ap->namarr.table, NV_ADD);
            if (mp && nv_isnull(mp)) {
                Namfun_t *fp;
                ap->val[0].np = mp;
                array_setbit(ap->bits, 0, ARRAY_CHILD);
                for (fp = np->nvfun; fp && !fp->disc->readf; fp = fp->next) {
                    ;  // empty loop
                }
                if (fp && fp->disc && fp->disc->readf) (*fp->disc->readf)(mp, NULL, 0, fp);
                i++;
            }
        } else if ((ap->val[0].cp = np->nvalue.cp)) {
            i++;
        } else if (nv_isattr(np, NV_INTEGER) && !nv_isnull(np)) {
            // TODO: Figure out why this statement is present since `d` is unused:
            // Sfdouble_t d = nv_getnum(np);
            i++;
        }
        ap->last = i;
        ap->namarr.nelem = i;
        ap->namarr.flags = flags;
        ap->namarr.hdr.disc = &array_disc;
        nv_disc(np, (Namfun_t *)ap, NV_FIRST);
        nv_onattr(np, NV_ARRAY);
        if (mp) {
            array_copytree(np, mp);
            ap->namarr.hdr.nofree &= ~1;
        }
    }
    for (; i < newsize; i++) ap->val[i].cp = 0;
    return ap;
}

bool nv_atypeindex(Namval_t *np, const char *tname) {
    Shell_t *shp = sh_ptr(np);
    Namval_t *tp;
    int offset = stktell(shp->stk);
    size_t n = strlen(tname) - 1;

    sfprintf(stkstd, "%s.%.*s%c", NV_CLASS, n, tname, 0);
    tp = nv_open(stkptr(shp->stk, offset), shp->var_tree, NV_NOADD | NV_VARNAME);
    stkseek(shp->stk, offset);
    if (tp) {
        struct index_array *ap = (struct index_array *)nv_arrayptr(np);
        if (!nv_hasdisc(tp, &ENUM_disc)) {
            errormsg(SH_DICT, ERROR_exit(1), e_notenum, tp->nvname);
            __builtin_unreachable();
        }
        if (!ap) ap = array_grow(np, ap, 1);
        ap->xp = calloc(NV_MINSZ, 1);
        np = nv_namptr(ap->xp, 0);
        np->nvname = tp->nvname;
        nv_onattr(np, NV_MINIMAL);
        nv_clone(tp, np, NV_NOFREE);
        nv_offattr(np, NV_RDONLY);
        return true;
    }

    errormsg(SH_DICT, ERROR_exit(1), e_unknowntype, n, tname);
    __builtin_unreachable();
}

Namarr_t *nv_arrayptr(Namval_t *np) {
    assert(np);
    if (nv_isattr(np, NV_ARRAY)) return (Namarr_t *)nv_hasdisc(np, &array_disc);
    return NULL;
}

//
// Verify that argument is an indexed array and convert to associative, freeing
// relevant storage.
//
static_fn Namarr_t *nv_changearray(Namval_t *np, void *(*fun)(Namval_t *, const char *, int)) {
    Namarr_t *ap;
    char numbuff[NUMSIZE + 1];
    unsigned dot, digit, n;
    union Value *up;
    struct index_array *save_ap;
    char *string_index = &numbuff[NUMSIZE];
    numbuff[NUMSIZE] = '\0';

    if (!fun || !(ap = nv_arrayptr(np)) || is_associative(ap)) return NULL;

    nv_stack(np, &ap->hdr);
    save_ap = (struct index_array *)nv_stack(np, 0);
    ap = (Namarr_t *)((*fun)(np, NULL, NV_AINIT));
    ap->nelem = 0;
    ap->flags = 0;
    ap->fun = fun;
    nv_onattr(np, NV_ARRAY);

    for (dot = 0; dot < (unsigned)save_ap->maxi; dot++) {
        if (save_ap->val[dot].cp) {
            if ((digit = dot) == 0) {
                *--string_index = '0';
            } else {
                while ((n = digit)) {
                    digit /= 10;
                    *--string_index = '0' + (n - 10 * digit);
                }
            }
            nv_putsub(np, string_index, 0, ARRAY_ADD);
            up = (union Value *)((*ap->fun)(np, NULL, 0));
            up->cp = save_ap->val[dot].cp;
            save_ap->val[dot].cp = 0;
        }
        string_index = &numbuff[NUMSIZE];
    }
    free(save_ap);
    return ap;
}

//
// Set the associative array processing method for node <np> to <fun>.
// The array pointer is returned if sucessful.
//
Namarr_t *nv_setarray(Namval_t *np, void *(*fun)(Namval_t *, const char *, int)) {
    Namarr_t *ap;
    int flags = 0;

    if (fun) {
        ap = nv_arrayptr(np);
        if (ap) {
            // If it's already an indexed array, convert to associative structure.
            if (!is_associative(ap)) ap = nv_changearray(np, fun);
            return ap;
        }
    }

    if (nv_isnull(np) && nv_isattr(np, NV_NOFREE)) {
        flags = ARRAY_TREE;
        nv_offattr(np, NV_NOFREE);
    }

    if (!fun) return NULL;

    // Must be done before calling *fun() because it is likely to mutate the namval.
    Namfun_t *fp = nv_isvtree(np);
    char *value = fp ? NULL : nv_getval(np);

    ap = (Namarr_t *)((*fun)(np, NULL, NV_AINIT));
    if (!ap) return NULL;

    // Check for preexisting initialization and save.
    ap->flags = flags;
    ap->fun = fun;
    nv_onattr(np, NV_ARRAY);
    if (fp || (value && value != Empty)) {
        nv_putsub(np, "0", 0, ARRAY_ADD);
        if (value) {
            nv_putval(np, value, 0);
        } else {
            Namval_t *mp = (Namval_t *)((*fun)(np, NULL, NV_ACURRENT));
            array_copytree(np, mp);
        }
    }
    return ap;
}

//
// Move parent subscript into child.
//
Namval_t *nv_arraychild(Namval_t *np, Namval_t *nq, int c) {
    Namfun_t *fp;
    Namarr_t *ap = nv_arrayptr(np);
    union Value *up;
    Namval_t *tp;

    if (!nq) return ap ? array_find(np, ap, ARRAY_LOOKUP) : 0;
    if (!ap) {
        nv_putsub(np, NULL, 0, ARRAY_FILL);
        ap = nv_arrayptr(np);
    }
    if (!(up = array_getup(np, ap, 0))) return NULL;
    np->nvalue.cp = up->cp;
    if ((tp = nv_type(np)) || c) {
        ap->flags |= ARRAY_NOCLONE;
        nq->nvenv = (char *)np;
        if (c == 't') {
            nv_clone(tp, nq, 0);
        } else {
            nv_clone(np, nq, NV_NODISC);
        }
        nv_offattr(nq, NV_ARRAY);
        ap->flags &= ~ARRAY_NOCLONE;
    }
    nq->nvenv = (char *)np;
    nq->nvshell = np->nvshell;
    if ((fp = nq->nvfun) && fp->disc && fp->disc->setdisc && (fp = nv_disc(nq, fp, NV_POP))) {
        free(fp);
    }
    if (!ap->fun) {
        struct index_array *aq = (struct index_array *)ap;
        array_setbit(aq->bits, aq->cur, ARRAY_CHILD);
        array_clrbit(aq->bits, aq->cur, ARRAY_UNSET);
        if (c == '.' && !nq->nvalue.cp) ap->nelem++;
        up->np = nq;
    }
    if (c == '.') nv_setvtree(nq);
    return nq;
}

//
// This routine sets subscript of <np> to the next element, if any.
// The return value is zero, if there are no more elements.
// Otherwise, 1 is returned.
//
bool nv_nextsub(Namval_t *np) {
    struct index_array *ap = (struct index_array *)nv_arrayptr(np);
    unsigned dot;
    struct index_array *aq = 0, *ar = 0;

    if (!ap || !(ap->namarr.flags & ARRAY_SCAN)) return false;
    if (is_associative(&ap->namarr)) {
        if ((*ap->namarr.fun)(np, NULL, NV_ANEXT)) return true;
        ap->namarr.flags &= ~(ARRAY_SCAN | ARRAY_NOCHILD);
        return false;
    }
    if (!(ap->namarr.flags & ARRAY_NOSCOPE)) ar = (struct index_array *)ap->namarr.scope;
    for (dot = ap->cur + 1; dot < (unsigned)ap->maxi; dot++) {
        aq = ap;
        if (!ap->val[dot].cp && !(ap->namarr.flags & ARRAY_NOSCOPE)) {
            if (!(aq = ar) || dot >= (unsigned)aq->maxi) continue;
        }
        if (aq->val[dot].cp == Empty && array_elem(&aq->namarr) < nv_aimax(np) + 1) {
            ap->cur = dot;
            if (nv_getval(np) == Empty) continue;
        }
        if (aq->val[dot].cp) {
            ap->cur = dot;
            if (array_isbit(aq->bits, dot, ARRAY_CHILD)) {
                Namval_t *mp = aq->val[dot].np;
                if ((aq->namarr.flags & ARRAY_NOCHILD) && nv_isvtree(mp) && !mp->nvfun->dsize) {
                    continue;
                }
                if (nv_isarray(mp)) {
                    aq = (struct index_array *)nv_arrayptr(mp);
                    if (aq && aq->namarr.nelem) nv_putsub(mp, NULL, 0, ARRAY_SCAN);
                }
            }
            return true;
        }
    }
    ap->namarr.flags &= ~(ARRAY_SCAN | ARRAY_NOCHILD);
    ap->cur = 0;
    return false;
}

//
// Set an array subscript for node <np> given the subscript <sp>.
// An array is created if necessary.
// <mode> can be a number, plus or more of symbolic constants
//    ARRAY_SCAN, ARRAY_UNDEF, ARRAY_ADD
// The node pointer is returned which can be NULL if <np> is
//    not already array and the ARRAY_ADD bit of <mode> is not set.
// ARRAY_FILL sets the specified subscript to the empty string when
//   ARRAY_ADD is specified and there is no value or sets all
// the elements up to the number specified if ARRAY_ADD is not specified.
//
Namval_t *nv_putsub(Namval_t *np, char *sp, long size, int flags) {
    Shell_t *shp = sh_ptr(np);
    struct index_array *ap = (struct index_array *)nv_arrayptr(np);
    if (!ap || !ap->namarr.fun) {
        if (sp && sp != Empty) {
            if (ap && ap->xp && !strmatch(sp, "+([0-9])")) {
                Namval_t *mp = nv_namptr(ap->xp, 0);
                nv_putval(mp, sp, 0);
                size = nv_getnum(mp);
            } else {
                Dt_t *root = shp->last_root;
                size = (int)sh_arith(shp, (char *)sp);
                shp->last_root = root;
            }
        }
        if (size < 0 && ap) size += array_maxindex(np);
        if (size >= ARRAY_MAX || (size < 0)) {
            errormsg(SH_DICT, ERROR_exit(1), e_subscript, nv_name(np));
            __builtin_unreachable();
        }
        if (!ap || size >= ap->maxi) {
            if (size == 0 && !(flags & ARRAY_FILL)) return NULL;
            if (shp->subshell) np = sh_assignok(np, 1);
            ap = array_grow(np, ap, size);
        }
        ap->namarr.flags &= ~ARRAY_UNDEF;
        ap->namarr.flags |= (flags & (ARRAY_SCAN | ARRAY_NOCHILD | ARRAY_UNDEF | ARRAY_NOSCOPE));
#if 0
		if(array_isbit(ap->bits,oldsize,ARRAY_CHILD))
			mp = ap->val[oldsize].np;
		if(size != oldsize && mp->nvalue.cp)
		{
			Namfun_t *nfp;
			for(nfp=np->nvfun; nfp; nfp=nfp->next)
			{
				if(nfp->disc && nfp->disc->readf)
				{
					(*nfp->disc->readf)(mp,(Sfio_t*)0,0,nfp);
					break;
				}
			}
		}
#endif
        ap->cur = size;
        if ((flags & ARRAY_SCAN) && (ap->cur--, !nv_nextsub(np))) np = 0;
        if (flags & (ARRAY_FILL | ARRAY_ADD)) {
            if (!(flags & ARRAY_ADD)) {
                int n;
                if (flags & ARRAY_SETSUB) {
                    for (n = 0; n <= ap->maxi; n++) ap->val[n].cp = 0;
                    ap->namarr.nelem = 0;
                    ap->namarr.flags = 0;
                }
                for (n = 0; n <= size; n++) {
                    if (!ap->val[n].cp) {
                        array_clrbit(ap->bits, n, ARRAY_UNSET);
                        ap->val[n].cp = Empty;
                        if (!array_covered(np, ap)) ap->namarr.nelem++;
                    }
                    ap->last = ap->namarr.nelem;
                }
                n = ap->maxi - ap->maxi;
                if (n) memset(&ap->val[size], 0, n * sizeof(union Value));
            } else if (!(sp = (char *)ap->val[size].cp) || sp == Empty) {
                if (shp->subshell) np = sh_assignok(np, 1);
                if (ap->namarr.flags & ARRAY_TREE) {
                    char *cp;
                    Namval_t *mp;
                    if (!ap->namarr.table) {
                        ap->namarr.table = dtopen(&_Nvdisc, Dtoset);
                        dtuserdata(ap->namarr.table, shp, 1);
                    }
                    sfprintf(shp->strbuf, "%d", ap->cur);
                    cp = sfstruse(shp->strbuf);
                    mp = nv_search(cp, ap->namarr.table, NV_ADD);
                    mp->nvenv = (char *)np;
                    nv_arraychild(np, mp, 0);
                    nv_setvtree(mp);
                } else {
                    ap->val[size].cp = Empty;
                }
                if (!sp && !array_covered(np, ap)) ap->namarr.nelem++;
                array_clrbit(ap->bits, size, ARRAY_UNSET);
            }
        } else if (!(flags & ARRAY_SCAN)) {
            ap->namarr.flags &= ~ARRAY_SCAN;
            if (array_isbit(ap->bits, size, ARRAY_CHILD) && ap->val[size].np) {
                nv_putsub(ap->val[size].np, NULL, 0, ARRAY_UNDEF);
            }
            if (sp && !(flags & ARRAY_ADD) && !ap->val[size].cp) np = 0;
        }
        return (Namval_t *)np;
    }
    ap->namarr.flags &= ~ARRAY_UNDEF;
    if (!(flags & ARRAY_FILL)) ap->namarr.flags &= ~ARRAY_SCAN;
    ap->namarr.flags |= (flags & (ARRAY_SCAN | ARRAY_NOCHILD | ARRAY_UNDEF | ARRAY_NOSCOPE));
    if (sp) {
        if (flags & ARRAY_SETSUB) {
            (*ap->namarr.fun)(np, sp, NV_ASETSUB);
            return np;
        }
        (*ap->namarr.fun)(np, sp, (flags & ARRAY_ADD) ? NV_AADD : 0);
        if (!(flags & (ARRAY_SCAN | ARRAY_ADD)) && !(*ap->namarr.fun)(np, NULL, NV_ACURRENT))
            np = 0;
    } else if (flags & ARRAY_SCAN) {
        (*ap->namarr.fun)(np, (char *)np, 0);
    } else if (flags & ARRAY_UNDEF) {
        (*ap->namarr.fun)(np, "", 0);
    }
    if ((flags & ARRAY_SCAN) && !nv_nextsub(np)) np = 0;
    return np;
}

//
// Process an array subscript for node <np> given the subscript <cp>.
// Returns pointer to character after the subscript.
//
char *nv_endsubscript(Namval_t *np, char *cp, int mode, void *context) {
    int count = 1, quoted = 0, c;
    char *sp = cp + 1;
    Shell_t *shp = (Shell_t *)context;

    // First find matching ']'.
    while (count > 0 && (c = *++cp)) {
        if (c == '\\' && (!(mode & NV_SUBQUOTE) || (c = cp[1]) == '[' || c == ']' || c == '\\' ||
                          c == '*' || c == '@')) {
            quoted = 1;
            cp++;
        } else if (c == '[') {
            count++;
        } else if (c == ']') {
            count--;
        }
    }
    *cp = 0;
    if (quoted) {  // strip escape characters
        count = stktell(shp->stk);
        sfwrite(shp->stk, sp, 1 + cp - sp);
        sh_trim(sp = stkptr(shp->stk, count));
    }
    if (mode && np) {
        Namarr_t *ap = nv_arrayptr(np);
        int scan = 0;
        if (ap) scan = ap->flags & ARRAY_SCAN;
        if ((mode & NV_ASSIGN) && (cp[1] == '=' || cp[1] == '+')) {
            mode |= NV_ADD;
        } else if (ap && cp[1] == '.' && (mode & NV_FARRAY)) {
            mode |= NV_ADD;
        }
        nv_putsub(np, sp, 0,
                  ((mode & NV_ADD) ? ARRAY_ADD : 0) |
                      (cp[1] && (mode & NV_ADD) ? ARRAY_FILL : mode & ARRAY_FILL));
        if (scan) ap->flags |= scan;
    }
    if (quoted) stkseek(shp->stk, count);
    *cp++ = c;
    return cp;
}

Namval_t *nv_opensub(Namval_t *np) {
    struct index_array *ap = (struct index_array *)nv_arrayptr(np);

    if (!ap) return NULL;
    if (is_associative(&ap->namarr)) {
        return (Namval_t *)((*ap->namarr.fun)(np, NULL, NV_ACURRENT));
    } else if (array_isbit(ap->bits, ap->cur, ARRAY_CHILD)) {
        return ap->val[ap->cur].np;
    }
    return NULL;
}

char *nv_getsub(Namval_t *np) {
    static char numbuff[NUMSIZE + 1];
    struct index_array *ap;
    unsigned dot, n;
    char *cp = &numbuff[NUMSIZE];

    if (!np || !(ap = (struct index_array *)nv_arrayptr(np))) return NULL;
    if (is_associative(&ap->namarr)) return (char *)((*ap->namarr.fun)(np, NULL, NV_ANAME));
    if (ap->xp) {
        np = nv_namptr(ap->xp, 0);
        np->nvalue.i16 = ap->cur;
        return nv_getval(np);
    }
    if ((dot = ap->cur) == 0) {
        *--cp = '0';
    } else {
        while ((n = dot)) {
            dot /= 10;
            *--cp = '0' + (n - 10 * dot);
        }
    }

    return cp;
}

//
// If <np> is an indexed array node, the current subscript index returned,
// otherwise returns -1.
//
int nv_aindex(Namval_t *np) {
    Namarr_t *ap = nv_arrayptr(np);

    if (!ap) {
        return 0;
    } else if (is_associative(ap)) {
        return -1;
    }
    return ((struct index_array *)(ap))->cur;
}

int nv_arraynsub(Namarr_t *ap) { return array_elem(ap); }

union Value *nv_aivec(Namval_t *np, unsigned char **bitp) {
    struct index_array *ap = (struct index_array *)nv_arrayptr(np);
    if (!ap || ap->namarr.fun || ap->namarr.fixed) return NULL;
    if (bitp) *bitp = ap->bits;
    return ap->val;
}

int nv_aipack(Namarr_t *arp) {
    struct index_array *ap = (struct index_array *)arp;
    int i, j;

    if (!ap || ap->namarr.fun || ap->namarr.fixed) return -1;
    for (i = j = 0; i < ap->maxi; i++) {
        if (ap->val[i].np) {
            ap->bits[j] = ap->bits[i];
            ap->val[j++] = ap->val[i];
        }
    }
    for (i = j; i < ap->maxi; i++) {
        ap->val[i].np = 0;
        ap->bits[i] = 0;
    }
    return ap->namarr.nelem = j;
}

int nv_aimax(Namval_t *np) {
    struct index_array *ap = (struct index_array *)nv_arrayptr(np);
    int sub = -1;

    if (!ap || is_associative(&ap->namarr)) {
        return -1;
    }
    sub = ap->maxi;
    while (--sub > 0 && ap->val[sub].cp == 0) {
        ;  // empty loop
    }
    return sub;
}

//
// This is the default implementation for associative arrays.
//
void *nv_associative(Namval_t *np, const char *sp, int mode) {
    Shell_t *shp = sh_ptr(np);
    struct assoc_array *ap = (struct assoc_array *)nv_arrayptr(np);
    int type;

    switch (mode) {
        case NV_AINIT: {
            ap = (struct assoc_array *)calloc(1, sizeof(struct assoc_array));
            if (ap) {
                ap->namarr.table = dtopen(&_Nvdisc, Dtoset);
                dtuserdata(ap->namarr.table, shp, 1);
                ap->cur = 0;
                ap->pos = 0;
                ap->namarr.hdr.disc = &array_disc;
                nv_disc(np, (Namfun_t *)ap, NV_FIRST);
                ap->namarr.hdr.dsize = sizeof(struct assoc_array);
                ap->namarr.hdr.nofree &= ~1;
            }
            return (void *)ap;
        }
        case NV_ADELETE: {
            if (ap->cur) {
                Dt_t *scope = ap->namarr.scope;
                if (!scope || scope == ap->namarr.table || !nv_search(ap->cur->nvname, scope, 0)) {
                    ap->namarr.nelem--;
                }
                _nv_unset(ap->cur, NV_RDONLY);
                nv_delete(ap->cur, ap->namarr.table, 0);
                ap->cur = 0;
            }
            return (void *)ap;
        }
        case NV_AFREE: {
            ap->pos = 0;
            if (ap->namarr.scope) {
                ap->namarr.table = dtview(ap->namarr.table, NULL);
                dtclose(ap->namarr.scope);
                ap->namarr.scope = 0;
            } else {
                if (ap->namarr.nelem == 0 && (ap->cur = nv_search("0", ap->namarr.table, 0))) {
                    nv_associative(np, NULL, NV_ADELETE);
                }
                dtclose(ap->namarr.table);
                ap->namarr.table = 0;
            }
            return (void *)ap;
        }
        case NV_ANEXT: {
            if (!ap->pos) {
                if ((ap->namarr.flags & ARRAY_NOSCOPE) && ap->namarr.scope &&
                    dtvnext(ap->namarr.table)) {
                    ap->namarr.scope = dtvnext(ap->namarr.table);
                    ap->namarr.table->view = 0;
                }
                if (!(ap->pos = ap->cur)) ap->pos = (Namval_t *)dtfirst(ap->namarr.table);
            } else {
                ap->pos = ap->nextpos;
            }
            for (; (ap->cur = ap->pos); ap->pos = ap->nextpos) {
                ap->nextpos = (Namval_t *)dtnext(ap->namarr.table, ap->pos);
                if (!nv_isnull(ap->cur)) {
                    if ((ap->namarr.flags & ARRAY_NOCHILD) && nv_isattr(ap->cur, NV_CHILD)) {
                        continue;
                    }
                    return (void *)ap;
                }
            }
            if ((ap->namarr.flags & ARRAY_NOSCOPE) && ap->namarr.scope &&
                !dtvnext(ap->namarr.table)) {
                ap->namarr.table->view = (Dt_t *)ap->namarr.scope;
                ap->namarr.scope = ap->namarr.table;
            }
            return NULL;
        }
        case NV_ASETSUB: {
            ap->cur = (Namval_t *)sp;
            return (void *)ap->cur;
        }
        case NV_ACURRENT: {
            if (ap->cur) ap->cur->nvenv = (char *)np;
            return (void *)ap->cur;
        }
        case NV_ANAME: {
            if (ap->cur) {
                if (!shp->instance && nv_isnull(ap->cur)) return NULL;
                return (void *)ap->cur->nvname;
            }
            return NULL;
        }
        default: {
            if (sp) {
                Namval_t *mp = 0;
                ap->cur = 0;
                if (sp == (char *)np) return 0;
                type = nv_isattr(np, NV_PUBLIC & ~(NV_ARRAY | NV_CHILD | NV_MINIMAL));
                if (mode) {
                    mode = NV_ADD | HASH_NOSCOPE;
                } else if (ap->namarr.flags & ARRAY_NOSCOPE) {
                    mode = HASH_NOSCOPE;
                }
                if (*sp == 0 && sh_isoption(shp, SH_XTRACE) && (mode & NV_ADD)) {
                    errormsg(SH_DICT, ERROR_warn(0), "adding empty subscript");
                }
                if (shp->subshell && (mp = nv_search(sp, ap->namarr.table, 0)) && nv_isnull(mp)) {
                    ap->cur = mp;
                }
                if ((mp || (mp = nv_search(sp, ap->namarr.table, mode))) && nv_isnull(mp) &&
                    (mode & NV_ADD)) {
                    mp->nvshell = np->nvshell;
                    nv_onattr(mp, type);
                    mp->nvenv = (char *)np;
                    if ((mode & NV_ADD) && nv_type(np)) nv_arraychild(np, mp, 0);
                    if (shp->subshell) sh_assignok(np, 1);
                    if (type & NV_INTEGER) {
                        nv_onattr(mp, NV_NOTSET);
                    } else if (!ap->namarr.scope || !nv_search(sp, dtvnext(ap->namarr.table), 0)) {
                        ap->namarr.nelem++;
                    }
                    if (nv_isnull(mp)) {
                        if (ap->namarr.flags & ARRAY_TREE) nv_setvtree(mp);
                        mp->nvalue.cp = Empty;
                    }
                } else if (ap->namarr.flags & ARRAY_SCAN) {
                    Namval_t fake;
                    memset(&fake, 0, sizeof(fake));
                    fake.nvname = (char *)sp;
                    ap->pos = mp = (Namval_t *)dtprev(ap->namarr.table, &fake);
                    ap->nextpos = (Namval_t *)dtnext(ap->namarr.table, mp);
                } else if (!mp && *sp && mode == 0) {
                    mp = nv_search(sp, ap->namarr.table, NV_ADD | HASH_NOSCOPE);
                    if (!mp->nvalue.cp) mp->nvsize |= 1;
                }
                np = mp;
                if (ap->pos && ap->pos == np) {
                    ap->namarr.flags |= ARRAY_SCAN;
                } else if (!(ap->namarr.flags & ARRAY_SCAN)) {
                    ap->pos = 0;
                }
                ap->cur = np;
            }
            if (ap->cur) return &ap->cur->nvalue;
            return &ap->cur;
        }
    }
}

//
// Assign values to an array.
//
void nv_setvec(Namval_t *np, int append, int argc, char *argv[]) {
    int arg0 = 0;
    struct index_array *ap = NULL;
    // struct index_array *aq;  // see TODO below

    if (nv_isarray(np)) {
        ap = (struct index_array *)nv_arrayptr(np);
        if (ap && is_associative(&ap->namarr)) {
            errormsg(SH_DICT, ERROR_exit(1), "cannot append index array to associative array %s",
                     nv_name(np));
            __builtin_unreachable();
        }
    }
    if (append) {
        if (ap && ap->namarr.nelem == 0) {
            arg0 = 0;
        } else if (ap) {
            // TODO: Figure out if this is still needed. This statement is leftover from ksh93u+.
            // But in the subsequent changes it has become a no-op. Should it just be removed or
            // should it be modified to do something useful in light of the changes after 93u?
            // if (!(aq = (struct index_array *)ap->namarr.scope)) aq = ap;
            if (ap->namarr.nelem > ap->last) ap->last = array_maxindex(np);
            arg0 = ap->last;
        } else {
            nv_offattr(np, NV_ARRAY);
            if (!nv_isnull(np) && np->nvalue.cp != Empty) arg0 = 1;
        }
    }
    if (ap) ap->last = arg0 + argc;
    while (--argc >= 0) {
        nv_putsub(np, NULL, (long)argc + arg0, ARRAY_FILL | ARRAY_ADD);
        nv_putval(np, argv[argc], 0);
    }
    if (!ap && (ap = (struct index_array *)nv_arrayptr(np))) ap->last = array_maxindex(np);
}
