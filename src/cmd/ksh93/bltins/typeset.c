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

#include <float.h>
#include <limits.h>  // IWYU pragma: keep
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>

#include "ast.h"
#include "ast_assert.h"
#include "builtins.h"
#include "cdt.h"
#include "defs.h"
#include "error.h"
#include "history.h"
#include "name.h"
#include "option.h"
#include "path.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"

static_fn int print_namval(Sfio_t *, Namval_t *, bool, struct tdata *);
static_fn void print_attribute(Namval_t *, void *);
static_fn void print_all(Sfio_t *, Dt_t *, struct tdata *);
static_fn void pushname(Namval_t *, void *);

int b_typeset(int argc, char *argv[], Shbltin_t *context) {
    int n;
    nvflag_t nvflags = NV_VARNAME | NV_ASSIGN;
    struct tdata tdata;
    const char *optstring = sh_opttypeset;
    Namdecl_t *ntp = (Namdecl_t *)context->ptr;
    Dt_t *troot;
    bool isfloat = false, isshort = false, sflag = false;
    bool local = strcmp(argv[0], "local") == 0;
    UNUSED(argc);

    memset(&tdata, 0, sizeof(tdata));
    tdata.sh = context->shp;
    if (ntp) {
        tdata.tp = ntp->tp;
        opt_info.disc = ntp->optinfof;
        optstring = ntp->optstring;
    }
    troot = tdata.sh->var_tree;
    opt_info.index = 0;
    while ((n = optget(argv, optstring))) {
        if (tdata.aflag == 0) tdata.aflag = *opt_info.option;
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'a': {
                nvflags |= NV_IARRAY;
                if (opt_info.arg && *opt_info.arg != '[') {
                    opt_info.index--;
                    goto endargs;
                }
                tdata.tname = opt_info.arg;
                break;
            }
            case 'A': {
                nvflags |= NV_ARRAY;
                break;
            }
            case 'C': {
                nvflags |= NV_COMVAR;
                break;
            }
            case 'E': {
                // The following is for ksh88 compatibility.
                if (opt_info.offset && !strchr(argv[opt_info.index], 'E')) {
                    tdata.argnum = (int)opt_info.num;
                    break;
                }
            }
            // FALLTHRU
            case 'F':
            case 'X': {
                if (!opt_info.arg || (tdata.argnum = opt_info.num) < 0) {
                    if (n == 'X') {
                        tdata.argnum = 2 * (nv_isflag(nvflags, NV_LONG)
                                                ? sizeof(Sfdouble_t)
                                                : (isshort ? sizeof(float) : sizeof(double)));
                    } else {
                        tdata.argnum =
                            (nv_isflag(nvflags, NV_LONG) ? LDBL_DIG
                                                         : (isshort ? FLT_DIG : DBL_DIG)) -
                            2;
                    }
                }
                isfloat = true;
                if (n == 'E') {
                    nvflags &= ~NV_HEXFLOAT;
                    nvflags |= NV_EXPNOTE;
                } else if (n == 'X') {
                    nvflags &= ~NV_EXPNOTE;
                    nvflags |= NV_HEXFLOAT;
                }
                break;
            }
            case 'b': {
                nvflags |= NV_BINARY;
                break;
            }
            case 'm': {
                nvflags |= NV_MOVE;
                break;
            }
            case 'n': {
                nvflags &= ~NV_VARNAME;
                nvflags |= (NV_REF | NV_IDENT);
                break;
            }
            case 'H': {
                nvflags |= NV_HOST;
                break;
            }
            case 'T': {
                nvflags |= NV_TYPE;
                tdata.prefix = opt_info.arg;
                break;
            }
            case 'L':
            case 'Z':
            case 'R': {
                if (tdata.argnum == 0) tdata.argnum = (int)opt_info.num;
                if (tdata.argnum < 0) {
                    errormsg(SH_DICT, ERROR_exit(1), e_badfield, tdata.argnum);
                    __builtin_unreachable();
                }
                if (n == 'Z') {
                    nvflags |= NV_ZFILL;
                } else {
                    nvflags &= ~(NV_LJUST | NV_RJUST);
                    nvflags |= (n == 'L' ? NV_LJUST : NV_RJUST);
                }
                break;
            }
            case 'M': {
                tdata.wctname = opt_info.arg;
                if (tdata.wctname && !wctrans(tdata.wctname)) {
                    errormsg(SH_DICT, ERROR_exit(1), e_unknownmap, tdata.wctname);
                    __builtin_unreachable();
                }
                if (tdata.wctname && strcmp(tdata.wctname, e_tolower) == 0) {
                    nvflags |= NV_UTOL;
                } else {
                    nvflags |= NV_LTOU;
                }
                if (!tdata.wctname) nvflags |= NV_UTOL;
                break;
            }
            case 'f': {
                nvflags &= ~(NV_VARNAME | NV_ASSIGN);
                troot = tdata.sh->fun_tree;
                break;
            }
            case 'i': {
                if (!opt_info.arg || (tdata.argnum = opt_info.num) < 0) tdata.argnum = 10;
                nvflags |= NV_INTEGER;
                break;
            }
            case 'l': {
                tdata.wctname = e_tolower;
                nvflags |= NV_UTOL;  // same as: nvflags |= NV_LONG
                break;
            }
            case 'p': {
                tdata.prefix = argv[0];
                tdata.pflag = true;
                nvflags &= ~NV_ASSIGN;
                break;
            }
            case 'r': {
                nvflags |= NV_RDONLY;
                break;
            }
            case 'S': {
                sflag = true;
                break;
            }
            case 'h': {
                tdata.help = opt_info.arg;
                break;
            }
            case 's': {
                isshort = true;
                break;
            }
            case 't': {
                nvflags |= NV_TAGGED;
                break;
            }
            case 'u': {
                tdata.wctname = e_toupper;
                nvflags |= NV_LTOU;  // same as: nvflags |= NV_UNSIGN
                break;
            }
            case 'x': {
                nvflags &= ~NV_VARNAME;
                nvflags |= (NV_EXPORT | NV_IDENT);
                break;
            }
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(0), "%s", opt_info.arg);
                opt_info.disc = NULL;
                return 2;
            }
        }
    }
