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
#define putenv ___putenv

#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wchar.h>

#include "defs.h"

#include "argnod.h"
#include "ast.h"
#include "ast_api.h"
#include "ast_assert.h"
#include "cdt.h"
#include "error.h"
#include "fault.h"
#include "lexstates.h"
#include "name.h"
#include "nvapi.h"
#include "sfio.h"
#include "shellapi.h"
#include "stak.h"
#include "stk.h"
#include "variables.h"

#define NVCACHE 8  // must be a power of 2

// This var used to be writable but was treated as if it was immutable except for one assignment
// that failed to validate it was modifying only the first, and only, char. So we now make it
// truly immutable to catch any attempts to modify it.
static const char *EmptyStr = "";

static char *savesub = NULL;
static Namval_t NullNode;
static Dt_t *Refdict;
static Dtdisc_t _Refdisc = {offsetof(struct Namref, np), sizeof(struct Namval_t *),
                            sizeof(struct Namref)};

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

#if NVCACHE
struct Namcache {
    struct Cache_entry {
        Dt_t *root;
        Dt_t *last_root;
        char *name;
        Namval_t *np;
        Namval_t *last_table;
        Namval_t *namespace;
        int flags;
        short size;
        short len;
    } entries[NVCACHE];
    short index;
    short ok;
};
static struct Namcache nvcache;
#endif  // NVCACHE

char nv_local = 0;
static_fn void (*nullscan)(Namval_t *, void *);

