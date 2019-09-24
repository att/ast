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

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "argnod.h"
#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "jobs.h"
#include "name.h"
#include "path.h"
#include "shcmd.h"
#include "shnodes.h"

struct login {
    Shell_t *sh;
    int clear;
    char *arg0;
};

static_fn void noexport(Namval_t *np, void *data) {
    UNUSED(data);
    nv_offattr(np, NV_EXPORT);
}

static int exec_args(char *argv[], struct login *logp) {
    Shell_t *shp = logp->sh;

    if (sh_isoption(shp, SH_RESTRICTED)) {
        errormsg(SH_DICT, ERROR_exit(1), e_restricted, argv[0]);
        __builtin_unreachable();
    }

    checkpt_t *pp = shp->jmplist;
    struct argnod *arg = shp->envlist;
    Namval_t *np;
    char *cp;
    const char *pname;

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
    abort();
}

static const char *short_options = "+:a:c";
static const struct option long_options[] = {
    {"help", no_argument, NULL, 1},  // all builtins support --help
    {NULL, 0, NULL, 0}};

//
// Builtin `exec` command.
//
int b_exec(int argc, char *argv[], Shbltin_t *context) {
    struct login logdata;
    int opt;
    logdata.clear = 0;
    logdata.arg0 = NULL;
    logdata.sh = context->shp;
    logdata.sh->st.ioset = 0;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];

    optind = opterr = 0;
    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 'a': {
                logdata.arg0 = optarg;
                break;
            }
            case 'c': {
                logdata.clear = 1;
                break;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[optind - 1]);
                return 2;
            }
            case '?': {
                builtin_unknown_option(shp, cmd, argv[optind - 1]);
                return 2;
            }
            default: { abort(); }
        }
    }

    argv += optind;
    if (*argv) return exec_args(argv, &logdata);
    return 0;
}