endargs:
    argv += opt_info.index;

    if (local && context->shp->var_base == context->shp->var_tree) {
        errormsg(SH_DICT, ERROR_exit(1), "local can only be used in a function");
        __builtin_unreachable();
    }

    opt_info.disc = NULL;
    // Handle argument of + and - specially.
    if (*argv && argv[0][1] == 0 && (*argv[0] == '+' || *argv[0] == '-')) {
        tdata.aflag = *argv[0];
    } else if (opt_info.index) {
        argv--;
    }
    if (nv_isflag(nvflags, NV_ZFILL) && !nv_isflag(nvflags, NV_LJUST)) nvflags |= NV_RJUST;
    if (nv_isflag(nvflags, NV_INTEGER) &&
        (nv_isflag(nvflags, NV_LJUST) || nv_isflag(nvflags, NV_RJUST) ||
         nv_isflag(nvflags, NV_ZFILL))) {
        error_info.errors++;
    }
    if (nv_isflag(nvflags, NV_BINARY) &&
        (nv_isflag(nvflags, NV_LJUST) || nv_isflag(nvflags, NV_UTOL) ||
         nv_isflag(nvflags, NV_LTOU))) {
        error_info.errors++;
    }
    if (nv_isflag(nvflags, NV_MOVE) && !nv_isflag(nvflags, NV_MOVE) &&
        !nv_isflag(nvflags, NV_VARNAME) && !nv_isflag(nvflags, NV_ASSIGN)) {
        error_info.errors++;
    }
    if (nv_isflag(nvflags, NV_REF) && !nv_isflag(nvflags, NV_REF) &&
        !nv_isflag(nvflags, NV_IDENT) && !nv_isflag(nvflags, NV_ASSIGN)) {
        error_info.errors++;
    }
    if (nv_isflag(nvflags, NV_TYPE) && !nv_isflag(nvflags, NV_TYPE) &&
        !nv_isflag(nvflags, NV_VARNAME) && !nv_isflag(nvflags, NV_ASSIGN)) {
        error_info.errors++;
    }
    if (troot == tdata.sh->fun_tree &&
        ((isfloat || nvflags & ~(NV_FUNCT | NV_TAGGED | NV_EXPORT | NV_LTOU)))) {
        error_info.errors++;
    }
    if (sflag && troot == tdata.sh->fun_tree) {
        // Static function.
        sflag = false;
        nvflags |= NV_STATICF;
    }
    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

#if _ast_sizeof_pointer < 8
    if (tdata.argnum > SHRT_MAX) {
        errormsg(SH_DICT, ERROR_exit(2), "option argument cannot be greater than %d", SHRT_MAX);
        __builtin_unreachable();
    }
