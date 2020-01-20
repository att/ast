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

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "argnod.h"
#include "ast.h"
#include "ast_assert.h"
#include "cdt.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "lexstates.h"
#include "name.h"
#include "sfio.h"
#include "stk.h"
#include "variables.h"

#define NVCACHE 8  // must be a power of 2 and not zero

// This var used to be writable but was treated as if it was immutable except for one assignment
// that failed to validate it was modifying only the first, and only, char. So we now make it
// truly immutable to catch any attempts to modify it.
static const char *EmptyStr = "";

static char *savesub = NULL;
static Namval_t NullNode = {.nvname = ".deleted"};
static Dt_t *Refdict;
static Dtdisc_t _Refdisc = {.key = offsetof(struct Namref, np),
                            .size = sizeof(struct Namval_t *),
                            .link = sizeof(struct Namref)};

// The following array of strings must be kept in sync with enum value_type in file "name.h".
const char *value_type_names[] = {
    "vt_do_not_use",  // VT_do_not_use = 0
    "vp",             // VT_vp
    "cp",             // VT_cp
    "const_cp",       // VT_const_cp
    "pp",             // VT_pp
    "uc",             // VT_uc
    "h",              // VT_h
    "i",              // VT_i
    "l",              // VT_l
    "d",              // VT_d
    "f",              // VT_f
    "i16",            // VT_i16
    "ip",             // VT_ip
    "i16p",           // VT_i16p
    "i32p",           // VT_i16p
    "i64p",           // VT_i64p
    "dp",             // VT_dp
    "fp",             // VT_fp
    "sfdoublep",      // VT_sfdoublep
    "np",             // VT_np
    "up",             // VT_up
    "rp",             // VT_rp
    "funp",           // VT_funp
    "nrp",            // VT_nrp
    "shbltinp",       // VT_shbltinp
    "pathcomp",       // VT_pathcomp
    "pidp",           // VT_pidp
    "uidp",           // VT_uidp
    "vt_sentinal"     // VT_sentinal
};

static_fn void attstore(Namval_t *, void *);
static_fn void pushnam(Namval_t *, void *);
static_fn char *staknam(Shell_t *, Namval_t *, char *);
static_fn void rightjust(char *, int, int);
static_fn char *lastdot(char *, int, void *);

struct adata {
    Shell_t *sh;
    Namval_t *tp;
    char *mapname;
    char **argnam;
    int attsize;
    char *attval;
};

struct sh_type {
    void *previous;
    Namval_t **nodes;
    Namval_t *rp;
    short numnodes;
    short maxnodes;
};

struct Namcache {
    struct Cache_entry {
        Dt_t *root;
        Dt_t *last_root;
        char *name;
        Namval_t *np;
        Namval_t *last_table;
        Namval_t *namespace;
        nvflag_t flags;
        short size;
        short len;
    } entries[NVCACHE];
    short index;
    short ok;
};
static struct Namcache nvcache;

bool nv_local = false;

// ======== name value pair routines ========

#include "builtins.h"
#include "shnodes.h"

static_fn char *getbuf(size_t len) {
    static char *buf = NULL;
    static size_t buflen = 0;
    if (buflen < len) {
        if (buflen == 0) {
            buf = malloc(len);
        } else {
            buf = realloc(buf, len);
        }
        assert(buf);
        buflen = len;
    }
    return buf;
}

//
// Output variable name in format for re-input.
//
void sh_outname(Shell_t *shp, Sfio_t *out, char *name, int len) {
    const char *cp = name, *sp;
    int c, offset = stktell(shp->stk);

    while ((sp = strchr(cp, '['))) {
        if (len > 0 && cp + len <= sp) break;
        sfwrite(out, cp, ++sp - cp);
        stkseek(shp->stk, offset);
        while ((c = *sp++)) {
            if (c == ']') {
                break;
            } else if (c == '\\') {
                if (*sp == '[' || *sp == ']' || *sp == '\\') c = *sp++;
            }
            sfputc(shp->stk, c);
        }
        sfputc(shp->stk, 0);
        sfputr(out, sh_fmtq(stkptr(shp->stk, offset)), -1);
        if (len > 0) {
            sfputc(out, ']');
            return;
        }
        cp = sp - 1;
    }
    if (*cp) {
        if (len > 0) {
            sfwrite(out, cp, len);
        } else {
            sfputr(out, cp, -1);
        }
    }
    stkseek(shp->stk, offset);
}

Namval_t *nv_addnode(Namval_t *np, int remove) {
    Shell_t *shp = sh_ptr(np);
    struct sh_type *sp = (struct sh_type *)shp->mktype;
    int i;
    char *name = NULL;

    if (sp->numnodes == 0 && !nv_isnull(np) && shp->last_table) {
        // Could be an redefine.
        Dt_t *root = nv_dict(shp->last_table);
        sp->rp = np;
        nv_delete(np, root, NV_NOFREE);
        np = nv_search(sp->rp->nvname, root, NV_ADD);
    }
    if (sp->numnodes && strncmp(np->nvname, NV_CLASS, sizeof(NV_CLASS) - 1)) {
        name = (sp->nodes[0])->nvname;
        i = strlen(name);
        if (strncmp(np->nvname, name, i)) return np;
    }
    if (sp->rp && sp->numnodes) {
        // Check for a redefine.
        if (name && np->nvname[i] == '.' && np->nvname[i + 1] == '_' && np->nvname[i + 2] == 0) {
            sp->rp = NULL;
        } else {
            Dt_t *root = nv_dict(shp->last_table);
            nv_delete(sp->nodes[0], root, NV_NOFREE);
            dtinsert(root, sp->rp);
            errormsg(SH_DICT, ERROR_exit(1), e_redef, sp->nodes[0]->nvname);
            __builtin_unreachable();
        }
    }
    for (i = 0; i < sp->numnodes; i++) {
        if (np == sp->nodes[i]) {
            if (remove) {
                while (++i < sp->numnodes) sp->nodes[i - 1] = sp->nodes[i];
                sp->numnodes--;
            }
            return np;
        }
    }
    if (remove) return np;
    if (sp->numnodes == sp->maxnodes) {
        sp->maxnodes += 20;
        sp->nodes = realloc(sp->nodes, sizeof(Namval_t *) * sp->maxnodes);
    }
    sp->nodes[sp->numnodes++] = np;
    return np;
}

//
// Given a list of assignments, determine <name> is on the list.
// Returns a pointer to the argnod on the list or NULL.
//
struct argnod *nv_onlist(struct argnod *arg, const char *name) {
    char *cp;
    int len = strlen(name);

    for (; arg; arg = arg->argnxt.ap) {
        if (*arg->argval == 0 && arg->argchn.ap &&
            !(arg->argflag & ~(ARG_APPEND | ARG_QUOTED | ARG_MESSAGE))) {
            cp = ((struct fornod *)arg->argchn.ap)->fornam;
        } else {
            cp = arg->argval;
        }
        if (strncmp(cp, name, len) == 0 && (cp[len] == 0 || cp[len] == '=')) return arg;
    }
    return 0;
}

//
// Perform parameter assignment for a linked list of parameters.
// <flags> contains attributes for the parameters.
//
Namval_t **sh_setlist(Shell_t *shp, struct argnod *arg, nvflag_t flags, Namval_t *typ) {
    char *cp;
    Namval_t *np, *mp;
    char *trap = shp->st.trap[SH_DEBUGTRAP];
    char *prefix = shp->prefix;
    int traceon = (sh_isoption(shp, SH_XTRACE) != 0);
    nvflag_t array = flags & (NV_ARRAY | NV_IARRAY);
    Namarr_t *ap;
    Namval_t node, **nodelist = NULL, **nlp = NULL;
    struct Namref nr;
    bool maketype = nv_isflag(flags, NV_TYPE);
    struct sh_type shtp;

    nv_isvalid(flags);
    memset(&node, 0, sizeof(node));
    memset(&nr, 0, sizeof(nr));

    if (maketype) {
        shtp.previous = shp->mktype;
        // This is incredibly dangerous. Lint tools like cppcheck warn about the assignment of the
        // address of a local auto var (i.e., stack var) to a another var. This should be okay
        // because the code explicitly unwinds this assigment and this is needed to support nested
        // (i.e., recursive) expressions without explicitly performing heap allocations. Whether or
        // not the code actually does so is unclear.
        //
        // cppcheck-suppress autoVariables
        shp->mktype = &shtp;
        shtp.numnodes = 0;
        shtp.maxnodes = 20;
        shtp.rp = NULL;
        shtp.nodes = malloc(shtp.maxnodes * sizeof(Namval_t *));
    }
    if (shp->namespace && nv_dict(shp->namespace) == shp->var_tree) flags |= NV_NOSCOPE;
    flags &= ~(NV_TYPE | NV_ARRAY | NV_IARRAY);
    if (sh_isoption(shp, SH_ALLEXPORT)) flags |= NV_EXPORT;
    if (shp->prefix) {
        flags &= ~(NV_IDENT | NV_EXPORT);
        flags |= NV_VARNAME;
    } else {
        shp->prefix_root = shp->first_root = NULL;
    }
    if (flags & NV_DECL) {
        struct argnod *ap;
        int n = 0;
        for (ap = arg; ap; ap = ap->argnxt.ap) n++;
        nlp = nodelist = stkalloc(shp->stk, (n + 1) * sizeof(Namval_t *));
        nodelist[n] = 0;
    }
    for (; arg; arg = arg->argnxt.ap) {
        Namval_t *nq = NULL;
        shp->used_pos = 0;
        if (arg->argflag & ARG_MAC) {
            shp->prefix = NULL;
            cp = sh_mactrim(shp, arg->argval, (flags & NV_NOREF) ? -3 : -1);
            shp->prefix = prefix;
        } else {
            stkseek(shp->stk, 0);
            if (*arg->argval == 0 && arg->argchn.ap &&
                !(arg->argflag & ~(ARG_APPEND | ARG_QUOTED | ARG_MESSAGE | ARG_ARRAY))) {
                nvflag_t flag = NV_VARNAME | NV_ARRAY | NV_ASSIGN;
                int sub = 0;
                struct fornod *fp = (struct fornod *)arg->argchn.ap;
                Shnode_t *tp = fp->fortre;
                flag |= (flags & (NV_NOSCOPE | NV_STATIC));
                if (arg->argflag & ARG_ARRAY) array |= NV_IARRAY;
                if (arg->argflag & ARG_QUOTED) {
                    cp = sh_mactrim(shp, fp->fornam, -1);
                } else {
                    cp = fp->fornam;
                }
                error_info.line = fp->fortyp - shp->st.firstline;
                if (!array && tp->tre.tretyp != TLST && tp->com.comset && !tp->com.comarg &&
                    tp->com.comset->argval[0] == 0 && tp->com.comset->argval[1] == '[') {
                    array |= (tp->com.comset->argflag & ARG_MESSAGE) ? NV_IARRAY : NV_ARRAY;
                }
                if (prefix && tp->com.comset && *cp == '[') {
                    shp->prefix = NULL;
                    np = nv_open(prefix, shp->var_tree, flag);
                    shp->prefix = prefix;
                    if (np) {
                        if (nv_isvtree(np) && !nv_isarray(np)) {
                            sfputc(shp->stk, '.');
                            sfputr(shp->stk, cp, -1);
                            cp = stkfreeze(shp->stk, 1);
                        }
                        nv_close(np);
                    }
                }
                np = nv_open(cp, shp->var_tree, flag | NV_ASSIGN);
                if (nlp) *nlp++ = np;
                if ((arg->argflag & ARG_APPEND) && (tp->tre.tretyp & COMMSK) == TCOM &&
                    tp->com.comset && !nv_isvtree(np) &&
                    (((ap = nv_arrayptr(np)) && !ap->fun && !nv_opensub(np)) ||
                     (!ap && nv_isarray(np) && tp->com.comarg &&
                      !((mp = nv_search(tp->com.comarg->argval, shp->fun_tree, 0)) &&
                        nv_isattr(mp, BLT_DCL))))) {
                    if (tp->com.comarg) {
                        struct argnod *ap = tp->com.comset;
                        while (ap->argnxt.ap) ap = ap->argnxt.ap;
                        ap->argnxt.ap = tp->com.comarg;
                    }
                    tp->com.comarg = tp->com.comset;
                    tp->com.comset = NULL;
                    tp->com.comtyp = COMSCAN;
                }
                if (nv_isattr(np, NV_RDONLY) && np->nvfun && !(flags & NV_RDONLY)) {
                    errormsg(SH_DICT, ERROR_exit(1), e_readonly, nv_name(np));
                    __builtin_unreachable();
                }
                if (nv_isattr(np, NV_NOFREE) && nv_isnull(np)) nv_offattr(np, NV_NOFREE);
                if (nv_istable(np)) _nv_unset(np, 0);
                if (typ && !array && (!shp->prefix || nv_isnull(np) || nv_isarray(np))) {
                    if (!(nv_isnull(np)) && !nv_isarray(np)) _nv_unset(np, 0);
                    nv_settype(np, typ, 0);
                }
                if ((flags & NV_STATIC) && !nv_isattr(np, NV_EXPORT) && !nv_isnull(np)) {
                    goto check_type;
                }
                ap = nv_arrayptr(np);
                if (sh_isoption(shp, SH_BASH) && !array && !ap && !(flags & NV_COMVAR) &&
                    !np->nvfun && !tp->com.comset && !tp->com.comarg) {
                    array = NV_IARRAY;
                }
                if (array && (!ap || !ap->namfun.type)) {
                    if (!(arg->argflag & ARG_APPEND)) _nv_unset(np, NV_EXPORT);
                    if (array & NV_ARRAY) {
                        nv_setarray(np, nv_associative);
                        if (typ) nv_settype(np, typ, 0);
                    } else {
                        nv_onattr(np, NV_ARRAY);
                    }
                }
                if (array && tp->tre.tretyp != TLST && !tp->com.comset && !tp->com.comarg) {
                    goto check_type;
                }
                // Check for array assignment.
                if ((tp->tre.tretyp & COMMSK) == TCOM && tp->com.comarg && !tp->com.comset &&
                    ((array & NV_IARRAY) || !((mp = tp->com.comnamp) && nv_isattr(mp, BLT_DCL)))) {
                    int argc;
                    Dt_t *last_root = shp->last_root;
                    char **argv = sh_argbuild(shp, &argc, &tp->com, 0);
                    shp->last_root = last_root;
                    if (shp->mktype && shp->dot_depth == 0 &&
                        np == ((struct sh_type *)shp->mktype)->nodes[0]) {
                        shp->mktype = NULL;
                        errormsg(SH_DICT, ERROR_exit(1), "%s: not a known type name", argv[0]);
                        __builtin_unreachable();
                    }
                    if (!(arg->argflag & ARG_APPEND)) {
                        if (!nv_isarray(np) || ((ap = nv_arrayptr(np)) && (ap->nelem))) {
                            if (ap) ap->flags |= ARRAY_UNDEF;
                            _nv_unset(np, NV_EXPORT);
                        }
                    }
                    nv_setvec(np, (arg->argflag & ARG_APPEND), argc, argv);
                    nv_onattr(np, NV_ARRAY);
                    if (!traceon && !trap) goto check_type;

                    int n = -1;
                    char *name = nv_name(np);
                    if (arg->argflag & ARG_APPEND) n = '+';
                    if (trap) {
                        sh_debug(shp, trap, name, NULL, argv,
                                 (arg->argflag & ARG_APPEND) | ARG_ASSIGN);
                    }
                    if (traceon) {
                        sh_trace(shp, NULL, 0);
                        sfputr(sfstderr, name, n);
                        sfwrite(sfstderr, "=( ", 3);
                        while ((cp = *argv++)) sfputr(sfstderr, sh_fmtq(cp), ' ');
                        sfwrite(sfstderr, ")\n", 2);
                    }
                    goto check_type;
                }
                if ((tp->tre.tretyp & COMMSK) == TFUN) goto skip;
                if (tp->tre.tretyp == TCOM && !tp->com.comset && !tp->com.comarg) {
                    if (arg->argflag & ARG_APPEND) goto skip;

                    if (ap && ap->nelem > 0) {
                        nv_putsub(np, NULL, 0, ARRAY_SCAN);
                        if (!ap->fun && !(ap->flags & ARRAY_TREE) && !np->nvfun->next &&
                            !nv_type(np)) {
                            nvflag_t nvflag = np->nvflag;
                            uint32_t nvsize = np->nvsize;
                            _nv_unset(np, NV_EXPORT);
                            nv_setattr(np, nvflag);
                            np->nvsize = nvsize;
                        } else {
                            ap->nelem++;
                            while (1) {
                                ap->flags &= ~ARRAY_SCAN;
                                _nv_unset(np, NV_EXPORT);
                                ap->flags |= ARRAY_SCAN;
                                if (!nv_nextsub(np)) break;
                            }
                            ap->nelem--;
                        }
                    } else if (nv_isattr(np, NV_BINARY | NV_NOFREE | NV_RAW) !=
                                   (NV_BINARY | NV_NOFREE | NV_RAW) &&
                               !nv_isarray(np)) {
                        _nv_unset(np, NV_EXPORT);
                    }
                    goto skip;
                }
                if (tp->tre.tretyp == TLST || !tp->com.comset || tp->com.comset->argval[0] != '[') {
                    if (tp->tre.tretyp != TLST && !tp->com.comnamp && tp->com.comset &&
                        tp->com.comset->argval[0] == 0 && tp->com.comset->argchn.ap) {
                        if (prefix || nv_name(np)) cp = stkcopy(shp->stk, nv_name(np));
                        shp->prefix = cp;
                        if (tp->com.comset->argval[1] == '[') {
                            if ((arg->argflag & ARG_APPEND) &&
                                (!nv_isarray(np) || (nv_aindex(np) >= 0))) {
                                _nv_unset(np, 0);
                            }
                            if (!(array & NV_IARRAY) && !(tp->com.comset->argflag & ARG_MESSAGE)) {
                                nv_setarray(np, nv_associative);
                            }
                        }
                        shp->typeinit = NULL;
                        sh_setlist(shp, tp->com.comset, flags & ~NV_STATIC, 0);
                        shp->prefix = prefix;
                        if (tp->com.comset->argval[1] != '[') nv_setvtree(np);
                        nv_close(np);
                        goto check_type;
                    }
                    nq = np;
                    if (*cp != '.' && *cp != '[' && strchr(cp, '[')) {
                        cp = stkcopy(shp->stk, nv_name(np));
                        nv_close(np);
                        if (!(arg->argflag & ARG_APPEND)) flag &= ~NV_ARRAY;
                        shp->prefix_root = shp->first_root;
                        np = nv_open(cp, shp->prefix_root ? shp->prefix_root : shp->var_tree, flag);
                    }
                    if (arg->argflag & ARG_APPEND) {
                        if (nv_isarray(np)) {
                            if ((sub = nv_aimax(np)) < 0 && nv_arrayptr(np)) {
                                errormsg(SH_DICT, ERROR_exit(1), e_badappend, nv_name(np));
                                __builtin_unreachable();
                            }
                            if (sub == 0 && nv_type(np) && (ap = nv_arrayptr(np)) &&
                                array_elem(ap) == 0) {
                                nv_putsub(np, NULL, 0, ARRAY_ADD | ARRAY_FILL);
                            } else if (sub >= 0) {
                                sub++;
                            }
                            if (nv_type(np)) {
                                sfprintf(shp->strbuf, "%s[%d]\0", nv_name(np), sub);
                                nq =
                                    nv_open(sfstruse(shp->strbuf), shp->var_tree, flags | NV_ARRAY);
                            }
                        }
                        if (!nv_isnull(np) && FETCH_VT(np->nvalue, const_cp) != Empty &&
                            !nv_isvtree(np)) {
                            sub = 1;
                        }
                    } else if (((FETCH_VT(np->nvalue, const_cp) &&
                                 FETCH_VT(np->nvalue, const_cp) != Empty) ||
                                nv_isvtree(np) || nv_arrayptr(np)) &&
                               !nv_type(np) &&
                               nv_isattr(np, NV_MINIMAL | NV_EXPORT) != NV_MINIMAL) {
                        bool was_assoc_array = ap && ap->fun;
                        _nv_unset(np, NV_EXPORT);  // this can free ap
                        if (was_assoc_array) {
                            nv_setarray(np, nv_associative);
                        } else {
                            // nq is initialized to same value as np. When _nv_unset(np, NV_EXPORT);
                            // is called, it free's memory which is later causing crash at:
                            // if (nq && nv_type(nq)) nv_checkrequired(nq);
                            // Reset nq to 0 to avoid such crashes.
                            nq = NULL;
                        }
                    }
                } else {
                    if (!(arg->argflag & ARG_APPEND)) _nv_unset(np, NV_EXPORT);
                    if (!(array & NV_IARRAY) && !nv_isarray(np)) nv_setarray(np, nv_associative);
                }
            skip:
                if (sub > 0) {
                    sfprintf(stkstd, "%s[%d]", prefix ? nv_name(np) : cp, sub);
                    shp->prefix = stkfreeze(shp->stk, 1);
                    nv_putsub(np, NULL, sub, ARRAY_ADD | ARRAY_FILL);
                } else if (prefix) {
                    shp->prefix = stkcopy(shp->stk, nv_name(np));
                } else {
                    shp->prefix = cp;
                }
                shp->last_table = NULL;
                if (shp->prefix) {
                    if (*shp->prefix == '_' && shp->prefix[1] == '.' && nv_isref(VAR_underscore)) {
                        sfprintf(stkstd, "%s%s", nv_name(FETCH_VT(VAR_underscore->nvalue, nrp)->np),
                                 shp->prefix + 1);
                        shp->prefix = stkfreeze(stkstd, 1);
                    }
                    memset(&nr, 0, sizeof(nr));
                    memcpy(&node, VAR_underscore, sizeof(node));
                    STORE_VT(VAR_underscore->nvalue, nrp, &nr);
                    nr.np = np;
                    nr.root = shp->last_root;
                    nr.table = shp->last_table;
                    nv_setattr(VAR_underscore, NV_REF | NV_NOFREE);
                    VAR_underscore->nvfun = NULL;
                }
                sh_exec(shp, tp, sh_isstate(shp, SH_ERREXIT));
                if (nq && nv_type(nq)) nv_checkrequired(nq);
                if (shp->prefix) {
                    STORE_VT(VAR_underscore->nvalue, nrp, FETCH_VT(node.nvalue, nrp));
                    nv_setattr(VAR_underscore, node.nvflag);
                    VAR_underscore->nvfun = node.nvfun;
                }
                shp->prefix = prefix;
                if (nv_isarray(np) && (mp = nv_opensub(np))) np = mp;
                while (tp->tre.tretyp == TLST) {
                    if (!tp->lst.lstlef || tp->lst.lstlef->tre.tretyp != TCOM ||
                        tp->lst.lstlef->com.comarg ||
                        (tp->lst.lstlef->com.comset &&
                         tp->lst.lstlef->com.comset->argval[0] != '[')) {
                        break;
                    }
                    tp = tp->lst.lstrit;
                }
                if (!nv_isarray(np) && !typ &&
                    (tp->com.comarg || !tp->com.comset || tp->com.comset->argval[0] != '[')) {
                    nv_setvtree(np);
                    if (tp->com.comarg || tp->com.comset) np->nvfun->dsize = 0;
                }
                goto check_type;
            }
            cp = arg->argval;
            mp = NULL;
        }
        np = nv_open(cp, shp->prefix_root ? shp->prefix_root : shp->var_tree, flags);
        if (nlp) *nlp++ = np;
        if (!np->nvfun && (flags & NV_NOREF)) {
            if (shp->used_pos) {
                nv_onattr(np, NV_PARAM);
            } else {
                nv_offattr(np, NV_PARAM);
            }
        }
        if (traceon || trap) {
            char *sp = cp;
            char *name = nv_name(np);
            char *sub = NULL;
            int append = 0;
            if (nv_isarray(np)) sub = savesub;
            cp = lastdot(sp, '=', shp);
            if (cp) {
                if (cp[-1] == '+') append = ARG_APPEND;
                cp++;
            }
            if (traceon) {
                sh_trace(shp, NULL, 0);
                sh_outname(shp, sfstderr, name, -1);
                if (sub) sfprintf(sfstderr, "[%s]", sh_fmtq(sub));
                if (cp) {
                    if (append) sfputc(sfstderr, '+');
                    sfprintf(sfstderr, "=%s\n", sh_fmtq(cp));
                }
            }
            if (trap) {
                char *av[2];
                av[0] = cp;
                av[1] = 0;
                sh_debug(shp, trap, name, sub, av, append);
            }
        }
    check_type:
        if (maketype) {
            nv_open(shtp.nodes[0]->nvname, shp->var_tree,
                    NV_ASSIGN | NV_VARNAME | NV_NOADD | NV_NOFAIL);
            np = nv_mktype(shtp.nodes, shtp.numnodes);
            free(shtp.nodes);
            shp->mktype = shtp.previous;
            maketype = false;
            if (shp->namespace) free(shp->prefix);
            shp->prefix = NULL;
            if (nr.np == np) {
                STORE_VT(VAR_underscore->nvalue, nrp, FETCH_VT(node.nvalue, nrp));
                nv_setattr(VAR_underscore, node.nvflag);
                VAR_underscore->nvfun = node.nvfun;
            }
        }
    }
    return nodelist;
}

