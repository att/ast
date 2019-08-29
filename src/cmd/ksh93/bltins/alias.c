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
#include <float.h>
#include <limits.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wctype.h>

#include "argnod.h"
#include "ast.h"
#include "ast_assert.h"
#include "builtins.h"
#include "cdt.h"
#include "defs.h"
#include "dlldefs.h"
#include "error.h"
#include "fault.h"
#include "history.h"
#include "name.h"
#include "option.h"
#include "path.h"
#include "sfio.h"
#include "shcmd.h"
#include "stk.h"
#include "variables.h"

int b_alias(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);
    nvflag_t nvflags = NV_NOARRAY | NV_NOSCOPE | NV_ASSIGN;
    Dt_t *troot;
    int n;
    struct tdata tdata;

    memset(&tdata, 0, sizeof(tdata));
    tdata.sh = context->shp;
    troot = tdata.sh->alias_tree;
    if (*argv[0] == 'h') nvflags = NV_TAGGED;
    if (sh_isoption(tdata.sh, SH_BASH)) tdata.prefix = argv[0];
    if (!argv[1]) return setall(argv, nvflags, troot, &tdata);

    opt_info.offset = 0;
    opt_info.index = 1;
    *opt_info.option = 0;
    tdata.argnum = 0;
    tdata.aflag = *argv[1];
    while ((n = optget(argv, sh_optalias))) {
        switch (n) {  //!OCLINT(MissingDefaultStatement)
            case 'p': {
                tdata.prefix = argv[0];
                break;
            }
            case 't': {
                nvflags |= NV_TAGGED;
                break;
            }
            case 'x': {
                nvflags |= NV_EXPORT;
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
        errormsg(SH_DICT, ERROR_usage(2), "%s", optusage(NULL));
        __builtin_unreachable();
    }

    argv += (opt_info.index - 1);
    if (!nv_isflag(nvflags, NV_TAGGED)) return setall(argv, nvflags, troot, &tdata);

    // Hacks to handle hash -r | --.
    if (argv[1] && argv[1][0] == '-') {
        if (argv[1][1] == 'r' && argv[1][2] == 0) {
            Namval_t *np = nv_search_namval(PATHNOD, tdata.sh->var_tree, 0);
            nv_putval(np, nv_getval(np), NV_RDONLY);
            argv++;
            if (!argv[1]) return 0;
        }
        if (argv[1][0] == '-') {
            if (argv[1][1] == '-' && argv[1][2] == 0) {
                argv++;
            } else {
                errormsg(SH_DICT, ERROR_exit(1), e_option, argv[1]);
                __builtin_unreachable();
            }
        }
    }
    troot = tdata.sh->track_tree;
    return setall(argv, nvflags, troot, &tdata);
}