#endif

    if (isfloat) nvflags |= NV_DOUBLE;
    if (isshort) {
        nvflags &= ~NV_LONG;
        nvflags |= NV_INT16;
    }
    if (sflag) {
        if (tdata.sh->mktype) {
            nvflags |= NV_REF | NV_TAGGED;
        } else if (!tdata.sh->typeinit) {
            nvflags |= NV_STATIC | NV_IDENT;
        }
    }
    if (tdata.sh->fn_depth && !tdata.pflag) nvflags |= NV_NOSCOPE;
    if (tdata.help) tdata.help = strdup(tdata.help);
    if (nv_isflag(nvflags, NV_TYPE)) {
        Stk_t *stkp = tdata.sh->stk;
        int off = 0, offset = stktell(stkp);
        if (!tdata.prefix) return sh_outtype(tdata.sh, sfstdout);
        sfputr(stkp, NV_CLASS, -1);
        if (tdata.sh->namespace) {
            off = stktell(stkp) + 1;
            sfputr(stkp, nv_name(tdata.sh->namespace), '.');
        } else {
            sfputc(stkp, '.');
        }
        sfputr(stkp, tdata.prefix, 0);
        tdata.tp = nv_open(stkptr(stkp, offset), tdata.sh->var_tree, NV_VARNAME | NV_NOARRAY);
        if (!tdata.tp && off) {
            *stkptr(stkp, off) = 0;
            tdata.tp = nv_open(stkptr(stkp, offset), tdata.sh->var_tree, NV_VARNAME | NV_NOARRAY);
        }
        stkseek(stkp, offset);
        if (!tdata.tp) {
            errormsg(SH_DICT, ERROR_exit(1), "%s: unknown type", tdata.prefix);
            __builtin_unreachable();
        } else if (nv_isnull(tdata.tp)) {
            nv_newtype(tdata.tp);
        }
        tdata.tp->nvenv = (Namval_t *)tdata.help;
        tdata.tp->nvenv_is_cp = true;
        nvflags &= ~NV_TYPE;
        if (nv_isattr(tdata.tp, NV_TAGGED)) {
            nv_offattr(tdata.tp, NV_TAGGED);
            return 0;
        }
    } else if (tdata.aflag == 0 && ntp && ntp->tp) {
        tdata.aflag = '-';
    }
    if (!tdata.sh->mktype) tdata.help = NULL;
    if (tdata.aflag == '+' && argv[1] &&
        (nv_isflag(nvflags, NV_ARRAY) || nv_isflag(nvflags, NV_IARRAY) ||
         nv_isflag(nvflags, NV_COMVAR))) {
        errormsg(SH_DICT, ERROR_exit(1), e_nounattr);
        __builtin_unreachable();
    }

    // The setall() function dereferences argv[0]; i.e., it requires at least one value. But the
    // `if (*argv ...)` test above implies that may not be true when we reach this statement.
    // Tell lint tools we know that can't happen. Coverity Scan CID#340038.
    assert(*argv);
    return setall(argv, nvflags, troot, &tdata);
}

static_fn void print_value(Sfio_t *iop, Namval_t *np, struct tdata *tp) {
    char *name;
    int aflag = tp->aflag;
    Namval_t *table;

    if (nv_isnull(np)) {
        if (!np->nvflag) return;
        aflag = '+';
    } else if (nv_istable(np)) {
        Dt_t *root = tp->sh->last_root;
        Namval_t *nsp = tp->sh->namespace;
        char *cp;
        if (!tp->pflag) return;
        cp = name = nv_name(np);
        if (*name == '.') name++;
        if (tp->indent) sfnputc(iop, '\t', tp->indent);
        sfprintf(iop, "namespace %s\n", name);
        if (tp->indent) sfnputc(iop, '\t', tp->indent);
        sfprintf(iop, "{\n", name);
        tp->indent++;
        // Output types from namespace.
        tp->sh->namespace = NULL;
        tp->sh->prefix = nv_name(np) + 1;
        sh_outtype(tp->sh, iop);
        tp->sh->prefix = NULL;
        tp->sh->namespace = np;
        tp->sh->last_root = root;
        // Output variables from namespace.
        print_scan(iop, NV_NOSCOPE, nv_dict(np), aflag == '+', tp);
        tp->wctname = cp;
        tp->sh->namespace = NULL;
        // Output functions from namespace.
        print_scan(iop, NV_FUNCTION | NV_NOSCOPE, tp->sh->fun_tree, aflag == '+', tp);
        tp->wctname = NULL;
        tp->sh->namespace = nsp;
        if (--tp->indent) sfnputc(iop, '\t', tp->indent);
        sfwrite(iop, "}\n", 2);
        return;
    }
    table = tp->sh->last_table;
    sfputr(iop, nv_name(np), aflag == '+' ? '\n' : '=');
    tp->sh->last_table = table;
    if (aflag == '+') return;
    if (nv_isarray(np) && nv_arrayptr(np)) {
        nv_outnode(np, iop, -1, 0);
        sfwrite(iop, ")\n", 2);
    } else {
        if (nv_isvtree(np)) nv_onattr(np, NV_EXPORT);
        if (!(name = nv_getval(np))) name = Empty;
        if (!nv_isvtree(np)) name = sh_fmtq(name);
        sfputr(iop, name, '\n');
    }
}

