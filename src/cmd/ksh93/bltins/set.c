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

#include "argnod.h"
#include "builtins.h"
#include "defs.h"
#include "sfio.h"
#include "shcmd.h"

int b_set(int argc, char *argv[], Shbltin_t *context) {
    Shell_t *shp = context->shp;
    struct tdata tdata;
    int was_monitor = sh_isoption(shp, SH_MONITOR);

    memset(&tdata, 0, sizeof(tdata));
    tdata.sh = shp;
    tdata.prefix = NULL;
    if (argv[1]) {
        if (sh_argopts(argc, argv, tdata.sh) < 0) return 2;
        if (sh_isoption(shp, SH_VERBOSE)) {
            sh_onstate(shp, SH_VERBOSE);
        } else {
            sh_offstate(shp, SH_VERBOSE);
        }
        if (sh_isoption(shp, SH_MONITOR) && !was_monitor) {
            sh_onstate(shp, SH_MONITOR);
        } else if (!sh_isoption(shp, SH_MONITOR) && was_monitor) {
            sh_offstate(shp, SH_MONITOR);
        }
    } else {
        // Scan name chain and print.
        print_scan(sfstdout, 0, tdata.sh->var_tree, false, &tdata);
    }
    return 0;
}