//
// Copy the subscript onto the stack.
//
static_fn void stak_subscript(const char *sub, int last) {
    int c;
    sfputc(stkstd, '[');
    while ((c = *sub++)) {
        if (c == '[' || c == ']' || c == '\\') sfputc(stkstd, '\\');
        sfputc(stkstd, c);
    }
    sfputc(stkstd, last);
}

//
// Construct a new name from a prefix and base name on the stack.
//
static_fn char *copystack(Shell_t *shp, const char *prefix, const char *name, const char *sub) {
    int last = 0, offset = stktell(shp->stk);
    if (prefix) {
        sfputr(shp->stk, prefix, -1);
        if (*stkptr(shp->stk, stktell(shp->stk) - 1) == '.') {
            stkseek(shp->stk, stktell(shp->stk) - 1);
        }
        if (*name == '.' && name[1] == '[') last = stktell(shp->stk) + 2;
        if (*name != '[' && *name != '.' && *name != '=' && *name != '+') sfputc(shp->stk, '.');
        if (*name == '.' && (name[1] == '=' || name[1] == 0)) sfputc(shp->stk, '.');
    }
    if (last) {
        sfputr(shp->stk, name, -1);
        if (sh_checkid(stkptr(shp->stk, last), NULL)) stkseek(shp->stk, stktell(shp->stk) - 2);
    }
    if (sub) stak_subscript(sub, ']');
    if (!last) sfputr(shp->stk, name, 0);
    return stkptr(shp->stk, offset);
}

//
// Grow this stack string <name> by <n> bytes and move from cp-1 to end right
// by <n>.  Returns beginning of string on the stack.
//
static_fn char *stack_extend(Shell_t *shp, const char *cname, char *cp, int n) {
    char *name = (char *)cname;
    int offset = name - stkptr(shp->stk, 0);
    int m = cp - name;

    stkseek(shp->stk, offset + strlen(name) + n + 1);
    name = stkptr(shp->stk, offset);
    cp = name + m;
    m = strlen(cp) + 1;
    while (m-- > 0) cp[n + m] = cp[m];
    return name;
}

