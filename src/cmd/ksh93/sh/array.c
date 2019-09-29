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

#include "ast.h"
#include "ast_assert.h"
#include "cdt.h"
#include "defs.h"
#include "error.h"
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

// Constants for the `nv_associative()` "op" parameter.
const Nvassoc_op_t ASSOC_OP_INIT = {ASSOC_OP_INIT_val};
const Nvassoc_op_t ASSOC_OP_FREE = {ASSOC_OP_FREE_val};
const Nvassoc_op_t ASSOC_OP_NEXT = {ASSOC_OP_NEXT_val};
const Nvassoc_op_t ASSOC_OP_NAME = {ASSOC_OP_NAME_val};
const Nvassoc_op_t ASSOC_OP_DELETE = {ASSOC_OP_DELETE_val};
const Nvassoc_op_t ASSOC_OP_ADD = {ASSOC_OP_ADD_val};
const Nvassoc_op_t ASSOC_OP_ADD2 = {ASSOC_OP_ADD2_val};
const Nvassoc_op_t ASSOC_OP_CURRENT = {ASSOC_OP_CURRENT_val};
const Nvassoc_op_t ASSOC_OP_SETSUB = {ASSOC_OP_SETSUB_val};

struct index_array {
    Namarr_t namarr;
    void *xp;             // if set, subscripts will be converted
    int cur;              // index of current element
    int last;             // index of highest assigned element
    int maxi;             // maximum index for array
    unsigned char *bits;  // bit array for child subscripts
    struct Value val[1];  // array of value holders
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
    size_t size = aq->namarr.namfun.dsize;

    if (size == 0) size = aq->namarr.namfun.disc->dsize;
    ar = malloc(size);
    memcpy(ar, aq, size);
    if (flags & NV_RDONLY) {
        ar->namarr.namfun.nofree |= 1;
    } else {
        ar->namarr.namfun.nofree &= ~1;
    }
    if (is_associative(&ar->namarr)) {
        ar->namarr.scope = dtopen(&_Nvdisc, Dtoset);
        dtuserdata(ar->namarr.scope, shp, 1);
        dtview(ar->namarr.scope, ar->namarr.table);
        ar->namarr.table = ar->namarr.scope;
        return ar;
    }
    ar->namarr.scope = (Dt_t *)aq;
    memset(ar->val, 0, ar->maxi * sizeof(char *));
    ar->bits = (unsigned char *)&ar->val[ar->maxi];
    return ar;
}

static_fn bool array_unscope(Namval_t *np, Namarr_t *ap) {
    Namfun_t *fp;

    if (!ap->scope) return false;
    if (is_associative(ap)) (*ap->fun)(np, NULL, ASSOC_OP_FREE);
    fp = nv_disc(np, (Namfun_t *)ap, DISC_OP_POP);
    if (fp && !(fp->nofree & 1)) free(fp);
    nv_delete(np, NULL, 0);
    return true;
}

static_fn void array_syncsub(Namarr_t *aq, Namarr_t *ap) {
    ((struct index_array *)aq)->cur = ((struct index_array *)ap)->cur;
}

static_fn bool array_covered(Namval_t *np, struct index_array *ap) {
    UNUSED(np);
    struct index_array *aq = (struct index_array *)ap->namarr.scope;
    if (!is_associative(&ap->namarr) && aq) {
        return (ap->cur < aq->maxi) && FETCH_VT(aq->val[ap->cur], const_cp);
    }
    return false;
}

