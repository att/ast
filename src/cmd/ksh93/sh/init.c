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
// Shell initialization
//
//   David Korn
//   AT&T Labs
//
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#include "argnod.h"
#include "ast.h"
#include "ast_assert.h"
#include "cdt.h"
#include "defs.h"
#include "edit.h"
#include "error.h"
#include "fault.h"
#include "history.h"
#include "io.h"
#include "jobs.h"
#include "lexstates.h"
#include "name.h"
#include "option.h"
#include "path.h"
#include "sfio.h"
#include "shlex.h"
#include "shtable.h"
#include "stak.h"
#include "stk.h"
#include "variables.h"
#include "version.h"

#if USE_SPAWN
#include "ast_sys.h"
#endif

char e_version[] =
    "\n@(#)$Id: Version "
#define ATTRS 1
    "A"
#if SHOPT_BASH
#define ATTRS 1
    "B"
#endif
#if SHOPT_COSHELL
#define ATTRS 1
    "J"
#endif
#if ATTRS
    " "
#endif
    SH_RELEASE " $\0\n";

#if SHOPT_BASH
extern void bash_init(Shell_t *, int);
#endif

#define RANDMASK 0x7fff

#ifndef ARG_MAX
#define ARG_MAX (1 * 1024 * 1024)
#endif
#ifndef CHILD_MAX
#define CHILD_MAX (1 * 1024)
#endif
#ifndef CLK_TCK
#define CLK_TCK 60
#endif  // CLK_TCK

#ifndef environ
extern char **environ;
#endif

struct seconds {
    Namfun_t hdr;
    Shell_t *sh;
};

struct rand {
    Namfun_t hdr;
    Shell_t *sh;
    int32_t rand_last;
};

struct ifs {
    Namfun_t hdr;
    Namval_t *ifsnp;
};

struct match {
    Namfun_t hdr;
    const char *v;
    char *val;
    char *rval[2];
    int *match;
    char *nodes;
    char *names;
    int first;
    int vsize;
    int vlen;
    int msize;
    int nmatch;
    int index;
    int lastsub[2];
};

typedef struct _init_ {
    struct ifs IFS_init;
    Namfun_t PATH_init;
    Namfun_t FPATH_init;
    Namfun_t CDPATH_init;
    Namfun_t SHELL_init;
    Namfun_t ENV_init;
    Namfun_t VISUAL_init;
    Namfun_t EDITOR_init;
    Namfun_t HISTFILE_init;
    Namfun_t HISTSIZE_init;
    Namfun_t OPTINDEX_init;
    struct seconds SECONDS_init;
    struct rand RAND_init;
    Namfun_t LINENO_init;
    Namfun_t L_ARG_init;
    Namfun_t SH_VERSION_init;
    struct match SH_MATCH_init;
    Namfun_t SH_MATH_init;
#if SHOPT_COSHELL
    Namfun_t SH_JOBPOOL_init;
#endif  // SHOPT_COSHELL
    Namfun_t LC_TIME_init;
    Namfun_t LC_TYPE_init;
    Namfun_t LC_NUM_init;
    Namfun_t LC_COLL_init;
    Namfun_t LC_MSG_init;
    Namfun_t LC_ALL_init;
    Namfun_t LANG_init;
    Namfun_t OPTIONS_init;
    Namfun_t OPTastbin_init;
} Init_t;

static Init_t *ip;
static int nbltins;
static int shlvl;
static int rand_shift;

static_fn void env_init(Shell_t *);
static_fn Init_t *nv_init(Shell_t *);
static_fn Dt_t *inittree(Shell_t *, const struct shtable2 *);

//
// Invalidate all path name bindings.
//
static_fn void rehash(Namval_t *np, void *data) {
    UNUSED(data);

    nv_onattr(np, NV_NOALIAS);
}

//
// Out of memory routine for stak routines.
//
static_fn char *nospace(int unused) {
    UNUSED(unused);

    errormsg(SH_DICT, ERROR_exit(3), e_nospace);
    __builtin_unreachable();
}

// Trap for VISUAL and EDITOR variables.
static_fn void put_ed(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    const char *cp, *name = nv_name(np);
    int newopt = 0;
    Shell_t *shp = sh_ptr(np);

    if (*name == 'E' && nv_getval(sh_scoped(shp, VISINOD))) goto done;
    if (!(cp = val) && (*name == 'E' || !(cp = nv_getval(sh_scoped(shp, EDITNOD))))) goto done;
    // Turn on vi or emacs option if editor name is either.
    cp = path_basename(cp);
    if (strmatch(cp, "*[Vv][Ii]*")) {
        newopt = SH_VI;
    } else if (strmatch(cp, "*gmacs*")) {
        newopt = SH_GMACS;
    } else if (strmatch(cp, "*macs*")) {
        newopt = SH_EMACS;
    }
    if (newopt) {
        sh_offoption(shp, SH_VI);
        sh_offoption(shp, SH_EMACS);
        sh_offoption(shp, SH_GMACS);
        sh_onoption(shp, newopt);
    }
done:
    nv_putv(np, val, flags, fp);
}

// Trap for HISTFILE and HISTSIZE variables.
static_fn void put_history(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    Shell_t *shp = sh_ptr(np);
    void *histopen = shp ? shp->gd->hist_ptr : NULL;
    char *cp;

    if (val && histopen) {
        if (np == HISTFILE && (cp = nv_getval(np)) && strcmp(val, cp) == 0) return;
        if (np == HISTSIZE && sh_arith(shp, val) == nv_getnum(HISTSIZE)) return;
        hist_close(shp->gd->hist_ptr);
    }
    nv_putv(np, val, flags, fp);
    if (histopen) {
        if (val) {
            sh_histinit(shp);
        } else {
            hist_close(histopen);
        }
    }
}

// Trap for OPTINDEX.
static_fn void put_optindex(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    Shell_t *shp = sh_ptr(np);
    shp->st.opterror = shp->st.optchar = 0;
    nv_putv(np, val, flags, fp);
    if (!val) nv_disc(np, fp, DISC_OP_POP);
}

static_fn Sfdouble_t nget_optindex(Namval_t *np, Namfun_t *fp) {
    UNUSED(fp);

    return *FETCH_VT(np->nvalue, i32p);
}

static_fn Namfun_t *clone_optindex(Namval_t *np, Namval_t *mp, int flags, Namfun_t *fp) {
    UNUSED(flags);
    Namfun_t *dp = (Namfun_t *)malloc(sizeof(Namfun_t));

    memcpy((void *)dp, (void *)fp, sizeof(Namfun_t));
    STORE_VT(mp->nvalue, i32p, FETCH_VT(np->nvalue, i32p));
    dp->nofree = 0;
    return dp;
}

// Trap for restricted variables FPATH, PATH, SHELL, ENV.
static_fn void put_restricted(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    Shell_t *shp = sh_ptr(np);
    int path_scoped = 0, fpath_scoped = 0;
    Pathcomp_t *pp;
    char *name = nv_name(np);

    if (!(flags & NV_RDONLY) && sh_isoption(shp, SH_RESTRICTED)) {
        errormsg(SH_DICT, ERROR_exit(1), e_restricted, nv_name(np));
        __builtin_unreachable();
    }
    if (np == PATHNOD || (path_scoped = (strcmp(name, PATHNOD->nvname) == 0))) {
        nv_scan(shp->track_tree, rehash, NULL, NV_TAGGED, NV_TAGGED);
        if (path_scoped && !val) val = FETCH_VT(PATHNOD->nvalue, const_cp);
    }
    const char *cp = FETCH_VT(np->nvalue, const_cp);
    if (val && !(flags & NV_RDONLY) && cp && strcmp(val, cp) == 0) return;
    if (np == FPATHNOD || (fpath_scoped = (strcmp(name, FPATHNOD->nvname) == 0))) {
        shp->pathlist = (void *)path_unsetfpath(shp);
    }
    nv_putv(np, val, flags, fp);
    shp->universe = 0;
    if (shp->pathlist) {
        val = FETCH_VT(np->nvalue, const_cp);
        if (np == PATHNOD || path_scoped) {
            pp = (void *)path_addpath(shp, (Pathcomp_t *)shp->pathlist, val, PATH_PATH);
        } else if (val && (np == FPATHNOD || fpath_scoped)) {
            pp = (void *)path_addpath(shp, (Pathcomp_t *)shp->pathlist, val, PATH_FPATH);
        } else {
            return;
        }
        shp->pathlist = (void *)pp;
        if (pp) pp->shp = shp;
        if (!val && (flags & NV_NOSCOPE)) {
            Namval_t *mp = dtsearch(shp->var_tree, np);
            if (mp && (val = nv_getval(mp))) nv_putval(mp, val, NV_RDONLY);
        }
#if 0
sfprintf(sfstderr,"%d: name=%s val=%s\n",getpid(),name,val);
path_dump((Pathcomp_t*)shp->pathlist);
#endif
    }
}

static_fn void put_cdpath(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    Pathcomp_t *pp;
    Shell_t *shp = sh_ptr(np);

    nv_putv(np, val, flags, fp);
    if (!shp->cdpathlist) return;
    val = FETCH_VT(np->nvalue, const_cp);
    pp = (void *)path_addpath(shp, (Pathcomp_t *)shp->cdpathlist, val, PATH_CDPATH);
    shp->cdpathlist = (void *)pp;
    if (pp) pp->shp = shp;
}

#if 0
// TODO: Decide if this function serves a purpose.
//
// This function needs to be modified to handle international
// error message translations
//
static_fn char *msg_translate(const char *catalog, const char *message) {
    UNUSED(catalog);

    return (char *)message;
}
#endif