#if (SFIO_VERSION <= 20010201L)
#define _data data
#endif

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
    char *name = 0;

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
        if (strncmp(np->nvname, name, i)) return (np);
    }
    if (sp->rp && sp->numnodes) {
        // Check for a redefine.
        if (name && np->nvname[i] == '.' && np->nvname[i + 1] == '_' && np->nvname[i + 2] == 0) {
            sp->rp = 0;
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
    if (remove) return (np);
    if (sp->numnodes == sp->maxnodes) {
        sp->maxnodes += 20;
        sp->nodes = (Namval_t **)realloc(sp->nodes, sizeof(Namval_t *) * sp->maxnodes);
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
        if (strncmp(cp, name, len) == 0 && (cp[len] == 0 || cp[len] == '=')) return (arg);
    }
    return 0;
}

//
// Perform parameter assignment for a linked list of parameters.
// <flags> contains attributes for the parameters.
//
Namval_t **sh_setlist(Shell_t *shp, struct argnod *arg, int flags, Namval_t *typ) {
    char *cp;
    Namval_t *np, *mp;
    char *trap = shp->st.trap[SH_DEBUGTRAP];
    char *prefix = shp->prefix;
    int traceon = (sh_isoption(shp, SH_XTRACE) != 0);
    int array = (flags & (NV_ARRAY | NV_IARRAY));
    Namarr_t *ap;
    Namval_t node, **nodelist = 0, **nlp = 0;
    struct Namref nr;
    int maketype = flags & NV_TYPE;
    struct sh_type shtp;

    memset(&node, 0, sizeof(node));
    memset(&nr, 0, sizeof(nr));

    if (maketype) {
        shtp.previous = shp->mktype;
        shp->mktype = (void *)&shtp;
        shtp.numnodes = 0;
        shtp.maxnodes = 20;
        shtp.rp = 0;
        shtp.nodes = (Namval_t **)malloc(shtp.maxnodes * sizeof(Namval_t *));
    }
    if (shp->namespace && nv_dict(shp->namespace) == shp->var_tree) flags |= NV_NOSCOPE;
    flags &= ~(NV_TYPE | NV_ARRAY | NV_IARRAY);
    if (sh_isoption(shp, SH_ALLEXPORT)) flags |= NV_EXPORT;
    if (shp->prefix) {
        flags &= ~(NV_IDENT | NV_EXPORT);
        flags |= NV_VARNAME;
    } else {
        shp->prefix_root = shp->first_root = 0;
    }
    if (flags & NV_DECL) {
        struct argnod *ap;
        int n = 0;
        for (ap = arg; ap; ap = ap->argnxt.ap) n++;
        nlp = nodelist = (Namval_t **)stkalloc(shp->stk, (n + 1) * sizeof(Namval_t *));
        nodelist[n] = 0;
    }
    for (; arg; arg = arg->argnxt.ap) {
        Namval_t *nq = 0;
        shp->used_pos = 0;
        if (arg->argflag & ARG_MAC) {
            shp->prefix = 0;
            cp = sh_mactrim(shp, arg->argval, (flags & NV_NOREF) ? -3 : -1);
            shp->prefix = prefix;
        } else {
            stkseek(shp->stk, 0);
            if (*arg->argval == 0 && arg->argchn.ap &&
                !(arg->argflag & ~(ARG_APPEND | ARG_QUOTED | ARG_MESSAGE | ARG_ARRAY))) {
                int flag = (NV_VARNAME | NV_ARRAY | NV_ASSIGN);
                int sub = 0;
                struct fornod *fp = (struct fornod *)arg->argchn.ap;
                Shnode_t *tp = fp->fortre;
                flag |= (flags & (NV_NOSCOPE | NV_STATIC | NV_FARRAY));
                if (arg->argflag & ARG_ARRAY) array |= NV_IARRAY;
                if (arg->argflag & ARG_QUOTED) {
                    cp = sh_mactrim(shp, fp->fornam, -1);
                } else {
                    cp = fp->fornam;
                }
                error_info.line = fp->fortyp - shp->st.firstline;
                if (!array && tp->tre.tretyp != TLST && tp->com.comset && !tp->com.comarg &&
                    tp->com.comset->argval[0] == 0 && tp->com.comset->argval[1] == '[')
                    array |= (tp->com.comset->argflag & ARG_MESSAGE) ? NV_IARRAY : NV_ARRAY;
                if (prefix && tp->com.comset && *cp == '[') {
                    shp->prefix = 0;
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
                    tp->com.comset = 0;
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
                if (array && (!ap || !ap->hdr.type)) {
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
                        shp->mktype = 0;
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
                    if (traceon || trap) {
                        int n = -1;
                        char *name = nv_name(np);
                        if (arg->argflag & ARG_APPEND) n = '+';
                        if (trap) {
                            sh_debug(shp, trap, name, (char *)0, argv,
                                     (arg->argflag & ARG_APPEND) | ARG_ASSIGN);
                        }
                        if (traceon) {
                            sh_trace(shp, NULL, 0);
                            sfputr(sfstderr, name, n);
                            sfwrite(sfstderr, "=( ", 3);
                            while ((cp = *argv++)) sfputr(sfstderr, sh_fmtq(cp), ' ');
                            sfwrite(sfstderr, ")\n", 2);
                        }
                    }
                    goto check_type;
                }
                if ((tp->tre.tretyp & COMMSK) == TFUN) goto skip;
                if (tp->tre.tretyp == TCOM && !tp->com.comset && !tp->com.comarg) {
                    if (!(arg->argflag & ARG_APPEND)) {
                        if (ap && ap->nelem > 0) {
                            nv_putsub(np, NULL, 0, ARRAY_SCAN);
                            if (!ap->fun && !(ap->flags & ARRAY_TREE) && !np->nvfun->next &&
                                !nv_type(np)) {
                                int nvflag = np->nvflag;
                                int nvsize = np->nvsize;
                                _nv_unset(np, NV_EXPORT);
                                np->nvflag = nvflag;
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
                        shp->typeinit = 0;
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
                                nv_putsub(np, (char *)0, 0, ARRAY_ADD | ARRAY_FILL);
                            } else if (sub >= 0) {
                                sub++;
                            }
                            if (nv_type(np)) {
                                sfprintf(shp->strbuf, "%s[%d]\0", nv_name(np), sub);
                                nq =
                                    nv_open(sfstruse(shp->strbuf), shp->var_tree, flags | NV_ARRAY);
                            }
                        }
                        if (!nv_isnull(np) && np->nvalue.cp != Empty && !nv_isvtree(np)) sub = 1;
                    } else if (((np->nvalue.cp && np->nvalue.cp != Empty) || nv_isvtree(np) ||
                                nv_arrayptr(np)) &&
                               !nv_type(np) &&
                               nv_isattr(np, NV_MINIMAL | NV_EXPORT) != NV_MINIMAL) {
                        _nv_unset(np, NV_EXPORT);
                        if (ap && ap->fun) {
                            nv_setarray(np, nv_associative);
                        } else {
                            // nq is initialized to same value as np. When _nv_unset(np, NV_EXPORT);
                            // is called, it free's memory which is later causing crash at:
                            // if (nq && nv_type(nq)) nv_checkrequired(nq);
                            // Reset nq to 0 to avoid such crashes.
                            nq = 0;
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
                    nv_putsub(np, (char *)0, sub, ARRAY_ADD | ARRAY_FILL);
                } else if (prefix) {
                    shp->prefix = stkcopy(shp->stk, nv_name(np));
                } else {
                    shp->prefix = cp;
                }
                shp->last_table = 0;
                if (shp->prefix) {
                    if (*shp->prefix == '_' && shp->prefix[1] == '.' && nv_isref(L_ARGNOD)) {
                        sfprintf(stkstd, "%s%s", nv_name(L_ARGNOD->nvalue.nrp->np),
                                 shp->prefix + 1);
                        shp->prefix = stkfreeze(stkstd, 1);
                    }
                    memset(&nr, 0, sizeof(nr));
                    memcpy(&node, L_ARGNOD, sizeof(node));
                    L_ARGNOD->nvalue.nrp = &nr;
                    nr.np = np;
                    nr.root = shp->last_root;
                    nr.table = shp->last_table;
                    L_ARGNOD->nvflag = NV_REF | NV_NOFREE;
                    L_ARGNOD->nvfun = 0;
                }
                sh_exec(shp, tp, sh_isstate(shp, SH_ERREXIT));
                if (nq && nv_type(nq)) nv_checkrequired(nq);
                if (shp->prefix) {
                    L_ARGNOD->nvalue.nrp = node.nvalue.nrp;
                    L_ARGNOD->nvflag = node.nvflag;
                    L_ARGNOD->nvfun = node.nvfun;
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
            mp = 0;
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
            char *sub = 0;
            int append = 0;
            if (nv_isarray(np)) sub = savesub;
            cp = lastdot(sp, '=', (void *)shp);
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
            maketype = 0;
            if (shp->namespace) free(shp->prefix);
            shp->prefix = 0;
            if (nr.np == np) {
                L_ARGNOD->nvalue.nrp = node.nvalue.nrp;
                L_ARGNOD->nvflag = node.nvflag;
                L_ARGNOD->nvfun = node.nvfun;
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
    stakputc('[');
    while ((c = *sub++)) {
        if (c == '[' || c == ']' || c == '\\') stakputc('\\');
        stakputc(c);
    }
    stakputc(last);
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
        if (sh_checkid(stkptr(shp->stk, last), (char *)0)) stkseek(shp->stk, stktell(shp->stk) - 2);
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

static_fn Namval_t *nv_parentnode(Namval_t *np) {
    Namval_t *mp = np;
    if (nv_istable(np)) return nv_parent(np);
    mp = nv_typeparent(np);
    if (mp) return mp;
    if ((mp = (Namval_t *)np->nvenv) && !nv_isattr(np, NV_EXPORT)) return mp;
    return np;
}

Namval_t *nv_create(const char *name, Dt_t *root, int flags, Namfun_t *dp) {
    Shell_t *shp = sh_getinterp();
    char *sub = 0, *cp = (char *)name, *sp, *xp;
    int c;
    Namval_t *np = 0, *nq = 0;
    Namfun_t *fp = 0;
    long mode, add = 0;
    int copy = 0, isref, top = 0, noscope = (flags & NV_NOSCOPE);
    int nofree = 0, level = 0, zerosub = 0;
    Namarr_t *ap;
    Namval_t *qp = 0;

    if (root == shp->var_tree) {
        if (dtvnext(root)) {
            top = 1;
        } else {
            flags &= ~NV_NOSCOPE;
        }
    }
    if (!dp->disc) copy = dp->nofree & 1;
    if (*cp == '.') cp++;
    while (1) {
        // We have to use `memmove()` rather than `strcpy()` because the buffers may overlap.
        if (zerosub && !np) memmove(sp, cp - 1, strlen(cp - 1) + 1);
        zerosub = 0;
        switch (c = *(unsigned char *)(sp = cp)) {
            case '[': {
                if (flags & NV_NOARRAY) {
                    dp->last = cp;
                    return np;
                }
                cp = nv_endsubscript((Namval_t *)0, sp, 0, (void *)shp);
                if (sp == name || sp[-1] == '.') c = *(sp = cp);
                goto skip;
            }
            case '.': {
                if (flags & NV_IDENT) return (0);
                if (root == shp->var_tree) flags &= ~NV_EXPORT;
                if (!copy && !(flags & NV_NOREF)) {
                    c = sp - name;
                    copy = cp - name;
                    dp->nofree |= 1;
                    name = copystack(shp, (const char *)0, name, (const char *)0);
                    cp = (char *)name + copy;
                    sp = (char *)name + c;
                    c = '.';
                }
                // FALL THRU
            }
            case '+':
            case '=': {
            skip:
                *sp = 0;
            }
            case 0: {
                isref = 0;
                dp->last = cp;
                mode = (c == '.' || (flags & NV_NOADD)) ? add : NV_ADD;
                if (level++ || ((flags & NV_NOSCOPE) && c != '.')) mode |= HASH_NOSCOPE;
                np = 0;
                if (top) {
                    struct Ufunction *rp;
                    if ((rp = shp->st.real_fun) && !rp->sdict && (flags & NV_STATIC)) {
                        Dt_t *dp = dtview(shp->var_tree, (Dt_t *)0);
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
                            if (!(nq = nv_search((char *)np, shp->var_base, HASH_BUCKET))) nq = np;
                            shp->last_root = shp->var_tree->walk;
                            if ((flags & NV_NOSCOPE) && *cp != '.') {
                                if (mode == 0) {
                                    root = shp->var_tree->walk;
                                } else {
                                    nv_delete(np, (Dt_t *)0, NV_NOFREE);
                                    np = 0;
                                }
                            }
                        } else {
                            if (shp->var_tree->walk) root = shp->var_tree->walk;
                            flags |= NV_NOSCOPE;
                            noscope = 1;
                        }
                    }
                    if (rp && rp->sdict && (flags & NV_STATIC)) {
                        root = rp->sdict;
                        if (np && shp->var_tree->walk == shp->var_tree) {
                            _nv_unset(np, 0);
                            nv_delete(np, shp->var_tree, 0);
                            np = 0;
                        }
                        if (!np || shp->var_tree->walk != root) {
                            np = nv_search(name, root, HASH_NOSCOPE | NV_ADD);
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
                                ((np->nvfun = nv_cover(nq)) || nq == OPTINDNOD)) {
                                np->nvname = nq->nvname;
                                if (shp->namespace && nv_dict(shp->namespace) == shp->var_tree &&
                                    nv_isattr(nq, NV_EXPORT)) {
                                    nv_onattr(np, NV_EXPORT);
                                }
                                if (nq == OPTINDNOD) {
                                    np->nvfun = nq->nvfun;
                                    np->nvalue.lp = (&shp->st.optindex);
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
                    np->nvenv = (char *)shp->oldnp;
                }
                shp->oldnp = np;
                if (isref) {
#if NVCACHE
                    nvcache.ok = 0;
#endif
                    if (nv_isattr(np, NV_TABLE) || c == '.') {  // don't optimize
                        shp->argaddr = 0;
                    } else if ((flags & NV_NOREF) && (c != '[' && *cp != '.')) {
                        if (c && !(flags & NV_NOADD)) nv_unref(np);
                        return np;
                    }
                    while (nv_isref(np) && np->nvalue.cp) {
                        root = nv_reftree(np);
                        shp->last_root = root;
                        shp->last_table = nv_reftable(np);
                        sub = nv_refsub(np);
                        shp->oldnp = nv_refoldnp(np);
                        if (shp->oldnp) shp->oldnp = (Namval_t *)shp->oldnp->nvenv;
                        np = nv_refnode(np);
                        if (sub && c != '.') nv_putsub(np, sub, 0L, 0);
                        flags |= NV_NOSCOPE;
                        noscope = 1;
                    }
                    shp->first_root = root;
                    if (nv_isref(np) && (c == '[' || c == '.' || !(flags & NV_ASSIGN))) {
                        errormsg(SH_DICT, ERROR_exit(1), e_noref, nv_name(np));
                        __builtin_unreachable();
                    }
                    if (sub && c == 0) {
                        if (flags & NV_ARRAY) {
                            ap = nv_arrayptr(np);
                            nq = nv_opensub(np);
                            if ((flags & NV_ASSIGN) && (!nq || nv_isnull(nq))) ap->nelem++;
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
                        char *xp = 0;
                        ssize_t xlen;
                        c = (cp - sp);
                        // Eliminate namespace name.
                        if (shp->last_table && !nv_type(shp->last_table)) {
                            xp = nv_name(shp->last_table);
                            xlen = strlen(xp);
                        }
                        cp = nv_name(np);
                        if (xp && strncmp(cp, xp, xlen) && cp[xlen] == '.') cp += xlen + 1;
                        copy = strlen(cp);
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
                    nv_local = 1;
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
                        int n = 0;
                        sub = 0;
                        mode &= ~HASH_NOSCOPE;
                        if (c == '[') {
                            Namval_t *table;
                            n = mode | nv_isarray(np);
                            if (!mode && (flags & NV_ARRAY) && ((c = sp[1]) == '*' || c == '@') &&
                                sp[2] == ']') {
                                // Not implemented yet.
                                dp->last = cp;
                                return np;
                            }
                            if ((n & NV_ADD) && (flags & NV_ARRAY)) n |= ARRAY_FILL;
                            if (flags & NV_ASSIGN) n |= NV_ADD | ARRAY_FILL;
                            table = shp->last_table;
                            cp = nv_endsubscript(np, sp, n | (flags & (NV_ASSIGN | NV_FARRAY)),
                                                 np->nvshell);
                            shp->last_table = table;
#if 0
						if(scan)
							nv_putsub(np,NULL,0,ARRAY_SCAN);
#endif
                        } else {
                            cp = sp;
                        }
                        if ((c = *cp) == '.' || (c == '[' && nv_isarray(np)) || (n & ARRAY_FILL) ||
                            ((ap || (flags & NV_ASSIGN)) && (flags & NV_ARRAY))) {
                            int m = cp - sp;
                            sub = m ? nv_getsub(np) : 0;
                            if (!sub) {
                                if (m && !(n & NV_ADD) && *cp != '.') return (0);
                                zerosub = 1;
                                sub = "0";
                            }
                            n = strlen(sub) + 2;
                            if (!copy) {
                                copy = cp - name;
                                dp->nofree |= 1;
                                name = copystack(shp, (const char *)0, name, (const char *)0);
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
                            nv_putsub(np, (char *)0, n, 0);
                        } else if (n == 0 && (c == 0 || (c == '[' && !nv_isarray(np)))) {
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
                            nq = 0;
                            if ((tp = nv_type(np)) && nv_hasdisc(np, &ENUM_disc)) goto enumfix;
                            if (ap && ap->table && tp) nq = nv_search(sub, ap->table, 0);
                            if (!nq && !(nq = nv_opensub(np))) {
                                Namarr_t *ap = nv_arrayptr(np);
                                if (!sub && (flags & NV_NOADD)) return 0;
                                n = mode | ((flags & NV_NOADD) ? 0 : NV_ADD);
                                if (!(n & NV_ADD) && ap && tp) n |= NV_ADD;

                                if (!ap && (n & NV_ADD)) {
                                    nv_putsub(np, sub, 0, ARRAY_FILL);
                                    ap = nv_arrayptr(np);
                                }
                                if (n && ap && !ap->table) {
                                    ap->table = dtopen(&_Nvdisc, Dtoset);
                                    dtuserdata(ap->table, shp, 1);
                                }
                                if (ap && ap->table && (nq = nv_search(sub, ap->table, n))) {
                                    nq->nvenv = (char *)np;
                                }
                                if (nq && nv_isnull(nq)) nq = nv_arraychild(np, nq, c);
                            }
                            if (nq) {
                                if (c == '.' && !nv_isvtree(nq)) {
                                    if (flags & NV_NOADD) return (0);
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
                        if (c == 0 && (flags & NV_MOVE)) return (np);
                        nv_putsub(np, NULL, 0, ARRAY_UNDEF);
                    }
                    nv_onattr(np, nofree);
                    nofree = 0;
                    if (np) qp = np;
                enumfix:
                    if (c == '.' && (fp = np->nvfun)) {
                        for (; fp; fp = fp->next) {
                            if (fp->disc && fp->disc->createf) break;
                        }
                        if (fp) {
                            if ((nq = (*fp->disc->createf)(np, cp + 1, flags, fp)) == np) {
                                add = NV_ADD;
                                shp->last_table = 0;
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
            case '_': {
                if (cp[1] == '_' && (cp[2] == 0 || cp[2] == '.') && shp->oldnp) {
                    cp += 2;
                    dp->last = cp;
#if NVCACHE
                    nvcache.ok = 0;
#endif
                    shp->oldnp = np = nv_parentnode(shp->oldnp);
                    if (*cp == 0) return np;
                    sp = nv_name(np);
                    c = strlen(sp);
                    dp->nofree |= 1;
                    name = copystack(shp, (const char *)sp, cp, (const char *)0);
                    cp = (char *)name + c + 1;
                    break;
                }
                // FALL THRU
            }
            default: {
                shp->oldnp = np ? nq : qp;
                qp = 0;
                dp->last = cp;
                if ((c = mbchar(cp)) && !isaletter(c)) return (np);
                while (xp = cp, c = mbchar(cp), isaname(c)) {
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
void nv_delete(Namval_t *np, Dt_t *root, int flags) {
#if NVCACHE
    int c;
    struct Cache_entry *xp;
    for (c = 0, xp = nvcache.entries; c < NVCACHE; xp = &nvcache.entries[++c]) {
        if (xp->np == np) xp->root = 0;
    }
#endif  // NVCACHE
    if (!np && !root && flags == 0) {
        if (Refdict) dtclose(Refdict);
        Refdict = 0;
        return;
    }
    if (root || !(flags & NV_NOFREE)) {
        if (!(flags & NV_FUNCTION) && Refdict) {
            Namval_t **key = &np;
            struct Namref *rp;
            while ((rp = (struct Namref *)dtmatch(Refdict, (void *)key))) {
                if (rp->sub) free(rp->sub);
                rp->sub = 0;
                rp = dtremove(Refdict, (void *)rp);
                if (rp) rp->np = &NullNode;
            }
        }
    }
    if (root) {
        if (dtdelete(root, np)) {
            if (!(flags & NV_NOFREE) &&
                ((flags & NV_FUNCTION) || !nv_subsaved(np, flags & NV_TABLE))) {
                Namarr_t *ap;
                if (nv_isarray(np) && np->nvfun && (ap = nv_arrayptr(np)) && array_assoc(ap)) {
                    while (nv_associative(np, 0, NV_ANEXT)) nv_associative(np, 0, NV_ADELETE);
                    nv_associative(np, 0, NV_AFREE);
                    free(np->nvfun);
                }
                free(np);
            }
        }
#if 0
		else
		{
			sfprintf(sfstderr,"%s not deleted\n",nv_name(np));
			sfsync(sfstderr);
		}
#endif
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
Namval_t *nv_open(const char *name, Dt_t *root, int flags) {
    Shell_t *shp = sh_getinterp();
    char *cp = (char *)name;
    int c;
    Namval_t *np = 0;
    Namfun_t fun;
    int append = 0;
    const char *msg = e_varname;
    char *fname = 0;
    int offset = stktell(shp->stk);
    Dt_t *funroot;
#if NVCACHE
    struct Cache_entry *xp;
#endif

    memset(&fun, 0, sizeof(fun));
    shp->openmatch = 0;
    shp->last_table = 0;
    if (!name) return 0;
    sh_stats(STAT_NVOPEN);
    if (!root) root = shp->var_tree;
    shp->last_root = root;
    if (root == shp->fun_tree) {
        flags |= NV_NOREF;
        msg = e_badfun;
        if (strchr(name, '.')) {
            name = cp = copystack(shp, 0, name, (const char *)0);
            fname = strrchr(cp, '.');
            *fname = 0;
            fun.nofree |= 1;
            flags &= ~NV_IDENT;
            funroot = root;
            root = shp->var_tree;
        }
    } else if (!(flags & (NV_IDENT | NV_VARNAME | NV_ASSIGN))) {
        long mode = ((flags & NV_NOADD) ? 0 : NV_ADD);
        if (flags & NV_NOSCOPE) mode |= HASH_SCOPE | HASH_NOSCOPE;
        np = nv_search(name, root, mode);
        if (np && !(flags & NV_REF)) {
            while (nv_isref(np)) {
                shp->last_table = nv_reftable(np);
                np = nv_refnode(np);
            }
        }
        return np;
    } else if (shp->prefix && (flags & NV_ASSIGN)) {
        name = cp = copystack(shp, shp->prefix, name, (const char *)0);
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
        shp->last_table = 0;
    }
    c = !isaletter(c);
    if (c) goto skip;
#if NVCACHE
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
#endif  // NVCACHE
#if SHOPT_BASH
    if (root == shp->fun_tree && sh_isoption(shp, SH_BASH)) {
        c = ((flags & NV_NOSCOPE) ? HASH_NOSCOPE : 0) | ((flags & NV_NOADD) ? 0 : NV_ADD);
        np = nv_search(name, root, c);
        cp = Empty;
    } else
#endif  // SHOPT_BASH
    {
        np = nv_create(name, root, flags, &fun);
        cp = fun.last;
    }
#if NVCACHE
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
#endif  // NVCACHE
    if (fname) {
        c = ((flags & NV_NOSCOPE) ? HASH_NOSCOPE : 0) | ((flags & NV_NOADD) ? 0 : NV_ADD);
        *fname = '.';
        np = nv_search(name, funroot, c);
        *fname = 0;
    } else {
        if (*cp == '.' && cp[1] == '.') {
            append |= NV_NODISC;
            cp += 2;
        }
        if (*cp == '+' && cp[1] == '=') {
            append |= NV_APPEND;
            cp++;
        }
    }
    c = *cp;
skip:
    if (np && shp->mktype) np = nv_addnode(np, 0);
    if (c == '=' && np && (flags & NV_ASSIGN)) {
        cp++;
        if (sh_isstate(shp, SH_INIT)) {
            nv_putval(np, cp, NV_RDONLY);
            if (np == PWDNOD) nv_onattr(np, NV_TAGGED);
        } else {
            char *sub = 0, *prefix = shp->prefix;
            Namval_t *mp;
            Namarr_t *ap;
            int isref;
            shp->prefix = 0;
            if ((flags & NV_STATIC) && !shp->mktype) {
                if (!nv_isnull(np)) {
                    shp->prefix = prefix;
                    return np;
                }
            }
            isref = nv_isref(np);
            if (sh_isoption(shp, SH_XTRACE) && nv_isarray(np)) sub = nv_getsub(np);
            c = msg == e_aliname ? 0 : (append | (flags & NV_EXPORT));
            if (isref) nv_offattr(np, NV_REF);
            if (!append && (flags & NV_UNJUST)) {
                if (!np->nvfun) _nv_unset(np, NV_EXPORT);
            }
            if (flags & NV_MOVE) {
                ap = nv_arrayptr(np);
                if (ap) {
                    mp = nv_opensub(np);
                    if (mp) {
                        np = mp;
                    } else if (!array_assoc(ap) &&
                               (mp = nv_open(cp, shp->var_tree,
                                             NV_NOFAIL | NV_VARNAME | NV_NOARRAY | NV_NOASSIGN |
                                                 NV_NOADD)) &&
                               nv_isvtree(np)) {
                        ap->flags |= ARRAY_TREE;
                        nv_putsub(np, (char *)0, nv_aindex(np), ARRAY_ADD);
                        np = nv_opensub(np);
                        ap->flags &= ~ARRAY_TREE;
                    }
                }
                _nv_unset(np, NV_EXPORT);
                np->nvalue.cp = strdup(cp);
            } else {
                nv_putval(np, cp, c);
            }
            if (isref) {
                if (nv_search((char *)np, shp->var_base, HASH_BUCKET)) {
                    shp->last_root = shp->var_base;
                }
                nv_setref(np, (Dt_t *)0, NV_VARNAME);
            }
            savesub = sub;
            shp->prefix = prefix;
        }
        nv_onattr(np, flags & NV_ATTRIBUTES);
    } else if (c) {
        if (flags & NV_NOFAIL) return (0);
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

static_fn int ja_size(char *, int, int);
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
void nv_putval(Namval_t *np, const void *vp, int flags) {
    const char *string = vp;
    Shell_t *shp = sh_ptr(np);
    const char *sp = string;
    char *cp;
    union Value *up;
    int size = 0;
    int dot;
    int was_local = nv_local;

    if ((flags & NV_APPEND) && nv_isnull(np) && shp->var_tree->view) {
        Namval_t *mp = nv_search(np->nvname, shp->var_tree->view, 0);
        if (mp) nv_clone(mp, np, 0);
    }
    if (!(flags & NV_RDONLY) && nv_isattr(np, NV_RDONLY) && np->nvalue.cp != Empty) {
        errormsg(SH_DICT, ERROR_exit(1), e_readonly, nv_name(np));
        __builtin_unreachable();
    }
    // The following could cause the shell to fork if assignment would cause a side effect.
    shp->argaddr = 0;
    if (shp->subshell && !nv_local && !(flags & NV_RDONLY)) np = sh_assignok(np, 1);
    if (np->nvfun && np->nvfun->disc && !(flags & NV_NODISC) && !nv_isref(np)) {
        // This function contains disc.
        if (!nv_local) {
            nv_local = 1;
            nv_putv(np, sp, flags, np->nvfun);
            if (sp && ((flags & NV_EXPORT) || nv_isattr(np, NV_EXPORT))) sh_envput(shp->env, np);
            return;
        }
        // Called from disc, assign the actual value.
    }
    flags &= ~NV_NODISC;
    nv_local = 0;
    if (nv_isattr(np, NV_NOTSET) == NV_NOTSET) nv_offattr(np, NV_BINARY);
    if (flags & (NV_NOREF | NV_NOFREE)) {
        if (np->nvalue.cp && np->nvalue.cp != sp && !nv_isattr(np, NV_NOFREE)) {
            free(np->nvalue.sp);
        }
        np->nvalue.cp = (char *)sp;
        nv_setattr(np, (flags & ~NV_RDONLY) | NV_NOFREE);
        return;
    }
    up = &np->nvalue;
    if (np->nvalue.up && nv_isarray(np) && nv_arrayptr(np)) up = np->nvalue.up;
    if (up && up->cp == Empty) up->cp = 0;
    if (nv_isattr(np, NV_EXPORT)) nv_offattr(np, NV_IMPORT);
    if (nv_isattr(np, NV_INTEGER)) {
        if (nv_isattr(np, NV_DOUBLE) == NV_DOUBLE) {
            if (nv_isattr(np, NV_LONG) && sizeof(double) < sizeof(Sfdouble_t)) {
                Sfdouble_t ld, old = 0;
                if (flags & NV_INTEGER) {
                    if (flags & NV_LONG) {
                        ld = *((Sfdouble_t *)sp);
                    } else if (flags & NV_SHORT) {
                        ld = *((float *)sp);
                    } else {
                        ld = *((double *)sp);
                    }
                } else {
                    ld = sh_arith(shp, sp);
                }
                if (!up->ldp) {
                    up->ldp = calloc(1, sizeof(Sfdouble_t));
                } else if (flags & NV_APPEND) {
                    old = *(up->ldp);
                }
                *(up->ldp) = old ? ld + old : ld;
            } else {
                double d, od = 0;
                if (flags & NV_INTEGER) {
                    if (flags & NV_LONG) {
                        d = (double)(*(Sfdouble_t *)sp);
                    } else if (flags & NV_SHORT) {
                        d = (double)(*(float *)sp);
                    } else {
                        d = *(double *)sp;
                    }
                } else {
                    d = sh_arith(shp, sp);
                }
                if (!up->dp) {
                    if (nv_isattr(np, NV_SHORT)) {
                        up->fp = calloc(1, sizeof(float));
                    } else {
                        up->dp = calloc(1, sizeof(double));
                    }
                } else if (flags & NV_APPEND) {
                    od = *(up->dp);
                }
                if (nv_isattr(np, NV_SHORT)) {
                    *(up->fp) = (float)od ? d + od : d;
                } else {
                    *(up->dp) = od ? d + od : d;
                }
            }
        } else {
            if (nv_isattr(np, NV_LONG) && sizeof(int32_t) < sizeof(Sflong_t)) {
                Sflong_t ll = 0, oll = 0;
                if (flags & NV_INTEGER) {
                    if ((flags & NV_DOUBLE) == NV_DOUBLE) {
                        if (flags & NV_LONG) {
                            ll = *((Sfdouble_t *)sp);
                        } else if (flags & NV_SHORT) {
                            ll = *((float *)sp);
                        } else {
                            ll = *((double *)sp);
                        }
                    } else if (nv_isattr(np, NV_UNSIGN)) {
                        if (flags & NV_LONG) {
                            ll = *((Sfulong_t *)sp);
                        } else if (flags & NV_SHORT) {
                            ll = *((uint16_t *)sp);
                        } else {
                            ll = *((uint32_t *)sp);
                        }
                    } else {
                        if (flags & NV_LONG) {
                            ll = *((Sflong_t *)sp);
                        } else if (flags & NV_SHORT) {
                            ll = *((uint16_t *)sp);
                        } else {
                            ll = *((uint32_t *)sp);
                        }
                    }
                } else if (sp) {
                    ll = (Sflong_t)sh_arith(shp, sp);
                }
                if (!up->llp) {
                    up->llp = calloc(1, sizeof(Sflong_t));
                } else if (flags & NV_APPEND) {
                    oll = *(up->llp);
                }
                *(up->llp) = ll + oll;
            } else {
                int32_t l = 0, ol = 0;
                if (flags & NV_INTEGER) {
                    if ((flags & NV_DOUBLE) == NV_DOUBLE) {
                        Sflong_t ll;
                        if (flags & NV_LONG) {
                            ll = *((Sfdouble_t *)sp);
                        } else if (flags & NV_SHORT) {
                            ll = *((float *)sp);
                        } else {
                            ll = *((double *)sp);
                        }
                        l = (int32_t)ll;
                    } else if (nv_isattr(np, NV_UNSIGN)) {
                        if (flags & NV_LONG) {
                            l = *((Sfulong_t *)sp);
                        } else if (flags & NV_SHORT) {
                            l = *((uint16_t *)sp);
                        } else {
                            l = *(uint32_t *)sp;
                        }
                    } else {
                        if (flags & NV_LONG) {
                            l = *((Sflong_t *)sp);
                        } else if (flags & NV_SHORT) {
                            l = *((int16_t *)sp);
                        } else {
                            l = *(int32_t *)sp;
                        }
                    }
                } else if (sp) {
                    Sfdouble_t ld = sh_arith(shp, sp);
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
                    if (flags & NV_APPEND) s = ptr ? *up->i16p : up->i16;
                    if (ptr) {
                        *(up->i16p) = s + (int16_t)l;
                    } else {
                        up->i16 = s + (int16_t)l;
                    }
                    nv_onattr(np, NV_NOFREE);
                } else {
                    if (!up->lp) {
                        up->lp = calloc(1, sizeof(int32_t));
                    } else if (flags & NV_APPEND) {
                        ol = *(up->lp);
                    }
                    *(up->lp) = l + ol;
                }
            }
        }
    } else {
        char *tofree = NULL;
        int offset, append;
#if _lib_pathnative
        char buff[PATH_MAX];
#endif  // _lib_pathnative
        if (flags & NV_INTEGER) {
            if ((flags & NV_DOUBLE) == NV_DOUBLE) {
                if (flags & NV_LONG) {
                    sfprintf(shp->strbuf, "%.*Lg", LDBL_DIG, *((Sfdouble_t *)sp));
                } else {
                    sfprintf(shp->strbuf, "%.*g", DBL_DIG, *((double *)sp));
                }
            } else if (flags & NV_UNSIGN) {
                if (flags & NV_LONG) {
                    sfprintf(shp->strbuf, "%I*lu", sizeof(Sfulong_t), *((Sfulong_t *)sp));
                } else {
                    sfprintf(shp->strbuf, "%lu",
                             (unsigned long)((flags & NV_SHORT) ? *((uint16_t *)sp)
                                                                : *((uint32_t *)sp)));
                }
            } else {
                if (flags & NV_LONG) {
                    sfprintf(shp->strbuf, "%I*ld", sizeof(Sflong_t), *((Sflong_t *)sp));
                } else {
                    sfprintf(shp->strbuf, "%ld",
                             (long)((flags & NV_SHORT) ? *((int16_t *)sp) : *((int32_t *)sp)));
                }
            }
            sp = sfstruse(shp->strbuf);
        }
        if (nv_isattr(np, NV_HOST | NV_INTEGER) == NV_HOST && sp) {
#if _lib_pathnative
            // Return the host file name given the UNIX name.
            pathnative(sp, buff, sizeof(buff));
            if (buff[1] == ':' && buff[2] == '/') {
                buff[2] = '\\';
                if (*buff >= 'A' && *buff <= 'Z') *buff += 'a' - 'A';
            }
            sp = buff;
#else   // _lib_pathnative
            ;
#endif  // _lib_pathnative
        } else if ((nv_isattr(np, NV_RJUST | NV_ZFILL | NV_LJUST)) && sp) {
            for (; *sp == ' ' || *sp == '\t'; sp++) {
                ;  // empty loop
            }
            if ((nv_isattr(np, NV_ZFILL)) && (nv_isattr(np, NV_LJUST)))
                for (; *sp == '0'; sp++) {
                    ;  // empty loop
                }
            size = nv_size(np);
            if (size) size = ja_size((char *)sp, size, nv_isattr(np, NV_RJUST | NV_ZFILL));
        }
        if (!up->cp || *up->cp == 0) flags &= ~NV_APPEND;
        if (!nv_isattr(np, NV_NOFREE)) {
            // Delay free in case <sp> points into free region.
            tofree = up->sp;
        }
        if (nv_isattr(np, NV_BINARY) && !(flags & NV_RAW)) tofree = NULL;
        if (nv_isattr(np, NV_LJUST | NV_RJUST) &&
            nv_isattr(np, NV_LJUST | NV_RJUST) != (NV_LJUST | NV_RJUST)) {
            tofree = NULL;
        }
        if (sp) {
            append = 0;
            if (sp == up->cp && !(flags & NV_APPEND)) return;
            dot = strlen(sp);
#if (_AST_VERSION >= 20030127L)
            if (nv_isattr(np, NV_BINARY)) {
                int oldsize = (flags & NV_APPEND) ? nv_size(np) : 0;
                if (flags & NV_RAW) {
                    if (tofree) {
                        free(tofree);
                        nv_offattr(np, NV_NOFREE);
                    }
                    up->cp = sp;
                    return;
                }
                size = 0;
                if (nv_isattr(np, NV_ZFILL)) size = nv_size(np);
                if (size == 0) size = oldsize + (3 * dot / 4);
                cp = malloc(size + 1);
                *cp = 0;
                nv_offattr(np, NV_NOFREE);
                if (oldsize) memcpy((void *)cp, (void *)up->cp, oldsize);
                up->sp = cp;
                if (size <= oldsize) return;
                dot = base64decode(sp, dot, NULL, cp + oldsize, size - oldsize, NULL);
                dot += oldsize;
                if (!nv_isattr(np, NV_ZFILL) || nv_size(np) == 0) {
                    nv_setsize(np, dot);
                } else if (nv_isattr(np, NV_ZFILL) && (size > dot)) {
                    memset((void *)&cp[dot], 0, size - dot);
                }
                return;
            } else
#endif  // (_AST_VERSION >= 20030127L)
            {
                if (size == 0 && nv_isattr(np, NV_HOST) != NV_HOST &&
                    nv_isattr(np, NV_LJUST | NV_RJUST | NV_ZFILL)) {
                    nv_setsize(np, size = dot);
                    tofree = up->sp;
                } else if (size > dot) {
                    dot = size;
                } else if (nv_isattr(np, NV_LJUST | NV_RJUST) == NV_LJUST && dot > size) {
                    dot = size;
                }
                if (flags & NV_APPEND) {
                    if (dot == 0) return;
                    append = strlen(up->cp);
                    if (!tofree || size) {
                        offset = stktell(shp->stk);
                        sfputr(shp->stk, up->cp, -1);
                        sfputr(shp->stk, sp, 0);
                        sp = stkptr(shp->stk, offset);
                        dot += append;
                        append = 0;
                    } else {
                        flags &= ~NV_APPEND;
                    }
                }
            }
            if (size == 0 || tofree || dot || !(cp = (char *)up->cp)) {
                if (dot == 0 && !nv_isattr(np, NV_LJUST | NV_RJUST)) {
                    cp = (char *)EmptyStr;  // we'd better not try to modify this buf as it's const
                    nv_onattr(np, NV_NOFREE);
                } else {
                    if (tofree && tofree != Empty && tofree != EmptyStr) {
                        cp = (char *)realloc((void *)tofree, ((unsigned)dot + append + 8));
                        tofree = 0;
                    } else {
                        cp = (char *)malloc(((unsigned)dot + 8));
                    }
                    cp[dot + append] = 0;
                    nv_offattr(np, NV_NOFREE);
                }
            }

        } else {
            cp = NULL;
        }
        up->cp = cp;
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
    } else {
        *(sp = str + size) = 0;
    }
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

static_fn int ja_size(char *str, int size, int type) {
    char *cp = str;
    int c, n = size;
    int outsize;
    char *oldcp = cp;
    int oldn;
    wchar_t w;
    while (*cp) {
        oldn = n;
        w = mbchar(cp);
        if ((outsize = mbwidth(w)) < 0) outsize = 0;
        size -= outsize;
        c = cp - oldcp;
        n += (c - outsize);
        oldcp = cp;
        if (size <= 0 && type == 0) break;
    }
    // Check for right justified fields that need truncating.
    if (size < 0) {
        if (type == 0) {
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
        if (type) n -= (ja_size(str, size, 0) - size);
    }
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
    p = strcopy(q, nv_name(np));
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
    int flag = np->nvflag;
    struct adata *ap = (struct adata *)data;
    ap->sh = (Shell_t *)data;
    ap->tp = 0;
    if (!(flag & NV_EXPORT) || (flag & NV_FUNCT)) return;
    if ((flag & (NV_UTOL | NV_LTOU | NV_INTEGER)) == (NV_UTOL | NV_LTOU)) {
        data = (void *)nv_mapchar(np, 0);
        if (strcmp(data, e_tolower) && strcmp(data, e_toupper)) return;
    }
    flag &= (NV_RDONLY | NV_UTOL | NV_LTOU | NV_RJUST | NV_LJUST | NV_ZFILL | NV_INTEGER);
    *ap->attval++ = '=';
    if ((flag & NV_DOUBLE) == NV_DOUBLE) {
        // Export doubles as integers for ksh88 compatibility.
        *ap->attval++ = ' ' + (NV_INTEGER | (flag & ~(NV_DOUBLE | NV_EXPNOTE)));
        *ap->attval = ' ';
    } else {
        *ap->attval++ = ' ' + flag;
        if (flag & NV_INTEGER) {
            *ap->attval = ' ' + nv_size(np);
        } else {
            *ap->attval = ' ';
        }
    }
    ap->attval = strcopy(++ap->attval, nv_name(np));
}

static_fn void pushnam(Namval_t *np, void *data) {
    char *value;
    struct adata *ap = (struct adata *)data;
    ap->sh = sh_ptr(np);
    ap->tp = 0;
    if (nv_isattr(np, NV_IMPORT) && np->nvenv) {
        *ap->argnam++ = np->nvenv;
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
    data.tp = 0;
    data.mapname = 0;
    // L_ARGNOD gets generated automatically as full path name of command.
    nv_offattr(L_ARGNOD, NV_EXPORT);
    data.attsize = 6;
    namec = nv_scan(shp->var_tree, nullscan, (void *)0, NV_EXPORT, NV_EXPORT);
    namec += shp->nenv;
    er = (char **)stkalloc(shp->stk, (namec + 4) * sizeof(char *));
    data.argnam = (er += 2) + shp->nenv;
    if (shp->nenv) memcpy((void *)er, environ, shp->nenv * sizeof(char *));
    nv_scan(shp->var_tree, pushnam, &data, NV_EXPORT, NV_EXPORT);
    *data.argnam = (char *)stkalloc(shp->stk, data.attsize);
    cp = data.attval = strcopy(*data.argnam, e_envmarker);
    nv_scan(shp->var_tree, attstore, &data, 0,
            (NV_RDONLY | NV_UTOL | NV_LTOU | NV_RJUST | NV_LJUST | NV_ZFILL | NV_INTEGER));
    *data.attval = 0;
    if (cp != data.attval) data.argnam++;
    *data.argnam = 0;
    return er;
}

struct scan {
    void (*scanfn)(Namval_t *, void *);
    int scanmask;
    int scanflags;
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

    if (!is_abuiltin(np) && tp && tp->tp && nv_type(np) != tp->tp) return (0);
    if (sp->scanmask == NV_TABLE && nv_isvtree(np)) k = NV_TABLE;
    if (sp->scanmask ? (k & sp->scanmask) == sp->scanflags
                     : (!sp->scanflags || (k & sp->scanflags))) {
        if (tp && tp->mapname) {
            if (sp->scanflags == NV_FUNCTION || sp->scanflags == (NV_NOFREE | NV_BINARY | NV_RAW)) {
                int n = strlen(tp->mapname);
                if (strncmp(np->nvname, tp->mapname, n) || np->nvname[n] != '.' ||
                    strchr(&np->nvname[n + 1], '.')) {
                    return 0;
                }
            } else if ((sp->scanflags == NV_UTOL || sp->scanflags == NV_LTOU) &&
                       (cp = (char *)nv_mapchar(np, 0)) && strcmp(cp, tp->mapname)) {
                return 0;
            }
        }
        if (!np->nvalue.cp && !np->nvfun && !nv_isattr(np, ~NV_DEFAULT)) return (0);
        if (sp->scanfn) {
            if (nv_isarray(np)) nv_putsub(np, NULL, 0L, 0);
            (*sp->scanfn)(np, sp->scandata);
        }
        sp->scancount++;
    }
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
int nv_scan(Dt_t *root, void (*fn)(Namval_t *, void *), void *data, int mask, int flags) {
    Namval_t *np;
    Dt_t *base = 0;
    struct scan sdata;
    int (*hashfn)(Dt_t *, void *, void *);

    sdata.scanmask = mask;
    sdata.scanflags = flags & ~NV_NOSCOPE;
    sdata.scanfn = fn;
    sdata.scancount = 0;
    sdata.scandata = data;
    hashfn = scanfilter;
    if (flags & NV_NOSCOPE) base = dtview((Dt_t *)root, 0);
    for (np = (Namval_t *)dtfirst(root); np; np = (Namval_t *)dtnext(root, np)) {
        hashfn(root, np, &sdata);
    }
    if (base) dtview((Dt_t *)root, base);
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
        dtview(newscope, (Dt_t *)shp->var_tree);
        shp->var_tree = newscope;
        sh_setlist(shp, envlist, NV_EXPORT | NV_NOSCOPE | NV_IDENT | NV_ASSIGN, 0);
        if (!fun) return;
        shp->var_tree = dtview(newscope, 0);
    }
    if ((rp = shp->st.real_fun) && rp->sdict) {
        dtview(rp->sdict, newroot);
        newroot = rp->sdict;
    }
    dtview(newscope, (Dt_t *)newroot);
    shp->var_tree = newscope;
}

//
// Remove freeable local space associated with the nvalue field of nnod. This includes any strings
// representing the value(s) of the node, as well as its dope vector, if it is an array.
//
void sh_envnolocal(Namval_t *np, void *data) {
    struct adata *tp = (struct adata *)data;
    char *cp = 0;
    if (np == VERSIONNOD && nv_isref(np)) return;
    if (np == L_ARGNOD) return;
    if (np == tp->sh->namespace) return;
    if (nv_isref(np)) nv_unref(np);
    if (nv_isattr(np, NV_EXPORT) && nv_isarray(np)) {
        nv_putsub(np, NULL, 0, 0);
        cp = nv_getval(np);
        if (cp) cp = strdup(cp);
    }
    if (nv_isattr(np, NV_EXPORT | NV_NOFREE)) {
        if (nv_isref(np) && np != VERSIONNOD) {
            nv_offattr(np, NV_NOFREE | NV_REF);
            free(np->nvalue.nrp);
            np->nvalue.cp = 0;
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
                np->nvfun = 0;
            }
            if (nv_isattr(nq, NV_EXPORT)) sh_envput(shp, nq);
        }
        shp->last_root = root;
        shp->last_table = 0;
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
void _nv_unset(Namval_t *np, int flags) {
    Shell_t *shp = sh_ptr(np);
    union Value *up;

    if (!(flags & NV_RDONLY) && nv_isattr(np, NV_RDONLY)) {
        errormsg(SH_DICT, ERROR_exit(1), e_readonly, nv_name(np));
        __builtin_unreachable();
    }
    if (is_afunction(np) && np->nvalue.ip) {
        struct slnod *slp = (struct slnod *)(np->nvenv);
        if (np->nvalue.rp->running) {
            np->nvalue.rp->running |= 1;
            return;
        }
        if (slp && !nv_isattr(np, NV_NOFREE)) {
            struct Ufunction *rq, *rp = np->nvalue.rp;
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
                do {
                    if (rq->np != np) continue;
                    dtdelete(shp->fpathdict, rq);
                    break;
                } while ((rq = (struct Ufunction *)dtnext(shp->fpathdict, rq)));
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
            stakdelete(slp->slptr);
            free(np->nvalue.ip);
            np->nvalue.ip = 0;
        }
        goto done;
    }
    if (shp->subshell) np = sh_assignok(np, 0);
    nv_offattr(np, NV_NODISC);
    if (np->nvfun && !nv_isref(np)) {
        // This function contains disc.
        if (!nv_local) {
            Dt_t *last_root = shp->last_root;
            nv_local = 1;
            nv_putv(np, NULL, flags, np->nvfun);
            nv_local = 0;
            shp->last_root = last_root;
            return;
        }
        // Called from disc, assign the actual value.
        nv_local = 0;
    }
    if (nv_isattr(np, NV_INT16P | NV_DOUBLE) == NV_INT16) {
        np->nvalue.cp = nv_isarray(np) ? Empty : 0;
        goto done;
    } else if (np->nvalue.up && nv_isarray(np) && nv_arrayptr(np)) {
        up = np->nvalue.up;
    } else if (nv_isref(np) && !nv_isattr(np, NV_EXPORT | NV_MINIMAL) && np->nvalue.nrp) {
        if (np->nvalue.nrp->root) dtremove(Refdict, (void *)np->nvalue.nrp);
        if (np->nvalue.nrp->sub) free(np->nvalue.nrp->sub);
        free(np->nvalue.nrp);
        np->nvalue.cp = 0;
        up = 0;
    } else {
        up = &np->nvalue;
    }
    if (up && up->sp) {
        if (up->sp != Empty && up->sp != EmptyStr && !nv_isattr(np, NV_NOFREE)) free(up->sp);
        up->sp = NULL;
    }

done:
    if (!nv_isarray(np) || !nv_arrayptr(np)) {
        nv_setsize(np, 0);
        if (!nv_isattr(np, NV_MINIMAL) || nv_isattr(np, NV_EXPORT)) {
            if (nv_isattr(np, NV_EXPORT) && !strchr(np->nvname, '[')) {
                env_delete(shp->env, nv_name(np));
            }
            if (!(flags & NV_EXPORT) || nv_isattr(np, NV_EXPORT)) np->nvenv = 0;
            nv_setattr(np, 0);
        } else {
            nv_setattr(np, NV_MINIMAL);
            nv_delete(np, (Dt_t *)0, 0);
        }
    }
}

//
// Return the node pointer in the highest level scope.
//
Namval_t *sh_scoped(Shell_t *shp, Namval_t *np) {
    if (!dtvnext(shp->var_tree)) return (np);
    return dtsearch(shp->var_tree, np);
}

struct optimize {
    Namfun_t hdr;
    Shell_t *sh;
    char **ptr;
    struct optimize *next;
    Namval_t *np;
};

static struct optimize *opt_free;

static_fn void optimize_clear(Namval_t *np, Namfun_t *fp) {
    struct optimize *op = (struct optimize *)fp;
    nv_stack(np, fp);
    nv_stack(np, (Namfun_t *)0);
    for (; op && op->np == np; op = op->next) {
        if (op->ptr) {
            *op->ptr = 0;
            op->ptr = 0;
        }
    }
}

static_fn void put_optimize(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    nv_putv(np, val, flags, fp);
    optimize_clear(np, fp);
}

static_fn Namfun_t *clone_optimize(Namval_t *np, Namval_t *mp, int flags, Namfun_t *fp) {
    return NULL;
}

const Namdisc_t OPTIMIZE_disc = {sizeof(struct optimize), put_optimize, NULL, NULL, NULL, NULL,
                                 clone_optimize};

void nv_optimize(Namval_t *np) {
    Shell_t *shp = sh_ptr(np);
    Namfun_t *fp;
    struct optimize *op, *xp;

    if (shp->argaddr) {
        if (np == SH_LINENO) {
            shp->argaddr = 0;
            return;
        }
        for (fp = np->nvfun; fp; fp = fp->next) {
            if (fp->disc && (fp->disc->getnum || fp->disc->getval)) {
                shp->argaddr = 0;
                return;
            }
            if (fp->disc == &OPTIMIZE_disc) break;
        }
        if ((xp = (struct optimize *)fp) && xp->ptr == shp->argaddr) return;
        op = opt_free;
        if (op) {
            opt_free = op->next;
        } else {
            op = (struct optimize *)calloc(1, sizeof(struct optimize));
        }
        op->ptr = shp->argaddr;
        op->np = np;
        if (xp) {
            op->hdr.disc = 0;
            op->next = xp->next;
            xp->next = op;
        } else {
            op->hdr.disc = &OPTIMIZE_disc;
            op->next = (struct optimize *)shp->optlist;
            shp->optlist = (void *)op;
            nv_stack(np, &op->hdr);
        }
    }
}

void sh_optclear(Shell_t *shp, void *old) {
    struct optimize *op, *opnext;
    for (op = (struct optimize *)shp->optlist; op; op = opnext) {
        opnext = op->next;
        if (op->ptr && op->hdr.disc) {
            nv_stack(op->np, &op->hdr);
            nv_stack(op->np, (Namfun_t *)0);
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
char *nv_getval(Namval_t *np) {
    Shell_t *shp = sh_ptr(np);
    union Value *up = &np->nvalue;
    int numeric;

    if (!nv_local && shp->argaddr) nv_optimize(np);
    if ((!np->nvfun || !np->nvfun->disc) &&
        !nv_isattr(np, NV_ARRAY | NV_INTEGER | NV_FUNCT | NV_REF)) {
        goto done;
    }
    if (nv_isref(np)) {
        char *sub;
        if (!np->nvalue.cp) return (0);
        shp->last_table = nv_reftable(np);
        sub = nv_refsub(np);
        np = nv_refnode(np);
        if (sub) {
            sfprintf(shp->strbuf, "%s[%s]", nv_name(np), sub);
            return (sfstruse(shp->strbuf));
        }
        return nv_name(np);
    }
    if (np->nvfun && np->nvfun->disc) {
        if (!nv_local) {
            nv_local = 1;
            return nv_getv(np, np->nvfun);
        }
        nv_local = 0;
    }
    numeric = ((nv_isattr(np, NV_INTEGER)) != 0);
    if (numeric) {
        Sflong_t ll;
        if (nv_isattr(np, NV_NOTSET) == NV_NOTSET) return (NULL);
        if (!up->cp) return ("0");
        if (nv_isattr(np, NV_DOUBLE) == NV_DOUBLE) {
            Sfdouble_t ld;
            double d;
            char *format;
            if (nv_isattr(np, NV_LONG)) {
                ld = *up->ldp;
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
                    d = *up->fp;
                } else {
                    d = *up->dp;
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
            return (sfstruse(shp->strbuf));
        } else if (nv_isattr(np, NV_UNSIGN)) {
            if (nv_isattr(np, NV_LONG)) {
                ll = *(Sfulong_t *)up->llp;
            } else if (nv_isattr(np, NV_SHORT)) {
                if (nv_isattr(np, NV_INT16P) == NV_INT16P) {
                    ll = *(uint16_t *)(up->i16p);
                } else {
                    ll = (uint16_t)up->i16;
                }
            } else {
                ll = *(uint32_t *)(up->lp);
            }
        } else if (nv_isattr(np, NV_LONG)) {
            ll = *up->llp;
        } else if (nv_isattr(np, NV_SHORT)) {
            if (nv_isattr(np, NV_INT16P) == NV_INT16P) {
                ll = *up->i16p;
            } else {
                ll = up->i16;
            }
        } else {
            ll = *(up->lp);
        }
        if ((numeric = nv_size(np)) == 10) {
            if (nv_isattr(np, NV_UNSIGN)) {
                sfprintf(shp->strbuf, "%I*u", sizeof(ll), ll);
                return sfstruse(shp->strbuf);
            }
            numeric = 0;
        }
        return fmtbase(ll, numeric, numeric && numeric != 10);
    }
done:
#if (_AST_VERSION >= 20030127L)
    // If NV_RAW flag is on, return pointer to binary data otherwise, base64 encode the data and
    // return this string.
    if (up->cp && nv_isattr(np, NV_BINARY) && !nv_isattr(np, NV_RAW)) {
        int size = nv_size(np), insize = (4 * size) / 3 + size / 45 + 8;
        char *cp = getbuf(insize);
        char *ep;
        base64encode(up->cp, size, NULL, cp, insize, (void **)&ep);
        return cp;
    }
#endif  // (_AST_VERSION >= 20030127L)
    if (!nv_isattr(np, NV_LJUST | NV_RJUST) && (numeric = nv_size(np)) && up->cp &&
        up->cp[numeric]) {
        char *cp = getbuf(numeric + 1);
        memcpy(cp, up->cp, numeric);
        cp[numeric] = 0;
        return cp;
    }
    return up->sp;
}

Sfdouble_t nv_getnum(Namval_t *np) {
    Shell_t *shp = sh_ptr(np);
    union Value *up;
    Sfdouble_t r = 0;
    char *str;

    if (!nv_local && shp->argaddr) nv_optimize(np);
    if (nv_istable(np)) {
        errormsg(SH_DICT, ERROR_exit(1), e_number, nv_name(np));
        __builtin_unreachable();
    }
    if (np->nvfun && np->nvfun->disc) {
        if (!nv_local) {
            nv_local = 1;
            return nv_getn(np, np->nvfun);
        }
        nv_local = 0;
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
        if (!up->lp || up->cp == Empty) {
            r = 0;
        } else if (nv_isattr(np, NV_DOUBLE) == NV_DOUBLE) {
            if (nv_isattr(np, NV_LONG)) {
                r = *up->ldp;
            } else if (nv_isattr(np, NV_SHORT)) {
                r = *up->fp;
            } else {
                r = *up->dp;
            }
        } else if (nv_isattr(np, NV_UNSIGN)) {
            if (nv_isattr(np, NV_LONG)) {
                r = (Sfulong_t) * ((Sfulong_t *)up->llp);
            } else if (nv_isattr(np, NV_SHORT)) {
                if (nv_isattr(np, NV_INT16P) == NV_INT16P) {
                    r = (Sfulong_t)(*up->i16p);
                } else {
                    r = (Sfulong_t)(up->i16);
                }
            } else {
                r = *((uint32_t *)up->lp);
            }
        } else {
            if (nv_isattr(np, NV_LONG)) {
                r = *up->llp;
            } else if (nv_isattr(np, NV_SHORT)) {
                if (nv_isattr(np, NV_INT16P) == NV_INT16P) {
                    r = *up->i16p;
                } else {
                    r = up->i16;
                }
            } else {
                r = *up->lp;
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
void nv_newattr(Namval_t *np, unsigned newatts, int size) {
    Shell_t *shp = sh_ptr(np);
    char *sp;
    char *cp = 0;
    unsigned int n;
    Namval_t *mp = 0;
    Namarr_t *ap = 0;
    int oldsize, oldatts, trans;
    Namfun_t *fp = (newatts & NV_NODISC) ? np->nvfun : 0;
    char *prefix = shp->prefix;

    newatts &= ~NV_NODISC;
    // Check for restrictions.
    if (sh_isoption(shp, SH_RESTRICTED) &&
        ((sp = nv_name(np)) == nv_name(PATHNOD) || sp == nv_name(SHELLNOD) ||
         sp == nv_name(ENVNOD) || sp == nv_name(FPATHNOD))) {
        errormsg(SH_DICT, ERROR_exit(1), e_restricted, nv_name(np));
        __builtin_unreachable();
    }
    // Handle attributes that do not change data separately.
    n = np->nvflag;
    trans = !(n & NV_INTEGER) && (n & (NV_LTOU | NV_UTOL));
    if (newatts & NV_EXPORT) nv_offattr(np, NV_IMPORT);
    if (((n ^ newatts) & NV_EXPORT)) {
        // Record changes to the environment.
        if (n & NV_EXPORT) {
            nv_offattr(np, NV_EXPORT);
            env_delete(shp->env, nv_name(np));
        } else {
            nv_onattr(np, NV_EXPORT);
            sh_envput(shp, np);
        }
        if ((n ^ newatts) == NV_EXPORT && size == -1) return;
    }
    oldsize = nv_size(np);
    if (size == -1) size = oldsize;
    if ((size == oldsize || (n & NV_INTEGER)) && !trans && ((n ^ newatts) & ~NV_NOCHANGE) == 0) {
        if (size) nv_setsize(np, size);
        nv_offattr(np, ~NV_NOFREE);
        nv_onattr(np, newatts);
        return;
    }
    // For an array, change all the elements.
    if ((ap = nv_arrayptr(np)) && ap->nelem > 0) nv_putsub(np, NULL, 0, ARRAY_SCAN);
    oldsize = nv_size(np);
    oldatts = np->nvflag;
    if (fp) np->nvfun = 0;
    if (ap) {  // add element to prevent array deletion
        ap->nelem++;
    }
    do {
        nv_setsize(np, oldsize);
        np->nvflag = oldatts;
        sp = nv_getval(np);
        if (sp) {
            if (nv_isattr(np, NV_ZFILL)) {
                while (*sp == '0') sp++;
            }
            cp = (char *)malloc((n = strlen(sp)) + 8);
            strcpy(cp, sp);
            if (sp && (mp = nv_opensub(np))) {
                if (trans) {
                    nv_disc(np, &ap->hdr, NV_POP);
                    nv_clone(np, mp, 0);
                    nv_disc(np, &ap->hdr, NV_FIRST);
                    nv_offattr(mp, NV_ARRAY);
                }
                nv_newattr(mp, newatts & ~NV_ARRAY, size);
            }
            if (!mp) {
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

    if (!string || (c0 = *string) == 0) return (0);
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

    if (!shp->var_tree) {
#if 0
		if(name[0] == 'P' && name[1] == 'A' && name[2] == 'T' && name[3] == 'H' && name[4] == 0 || name[0] == 'L' && ((name[1] == 'C' || name[1] == 'D') && name[2] == '_' || name[1] == 'A' && name[1] == 'N') || name[0] == 'V' && name[1] == 'P' && name[2] == 'A' && name[3] == 'T' && name[4] == 'H' && name[5] == 0 || name[0] == '_' && name[1] == 'R' && name[2] == 'L' && name[3] == 'D' || name[0] == '_' && name[1] == 'A' && name[2] == 'S' && name[3] == 'T' && name[4] == '_')
#endif
        return oldgetenv(name);
    } else if ((np = nv_search(name, shp->var_tree, 0)) && nv_isattr(np, NV_EXPORT)) {
        return nv_getval(np);
    }
    return NULL;
}

#ifndef _NEXT_SOURCE
//
// Some dynamic linkers will make this file see the libc getenv(), so sh_getenv() is used for the
// astintercept() callback.  Plain getenv() is provided for static links.
//
char *getenv(const char *name) { return sh_getenv(name); }
#endif  // _NEXT_SOURCE

#undef putenv
//
// This version of putenv uses the hash storage to assign environment values.
//
int putenv(const char *name) {
    Shell_t *shp = sh_getinterp();
    Namval_t *np;
    if (name) {
        np = nv_open(name, shp->var_tree, NV_EXPORT | NV_IDENT | NV_NOARRAY | NV_ASSIGN);
        if (!strchr(name, '=')) _nv_unset(np, 0);
    }
    return 0;
}

//
// Override libast setenviron().
//
char *sh_setenviron(const char *name) {
    Shell_t *shp = sh_getinterp();
    Namval_t *np;

    if (name) {
        np = nv_open(name, shp->var_tree, NV_EXPORT | NV_IDENT | NV_NOARRAY | NV_ASSIGN);
        if (strchr(name, '=')) return (nv_getval(np));
        _nv_unset(np, 0);
    }
    return "";
}

//
// Same linker dance as with getenv() above.
//
char *setenviron(const char *name) { return sh_setenviron(name); }

//
// Normalize <cp> and return pointer to subscript if any if <eq> is specified, return pointer to
// first = not in a subscript.
//
static_fn char *lastdot(char *cp, int eq, void *context) {
    char *ep = 0;
    int c;

    if (eq) cp++;
    while ((c = mbchar(cp))) {
        if (c == '[') {
            if (*cp == ']') {
                cp++;
            } else {
                cp = nv_endsubscript((Namval_t *)0, ep = cp, 0, context);
            }
        } else if (c == '.') {
            if (*cp == '[') {
                cp = nv_endsubscript((Namval_t *)0, ep = cp, 0, context);
                if ((ep = sh_checkid(ep + 1, cp)) < cp) cp = strcpy(ep, cp);
            }
            ep = 0;
        } else if (eq && c == '=') {
            return cp - 1;
        }
    }
    return eq ? 0 : ep;
}

#if NVCACHE
//
// Purge all entries whose name is of the form name.
//
static_fn void cache_purge(const char *name) {
    int c;
    size_t len = strlen(name);
    struct Cache_entry *xp;

    for (c = 0, xp = nvcache.entries; c < NVCACHE; xp = &nvcache.entries[++c]) {
        if (xp->len <= len || xp->name[len] != '.') continue;
        if (strncmp(name, xp->name, len) == 0) xp->root = 0;
    }
}
#endif  // NVCACHE

bool nv_rename(Namval_t *np, int flags) {
    Shell_t *shp = sh_ptr(np);
    Namval_t *mp = 0, *nr = 0;
    char *cp;
    int arraynr, index = -1;
    Namval_t *last_table = shp->last_table;
    Dt_t *last_root = shp->last_root;
    Dt_t *hp = 0;
    char *nvenv = 0, *prefix = shp->prefix;
    Namarr_t *ap;

    if (nv_isattr(np, NV_PARAM) && shp->st.prevst) {
        if (!(hp = (Dt_t *)shp->st.prevst->save_tree)) hp = dtvnext(shp->var_tree);
    }
    if (!nv_isattr(np, NV_MINIMAL)) nvenv = np->nvenv;
    mp = nv_isarray(np) ? nv_opensub(np) : 0;
    if (flags & NV_MOVE) {
        if (!(cp = (char *)(mp ? mp : np)->nvalue.cp)) {
            errormsg(SH_DICT, ERROR_exit(1), e_varname, "");
            __builtin_unreachable();
        }
        (mp ? mp : np)->nvalue.cp = 0;
    } else if (!(cp = nv_getval(np))) {
        return false;
    }
    if (lastdot(cp, 0, (void *)shp) && nv_isattr(np, NV_MINIMAL)) {
        errormsg(SH_DICT, ERROR_exit(1), e_varname, nv_name(np));
        __builtin_unreachable();
    }
    arraynr = cp[strlen(cp) - 1] == ']';
    if (nv_isarray(np) && !mp) index = nv_aindex(np);
    shp->prefix = 0;
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
            mp->nvenv = nvenv = (void *)np;
        }
    }
    if (mp) {
        nvenv = (char *)np;
        np = mp;
    }
    if (nr == np) {
        if (index < 0) return (false);
        cp = nv_getval(np);
        if (cp) cp = strdup(cp);
    }
    _nv_unset(np, NV_EXPORT);
    if (nr == np) {
        nv_putsub(np, (char *)0, index, 0);
        nv_putval(np, cp, 0);
        free(cp);
        return true;
    }
    if ((shp->prev_table = shp->last_table) && nv_isvtree(nr)) shp->prev_table = 0;
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
            nvenv = (void *)np;
        } else {
            mp = np;
        }
        if (nr == SH_MATCHNOD) {
            Sfio_t *iop;
            Dt_t *save_root = shp->var_tree;
            bool trace = sh_isoption(shp, SH_XTRACE);
            sfprintf(shp->strbuf, "typeset -a %s=", nv_name(mp));
            nv_outnode(nr, shp->strbuf, -1, 0);
            sfwrite(shp->strbuf, ")\n", 2);
            cp = sfstruse(shp->strbuf);
            iop = sfopen((Sfio_t *)0, cp, "s");
            if (trace) sh_offoption(shp, SH_XTRACE);
            shp->var_tree = last_root;
            sh_eval(shp, iop, SH_READEVAL);
            shp->var_tree = save_root;
            if (trace) sh_onoption(shp, SH_XTRACE);
            if (flags & NV_MOVE) sh_setmatch(shp, 0, 0, 0, 0, 0);
        } else {
            nv_clone(nr, mp, (flags & NV_MOVE) | NV_COMVAR);
#if NVCACHE
            cache_purge(nv_name(nr));
#endif  // NVCACHE
        }
        mp->nvenv = nvenv;
        if (flags & NV_MOVE) {
            if (arraynr && !nv_isattr(nr, NV_MINIMAL) && (mp = (Namval_t *)nr->nvenv) &&
                (ap = nv_arrayptr(mp))) {
                nv_putsub(mp, nr->nvname, 0, 0);
                _nv_unset(mp, 0);
            }
            nv_delete(nr, (Dt_t *)0, NV_NOFREE);
        }
    } else {
        if (flags & NV_MOVE) {
            if (!nv_isattr(nr, NV_MINIMAL) && (mp = (Namval_t *)(nr->nvenv)) &&
                (ap = nv_arrayptr(mp))) {
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
void nv_setref(Namval_t *np, Dt_t *hp, int flags) {
    Shell_t *shp = sh_ptr(np);
    Namval_t *nq = 0, *nr = 0;
    char *ep, *cp;
    Dt_t *root = shp->last_root, *hpnext = 0;
    Namarr_t *ap = 0;
    Dt_t *openmatch;
    Namval_t *last_table = shp->last_table;

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
    if ((ep = lastdot(cp, 0, (void *)shp)) && nv_isattr(np, NV_MINIMAL)) {
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
                nr = 0;
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
        if (!(hp = dtvnext(hp)) || (nq = nv_search((char *)np, hp, NV_ADD | HASH_BUCKET)) == np) {
            errormsg(SH_DICT, ERROR_exit(1), e_selfref, nv_name(np));
            __builtin_unreachable();
        }
        if (nv_isarray(nq)) nv_putsub(nq, (char *)0, 0, ARRAY_UNDEF);
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
        shp->last_root = 0;
        nr = nq = nv_open(cp, hp, flags);
        if (shp->last_root) hp = shp->last_root;
    }
    shp->namref_root = 0;
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
        if (nv_isarray(nq)) {
            ep = nv_getsub(nq);
        } else {
            int n;
            ep[n = strlen(ep) - 1] = 0;
            nv_putsub(nr, ep, 0, ARRAY_FILL);
            ep[n] = ']';
            nq = nv_opensub(nr);
            if (nq) {
                ep = 0;
            } else {
                ep = nv_getsub(nq = nr);
            }
        }
    }
    shp->instance = 0;
    shp->last_root = root;
    _nv_unset(np, 0);
    nv_delete(np, (Dt_t *)0, 0);
    np->nvalue.nrp = newof(0, struct Namref, 1, sizeof(Dtlink_t));
    np->nvalue.nrp->np = nq;
    np->nvalue.nrp->root = hp;
    np->nvalue.nrp->oldnp = shp->oldnp;
    if (ep) {
        np->nvalue.nrp->sub = strdup(ep);
    }
    np->nvalue.nrp->table = last_table;
    nv_onattr(np, NV_REF | NV_NOFREE);
    if (!Refdict) {
        NullNode.nvname = ".deleted";
        NullNode.nvflag = NV_RDONLY;
        NullNode.nvshell = nq->nvshell;
        Refdict = dtopen(&_Refdisc, Dtobag);
    }
    dtinsert(Refdict, np->nvalue.nrp);
}

//
// Get the scope corresponding to <index> whence uses the same values as lseeek().
//
Shscope_t *sh_getscope_20120720(Shell_t *shp, int index, int whence) {
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
    if (index < 0) return ((Shscope_t *)0);
    while (index-- && (sp = sp->prevst)) {
        ;  // empty loop
    }
    return (Shscope_t *)sp;  // here be dragons
}

#undef sh_getscope
Shscope_t *sh_getscope(int index, int whence) {
    return sh_getscope_20120720(sh_getinterp(), index, whence);
}

//
// Make <scoped> the top scope and return previous scope.
//
Shscope_t *sh_setscope(Shell_t *shp, Shscope_t *scope) {
    Shscope_t *old = (Shscope_t *)shp->st.self;
    *shp->st.self = shp->st;
    shp->st = *((struct sh_scoped *)scope);
    shp->var_tree = scope->var_tree;
    SH_PATHNAMENOD->nvalue.cp = shp->st.filename;
    SH_FUNNAMENOD->nvalue.cp = shp->st.funname;
    return old;
}

#undef sh_setscope
Shscope_t *sh_setscope(Shscope_t *scope) { return (sh_setscope_20120720(sh_getinterp(), scope)); }

void sh_unscope(Shell_t *shp) {
    Dt_t *root = shp->var_tree;
    Dt_t *dp = dtview(root, (Dt_t *)0);
    if (dp) {
        table_unset(shp, root, NV_RDONLY | NV_NOSCOPE, dp);
        if (shp->st.real_fun && dp == shp->st.real_fun->sdict) {
            dp = dtview(dp, (Dt_t *)0);
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
    if (!np->nvalue.nrp) return;
    nq = nv_refnode(np);
    if (Refdict) {
        if (np->nvalue.nrp->sub) free(np->nvalue.nrp->sub);
        dtremove(Refdict, (void *)np->nvalue.nrp);
    }
    free(np->nvalue.nrp);
    np->nvalue.cp = strdup(nv_name(nq));

    for (Namfun_t *fp = nq->nvfun; fp; fp = fp->next) {
        if (fp->disc == &OPTIMIZE_disc) {
            optimize_clear(nq, fp);
            return;
        }
    }
}

//
// These following are for binary compatibility with the old hash library. They will be removed
// someday.
//

#undef hashscope

Dt_t *hashscope(Dt_t *root) { return dtvnext(root); }

#undef hashfree

Dt_t *hashfree(Dt_t *root) {
    Dt_t *dp = dtvnext(root);
    dtclose(root);
    return dp;
}

#undef hashname

char *hashname(void *obj) {
    Namval_t *np = (Namval_t *)obj;
    return np->nvname;
}

#undef hashlook

void *hashlook(Dt_t *root, const char *name, int mode, int size) {
    UNUSED(size);

    return nv_search(name, root, mode);
}

char *nv_name(Namval_t *np) {
    Shell_t *shp = sh_ptr(np);
    Namval_t *table = 0;
    Namfun_t *fp = 0;
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
        Namval_t *nq = shp->last_table, *mp = (Namval_t *)np->nvenv;
        if (mp && (mp->nvname == NULL || *mp->nvname == 0)) mp = NULL;
        if (mp && (!strchr(np->nvname, '.') || (np->nvfun && nv_type(mp)) ||
                   (nv_isarray(mp) && (cp = nv_getsub(mp)) && strcmp(np->nvname, cp) == 0))) {
            if (np == shp->last_table) shp->last_table = 0;
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
        for (fp = np->nvfun; fp; fp = fp->next)
            if (fp->disc && fp->disc->namef) {
                if (np == shp->last_table) shp->last_table = 0;
                return (*fp->disc->namef)(np, fp);
            }
    }
    if (!(table = shp->last_table) || *np->nvname == '.' || table == shp->namespace ||
        np == table) {
        return np->nvname;
    }
    if (table) {
        cp = nv_name(table);
        sfprintf(shp->strbuf, "%s.%s", cp, np->nvname);
    } else {
        sfprintf(shp->strbuf, "%s", np->nvname);
    }
    return sfstruse(shp->strbuf);
}

Namval_t *nv_lastdict(void *context) {
    Shell_t *shp = (Shell_t *)context;
    return shp->last_table;
}

#undef nv_context
//
// Returns the data context for a builtin.
//
void *nv_context(Namval_t *np) { return np->nvfun; }

int nv_isnull(Namval_t *np) {
    if (np->nvalue.cp) return 0;
    if (np == IFSNOD) return 1;
    if (nv_isattr(np, NV_INT16) == NV_INT16 && !np->nvfun) {
        return nv_isattr(np, NV_NOTSET) == NV_NOTSET;
    }
    if (!nv_attr(np) || nv_isattr(np, NV_NOTSET) != NV_NOTSET) {
        return !np->nvfun || !np->nvfun->disc || !nv_hasget(np);
    }
    return 0;
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
