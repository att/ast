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
#include "config_ast.h"  // IWYU pragma: keep

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "io.h"
#include "name.h"
#include "option.h"
#include "path.h"
#include "sfio.h"
#include "shcmd.h"
#include "shnodes.h"
#include "stk.h"
#include "variables.h"

#define DOTMAX MAXDEPTH  // maximum level of . nesting -- same as path recursion max

int b_source(int n, char *argv[], Shbltin_t *context) {
    char *script;
    Namval_t *np;
    int jmpval;
    Shell_t *shp = context->shp;
    struct sh_scoped savst, *prevscope = shp->st.self;
    int fd;
    char *filename = NULL;
    char *buffer = NULL;
    struct dolnod *saveargfor = NULL;
    volatile struct dolnod *argsave = NULL;
    checkpt_t buff;
    Sfio_t *iop = NULL;
    short level;
    Optdisc_t disc;

    memset(&disc, 0, sizeof(disc));
    disc.version = OPT_VERSION;
    opt_info.disc = &disc;

    while ((n = optget(argv, sh_optdot))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
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
    script = *argv;
    if (error_info.errors || !script) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    if (shp->dot_depth + 1 > DOTMAX) {
        errormsg(SH_DICT, ERROR_exit(1), e_toodeep, script);
        __builtin_unreachable();
    }
    np = shp->posix_fun;
    if (!np) {
        // Check for KornShell style function first.
        np = nv_search(script, shp->fun_tree, 0);
        if (np && is_afunction(np) && !nv_isattr(np, NV_FPOSIX)) {
            if (!FETCH_VT(np->nvalue, ip)) {
                // TODO: Replace this with a comment explaining why the return value of this
                // path_search() call is ignored. At the time I wrote this (2019-03-16) no unit test
                // exercises this statement. I added the void cast to silence Coverity Scan 253792.
                (void)path_search(shp, script, NULL, 0);
                if (FETCH_VT(np->nvalue, ip)) {
                    if (nv_isattr(np, NV_FPOSIX)) np = NULL;
                } else {
                    errormsg(SH_DICT, ERROR_exit(1), e_found, script);
                    __builtin_unreachable();
                }
            }
        } else {
            np = NULL;
        }

        if (!np) {
            fd = path_open(shp, script, path_get(shp, script));
            if (fd < 0) {
                errormsg(SH_DICT, ERROR_system(1), e_open, script);
                __builtin_unreachable();
            }
            filename = path_fullname(shp, stkptr(shp->stk, PATH_OFFSET));
        }
    }
    *prevscope = shp->st;
    shp->st.lineno = np ? ((struct functnod *)nv_funtree(np))->functline : 1;
    shp->st.var_local = shp->st.save_tree = shp->var_tree;
    if (filename) {
        shp->st.filename = filename;
        shp->st.lineno = 1;
    }
    level = shp->fn_depth + shp->dot_depth + 1;
    nv_putval(SH_LEVELNOD, (char *)&level, NV_INT16);
    shp->st.prevst = prevscope;
    shp->st.self = &savst;
    shp->topscope = (Shscope_t *)shp->st.self;
    prevscope->save_tree = shp->var_tree;
    if (np) {
        struct Ufunction *rp = FETCH_VT(np->nvalue, rp);
        shp->st.filename = strdup(rp->fname ? rp->fname : "");
    }
    nv_putval(SH_PATHNAMENOD, shp->st.filename, NV_NOFREE);
    shp->posix_fun = NULL;
    if (np || argv[1]) argsave = sh_argnew(shp, argv, &saveargfor);
    sh_pushcontext(shp, &buff, SH_JMPDOT);
    jmpval = sigsetjmp(buff.buff, 0);
    if (jmpval == 0) {
        shp->dot_depth++;
        if (np) {
            sh_exec(shp, (Shnode_t *)(nv_funtree(np)), sh_isstate(shp, SH_ERREXIT));
        } else {
            buffer = malloc(IOBSIZE + 1);
            iop = sfnew(NULL, buffer, IOBSIZE, fd, SF_READ);
            sh_offstate(shp, SH_NOFORK);
            sh_eval(shp, iop, sh_isstate(shp, SH_PROFILE) ? SH_FUNEVAL : 0);
        }
    }
    sh_popcontext(shp, &buff);
    if (buffer) free(buffer);
    if (!np) {
        free(shp->st.filename);
        shp->st.filename = NULL;
    }
    shp->dot_depth--;
    if ((np || argv[1]) && jmpval != SH_JMPSCRIPT) {
        sh_argreset(shp, (struct dolnod *)argsave, saveargfor);
    } else {
        prevscope->dolc = shp->st.dolc;
        prevscope->dolv = shp->st.dolv;
    }
    if (shp->st.self != &savst) *shp->st.self = shp->st;
    // Only restore the top Shscope_t portion for posix functions.
    memcpy(&shp->st, prevscope, sizeof(Shscope_t));
    shp->topscope = (Shscope_t *)prevscope;
    nv_putval(SH_PATHNAMENOD, shp->st.filename, NV_NOFREE);
    if (jmpval && jmpval != SH_JMPFUN) siglongjmp(shp->jmplist->buff, jmpval);
    return shp->exitval;
}