//
// Replace discipline with new one.
//
static_fn void array_setptr(Namval_t *np, struct index_array *old, struct index_array *new) {
    Namfun_t **fp = &np->nvfun;

    while (*fp && *fp != &old->namarr.namfun) fp = &((*fp)->next);
    if (*fp) {
        new->namarr.namfun.next = (*fp)->next;
        *fp = &new->namarr.namfun;
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
    maxi = roundof(maxi, ARRAY_INCR);
    return maxi > ARRAY_MAX ? ARRAY_MAX : maxi;
}

static_fn struct index_array *array_grow(Namval_t *, struct index_array *, int);

// Return next index after the highest element in an array.
int array_maxindex(Namval_t *np) {
    struct index_array *ap = (struct index_array *)nv_arrayptr(np);
    assert(ap);
    int i = ap->maxi;
    if (is_associative(&ap->namarr)) return -1;
    while (--i >= 0 && !FETCH_VT(ap->val[i], const_cp)) {
        ;  // empty loop
    }
    return i + 1;
}

// Check if array is empty
int array_isempty(Namval_t *np) { return array_maxindex(np) <= 0; }

static_fn struct Value *array_getup(Namval_t *np, Namarr_t *arp, int update) {
    struct index_array *ap = (struct index_array *)arp;
    struct Value *up;
    bool nofree = false;

    if (!arp) return &np->nvalue;
    if (is_associative(&ap->namarr)) {
        Namval_t *mp;
        mp = (*arp->fun)(np, NULL, ASSOC_OP_CURRENT);
        if (mp) {
            nofree = nv_isattr(mp, NV_NOFREE) == NV_NOFREE;
            up = &(mp->nvalue);  // parens are to silence false positive from cppcheck
        } else {
            return (*arp->fun)(np, NULL, ASSOC_OP_ADD2);
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
    struct Value *up;

    if (is_associative(&ap->namarr)) {
        np = nv_opensub(np);
        return np && !nv_isnull(np);
    }
    if (ap->cur >= ap->maxi) return false;
    up = &(ap->val[ap->cur]);
    if (FETCH_VTP(up, const_cp) == Empty) {
        Namfun_t *fp = &arp->namfun;
        for (fp = fp->next; fp; fp = fp->next) {
            if (fp->disc && (fp->disc->getnum || fp->disc->getval)) return true;
        }
    }
    return FETCH_VTP(up, const_cp) && FETCH_VTP(up, const_cp) != Empty;
}

//
// Get the Value pointer for an array.
// Delete space as necessary if flag is ARRAY_DELETE.
// After the lookup is done the last @ or * subscript is incremented.
//
static_fn Namval_t *array_find(Namval_t *np, Namarr_t *arp, int flag) {
    Shell_t *shp = sh_ptr(np);
    struct index_array *ap = (struct index_array *)arp;
    struct Value *up;
    Namval_t *mp;

    if (flag & ARRAY_LOOKUP) {
        ap->namarr.flags &= ~ARRAY_NOSCOPE;
    } else {
        ap->namarr.flags |= ARRAY_NOSCOPE;
    }

    bool wasundef = nv_isflag(ap->namarr.flags, ARRAY_UNDEF);
    if (wasundef) {
        ap->namarr.flags &= ~ARRAY_UNDEF;
        // Delete array is the same as delete array[@].
        if (flag & ARRAY_DELETE) {
            nv_putsub(np, NULL, 0, ARRAY_SCAN | ARRAY_NOSCOPE);
            ap->namarr.flags |= ARRAY_SCAN;
        } else {  // same as array[0]
            if (is_associative(&ap->namarr)) {
                (*ap->namarr.fun)(np, "0", flag == ARRAY_ASSIGN ? ASSOC_OP_ADD : ASSOC_OP_ADD2);
            } else {
                ap->cur = 0;
            }
        }
    }
    if (nv_isattr(np, NV_NOTSET) == NV_NOTSET) nv_offattr(np, NV_BINARY);
    if (is_associative(&ap->namarr)) {
        mp = (*arp->fun)(np, NULL, ASSOC_OP_CURRENT);
        if (!mp) {
            static struct Value dummy_value;
            STORE_VT(dummy_value, cp, NULL);
            up = &dummy_value;
        } else if (nv_isarray(mp)) {
            if (wasundef) nv_putsub(mp, NULL, 0, ARRAY_UNDEF);
            return mp;
        } else {
            up = &mp->nvalue;
            if (nv_isvtree(mp)) {
                if (!FETCH_VTP(up, const_cp) && flag == ARRAY_ASSIGN) {
                    nv_arraychild(np, mp, 0);
                    ap->namarr.nelem++;
                }
                return mp;
            } else if (!FETCH_VTP(up, const_cp)) {
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
        if ((!FETCH_VTP(up, const_cp) || FETCH_VTP(up, const_cp) == Empty) && nv_type(np) &&
            nv_isvtree(np)) {
            char *cp;
            if (!ap->namarr.table) {
                ap->namarr.table = dtopen(&_Nvdisc, Dtoset);
                dtuserdata(ap->namarr.table, shp, 1);
            }
            sfprintf(shp->strbuf, "%d", ap->cur);
            cp = sfstruse(shp->strbuf);
            mp = nv_search(cp, ap->namarr.table, NV_ADD);
            assert(mp);  // it is theoretically possible for that nv_search() to fail
            mp->nvenv = np;
            nv_arraychild(np, mp, 0);
        }
        struct Namval *up_np = FETCH_VTP(up, np);
        if (up_np && array_isbit(ap->bits, ap->cur, ARRAY_CHILD)) {
            if (wasundef && nv_isarray(up_np)) nv_putsub(up_np, NULL, 0, ARRAY_UNDEF);
            return up_np;
        }
        if (flag & ARRAY_ASSIGN) {
            array_clrbit(ap->bits, ap->cur, ARRAY_UNSET);
        } else if (!FETCH_VTP(up, np) && nv_isattr(np, NV_INTEGER) &&
                   array_isbit(ap->bits, ap->cur, ARRAY_UNSET)) {
            nv_onattr(np, NV_BINARY);
        }
    }
    STORE_VT(np->nvalue, const_cp, FETCH_VTP(up, const_cp));
    if (!FETCH_VTP(up, const_cp) && nv_isattr(np, NV_NOTSET) != NV_NOTSET) {
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

bool nv_arraysettype(Namval_t *np, Namval_t *tp, const char *sub, nvflag_t flags) {
    Shell_t *shp = sh_ptr(np);
    Namval_t *nq;
    bool rdonly = nv_isattr(np, NV_RDONLY) == NV_RDONLY;
    bool xtrace = sh_isoption(shp, SH_XTRACE);

    shp->last_table = NULL;
    if (!tp->nvfun) return true;

    Namarr_t *ap = nv_arrayptr(np);
    assert(ap);
    if (!ap->table) {
        ap->table = dtopen(&_Nvdisc, Dtoset);
        dtuserdata(ap->table, shp, 1);
    }
    nq = nv_search(sub, ap->table, NV_ADD);
    if (nq) {
        char *av[2] = {NULL, NULL};

        const char *cp = FETCH_VT(nq->nvalue, const_cp);
        if (!nq->nvfun && cp && *cp == 0) _nv_unset(nq, NV_RDONLY);
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
            shp->prefix = NULL;
            sh_eval(shp, sh_sfeval((const char **)av), 0);
            shp->prefix = prefix;
            ap->flags |= ARRAY_SCAN;
            if (xtrace) sh_onoption(shp, SH_XTRACE);
        }
        if (av[0]) free(av[0]);
        return true;
    }
    return false;
}

static_fn Namfun_t *array_clone(Namval_t *np, Namval_t *mp, nvflag_t flags, Namfun_t *fp) {
    Namarr_t *ap = (Namarr_t *)fp;
    Namval_t *nq, *mq;
    char *name;
    char *sub = NULL;
    int flg, skipped = 0;
    Dt_t *otable = ap->table;
    struct index_array *aq = (struct index_array *)ap, *ar;
    Shell_t *shp = sh_ptr(np);

    if (flags & NV_MOVE) {
        if ((flags & NV_COMVAR) && nv_putsub(np, NULL, 0, ARRAY_SCAN)) {
            do {
                nq = nv_opensub(np);
                if (nq) nq->nvenv = mp;
            } while (nv_nextsub(np));
        }
        return fp;
    }
    flg = ap->flags;
    if (flg & ARRAY_NOCLONE) return 0;
    if ((flags & NV_TYPE) && !ap->scope) {
        aq = array_scope(np, aq, flags);
        return &aq->namarr.namfun;
    }
    ap = (Namarr_t *)nv_clone_disc(fp, 0);
    if (flags & NV_COMVAR) {
        ap->scope = NULL;
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
    mp->nvfun = &ap->namfun;
    mp->nvflag &= NV_MINIMAL;
    mp->nvflag |= (np->nvflag & ~(NV_MINIMAL | NV_NOFREE));
    if (!(flg & (ARRAY_SCAN | ARRAY_UNDEF)) && (sub = nv_getsub(np))) sub = strdup(sub);
    ar = (struct index_array *)ap;
    if (!is_associative(ap)) ar->bits = (unsigned char *)&ar->val[ar->maxi];
    if (!nv_putsub(np, NULL, 0, ARRAY_SCAN | ((flags & NV_COMVAR) ? 0 : ARRAY_NOSCOPE))) {
        if (ap->fun) (*ap->fun)(np, (char *)np, ASSOC_OP_ADD2);
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
            assert(mq);
            if (flags & NV_COMVAR) mq->nvenv = mp;
        }
        if (nq && (((flags & NV_COMVAR) && nv_isvtree(nq)) || nv_isarray(nq))) {
            STORE_VT(mq->nvalue, const_cp, NULL);
            if (!is_associative(ap)) STORE_VT(ar->val[ar->cur], np, mq);
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
            if (!is_associative(ap)) STORE_VT(ar->val[ar->cur], const_cp, NULL);
            nv_putval(mp, (char *)&d, NV_LDOUBLE);
        } else {
            if (!is_associative(ap)) STORE_VT(ar->val[ar->cur], const_cp, NULL);
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
    return &ap->namfun;
}

static_fn char *array_getval(Namval_t *np, Namfun_t *disc) {
    Namarr_t *aq, *ap = (Namarr_t *)disc;
    Namval_t *mp;
    char *cp = NULL;

    mp = array_find(np, ap, ARRAY_LOOKUP);
    if (mp != np) {
        if (!mp && !is_associative(ap) && (aq = (Namarr_t *)ap->scope)) {
            array_syncsub(aq, ap);
            if ((mp = array_find(np, aq, ARRAY_LOOKUP)) == np) return nv_getv(np, &aq->namfun);
        }
        if (mp) {
            cp = nv_getval(mp);
            nv_offattr(mp, NV_EXPORT);
        }
        return cp;
    }
    return nv_getv(np, &ap->namfun);
}

static_fn Sfdouble_t array_getnum(Namval_t *np, Namfun_t *disc) {
    Namarr_t *aq, *ap = (Namarr_t *)disc;
    Namval_t *mp;

    mp = array_find(np, ap, ARRAY_LOOKUP);
    if (mp != np) {
        if (!mp && !is_associative(ap) && (aq = (Namarr_t *)ap->scope)) {
            array_syncsub(aq, ap);
            if ((mp = array_find(np, aq, ARRAY_LOOKUP)) == np) return nv_getn(np, &aq->namfun);
        }
        return mp ? nv_getnum(mp) : 0;
    }
    return nv_getn(np, &ap->namfun);
}

static_fn void array_putval(Namval_t *np, const void *string, nvflag_t flags, Namfun_t *dp) {
    Namarr_t *ap = (Namarr_t *)dp;
    struct Value *up;
    Namval_t *mp;
    struct index_array *aq = (struct index_array *)ap;
    int scan;
    bool nofree = nv_isattr(np, NV_NOFREE) == NV_NOFREE;

    do {
        bool xfree = is_associative(ap) ? false : array_isbit(aq->bits, aq->cur, ARRAY_NOFREE);
        mp = array_find(np, ap, string ? ARRAY_ASSIGN : ARRAY_DELETE);
        scan = ap->flags & ARRAY_SCAN;
        if (mp && mp != np) {
            if (!is_associative(ap) && string && !(flags & NV_APPEND) && !nv_type(np) &&
                nv_isvtree(mp) && !(ap->flags & ARRAY_TREE)) {
                if (!nv_isattr(np, NV_NOFREE)) _nv_unset(mp, flags & NV_RDONLY);
                array_clrbit(aq->bits, aq->cur, ARRAY_CHILD);
                STORE_VT(aq->val[aq->cur], const_cp, NULL);
                if (!nv_isattr(mp, NV_NOFREE)) nv_delete(mp, ap->table, 0);
                goto skip;
            }
            if (!xfree) nv_putval(mp, string, flags);
            if (string) {
                if (ap->namfun.type && ap->namfun.type != nv_type(mp)) {
                    nv_arraysettype(np, ap->namfun.type, nv_getsub(np), 0);
                }
                continue;
            }
            ap->flags |= scan;
        }
        if (!string) {
            if (mp) {
                if (is_associative(ap)) {
                    (*ap->fun)(np, NULL, ASSOC_OP_DELETE);
                    STORE_VT(np->nvalue, const_cp, NULL);
                } else {
                    if (mp != np) {
                        array_clrbit(aq->bits, aq->cur, ARRAY_CHILD);
                        STORE_VT(aq->val[aq->cur], const_cp, NULL);
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
                    (*ap->fun)(np, NULL, ASSOC_OP_FREE);
                } else if (ap->table) {
                    dtclose(ap->table);
                    ap->table = NULL;
                }
                nv_offattr(np, NV_ARRAY);
            }
            if (!mp || mp != np || is_associative(ap)) continue;
        }
    skip:
        // Prevent empty string from being deleted.
        up = array_getup(np, ap, !nofree);
        if (FETCH_VTP(up, const_cp) == Empty && nv_isattr(np, NV_INTEGER)) {
            nv_offattr(np, NV_BINARY);
            STORE_VTP(up, const_cp, NULL);
        }
        if (nv_isarray(np)) STORE_VT(np->nvalue, up, up);
        nv_putv(np, string, flags, &ap->namfun);
        if (nofree && !FETCH_VTP(up, const_cp)) STORE_VTP(up, const_cp, Empty);
        if (!is_associative(ap)) {
            if (string) {
                array_clrbit(aq->bits, aq->cur, ARRAY_NOFREE);
            } else if (mp == np) {
                STORE_VT(aq->val[aq->cur], const_cp, NULL);
            }
        }
        if (string && ap->namfun.type && nv_isvtree(np)) {
            nv_arraysettype(np, ap->namfun.type, nv_getsub(np), 0);
        }
    } while (!string && nv_nextsub(np));
    ap->flags &= ~ARRAY_NOSCOPE;
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
        nfp = nv_disc(np, &ap->namfun, DISC_OP_POP);
        if (nfp && !(nfp->nofree & 1)) {
            ap = NULL;
            free(nfp);
        }
        if (!nv_isnull(np)) {
            if (!np->nvfun) nv_onattr(np, NV_NOFREE);
            _nv_unset(np, flags);
        } else {
            nv_offattr(np, NV_NOFREE);
        }
        if (FETCH_VT(np->nvalue, const_cp) == Empty) STORE_VT(np->nvalue, const_cp, NULL);
    }
    if (!string && (flags & NV_TYPE) && ap) array_unscope(np, ap);
}

static const Namdisc_t array_disc = {.dsize = sizeof(Namarr_t),
                                     .putval = array_putval,
                                     .getval = array_getval,
                                     .getnum = array_getnum,
                                     .clonef = array_clone};

static_fn void array_copytree(Namval_t *np, Namval_t *mp) {
    Namfun_t *fp = nv_disc(np, NULL, DISC_OP_POP);
    nv_offattr(np, NV_ARRAY);
    nv_clone(np, mp, 0);
    char *sp = FETCH_VT(np->nvalue, cp);
    if (sp && !nv_isattr(np, NV_NOFREE)) free(sp);
    STORE_VT(np->nvalue, up, &mp->nvalue);
    fp->nofree &= ~1;
    nv_disc(np, (Namfun_t *)fp, DISC_OP_FIRST);
    fp->nofree |= 1;
    nv_onattr(np, NV_ARRAY);
    mp->nvenv = np;
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
    size = (newsize - 1) * sizeof(struct Value) + newsize;
    ap = calloc(1, sizeof(*ap) + size);
    ap->maxi = newsize;
    ap->cur = maxi;
    ap->bits = (unsigned char *)&ap->val[newsize];
    memset(ap->bits, ARRAY_UNSET, newsize);
    if (arp) {
        ap->namarr = arp->namarr;
        ap->namarr.namfun.dsize = sizeof(*ap) + size;
        ap->last = arp->last;
        for (i = 0; i < arp->maxi; i++) {
            ap->bits[i] = arp->bits[i];
            STORE_VT(ap->val[i], const_cp, FETCH_VT(arp->val[i], const_cp));
        }
        memcpy(ap->bits, arp->bits, arp->maxi);
        array_setptr(np, arp, ap);
        free(arp);
    } else {
        int flags = 0;
        Namval_t *mp = NULL;
        ap->namarr.namfun.dsize = sizeof(*ap) + size;
        i = 0;
        ap->namarr.fun = NULL;
        if ((nv_isnull(np) || FETCH_VT(np->nvalue, const_cp) == Empty) &&
            nv_isattr(np, NV_NOFREE)) {
            flags = ARRAY_TREE;
            nv_offattr(np, NV_NOFREE);
        }
        if (FETCH_VT(np->nvalue, const_cp) == Empty) STORE_VT(np->nvalue, const_cp, NULL);
        if (nv_hasdisc(np, &array_disc) || (nv_type(np) && nv_isvtree(np))) {
            Shell_t *shp = sh_ptr(np);
            ap->namarr.table = dtopen(&_Nvdisc, Dtoset);
            dtuserdata(ap->namarr.table, shp, 1);
            mp = nv_search("0", ap->namarr.table, NV_ADD);
            if (mp && nv_isnull(mp)) {
                Namfun_t *fp;
                STORE_VT(ap->val[0], np, mp);
                array_setbit(ap->bits, 0, ARRAY_CHILD);
                for (fp = np->nvfun; fp && !fp->disc->readf; fp = fp->next) {
                    ;  // empty loop
                }
                if (fp && fp->disc && fp->disc->readf) (*fp->disc->readf)(mp, NULL, 0, fp);
                i++;
            }
        } else {
            const char *cp = FETCH_VT(np->nvalue, const_cp);
            STORE_VT(ap->val[0], const_cp, cp);
            if (cp) {
                i++;
            } else if (nv_isattr(np, NV_INTEGER) && !nv_isnull(np)) {
                // TODO: Figure out why this statement is present since `d` is unused:
                // Sfdouble_t d = nv_getnum(np);
                i++;
            }
        }
        ap->last = i;
        ap->namarr.nelem = i;
        ap->namarr.flags = flags;
        ap->namarr.namfun.disc = &array_disc;
        nv_disc(np, (Namfun_t *)ap, DISC_OP_FIRST);
        nv_onattr(np, NV_ARRAY);
        if (mp) {
            array_copytree(np, mp);
            ap->namarr.namfun.nofree &= ~1;
        }
    }
    for (; i < newsize; i++) STORE_VT(ap->val[i], const_cp, NULL);
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
static_fn Namarr_t *nv_changearray(Namval_t *np,
                                   void *(*fun)(Namval_t *, const char *, Nvassoc_op_t)) {
    Namarr_t *ap;
    char numbuff[NUMSIZE + 1];
    unsigned dot, digit, n;
    struct Value *up;
    struct index_array *save_ap;
    char *string_index = &numbuff[NUMSIZE];
    numbuff[NUMSIZE] = '\0';

    if (!fun || !(ap = nv_arrayptr(np)) || is_associative(ap)) return NULL;

    nv_stack(np, &ap->namfun);
    save_ap = (struct index_array *)nv_stack(np, 0);
    ap = (Namarr_t *)((*fun)(np, NULL, ASSOC_OP_INIT));
    ap->nelem = 0;
    ap->flags = 0;
    ap->fun = fun;
    nv_onattr(np, NV_ARRAY);

    for (dot = 0; dot < (unsigned)save_ap->maxi; dot++) {
        if (FETCH_VT(save_ap->val[dot], const_cp)) {
            if ((digit = dot) == 0) {
                *--string_index = '0';
            } else {
                while ((n = digit)) {
                    digit /= 10;
                    *--string_index = '0' + (n - 10 * digit);
                }
            }
            nv_putsub(np, string_index, 0, ARRAY_ADD);
            up = (struct Value *)((*ap->fun)(np, NULL, ASSOC_OP_ADD2));
            STORE_VTP(up, const_cp, FETCH_VT(save_ap->val[dot], const_cp));
            STORE_VT(save_ap->val[dot], const_cp, NULL);
        }
        string_index = &numbuff[NUMSIZE];
    }
    free(save_ap);
    return ap;
}

//
// Set the associative array processing method for node <np> to <fun>.
// The array pointer is returned if successful.
//
Namarr_t *nv_setarray(Namval_t *np, void *(*fun)(Namval_t *, const char *, Nvassoc_op_t)) {
    Namarr_t *ap;
    nvflag_t flags = 0;

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

    ap = (Namarr_t *)((*fun)(np, NULL, ASSOC_OP_INIT));
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
            Namval_t *mp = (*fun)(np, NULL, ASSOC_OP_CURRENT);
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
    struct Value *up;
    Namval_t *tp;

    if (!nq) return ap ? array_find(np, ap, ARRAY_LOOKUP) : NULL;
    if (!ap) {
        nv_putsub(np, NULL, 0, ARRAY_FILL);
        ap = nv_arrayptr(np);
        assert(ap);
    }
    if (!(up = array_getup(np, ap, 0))) return NULL;
    STORE_VT(np->nvalue, const_cp, FETCH_VTP(up, const_cp));
    tp = nv_type(np);
    if (tp || c) {
        ap->flags |= ARRAY_NOCLONE;
        nq->nvenv = np;
        if (c == 't') {
            assert(tp);
            nv_clone(tp, nq, 0);
        } else {
            nv_clone(np, nq, NV_NODISC);
        }
        nv_offattr(nq, NV_ARRAY);
        ap->flags &= ~ARRAY_NOCLONE;
    }
    nq->nvenv = np;
    nq->nvshell = np->nvshell;
    if ((fp = nq->nvfun) && fp->disc && fp->disc->setdisc && (fp = nv_disc(nq, fp, DISC_OP_POP))) {
        free(fp);
    }
    if (!ap->fun) {
        struct index_array *aq = (struct index_array *)ap;
        array_setbit(aq->bits, aq->cur, ARRAY_CHILD);
        array_clrbit(aq->bits, aq->cur, ARRAY_UNSET);
        if (c == '.' && !FETCH_VT(nq->nvalue, const_cp)) ap->nelem++;
        STORE_VTP(up, np, nq);
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
    struct index_array *aq = NULL;
    struct index_array *ar = NULL;

    if (!ap || !(ap->namarr.flags & ARRAY_SCAN)) return false;
    if (is_associative(&ap->namarr)) {
        if ((*ap->namarr.fun)(np, NULL, ASSOC_OP_NEXT)) return true;
        ap->namarr.flags &= ~(ARRAY_SCAN | ARRAY_NOCHILD);
        return false;
    }
    if (!(ap->namarr.flags & ARRAY_NOSCOPE)) ar = (struct index_array *)ap->namarr.scope;
    for (dot = ap->cur + 1; dot < (unsigned)ap->maxi; dot++) {
        aq = ap;
        if (!FETCH_VT(ap->val[dot], const_cp) && !(ap->namarr.flags & ARRAY_NOSCOPE)) {
            if (!(aq = ar) || dot >= (unsigned)aq->maxi) continue;
        }
        if (FETCH_VT(aq->val[dot], const_cp) == Empty &&
            array_elem(&aq->namarr) < nv_aimax(np) + 1) {
            ap->cur = dot;
            if (nv_getval(np) == Empty) continue;
        }
        if (FETCH_VT(aq->val[dot], const_cp)) {
            ap->cur = dot;
            if (array_isbit(aq->bits, dot, ARRAY_CHILD)) {
                Namval_t *mp = FETCH_VT(aq->val[dot], np);
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
Namval_t *nv_putsub(Namval_t *np, char *sp, long size, nvflag_t flags) {
    Shell_t *shp = sh_ptr(np);
    struct index_array *ap = (struct index_array *)nv_arrayptr(np);
    if (!ap || !is_associative(&ap->namarr)) {
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
        ap->cur = size;
        if ((flags & ARRAY_SCAN) && (ap->cur--, !nv_nextsub(np))) np = NULL;
        if (flags & (ARRAY_FILL | ARRAY_ADD)) {
            if (!(flags & ARRAY_ADD)) {
                int n;
                if (flags & ARRAY_SETSUB) {
                    for (n = 0; n <= ap->maxi; n++) STORE_VT(ap->val[n], const_cp, NULL);
                    ap->namarr.nelem = 0;
                    ap->namarr.flags = 0;
                }
                for (n = 0; n <= size; n++) {
                    if (!FETCH_VT(ap->val[n], const_cp)) {
                        array_clrbit(ap->bits, n, ARRAY_UNSET);
                        STORE_VT(ap->val[n], const_cp, Empty);
                        if (!array_covered(np, ap)) ap->namarr.nelem++;
                    }
                    ap->last = ap->namarr.nelem;
                }
                n = ap->maxi - ap->maxi;
                if (n) memset(&ap->val[size], 0, n * sizeof(struct Value));
            } else if (!(sp = (char *)FETCH_VT(ap->val[size], const_cp)) || sp == Empty) {
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
                    assert(mp);
                    mp->nvenv = np;
                    nv_arraychild(np, mp, 0);
                    nv_setvtree(mp);
                } else {
                    STORE_VT(ap->val[size], const_cp, Empty);
                }
                if (!sp && !array_covered(np, ap)) ap->namarr.nelem++;
                array_clrbit(ap->bits, size, ARRAY_UNSET);
            }
        } else if (!(flags & ARRAY_SCAN)) {
            ap->namarr.flags &= ~ARRAY_SCAN;
            if (array_isbit(ap->bits, size, ARRAY_CHILD) && FETCH_VT(ap->val[size], np)) {
                nv_putsub(FETCH_VT(ap->val[size], np), NULL, 0, ARRAY_UNDEF);
            }
            if (sp && !(flags & ARRAY_ADD) && !FETCH_VT(ap->val[size], const_cp)) np = NULL;
        }
        return np;
    }
    ap->namarr.flags &= ~ARRAY_UNDEF;
    if (!(flags & ARRAY_FILL)) ap->namarr.flags &= ~ARRAY_SCAN;
    ap->namarr.flags |= (flags & (ARRAY_SCAN | ARRAY_NOCHILD | ARRAY_UNDEF | ARRAY_NOSCOPE));
    if (sp) {
        if (flags & ARRAY_SETSUB) {
            (*ap->namarr.fun)(np, sp, ASSOC_OP_SETSUB);
            return np;
        }
        (*ap->namarr.fun)(np, sp, (flags & ARRAY_ADD) ? ASSOC_OP_ADD : ASSOC_OP_ADD2);
        if (!(flags & (ARRAY_SCAN | ARRAY_ADD)) && !(*ap->namarr.fun)(np, NULL, ASSOC_OP_CURRENT)) {
            np = NULL;
        }
    } else if (flags & ARRAY_SCAN) {
        (*ap->namarr.fun)(np, (char *)np, ASSOC_OP_ADD2);
    } else if (flags & ARRAY_UNDEF) {
        (*ap->namarr.fun)(np, "", ASSOC_OP_ADD2);
    }
    if ((flags & ARRAY_SCAN) && !nv_nextsub(np)) np = NULL;
    return np;
}

//
// Process an array subscript for node <np> given the subscript <cp>.
// Returns pointer to character after the subscript.
//
char *nv_endsubscript(Namval_t *np, char *cp, nvflag_t mode, void *context) {
    int count = 1, quoted = 0, c;
    char *sp = cp + 1;
    Shell_t *shp = context;

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
        bool scan = false;
        if (ap) scan = nv_isflag(ap->flags, ARRAY_SCAN);
        if ((mode & NV_ASSIGN) && (cp[1] == '=' || cp[1] == '+')) mode |= NV_ADD;
        nv_putsub(np, sp, 0,
                  ((mode & NV_ADD) ? ARRAY_ADD : 0) |
                      (cp[1] && (mode & NV_ADD) ? ARRAY_FILL : mode & ARRAY_FILL));
        // The nv_putsub() can invalidate `ap` but only if `scan` is zero. So don't use `if (ap)`
        // since that can result in dereferencing a stale pointer. But if `scan` is non-zero then
        // `ap` should still be valid.  See https://github.com/att/ast/issues/828.
        if (scan) ap->flags |= ARRAY_SCAN;
    }
    if (quoted) stkseek(shp->stk, count);
    *cp++ = c;
    return cp;
}

Namval_t *nv_opensub(Namval_t *np) {
    struct index_array *ap = (struct index_array *)nv_arrayptr(np);

    if (!ap) return NULL;
    if (is_associative(&ap->namarr)) {
        return (*ap->namarr.fun)(np, NULL, ASSOC_OP_CURRENT);
    } else if (array_isbit(ap->bits, ap->cur, ARRAY_CHILD)) {
        return FETCH_VT(ap->val[ap->cur], np);
    }
    return NULL;
}

char *nv_getsub(Namval_t *np) {
    static char numbuff[NUMSIZE + 1];
    struct index_array *ap;
    unsigned dot, n;
    char *cp = &numbuff[NUMSIZE];

    if (!np || !(ap = (struct index_array *)nv_arrayptr(np))) return NULL;
    if (is_associative(&ap->namarr)) return (char *)((*ap->namarr.fun)(np, NULL, ASSOC_OP_NAME));
    if (ap->xp) {
        np = nv_namptr(ap->xp, 0);
        STORE_VT(np->nvalue, i16, ap->cur);
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

struct Value *nv_aivec(Namval_t *np, unsigned char **bitp) {
    struct index_array *ap = (struct index_array *)nv_arrayptr(np);
    if (!ap || is_associative(&ap->namarr)) return NULL;
    if (bitp) *bitp = ap->bits;
    return ap->val;
}

int nv_aipack(Namarr_t *arp) {
    struct index_array *ap = (struct index_array *)arp;
    int i, j;

    if (!ap || is_associative(&ap->namarr)) return -1;
    for (i = j = 0; i < ap->maxi; i++) {
        if (FETCH_VT(ap->val[i], np)) {
            ap->bits[j] = ap->bits[i];
            ap->val[j++] = ap->val[i];
        }
    }
    for (i = j; i < ap->maxi; i++) {
        STORE_VT(ap->val[i], np, NULL);
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
    while (--sub > 0 && !FETCH_VT(ap->val[sub], const_cp)) {
        ;  // empty loop
    }
    return sub;
}

static_fn void *nv_assoc_op_init(Namval_t *np, const char *sp) {
    UNUSED(sp);
    Shell_t *shp = sh_ptr(np);
    struct assoc_array *ap = (struct assoc_array *)nv_arrayptr(np);

    assert(!ap);
    ap = calloc(1, sizeof(struct assoc_array));
    ap->namarr.table = dtopen(&_Nvdisc, Dtoset);
    dtuserdata(ap->namarr.table, shp, 1);
    ap->cur = NULL;
    ap->pos = NULL;
    ap->namarr.namfun.disc = &array_disc;
    nv_disc(np, (Namfun_t *)ap, DISC_OP_FIRST);
    ap->namarr.namfun.dsize = sizeof(struct assoc_array);
    ap->namarr.namfun.nofree &= ~1;
    return ap;
}

static_fn void *nv_assoc_op_delete(Namval_t *np, const char *sp) {
    UNUSED(sp);
    struct assoc_array *ap = (struct assoc_array *)nv_arrayptr(np);

    assert(ap);
    if (ap->cur) {
        Dt_t *scope = ap->namarr.scope;
        if (!scope || scope == ap->namarr.table || !nv_search(ap->cur->nvname, scope, 0)) {
            ap->namarr.nelem--;
        }
        _nv_unset(ap->cur, NV_RDONLY);
        nv_delete(ap->cur, ap->namarr.table, 0);
        ap->cur = 0;
    }
    return ap;
}

static_fn void *nv_assoc_op_free(Namval_t *np, const char *sp) {
    UNUSED(sp);
    struct assoc_array *ap = (struct assoc_array *)nv_arrayptr(np);

    assert(ap);
    ap->pos = NULL;
    if (ap->namarr.scope) {
        ap->namarr.table = dtview(ap->namarr.table, NULL);
        dtclose(ap->namarr.scope);
        ap->namarr.scope = NULL;
    } else {
        if (ap->namarr.nelem == 0 && (ap->cur = nv_search("0", ap->namarr.table, 0))) {
            nv_associative(np, NULL, ASSOC_OP_DELETE);
        }
        dtclose(ap->namarr.table);
        ap->namarr.table = NULL;
    }
    return ap;
}

static_fn void *nv_assoc_op_next(Namval_t *np, const char *sp) {
    UNUSED(sp);
    struct assoc_array *ap = (struct assoc_array *)nv_arrayptr(np);

    assert(ap);
    if (!ap->pos) {
        if ((ap->namarr.flags & ARRAY_NOSCOPE) && ap->namarr.scope && dtvnext(ap->namarr.table)) {
            ap->namarr.scope = dtvnext(ap->namarr.table);
            ap->namarr.table->view = 0;
        }
        if (!(ap->pos = ap->cur)) ap->pos = dtfirst(ap->namarr.table);
    } else {
        ap->pos = ap->nextpos;
    }
    for (; (ap->cur = ap->pos); ap->pos = ap->nextpos) {
        ap->nextpos = dtnext(ap->namarr.table, ap->pos);
        if (!nv_isnull(ap->cur)) {
            if ((ap->namarr.flags & ARRAY_NOCHILD) && nv_isattr(ap->cur, NV_CHILD)) {
                continue;
            }
            return ap;
        }
    }
    if ((ap->namarr.flags & ARRAY_NOSCOPE) && ap->namarr.scope && !dtvnext(ap->namarr.table)) {
        ap->namarr.table->view = ap->namarr.scope;
        ap->namarr.scope = ap->namarr.table;
    }
    return NULL;
}

static_fn void *nv_assoc_op_setsub(Namval_t *np, const char *sp) {
    struct assoc_array *ap = (struct assoc_array *)nv_arrayptr(np);

    assert(ap);
    ap->cur = (Namval_t *)sp;
    return ap->cur;
}

static_fn void *nv_assoc_op_current(Namval_t *np, const char *sp) {
    UNUSED(sp);
    struct assoc_array *ap = (struct assoc_array *)nv_arrayptr(np);

    assert(ap);
    if (ap->cur) ap->cur->nvenv = np;
    return ap->cur;
}

static_fn void *nv_assoc_op_name(Namval_t *np, const char *sp) {
    UNUSED(sp);
    Shell_t *shp = sh_ptr(np);
    struct assoc_array *ap = (struct assoc_array *)nv_arrayptr(np);

    assert(ap);
    if (ap->cur) {
        if (!shp->instance && nv_isnull(ap->cur)) return NULL;
        return ap->cur->nvname;
    }
    return NULL;
}

static_fn void *nv_assoc_op_add(Namval_t *np, const char *sp, bool add) {
    struct assoc_array *ap = (struct assoc_array *)nv_arrayptr(np);

    assert(ap);
    if (sp) {
        Shell_t *shp = sh_ptr(np);
        Namval_t *mp = NULL;
        ap->cur = NULL;
        if (sp == (char *)np) return NULL;
        nvflag_t type = nv_isattr(np, ~(NV_NOFREE | NV_ARRAY | NV_CHILD | NV_MINIMAL));
        nvflag_t mode = 0;

        if (add) {
            mode = NV_ADD | NV_NOSCOPE;
        } else if (ap->namarr.flags & ARRAY_NOSCOPE) {
            mode = NV_NOSCOPE;
        }
        if (*sp == 0 && sh_isoption(shp, SH_XTRACE) && add) {
            errormsg(SH_DICT, ERROR_warn(0), "adding empty subscript");
        }
        if (shp->subshell && (mp = nv_search(sp, ap->namarr.table, 0)) && nv_isnull(mp)) {
            ap->cur = mp;
        }
        if ((mp || (mp = nv_search(sp, ap->namarr.table, mode))) && nv_isnull(mp) && add) {
            mp->nvshell = np->nvshell;
            nv_onattr(mp, type);
            mp->nvenv = np;
            if (add && nv_type(np)) nv_arraychild(np, mp, 0);
            if (shp->subshell) sh_assignok(np, 1);
            if (type & NV_INTEGER) {
                nv_onattr(mp, NV_NOTSET);
            } else if (!ap->namarr.scope || !nv_search(sp, dtvnext(ap->namarr.table), 0)) {
                ap->namarr.nelem++;
            }
            if (nv_isnull(mp)) {
                if (nv_isflag(ap->namarr.flags, ARRAY_TREE)) nv_setvtree(mp);
                STORE_VT(mp->nvalue, const_cp, Empty);
            }
        } else if (nv_isflag(ap->namarr.flags, ARRAY_SCAN)) {
            Namval_t fake;
            memset(&fake, 0, sizeof(fake));
            fake.nvname = (char *)sp;
            ap->pos = mp = dtprev(ap->namarr.table, &fake);
            ap->nextpos = dtnext(ap->namarr.table, mp);
        } else if (!mp && *sp && mode == 0) {
            mp = nv_search(sp, ap->namarr.table, NV_ADD | NV_NOSCOPE);
            assert(mp);
            if (!FETCH_VT(mp->nvalue, const_cp)) mp->nvsize |= 1;
        }
        np = mp;
        if (ap->pos && ap->pos == np) {
            ap->namarr.flags |= ARRAY_SCAN;
        } else if (!nv_isflag(ap->namarr.flags, ARRAY_SCAN)) {
            ap->pos = NULL;
        }
        ap->cur = np;
    }
    if (ap->cur) return &ap->cur->nvalue;

    static struct Value dummy_value;
    STORE_VT(dummy_value, cp, NULL);
    return &dummy_value;
}

//
// This is the default implementation for associative arrays.
//
void *nv_associative(Namval_t *np, const char *sp, Nvassoc_op_t op) {
    if (op.val == ASSOC_OP_INIT_val) return nv_assoc_op_init(np, sp);
    if (op.val == ASSOC_OP_DELETE_val) return nv_assoc_op_delete(np, sp);
    if (op.val == ASSOC_OP_FREE_val) return nv_assoc_op_free(np, sp);
    if (op.val == ASSOC_OP_NEXT_val) return nv_assoc_op_next(np, sp);
    if (op.val == ASSOC_OP_SETSUB_val) return nv_assoc_op_setsub(np, sp);
    if (op.val == ASSOC_OP_CURRENT_val) return nv_assoc_op_current(np, sp);
    if (op.val == ASSOC_OP_NAME_val) return nv_assoc_op_name(np, sp);
    if (op.val == ASSOC_OP_ADD_val) return nv_assoc_op_add(np, sp, true);
    if (op.val == ASSOC_OP_ADD2_val) return nv_assoc_op_add(np, sp, false);
    abort();
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
            if (!nv_isnull(np) && FETCH_VT(np->nvalue, const_cp) != Empty) arg0 = 1;
        }
    }
    if (ap) ap->last = arg0 + argc;
    while (--argc >= 0) {
        nv_putsub(np, NULL, (long)argc + arg0, ARRAY_FILL | ARRAY_ADD);
        nv_putval(np, argv[argc], 0);
    }
    if (!ap && (ap = (struct index_array *)nv_arrayptr(np))) ap->last = array_maxindex(np);
}