// Trap for LC_ALL, LC_CTYPE, LC_MESSAGES, LC_COLLATE and LANG.
static_fn void put_lang(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    Shell_t *shp = sh_ptr(np);
    int type;
    char *name = nv_name(np);

    // So that the platform's locale subsystem will work we need to actually put the locale var in
    // the environment arena.
    if (val) {
        setenv(name, val, 1);
    } else {
        unsetenv(name);
    }

    if (name == (LCALLNOD)->nvname) {
        type = LC_ALL;
    } else if (name == (LCTYPENOD)->nvname) {
        type = LC_CTYPE;
    } else if (name == (LCMSGNOD)->nvname) {
        type = LC_MESSAGES;
    } else if (name == (LCCOLLNOD)->nvname) {
        type = LC_COLLATE;
    } else if (name == (LCNUMNOD)->nvname) {
        type = LC_NUMERIC;
    } else if (name == (LANGNOD)->nvname && (!(name = nv_getval(LCALLNOD)) || !*name)) {
        type = LC_ALL;
    } else {
        type = -1;
    }

    if (!sh_isstate(shp, SH_INIT) && (type >= 0 || type == LC_ALL || type == LC_ALL)) {
        char *r;
        r = ast_setlocale(type, val ? val : "");
        if (!r && val) {
            if (!sh_isstate(shp, SH_INIT) || shp->login_sh == 0) {
                errormsg(SH_DICT, 0, e_badlocale, val);
            }
            return;
        }
    }

    nv_putv(np, val, flags, fp);
}

// Trap for IFS assignment and invalidates state table.
static_fn void put_ifs(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    struct ifs *ifsp = (struct ifs *)fp;
    ifsp->ifsnp = 0;
    if (!val) {
        fp = nv_stack(np, NULL);
        if (fp && !fp->nofree) {
            free(fp);
            fp = 0;
        }
    }
    if (val != FETCH_VT(np->nvalue, const_cp)) nv_putv(np, val, flags, fp);
    if (!val) {
        if (fp) fp->next = np->nvfun;
        np->nvfun = fp;
    }
}

//
// This is the lookup function for IFS. It keeps the sh.ifstable up to date.
//
static_fn char *get_ifs(Namval_t *np, Namfun_t *fp) {
    struct ifs *ifsp = (struct ifs *)fp;
    char *cp, *value;
    int c, n;
    Shell_t *shp = sh_ptr(np);

    value = nv_getv(np, fp);
    if (np == ifsp->ifsnp) return value;

    ifsp->ifsnp = np;
    memset(shp->ifstable, 0, (1 << CHAR_BIT));
    cp = value;
    if (cp) {
        while (n = mblen(cp, MB_CUR_MAX), c = *(unsigned char *)cp) {
            cp++;
            if (n > 1) {
                cp += (n - 1);
                shp->ifstable[c] = S_MBYTE;
                continue;
            }
            n = S_DELIM;
            if (c == *cp) {
                cp++;
            } else if (c == '\n') {
                n = S_NL;
            } else if (isspace(c)) {
                n = S_SPACE;
            }
            shp->ifstable[c] = n;
        }
    } else {
        shp->ifstable[' '] = shp->ifstable['\t'] = S_SPACE;
        shp->ifstable['\n'] = S_NL;
    }
    return value;
}

//
// These functions are used to get and set the SECONDS variable.
//
#ifdef timeofday
#define dtime(tp) ((double)((tp)->tv_sec) + 1e-6 * ((double)((tp)->tv_usec)))
#define tms timeval
#else
#define dtime(tp) (((double)times(tp)) / shgd->lim.clk_tck)
#define timeofday(a)
#endif

static_fn void put_seconds(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    double d;
    struct tms tp;
    if (!val) {
        nv_putv(np, val, flags, fp);
        fp = nv_stack(np, NULL);
        if (fp && !fp->nofree) free(fp);
        return;
    }
    if (!FETCH_VT(np->nvalue, dp)) {
        nv_setsize(np, 3);
        nv_onattr(np, NV_DOUBLE);
        STORE_VT(np->nvalue, dp, calloc(1, sizeof(double)));
    }
    nv_putv(np, val, flags, fp);
    d = *FETCH_VT(np->nvalue, dp);
    timeofday(&tp);
    *FETCH_VT(np->nvalue, dp) = dtime(&tp) - d;
}

static_fn char *get_seconds(Namval_t *np, Namfun_t *fp) {
    Shell_t *shp = sh_ptr(np);
    int places = nv_size(np);
    struct tms tp;
    double *dp = FETCH_VT(np->nvalue, dp);
    double d, offset = (dp ? *dp : 0.0);
    UNUSED(fp);

    timeofday(&tp);
    d = dtime(&tp) - offset;
    sfprintf(shp->strbuf, "%.*f", places, d);
    return sfstruse(shp->strbuf);
}

static_fn Sfdouble_t nget_seconds(Namval_t *np, Namfun_t *fp) {
    struct tms tp;
    double *dp = FETCH_VT(np->nvalue, dp);
    double offset = (dp ? *dp : 0.0);
    UNUSED(fp);

    timeofday(&tp);
    return dtime(&tp) - offset;
}

//
// These three functions are used to get and set the RANDOM variable.
//
static_fn void put_rand(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    struct rand *rp = (struct rand *)fp;

    if (!val) {
        fp = nv_stack(np, NULL);
        if (fp && !fp->nofree) free(fp);
        _nv_unset(np, 0);
        return;
    }

    double seedf;
    if (flags & NV_INTEGER) {
        seedf = *(double *)val;
    } else {
        seedf = sh_arith(rp->sh, val);
    }
    srand((unsigned int)seedf & RANDMASK);
    rp->rand_last = -1;
    if (!FETCH_VT(np->nvalue, i32p)) STORE_VT(np->nvalue, i32p, &rp->rand_last);
}

//
// Get random number in range of 0 - 2**15.
// Never pick same number twice in a row.
//
static_fn Sfdouble_t nget_rand(Namval_t *np, Namfun_t *fp) {
    long cur, last = *FETCH_VT(np->nvalue, i32p);
    UNUSED(fp);

    do {
        cur = (rand() >> rand_shift) & RANDMASK;
    } while (cur == last);
    *FETCH_VT(np->nvalue, i32p) = cur;
    return (Sfdouble_t)cur;
}

static_fn char *get_rand(Namval_t *np, Namfun_t *fp) {
    long n = nget_rand(np, fp);
    return fmtbase(n, 10, 0);
}

//
// These three routines are for LINENO.
//
static_fn Sfdouble_t nget_lineno(Namval_t *np, Namfun_t *fp) {
    double d = 1;
    UNUSED(np);
    UNUSED(fp);

    if (error_info.line > 0) {
        d = error_info.line;
    } else if (error_info.context && error_info.context->line > 0) {
        d = error_info.context->line;
    }
    return d;
}

static_fn void put_lineno(Namval_t *np, const void *vp, int flags, Namfun_t *fp) {
    Shell_t *shp = sh_ptr(np);
    if (!vp) {
        fp = nv_stack(np, NULL);
        if (fp && !fp->nofree) free(fp);
        _nv_unset(np, 0);
        return;
    }

    double lineno;
    if (flags & NV_INTEGER) {
        lineno = *(double *)vp;
    } else {
        lineno = sh_arith(shp, vp);
    }
    shp->st.firstline += nget_lineno(np, fp) + 1 - (long)lineno;
}

static_fn char *get_lineno(Namval_t *np, Namfun_t *fp) {
    long n = nget_lineno(np, fp);
    return fmtbase(n, 10, 0);
}

static_fn char *get_lastarg(Namval_t *np, Namfun_t *fp) {
    UNUSED(fp);
    Shell_t *shp = sh_ptr(np);
    char *cp;
    int pid;

    if (sh_isstate(shp, SH_INIT) && (cp = shp->lastarg) && *cp == '*' &&
        (pid = strtol(cp + 1, &cp, 10)) && *cp == '*') {
        nv_putval(np, cp + 1, 0);
    }
    return shp->lastarg;
}

static_fn void put_lastarg(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    UNUSED(fp);
    Shell_t *shp = sh_ptr(np);
    if (flags & NV_INTEGER) {
        sfprintf(shp->strbuf, "%.*g", 12, *((double *)val));
        val = sfstruse(shp->strbuf);
    }
    if (val) val = strdup(val);
    if (shp->lastarg && !nv_isattr(np, NV_NOFREE)) {
        free(shp->lastarg);
    } else {
        nv_offattr(np, NV_NOFREE);
    }
    shp->lastarg = (char *)val;
    nv_offattr(np, NV_EXPORT);
    np->nvenv = 0;
}

static_fn void astbin_update(Shell_t *shp, const char *from, const char *to) {
    Namval_t *mp, *np;
    const char *path;
    size_t len, tolen, flen;
    int offset = stktell(shp->stk);
    int bin = (strcmp(from, "/bin") == 0 || strcmp(from, "/usr/bin") == 0);
    int tobin = (strcmp(to, "/bin") == 0 || strcmp(to, "/usr/bin") == 0);
    if (bin) from = "/usr/bin";
    if (tobin) to = "/usr/bin";
    len = strlen(from);
    tolen = strlen(to);
    for (np = (Namval_t *)dtfirst(shp->bltin_tree); np;
         np = (Namval_t *)dtnext(shp->bltin_tree, np)) {
        flen = len;
        if (bin && strncmp(from + 4, np->nvname, len - 4) == 0) {
            flen -= 4;
        } else if (strncmp(from, np->nvname, len)) {
            continue;
        }
        nv_onattr(np, BLT_DISABLE);
        mp = nv_search(strrchr(np->nvname, '/') + 1, shp->track_tree, 0);
        if (mp) nv_onattr(mp, NV_NOALIAS);
        stkseek(shp->stk, offset);
        sfwrite(shp->stk, to, tolen);
        sfputr(shp->stk, np->nvname + flen, 0);
        path = stkptr(shp->stk, offset);
        mp = 0;
        if (tobin) {
            mp = nv_search(path, shp->bltin_tree, 0);
            if (!mp) path += 4;
        }
        if (!mp) mp = nv_search(path, shp->bltin_tree, 0);
        if (mp) {
            nv_offattr(mp, BLT_DISABLE);
        } else {
            sh_addbuiltin(shp, path, FETCH_VT(np->nvalue, shbltinp), 0);
        }
    }
    if (strcmp(from, SH_CMDLIB_DIR) == 0) path_cmdlib(shp, from, false);
    if (strcmp(to, SH_CMDLIB_DIR) == 0) path_cmdlib(shp, to, true);
}

