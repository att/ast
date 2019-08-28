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
// command [-pvVx] name [arg...]
//
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "builtins.h"
#include "defs.h"
#include "error.h"
#include "option.h"
#include "shcmd.h"

//
// The `command` command is called with argc==0 when checking for -V or -v option. In this case
// return 0 when -v or -V or unknown option, otherwise the shift count to the command is returned.
//
int b_command(int argc, char *argv[], Shbltin_t *context) {
    int n, flags = 0;
    Shell_t *shp = context->shp;
    Optdisc_t disc;

    memset(&disc, 0, sizeof(disc));
    disc.version = OPT_VERSION;
    opt_info.disc = &disc;
    opt_info.index = opt_info.offset = 0;

    while ((n = optget(argv, sh_optcommand))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'p': {
                if (sh_isoption(shp, SH_RESTRICTED)) {
                    errormsg(SH_DICT, ERROR_exit(1), e_restricted, "-p");
                    __builtin_unreachable();
                }
                sh_onstate(shp, SH_DEFPATH);
                break;
            }
            case 'v': {
                flags |= WHENCE_X_FLAG;
                break;
            }
            case 'V': {
                flags |= WHENCE_V_FLAG;
                break;
            }
            case 'x': {
                shp->xargexit = 1;
                break;
            }
            case ':': {
                if (argc == 0) return 0;
                errormsg(SH_DICT, 2, "%s", opt_info.arg);
                break;
            }
            case '?': {
                if (argc == 0) return 0;
                errormsg(SH_DICT, ERROR_usage(2), "%s", opt_info.arg);
                __builtin_unreachable();
            }
        }
    }
    if (argc == 0) return flags ? 0 : opt_info.index;
    argv += opt_info.index;
    if (error_info.errors || !*argv) {
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }
    return whence(shp, argv, flags);
}
