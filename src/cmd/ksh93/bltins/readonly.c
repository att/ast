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
// The `readonly` builtin.
//
int b_readonly(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);
    int flag;
    char *command = argv[0];
    struct tdata tdata;

    memset(&tdata, 0, sizeof(tdata));
    tdata.sh = context->shp;
    tdata.aflag = '-';
    tdata.argnum = -1;  // do not change size
    while ((flag = optget(argv, sh_optreadonly))) {
        switch (flag) {  //!OCLINT(MissingDefaultStatement)
            case 'p': {
                tdata.prefix = command;
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
    if (error_info.errors) {
        errormsg(SH_DICT, ERROR_usage(2), optusage(NULL));
        __builtin_unreachable();
    }

    argv += (opt_info.index - 1);
    nvflag_t nvflags = (NV_ASSIGN | NV_RDONLY | NV_VARNAME);
    return setall(argv, nvflags, tdata.sh->var_tree, &tdata);
}
