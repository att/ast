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

#include "ast.h"
#include "builtins.h"
#include "defs.h"
#include "shcmd.h"

//
// Builtin `echo`.
//
// See https://github.com/att/ast/issues/370 for a discussion about the `echo` builtin.
int b_echo(int argc, char *argv[], Shbltin_t *context) {
    static char bsd_univ;
    struct print prdata;
    prdata.options = sh_optecho + 5;
    prdata.raw = prdata.echon = 0;
    prdata.sh = context->shp;
    UNUSED(argc);

    // The external `echo` command is different on BSD and ATT platforms. So
    // base our behavior on the contents of $PATH.
    if (!prdata.sh->echo_universe_valid) {
        bsd_univ = path_is_bsd_universe();
        prdata.sh->echo_universe_valid = true;
    }
    if (!bsd_univ) return b_print(0, argv, (Shbltin_t *)&prdata);
    prdata.options = sh_optecho;
    prdata.raw = 1;
    while (argv[1] && *argv[1] == '-') {
        if (strcmp(argv[1], "-n") == 0) {
            prdata.echon = 1;
        } else if (strcmp(argv[1], "-e") == 0) {
            prdata.raw = 0;
        } else if (strcmp(argv[1], "-ne") == 0 || strcmp(argv[1], "-en") == 0) {
            prdata.raw = 0;
            prdata.echon = 1;
        } else {
            break;
        }
        argv++;
    }
    return b_print(0, argv, (Shbltin_t *)&prdata);
}