Namval_t *nv_create(const char *name, Dt_t *root, nvflag_t flags, Namfun_t *dp) {
    Shell_t *shp = sh_getinterp();
    char *sub = NULL;
    char *cp = (char *)name;
    Namval_t *np = NULL;
    Namval_t *nq = NULL;
    Namfun_t *fp = NULL;
    Namarr_t *ap = NULL;
    Namval_t *qp = NULL;
    long add = 0;
    int copy = 0;
    int top = 0;
    bool noscope = nv_isflag(flags, NV_NOSCOPE);
    nvflag_t nofree = 0;
    int level = 0;
    int zerosub = 0;
    char *sp, *xp;
    int c, n;
    long mode;
    int isref;

    nv_isvalid(flags);
    if (root == shp->var_tree) {
        if (dtvnext(root)) {
            top = 1;
        } else {
            flags &= ~NV_NOSCOPE;
        }
    }
    if (!dp->disc) copy = dp->nofree & 1;
    if (*cp == '.') cp++;
    while (true) {
        if (zerosub) {
            assert(np || sp);
            if (!np) memcpy(sp, cp - 1, strlen(cp - 1) + 1);
            zerosub = 0;
        }
        sp = cp;
        switch (c = *(unsigned char *)sp) {
            case '[': {
                if (flags & NV_NOARRAY) {
                    dp->last = cp;
                    return np;
                }
                cp = nv_endsubscript(NULL, sp, 0, shp);
                if (sp == name || sp[-1] == '.') c = *(sp = cp);
                goto skip;
            }
            case '.': {
                if (flags & NV_IDENT) return 0;
                if (root == shp->var_tree) flags &= ~NV_EXPORT;
                if (!copy && !(flags & NV_NOREF)) {
                    c = sp - name;
                    copy = cp - name;
                    dp->nofree |= 1;
                    name = copystack(shp, NULL, name, NULL);
                    cp = (char *)name + copy;
                    sp = (char *)name + c;
                    c = '.';
                }
            }
            // FALLTHRU
            case '+':
            case '=': {
            skip:
                *sp = 0;
            }
            // FALLTHRU
            case 0: {
                isref = 0;
                dp->last = cp;
                mode = (c == '.' || (flags & NV_NOADD)) ? add : NV_ADD;
                if (level++ || ((flags & NV_NOSCOPE) && c != '.')) mode |= NV_NOSCOPE;
                np = NULL;
                if (top) {
                    struct Ufunction *rp;
                    if ((rp = shp->st.real_fun) && !rp->sdict && (flags & NV_STATIC)) {
                        Dt_t *dp = dtview(shp->var_tree, NULL);
                        rp->sdict = dtopen(&_Nvdisc, Dtoset);
                        dtuserdata(rp->sdict, shp, 1);
                        dtview(rp->sdict, dp);
                        dtview(shp->var_tree, rp->sdict);
                    }
                    np = nv_search(name, shp->var_tree, 0);
                    if (np) {
                        if (shp->var_tree->walk == shp->var_base ||
                            (shp->var_tree->walk != shp->var_tree && shp->namespace &&
                             nv_dict(shp->namespace) == shp->var_tree->walk)) {
                            if (!(nq = nv_search_namval(np, shp->var_base, 0))) nq = np;
                            shp->last_root = shp->var_tree->walk;
                            if ((flags & NV_NOSCOPE) && *cp != '.') {
                                if (mode == 0) {
                                    root = shp->var_tree->walk;
                                } else {
                                    nv_delete(np, NULL, NV_NOFREE);
                                    np = NULL;
                                }
                            }
                        } else {
                            if (shp->var_tree->walk) root = shp->var_tree->walk;
                            flags |= NV_NOSCOPE;
                            noscope = true;
                        }
                    }
                    if (rp && rp->sdict && (flags & NV_STATIC)) {
                        root = rp->sdict;
                        if (np && shp->var_tree->walk == shp->var_tree) {
                            _nv_unset(np, 0);
                            nv_delete(np, shp->var_tree, 0);
                            np = NULL;
                        }
                        if (!np || shp->var_tree->walk != root) {
                            np = nv_search(name, root, NV_NOSCOPE | NV_ADD);
                        }
                    }
                }
                if (!np && !noscope && *name != '.' && shp->namespace && root == shp->var_tree) {
                    root = nv_dict(shp->namespace);
                }
                if (np || (np = nv_search(name, root, mode))) {
                    isref = nv_isref(np);
                    shp->openmatch = root->walk ? root->walk : root;
                    if (top) {
                        if (nq == np) {
                            flags &= ~NV_NOSCOPE;
                            root = shp->last_root;
                        } else if (nq) {
                            if (nv_isnull(np) && c != '.' &&
                                ((np->nvfun = nv_cover(nq)) || nq == VAR_OPTIND)) {
                                np->nvname = nq->nvname;
                                if (shp->namespace && nv_dict(shp->namespace) == shp->var_tree &&
                                    nv_isattr(nq, NV_EXPORT)) {
                                    nv_onattr(np, NV_EXPORT);
                                }
                                if (nq == VAR_OPTIND) {
                                    np->nvfun = nq->nvfun;
                                    STORE_VT(np->nvalue, i32p, &shp->st.optindex);
                                    nv_onattr(np, NV_INTEGER | NV_NOFREE);
                                }
                            }
                            flags |= NV_NOSCOPE;
                        }
                    } else if (add && nv_isnull(np) && c == '.' && cp[1] != '.') {
                        nv_setvtree(np);
                    }
                    if (shp->namespace && root == nv_dict(shp->namespace)) {
                        flags |= NV_NOSCOPE;
                        shp->last_table = shp->namespace;
                    }
                }
                if (c) *sp = c;
                top = 0;
                if (np && !nv_isattr(np, NV_MINIMAL) && shp->oldnp && !np->nvenv &&
                    shp->oldnp != np && !(flags & NV_ARRAY)) {
                    np->nvenv = shp->oldnp;
                }
                shp->oldnp = np;
                if (isref) {
                    nvcache.ok = 0;
                    if (nv_isattr(np, NV_TABLE) || c == '.') {  // don't optimize
                        shp->argaddr = NULL;
                    } else if ((flags & NV_NOREF) && (c != '[' && *cp != '.')) {
                        if (c && !(flags & NV_NOADD)) nv_unref(np);
                        return np;
                    }
                    while (nv_isref(np) && FETCH_VT(np->nvalue, const_cp)) {
                        root = nv_reftree(np);
                        shp->last_root = root;
                        shp->last_table = nv_reftable(np);
                        sub = nv_refsub(np);
                        shp->oldnp = nv_refoldnp(np);
                        if (shp->oldnp) shp->oldnp = shp->oldnp->nvenv;
                        np = nv_refnode(np);
                        if (sub && c != '.') nv_putsub(np, sub, 0L, 0);
                        flags |= NV_NOSCOPE;
                        noscope = true;
                    }
                    shp->first_root = root;
                    if (nv_isref(np) && (c == '[' || c == '.' || !(flags & NV_ASSIGN))) {
                        errormsg(SH_DICT, ERROR_exit(1), e_noref, nv_name(np));
                        __builtin_unreachable();
                    }
                    if (sub && c == 0) {
                        if (flags & NV_ARRAY) {
                            nq = nv_opensub(np);
                            if ((flags & NV_ASSIGN) && (!nq || nv_isnull(nq))) {
                                ap = nv_arrayptr(np);
                                assert(ap);
                                ap->nelem++;
                            }
                            if (!nq) {
                                goto addsub;
                            } else {
                                np = nq;
                            }
                        }
                        return np;
                    }
                    if (np == nq) {
                        flags &= ~(noscope ? 0 : NV_NOSCOPE);
                    } else if (c) {
                        char *xp = NULL;
                        size_t xlen;
                        c = (cp - sp);
                        // Eliminate namespace name.
                        if (shp->last_table && !nv_type(shp->last_table)) {
                            xp = nv_name(shp->last_table);
                            xlen = strlen(xp);
                        }
                        cp = nv_name(np);
                        copy = strlen(cp);
                        if (xp && copy > xlen && !strncmp(cp, xp, xlen) && cp[xlen] == '.') {
                            cp += xlen + 1;
                        }
                        dp->nofree |= 1;
                        name = copystack(shp, cp, sp, sub);
                        sp = (char *)name + copy;
                        cp = sp + c;
                        c = *sp;
                        if (!noscope) flags &= ~NV_NOSCOPE;
                    }
                    flags |= NV_NOREF;
                    if (*cp == 0 && nv_isnull(np) && !nv_isarray(np)) nofree = NV_NOFREE;
                }
                shp->last_root = root;
                if (*cp && cp[1] == '.') cp++;
                if (c == '.' && (cp[1] == 0 || cp[1] == '=' || cp[1] == '+')) {
                    nv_local = true;
                    if (np) nv_onattr(np, nofree);
                    return np;
                }
                if (cp[-1] == '.') cp--;
                do {
                    if (!np) {
                        if (!nq && !shp->namref_root && *sp == '[' && *cp == 0 && cp[-1] == ']') {
                            // For backward compatibility evaluate subscript for possible side
                            // effects.
                            cp[-1] = 0;
                            sh_arith(shp, sp + 1);
                            cp[-1] = ']';
                        }
                        return np;
                    }
                    ap = nv_arrayptr(np);
                    if (c == '[' || (c == '.' && nv_isarray(np))) {
                        nvflag_t nvflags = 0;
                        sub = NULL;
                        mode &= ~NV_NOSCOPE;
                        if (c == '[') {
                            Namval_t *table;
                            nvflags = mode;
                            if (nv_isarray(np)) nvflags |= NV_ARRAY;
                            if (!mode && nv_isflag(flags, NV_ARRAY) &&
                                ((c = sp[1]) == '*' || c == '@') && sp[2] == ']') {
                                // Not implemented yet.
                                dp->last = cp;
                                return np;
                            }
                            if (nv_isflag(nvflags, NV_ADD) && nv_isflag(flags, NV_ARRAY)) {
                                nvflags |= ARRAY_FILL;
                            }
                            if (nv_isflag(flags, NV_ASSIGN)) nvflags |= NV_ADD | ARRAY_FILL;
                            table = shp->last_table;
                            if (nv_isflag(flags, NV_ASSIGN)) nvflags |= NV_ASSIGN;
                            cp = nv_endsubscript(np, sp, nvflags, np->nvshell);
                            ap = nv_arrayptr(np);  // nv_endsubscript() may have moved the array
                            shp->last_table = table;
                        } else {
                            cp = sp;
                        }
                        if ((c = *cp) == '.' || (c == '[' && nv_isarray(np)) ||
                            nv_isflag(nvflags, ARRAY_FILL) ||
                            ((ap || nv_isflag(flags, NV_ASSIGN)) && nv_isflag(flags, NV_ARRAY))) {
                            int m = cp - sp;
                            sub = m ? nv_getsub(np) : 0;
                            if (!sub) {
                                if (m && !nv_isflag(nvflags, NV_ADD) && *cp != '.') return 0;
                                zerosub = 1;
                                sub = "0";
                            }
                            n = strlen(sub) + 2;
                            if (!copy) {
                                copy = cp - name;
                                dp->nofree |= 1;
                                name = copystack(shp, NULL, name, NULL);
                                cp = (char *)name + copy;
                                sp = cp - m;
                            }
                            if (n <= m) {
                                if (n) {
                                    memcpy(sp + 1, sub, n - 2);
                                    sp[n - 1] = ']';
                                }
                                if (n < m) {
                                    char *dp = sp + n;
                                    while ((*dp++ = *cp++)) {
                                        ;  // empty loop
                                    }
                                    cp = sp + n;
                                }
                            } else {
                                int r = n - m;
                                m = sp - name;
                                name = stack_extend(shp, name, cp - 1, r);
                                sp = (char *)name + m;
                                *sp = '[';
                                memcpy(sp + 1, sub, n - 2);
                                sp[n - 1] = ']';
                                cp = sp + n;
                            }
                        } else if (c == 0 && mode && (n = nv_aindex(np)) >= 0) {
                            nv_putsub(np, NULL, n, 0);
                        } else if (nvflags == 0 && (c == 0 || (c == '[' && !nv_isarray(np)))) {
                            // Subscript must be 0.
                            cp[-1] = 0;
                            n = sh_arith(shp, sp + 1);
                            cp[-1] = ']';
                            if (n) return 0;
                            if (c) sp = cp;
                        }
                        dp->last = cp;
                        if (nv_isarray(np) && (c == '[' || c == '.' || (flags & NV_ARRAY))) {
                            Namval_t *tp;
                        addsub:
                            sp = cp;
                            nq = NULL;
                            tp = nv_type(np);
                            if (tp && nv_hasdisc(np, &ENUM_disc)) goto enumfix;
                            if (ap && ap->table && tp) {
                                // Coverity Scan CID#340996 points out that at this juncture it
                                // should be impossible for `sub` to be NULL but there is a
                                // theoretical route here where it is NULL. So assert that
                                // requirement.
                                assert(sub);
                                nq = nv_search(sub, ap->table, 0);
                            }
                            if (!nq) nq = nv_opensub(np);
                            if (!nq) {
                                Namarr_t *ap = nv_arrayptr(np);
                                if (!sub && (flags & NV_NOADD)) return 0;
                                nvflags = mode;
                                if (!nv_isflag(flags, NV_NOADD)) nvflags |= NV_ADD;
                                if (!nv_isflag(nvflags, NV_ADD) && ap && tp) nvflags |= NV_ADD;

                                if (!ap && nv_isflag(nvflags, NV_ADD)) {
                                    nv_putsub(np, sub, 0, ARRAY_FILL);
                                    ap = nv_arrayptr(np);
                                }
                                if (nvflags && ap && !ap->table) {
                                    ap->table = dtopen(&_Nvdisc, Dtoset);
                                    dtuserdata(ap->table, shp, 1);
                                }
                                if (ap && ap->table) {
                                    assert(sub);  // Coverity Scan CID#340996
                                    nq = nv_search(sub, ap->table, nvflags);
                                    if (nq) nq->nvenv = np;
                                }
                                if (nq && nv_isnull(nq)) nq = nv_arraychild(np, nq, c);
                            }
                            if (nq) {
                                if (c == '.' && !nv_isvtree(nq)) {
                                    if (flags & NV_NOADD) return 0;
                                    nv_setvtree(nq);
                                }
                                nv_onattr(np, nofree);
                                nofree = 0;
                                np = nq;
                            } else if (strncmp(cp, "[0]", 3)) {
                                return nq;
                            } else {
                                // Ignore [0].
                                dp->last = cp += 3;
                                c = *cp;
                            }
                        }
                    } else if (nv_isarray(np)) {
                        if (c == 0 && (flags & NV_MOVE)) return np;
                        nv_putsub(np, NULL, 0, ARRAY_UNDEF);
                    }
                    nv_onattr(np, nofree);
                    nofree = 0;
                    qp = np;

                enumfix:
                    if (c == '.' && (fp = np->nvfun)) {
                        for (; fp; fp = fp->next) {
                            if (fp->disc && fp->disc->createf) break;
                        }
                        if (fp) {
                            if ((nq = (*fp->disc->createf)(np, cp + 1, flags, fp)) == np) {
                                add = NV_ADD;
                                shp->last_table = NULL;
                                break;
                            } else if ((np = nq)) {
                                if ((c = *(sp = cp = dp->last = fp->last)) == 0) {
                                    if (nv_isarray(np) && sp[-1] != ']') {
                                        nv_putsub(np, NULL, 0, ARRAY_UNDEF);
                                    }
                                    return np;
                                }
                                goto enumfix;
                            }
                        }
                    }
                } while (c == '[');
                if (c != '.' || cp[1] == '.') return np;
                cp++;
                break;
            }
            default: {
                shp->oldnp = np ? nq : qp;
                qp = NULL;
                dp->last = cp;
                if ((c = mb1char(&cp)) && !isaletter(c)) return np;
                while (xp = cp, c = mb1char(&cp), isaname(c)) {
                    ;  // empty loop
                }
                cp = xp;
            }
        }
    }
}

//
// Delete the node <np> from the dictionary <root> and clear from the cache.
// If <root> is NULL, only the cache is cleared.
// If flags does not contain NV_NOFREE, the node is freed.
// If np==0  && !root && flags==0,  delete the Refdict dictionary.
//
void nv_delete(Namval_t *np, Dt_t *root, nvflag_t flags) {
    int c;
    struct Cache_entry *xp;

    nv_isvalid(flags);
    for (c = 0, xp = nvcache.entries; c < NVCACHE; xp = &nvcache.entries[++c]) {
        if (xp->np == np) xp->root = NULL;
    }
    if (!np && !root && flags == 0) {
        if (Refdict) dtclose(Refdict);
        Refdict = 0;
        return;
    }
    if (root || !(flags & NV_NOFREE)) {
        if (!(flags & NV_FUNCTION) && Refdict) {
            Namval_t **key = &np;
            struct Namref *rp;
            while ((rp = (struct Namref *)dtmatch(Refdict, key))) {
                if (rp->sub) free(rp->sub);
                rp->sub = NULL;
                rp = dtremove(Refdict, rp);
                if (rp) rp->np = &NullNode;
            }
        }
    }
    if (root) {
        if (dtdelete(root, np)) {
            if (!nv_isflag(flags, NV_NOFREE) &&
                (nv_isflag(flags, NV_FUNCTION) || !nv_subsaved(np, nv_isflag(flags, NV_TABLE)))) {
                Namarr_t *ap;
                assert(np);
                if (nv_isarray(np) && np->nvfun && (ap = nv_arrayptr(np)) && is_associative(ap)) {
                    while (nv_associative(np, 0, ASSOC_OP_NEXT)) {
                        nv_associative(np, 0, ASSOC_OP_DELETE);
                    }
                    nv_associative(np, 0, ASSOC_OP_FREE);
                    free(np->nvfun);
                }
                free(np);
            }
        }
    }
}

