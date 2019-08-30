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

#include <stdbool.h>
#include <string.h>

#include "builtins.h"
#include "cdt.h"
#include "defs.h"
#include "error.h"
#include "name.h"
#include "option.h"
#include "shcmd.h"

// The `unalias` builtin.
int b_unalias(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);
    Shell_t *shp = context->shp;
    Dt_t *troot = shp->alias_tree;
    nvflag_t nvflags = NV_NOSCOPE;
    bool all = false;
    int n;

    if (shp->subshell) troot = sh_subaliastree(shp, 0);
    while ((n = optget(argv, sh_optunalias))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'a': {
                all = true;
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
    if (error_info.errors || (!*argv && !all)) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    if (!troot) return 1;
    if (all) {
        dtclear(troot);
        return 0;
    }
    return nv_unall(argv, true, nvflags, troot, shp);
}