int setall(char **argv, nvflag_t flag, Dt_t *troot, struct tdata *tp) {
    char *name;
    char *last = NULL;
    nvflag_t nvflags =
        (flag & (NV_ARRAY | NV_NOARRAY | NV_VARNAME | NV_IDENT | NV_ASSIGN | NV_STATIC | NV_MOVE));
    int r = 0, ref = 0;
    bool comvar = nv_isflag(flag, NV_COMVAR);
    bool iarray = nv_isflag(flag, NV_IARRAY);
    size_t len;
    Shell_t *shp = tp->sh;

    if (!shp->prefix) {
        if (!tp->pflag) nvflags |= NV_NOSCOPE;
    } else if (*shp->prefix == 0) {
        shp->prefix = NULL;
    }
    if (*argv[0] == '+') nvflags |= NV_NOADD;
    flag &= ~(NV_NOARRAY | NV_NOSCOPE | NV_VARNAME | NV_IDENT | NV_STATIC | NV_COMVAR | NV_IARRAY);
    if (argv[1]) {
        if (flag & NV_REF) {
            flag &= ~NV_REF;
            ref = 1;
            if (tp->aflag != '-') nvflags |= NV_NOREF;
        }
        if (tp->pflag) nvflags |= (NV_NOREF | NV_NOADD | NV_NOFAIL);
        argv++;
        while (*argv) {
            name = *argv++;
            unsigned newflag;
            Namval_t *np;
            Namarr_t *ap;
            Namval_t *mp;
            nvflag_t curflag;
            if (troot == shp->fun_tree) {
                // Functions can be exported or traced but not set.
                flag &= ~NV_ASSIGN;
                if (nv_isflag(flag, NV_LTOU)) {
                    // Function names cannot be special builtin.
                    if ((np = nv_search(name, shp->bltin_tree, 0)) && nv_isattr(np, BLT_SPC)) {
                        errormsg(SH_DICT, ERROR_exit(1), e_badfun, name);
                        __builtin_unreachable();
                    }
                    if (shp->namespace) {
                        np = sh_fsearch(shp, name, NV_ADD | NV_NOSCOPE);
                    } else {
                        np = nv_open(name, sh_subfuntree(shp, true),
                                     NV_NOARRAY | NV_IDENT | NV_NOSCOPE);
                    }
                } else {
                    if (shp->prefix) {
                        sfprintf(shp->strbuf, "%s.%s%c", shp->prefix, name, 0);
                        name = sfstruse(shp->strbuf);
                    }
                    np = NULL;
                    if (shp->namespace) np = sh_fsearch(shp, name, NV_NOSCOPE);
                    if (!np) {
                        np = nv_search(name, troot, 0);
                        if (np) {
                            if (!is_afunction(np)) np = NULL;
                        } else if (strncmp(name, ".sh.math.", 9) == 0 && sh_mathstd(name + 9)) {
                            continue;
                        }
                    }
                }
                if (np && (nv_isflag(flag, NV_LTOU) || !nv_isnull(np) || nv_isattr(np, NV_LTOU))) {
                    if (flag == 0 && !tp->help) {
                        print_namval(sfstdout, np, tp->aflag == '+', tp);
                        continue;
                    }
                    if (shp->subshell && !shp->subshare) sh_subfork();
                    if (tp->aflag == '-') {
                        nv_onattr(np, flag | NV_FUNCTION);
                    } else if (tp->aflag == '+') {
                        nv_offattr(np, flag);
                    }
                } else {
                    r++;
                }
                if (tp->help) {
                    int offset = stktell(shp->stk);
                    if (!np) {
                        sfputr(shp->stk, shp->prefix, '.');
                        sfputr(shp->stk, name, 0);
                        np = nv_search(stkptr(shp->stk, offset), troot, 0);
                        stkseek(shp->stk, offset);
                    }
                    if (np && FETCH_VT(np->nvalue, rp)) FETCH_VT(np->nvalue, rp)->help = tp->help;
                }
                continue;
            }
            // Tracked alias.
            if (troot == shp->track_tree && tp->aflag == '-') {
                np = nv_search(name, troot, NV_ADD);
                path_alias(np, path_absolute(shp, nv_name(np), NULL));
                continue;
            }
            if (shp->nodelist && (len = strlen(name)) && name[len - 1] == '@') {
                np = *shp->nodelist++;
            } else {
                np = nv_open(name, troot, nvflags | (nv_isflag(nvflags, NV_ASSIGN) ? 0 : NV_ARRAY));
            }
            if (!np) continue;
            if (nv_isnull(np) && !nv_isarray(np) && nv_isattr(np, NV_NOFREE)) {
                nv_offattr(np, NV_NOFREE);
            } else if (tp->tp && !nv_isattr(np, NV_MINIMAL | NV_EXPORT) && (mp = np->nvenv) &&
                       (ap = nv_arrayptr(mp)) && (ap->flags & ARRAY_TREE)) {
                errormsg(SH_DICT, ERROR_exit(1), e_typecompat, nv_name(np));
                __builtin_unreachable();
            } else if ((ap = nv_arrayptr(np)) && nv_aindex(np) > 0 && ap->nelem == 1 &&
                       nv_getval(np) == Empty) {
                ap->nelem++;
                _nv_unset(np, 0);
                ap->nelem--;
            } else if (iarray && ap && ap->fun) {
                errormsg(SH_DICT, ERROR_exit(1),
                         "cannot change associative array %s to index array", nv_name(np));
                __builtin_unreachable();
            } else if ((iarray || nv_isflag(flag, NV_ARRAY)) && nv_isvtree(np) && !nv_type(np)) {
                _nv_unset(np, NV_EXPORT);
            }
            if (tp->pflag) {
                if (!nv_istable(np)) nv_attribute(np, sfstdout, tp->prefix, 1);
                print_value(sfstdout, np, tp);
                continue;
            }
            if (flag == NV_ASSIGN && !ref && tp->aflag != '-' && !strchr(name, '=')) {
                if (troot != shp->var_tree &&
                    (nv_isnull(np) || !print_namval(sfstdout, np, false, tp))) {
                    sfprintf(sfstderr, sh_translate(e_noalias), name);
                    r++;
                }
                if (!comvar && !iarray) continue;
            }
            if (!nv_isarray(np) && !strchr(name, '=') &&
                !(shp->envlist && nv_onlist(shp->envlist, name))) {
                bool export_import =
                    nv_isattr(np, (NV_EXPORT | NV_IMPORT)) == (NV_EXPORT | NV_IMPORT);
                if (comvar || (shp->last_root == shp->var_tree &&
                               ((tp->tp && tp->tp != nv_type(np)) ||
                                (!shp->st.real_fun && (nvflags & NV_STATIC)) ||
                                (!(flag & (NV_EXPORT | NV_RDONLY)) && export_import)))) {
                    bool nv_int_set = (flag & (NV_HOST | NV_INTEGER)) != NV_HOST;
                    if (nv_int_set) _nv_unset(np, NV_EXPORT);
                }
            }
            if (troot == shp->var_tree) {
                if (iarray) {
                    if (tp->tname) {
                        nv_atypeindex(np, tp->tname + 1);
                    } else if (nv_isnull(np)) {
                        nv_onattr(np, NV_ARRAY | (comvar ? NV_NOFREE : 0));
                    } else {
                        if (ap && comvar) ap->flags |= ARRAY_TREE;
                        nv_putsub(np, NULL, 0, 0);
                    }
                } else if (nvflags & NV_ARRAY) {
                    if (comvar) {
                        Namarr_t *ap = nv_arrayptr(np);
                        if (ap) {
                            ap->flags |= ARRAY_TREE;
                        } else {
                            _nv_unset(np, NV_RDONLY);
                            nv_onattr(np, NV_NOFREE);
                        }
                    }
                    nv_setarray(np, nv_associative);
                } else if (comvar && !nv_isvtree(np) && !nv_rename(np, flag | NV_COMVAR)) {
                    nv_setvtree(np);
                }
            }
            if (flag & NV_MOVE) {
                nv_rename(np, flag);
                nv_close(np);
                continue;
            }
            if (tp->tp && nv_type(np) != tp->tp) {
                nv_settype(np, tp->tp, tp->aflag == '-' ? 0 : NV_APPEND);
                flag = (np->nvflag & NV_NOCHANGE);
            }
            if (tp->tp) nv_checkrequired(np);
            flag &= ~NV_ASSIGN;
            last = strchr(name, '=');
            if (last) *last = 0;
            if (shp->typeinit) continue;
            curflag = np->nvflag;
            if (!(flag & NV_INTEGER) && (flag & (NV_UNSIGN | NV_LONG))) {
                Namfun_t *fp;
                char *cp;
                if (!tp->wctname) {
                    errormsg(SH_DICT, ERROR_exit(1), e_mapchararg, nv_name(np));
                    __builtin_unreachable();
                }
                cp = (char *)nv_mapchar(np, NULL);
                fp = nv_mapchar(np, tp->wctname);
                if (fp) {
                    if (tp->aflag == '+') {
                        if (cp && strcmp(cp, tp->wctname) == 0) {
                            nv_disc(np, fp, DISC_OP_POP);
                            if (!(fp->nofree & 1)) free(fp);
                            nv_offattr(np, flag & (NV_UNSIGN | NV_LONG));
                        }
                    } else if (!cp || strcmp(cp, tp->wctname)) {
                        nv_disc(np, fp, DISC_OP_LAST);
                        nv_onattr(np, flag & (NV_UNSIGN | NV_LONG));
                    }
                }
            }
            if (tp->aflag == '-') {
                if ((flag & NV_EXPORT) && (strchr(name, '.') || nv_isvtree(np))) {
                    errormsg(SH_DICT, ERROR_exit(1), e_badexport, name);
                    __builtin_unreachable();
                }
#if SHOPT_BASH
                if (flag & NV_EXPORT) nv_offattr(np, NV_IMPORT);
#endif  // SHOPT_BASH
                newflag = curflag;
                if (flag & ~NV_NOCHANGE) newflag &= NV_NOCHANGE;
                newflag |= flag;
                if (flag & (NV_LJUST | NV_RJUST)) {
                    if (!(flag & NV_RJUST)) {
                        newflag &= ~NV_RJUST;
                    } else if (!(flag & NV_LJUST)) {
                        newflag &= ~NV_LJUST;
                    }
                }
            } else {
                if ((flag & NV_RDONLY) && (curflag & NV_RDONLY)) {
                    errormsg(SH_DICT, ERROR_exit(1), e_readonly, nv_name(np));
                    __builtin_unreachable();
                }
                newflag = curflag & ~flag;
            }
            if (tp->aflag && (tp->argnum > 0 || (curflag != newflag))) {
                if (shp->subshell) sh_assignok(np, 1);
                if (troot != shp->var_tree) {
                    nv_setattr(np, newflag & ~NV_ASSIGN);
                } else {
                    char *oldname = NULL;
                    size_t len = strlen(name);
                    if (tp->argnum == 1 && newflag == NV_INTEGER && nv_isattr(np, NV_INTEGER)) {
                        tp->argnum = 10;
                    }
                    if (np->nvfun && !nv_isarray(np) && name[len - 1] == '.') newflag |= NV_NODISC;
                    nv_newattr(np, newflag & ~NV_ASSIGN, tp->argnum);
                    if (oldname) np->nvname = oldname;
                }
            }
            if (tp->help && !nv_isattr(np, NV_MINIMAL | NV_EXPORT)) {
                np->nvenv = (Namval_t *)tp->help;
                np->nvenv_is_cp = true;
                nv_onattr(np, NV_EXPORT);
            }
            if (last) *last = '=';
            // Set or unset references.
            if (ref) {
                if (tp->aflag == '-') {
                    Dt_t *hp = NULL;
                    if (nv_isattr(np, NV_PARAM) && shp->st.prevst) {
                        hp = shp->st.prevst->save_tree;
                        if (!hp) hp = dtvnext(shp->var_tree);
                    }
                    if (tp->sh->mktype) {
                        nv_onattr(np, NV_REF | NV_FUNCT);
                    } else {
                        nv_setref(np, hp, NV_VARNAME);
                    }
                } else {
                    nv_unref(np);
                }
            }
            nv_close(np);
        }
    } else {
        if (shp->prefix) errormsg(SH_DICT, 2, e_subcomvar, shp->prefix);
        if (tp->aflag) {
            if (troot == shp->fun_tree) {
                flag |= NV_FUNCTION;
                tp->prefix = NULL;
            } else if (troot == shp->var_tree) {
                flag |= (nvflags & NV_ARRAY);
                if (iarray) flag |= NV_ARRAY | NV_IARRAY;
                if (comvar) flag |= NV_TABLE;
                if (!(flag & ~NV_ASSIGN)) tp->noref = 1;
            }
            if ((flag & (NV_UTOL | NV_LTOU)) == (NV_UTOL | NV_LTOU)) {
                print_scan(sfstdout, flag & ~NV_UTOL, troot, tp->aflag == '+', tp);
                flag &= ~NV_LTOU;
            }
            print_scan(sfstdout, flag, troot, tp->aflag == '+', tp);
            if (tp->noref) {
                tp->noref = 0;
                print_scan(sfstdout, flag | NV_REF, troot, tp->aflag == '+', tp);
            }
        } else if (troot == shp->alias_tree) {
            print_scan(sfstdout, 0, troot, false, tp);
        } else {
            print_all(sfstdout, troot, tp);
        }
        sfsync(sfstdout);
    }
    return r;
}