//
// Put <arg> into associative memory.
// If <flags> & NV_ARRAY then follow array to next subscript.
// If <flags> & NV_NOARRAY then subscript is not allowed.
// If <flags> & NV_NOSCOPE then use the current scope only.
// If <flags> & NV_ASSIGN then assignment is allowed.
// If <flags> & NV_IDENT then name must be an identifier.
// If <flags> & NV_VARNAME then name must be a valid variable name.
// If <flags> & NV_NOADD then node will not be added if not found.
// If <flags> & NV_NOREF then don't follow reference.
// If <flags> & NV_NOFAIL then don't generate an error message on failure.
// If <flags> & NV_STATIC then unset before an assignment.
// If <flags> & NV_UNJUST then unset attributes before assignment.
// SH_INIT is only set while initializing the environment.
//
Namval_t *nv_open(const char *name, Dt_t *root, nvflag_t flags) {
    Shell_t *shp = sh_getinterp();
    char *cp = (char *)name;
    int c;
    Namval_t *np = NULL;
    Namfun_t fun;
    nvflag_t append_nvflags = 0;
    const char *msg = e_varname;
    char *fname = NULL;
    int offset = stktell(shp->stk);
    Dt_t *funroot;
    struct Cache_entry *xp;

    nv_isvalid(flags);
    // It's not clear why these two assignments are required before the check for non-empty name.
    shp->openmatch = NULL;
    shp->last_table = NULL;
    if (!name || !*name) return NULL;

    memset(&fun, 0, sizeof(fun));
    sh_stats(STAT_NVOPEN);
    if (!root) root = shp->var_tree;
    shp->last_root = root;
    if (root == shp->fun_tree) {
        flags |= NV_NOREF;
        msg = e_badfun;
        if (strchr(name, '.')) {
            name = cp = copystack(shp, 0, name, NULL);
            fname = strrchr(cp, '.');
            *fname = 0;
            fun.nofree |= 1;
            flags &= ~NV_IDENT;
            funroot = root;
            root = shp->var_tree;
        }
    } else if (!(flags & (NV_IDENT | NV_VARNAME | NV_ASSIGN))) {
        nvflag_t mode = ((flags & NV_NOADD) ? 0 : NV_ADD);
        if (flags & NV_NOSCOPE) mode |= NV_NOSCOPE;
        np = nv_search(name, root, mode);
        if (np && !(flags & NV_REF)) {
            while (nv_isref(np)) {
                shp->last_table = nv_reftable(np);
                np = nv_refnode(np);
            }
        }
        return np;
    } else if (shp->prefix && (flags & NV_ASSIGN)) {
        name = cp = copystack(shp, shp->prefix, name, NULL);
        fun.nofree |= 1;
    }
    c = *(unsigned char *)cp;
    if (root == shp->alias_tree) {
        msg = e_aliname;
        while ((c = *(unsigned char *)cp++) && (c != '=') && (c != '/') &&
               (c >= 0x200 || !(c = sh_lexstates[ST_NORM][c]) || c == S_EPAT || c == S_COLON)) {
            ;  // empty loop
        }
        if (shp->subshell && c == '=') root = sh_subaliastree(shp, 1);
        c = *--cp;
        if (c) *cp = 0;
        np = nv_search(name, root, (flags & NV_NOADD) ? 0 : NV_ADD);
        if (c) *cp = c;
        goto skip;
    } else if (flags & NV_IDENT) {
        msg = e_ident;
    } else if (c == '.') {
        c = *++cp;
        flags |= NV_NOREF;
        if (root == shp->var_tree) root = shp->var_base;
        shp->last_table = NULL;
    }
    c = !isaletter(c);
    if (c) goto skip;
    for (c = 0, xp = nvcache.entries; c < NVCACHE; xp = &nvcache.entries[++c]) {
        if (xp->root != root) continue;
        if ((*name != '_' || name[1] != '.') && *name == *xp->name &&
            xp->namespace == shp->namespace && (flags & (NV_ARRAY | NV_NOSCOPE)) == xp->flags &&
            strncmp(xp->name, name, xp->len) == 0 &&
            (name[xp->len] == 0 || name[xp->len] == '=' || name[xp->len] == '+')) {
            sh_stats(STAT_NVHITS);
            np = xp->np;
            cp = (char *)name + xp->len;
            if (nv_isarray(np) && !(flags & NV_MOVE)) nv_putsub(np, NULL, 0, ARRAY_UNDEF);
            shp->last_table = xp->last_table;
            shp->last_root = xp->last_root;
            goto nocache;
        }
    }
    nvcache.ok = 1;
#if SHOPT_BASH
    if (root == shp->fun_tree && sh_isoption(shp, SH_BASH)) {
        nvflag_t nvflags = 0;
        if (nv_isflag(flags, NV_NOSCOPE)) nvflags |= NV_NOSCOPE;
        if (!nv_isflag(flags & NV_NOADD)) nvflags |= NV_ADD;
        np = nv_search(name, root, nvflags);
        cp = Empty;
    } else
#endif  // SHOPT_BASH
    {
        np = nv_create(name, root, flags, &fun);
        cp = fun.last;
    }
    if (np && nvcache.ok && cp[-1] != ']') {
        xp = &nvcache.entries[nvcache.index];
        if (*cp) {
            char *sp = strchr(name, *cp);
            if (!sp) goto nocache;
            xp->len = sp - name;
        } else {
            xp->len = strlen(name);
        }
        c = roundof(xp->len + 1, 32);
        if (c > xp->size) {
            if (xp->size == 0) {
                xp->name = malloc(c);
            } else {
                xp->name = realloc(xp->name, c);
            }
            xp->size = c;
        }
        memcpy(xp->name, name, xp->len);
        xp->name[xp->len] = 0;
        xp->root = root;
        xp->np = np;
        xp->namespace = shp->namespace;
        xp->last_table = shp->last_table;
        xp->last_root = shp->last_root;
        xp->flags = (flags & (NV_ARRAY | NV_NOSCOPE));
        nvcache.index = (nvcache.index + 1) & (NVCACHE - 1);
    }
nocache:
    nvcache.ok = 0;
    if (fname) {
        c = ((flags & NV_NOSCOPE) ? NV_NOSCOPE : 0) | ((flags & NV_NOADD) ? 0 : NV_ADD);
        *fname = '.';
        np = nv_search(name, funroot, c);
        *fname = 0;
    } else {
        if (*cp == '.' && cp[1] == '.') {
            append_nvflags |= NV_NODISC;
            cp += 2;
        }
        if (*cp == '+' && cp[1] == '=') {
            append_nvflags |= NV_APPEND;
            cp++;
        }
    }
    c = *cp;
skip:
    if (np && shp->mktype) np = nv_addnode(np, 0);
    if (np && c == '=' && (flags & NV_ASSIGN)) {
        cp++;
        if (sh_isstate(shp, SH_INIT)) {
            nv_putval(np, cp, NV_RDONLY);
            if (np == VAR_PWD) nv_onattr(np, NV_TAGGED);
        } else {
            char *sub = NULL;
            char *prefix = shp->prefix;
            Namval_t *mp;
            Namarr_t *ap;
            int isref;
            shp->prefix = NULL;
            if ((flags & NV_STATIC) && !shp->mktype && !nv_isnull(np)) {
                shp->prefix = prefix;
                return np;
            }
            isref = nv_isref(np);
            if (sh_isoption(shp, SH_XTRACE) && nv_isarray(np)) sub = nv_getsub(np);
            nvflag_t nvflags = msg == e_aliname ? 0 : (append_nvflags | (flags & NV_EXPORT));
            if (isref) nv_offattr(np, NV_REF);
            if (!append_nvflags && (flags & NV_UNJUST) && !np->nvfun) _nv_unset(np, NV_EXPORT);
            if (nv_isflag(flags, NV_MOVE)) {
                ap = nv_arrayptr(np);
                if (ap) {
                    mp = nv_opensub(np);
                    if (mp) {
                        np = mp;
                    } else if (!is_associative(ap) &&
                               (mp = nv_open(cp, shp->var_tree,
                                             NV_NOFAIL | NV_VARNAME | NV_NOARRAY | NV_NOADD)) &&
                               nv_isvtree(np)) {
                        ap->flags |= ARRAY_TREE;
                        nv_putsub(np, NULL, nv_aindex(np), ARRAY_ADD);
                        np = nv_opensub(np);
                        assert(np);
                        ap->flags &= ~ARRAY_TREE;
                    }
                }
                _nv_unset(np, NV_EXPORT);
                STORE_VT(np->nvalue, const_cp, strdup(cp));
            } else {
                nv_putval(np, cp, nvflags);
            }
            if (isref) {
                if (nv_search_namval(np, shp->var_base, 0)) {
                    shp->last_root = shp->var_base;
                }
                nv_setref(np, NULL, NV_VARNAME);
            }
            savesub = sub;
            shp->prefix = prefix;
        }
        // See https://github.com/att/ast/issues/1038 for why this mask op exists. Note that
        // NV_ARRAY never occurs AFAICT but it was in the original version so retain it for
        // now out of an abundance of caution.
        //
        // TODO: Pass `flags` as-is as soon as the aliasing of these symbols is eliminated.
        nv_onattr(np, flags & ~(NV_ARRAY | NV_IDENT | NV_REF));
    } else if (c) {
        if (flags & NV_NOFAIL) return 0;
        if (c == '.') {
            msg = e_noparent;
        } else if (c == '[') {
            msg = e_noarray;
        }
        errormsg(SH_DICT, ERROR_exit(1), msg, name);
        __builtin_unreachable();
    }
    if (fun.nofree & 1) stkseek(shp->stk, offset);
    return np;
}

static_fn int ja_size(char *, int, bool);
static_fn void ja_restore(void);
static char *savep;
static char savechars[8 + 1];

//
// Put value <string> into name-value node <np>.
// If <np> is an array, then the element given by the current index is assigned to.
// If <flags> contains NV_RDONLY, readonly attribute is ignored.
// If <flags> contains NV_INTEGER, string is a pointer to a number.
// If <flags> contains NV_NOFREE, previous value is freed, and <string>
// becomes value of node and <flags> becomes attributes.
//
void nv_putval(Namval_t *np, const void *vp, nvflag_t flags) {
    Shell_t *shp = sh_ptr(np);
    const char *sp = vp;
    char *cp;
    struct Value *up;
    int size = 0;
    int dot = INT_MAX;  // make sure if used before set bad things happen
    bool was_local = nv_local;

    nv_isvalid(flags);
    if ((flags & NV_APPEND) && nv_isnull(np) && shp->var_tree->view) {
        Namval_t *mp = nv_search(np->nvname, shp->var_tree->view, 0);
        if (mp) nv_clone(mp, np, 0);
    }
    if (!(flags & NV_RDONLY) && nv_isattr(np, NV_RDONLY) &&
        FETCH_VT(np->nvalue, const_cp) != Empty) {
        errormsg(SH_DICT, ERROR_exit(1), e_readonly, nv_name(np));
        __builtin_unreachable();
    }
    // The following could cause the shell to fork if assignment would cause a side effect.
    shp->argaddr = NULL;
    if (shp->subshell && !nv_local && !(flags & NV_RDONLY)) np = sh_assignok(np, 1);
    if (np->nvfun && np->nvfun->disc && !(flags & NV_NODISC) && !nv_isref(np)) {
        // This function contains disc.
        if (!nv_local) {
            nv_local = true;
            nv_putv(np, sp, flags, np->nvfun);
            if (sp && ((flags & NV_EXPORT) || nv_isattr(np, NV_EXPORT))) sh_envput(shp->env, np);
            return;
        }
        // Called from disc, assign the actual value.
    }
    flags &= ~NV_NODISC;
    nv_local = false;
    if (nv_isattr(np, NV_NOTSET) == NV_NOTSET) nv_offattr(np, NV_BINARY);
    if (flags & (NV_NOREF | NV_NOFREE)) {
        if (FETCH_VT(np->nvalue, const_cp) && FETCH_VT(np->nvalue, const_cp) != sp &&
            !nv_isattr(np, NV_NOFREE)) {
            free(FETCH_VT(np->nvalue, cp));
        }
        STORE_VT(np->nvalue, const_cp, sp);
        nv_setattr(np, (flags & ~NV_RDONLY) | NV_NOFREE);
        return;
    }
    up = &np->nvalue;
    if (IS_VT(np->nvalue, up) && nv_isarray(np) && nv_arrayptr(np)) {
        // The conditional assignment below is suspicious. But the original
        // version of this code didn't test the assigned type was `up`. It
        // simply tested that the pointer was non-NULL. And in most cases the
        // assigned type was not `up`. Making the test whether the pointer was
        // non-NULL doubly suspicious.
        if (FETCH_VT(np->nvalue, up)) up = FETCH_VT(np->nvalue, up);
    }
    if (up && FETCH_VTP(up, const_cp) == Empty) STORE_VTP(up, const_cp, NULL);
    if (nv_isattr(np, NV_EXPORT)) nv_offattr(np, NV_IMPORT);
    if (nv_isattr(np, NV_INTEGER)) {
        if (nv_isattr(np, NV_DOUBLE) == NV_DOUBLE) {
            if (nv_isattr(np, NV_LONG) && sizeof(double) < sizeof(Sfdouble_t)) {
                Sfdouble_t ld, old = 0;
                assert(vp);
                if (flags & NV_INTEGER) {
                    if (flags & NV_LONG) {
                        ld = *(Sfdouble_t *)vp;
                    } else if (flags & NV_SHORT) {
                        ld = *(float *)vp;
                    } else {
                        ld = *(double *)vp;
                    }
                } else {
                    ld = sh_arith(shp, vp);
                }
                if (!FETCH_VTP(up, sfdoublep)) {
                    STORE_VTP(up, sfdoublep, calloc(1, sizeof(Sfdouble_t)));
                } else if (flags & NV_APPEND) {
                    old = *FETCH_VTP(up, sfdoublep);
                }
                *FETCH_VTP(up, sfdoublep) = old ? ld + old : ld;
            } else {
                double d, od = 0;
                if (flags & NV_INTEGER) {
                    if (flags & NV_LONG) {
                        d = *(Sfdouble_t *)vp;
                    } else if (flags & NV_SHORT) {
                        d = *(float *)vp;
                    } else {
                        d = *(double *)vp;
                    }
                } else {
                    d = sh_arith(shp, vp);
                }
                if (!FETCH_VTP(up, dp)) {
                    if (nv_isattr(np, NV_SHORT)) {
                        STORE_VTP(up, fp, calloc(1, sizeof(float)));
                    } else {
                        STORE_VTP(up, dp, calloc(1, sizeof(double)));
                    }
                } else if (flags & NV_APPEND) {
                    od = *FETCH_VTP(up, dp);
                }
                if (nv_isattr(np, NV_SHORT)) {
                    *FETCH_VTP(up, fp) = (float)od ? d + od : d;
                } else {
                    *FETCH_VTP(up, dp) = od ? d + od : d;
                }
            }
        } else {
            if (nv_isattr(np, NV_LONG) && sizeof(int32_t) < sizeof(Sflong_t)) {
                Sflong_t ll = 0, oll = 0;
                if (flags & NV_INTEGER) {
                    if ((flags & NV_DOUBLE) == NV_DOUBLE) {
                        if (flags & NV_LONG) {
                            ll = *((Sfdouble_t *)vp);
                        } else if (flags & NV_SHORT) {
                            ll = *(float *)vp;
                        } else {
                            ll = *(double *)vp;
                        }
                    } else if (nv_isattr(np, NV_UNSIGN)) {
                        if (flags & NV_LONG) {
                            ll = *(Sfulong_t *)vp;
                        } else if (flags & NV_SHORT) {
                            ll = *(uint16_t *)vp;
                        } else {
                            ll = *(uint32_t *)vp;
                        }
                    } else {
                        if (flags & NV_LONG) {
                            ll = *(Sflong_t *)vp;
                        } else if (flags & NV_SHORT) {
                            ll = *(uint16_t *)vp;
                        } else {
                            ll = *(uint32_t *)vp;
                        }
                    }
                } else if (vp) {
                    ll = sh_arith(shp, vp);
                }
                if (!FETCH_VTP(up, i64p)) {
                    STORE_VTP(up, i64p, calloc(1, sizeof(Sflong_t)));
                } else if (flags & NV_APPEND) {
                    oll = *FETCH_VTP(up, i64p);
                }
                *FETCH_VTP(up, i64p) = ll + oll;
            } else {
                int32_t l = 0, ol = 0;
                if (flags & NV_INTEGER) {
                    if ((flags & NV_DOUBLE) == NV_DOUBLE) {
                        Sflong_t ll;
                        if (flags & NV_LONG) {
                            ll = *(Sfdouble_t *)vp;
                        } else if (flags & NV_SHORT) {
                            ll = *(float *)vp;
                        } else {
                            ll = *(double *)vp;
                        }
                        l = (int32_t)ll;
                    } else if (nv_isattr(np, NV_UNSIGN)) {
                        if (flags & NV_LONG) {
                            l = *(Sfulong_t *)vp;
                        } else if (flags & NV_SHORT) {
                            l = *(uint16_t *)vp;
                        } else {
                            l = *(uint32_t *)vp;
                        }
                    } else {
                        if (flags & NV_LONG) {
                            l = *((Sflong_t *)vp);
                        } else if (flags & NV_SHORT) {
                            l = *(int16_t *)vp;
                        } else {
                            l = *(int32_t *)vp;
                        }
                    }
                } else if (vp) {
                    Sfdouble_t ld = sh_arith(shp, vp);
                    if (ld < 0) {
                        l = (int32_t)ld;
                    } else {
                        l = (uint32_t)ld;
                    }
                }
                if (nv_size(np) <= 1) nv_setsize(np, 10);
                if (nv_isattr(np, NV_SHORT)) {
                    int16_t s = 0;
                    bool ptr = (nv_isattr(np, NV_INT16P) == NV_INT16P);
                    if (flags & NV_APPEND) s = ptr ? *FETCH_VTP(up, i16p) : FETCH_VTP(up, i16);
                    if (ptr) {
                        *FETCH_VTP(up, i16p) = s + (int16_t)l;
                    } else {
                        STORE_VTP(up, i16, s + (int16_t)l);
                    }
                    nv_onattr(np, NV_NOFREE);
                } else {
                    if (!FETCH_VTP(up, i32p)) {
                        STORE_VTP(up, i32p, calloc(1, sizeof(int32_t)));
                    } else if (flags & NV_APPEND) {
                        ol = *FETCH_VTP(up, i32p);
                    }
                    *FETCH_VTP(up, i32p) = l + ol;
                }
            }
        }
    } else {
        char *tofree = NULL;
        int offset = 0;
        int append;
        if (flags & NV_INTEGER) {
            if ((flags & NV_DOUBLE) == NV_DOUBLE) {
                if (flags & NV_LONG) {
                    sfprintf(shp->strbuf, "%.*Lg", LDBL_DIG, *((Sfdouble_t *)vp));
                } else {
                    sfprintf(shp->strbuf, "%.*g", DBL_DIG, *((double *)vp));
                }
            } else if (flags & NV_UNSIGN) {
                if (flags & NV_LONG) {
                    sfprintf(shp->strbuf, "%I*lu", sizeof(Sfulong_t), *((Sfulong_t *)vp));
                } else {
                    sfprintf(shp->strbuf, "%lu",
                             (unsigned long)((flags & NV_SHORT) ? *((uint16_t *)vp)
                                                                : *((uint32_t *)vp)));
                }
            } else {
                if (flags & NV_LONG) {
                    sfprintf(shp->strbuf, "%I*ld", sizeof(Sflong_t), *((Sflong_t *)vp));
                } else {
                    sfprintf(shp->strbuf, "%ld",
                             (long)((flags & NV_SHORT) ? *((int16_t *)vp) : *((int32_t *)vp)));
                }
            }
            sp = sfstruse(shp->strbuf);
        }

        // TODO: This if statement is noop, shall we remove this ?
        if (nv_isattr(np, NV_HOST | NV_INTEGER) == NV_HOST && sp) {
            ;
        } else if ((nv_isattr(np, NV_RJUST | NV_ZFILL | NV_LJUST)) && sp) {
            for (; *sp == ' ' || *sp == '\t'; sp++) {
                ;  // empty loop
            }
            if ((nv_isattr(np, NV_ZFILL)) && (nv_isattr(np, NV_LJUST))) {
                for (; *sp == '0'; sp++) {
                    ;  // empty loop
                }
            }
            size = nv_size(np);
            if (size) {
                bool right_adjust =
                    nv_isattr(np, NV_RJUST) == NV_RJUST || nv_isattr(np, NV_ZFILL) == NV_ZFILL;
                size = ja_size((char *)sp, size, right_adjust);
            }
        }
        if (!FETCH_VTP(up, const_cp) || *FETCH_VTP(up, const_cp) == 0) flags &= ~NV_APPEND;
        if (!nv_isattr(np, NV_NOFREE)) {
            // Delay free in case <sp> points into free region.
            tofree = FETCH_VTP(up, cp);
        }
        if (nv_isattr(np, NV_BINARY) && !(flags & NV_RAW)) tofree = NULL;
        if (nv_isattr(np, NV_LJUST | NV_RJUST) &&
            nv_isattr(np, NV_LJUST | NV_RJUST) != (NV_LJUST | NV_RJUST)) {
            tofree = NULL;
        }
        if (sp) {
            append = 0;
            if (sp == FETCH_VTP(up, const_cp) && !(flags & NV_APPEND)) return;
            dot = strlen(sp);
            if (nv_isattr(np, NV_BINARY)) {
                int oldsize = (flags & NV_APPEND) ? nv_size(np) : 0;
                if (flags & NV_RAW) {
                    if (tofree) {
                        free(tofree);
                        nv_offattr(np, NV_NOFREE);
                    }
                    STORE_VTP(up, const_cp, sp);
                    return;
                }
                size = 0;
                if (nv_isattr(np, NV_ZFILL)) size = nv_size(np);
                if (size == 0) size = oldsize + (3 * dot / 4);
                cp = malloc(size + 1);
                *cp = 0;
                nv_offattr(np, NV_NOFREE);
                if (oldsize) memcpy(cp, FETCH_VTP(up, const_cp), oldsize);
                STORE_VTP(up, cp, cp);
                if (size <= oldsize) return;
                dot = base64decode(sp, dot, cp + oldsize, size - oldsize);
                dot += oldsize;
                if (!nv_isattr(np, NV_ZFILL) || nv_size(np) == 0) {
                    nv_setsize(np, dot);
                } else if (nv_isattr(np, NV_ZFILL) && (size > dot)) {
                    memset(&cp[dot], 0, size - dot);
                }
                return;
            }

            if (size == 0 && nv_isattr(np, NV_HOST) != NV_HOST &&
                nv_isattr(np, NV_LJUST | NV_RJUST | NV_ZFILL)) {
                size = dot;
                nv_setsize(np, size);
                tofree = FETCH_VTP(up, cp);
            } else if (size > dot) {
                dot = size;
            } else if (nv_isattr(np, NV_LJUST | NV_RJUST) == NV_LJUST && dot > size) {
                dot = size;
            }

            if (flags & NV_APPEND) {
                if (dot == 0) return;
                append = strlen(FETCH_VTP(up, const_cp));
                if (!tofree || size) {
                    offset = stktell(shp->stk);
                    sfputr(shp->stk, FETCH_VTP(up, const_cp), -1);
                    sfputr(shp->stk, sp, 0);
                    sp = stkptr(shp->stk, offset);
                    dot += append;
                    append = 0;
                } else {
                    flags &= ~NV_APPEND;
                }
            }

            if (size == 0 || tofree || dot || !(cp = FETCH_VTP(up, cp))) {
                if (dot == 0 && !nv_isattr(np, NV_LJUST | NV_RJUST)) {
                    cp = (char *)EmptyStr;  // we'd better not try to modify this buf as it's const
                    nv_onattr(np, NV_NOFREE);
                } else {
                    if (tofree && tofree != Empty && tofree != EmptyStr) {
                        cp = realloc(tofree, (unsigned)dot + append + 8);
                        tofree = 0;
                    } else {
                        cp = malloc((unsigned)dot + 8);
                    }
                    cp[dot + append] = 0;
                    nv_offattr(np, NV_NOFREE);
                }
            }

        } else {
            cp = NULL;
        }
        STORE_VTP(up, cp, cp);
        if (sp) {
            int c = cp[dot + append];
            memmove(cp + append, sp, dot);
            if (cp == EmptyStr) {
                assert(dot == 0 && append == 0 && c == 0);
            } else {
                cp[dot + append] = c;
            }
            if (nv_isattr(np, NV_RJUST) && nv_isattr(np, NV_ZFILL)) {
                rightjust(cp, size, '0');
            } else if (nv_isattr(np, NV_LJUST | NV_RJUST) == NV_RJUST) {
                rightjust(cp, size, ' ');
            } else if (nv_isattr(np, NV_LJUST | NV_RJUST) == NV_LJUST) {
                char *dp;
                dp = strlen(cp) + cp;
                cp = cp + size;
                for (; dp < cp; *dp++ = ' ') {
                    ;  // empty loop
                }
            }
            // Restore original string.
            if (savep) ja_restore();
        }

        if (flags & NV_APPEND) stkseek(shp->stk, offset);
        if (tofree && tofree != Empty && tofree != EmptyStr) free(tofree);
    }
    if (!was_local && ((flags & NV_EXPORT) || nv_isattr(np, NV_EXPORT))) sh_envput(shp, np);
    return;
}

