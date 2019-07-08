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
// eval [arg...]
// jobs [-lnp] [job...]
// login [arg...]
// let expr...
// . file [arg...]
// :, true, false
// vpath [top] [base]
// vmap [top] [base]
// wait [job...]
// shift [n]
//
//   David Korn
//   AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "argnod.h"
#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "fault.h"
#include "io.h"
#include "jobs.h"
#include "name.h"
#include "option.h"
#include "path.h"
#include "sfio.h"
#include "shcmd.h"
#include "shnodes.h"
#include "stk.h"
#include "variables.h"

#define DOTMAX MAXDEPTH  // Maximum level of . nesting.

static_fn void noexport(Namval_t *, void *);

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
        switch (n) {
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
            default: { break; }
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

int b_eval(int argc, char *argv[], Shbltin_t *context) {
    int r;
    Shell_t *shp = context->shp;
    UNUSED(argc);

    while ((r = optget(argv, sh_opteval))) {
        switch (r) {
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(0), "%s", opt_info.arg);
                return 2;
            }
            default: { break; }
        }
    }

    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    argv += opt_info.index;
    if (*argv && **argv) {
        sh_offstate(shp, SH_MONITOR);
        sh_eval(shp, sh_sfeval((const char **)argv), 0);
    }
    return shp->exitval;
}

int b_dot_cmd(int n, char *argv[], Shbltin_t *context) {
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
        switch (n) {
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(0), "%s", opt_info.arg);
                return 2;
            }
            default: { break; }
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

//
// Builtin `shift`.
//
int b_shift(int n, char *argv[], Shbltin_t *context) {
    char *arg;
    Shell_t *shp = context->shp;
    while ((n = optget(argv, sh_optshift))) {
        switch (n) {
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(0), "%s", opt_info.arg);
                return 2;
            }
            default: { break; }
        }
    }

    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    argv += opt_info.index;
    n = ((arg = *argv) ? (int)sh_arith(shp, arg) : 1);
    if (n < 0 || shp->st.dolc < n) {
        errormsg(SH_DICT, ERROR_exit(1), e_number, arg);
        __builtin_unreachable();
    } else {
        shp->st.dolv += n;
        shp->st.dolc -= n;
    }
    return 0;
}

//
// Builtin `wait`.
//
int b_wait(int n, char *argv[], Shbltin_t *context) {
    Shell_t *shp = context->shp;
    while ((n = optget(argv, sh_optwait))) {
        switch (n) {
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
            default: { break; }
        }
    }

    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    argv += opt_info.index;
    job_bwait(argv);
    return shp->exitval;
}

#ifdef JOBS
//
// Builtin `bg`.
//
int b_bg(int n, char *argv[], Shbltin_t *context) {
    int flag = **argv;
    Shell_t *shp = context->shp;
    const char *optstr = sh_optbg;
    if (*argv[0] == 'f') {
        optstr = sh_optfg;
    } else if (*argv[0] == 'd') {
        optstr = sh_optdisown;
    }

    while ((n = optget(argv, optstr))) {
        switch (n) {
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
            default: { break; }
        }
    }
    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    argv += opt_info.index;
    if (!sh_isoption(shp, SH_MONITOR) || !job.jobcontrol) {
        if (sh_isstate(shp, SH_INTERACTIVE)) {
            errormsg(SH_DICT, ERROR_exit(1), e_no_jctl);
            __builtin_unreachable();
        }
        return 1;
    }
    if (flag == 'd' && *argv == 0) argv = NULL;
    if (job_walk(shp, sfstdout, job_switch, flag, argv)) {
        errormsg(SH_DICT, ERROR_exit(1), e_no_job);
        __builtin_unreachable();
    }
    return shp->exitval;
}

//
// Builtin `jobs`.
//
int b_jobs(int n, char *argv[], Shbltin_t *context) {
    int flag = 0;
    Shell_t *shp = context->shp;
    while ((n = optget(argv, sh_optjobs))) {
        switch (n) {
            case 'l': {
                flag = JOB_LFLAG;
                break;
            }
            case 'n': {
                flag = JOB_NFLAG;
                break;
            }
            case 'p': {
                flag = JOB_PFLAG;
                break;
            }
            case ':': {
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
            default: { break; }
        }
    }

    argv += opt_info.index;
    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    if (*argv == 0) argv = NULL;
    if (job_walk(shp, sfstdout, job_list, flag, argv)) {
        errormsg(SH_DICT, ERROR_exit(1), e_no_job);
        __builtin_unreachable();
    }
    job_wait((pid_t)0);
    return shp->exitval;
}
#endif  // JOBS