//
// Print out the name and value of a name-value pair <np>.
//
static_fn int print_namval(Sfio_t *file, Namval_t *np, bool omit_attrs, struct tdata *tp) {
    char *cp;
    int indent = tp->indent, outname = 0, isfun;
    struct Ufunction *rp;

    sh_sigcheck(tp->sh);
    if (tp->noref && nv_isref(np)) return 0;
    if (nv_isattr(np, NV_NOPRINT | NV_INTEGER) == NV_NOPRINT) {
        if (is_abuiltin(np) && strcmp(np->nvname, ".sh.tilde")) {
            if (tp->prefix) sfputr(file, tp->prefix, ' ');
            sfputr(file, nv_name(np), '\n');
        }
        return 0;
    }
    if (nv_istable(np)) {
        print_value(file, np, tp);
        return 0;
    }
    isfun = is_afunction(np);
    if (tp->prefix) {
        outname = (*tp->prefix == 't' &&
                   (!nv_isnull(np) ||
                    nv_isattr(np, NV_FLOAT | NV_RDONLY | NV_BINARY | NV_RJUST | NV_NOPRINT)));
        if (indent && (isfun || outname || *tp->prefix != 't')) {
            sfnputc(file, '\t', indent);
            indent = 0;
        }
        if (!isfun) {
            if (*tp->prefix == 't') {
                nv_attribute(np, tp->outfile, tp->prefix, tp->aflag);
            } else {
                sfputr(file, tp->prefix, ' ');
            }
        }
    }
    if (isfun) {
        Sfio_t *iop = NULL;
        char *fname = NULL;
        if (nv_isattr(np, NV_NOFREE)) return 0;
        if (!omit_attrs) {
            if (!FETCH_VT(np->nvalue, ip)) {
                sfputr(file, "typeset -fu", ' ');
            } else if (!nv_isattr(np, NV_FPOSIX)) {
                sfputr(file, "function", ' ');
            }
        }
        cp = nv_name(np);
        if (tp->wctname) cp += strlen(tp->wctname) + 1;
        sfputr(file, cp, -1);
        if (nv_isattr(np, NV_FPOSIX)) sfwrite(file, "()", 2);
        rp = FETCH_VT(np->nvalue, rp);
        if (rp && rp->hoffset >= 0) {
            fname = rp->fname;
        } else {
            omit_attrs = false;
        }
        if (omit_attrs) {
            rp = FETCH_VT(np->nvalue, rp);
            if (tp->pflag && rp && rp->hoffset >= 0) {
                sfprintf(file, " #line %d %s\n", rp->lineno, fname ? sh_fmtq(fname) : "");
            } else {
                sfputc(file, '\n');
            }
        } else {
            if (nv_isattr(np, NV_FTMP)) {
                fname = 0;
                iop = tp->sh->heredocs;
            } else if (fname) {
                iop = sfopen(iop, fname, "r");
            } else if (tp->sh->gd->hist_ptr) {
                iop = (tp->sh->gd->hist_ptr)->histfp;
            }
            if (iop && sfseek(iop, FETCH_VT(np->nvalue, rp)->hoffset, SEEK_SET) >= 0) {
                sfmove(iop, file, nv_size(np), -1);
            }
            if (fname) sfclose(iop);
        }
        return nv_size(np) + 1;
    }

    if (nv_arrayptr(np)) {
        if (indent) sfnputc(file, '\t', indent);
        print_value(file, np, tp);
        return 0;
    }

    if (nv_isvtree(np)) nv_onattr(np, NV_EXPORT);
    cp = nv_getval(np);
    if (cp) {
        if (indent) sfnputc(file, '\t', indent);
        sfputr(file, nv_name(np), -1);
        sfputc(file, omit_attrs ? '\n' : '=');
        if (!omit_attrs) {
            if (nv_isref(np) && nv_refsub(np)) {
                sfputr(file, sh_fmtq(cp), -1);
                sfprintf(file, "[%s]\n", sh_fmtq(nv_refsub(np)));
            } else {
                sfputr(file, nv_isvtree(np) ? cp : sh_fmtq(cp), '\n');
            }
        }
        return 1;
    } else if (outname || (tp->scanmask && tp->scanroot == tp->sh->var_tree)) {
        sfputr(file, nv_name(np), '\n');
    }

    return 0;
}

