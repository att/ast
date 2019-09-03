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

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "argnod.h"
#include "builtins.h"
#include "cdt.h"
#include "defs.h"
#include "error.h"
#include "name.h"
#include "shcmd.h"
#include "variables.h"

static const char *short_options = ":ptx";
static const struct option long_options[] = {{"help", 0, NULL, 1},  // all builtins supports --help
                                             {NULL, 0, NULL, 0}};

int b_alias(int argc, char *argv[], Shbltin_t *context) {
    UNUSED(argc);
    int opt;
    Dt_t *troot;
    struct tdata tdata;
    Shell_t *shp = context->shp;
    char *cmd = argv[0];
    nvflag_t nvflags = NV_NOARRAY | NV_NOSCOPE | NV_ASSIGN;

    memset(&tdata, 0, sizeof(tdata));
    tdata.sh = shp;
    troot = tdata.sh->alias_tree;
    if (sh_isoption(tdata.sh, SH_BASH)) tdata.prefix = cmd;
    if (!argv[1]) return setall(argv, nvflags, troot, &tdata);

    tdata.argnum = 0;
    tdata.aflag = *argv[1];

    optind = 0;
    while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (opt) {
            case 1: {
                builtin_print_help(shp, cmd);
                return 0;
            }
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

    argv += (optind - 1);
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