//
// Right-justify <str> so that it contains no more than <size> characters.  If <str> contains fewer
// than <size> characters, left-pad with <fill>.  Trailing blanks in <str> will be ignored.
//
// If the leftmost digit in <str> is not a digit, <fill> will default to a blank.
//
static_fn void rightjust(char *str, int size, int fill) {
    int n;
    char *cp, *sp;
    n = strlen(str);

    // Ignore trailing blanks.
    for (cp = str + n; n && *--cp == ' '; n--) {
        ;  // empty loop
    }
    if (n == size) return;
    if (n > size) {
        *(str + n) = 0;
        for (sp = str, cp = str + n - size; sp <= str + size; *sp++ = *cp++) {
            ;  // empty loop
        }
        return;
    }
    *(sp = str + size) = 0;
    if (n == 0) {
        while (sp > str) *--sp = ' ';
        return;
    }
    while (n--) {
        sp--;
        *sp = *cp--;
    }
    if (!isdigit(*str)) fill = ' ';
    while (sp > str) *--sp = fill;
    return;
}

//
// Handle left and right justified fields for multi-byte chars given physical size, return a logical
// size which reflects the screen width of multi-byte characters. Multi-width characters replaced by
// spaces if they cross the boundary. <type> is non-zero for right justified  fields.
//

static_fn int ja_size(char *str, int size, bool right_adjust) {
    char *cp = str;
    int n = size;
    int oldn = 0;
    int c = 0;
    char *oldcp = cp;
    int outsize;
    wchar_t w;

    while (*cp) {
        oldn = n;
        w = mb1char(&cp);
        if ((outsize = wcwidth(w)) < 0) outsize = 0;
        size -= outsize;
        c = cp - oldcp;
        n += (c - outsize);
        oldcp = cp;
        if (size <= 0 && !right_adjust) break;
    }

    // Check for right justified fields that need truncating.
    if (size >= 0) return n;

    if (!right_adjust) {
        // Left justified and character crosses field boundary.
        n = oldn;
        // Save boundary char and replace with spaces.
        size = c;
        savechars[size] = 0;
        while (size--) {
            savechars[size] = cp[size];
            cp[size] = ' ';
        }
        savep = cp;
    }
    size = -size;
    if (right_adjust) n -= (ja_size(str, size, 0) - size);
    return n;
}

static_fn void ja_restore(void) {
    char *cp = savechars;
    while (*cp) *savep++ = *cp++;
    savep = 0;
}

static_fn char *staknam(Shell_t *shp, Namval_t *np, char *value) {
    char *p, *q;

    q = stkalloc(shp->stk, strlen(nv_name(np)) + (value ? strlen(value) : 0) + 2);
    p = stpcpy(q, nv_name(np));
    if (value) {
        *p++ = '=';
        strcpy(p, value);
    }
    return q;
}

//
// Put the name and attribute into value of attributes variable.
//
static_fn void attstore(Namval_t *np, void *data) {
    nvflag_t nvflags = np->nvflag;
    struct adata *ap = (struct adata *)data;
    ap->sh = data;
    ap->tp = NULL;

    if (!nv_isflag(nvflags, NV_EXPORT) || nv_isflag(nvflags, NV_FUNCT)) return;

    if ((nvflags & (NV_UTOL | NV_LTOU | NV_INTEGER)) == (NV_UTOL | NV_LTOU)) {
        data = nv_mapchar(np, NULL);
        if (strcmp(data, e_tolower) && strcmp(data, e_toupper)) return;
    }
    nvflags &= (NV_RDONLY | NV_UTOL | NV_LTOU | NV_RJUST | NV_LJUST | NV_ZFILL | NV_INTEGER);
    *ap->attval++ = '=';
    if (nv_isflag(nvflags, NV_DOUBLE)) {
        // Export doubles as integers for ksh88 compatibility.
        *ap->attval++ = ' ' + (NV_INTEGER | (nvflags & ~(NV_DOUBLE | NV_EXPNOTE)));
        *ap->attval = ' ';
    } else {
        *ap->attval++ = ' ' + nvflags;
        if (nvflags & NV_INTEGER) {
            *ap->attval = ' ' + nv_size(np);
        } else {
            *ap->attval = ' ';
        }
    }
    ap->attval = stpcpy(++ap->attval, nv_name(np));
}

static_fn void pushnam(Namval_t *np, void *data) {
    char *value;
    struct adata *ap = (struct adata *)data;
    ap->sh = sh_ptr(np);
    ap->tp = NULL;
    if (nv_isattr(np, NV_IMPORT) && np->nvenv) {
        assert(np->nvenv_is_cp);
        *ap->argnam++ = (char *)np->nvenv;
    } else if ((value = nv_getval(np))) {
        *ap->argnam++ = staknam(ap->sh, np, value);
    }
    if (nv_isattr(np,
                  NV_RDONLY | NV_UTOL | NV_LTOU | NV_RJUST | NV_LJUST | NV_ZFILL | NV_INTEGER)) {
        ap->attsize += (strlen(nv_name(np)) + 4);
    }
}

//
// Generate the environment list for the child.
//
char **sh_envgen(Shell_t *shp) {
    char **er;
    int namec;
    char *cp;
    struct adata data;

    data.sh = shp;
    data.tp = NULL;
    data.mapname = 0;
    // VAR_underscore gets generated automatically as full path name of command.
    nv_offattr(VAR_underscore, NV_EXPORT);
    data.attsize = 6;
    namec = nv_scan(shp->var_tree, NULL, NULL, NV_EXPORT, NV_EXPORT);
    namec += shp->nenv;
    er = stkalloc(shp->stk, (namec + 4) * sizeof(char *));
    data.argnam = (er += 2) + shp->nenv;
    if (shp->nenv) memcpy(er, environ, shp->nenv * sizeof(char *));
    nv_scan(shp->var_tree, pushnam, &data, NV_EXPORT, NV_EXPORT);
    *data.argnam = stkalloc(shp->stk, data.attsize);
    cp = data.attval = stpcpy(*data.argnam, e_envmarker);
    nv_scan(shp->var_tree, attstore, &data, 0,
            (NV_RDONLY | NV_UTOL | NV_LTOU | NV_RJUST | NV_LJUST | NV_ZFILL | NV_INTEGER));
    *data.attval = 0;
    if (cp != data.attval) data.argnam++;
    *data.argnam = 0;
    return er;
}

struct scan {
    void (*scanfn)(Namval_t *, void *);
    nvflag_t scanmask;
    nvflag_t scanflags;
    int scancount;
    void *scandata;
};

static_fn int scanfilter(Dt_t *dict, void *arg, void *data) {
    Namval_t *np = (Namval_t *)arg;
    int k = np->nvflag;
    struct scan *sp = (struct scan *)data;
    struct adata *tp = (struct adata *)sp->scandata;
    char *cp;
    UNUSED(dict);

    if (!is_abuiltin(np) && tp && tp->tp && nv_type(np) != tp->tp) return 0;
    if (sp->scanmask == NV_TABLE && nv_isvtree(np)) k = NV_TABLE;
    if (!(sp->scanmask ? (k & sp->scanmask) == sp->scanflags
                       : (!sp->scanflags || (k & sp->scanflags)))) {
        return 0;
    }

    if (tp && tp->mapname) {
        if (sp->scanflags == NV_FUNCTION || sp->scanflags == (NV_NOFREE | NV_BINARY | NV_RAW)) {
            int n = strlen(tp->mapname);
            if (strncmp(np->nvname, tp->mapname, n) || np->nvname[n] != '.' ||
                strchr(&np->nvname[n + 1], '.')) {
                return 0;
            }
        } else if ((sp->scanflags == NV_UTOL || sp->scanflags == NV_LTOU) &&
                   (cp = (char *)nv_mapchar(np, NULL)) && strcmp(cp, tp->mapname)) {
            return 0;
        }
    }
    if (!FETCH_VT(np->nvalue, const_cp) && !np->nvfun && !np->nvflag) return 0;
    if (sp->scanfn) {
        if (nv_isarray(np)) nv_putsub(np, NULL, 0L, 0);
        (*sp->scanfn)(np, sp->scandata);
    }
    sp->scancount++;
    return 0;
}

//
// Walk through the name-value pairs.
// If <mask> is non-zero, then only nodes with (nvflags&mask)==flags
//     are visited.
// If <mask> is zero, and <flags> non-zero, then nodes with one or
//     more of <flags> is visited.
// If <mask> and <flags> are zero, then all nodes are visted.
//
int nv_scan(Dt_t *root, void (*fn)(Namval_t *, void *), void *data, nvflag_t mask, nvflag_t flags) {
    Namval_t *np;
    Dt_t *base = NULL;
    struct scan sdata;
    int (*hashfn)(Dt_t *, void *, void *);

    nv_isvalid(flags);
    nv_isvalid(mask);
    sdata.scanmask = mask;
    sdata.scanflags = flags & ~NV_NOSCOPE;
    sdata.scanfn = fn;
    sdata.scancount = 0;
    sdata.scandata = data;
    hashfn = scanfilter;
    if (flags & NV_NOSCOPE) base = dtview(root, 0);
    for (np = (Namval_t *)dtfirst(root); np; np = (Namval_t *)dtnext(root, np)) {
        hashfn(root, np, &sdata);
    }
    if (base) dtview(root, base);
    return sdata.scancount;
}

//
// Create a new environment scope.
//
void sh_scope(Shell_t *shp, struct argnod *envlist, int fun) {
    Dt_t *newscope, *newroot = (sh_isoption(shp, SH_BASH) ? shp->var_tree : shp->var_base);
    struct Ufunction *rp;

    if (shp->namespace) newroot = nv_dict(shp->namespace);
    newscope = dtopen(&_Nvdisc, Dtoset);
    dtuserdata(newscope, shp, 1);
    if (envlist) {
        dtview(newscope, shp->var_tree);
        shp->var_tree = newscope;
        sh_setlist(shp, envlist, NV_EXPORT | NV_NOSCOPE | NV_IDENT | NV_ASSIGN, 0);
        if (!fun) return;
        shp->var_tree = dtview(newscope, 0);
    }
    if ((rp = shp->st.real_fun) && rp->sdict) {
        dtview(rp->sdict, newroot);
        newroot = rp->sdict;
    }
    dtview(newscope, newroot);
    shp->var_tree = newscope;
}