//
// Print attributes at all nodes.
//
static_fn void print_all(Sfio_t *file, Dt_t *root, struct tdata *tp) {
    tp->outfile = file;
    nv_scan(root, print_attribute, tp, 0, 0);
}

//
// Print the attributes of name value pair give by <np>.
//
static_fn void print_attribute(Namval_t *np, void *data) {
    struct tdata *dp = (struct tdata *)data;
    nv_attribute(np, dp->outfile, dp->prefix, dp->aflag);
}

//
// Print the nodes in tree <root> which have attributes <flag> set of <option> is non-zero, no
// subscript or value is printed.
//
void print_scan(Sfio_t *file, nvflag_t flag, Dt_t *root, bool omit_attrs, struct tdata *tp) {
    char **argv;
    Namval_t *np;
    int namec;
    Namval_t *onp = NULL;
    char *name = NULL;
    size_t len;

    tp->sh->last_table = NULL;
    flag &= ~NV_ASSIGN;
    tp->scanmask = flag & ~NV_NOSCOPE;
    tp->scanroot = root;
    tp->outfile = file;
    if (!tp->prefix && tp->tp) tp->prefix = nv_name(tp->tp);
    if (nv_isflag(flag, NV_INTEGER)) tp->scanmask |= (NV_DOUBLE | NV_EXPNOTE);
    if (flag == NV_LTOU || flag == NV_UTOL) tp->scanmask |= NV_UTOL | NV_LTOU;
    if (root == tp->sh->bltin_tree) tp->scanmask |= BLT_DISABLE;
    namec = nv_scan(root, NULL, tp, tp->scanmask, flag & ~NV_IARRAY);
    argv = tp->argnam = stkalloc(tp->sh->stk, (namec + 1) * sizeof(char *));
    namec = nv_scan(root, pushname, tp, tp->scanmask, flag & ~NV_IARRAY);
    strsort(argv, namec, strcoll);
    if (namec == 0 && tp->sh->namespace && nv_dict(tp->sh->namespace) == root) {
        sfnputc(file, '\t', tp->indent);
        sfwrite(file, ":\n", 2);
    } else {
        while (namec--) {
            np = nv_search(*argv++, root, 0);
            if (!np || np == onp) continue;
            if (nv_isnull(np) && !np->nvfun && !nv_isattr(np, ~NV_NOFREE)) continue;

            onp = np;
            if (name) {
                char *newname = nv_name(np);
                if (strncmp(name, newname, len) == 0 && newname[len] == '.') continue;
                name = NULL;
            }
            if (flag & NV_ARRAY) {
                if (nv_aindex(np) >= 0) {
                    if (!(flag & NV_IARRAY)) continue;
                } else if ((flag & NV_IARRAY)) {
                    continue;
                }
            }
            tp->scanmask = flag & ~NV_NOSCOPE;
            tp->scanroot = root;
            tp->sh->last_root = root;
            print_namval(file, np, omit_attrs, tp);
            if (!is_abuiltin(np) && nv_isvtree(np)) {
                name = nv_name(np);
                len = strlen(name);
            }
        }
    }
}

//
// Add the name of the node to the argument list argnam.
//
static_fn void pushname(Namval_t *np, void *data) {
    struct tdata *tp = (struct tdata *)data;
    *tp->argnam++ = nv_name(np);
}
