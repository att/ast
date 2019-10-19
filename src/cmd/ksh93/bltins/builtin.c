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

#include <dlfcn.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "builtins.h"
#include "defs.h"
#include "dlldefs.h"
#include "error.h"
#include "name.h"
#include "optget_long.h"
#include "path.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"

typedef struct Libcomp_s {
    void *dll;
    char *lib;
    dev_t dev;
    ino_t ino;
    nvflag_t attr;
} Libcomp_t;

static Libcomp_t *liblist = NULL;
static int nlib = 0;

typedef void (*Libinit_f)(int, void *);

#define GROWLIB 4

static int maxlib;
static const char *short_options = "df:lnps";
static const struct optget_option long_options[] = {
    {"help", optget_no_arg, NULL, 1},  // all builtins supports --help
    {NULL, 0, NULL, 0}};

//
// Add library to loaded list. Call (*lib_init)() on first load if defined. Always move to head of
// search list.
//
// Return: 0: already loaded 1: first load
//
int sh_addlib(Shell_t *shp, void *dll, char *name, Pathcomp_t *pp) {
    int n;
    int r;
    Libinit_f initfn;
    Shbltin_t *sp = &shp->bltindata;

    sp->nosfio = 0;
    for (n = r = 0; n < nlib; n++) {
        if (r) {
            liblist[n - 1] = liblist[n];
        } else if (liblist[n].dll == dll) {
            r++;
        }
    }
    if (r) {
        nlib--;
    } else if ((initfn = (Libinit_f)dlllook(dll, "lib_init"))) {
        (*initfn)(0, sp);
    }
    if (nlib >= maxlib) {
        maxlib += GROWLIB;
        liblist = realloc(liblist, (maxlib + 1) * sizeof(Libcomp_t));
    }
    liblist[nlib].dll = dll;
    liblist[nlib].attr = (sp->nosfio ? BLT_NOSFIO : 0);
    if (name) liblist[nlib].lib = strdup(name);
    if (pp) {
        liblist[nlib].dev = pp->dev;
        liblist[nlib].ino = pp->ino;
    }
    nlib++;
    return !r;
}

Shbltin_f sh_getlib(Shell_t *shp, char *sym, Pathcomp_t *pp) {
    UNUSED(shp);
    int n;

    for (n = 0; n < nlib; n++) {
        if (liblist[n].ino == pp->ino && liblist[n].dev == pp->dev) {
            return (Shbltin_f)dlllook(liblist[n].dll, sym);
        }
    }
    return 0;
}

//
// The `builtin` command. Add, change or list built-ins. Adding builtins requires dlopen()
// interface.
//
int b_builtin(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    char *arg = NULL, *name;
    int n, opt;
    int r = 0;
    nvflag_t flag = 0;
    Namval_t *np = NULL;
    void *disable = NULL;
    struct tdata tdata;
    Shbltin_f addr;
    Stk_t *stkp;
    char *errmsg;
    void *library = NULL;
    unsigned long ver;
    bool list = false;
    char path[PATH_MAX];

    memset(&tdata, 0, sizeof(tdata));
    tdata.sh = context->shp;
    stkp = tdata.sh->stk;
    if (!tdata.sh->pathlist) path_absolute(tdata.sh, argv[0], NULL);

    optget_ind = 0;
    while ((opt = optget_long(argc, argv, short_options, long_options)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
            case 's': {
                flag = BLT_SPC;
                break;
            }
            case 'n': {
                flag = BLT_DISABLE;
                disable = builtin_disable;
                break;
            }
            case 'd': {
                disable = builtin_delete;
                break;
            }
            case 'f': {
                arg = optget_arg;
                break;
            }
            case 'l': {
                list = true;
                break;
            }
            case 'p': {
                tdata.prefix = argv[0];
                break;
            }
            case ':': {
                builtin_missing_argument(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            case '?': {
                builtin_unknown_option(shp, cmd, argv[optget_ind - 1]);
                return 2;
            }
            default: { abort(); }
        }
    }

    argv += optget_ind;
    if (arg || *argv) {
        if (sh_isoption(tdata.sh, SH_RESTRICTED)) {
            errormsg(SH_DICT, ERROR_exit(1), e_restricted, argv[-optget_ind]);
            __builtin_unreachable();
        }
        if (tdata.sh->subshell && !tdata.sh->subshare) sh_subfork();
    }
    if (tdata.prefix && disable == builtin_disable) {
        if (*tdata.prefix == 'e') {
            tdata.prefix = "enable -n";
        } else {
            tdata.prefix = "builtin -n";
        }
    }

    if (arg) {
        if (!(library = dllplugin(SH_ID, arg, NULL, SH_PLUGIN_VERSION, &ver, RTLD_LAZY, path,
                                  sizeof(path)))) {
            errormsg(SH_DICT, ERROR_exit(0), "%s: %s", arg, dllerror(0));
            return 1;
        }
        if (list) sfprintf(sfstdout, "%s %08lu %s\n", arg, ver, path);
        sh_addlib(tdata.sh, library, arg, NULL);
    } else {
        if (*argv == 0 && disable != builtin_delete) {
            if (tdata.prefix) {
                for (n = 0; n < nlib; n++) {
                    sfprintf(sfstdout, "%s -f %s\n", tdata.prefix, liblist[n].lib);
                }
            }
            print_scan(sfstdout, flag, tdata.sh->bltin_tree, true, &tdata);
            return 0;
        }
    }
    size_t stkoff = stktell(stkp);
    r = 0;
    while (*argv) {
        arg = *argv;
        if (tdata.prefix) {
            sfprintf(sfstdout, "%s %s\n", tdata.prefix, arg);
            argv++;
            continue;
        }
        name = path_basename(arg);
        sfwrite(stkp, "b_", 2);
        sfputr(stkp, name, 0);
        errmsg = 0;
        addr = NULL;
        if (disable || nlib) {
            for (n = (nlib ? nlib : disable ? 1 : 0); --n >= 0;) {
                if (!disable && !liblist[n].dll) continue;
                if (disable || (addr = (Shbltin_f)dlllook(liblist[n].dll, stkptr(stkp, stkoff)))) {
                    np = sh_addbuiltin(tdata.sh, arg, addr, disable);
                    if (np) {
                        if (disable || nv_isattr(np, BLT_SPC)) {
                            errmsg = "restricted name";
                        } else {
                            nv_onattr(np, liblist[n].attr);
                        }
                    }
                    break;
                }
            }
        }
        if (!addr) {
            np = nv_search(arg, context->shp->bltin_tree, 0);
            if (np) {
                if (nv_isattr(np, BLT_SPC)) errmsg = "restricted name";
                addr = FETCH_VT(np->nvalue, shbltinp);
            }
        }
        if (!disable && !addr) {
            np = sh_addbuiltin(tdata.sh, arg, NULL, 0);
            if (!np) errmsg = "not found";
        }
        if (errmsg) {
            errormsg(SH_DICT, ERROR_exit(0), "%s: %s", *argv, errmsg);
            r = 1;
        }
        if (!disable && np) nv_offattr(np, BLT_DISABLE);
        stkseek(stkp, stkoff);
        argv++;
    }
    return r;
}