//
// Remove freeable local space associated with the nvalue field of nnod. This includes any strings
// representing the value(s) of the node, as well as its dope vector, if it is an array.
//
void sh_envnolocal(Namval_t *np, void *data) {
    struct adata *tp = (struct adata *)data;
    char *cp = NULL;
    if (np == VAR_KSH_VERSION && nv_isref(np)) return;
    if (np == VAR_underscore) return;
    if (np == tp->sh->namespace) return;
    if (nv_isref(np)) nv_unref(np);
    if (nv_isattr(np, NV_EXPORT) && nv_isarray(np)) {
        nv_putsub(np, NULL, 0, 0);
        cp = nv_getval(np);
        if (cp) cp = strdup(cp);
    }
    if (nv_isattr(np, NV_EXPORT | NV_NOFREE)) {
        if (nv_isref(np) && np != VAR_KSH_VERSION) {
            nv_offattr(np, NV_NOFREE | NV_REF);
            free(FETCH_VT(np->nvalue, nrp));
            STORE_VT(np->nvalue, const_cp, NULL);
        }
        if (!cp) return;
    }
    if (nv_isarray(np)) nv_putsub(np, NULL, 0, ARRAY_UNDEF);
    _nv_unset(np, NV_RDONLY);
    nv_setattr(np, 0);
    if (cp) {
        nv_putval(np, cp, 0);
        free(cp);
    }
}

//
// Currently this is a dummy, but someday will be needed for reference counting.
//
void nv_close(Namval_t *np) { UNUSED(np); }

static_fn void table_unset(Shell_t *shp, Dt_t *root, int flags, Dt_t *oroot) {
    Namval_t *np, *nq, *npnext;
    for (np = (Namval_t *)dtfirst(root); np; np = npnext) {
        nq = dtsearch(oroot, np);
        if (nq) {
            if (nv_cover(nq)) {
                int subshell = shp->subshell;
                shp->subshell = 0;
                if (nv_isattr(nq, NV_INTEGER)) {
                    Sfdouble_t d = nv_getnum(nq);
                    nv_putval(nq, (char *)&d, NV_LDOUBLE);
                } else if (shp->test & 4) {
                    nv_putval(nq, strdup(nv_getval(nq)), NV_RDONLY);
                } else {
                    nv_putval(nq, nv_getval(nq), NV_RDONLY);
                }
                shp->subshell = subshell;
                np->nvfun = NULL;
            }
            if (nv_isattr(nq, NV_EXPORT)) sh_envput(shp, nq);
        }
        shp->last_root = root;
        shp->last_table = NULL;
        if (nv_isvtree(np)) {
            int len = strlen(np->nvname);
            npnext = (Namval_t *)dtnext(root, np);
            while ((nq = npnext) && strncmp(np->nvname, nq->nvname, len) == 0 &&
                   nq->nvname[len] == '.') {
                _nv_unset(nq, flags);
                npnext = (Namval_t *)dtnext(root, nq);
                nv_delete(nq, root, NV_TABLE);
            }
        }
        npnext = (Namval_t *)dtnext(root, np);
        if (nv_arrayptr(np)) nv_putsub(np, NULL, 0, ARRAY_SCAN);
        _nv_unset(np, flags);
        nv_delete(np, root, NV_TABLE);
    }
}

//
// Set the value of <np> to 0, and nullify any attributes that <np> may have had.  Free any freeable
// space occupied by the value of <np>.  If <np> denotes an array member, it will retain its
// attributes.
//
// <flags> can contain NV_RDONLY to override the readonly attribute being cleared.
// <flags> can contain NV_EXPORT to override preserve nvenv
//
void _nv_unset(Namval_t *np, nvflag_t flags) {
    Shell_t *shp = sh_ptr(np);
    struct Value *up;

    nv_isvalid(flags);
    if (!(flags & NV_RDONLY) && nv_isattr(np, NV_RDONLY)) {
        errormsg(SH_DICT, ERROR_exit(1), e_readonly, nv_name(np));
        __builtin_unreachable();
    }
    if (is_afunction(np) && FETCH_VT(np->nvalue, rp)) {
        struct slnod *slp = (struct slnod *)(np->nvenv);
        if (FETCH_VT(np->nvalue, rp)->running) {
            FETCH_VT(np->nvalue, rp)->running |= 1;
            return;
        }
        if (!slp) goto done;
        if (nv_isattr(np, NV_NOFREE)) goto done;

        struct Ufunction *rq, *rp = FETCH_VT(np->nvalue, rp);
        // Free function definition.
        char *name = nv_name(np), *cp = strrchr(name, '.');
        if (cp) {
            Namval_t *npv;
            *cp = 0;
            npv = nv_open(name, shp->var_tree, NV_NOARRAY | NV_VARNAME | NV_NOADD);
            *cp++ = '.';
            if (npv && npv != shp->namespace) {
                nv_setdisc(npv, cp, NULL, (Namfun_t *)npv);
            }
        }
        if (rp->fname && shp->fpathdict &&
            (rq = (struct Ufunction *)nv_search(rp->fname, shp->fpathdict, 0))) {
            while (rq) {
                if (rq->np == np) {
                    dtdelete(shp->fpathdict, rq);
                    break;
                }
                rq = dtnext(shp->fpathdict, rq);
            }
        }
        if (rp->sdict) {
            Namval_t *mp, *nq;
            for (mp = (Namval_t *)dtfirst(rp->sdict); mp; mp = nq) {
                nq = dtnext(rp->sdict, mp);
                _nv_unset(mp, NV_RDONLY);
                nv_delete(mp, rp->sdict, 0);
            }
            dtclose(rp->sdict);
        }
        stkclose(slp->slptr);
        free(FETCH_VT(np->nvalue, ip));
        STORE_VT(np->nvalue, ip, NULL);
        goto done;
    }
    if (shp->subshell) np = sh_assignok(np, 0);
    nv_offattr(np, NV_NODISC);
    if (np->nvfun && !nv_isref(np)) {
        // This function contains disc.
        if (!nv_local) {
            Dt_t *last_root = shp->last_root;
            nv_local = true;
            nv_putv(np, NULL, flags, np->nvfun);
            nv_local = false;
            shp->last_root = last_root;
            return;
        }
        // Called from disc, assign the actual value.
        nv_local = false;
    }
    if (nv_isattr(np, NV_INT16P | NV_DOUBLE) == NV_INT16) {
        STORE_VT(np->nvalue, const_cp, nv_isarray(np) ? Empty : NULL);
        goto done;
    } else if (FETCH_VT(np->nvalue, up) && nv_isarray(np) && nv_arrayptr(np)) {
        up = FETCH_VT(np->nvalue, up);
    } else if (nv_isref(np) && !nv_isattr(np, NV_EXPORT | NV_MINIMAL) &&
               FETCH_VT(np->nvalue, nrp)) {
        if (FETCH_VT(np->nvalue, nrp)->root) dtremove(Refdict, FETCH_VT(np->nvalue, nrp));
        if (FETCH_VT(np->nvalue, nrp)->sub) free(FETCH_VT(np->nvalue, nrp)->sub);
        free(FETCH_VT(np->nvalue, nrp));
        STORE_VT(np->nvalue, const_cp, NULL);
        up = NULL;
    } else {
        up = &np->nvalue;
    }
    if (up && FETCH_VTP(up, cp)) {
        if (FETCH_VTP(up, cp) != Empty && FETCH_VTP(up, cp) != EmptyStr &&
            !nv_isattr(np, NV_NOFREE)) {
            free(FETCH_VTP(up, cp));
        }
        STORE_VTP(up, cp, NULL);
    }

done:
    if (!nv_isarray(np) || !nv_arrayptr(np)) {
        nv_setsize(np, 0);
        if (!nv_isattr(np, NV_MINIMAL) || nv_isattr(np, NV_EXPORT)) {
            if (nv_isattr(np, NV_EXPORT) && !strchr(np->nvname, '[')) {
                env_delete(shp->env, nv_name(np));
            }
            if (!(flags & NV_EXPORT) || nv_isattr(np, NV_EXPORT)) np->nvenv = NULL;
            nv_setattr(np, 0);
        } else {
            nv_setattr(np, NV_MINIMAL);
            nv_delete(np, NULL, 0);
        }
    }
}

//
// Return the node pointer in the highest level scope.
//
Namval_t *sh_scoped(Shell_t *shp, Namval_t *np) {
    if (!dtvnext(shp->var_tree)) return np;
    return dtsearch(shp->var_tree, np);
}

struct optimize {
    Namfun_t namfun;
    Shell_t *sh;
    char **ptr;
    struct optimize *next;
    Namval_t *np;
};

static struct optimize *opt_free;

static_fn void optimize_clear(Namval_t *np, Namfun_t *fp) {
    struct optimize *op = (struct optimize *)fp;
    nv_stack(np, fp);
    nv_stack(np, NULL);
    for (; op && op->np == np; op = op->next) {
        if (op->ptr) {
            *op->ptr = 0;
            op->ptr = NULL;
        }
    }
}

static_fn void put_optimize(Namval_t *np, const void *val, nvflag_t flags, Namfun_t *fp) {
    nv_isvalid(flags);
    nv_putv(np, val, flags, fp);
    optimize_clear(np, fp);
}

static_fn Namfun_t *clone_optimize(Namval_t *np, Namval_t *mp, nvflag_t flags, Namfun_t *fp) {
    UNUSED(np);
    UNUSED(mp);
    UNUSED(flags);
    UNUSED(fp);

    nv_isvalid(flags);
    return NULL;
}

const Namdisc_t OPTIMIZE_disc = {
    .dsize = sizeof(struct optimize), .putval = put_optimize, .clonef = clone_optimize};

void nv_optimize(Namval_t *np) {
    Shell_t *shp = sh_ptr(np);
    Namfun_t *fp;
    struct optimize *op, *xp;

    if (shp->argaddr) {
        if (np == VAR_sh_lineno) {
            shp->argaddr = NULL;
            return;
        }
        for (fp = np->nvfun; fp; fp = fp->next) {
            if (fp->disc && (fp->disc->getnum || fp->disc->getval)) {
                shp->argaddr = NULL;
                return;
            }
            if (fp->disc == &OPTIMIZE_disc) break;
        }
        if ((xp = (struct optimize *)fp) && xp->ptr == shp->argaddr) return;
        op = opt_free;
        if (xp && xp->next) {
            for (struct optimize *xpn = xp->next; xpn; xpn = xpn->next) {
                if (xpn->ptr == shp->argaddr && xpn->np == np) return;
            }
        }
        if (op) {
            opt_free = op->next;
        } else {
            op = calloc(1, sizeof(struct optimize));
        }
        op->ptr = shp->argaddr;
        op->np = np;
        if (xp) {
            op->namfun.disc = NULL;
            op->next = xp->next;
            xp->next = op;
        } else {
            op->namfun.disc = &OPTIMIZE_disc;
            op->next = (struct optimize *)shp->optlist;
            shp->optlist = op;
            nv_stack(np, &op->namfun);
        }
    }
}

void sh_optclear(Shell_t *shp, void *old) {
    struct optimize *op, *opnext;
    for (op = (struct optimize *)shp->optlist; op; op = opnext) {
        opnext = op->next;
        if (op->ptr && op->namfun.disc) {
            nv_stack(op->np, &op->namfun);
            nv_stack(op->np, NULL);
        }
        op->next = opt_free;
        opt_free = op;
    }
    shp->optlist = old;
}

//
// Return a pointer to a character string that denotes the value of <np>.  If <np> refers to an
// array,  return a pointer to the value associated with the current index.
//
// If the value of <np> is an integer, the string returned will be overwritten by the next call to
// nv_getval.
//
// If <np> has no value, 0 is returned.
//
// TODO: Convert this to return a const char *.
char *nv_getval(Namval_t *np) {
    Shell_t *shp = sh_ptr(np);
    struct Value *up = &np->nvalue;
    int numeric;

    if (!nv_local && shp->argaddr) nv_optimize(np);
    if ((!np->nvfun || !np->nvfun->disc) &&
        !nv_isattr(np, NV_ARRAY | NV_INTEGER | NV_FUNCT | NV_REF)) {
        goto done;
    }
    if (nv_isref(np)) {
        char *sub;
        if (!FETCH_VT(np->nvalue, const_cp)) return NULL;
        shp->last_table = nv_reftable(np);
        sub = nv_refsub(np);
        np = nv_refnode(np);
        if (sub) {
            sfprintf(shp->strbuf, "%s[%s]", nv_name(np), sub);
            return sfstruse(shp->strbuf);
        }
        return nv_name(np);
    }
    if (np->nvfun && np->nvfun->disc) {
        if (!nv_local) {
            nv_local = true;
            return nv_getv(np, np->nvfun);
        }
        nv_local = false;
    }
    numeric = (nv_isattr(np, NV_INTEGER) != 0);
    if (numeric) {
        Sflong_t ll;
        if (nv_isattr(np, NV_NOTSET) == NV_NOTSET) return NULL;
        if (!FETCH_VTP(up, vp)) return "0";
        if (nv_isattr(np, NV_DOUBLE) == NV_DOUBLE) {
            Sfdouble_t ld;
            double d;
            char *format;
            if (nv_isattr(np, NV_LONG)) {
                ld = *FETCH_VTP(up, sfdoublep);
                if (nv_isattr(np, NV_EXPNOTE)) {
                    format = "%.*Lg";
                } else if (nv_isattr(np, NV_HEXFLOAT)) {
                    format = "%.*La";
                } else {
                    format = "%.*Lf";
                }
                sfprintf(shp->strbuf, format, nv_size(np), ld);
            } else {
                if (nv_isattr(np, NV_SHORT)) {
                    d = *FETCH_VTP(up, fp);
                } else {
                    d = *FETCH_VTP(up, dp);
                }
                if (nv_isattr(np, NV_EXPNOTE)) {
                    format = "%.*g";
                } else if (nv_isattr(np, NV_HEXFLOAT)) {
                    format = "%.*a";
                } else {
                    format = "%.*f";
                }
                sfprintf(shp->strbuf, format, nv_size(np), d);
            }
            return sfstruse(shp->strbuf);
        } else if (nv_isattr(np, NV_UNSIGN)) {
            if (nv_isattr(np, NV_LONG)) {
                ll = *(Sfulong_t *)FETCH_VTP(up, i64p);
            } else if (nv_isattr(np, NV_SHORT)) {
                if (nv_isattr(np, NV_INT16P) == NV_INT16P) {
                    ll = *(uint16_t *)FETCH_VTP(up, i16p);
                } else {
                    ll = (uint16_t)FETCH_VTP(up, i16);
                }
            } else {
                ll = *(uint32_t *)FETCH_VTP(up, i32p);
            }
        } else if (nv_isattr(np, NV_LONG)) {
            ll = *FETCH_VTP(up, i64p);
        } else if (nv_isattr(np, NV_SHORT)) {
            if (nv_isattr(np, NV_INT16P) == NV_INT16P) {
                ll = *FETCH_VTP(up, i16p);
            } else {
                ll = FETCH_VTP(up, i16);
            }
        } else {
            if (IS_VTP(up, pidp)) {
                ll = *FETCH_VTP(up, pidp);
            } else if (IS_VTP(up, uidp)) {
                ll = *FETCH_VTP(up, uidp);
            } else {
                ll = *FETCH_VTP(up, i32p);  // presumably it's a pointer to a 32 bit int
            }
        }
        numeric = nv_size(np);
        if (numeric == 10) {
            if (nv_isattr(np, NV_UNSIGN)) {
                sfprintf(shp->strbuf, "%I*u", sizeof(ll), ll);
                return sfstruse(shp->strbuf);
            }
            numeric = 0;
        }
        return fmtbase(ll, numeric, numeric && numeric != 10);
    }
done:
    // If NV_RAW flag is on, return pointer to binary data otherwise, base64 encode the data and
    // return this string.
    if (FETCH_VTP(up, const_cp) && nv_isattr(np, NV_BINARY) && !nv_isattr(np, NV_RAW)) {
        int size = nv_size(np), insize = (4 * size) / 3 + size / 45 + 8;
        char *cp = getbuf(insize);
        base64encode(FETCH_VTP(up, const_cp), size, cp, insize);
        return cp;
    }

    // See issue #690. The original version of this block of code did an invalid `up->cp[numeric]`.
    // That is invalid because it assumes the string pointed to by `up->cp` is at least `numeric`
    // (i.e., `nv_size(np)`) chars long. That is an invalid assumption. This version avoids a
    // possible incorrect reference past the end of the string but it's not obvious what this block
    // is meant to do and may therefore still be incorrect.
    if (FETCH_VTP(up, const_cp) && !nv_isattr(np, NV_LJUST | NV_RJUST)) {
        int size = nv_size(np);
        if (size > 0 && size < strlen(FETCH_VTP(up, const_cp))) {
            char *cp = getbuf(size + 1);
            strlcpy(cp, FETCH_VTP(up, const_cp), size);
            return cp;
        }
    }

    // TODO: remove the cast when this function is converted to return a const char *.
    return (char *)FETCH_VTP(up, const_cp);
}

