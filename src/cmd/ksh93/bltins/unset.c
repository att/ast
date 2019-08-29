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

#include <setjmp.h>
#include <stdbool.h>
#include <string.h>

#include "builtins.h"
#include "cdt.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "name.h"
#include "option.h"
#include "sfio.h"
#include "shcmd.h"

static_fn int unall(int, char **, Dt_t *, Shell_t *);

// The `unalias` builtin.
int b_unalias(int argc, char *argv[], Shbltin_t *context) {
    Shell_t *shp = context->shp;
    return unall(argc, argv, shp->alias_tree, shp);
}

// The `unset` builtin.
int b_unset(int argc, char *argv[], Shbltin_t *context) {
    Shell_t *shp = context->shp;
    return unall(argc, argv, shp->var_tree, shp);
}

//
// The removing of Shell variable names, aliases, and functions is performed here. Unset functions
// with unset -f. Non-existent items being deleted give non-zero exit status.
//
static_fn int unall(int argc, char **argv, Dt_t *troot, Shell_t *shp) {
    Namval_t *np;
    const char *name;
    volatile int r;
    Dt_t *dp;
    nvflag_t nvflags = 0;
    int all = 0, isfun, jmpval;
    checkpt_t buff;
    enum { ALIAS, VARIABLE } type;
    UNUSED(argc);

    if (troot == shp->alias_tree) {
        type = ALIAS;
        name = sh_optunalias;
        if (shp->subshell) troot = sh_subaliastree(shp, 0);
    } else {
        type = VARIABLE;
        name = sh_optunset;
    }
    while ((r = optget(argv, name))) {
        switch (r) {  //!OCLINT(MissingDefaultStatement)
            case 'f': {
                troot = sh_subfuntree(shp, true);
                break;
            }
            case 'a': {
                all = 1;
                break;
            }
            case 'n': {
                nvflags = NV_NOREF;
            }
            // FALLTHRU
            case 'v': {
                troot = shp->var_tree;
                break;
            }
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(0), "%s", opt_info.arg);
                return 2;
            }
        }
    }
    argv += opt_info.index;
    if (error_info.errors || (*argv == 0 && !all)) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    if (!troot) return 1;
    r = 0;
    if (troot == shp->var_tree) {
        nvflags |= NV_VARNAME;
    } else {
        nvflags = NV_NOSCOPE;
    }
    if (all) {
        dtclear(troot);
        return r;
    }
    sh_pushcontext(shp, &buff, 1);
    while (*argv) {
        name = *argv++;
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
            } else if (type == ALIAS) {
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

        } else if (type == ALIAS) {
            // Alias not found
            sfprintf(sfstderr, sh_translate(e_noalias), name);
            r = 1;
        }
    }
    sh_popcontext(shp, &buff);
    return r;
}