static_fn void put_astbin(Namval_t *np, const void *vp, int flags, Namfun_t *fp) {
    const char *val = vp;
    if (!val || *val == 0) val = (char *)e_astbin;
    if (strcmp(FETCH_VT(np->nvalue, const_cp), val)) {
        astbin_update(np->nvshell, FETCH_VT(np->nvalue, const_cp), val);
        nv_putv(np, val, flags, fp);
    }
}

// These two routines are for SH_OPTIONS.
static_fn void put_options(Namval_t *np, const void *val, int flags, Namfun_t *fp) {
    UNUSED(flags);
    Shell_t *shp = np->nvshell;
    Namval_t *mp;
    int c, offset = stktell(shp->stk);
    const char *cp = val, *sp;

    if (!cp) {
        nv_putv(np, val, 0, fp);
        return;
    }
    sfputr(shp->stk, ".sh.op", '_');
    while ((c = *cp) == ' ' || c == '\t') c++;
    while ((c = *cp++)) {
        if (c == '=') {
            sfputc(shp->stk, 0);
            mp = nv_open(sp = stkptr(shp->stk, offset), shp->var_base, NV_VARNAME | NV_NOADD);
            if (mp) {
                nv_putval(mp, cp, 0);
            } else {
                sfprintf(sfstderr, "%s unknown option\n", sp);
            }
            stkseek(shp->stk, offset);
            break;
        }
        sfputc(shp->stk, c);
    }
}

static_fn char *get_options(Namval_t *np, Namfun_t *fp) {
    Shell_t *shp = np->nvshell;
    int offset = stktell(shp->stk);
    char const *cp;

    sfputr(shp->stk, "astbin", '=');
    if (!(cp = nv_getval(SH_ASTBIN))) cp = "/bin";
    sfputr(shp->stk, cp, 0);
    cp = stkptr(shp->stk, offset);
    nv_putv(np, cp, 0, fp);
    stkseek(shp->stk, offset);
    return (char *)FETCH_VT(np->nvalue, const_cp);
}

#if 0
// TODO: Decide if this function serves a purpose.
static_fn int hasgetdisc(Namfun_t *fp) {
    while (fp && !fp->disc->getnum && !fp->disc->getval) fp = fp->next;
    return fp != 0;
}
#endif

static_fn void match2d(Shell_t *shp, struct match *mp) {
    Namval_t *np;
    int i;
    Namarr_t *ap;

    nv_disc(SH_MATCHNOD, &mp->hdr, DISC_OP_POP);
    np = nv_namptr(mp->nodes, 0);
    for (i = 0; i < mp->nmatch; i++) {
        np->nvname = mp->names + 3 * i;
        if (i > 9) {
            *np->nvname = '0' + i / 10;
            np->nvname[1] = '0' + (i % 10);
        } else {
            *np->nvname = '0' + i;
        }
        np->nvshell = shp;
        nv_putsub(np, NULL, 1, 0);
        nv_putsub(np, NULL, 0, 0);
        nv_putsub(SH_MATCHNOD, NULL, i, 0);
        nv_arraychild(SH_MATCHNOD, np, 0);
        np = nv_namptr(np + 1, 0);
    }
    ap = nv_arrayptr(SH_MATCHNOD);
    if (ap) ap->nelem = mp->nmatch;
}

//
// Store the most recent value for use in .sh.match. Treat .sh.match as a two
// dimensional array.
//
void sh_setmatch(Shell_t *shp, const char *v, int vsize, int nmatch, int match[], int index) {
    struct match *mp = &ip->SH_MATCH_init;
    int i, n, x, savesub = shp->subshell;
    Namarr_t *ap = nv_arrayptr(SH_MATCHNOD);
    Namval_t *np;

    if (shp->intrace) return;
    shp->subshell = 0;
    if (index < 0) {
        np = nv_namptr(mp->nodes, 0);
        if (mp->index == 0) match2d(shp, mp);
        for (i = 0; i < mp->nmatch; i++) {
            nv_disc(np, &mp->hdr, DISC_OP_LAST);
            nv_putsub(np, NULL, mp->index, 0);
            for (x = mp->index; x >= 0; x--) {
                n = i + x * mp->nmatch;
                if (mp->match[2 * n + 1] > mp->match[2 * n]) nv_putsub(np, Empty, x, ARRAY_ADD);
            }
            if ((ap = nv_arrayptr(np)) && array_elem(ap) == 0) {
                nv_putsub(SH_MATCHNOD, NULL, i, 0);
                _nv_unset(SH_MATCHNOD, NV_RDONLY);
            }
            np = nv_namptr(np + 1, 0);
        }
        shp->subshell = savesub;
        return;
    }
    mp->index = index;
    if (index == 0) {
        if (mp->nodes) {
            np = nv_namptr(mp->nodes, 0);
            for (i = 0; i < mp->nmatch; i++) {
                if (np->nvfun && np->nvfun != &mp->hdr) {
                    free(np->nvfun);
                    np->nvfun = 0;
                }
                np = nv_namptr(np + 1, 0);
            }
            free(mp->nodes);
            mp->nodes = 0;
        }
        mp->vlen = 0;
        if (ap && ap->hdr.next != &mp->hdr) free(ap);
        STORE_VT(SH_MATCHNOD->nvalue, const_cp, NULL);
        SH_MATCHNOD->nvfun = 0;
        if (!(mp->nmatch = nmatch) && !v) {
            shp->subshell = savesub;
            return;
        }
        mp->nodes = (char *)calloc(mp->nmatch * (NV_MINSZ + sizeof(void *) + 3), 1);
        mp->names = mp->nodes + mp->nmatch * (NV_MINSZ + sizeof(void *));
        nv_disc(SH_MATCHNOD, &mp->hdr, DISC_OP_LAST);
        for (i = nmatch; --i >= 0;) {
            if (match[2 * i] >= 0) nv_putsub(SH_MATCHNOD, Empty, i, ARRAY_ADD);
        }
        mp->v = v;
        mp->first = match[0];
    } else {
        if (index == 1) match2d(shp, mp);
    }
    shp->subshell = savesub;
    if (mp->nmatch) {
        for (n = mp->first + (mp->v - v), vsize = 0, i = 0; i < 2 * nmatch; i++) {
            if (match[i] >= 0 && (match[i] - n) > vsize) vsize = match[i] - n;
        }
        index *= 2 * mp->nmatch;
        i = (index + 2 * mp->nmatch) * sizeof(match[0]);
        if (i >= mp->msize) {
            if (mp->msize) {
                mp->match = (int *)realloc(mp->match, 2 * i);
            } else {
                mp->match = (int *)malloc(2 * i);
            }
            mp->msize = 2 * i;
        }
        if (vsize >= mp->vsize) {
            if (mp->vsize) {
                mp->val = (char *)realloc(mp->val, x = 2 * vsize);
            } else {
                mp->val = (char *)malloc(x = vsize + 1);
            }
            mp->vsize = x;
        }
        memcpy(mp->match + index, match, nmatch * 2 * sizeof(match[0]));
        for (i = 0; i < 2 * nmatch; i++) {
            if (match[i] >= 0) mp->match[index + i] -= n;
        }
        while (i < 2 * mp->nmatch) mp->match[index + i++] = -1;
        if (index == 0) v += mp->first;
        memcpy(mp->val + mp->vlen, v, vsize - mp->vlen);
        mp->val[mp->vlen = vsize] = 0;
        mp->lastsub[0] = mp->lastsub[1] = -1;
    }
}

static_fn char *get_match(Namval_t *np, Namfun_t *fp) {
    struct match *mp = (struct match *)fp;
    int sub, sub2 = 0, n, i = !mp->index;
    char *val;

    sub = nv_aindex(SH_MATCHNOD);
    if (sub < 0) sub = 0;
    if (np != SH_MATCHNOD) sub2 = nv_aindex(np);
    if (sub >= mp->nmatch) return 0;
    if (sub2 > 0) sub += sub2 * mp->nmatch;
    if (sub == mp->lastsub[!i]) return mp->rval[!i];
    if (sub == mp->lastsub[i]) return mp->rval[i];
    n = mp->match[2 * sub + 1] - mp->match[2 * sub];
    if (n <= 0) return mp->match[2 * sub] < 0 ? Empty : "";
    val = mp->val + mp->match[2 * sub];
    if (mp->val[mp->match[2 * sub + 1]] == 0) return val;
    mp->index = i;
    if (mp->rval[i]) {
        free(mp->rval[i]);
        mp->rval[i] = 0;
    }
    mp->rval[i] = (char *)malloc(n + 1);
    mp->lastsub[i] = sub;
    memcpy(mp->rval[i], val, n);
    mp->rval[i][n] = 0;
    return mp->rval[i];
}

static_fn char *name_match(const Namval_t *np, Namfun_t *fp) {
    UNUSED(fp);
    Shell_t *shp = sh_ptr(np);
    int sub = nv_aindex(SH_MATCHNOD);
    sfprintf(shp->strbuf, ".sh.match[%d]", sub);
    return sfstruse(shp->strbuf);
}

static const Namdisc_t SH_MATCH_disc = {
    .dsize = sizeof(struct match), .getval = get_match, .namef = name_match};

static_fn char *get_version(Namval_t *np, Namfun_t *fp) { return nv_getv(np, fp); }

// This is invoked when var `.sh.version` is used in a numeric context such as
// `$(( .sh.version ))`.
//
static Sfdouble_t nget_version(Namval_t *np, Namfun_t *fp) {
    UNUSED(np);

    // We should not need to convert version number string every time this function is called
    static Sflong_t version_number = -1;
    if (version_number != -1) return (Sfdouble_t)version_number;

    char *cp = strdup(SH_RELEASE);
    char *dash;
    char *dot;
    char *major_str, *minor_str, *patch_str;
    int major, minor, patch;
    UNUSED(fp);

    // Version string in development version could be set like 2017.0.0-devel-1509-g95d59865
    // If a '-' exists in version string, set it as end of string i.e. version string becomes
    // 2017.0.0
    dash = strchr(cp, '-');
    if (dash) *dash = 0;

    // Major version number starts at beginning of string
    major_str = cp;

    // Find the first '.' and set it to NULL, so major version string is set to 2017
    dot = strchr(cp, '.');
    if (!dot) {
        // If there is no . in version string, it means version string is either empty, invalid
        // or it's using old versioning scheme.
        free(cp);
        version_number = 0;
        return version_number;
    }

    *dot = 0;

    // Minor version string starts after first '.'
    minor_str = dot + 1;

    // Find the second '.' and set it to NULL, so minor version string is set to 0
    dot = strchr(minor_str, '.');
    assert(dot);
    *dot = 0;

    // Patch number starts after second '.'
    patch_str = dot + 1;

    major = atoi(major_str);
    minor = atoi(minor_str);
    patch = atoi(patch_str);

    assert(minor < 100);
    assert(patch < 100);
    // This will break if minor or patch number goes above 99
    version_number = major * 10000 + minor * 100 + patch;

    free(cp);
    return (Sfdouble_t)version_number;
}

