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
// exec [arg...]
// login [arg...]
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "argnod.h"
#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "jobs.h"
#include "name.h"
#include "option.h"
#include "path.h"
#include "shcmd.h"
#include "shnodes.h"

struct login {
    Shell_t *sh;
    int clear;
    char *arg0;
};

//
// Builtin `exec`.
//
int b_exec(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);
    struct login logdata;
    int n;
    logdata.clear = 0;
    logdata.arg0 = NULL;
    logdata.sh = context->shp;
    logdata.sh->st.ioset = 0;
    while ((n = optget(argv, sh_optexec))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'a': {
                logdata.arg0 = opt_info.arg;
                break;
            }
            case 'c': {
                logdata.clear = 1;
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
    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    if (*argv) B_login(0, argv, (Shbltin_t *)&logdata);
    return 0;
}

static_fn void noexport(Namval_t *np, void *data) {
    UNUSED(data);
    nv_offattr(np, NV_EXPORT);
}

//
// Builtin `login`.
//
int B_login(int argc, char *argv[], Shbltin_t *context) {
    checkpt_t *pp;
    struct login *logp = NULL;
    Shell_t *shp;
    const char *pname;
    if (argc) {
        shp = context->shp;
    } else {
        logp = (struct login *)context;
        shp = logp->sh;
    }

    pp = shp->jmplist;
    if (sh_isoption(shp, SH_RESTRICTED)) {
        errormsg(SH_DICT, ERROR_exit(1), e_restricted, argv[0]);
        __builtin_unreachable();
    } else {
        struct argnod *arg = shp->envlist;
        Namval_t *np;
        char *cp;
        if (shp->subshell && !shp->subshare) sh_subfork();
        if (logp && logp->clear) {
            nv_scan(shp->var_tree, noexport, 0, NV_EXPORT, NV_EXPORT);
        }
        while (arg) {
            cp = strchr(arg->argval, '=');
            if (cp && (*cp = 0, np = nv_search(arg->argval, shp->var_tree, 0))) {
                nv_onattr(np, NV_EXPORT);
                sh_envput(shp, np);
            }
            if (cp) *cp = '=';
            arg = arg->argnxt.ap;
        }
        pname = argv[0];
        if (logp && logp->arg0) argv[0] = logp->arg0;
#ifdef JOBS
        if (job_close(shp) < 0) return 1;
#endif  // JOBS
        // Force bad exec to terminate shell.
        pp->mode = SH_JMPEXIT;
        sh_sigreset(shp, 2);
        sh_freeup(shp);
        path_exec(shp, pname, argv, NULL);
        sh_done(shp, 0);
    }
    return 1;
}
