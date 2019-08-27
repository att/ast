/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2013 AT&T Intellectual Property          *
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
// David Korn
// AT&T Labs
//
// Shell script to shell binary converter.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>
#include <sys/stat.h>

#include "argnod.h"
#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "name.h"
#include "option.h"
#include "sfio.h"
#include "shnodes.h"
#include "stk.h"

#define CNTL(x) ((x)&037)
#define VERSION 3
static const char header[6] = {CNTL('k'), CNTL('s'), CNTL('h'), 0, VERSION, 0};

int main(int argc, char *argv[]) {
    Sfio_t *in, *out;
    Shell_t *shp;
    Namval_t *np;
    Shnode_t *t;
    char *cp;
    int n, nflag = 0, vflag = 0, dflag = 0;

    error_info.id = argv[0];
    shp = sh_init(argc, argv, NULL);
    while ((n = optget(argv, sh_optshcomp))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'D': {
                dflag = 1;
                break;
            }
            case 'v': {
                vflag = 1;
                break;
            }
            case 'n': {
                nflag = 1;
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
        }
    }
    shp->shcomp = 1;
    argv += opt_info.index;
    argc -= opt_info.index;
    if (error_info.errors || argc > 2) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    cp = *argv;
    if (cp) {
        argv++;
        in = sh_pathopen(shp, cp);
    } else {
        in = sfstdin;
    }
    cp = *argv;
    if (cp) {
        struct stat statb;
        if (!(out = sfopen(NULL, cp, "w"))) {
            errormsg(SH_DICT, ERROR_system(1), "%s: cannot create", cp);
            __builtin_unreachable();
        }
        if (fstat(sffileno(out), &statb) >= 0) {
            chmod(cp, (statb.st_mode & ~S_IFMT) | S_IXUSR | S_IXGRP | S_IXOTH);
        }
    } else {
        out = sfstdout;
    }
    if (dflag) {
        sh_onoption(shp, SH_DICTIONARY);
        sh_onoption(shp, SH_NOEXEC);
    }
    sh_trap(shp, "enum _Bool=(false true) ;", 0);
    if (nflag) sh_onoption(shp, SH_NOEXEC);
    if (vflag) sh_onoption(shp, SH_VERBOSE);
    if (!dflag) sfwrite(out, header, sizeof(header));
    shp->inlineno = 1;
    sh_onoption(shp, SH_BRACEEXPAND);
    while (1) {
        stkset(stkstd, NULL, 0);
        t = sh_parse(shp, in, 0);
        if (t) {
            if ((t->tre.tretyp & (COMMSK | COMSCAN)) == TCOM && t->com.comnamp &&
                strcmp(nv_name((Namval_t *)t->com.comnamp), "alias") == 0) {
                sh_exec(shp, t, 0);
            }
            if (!dflag && sh_tdump(out, t) < 0) {
                errormsg(SH_DICT, ERROR_exit(1), "dump failed");
                __builtin_unreachable();
            }
        } else if (sfeof(in)) {
            break;
        }
        if (sferror(in)) {
            errormsg(SH_DICT, ERROR_system(1), "I/O error");
            __builtin_unreachable();
        }
        if (t && ((t->tre.tretyp & COMMSK) == TCOM) && (np = t->com.comnamp) &&
            (cp = nv_name(np))) {
            if (strcmp(cp, "exit") == 0) break;
            // Check for exec of a command.
            if (strcmp(cp, "exec") == 0) {
                if (t->com.comtyp & COMSCAN) {
                    if (t->com.comarg->argnxt.ap) break;
                } else {
                    struct dolnod *ap = (struct dolnod *)t->com.comarg;
                    if (ap->dolnum > 1) break;
                }
            }
        }
    }
    // Copy any remaining input.
    sfmove(in, out, SF_UNBOUND, -1);
    if (in != sfstdin) sfclose(in);
    if (out != sfstdout) sfclose(out);
    return 0;
}