static const Namdisc_t SH_VERSION_disc = {
    .dsize = 0, .getval = get_version, .getnum = nget_version};
static const Namdisc_t IFS_disc = {
    sizeof(struct ifs), put_ifs, get_ifs, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
const Namdisc_t RESTRICTED_disc = {.dsize = sizeof(Namfun_t), .putval = put_restricted};
static const Namdisc_t CDPATH_disc = {.dsize = sizeof(Namfun_t), .putval = put_cdpath};
static const Namdisc_t EDITOR_disc = {.dsize = sizeof(Namfun_t), .putval = put_ed};
static const Namdisc_t HISTFILE_disc = {.dsize = sizeof(Namfun_t), .putval = put_history};
static const Namdisc_t OPTINDEX_disc = {.dsize = sizeof(Namfun_t),
                                        .putval = put_optindex,
                                        .getnum = nget_optindex,
                                        .clonef = clone_optindex};
static const Namdisc_t SECONDS_disc = {.dsize = sizeof(struct seconds),
                                       .putval = put_seconds,
                                       .getval = get_seconds,
                                       .getnum = nget_seconds};
static const Namdisc_t RAND_disc = {
    .dsize = sizeof(struct rand), .putval = put_rand, .getval = get_rand, .getnum = nget_rand};
static const Namdisc_t LINENO_disc = {
    .dsize = sizeof(Namfun_t), .putval = put_lineno, .getval = get_lineno, .getnum = nget_lineno};
static const Namdisc_t L_ARG_disc = {
    .dsize = sizeof(Namfun_t), .putval = put_lastarg, .getval = get_lastarg};
static const Namdisc_t OPTastbin_disc = {.dsize = sizeof(Namfun_t), .putval = put_astbin};
static const Namdisc_t OPTIONS_disc = {
    .dsize = sizeof(Namfun_t), .putval = put_options, .getval = get_options};

#define MAX_MATH_ARGS 3

static_fn char *name_math(const Namval_t *np, Namfun_t *fp) {
    UNUSED(fp);
    Shell_t *shp = sh_ptr(np);
    sfprintf(shp->strbuf, ".sh.math.%s", np->nvname);
    return sfstruse(shp->strbuf);
}

static const Namdisc_t math_child_disc = {.dsize = 0, .namef = name_math};

static Namfun_t math_child_fun = {
    .disc = &math_child_disc, .nofree = 1, .subshell = 0, .dsize = sizeof(Namfun_t)};

static_fn void math_init(Shell_t *shp) {
    Namval_t *np;
    char *name;
    int i;

    shp->mathnodes = (char *)calloc(1, MAX_MATH_ARGS * (NV_MINSZ + 5));
    name = shp->mathnodes + MAX_MATH_ARGS * NV_MINSZ;
    for (i = 0; i < MAX_MATH_ARGS; i++) {
        np = nv_namptr(shp->mathnodes, i);
        np->nvfun = &math_child_fun;
        np->nvshell = shp;
        memcpy(name, "arg", 3);
        name[3] = '1' + i;
        np->nvname = name;
        name += 5;
        nv_onattr(np, NV_MINIMAL | NV_NOFREE | NV_LDOUBLE | NV_RDONLY);
    }
}

static_fn Namval_t *create_math(Namval_t *np, const void *vp, int flag, Namfun_t *fp) {
    UNUSED(flag);
    const char *name = vp;
    Shell_t *shp = sh_ptr(np);
    if (!name) return SH_MATHNOD;
    if (name[0] != 'a' || name[1] != 'r' || name[2] != 'g' || name[4] || !isdigit(name[3]) ||
        (name[3] == '0' || (name[3] - '0') > MAX_MATH_ARGS)) {
        return 0;
    }
    fp->last = (char *)&name[4];
    return nv_namptr(shp->mathnodes, name[3] - '1');
}

static_fn char *get_math(Namval_t *np, Namfun_t *fp) {
    UNUSED(fp);
    Shell_t *shp = sh_ptr(np);
    Namval_t *mp, fake;
    char *val;
    int first = 0;

    memset(&fake, 0, sizeof(fake));
    fake.nvname = ".sh.math.";

    mp = (Namval_t *)dtprev(shp->fun_tree, &fake);
    while ((mp = (Namval_t *)dtnext(shp->fun_tree, mp))) {
        if (strncmp(mp->nvname, ".sh.math.", 9)) break;
        if (first++) sfputc(shp->strbuf, ' ');
        sfputr(shp->strbuf, mp->nvname + 9, -1);
    }
    val = sfstruse(shp->strbuf);
    return val;
}

static_fn char *setdisc_any(Namval_t *np, const void *event, Namval_t *action, Namfun_t *fp) {
    UNUSED(fp);
    Shell_t *shp = sh_ptr(np);
    Namval_t *mp, fake;
    char *name;
    int off = stktell(shp->stk);
#if 0
    bool getname = false;
#endif

    memset(&fake, 0, sizeof(fake));
    fake.nvname = nv_name(np);

    // Something is wrong with the block of code I commented out. If event==NULL and action!=NULL
    // then we fall thru to the `sfputr()` call which requires event!=NULL. Coverity CID#253657.
    //
    // In practice at the time I wrote this comment this is never called with event==NULL. The only
    // path to here currently is from the `nv_setdisc()` in the `case TFUN:` block in `sh_exec()`.
    // AFAICT that will never pass a NULL event pointer.
    assert(event);
#if 0
    if (!event) {
        if (!action) {
            mp = (Namval_t *)dtprev(shp->fun_tree, &fake);
            return (char *)dtnext(shp->fun_tree, mp);
        }
        getname = true;
    }
#endif
    sfputr(shp->stk, fake.nvname, '.');
    sfputr(shp->stk, event, 0);
    name = stkptr(shp->stk, off);
    mp = nv_search(name, shp->fun_tree, action ? NV_ADD : 0);
    stkseek(shp->stk, off);
#if 0
    if (getname) return mp ? (char *)dtnext(shp->fun_tree, mp) : 0;
#endif
    if (action == np) action = mp;
    return action ? (char *)action : "";
}

static const Namdisc_t SH_MATH_disc = {
    .dsize = 0, .getval = get_math, .setdisc = setdisc_any, .createf = create_math};

#if SHOPT_COSHELL
static const Namdisc_t SH_JOBPOOL_disc = {.dsize = 0, .setdisc = setdisc_any};
#endif  // SHOPT_COSHELL

#if 0
// TODO: Decide if this function serves a purpose.
static_fn char *get_nspace(Namval_t *np, Namfun_t *fp) {
    Shell_t *shp = sh_ptr(np);
    if (shp->namespace) return nv_name(shp->namespace);
    return (char *)FETCH_VT(np->nvalue, const_cp);
}
#endif

#if 0
// TODO: Decide if this variable serves a purpose.
static const Namdisc_t NSPACE_disc = {0, NULL, get_nspace};
static Namfun_t NSPACE_init = {&NSPACE_disc, 1};
#endif

static const Namdisc_t LC_disc = {.dsize = sizeof(Namfun_t), .putval = put_lang};

//
// This function will get called whenever a configuration parameter changes.
//
static_fn int newconf(const char *name, const char *path, const char *value) {
    UNUSED(path);
    Shell_t *shp = sh_getinterp();
    char *arg;

    if (!name) {
        sh_setenviron(value);
    } else if (strcmp(name, "UNIVERSE") == 0 && strcmp(astconf(name, 0, 0), value)) {
        shp->universe = 0;
        // Set directory in new universe.
        if (*(arg = path_pwd(shp)) == '/') sh_chdir(arg);
        // Clear out old tracked alias.
        stkseek(shp->stk, 0);
        sfputr(shp->stk, nv_getval(PATHNOD), 0);
        nv_putval(PATHNOD, stkseek(shp->stk, 0), NV_RDONLY);
    }
    return 1;
}

//
// Return SH_TYPE_* bitmask for path, 0 for "not a shell".
//
int sh_type(const char *path) {
    const char *s;
    int t = 0;

    s = (const char *)strrchr(path, '/');
    if (s) {
        if (*path == '-') t |= SH_TYPE_LOGIN;
        s++;
    } else {
        s = path;
    }
    if (*s == '-') {
        s++;
        t |= SH_TYPE_LOGIN;
    }
    for (;;) {
        if (!(t & (SH_TYPE_KSH | SH_TYPE_BASH))) {
            if (*s == 'k') {
                s++;
                t |= SH_TYPE_KSH;
                continue;
            }
#if SHOPT_BASH
            if (*s == 'b' && *(s + 1) == 'a') {
                s += 2;
                t |= SH_TYPE_BASH;
                continue;
            }
#endif
        }
        if (!(t & (SH_TYPE_PROFILE | SH_TYPE_RESTRICTED)) && *s == 'r') {
            s++;
            t |= SH_TYPE_RESTRICTED;
            continue;
        }
        break;
    }
    if (*s++ == 's' && (*s == 'h' || *s == 'u')) {
        s++;
        t |= SH_TYPE_SH;
        if ((t & SH_TYPE_KSH) && *s == '9' && *(s + 1) == '3') s += 2;
#if __CYGWIN__
        if (*s == '.' && *(s + 1) == 'e' && *(s + 2) == 'x' && *(s + 3) == 'e') s += 4;
#endif
        if (!isalnum(*s)) return t;
    }
    return t & ~(SH_TYPE_BASH | SH_TYPE_KSH | SH_TYPE_PROFILE | SH_TYPE_RESTRICTED);
}

#if 0
// TODO: Decide if this function serves a purpose.
static_fn void put_mode(Namval_t *np, const char *val, int flag, Namfun_t *nfp) {
    if (val) {
        mode_t mode;
        char *last = 0;
        if (flag & NV_INTEGER) {
            if (flag & NV_LONG) {
                mode = *(Sfdouble_t *)val;
            } else {
                mode = *(double *)val;
            }
        } else {
            mode = strperm(val, &last, 0);
            if (*last) {
                errormsg(SH_DICT, ERROR_exit(1), "%s: invalid mode string", val);
                __builtin_unreachable();
            }
        }
        nv_putv(np, (char *)&mode, NV_INTEGER, nfp);
    } else {
        nv_putv(np, val, flag, nfp);
    }
}
#endif

#if 0
// TODO: Decide if this variable serves a purpose.
static const Namdisc_t modedisc = {
    0, put_mode, get_mode,
};
#endif

// The need for the function below is unfortunate. It is due to the API split on 2012-07-20 that
// required many functions to be passed a pointer to the shell interpreter context. And some
// putative functions outside the ksh code cannnot do so. For example, the libast `errorv()`
// function has no way to pass a shell interpreter pointer to the function assigned to
// `error_info.exit`.
//
// See https://github.com/att/ast/issues/983
//
static_fn void no_shell_context_sh_exit(int exit_val) { sh_exit(sh_getinterp(), exit_val); }

//
// Initialize the shell.
//
Shell_t *sh_init(int argc, char *argv[], Shinit_f userinit) {
    static int beenhere;
    Shell_t *shp;
    size_t n;
    int type;
    static char *login_files[2];

    n = strlen(e_version);
    if (e_version[n - 1] == '$' && e_version[n - 2] == ' ') e_version[n - 2] = 0;
    if (!beenhere) {
        beenhere = 1;
        shp = sh_getinterp();
        shgd = calloc(1, sizeof(struct shared));
        shgd->pid = getpid();
        shgd->ppid = getppid();
        shgd->userid = getuid();
        shgd->euserid = geteuid();
        shgd->groupid = getgid();
        shgd->egroupid = getegid();
        shgd->lim.clk_tck = getconf("CLK_TCK");
        shgd->lim.arg_max = getconf("ARG_MAX");
        shgd->lim.child_max = getconf("CHILD_MAX");
        shgd->lim.ngroups_max = getconf("NGROUPS_MAX");
        shgd->lim.posix_version = getconf("VERSION");
        shgd->lim.posix_jobcontrol = getconf("JOB_CONTROL");
        if (shgd->lim.arg_max <= 0) shgd->lim.arg_max = ARG_MAX;
        if (shgd->lim.child_max <= 0) shgd->lim.child_max = CHILD_MAX;
        if (shgd->lim.clk_tck <= 0) shgd->lim.clk_tck = CLK_TCK;
        shgd->ed_context = (void *)ed_open(shp);
        error_info.exit = no_shell_context_sh_exit;
        error_info.id = path_basename(argv[0]);
    } else {
        shp = calloc(1, sizeof(Shell_t));
    }
    umask(shp->mask = umask(0));
    shp->gd = shgd;
    shp->mac_context = sh_macopen(shp);
    shp->arg_context = sh_argopen(shp);
    shp->lex_context = (void *)sh_lexopen(0, shp, 1);
    shp->strbuf = sfstropen();
    shp->stk = stkstd;
    sfsetbuf(shp->strbuf, NULL, 64);
    sh_onstate(shp, SH_INIT);
    error_info.catalog = e_dict;
    shp->cpipe[0] = -1;
    shp->coutpipe = -1;
    for (n = 0; n < 10; n++) {
        // Don't use lower bits when rand() generates large numbers.
        if (rand() > RANDMASK) {
            rand_shift = 3;
            break;
        }
    }
    sh_ioinit(shp);
    shp->pwdfd = sh_diropenat(shp, AT_FDCWD, e_dot);
#if O_SEARCH
    // This should _never_ happen, guaranteed by design and goat sacrifice.
    // If shell starts in a directory that it does not have access to, this will cause error.
    // if (shp->pwdfd < 0) errormsg(SH_DICT, ERROR_system(1), "Can't obtain directory fd.");
#endif

    // Initialize signal handling.
    sh_siginit(shp);
    stakinstall(NULL, nospace);
    // Set up memory for name-value pairs.
    shp->init_context = nv_init(shp);
    // Read the environment.
    if (argc > 0) {
        shgd->shtype = type = sh_type(*argv);
        if (type & SH_TYPE_LOGIN) shp->login_sh = 2;
    }
    env_init(shp);
    if (!FETCH_VT(ENVNOD->nvalue, const_cp)) {
        sfprintf(shp->strbuf, "%s/.kshrc", nv_getval(HOME));
        nv_putval(ENVNOD, sfstruse(shp->strbuf), NV_RDONLY);
    }
    *FETCH_VT(SHLVL->nvalue, ip) += 1;
    nv_offattr(SHLVL, NV_IMPORT);
#if USE_SPAWN
    {
        // Try to find the pathname for this interpreter.
        // Try using environment variable _ or argv[0].
        char *cp = nv_getval(L_ARGNOD);
        char buff[PATH_MAX + 1];
        shp->gd->shpath = 0;
        if ((n = pathprog(NULL, buff, sizeof(buff))) > 0 && n <= sizeof(buff)) {
            shp->gd->shpath = strdup(buff);
        } else if ((cp && (sh_type(cp) & SH_TYPE_SH)) || (argc > 0 && strchr(cp = *argv, '/'))) {
            if (*cp == '/') {
                shp->gd->shpath = strdup(cp);
            } else if ((cp = nv_getval(PWDNOD))) {
                int offset = staktell();
                stakputs(cp);
                stakputc('/');
                stakputs(argv[0]);
                n = staktell() - offset;
                pathcanon(stakptr(offset), n, PATH_DOTDOT);
                shp->gd->shpath = strdup(stakptr(offset));
                stakseek(offset);
            }
        }
    }
#endif
    nv_putval(IFSNOD, (char *)e_sptbnl, NV_RDONLY);
    astconfdisc(newconf);
    shp->st.tmout = READ_TIMEOUT;
    // Initialize jobs table.
    job_clear(shp);
    sh_onoption(shp, SH_MULTILINE);
    if (argc > 0) {
        int dolv_index = -1;
        // Check for restricted shell.
        if (type & SH_TYPE_RESTRICTED) sh_onoption(shp, SH_RESTRICTED);
#if SHOPT_BASH
        // Check for invocation as bash.
        if (type & SH_TYPE_BASH) {
            shp->userinit = userinit = bash_init;
            sh_onoption(shp, SH_BASH);
            sh_onstate(shp, SH_PREINIT);
            (*userinit)(shp, 0);
            sh_offstate(shp, SH_PREINIT);
        }
#endif
        // Look for options. shp->st.dolc is $#.
        if ((shp->st.dolc = sh_argopts(-argc, argv, shp)) < 0) {
            shp->exitval = 2;
            sh_done(shp, 0);
        }
        opt_info.disc = 0;
        dolv_index = (argc - 1) - shp->st.dolc;
        shp->st.dolv = argv + dolv_index;
        shp->st.repl_index = dolv_index;
        shp->st.repl_arg = argv[dolv_index];
        shp->st.dolv[0] = argv[0];
        if (shp->st.dolc < 1) sh_onoption(shp, SH_SFLAG);
        if (!sh_isoption(shp, SH_SFLAG)) {
            shp->st.dolc--;
            shp->st.dolv++;
#if __CYGWIN__
            {
                char *name;
                name = shp->st.dolv[0];
                if (name[1] == ':' && (name[2] == '/' || name[2] == '\\')) {
#if _lib_pathposix
                    char *p;

                    if ((n = pathposix(name, NULL, 0)) > 0 && (p = (char *)malloc(++n))) {
                        pathposix(name, p, n);
                        name = p;
                    } else
#endif
                    {
                        name[1] = name[0];
                        name[0] = name[2] = '/';
                    }
                }
            }
#endif  // __CYGWIN__
        }
        if (beenhere == 1) {
            struct lconv *lc;
            shp->decomma = (lc = localeconv()) && lc->decimal_point && *lc->decimal_point == ',';
            beenhere = 2;
        }
    }
    // set[ug]id scripts require the -p flag.
    if (shp->gd->userid != shp->gd->euserid || shp->gd->groupid != shp->gd->egroupid) {
        sh_onoption(shp, SH_PRIVILEGED);
        // Careful of #! setuid scripts with name beginning with -.
        if (shp->login_sh && argv[1] && strcmp(argv[0], argv[1]) == 0) {
            errormsg(SH_DICT, ERROR_exit(1), e_prohibited);
            __builtin_unreachable();
        }
    } else {
        sh_offoption(shp, SH_PRIVILEGED);
    }
    // shname for $0 in profiles and . scripts.
    if (sh_isdevfd(argv[1])) {
        shp->shname = strdup(argv[0]);
    } else {
        shp->shname = strdup(shp->st.dolv[0]);
    }
    //
    // Return here for shell script execution but not for parenthesis subshells.
    //
    error_info.id = strdup(shp->st.dolv[0]);  // error_info.id is $0
    shp->jmpbuffer = (void *)&shp->checkbase;
#if USE_SPAWN
    shp->vex = spawnvex_open(0);
    shp->vexp = spawnvex_open(0);
#endif
    sh_pushcontext(shp, &shp->checkbase, SH_JMPSCRIPT);
    shp->st.self = &shp->global;
    shp->topscope = (Shscope_t *)shp->st.self;
    sh_offstate(shp, SH_INIT);
    login_files[0] = (char *)e_profile;
    shp->gd->login_files = login_files;
    shp->bltindata.version = SH_VERSION;
    shp->bltindata.shp = shp;
    shp->bltindata.shrun = sh_run;
    shp->bltindata.shexit = sh_exit;

#if 0
#define NV_MKINTTYPE(x, y, z) nv_mkinttype(#x, sizeof(x), (x)-1 < 0, (y), (Namdisc_t *)z);
	NV_MKINTTYPE(pid_t,"process id",0);
	NV_MKINTTYPE(gid_t,"group id",0);
	NV_MKINTTYPE(uid_t,"user id",0);
	NV_MKINTTYPE(size_t,(const char*)0,0);
	NV_MKINTTYPE(ssize_t,(const char*)0,0);
	NV_MKINTTYPE(off_t,"offset in bytes",0);
	NV_MKINTTYPE(ino_t,"\ai-\anode number",0);
	NV_MKINTTYPE(mode_t,(const char*)0,&modedisc);
	NV_MKINTTYPE(dev_t,"device id",0);
	NV_MKINTTYPE(nlink_t,"hard link count",0);
	NV_MKINTTYPE(blkcnt_t,"block count",0);
	NV_MKINTTYPE(time_t,"seconds since the epoch",0);
	nv_mkstat();

#endif
    shp->userinit = userinit;
    if (userinit) (*userinit)(shp, 0);
    shp->exittrap = 0;
    shp->errtrap = 0;
    shp->end_fn = 0;
    return shp;
}

Shell_t *sh_getinterp(void) { return &sh; }

//
// Reinitialize before executing a script.
//
int sh_reinit(Shell_t *shp, char *argv[]) {
    Shopt_t opt;
    Namval_t *np, *npnext;
    Dt_t *dp;
    struct adata {
        Shell_t *sh;
        void *extra[2];
    } data;
    for (np = dtfirst(shp->fun_tree); np; np = npnext) {
        if ((dp = shp->fun_tree)->walk) dp = dp->walk;
        npnext = (Namval_t *)dtnext(shp->fun_tree, np);
        if (np >= shgd->bltin_cmds && np < &shgd->bltin_cmds[nbltins]) continue;
        if (is_abuiltin(np) && nv_isattr(np, NV_EXPORT)) continue;
        if (*np->nvname == '/') continue;
        nv_delete(np, dp, NV_NOFREE);
    }
    dtclose(shp->alias_tree);
    shp->alias_tree = inittree(shp, shtab_aliases);
    shp->last_root = shp->var_tree;
    shp->inuse_bits = 0;
    if (shp->userinit) (*shp->userinit)(shp, 1);
    if (shp->heredocs) {
        sfclose(shp->heredocs);
        shp->heredocs = 0;
    }
    // Remove locals.
    sh_onstate(shp, SH_INIT);
    memset(&data, 0, sizeof(data));
    data.sh = shp;
    nv_scan(shp->var_tree, sh_envnolocal, (void *)&data, NV_EXPORT, 0);
    nv_scan(shp->var_tree, sh_envnolocal, (void *)&data, NV_ARRAY, NV_ARRAY);
    sh_offstate(shp, SH_INIT);
    memset(shp->st.trapcom, 0, (shp->st.trapmax + 1) * sizeof(char *));
    memset((void *)&opt, 0, sizeof(opt));
    if (shp->namespace) {
        dp = nv_dict(shp->namespace);
        if (dp == shp->var_tree) shp->var_tree = dtview(dp, 0);
        _nv_unset(shp->namespace, NV_RDONLY);
        shp->namespace = 0;
    }
    if (sh_isoption(shp, SH_TRACKALL)) on_option(&opt, SH_TRACKALL);
    if (sh_isoption(shp, SH_EMACS)) on_option(&opt, SH_EMACS);
    if (sh_isoption(shp, SH_GMACS)) on_option(&opt, SH_GMACS);
    if (sh_isoption(shp, SH_VI)) on_option(&opt, SH_VI);
    shp->options = opt;
    // Set up new args.
    if (argv) shp->arglist = sh_argcreate(argv);
    if (shp->arglist) sh_argreset(shp, shp->arglist, NULL);
    shp->envlist = 0;
    shp->curenv = 0;
    shp->shname = error_info.id = strdup(shp->st.dolv[0]);
    sh_offstate(shp, SH_FORKED);
    shp->fn_depth = shp->dot_depth = 0;
    sh_sigreset(shp, 0);
    if (!FETCH_VT(SHLVL->nvalue, ip)) {
        shlvl = 0;
        STORE_VT(SHLVL->nvalue, ip, &shlvl);
        nv_onattr(SHLVL, NV_INTEGER | NV_EXPORT | NV_NOFREE);
    }
    *FETCH_VT(SHLVL->nvalue, ip) += 1;
    nv_offattr(SHLVL, NV_IMPORT);
    shp->st.filename = strdup(shp->lastarg);
    nv_delete(NULL, NULL, 0);
    job.exitval = 0;
    shp->inpipe = shp->outpipe = 0;
    job_clear(shp);
    job.in_critical = 0;
    shp->exittrap = 0;
    shp->errtrap = 0;
    shp->end_fn = 0;
    return 1;
}

//
// Set when creating a local variable of this name.
//
Namfun_t *nv_cover(Namval_t *np) {
    if (np == IFSNOD || np == PATHNOD || np == SHELLNOD || np == FPATHNOD || np == CDPNOD ||
        np == SECONDS || np == ENVNOD || np == LINENO) {
        return np->nvfun;
    }
    if (np == LCALLNOD || np == LCTYPENOD || np == LCMSGNOD || np == LCCOLLNOD || np == LCNUMNOD ||
        np == LANGNOD) {
        return np->nvfun;
    }
    return 0;
}

static const char *shdiscnames[] = {"tilde", 0};

#if 0
// TODO: Decide if this function serves a purpose.
static_fn Namval_t *create_sig(Namval_t *np, const char *name, int flag, Namfun_t *fp) { return 0; }
#endif

typedef struct Svars {
    Namfun_t hdr;
    Shell_t *sh;
    Namval_t *parent;
    char *nodes;
    void *data;
    size_t dsize;
    int numnodes;
    int current;
} Svars_t;

static_fn Namval_t *next_svar(Namval_t *np, Dt_t *root, Namfun_t *fp) {
    UNUSED(np);
    struct Svars *sp = (struct Svars *)fp;
    if (!root) {
        sp->current = 0;
    } else if (++sp->current >= sp->numnodes) {
        return NULL;
    }
    return nv_namptr(sp->nodes, sp->current);
}

// This is used to assign values to the attributes of the .sh.sig compound var.
// Do not use it for any other purpose.
static_fn Namval_t *create_svar(Namval_t *np, const void *vp, int flag, Namfun_t *fp) {
    UNUSED(flag);
    const char *name = vp;
    Svars_t *sp = (Svars_t *)fp;
    Shell_t *shp = sp->sh;

    assert(name);
    for (int i = 0; i < sp->numnodes; i++) {
        Namval_t *nq = nv_namptr(sp->nodes, i);
        if (strcmp(name, nq->nvname) == 0) {
            sp->hdr.last = (char *)name + strlen(name);
            shp->last_table = sp->parent;
            return nq;
        }
    }

    errormsg(SH_DICT, ERROR_exit(1), e_notelem, strlen(name), name, nv_name(np));
    __builtin_unreachable();
}

static_fn Namfun_t *clone_svar(Namval_t *np, Namval_t *mp, int flags, Namfun_t *fp) {
    struct Svars *sp = (struct Svars *)fp;
    struct Svars *dp;
    int i;
    Sfdouble_t d;

    dp = malloc(fp->dsize + sp->dsize + sizeof(void *));
    memcpy(dp, sp, fp->dsize);
    dp->nodes = (char *)(dp + 1);
    dp->data = (void *)((char *)dp + fp->dsize + sizeof(void *));
    memcpy(dp->data, sp->data, sp->dsize);
    dp->hdr.nofree = (flags & NV_RDONLY ? 1 : 0);
    for (i = dp->numnodes; --i >= 0;) {
        np = nv_namptr(sp->nodes, i);
        mp = nv_namptr(dp->nodes, i);
        nv_offattr(mp, NV_RDONLY);
        const char *cp = FETCH_VT(np->nvalue, cp);
        if (cp >= (char *)sp->data && cp < (char *)sp->data + sp->dsize) {
            STORE_VT(mp->nvalue, cp, (char *)(dp->data) + (cp - (char *)sp->data));
        } else if (nv_isattr(np, NV_INTEGER)) {
            d = nv_getnum(np);
            nv_putval(mp, (char *)&d, NV_DOUBLE);
        } else if ((cp = nv_getval(np))) {
            nv_putval(mp, cp, 0);
        }
    }
    return &dp->hdr;
}

static const Namdisc_t svar_disc = {
    .dsize = 0, .createf = create_svar, .clonef = clone_svar, .nextf = next_svar};

static_fn char *name_svar(const Namval_t *np, Namfun_t *fp) {
    Shell_t *shp = sh_ptr(np);
    Namval_t *mp = *(Namval_t **)(fp + 1);
    sfprintf(shp->strbuf, ".sh.%s.%s", mp->nvname, np->nvname);
    return sfstruse(shp->strbuf);
}

static const Namdisc_t svar_child_disc = {.dsize = 0, .namef = name_svar};

#if 0
// TODO: Decide if this variable serves a purpose.
static Namfun_t svar_child_fun = {&svar_child_disc, 1, 0, sizeof(Namfun_t)};
#endif

static_fn int svar_init(Shell_t *shp, Namval_t *pp, const Shtable_t *tab, size_t extra) {
    int i;
    struct Svars *sp;
    Namval_t *np, **mp;
    Namfun_t *fp;

    int nnodes = 0;
    while (*tab[nnodes].sh_name) nnodes++;
    size_t xtra = nnodes * (sizeof(int) + NV_MINSZ) + sizeof(Namfun_t) + sizeof(void *) + extra;
    sp = calloc(1, sizeof(struct Svars) + xtra);
    sp->nodes = (char *)(sp + 1);
    fp = (Namfun_t *)((char *)sp->nodes + nnodes * NV_MINSZ);
    memset(fp, 0, sizeof(Namfun_t));
    fp->dsize = sizeof(Namfun_t);
    fp->disc = &svar_child_disc;
    fp->nofree = 1;
    mp = (Namval_t **)(fp + 1);
    *mp = pp;
    sp->numnodes = nnodes;
    sp->parent = pp;
    sp->sh = shp;
    for (i = 0; i < nnodes; i++) {
        np = nv_namptr(sp->nodes, i);
        np->nvfun = fp;
        np->nvname = (char *)tab[i].sh_name;
        np->nvshell = shp;
        nv_onattr(np, tab[i].sh_number);
        if (tab[i].sh_number & NV_INTEGER) nv_setsize(np, 10);
    }
    sp->hdr.dsize = sizeof(struct Svars) + nnodes * (sizeof(int) + NV_MINSZ);
    sp->hdr.disc = &svar_disc;
    nv_stack(pp, &sp->hdr);
    sp->hdr.nofree = 1;
    nv_setvtree(pp);
    return nnodes;
}

static_fn void stat_init(Shell_t *shp) {
    Namval_t *np;
    struct Svars *sp;
    int i, n;
    n = svar_init(shp, SH_STATS, shtab_stats, 0);
    shgd->stats = (int *)calloc(sizeof(int), n + 1);
    sp = (struct Svars *)SH_STATS->nvfun->next;
    sp->data = (void *)shgd->stats;
    sp->dsize = (n + 1) * sizeof(shgd->stats[0]);
    for (i = 0; i < n; i++) {
        np = nv_namptr(sp->nodes, i);
        STORE_VT(np->nvalue, ip, &shgd->stats[i]);
    }
}

#define SIGNAME_MAX 32
static_fn void siginfo_init(Shell_t *shp) {
    struct Svars *sp;
    svar_init(shp, SH_SIG, shtab_siginfo, sizeof(siginfo_t) + sizeof(char *) + SIGNAME_MAX);
    sp = (struct Svars *)SH_SIG->nvfun->next;
    sp->dsize = sizeof(siginfo_t) + SIGNAME_MAX;
}

static_fn const char *siginfocode2str(int sig, int code) {
    const struct shtable4 *sc;
    for (sc = shtab_siginfo_codes; sc->str; sc++) {
        if (((sc->sig == sig) || (sc->sig == 0)) && (sc->code == code)) return sc->str;
    }
    return NULL;
}

void sh_setsiginfo(siginfo_t *sip) {
    Namval_t *np;
    Namfun_t *fp = SH_SIG->nvfun;
    struct Svars *sp;
    const char *sistr;
    char *signame;

    while (fp->disc->createf != create_svar) fp = fp->next;
    if (!fp) return;  // this should probably be an abort() or errormsg(SH_DICT, ERROR_exit(1), ...)

    sp = (struct Svars *)fp;
    sp->data = (void *)((char *)sp + fp->dsize + sizeof(void *));
    signame = (char *)sp->data + sizeof(siginfo_t);
    memcpy(sp->data, sip, sizeof(siginfo_t));
    sip = (siginfo_t *)sp->data;
    np = create_svar(SH_SIG, "signo", 0, fp);
    STORE_VT(np->nvalue, ip, &sip->si_signo);
    np = create_svar(SH_SIG, "name", 0, fp);
    sh_siglist(sp->sh, sp->sh->strbuf, sip->si_signo + 1);
    sfseek(sp->sh->strbuf, (Sfoff_t)-1, SEEK_END);
    sfputc(sp->sh->strbuf, 0);
    char *p = sfstruse(sp->sh->strbuf);
    assert(p);
    // If the source signal name is longer than SIGNAME_MAX something is horribly wrong.
    if (strlcpy(signame, p, SIGNAME_MAX) >= SIGNAME_MAX) abort();  // this can't happen
    STORE_VT(np->nvalue, const_cp, signame);
    np = create_svar(SH_SIG, "pid", 0, fp);
    STORE_VT(np->nvalue, pidp, &sip->si_pid);
    np = create_svar(SH_SIG, "uid", 0, fp);
    STORE_VT(np->nvalue, uidp, &sip->si_uid);
    np = create_svar(SH_SIG, "code", 0, fp);
    sistr = siginfocode2str(sip->si_signo, sip->si_code);
    if (sistr) {
        STORE_VT(np->nvalue, const_cp, sistr);
        nv_offattr(np, NV_INTEGER);  // NV_NOFREE already set in shtab_siginfo[]
    } else {
        nv_onattr(np, NV_INTEGER);
        STORE_VT(np->nvalue, ip, &sip->si_code);
    }
    np = create_svar(SH_SIG, "status", 0, fp);
    STORE_VT(np->nvalue, ip, &sip->si_status);
    np = create_svar(SH_SIG, "addr", 0, fp);
    nv_setsize(np, 16);
    STORE_VT(np->nvalue, vp, &sip->si_addr);
    np = create_svar(SH_SIG, "value", 0, fp);
    nv_setsize(np, 10);
    STORE_VT(np->nvalue, ip, &(sip->si_value.sival_int));
}

//
// Initialize the shell name and alias table.
//
static_fn Init_t *nv_init(Shell_t *shp) {
    double d = 0;

    ip = calloc(1, sizeof(Init_t));
    if (!ip) return NULL;
    shp->nvfun.last = (char *)shp;
    shp->nvfun.nofree = 1;
    shp->var_base = shp->var_tree = inittree(shp, shtab_variables);
    STORE_VT(SHLVL->nvalue, ip, &shlvl);
    ip->IFS_init.hdr.disc = &IFS_disc;
    ip->PATH_init.disc = &RESTRICTED_disc;
    ip->PATH_init.nofree = 1;
    ip->FPATH_init.disc = &RESTRICTED_disc;
    ip->FPATH_init.nofree = 1;
    ip->CDPATH_init.disc = &CDPATH_disc;
    ip->CDPATH_init.nofree = 1;
    ip->SHELL_init.disc = &RESTRICTED_disc;
    ip->SHELL_init.nofree = 1;
    ip->ENV_init.disc = &RESTRICTED_disc;
    ip->ENV_init.nofree = 1;
    ip->VISUAL_init.disc = &EDITOR_disc;
    ip->VISUAL_init.nofree = 1;
    ip->EDITOR_init.disc = &EDITOR_disc;
    ip->EDITOR_init.nofree = 1;
    ip->HISTFILE_init.disc = &HISTFILE_disc;
    ip->HISTFILE_init.nofree = 1;
    ip->HISTSIZE_init.disc = &HISTFILE_disc;
    ip->HISTSIZE_init.nofree = 1;
    ip->OPTINDEX_init.disc = &OPTINDEX_disc;
    ip->OPTINDEX_init.nofree = 1;
    ip->SECONDS_init.hdr.disc = &SECONDS_disc;
    ip->SECONDS_init.hdr.nofree = 1;
    ip->RAND_init.hdr.disc = &RAND_disc;
    ip->RAND_init.hdr.nofree = 1;
    ip->RAND_init.sh = shp;
    ip->SH_MATCH_init.hdr.disc = &SH_MATCH_disc;
    ip->SH_MATCH_init.hdr.nofree = 1;
    ip->SH_MATH_init.disc = &SH_MATH_disc;
    ip->SH_MATH_init.nofree = 1;
#if SHOPT_COSHELL
    ip->SH_JOBPOOL_init.disc = &SH_JOBPOOL_disc;
    ip->SH_JOBPOOL_init.nofree = 1;
    nv_stack(SH_JOBPOOL, &ip->SH_JOBPOOL_init);
#endif  // SHOPT_COSHELL
    ip->SH_VERSION_init.disc = &SH_VERSION_disc;
    ip->SH_VERSION_init.nofree = 1;
    ip->LINENO_init.disc = &LINENO_disc;
    ip->LINENO_init.nofree = 1;
    ip->L_ARG_init.disc = &L_ARG_disc;
    ip->L_ARG_init.nofree = 1;
    ip->LC_TIME_init.disc = &LC_disc;
    ip->LC_TIME_init.nofree = 1;
    ip->LC_TYPE_init.disc = &LC_disc;
    ip->LC_TYPE_init.nofree = 1;
    ip->LC_NUM_init.disc = &LC_disc;
    ip->LC_NUM_init.nofree = 1;
    ip->LC_COLL_init.disc = &LC_disc;
    ip->LC_COLL_init.nofree = 1;
    ip->LC_MSG_init.disc = &LC_disc;
    ip->LC_MSG_init.nofree = 1;
    ip->LC_ALL_init.disc = &LC_disc;
    ip->LC_ALL_init.nofree = 1;
    ip->LANG_init.disc = &LC_disc;
    ip->LANG_init.nofree = 1;
    ip->OPTIONS_init.disc = &OPTIONS_disc;
    ip->OPTastbin_init.disc = &OPTastbin_disc;
    nv_stack(IFSNOD, &ip->IFS_init.hdr);
    ip->IFS_init.hdr.nofree = 1;
    nv_stack(PATHNOD, &ip->PATH_init);
    nv_stack(FPATHNOD, &ip->FPATH_init);
    nv_stack(CDPNOD, &ip->CDPATH_init);
    nv_stack(SHELLNOD, &ip->SHELL_init);
    nv_stack(ENVNOD, &ip->ENV_init);
    nv_stack(VISINOD, &ip->VISUAL_init);
    nv_stack(EDITNOD, &ip->EDITOR_init);
    nv_stack(HISTFILE, &ip->HISTFILE_init);
    nv_stack(HISTSIZE, &ip->HISTSIZE_init);
    nv_stack(OPTINDNOD, &ip->OPTINDEX_init);
    nv_stack(SECONDS, &ip->SECONDS_init.hdr);
    nv_stack(L_ARGNOD, &ip->L_ARG_init);
    nv_putval(SECONDS, (char *)&d, NV_DOUBLE);
    nv_stack(RANDNOD, &ip->RAND_init.hdr);
    d = (shp->gd->pid & RANDMASK);
    nv_putval(RANDNOD, (char *)&d, NV_DOUBLE);
    nv_stack(LINENO, &ip->LINENO_init);
    SH_MATCHNOD->nvfun = &ip->SH_MATCH_init.hdr;
    nv_putsub(SH_MATCHNOD, NULL, 10, 0);
    nv_stack(SH_MATHNOD, &ip->SH_MATH_init);
    nv_stack(SH_VERSIONNOD, &ip->SH_VERSION_init);
    nv_stack(OPTIONS, &ip->OPTIONS_init);
    nv_stack(SH_ASTBIN, &ip->OPTastbin_init);
    nv_stack(LCTIMENOD, &ip->LC_TIME_init);
    nv_stack(LCTYPENOD, &ip->LC_TYPE_init);
    nv_stack(LCALLNOD, &ip->LC_ALL_init);
    nv_stack(LCMSGNOD, &ip->LC_MSG_init);
    nv_stack(LCCOLLNOD, &ip->LC_COLL_init);
    nv_stack(LCNUMNOD, &ip->LC_NUM_init);
    nv_stack(LANGNOD, &ip->LANG_init);
    STORE_VT((PPIDNOD)->nvalue, pidp, &shp->gd->ppid);
    STORE_VT((TMOUTNOD)->nvalue, i32p, &shp->st.tmout);
    STORE_VT((MCHKNOD)->nvalue, i32p, &sh_mailchk);
    STORE_VT((OPTINDNOD)->nvalue, i32p, &shp->st.optindex);
    // Set up the seconds clock.
    shp->alias_tree = inittree(shp, shtab_aliases);
    dtuserdata(shp->alias_tree, shp, 1);
    shp->track_tree = dtopen(&_Nvdisc, Dtset);
    dtuserdata(shp->track_tree, shp, 1);
    shp->bltin_tree = inittree(shp, (const struct shtable2 *)shtab_builtins);
    dtuserdata(shp->bltin_tree, shp, 1);
    shp->fun_tree = dtopen(&_Nvdisc, Dtoset);
    dtuserdata(shp->fun_tree, shp, 1);
    dtview(shp->fun_tree, shp->bltin_tree);
    nv_mount(DOTSHNOD, "type", shp->typedict = dtopen(&_Nvdisc, Dtoset));
    nv_adddisc(DOTSHNOD, shdiscnames, NULL);
    STORE_VT(DOTSHNOD->nvalue, const_cp, Empty);
    nv_onattr(DOTSHNOD, NV_RDONLY);
    STORE_VT(SH_LINENO->nvalue, i64p, &shp->st.lineno);
    STORE_VT(SH_PWDFD->nvalue, ip, &shp->pwdfd);
    struct Namref *nrp = calloc(1, sizeof(struct Namref));
    STORE_VT(VERSIONNOD->nvalue, nrp, nrp);
    nrp->np = SH_VERSIONNOD;
    nrp->root = nv_dict(DOTSHNOD);
    nrp->table = DOTSHNOD;
    nv_onattr(VERSIONNOD, NV_REF);
    math_init(shp);
    if (!shgd->stats) stat_init(shp);
    siginfo_init(shp);
    return ip;
}

//
// Initialize name-value pairs.
//
static_fn Dt_t *inittree(Shell_t *shp, const struct shtable2 *name_vals) {
    Namval_t *np;
    const struct shtable2 *tp;
    unsigned n = 0;
    Dt_t *treep;
    Dt_t *base_treep;
    Dt_t *dict = NULL;

    for (tp = name_vals; *tp->sh_name; tp++) n++;
    np = (Namval_t *)calloc(n, sizeof(Namval_t));
    if (!shgd->bltin_nodes) {
        shgd->bltin_nodes = np;
        shgd->bltin_nnodes = n;
    } else if (name_vals == (const struct shtable2 *)shtab_builtins) {
        shgd->bltin_cmds = np;
        nbltins = n;
    }
    base_treep = treep = dtopen(&_Nvdisc, Dtoset);
    dtuserdata(treep, shp, 1);
    for (tp = name_vals; *tp->sh_name; tp++, np++) {
        if ((np->nvname = strrchr(tp->sh_name, '.')) && np->nvname != ((char *)tp->sh_name)) {
            np->nvname++;
        } else {
            np->nvname = (char *)tp->sh_name;
            treep = base_treep;
        }
        np->nvenv = 0;
        np->nvshell = (void *)shp;
        if (name_vals == (const struct shtable2 *)shtab_builtins) {
            STORE_VT(np->nvalue, shbltinp, ((struct shtable3 *)tp)->sh_value);
        } else {
            if (name_vals == shtab_variables) np->nvfun = &shp->nvfun;
            STORE_VT(np->nvalue, const_cp, tp->sh_value);
        }
        nv_setattr(np, tp->sh_number);
        nv_setsize(np, nv_isattr(np, NV_INTEGER) ? 10 : 0);
        // The sole reason for using nv_isattr(np, NV_TABLE) here appears to be the special `.sh`
        // compound var. At this point the nv_isattr test is true while nv_istable(np) is false.
        // Ffter the nv_mount() the reverse is true.
        if (nv_isattr(np, NV_TABLE)) {
            dict = dtopen(&_Nvdisc, Dtoset);
            nv_mount(np, NULL, dict);
            dtinsert(treep, np);
            treep = dict;
        } else {
            dtinsert(treep, np);
        }
    }
    // The loop above has to run at least one interation otherwise this leaks memory pointed to by
    // `np`. Hopefully this assert silences Coverity Scan CID#253829.
    assert(tp != name_vals);
    return treep;
}

//
// Read in the process environment and set up name-value pairs
// Skip over items that are not name-value pairs
//
static_fn void env_init(Shell_t *shp) {
    char *cp;
    Namval_t *np;
    char **ep = environ;
    char *next = NULL;

    if (ep) {
        while (*ep) {
            cp = *ep++;
            // The magic "A__z" env var is an invention of ksh88 and still used by ksh93.
            // See var `e_envmarker`.
            if (*cp == 'A' && cp[1] == '_' && cp[2] == '_' && cp[3] == 'z' && cp[4] == '=') {
                next = cp + 4;
            } else {
                np = nv_open(cp, shp->var_tree, (NV_EXPORT | NV_IDENT | NV_ASSIGN | NV_NOFAIL));
                if (np) {
                    nv_onattr(np, NV_IMPORT);
                    np->nvenv = cp;
                    nv_close(np);
                } else {
                    // Swap with front
                    ep[-1] = environ[shp->nenv];
                    environ[shp->nenv++] = cp;
                }
            }
        }

        // This loop deals with the value of the magic "A__z" env var that is used to pass
        // properties of exported variables to subshells.
        while (next) {
            cp = next;
            next = strchr(++cp, '=');
            if (next) *next = 0;
            np = nv_search(cp + 2, shp->var_tree, NV_ADD);
            if (np == SHLVL) continue;
            if (!nv_isattr(np, NV_IMPORT | NV_EXPORT)) continue;

            int flag = *(unsigned char *)cp - ' ';
            int size = *(unsigned char *)(cp + 1) - ' ';
            if ((flag & NV_INTEGER) && size == 0) {
                // Check for floating
                char *ep, *val = nv_getval(np);
                strtol(val, &ep, 10);
                if (*ep == '.' || *ep == 'e' || *ep == 'E') {
                    char *lp;
                    flag |= NV_DOUBLE;
                    if (*ep == '.') {
                        strtol(ep + 1, &lp, 10);
                        if (*lp) ep = lp;
                    }
                    if (*ep && *ep != '.') {
                        flag |= NV_EXPNOTE;
                        size = ep - val;
                    } else {
                        size = strlen(ep);
                    }
                    size--;
                }
            }
            nv_newattr(np, flag | NV_IMPORT | NV_EXPORT, size);
        }
    }

    if (nv_isnull(PWDNOD) || nv_isattr(PWDNOD, NV_TAGGED)) {
        nv_offattr(PWDNOD, NV_TAGGED);
        path_pwd(shp);
    }

    cp = nv_getval(SHELLNOD);
    if (cp && (sh_type(cp) & SH_TYPE_RESTRICTED)) {
        sh_onoption(shp, SH_RESTRICTED);  // Restricted shell
    }
    return;
}

//
// This code is for character mapped variables with wctrans().
//
struct Mapchar {
    Namfun_t hdr;
    const char *name;
    wctrans_t trans;
};

static_fn void put_trans(Namval_t *np, const void *vp, int flags, Namfun_t *fp) {
    const char *val = vp;
    struct Mapchar *mp = (struct Mapchar *)fp;
    int c, offset = staktell(), off = offset;
    if (val) {
        if (!mp->trans || (flags & NV_INTEGER)) goto skip;
        while ((c = mb1char(val))) {
            c = towctrans(c, mp->trans);
            stakseek(off + c);
            stakseek(off);
            c = wctomb(stakptr(off), c);
            off += c;
            stakseek(off);
        }
        stakputc(0);
        val = stakptr(offset);
    } else {
        nv_putv(np, val, flags, fp);
        nv_disc(np, fp, DISC_OP_POP);
        if (!(fp->nofree & 1)) free(fp);
        stakseek(offset);
        return;
    }
skip:
    nv_putv(np, val, flags, fp);
    stakseek(offset);
}

static const Namdisc_t TRANS_disc = {
    sizeof(struct Mapchar), put_trans, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

Namfun_t *nv_mapchar(Namval_t *np, const char *name) {
    struct Mapchar *mp = (struct Mapchar *)nv_hasdisc(np, &TRANS_disc);

    if (!name) return mp ? (Namfun_t *)mp->name : NULL;

    wctrans_t trans = wctrans(name);
    if (!trans) return NULL;

    size_t n = 0;
    int low = strcmp(name, e_tolower);
    if (low && strcmp(name, e_toupper)) n += strlen(name) + 1;
    if (mp) {
        if (strcmp(name, mp->name) == 0) return &mp->hdr;
        nv_disc(np, &mp->hdr, DISC_OP_POP);
        if (!(mp->hdr.nofree & 1)) free(mp);
    }
    mp = calloc(1, sizeof(struct Mapchar) + n);
    mp->trans = trans;
    if (low == 0) {
        mp->name = e_tolower;
    } else if (n == 0) {
        mp->name = e_toupper;
    } else {
        mp->name = (char *)(mp + 1);
        strcpy((char *)mp->name, name);
    }
    mp->hdr.disc = &TRANS_disc;
    return &mp->hdr;
}