Sfdouble_t nv_getnum(Namval_t *np) {
    Shell_t *shp = sh_ptr(np);
    struct Value *up;
    Sfdouble_t r = 0;
    char *str;

    if (!nv_local && shp->argaddr) nv_optimize(np);
    if (nv_istable(np)) {
        errormsg(SH_DICT, ERROR_exit(1), e_number, nv_name(np));
        __builtin_unreachable();
    }
    if (np->nvfun && np->nvfun->disc) {
        if (!nv_local) {
            nv_local = true;
            return nv_getn(np, np->nvfun);
        }
        nv_local = false;
    }
    if (nv_isref(np)) {
        str = nv_refsub(np);
        np = nv_refnode(np);
        if (str) nv_putsub(np, str, 0L, 0);
    }
    if (nv_isattr(np, NV_INTEGER)) {
        if (sh_isoption(shp, SH_NOUNSET) && nv_isattr(np, NV_NOTSET) == NV_NOTSET) {
            int i = nv_aindex(np);
            errormsg(SH_DICT, ERROR_exit(1), e_notset2, nv_name(np), i);
            __builtin_unreachable();
        }
        up = &np->nvalue;
        if (!FETCH_VTP(up, i32p) || FETCH_VTP(up, const_cp) == Empty) {
            r = 0;
        } else if (nv_isattr(np, NV_DOUBLE) == NV_DOUBLE) {
            if (nv_isattr(np, NV_LONG)) {
                r = *FETCH_VTP(up, sfdoublep);
            } else if (nv_isattr(np, NV_SHORT)) {
                r = *FETCH_VTP(up, fp);
            } else {
                r = *FETCH_VTP(up, dp);
            }
        } else if (nv_isattr(np, NV_UNSIGN)) {
            if (nv_isattr(np, NV_LONG)) {
                r = (Sfulong_t) * ((Sfulong_t *)FETCH_VTP(up, i64p));
            } else if (nv_isattr(np, NV_SHORT)) {
                if (nv_isattr(np, NV_INT16P) == NV_INT16P) {
                    r = (Sfulong_t)*FETCH_VTP(up, i16p);
                } else {
                    r = (Sfulong_t)FETCH_VTP(up, i16);
                }
            } else {
                r = *((uint32_t *)FETCH_VTP(up, i32p));
            }
        } else {
            if (nv_isattr(np, NV_LONG)) {
                r = *FETCH_VTP(up, i64p);
            } else if (nv_isattr(np, NV_SHORT)) {
                if (nv_isattr(np, NV_INT16P) == NV_INT16P) {
                    r = *FETCH_VTP(up, i16p);
                } else {
                    r = FETCH_VTP(up, i16);
                }
            } else {
                r = *FETCH_VTP(up, i32p);
            }
        }
    } else if ((str = nv_getval(np)) && *str != 0) {
        if (nv_isattr(np, NV_LJUST | NV_RJUST) ||
            (*str == '0' && !(str[1] == 'x' || str[1] == 'X'))) {
            while (*str == '0') str++;
        }
        r = sh_arith(shp, str);
    }
    return r;
}

//
// Give <np> the attributes <newatts,> and change its current value to conform to <newatts>.  The
// <size> of left and right justified fields may be given.
//
void nv_newattr(Namval_t *np, nvflag_t newatts, int size) {
    Shell_t *shp = sh_ptr(np);
    char *sp;
    char *cp = NULL;
    Namval_t *mp = NULL;
    Namarr_t *ap = NULL;
    int oldsize, oldatts, trans;
    Namfun_t *fp = (newatts & NV_NODISC) ? np->nvfun : 0;
    char *prefix = shp->prefix;

    nv_isvalid(newatts);
    newatts &= ~NV_NODISC;
    // Check for restrictions.
    if (sh_isoption(shp, SH_RESTRICTED) &&
        ((sp = nv_name(np)) == nv_name(VAR_PATH) || sp == nv_name(VAR_SHELL) ||
         sp == nv_name(VAR_ENV) || sp == nv_name(VAR_FPATH))) {
        errormsg(SH_DICT, ERROR_exit(1), e_restricted, nv_name(np));
        __builtin_unreachable();
    }
    // Handle attributes that do not change data separately.
    nvflag_t nvflags = np->nvflag;
    trans = !nv_isflag(nvflags, NV_INTEGER) && (nvflags & (NV_LTOU | NV_UTOL));
    if (newatts & NV_EXPORT) nv_offattr(np, NV_IMPORT);
    if (((nvflags ^ newatts) & NV_EXPORT)) {
        // Record changes to the environment.
        if (nv_isflag(nvflags, NV_EXPORT)) {
            nv_offattr(np, NV_EXPORT);
            env_delete(shp->env, nv_name(np));
        } else {
            nv_onattr(np, NV_EXPORT);
            sh_envput(shp, np);
        }
        if ((nvflags ^ newatts) == NV_EXPORT && size == -1) return;
    }
    oldsize = nv_size(np);
    if (size == -1) size = oldsize;
    if ((size == oldsize || nv_isflag(nvflags, NV_INTEGER)) && !trans &&
        ((nvflags ^ newatts) & ~NV_NOCHANGE) == 0) {
        if (size) nv_setsize(np, size);
        if (nv_isattr(np, NV_NOFREE)) newatts |= NV_NOFREE;
        nv_setattr(np, newatts);
        return;
    }
    // For an array, change all the elements.
    if ((ap = nv_arrayptr(np)) && ap->nelem > 0) nv_putsub(np, NULL, 0, ARRAY_SCAN);
    oldsize = nv_size(np);
    oldatts = np->nvflag;
    if (fp) np->nvfun = NULL;
    if (ap) {  // add element to prevent array deletion
        ap->nelem++;
    }
    do {
        nv_setsize(np, oldsize);
        nv_setattr(np, oldatts);
        sp = nv_getval(np);
        if (sp) {
            if (nv_isattr(np, NV_ZFILL)) {
                while (*sp == '0') sp++;
            }
            // TODO: Figure out what the magic number 8 means and replace with an appropriate
            // symbol here and elsewhere in this module. This was the original statement:
            //   cp = malloc((n = strlen(sp)) + 8);
            // That, however, can result in a buffer that is too small because the `size` passed
            // into this function may be larger than `n + 8`. Resulting in the memmove() in
            // nv_putval() to read past the end of the buffer. So instead do this hack:
            size_t n = strlen(sp);
            cp = malloc(n + 8 >= size + 8 ? n + 8 : size + 8);
            strcpy(cp, sp);

            mp = nv_opensub(np);
            if (mp) {
                if (trans) {
                    nv_disc(np, &ap->namfun, DISC_OP_POP);
                    nv_clone(np, mp, 0);
                    nv_disc(np, &ap->namfun, DISC_OP_FIRST);
                    nv_offattr(mp, NV_ARRAY);
                }
                nv_newattr(mp, newatts & ~NV_ARRAY, size);
            } else {
                if (ap) ap->flags &= ~ARRAY_SCAN;
                if (!trans) _nv_unset(np, NV_RDONLY | NV_EXPORT);
                if (ap) ap->flags |= ARRAY_SCAN;
            }
            if (size == 0 && (newatts & NV_HOST) != NV_HOST &&
                (newatts & (NV_LJUST | NV_RJUST | NV_ZFILL))) {
                size = n;
            }
        } else if (!trans) {
            _nv_unset(np, NV_EXPORT);
        }
        nv_setsize(np, size);
        np->nvflag &= (NV_ARRAY | NV_NOFREE);
        np->nvflag |= newatts;
        if (cp) {
            if (!mp) nv_putval(np, cp, NV_RDONLY);
            free(cp);
            cp = NULL;
        }
    } while (ap && nv_nextsub(np));
    if (fp) np->nvfun = fp;
    if (ap) ap->nelem--;
    shp->prefix = prefix;
    return;
}

static_fn char *oldgetenv(const char *string) {
    char c0, c1;
    char *cp;
    const char *sp;
    char **av = environ;

    if (!string || (c0 = *string) == 0) return 0;
    if ((c1 = *++string) == 0) c1 = '=';
    while ((cp = *av++)) {
        if (cp[0] != c0 || cp[1] != c1) continue;
        sp = string;
        while (*sp && *sp++ == *++cp) {
            ;  // empty loop
        }
        if (*sp == 0 && *++cp == '=') return cp + 1;
    }
    return NULL;
}

//
// This version of getenv uses the hash storage to access environment values.
//
char *sh_getenv(const char *name) {
    Shell_t *shp = sh_getinterp();
    Namval_t *np;

    if (!shp->var_tree) return oldgetenv(name);
    if ((np = nv_search(name, shp->var_tree, 0)) && nv_isattr(np, NV_EXPORT)) return nv_getval(np);
    return NULL;
}

//
// This uses namval hash storage to manage environment vars rather than the
// libc mechanism as would be done by putenv(), setenv(), or unsetenv().
//
char *sh_setenviron(const char *name) {
    assert(name);

    Shell_t *shp = sh_getinterp();
    Namval_t *np = nv_open(name, shp->var_tree, NV_EXPORT | NV_IDENT | NV_NOARRAY | NV_ASSIGN);
    if (strchr(name, '=')) return nv_getval(np);
    _nv_unset(np, 0);
    return "";
}

//
// Normalize <cp> and return pointer to subscript if any if <eq> is specified, return pointer to
// first = not in a subscript.
//
static_fn char *lastdot(char *cp, int eq, void *context) {
    char *ep = NULL;
    int c;

    if (eq) cp++;
    while ((c = mb1char(&cp))) {
        if (c == '[') {
            if (*cp == ']') {
                cp++;
            } else {
                cp = nv_endsubscript(NULL, ep = cp, 0, context);
            }
        } else if (c == '.') {
            if (*cp == '[') {
                cp = nv_endsubscript(NULL, ep = cp, 0, context);
                if ((ep = sh_checkid(ep + 1, cp)) < cp) cp = strcpy(ep, cp);
            }
            ep = NULL;
        } else if (eq && c == '=') {
            return cp - 1;
        }
    }
    return eq ? 0 : ep;
}

//
// Purge all entries whose name is of the form name.
//
static_fn void cache_purge(const char *name) {
    int c;
    size_t len = strlen(name);
    struct Cache_entry *xp;

    for (c = 0, xp = nvcache.entries; c < NVCACHE; xp = &nvcache.entries[++c]) {
        if (xp->len <= len || xp->name[len] != '.') continue;
        if (strncmp(name, xp->name, len) == 0) xp->root = NULL;
    }
}

bool nv_rename(Namval_t *np, nvflag_t flags) {
    Shell_t *shp = sh_ptr(np);
    Namval_t *mp = NULL;
    Namval_t *nr = NULL;
    char *cp;
    int arraynr, index = -1;
    Namval_t *last_table = shp->last_table;
    Dt_t *last_root = shp->last_root;
    Dt_t *hp = NULL;
    Namval_t *nvenv = NULL;
    char *prefix = shp->prefix;
    Namarr_t *ap;

    nv_isvalid(flags);
    if (nv_isattr(np, NV_PARAM) && shp->st.prevst) {
        hp = shp->st.prevst->save_tree;
        if (!hp) hp = dtvnext(shp->var_tree);
    }
    if (!nv_isattr(np, NV_MINIMAL)) nvenv = np->nvenv;
    mp = nv_isarray(np) ? nv_opensub(np) : NULL;
    if (flags & NV_MOVE) {
        cp = (char *)FETCH_VT((mp ? mp : np)->nvalue, const_cp);
        if (!cp) {
            errormsg(SH_DICT, ERROR_exit(1), e_varname, "");
            __builtin_unreachable();
        }
        STORE_VT((mp ? mp : np)->nvalue, const_cp, NULL);
    } else if (!(cp = nv_getval(np))) {
        return false;
    }
    if (lastdot(cp, 0, shp) && nv_isattr(np, NV_MINIMAL)) {
        errormsg(SH_DICT, ERROR_exit(1), e_varname, nv_name(np));
        __builtin_unreachable();
    }
    arraynr = cp[strlen(cp) - 1] == ']';
    if (nv_isarray(np) && !mp) index = nv_aindex(np);
    shp->prefix = NULL;
    if (!hp) hp = shp->var_tree;
    if (!(nr = nv_open(cp, hp, flags | NV_ARRAY | NV_NOSCOPE | NV_NOADD | NV_NOFAIL))) {
        if (shp->namespace) {
            hp = nv_dict(shp->namespace);
        } else {
            hp = shp->var_base;
        }
    } else if (shp->last_root) {
        hp = shp->last_root;
    }
    if (!nr) nr = nv_open(cp, hp, flags | NV_NOREF | ((flags & NV_MOVE) ? 0 : NV_NOFAIL));
    shp->prefix = prefix;
    if (sh_isoption(shp, SH_NOUNSET) && (!nr || nv_isnull(nr))) {
        errormsg(SH_DICT, ERROR_exit(1), e_notset, cp);
        __builtin_unreachable();
    }
    if (!nr) {
        if (!nv_isvtree(np)) _nv_unset(np, 0);
        return false;
    }
    if (flags & NV_MOVE) free(cp);
    if (!mp && index >= 0 && nv_isvtree(nr)) {
        sfprintf(shp->strbuf, "%s[%d]%c", nv_name(np), index, 0);
        // Create a virtual node.
        mp = nv_open(sfstruse(shp->strbuf), shp->var_tree, NV_VARNAME | NV_ADD | NV_ARRAY);
        if (mp) {
            ap = nv_arrayptr(np);
            if (ap) ap->nelem++;
            mp->nvenv = nvenv = np;
        }
    }
    if (mp) {
        nvenv = np;
        np = mp;
    }
    if (nr == np) {
        if (index < 0) return false;
        cp = nv_getval(np);
        if (cp) cp = strdup(cp);
    }
    _nv_unset(np, NV_EXPORT);
    if (nr == np) {
        nv_putsub(np, NULL, index, 0);
        nv_putval(np, cp, 0);
        free(cp);
        return true;
    }
    if ((shp->prev_table = shp->last_table) && nv_isvtree(nr)) shp->prev_table = NULL;
    shp->prev_root = shp->last_root;
    shp->last_table = last_table;
    shp->last_root = last_root;
    if ((nv_arrayptr(nr) && !arraynr) || nv_isvtree(nr)) {
        ap = nv_arrayptr(np);
        if (ap) {
            if (!ap->table) {
                ap->table = dtopen(&_Nvdisc, Dtoset);
                dtuserdata(ap->table, shp, 1);
            }
            if (ap->table) mp = nv_search(nv_getsub(np), ap->table, NV_ADD);
            nv_arraychild(np, mp, 0);
            nvenv = np;
        } else {
            mp = np;
        }
        assert(mp);
        if (nr == VAR_sh_match) {
            Sfio_t *iop;
            Dt_t *save_root = shp->var_tree;
            bool trace = sh_isoption(shp, SH_XTRACE);
            sfprintf(shp->strbuf, "typeset -a %s=", nv_name(mp));
            nv_outnode(nr, shp->strbuf, -1, 0);
            sfwrite(shp->strbuf, ")\n", 2);
            cp = sfstruse(shp->strbuf);
            iop = sfopen(NULL, cp, "s");
            if (trace) sh_offoption(shp, SH_XTRACE);
            shp->var_tree = last_root;
            sh_eval(shp, iop, SH_READEVAL);
            shp->var_tree = save_root;
            if (trace) sh_onoption(shp, SH_XTRACE);
            if (flags & NV_MOVE) sh_setmatch(shp, 0, 0, 0, 0, 0);
        } else {
            nv_clone(nr, mp, (flags & NV_MOVE) | NV_COMVAR);
            cache_purge(nv_name(nr));
        }
        mp->nvenv = nvenv;
        if (flags & NV_MOVE) {
            if (arraynr && !nv_isattr(nr, NV_MINIMAL) && (mp = nr->nvenv) &&
                (ap = nv_arrayptr(mp))) {
                nv_putsub(mp, nr->nvname, 0, 0);
                _nv_unset(mp, 0);
            }
            nv_delete(nr, NULL, NV_NOFREE);
        }
    } else {
        if (flags & NV_MOVE) {
            if (!nv_isattr(nr, NV_MINIMAL) && (mp = nr->nvenv) && (ap = nv_arrayptr(mp))) {
                ap->nelem--;
            }
            if (!nv_isarray(np) && !nv_isarray(nr)) {
                nv_clone(nr, np, flags & NV_MOVE);
            } else {
                nv_putval(np, nv_getval(nr), 0);
                _nv_unset(nr, 0);
            }
        } else {
            nv_clone(nr, np, NV_COMVAR);
        }
    }
    return true;
}

