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

#include <string.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "name.h"
#include "option.h"
#include "shcmd.h"

//
// Builtin `printf` command.
//
int b_printf(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);
    int n;
    Shell_t *shp = context->shp;
    struct print prdata;

    memset(&prdata, 0, sizeof(prdata));
    prdata.fd = 1;

    while ((n = optget(argv, sh_optprintf))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'v': {
                prdata.var_name = nv_open(opt_info.arg, shp->var_tree, NV_VARNAME | NV_NOARRAY);
                if (!prdata.var_name) {
                    errormsg(SH_DICT, 2, "Cannot create variable %s", opt_info.arg);
                    return 2;
                }
                break;
            }
            case ':': {
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
            case '?': {
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
        }
    }

    argv += opt_info.index;
    if (!*argv) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    prdata.format = *argv++;
    return sh_print(argv, shp, &prdata);
}