//
// Create a reference node from <np> to $np in dictionary <hp>.
//
void nv_setref(Namval_t *np, Dt_t *hp, nvflag_t flags) {
    Shell_t *shp = sh_ptr(np);
    Namval_t *nq = NULL;
    Namval_t *nr = NULL;
    char *ep, *cp;
    Dt_t *openmatch;
    Dt_t *root = shp->last_root;
    Dt_t *hpnext = NULL;
    Namarr_t *ap = NULL;
    Namval_t *last_table = shp->last_table;

    nv_isvalid(flags);
    if (nv_isref(np)) return;
    if (nv_isarray(np)) {
        errormsg(SH_DICT, ERROR_exit(1), e_badref, nv_name(np));
        __builtin_unreachable();
    }
    if (!(cp = nv_getval(np))) {
        _nv_unset(np, 0);
        nv_onattr(np, NV_REF);
        return;
    }
    if ((ep = lastdot(cp, 0, shp)) && nv_isattr(np, NV_MINIMAL)) {
        errormsg(SH_DICT, ERROR_exit(1), e_badref, nv_name(np));
        __builtin_unreachable();
    }
    if (hp) hpnext = dtvnext(hp);
    shp->namref_root = hp;
    if ((nr = nv_open(cp, hp ? hp : shp->var_tree, flags | NV_NOSCOPE | NV_NOADD | NV_NOFAIL))) {
        nq = nr;
    } else if (hpnext && dtvnext(hpnext) == shp->var_base &&
               (nr = nv_open(cp, hpnext, flags | NV_NOSCOPE | NV_NOADD | NV_NOFAIL))) {
        nq = nr;
    } else if ((openmatch = shp->openmatch) && hpnext == shp->var_base &&
               (nr = nv_open(cp, hpnext, flags | NV_NOSCOPE | NV_NOADD | NV_NOFAIL))) {
        nq = nr;
    }
    if (nq) {
        hp = shp->last_root;
    } else {
        char *xp;
        if (hp && (xp = strchr(cp, '.'))) {
            *xp = 0;
            nr = nv_open(cp, hp, flags | NV_NOSCOPE | NV_NOADD | NV_NOFAIL);
            *xp = '.';
            if (nr && nv_isvtree(nr) && (nr = nv_open(cp, hp, flags | NV_NOSCOPE | NV_NOFAIL))) {
                nq = nr;
            } else {
                nr = NULL;
            }
        }
        hp = hp ? (openmatch ? openmatch : shp->var_base) : shp->var_tree;
    }
    if (nr == np) {
        if (shp->namespace && nv_dict(shp->namespace) == hp) {
            errormsg(SH_DICT, ERROR_exit(1), e_selfref, nv_name(np));
            __builtin_unreachable();
        }
        // Bind to earlier scope, or add to global scope.
        if (!(hp = dtvnext(hp)) || (nq = nv_search_namval(np, hp, NV_ADD)) == np) {
            errormsg(SH_DICT, ERROR_exit(1), e_selfref, nv_name(np));
            __builtin_unreachable();
        }
        if (nv_isarray(nq)) nv_putsub(nq, NULL, 0, ARRAY_UNDEF);
    }
    if (nq && !nv_isnull(nq)) last_table = shp->last_table;
    if (nq && ep && nv_isarray(nq) && !nv_getsub(nq)) {
        if (!nv_arrayptr(nq)) {
            nv_putsub(nq, "1", 0, ARRAY_FILL);
            _nv_unset(nq, NV_RDONLY);
        }
        nv_endsubscript(nq, ep - 1, NV_ARRAY, nq->nvshell);
    }
    if (!nr) {
        shp->last_root = NULL;
        nr = nq = nv_open(cp, hp, flags);
        if (shp->last_root) hp = shp->last_root;
    }
    shp->namref_root = NULL;
    if (shp->last_root == shp->var_tree && root != shp->var_tree) {
        _nv_unset(np, NV_RDONLY);
        nv_onattr(np, NV_REF);
        errormsg(SH_DICT, ERROR_exit(1), e_globalref, nv_name(np));
        __builtin_unreachable();
    }
    shp->instance = 1;
    if (nq && !ep && (ap = nv_arrayptr(nq)) && !(ap->flags & (ARRAY_UNDEF | ARRAY_SCAN))) {
        ep = nv_getsub(nq);
    }
    if (ep) {
        // Cause subscript evaluation and return result.
        assert(nq);
        if (nv_isarray(nq)) {
            ep = nv_getsub(nq);
        } else {
            int n;
            ep[n = strlen(ep) - 1] = 0;
            nv_putsub(nr, ep, 0, ARRAY_FILL);
            ep[n] = ']';
            nq = nv_opensub(nr);
            if (nq) {
                ep = NULL;
            } else {
                ep = nv_getsub(nq = nr);
            }
        }
    }
    assert(nq);
    shp->instance = 0;
    shp->last_root = root;
    _nv_unset(np, 0);
    nv_delete(np, NULL, 0);
    struct Namref *nrp = calloc(1, sizeof(struct Namref) + sizeof(Dtlink_t));
    STORE_VT(np->nvalue, nrp, nrp);
    nrp->np = nq;
    nrp->root = hp;
    nrp->oldnp = shp->oldnp;
    if (ep) {
        nrp->sub = strdup(ep);
    }
    nrp->table = last_table;
    nv_onattr(np, NV_REF | NV_NOFREE);
    if (!Refdict) {
        // Note that this initialization of the NullNode singleton is predicated on the Refdict
        // singleton. Normally singletons should only ever be initialized once. But this is an
        // unusual use case.
        nv_setattr(&NullNode, NV_RDONLY);
        NullNode.nvshell = nq->nvshell;
        Refdict = dtopen(&_Refdisc, Dtobag);
    }
    dtinsert(Refdict, nrp);
}

//
// Get the scope corresponding to <index> whence uses the same values as lseeek().
//
Shscope_t *sh_getscope(Shell_t *shp, int index, int whence) {
    struct sh_scoped *sp, *topmost;

    if (whence == SEEK_CUR) {
        sp = &shp->st;
    } else {
        if ((struct sh_scoped *)shp->topscope != shp->st.self) {
            topmost = (struct sh_scoped *)shp->topscope;
        } else {
            topmost = &(shp->st);
        }
        sp = topmost;
        if (whence == SEEK_SET) {
            int n = 0;
            while ((sp = sp->prevst)) n++;
            index = n - index;
            sp = topmost;
        }
    }
    if (index < 0) return NULL;
    while (index-- && (sp = sp->prevst)) {
        ;  // empty loop
    }
    return (Shscope_t *)sp;  // here be dragons
}

//
// Make <scoped> the top scope and return previous scope.
//
Shscope_t *sh_setscope(Shell_t *shp, Shscope_t *scope) {
    Shscope_t *old = (Shscope_t *)shp->st.self;
    *shp->st.self = shp->st;
    shp->st = *((struct sh_scoped *)scope);
    shp->var_tree = scope->var_tree;
    STORE_VT(VAR_sh_file->nvalue, const_cp, shp->st.filename);
    STORE_VT(VAR_sh_fun->nvalue, const_cp, shp->st.funname);
    return old;
}

void sh_unscope(Shell_t *shp) {
    Dt_t *root = shp->var_tree;
    Dt_t *dp = dtview(root, NULL);
    if (dp) {
        table_unset(shp, root, NV_RDONLY | NV_NOSCOPE, dp);
        if (shp->st.real_fun && dp == shp->st.real_fun->sdict) {
            dp = dtview(dp, NULL);
            shp->st.real_fun->sdict->view = dp;
        }
        shp->var_tree = dp;
        dtclose(root);
    }
}

//
// The inverse of creating a reference node.
//
void nv_unref(Namval_t *np) {
    Namval_t *nq;

    if (!nv_isref(np)) return;
    nv_offattr(np, NV_NOFREE | NV_REF);
    if (!FETCH_VT(np->nvalue, nrp)) return;
    nq = nv_refnode(np);
    if (Refdict) {
        if (FETCH_VT(np->nvalue, nrp)->sub) free(FETCH_VT(np->nvalue, nrp)->sub);
        dtremove(Refdict, FETCH_VT(np->nvalue, nrp));
    }
    free(FETCH_VT(np->nvalue, nrp));
    STORE_VT(np->nvalue, const_cp, strdup(nv_name(nq)));

    for (Namfun_t *fp = nq->nvfun; fp; fp = fp->next) {
        if (fp->disc == &OPTIMIZE_disc) {
            optimize_clear(nq, fp);
            return;
        }
    }
}

char *nv_name(const Namval_t *np) {
    Shell_t *shp = sh_ptr(np);
    Namval_t *table = NULL;
    Namfun_t *fp = NULL;
    char *cp;

    if (!shp) shp = sh_getinterp();

    if (is_abuiltin(np) || is_afunction(np)) {
        if (shp->namespace && is_afunction(np)) {
            char *name = nv_name(shp->namespace);
            int n = strlen(name);
            if (strncmp(np->nvname, name, n) == 0 && np->nvname[n] == '.') {
                return np->nvname + n + 1;
            }
        }
        return np->nvname;
    }
    if (!np->nvname) goto skip;
    if (!nv_isattr(np, NV_MINIMAL | NV_EXPORT) && np->nvenv) {
        Namval_t *nq = shp->last_table;
        Namval_t *mp = np->nvenv;
        if (mp && (!mp->nvname || *mp->nvname == 0)) mp = NULL;
        if (mp && (!strchr(np->nvname, '.') || (np->nvfun && nv_type(mp)) ||
                   (nv_isarray(mp) && (cp = nv_getsub(mp)) && strcmp(np->nvname, cp) == 0))) {
            if (np == shp->last_table) shp->last_table = NULL;
            if (nv_isarray(mp)) {
                sfprintf(shp->strbuf, "%s[%s]", nv_name(mp), np->nvname);
            } else {
                sfprintf(shp->strbuf, "%s.%s", nv_name(mp), np->nvname);
            }
            shp->last_table = nq;
            return sfstruse(shp->strbuf);
        }
    }
    if (nv_istable(np)) {
        shp->last_table = nv_parent(np);
    } else if (!nv_isref(np)) {
    skip:
        for (fp = np->nvfun; fp; fp = fp->next) {
            if (fp->disc && fp->disc->namef) {
                if (np == shp->last_table) shp->last_table = NULL;
                return (*fp->disc->namef)(np, fp);
            }
        }
    }

    // The `if (!np->nvname) goto skip;` above means we can reach this juncture with `np->nvname`
    // being the NULL pointer. Tell the compiler and lint tools this can't happen via an assert.
    //
    // TODO: Rewrite this code to avoid the need for the "goto" above and this explicit suppression
    // of a logic warning.
    assert(np->nvname);

    table = shp->last_table;
    if (!table) return np->nvname;
    if (table == shp->namespace) return np->nvname;
    if (table == np) return np->nvname;
    if (*np->nvname == '.') return np->nvname;

    cp = nv_name(table);
    sfprintf(shp->strbuf, "%s.%s", cp, np->nvname);
    return sfstruse(shp->strbuf);
}

Namval_t *nv_lastdict(void *context) {
    Shell_t *shp = context;
    return shp->last_table;
}

bool nv_isnull(Namval_t *np) {
    if (FETCH_VT(np->nvalue, vp)) return false;
    // Why is VAR_IFS special-cased but not any of the other shell builtin vars?
    if (np == VAR_IFS) return true;
    if (nv_isattr(np, NV_INT16) == NV_INT16 && !np->nvfun) {
        return nv_isattr(np, NV_NOTSET) == NV_NOTSET;
    }
    if (!nv_attr(np) || nv_isattr(np, NV_NOTSET) != NV_NOTSET) {
        return !np->nvfun || !np->nvfun->disc || !nv_hasget(np);
    }
    return false;
}

#undef nv_setsize
int nv_setsize(Namval_t *np, int size) {
    int oldsize = nv_size(np);
    if (size >= 0) np->nvsize = size << 1;
    return oldsize;
}

#undef nv_unset

void nv_unset(Namval_t *np) {
    _nv_unset(np, 0);
    return;
}

//
// Removing of Shell variable names, aliases, and functions is performed here. Non-existent
// items being deleted give non-zero exit status. This is used by `unset` and `unalias`.
//
int nv_unall(char **names, bool aliases, nvflag_t nvflags, Dt_t *troot, Shell_t *shp) {
    Namval_t *np;
    volatile int r = 0;
    Dt_t *dp;
    int isfun, jmpval;
    checkpt_t buff;

    sh_pushcontext(shp, &buff, 1);
    while (*names) {
        char *name = *names++;
        jmpval = sigsetjmp(buff.buff, 0);
        np = NULL;
        if (jmpval == 0) {
            if (shp->namespace && troot != shp->var_tree) {
                np = sh_fsearch(shp, name, nvflags ? NV_NOSCOPE : 0);
            }
            if (!np) np = nv_open(name, troot, NV_NOADD | nvflags);
        } else {
            r = 1;
            continue;
        }
        if (np) {
            if (is_abuiltin(np) || nv_isattr(np, NV_RDONLY)) {
                if (nv_isattr(np, NV_RDONLY)) {
                    errormsg(SH_DICT, ERROR_warn(0), e_readonly, nv_name(np));
                }
                r = 1;
                continue;
            }
            isfun = is_afunction(np);
            if (troot == shp->var_tree) {
                if (nv_isarray(np) && name[strlen(name) - 1] == ']' && !nv_getsub(np)) {
                    r = 1;
                    continue;
                }

                if (shp->subshell) np = sh_assignok(np, 0);
            }
            if (!nv_isnull(np) || nv_size(np) || nv_isattr(np, ~(NV_MINIMAL | NV_NOFREE))) {
                _nv_unset(np, 0);
            }
            if (troot == shp->var_tree && shp->st.real_fun && (dp = shp->var_tree->walk) &&
                dp == shp->st.real_fun->sdict) {
                nv_delete(np, dp, NV_NOFREE);
            } else if (isfun) {
                struct Ufunction *rp = FETCH_VT(np->nvalue, rp);
                if (!rp || !rp->running) nv_delete(np, troot, 0);
            } else if (aliases) {
                // Alias has been unset by call to _nv_unset, remove it from the tree.
                nv_delete(np, troot, 0);
            }
#if 0
            // Causes unsetting local variable to expose global.
            else if(shp->var_tree == troot && shp->var_tree != shp->var_base &&
                    nv_search_namval(np, shp->var_tree, NV_NOSCOPE)) {
                    nv_delete(np,shp->var_tree,0);
            }
#endif
            else {
                nv_close(np);
            }

        } else if (aliases) {
            // Alias not found
            sfprintf(sfstderr, sh_translate(e_noalias), name);
            r = 1;
        }
    }
    sh_popcontext(shp, &buff);
    return r;
}
