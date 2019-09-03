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

#include <signal.h>
#include <stddef.h>

#include "name.h"
#include "shtable.h"

//
// This is the table of built-in aliases.  These should be exported.
//
const struct shtable2 shtab_aliases[] = {{"autoload", NV_NOFREE, "typeset -fu", NULL},
                                         {"bool", NV_NOFREE | BLT_DCL, "_Bool", NULL},
                                         {"command", NV_NOFREE, "command ", NULL},
                                         {"compound", NV_NOFREE | BLT_DCL, "typeset -C", NULL},
                                         {"fc", NV_NOFREE, "hist", NULL},
                                         {"float", NV_NOFREE | BLT_DCL, "typeset -lE", NULL},
                                         {"functions", NV_NOFREE, "typeset -f", NULL},
                                         {"hash", NV_NOFREE, "alias -t --", NULL},
                                         {"history", NV_NOFREE, "hist -l", NULL},
                                         {"integer", NV_NOFREE | BLT_DCL, "typeset -li", NULL},
                                         {"nameref", NV_NOFREE | BLT_DCL, "typeset -n", NULL},
                                         {"nohup", NV_NOFREE, "nohup ", NULL},
                                         {"r", NV_NOFREE, "hist -s", NULL},
                                         {"redirect", NV_NOFREE, "command exec", NULL},
#ifdef SIGTSTP
                                         {"stop", NV_NOFREE, "kill -s STOP", NULL},
                                         {"suspend", NV_NOFREE, "kill -s STOP \"$$\"", NULL},
#endif  // SIGTSTP
                                         {"type", NV_NOFREE, "whence -v", NULL},
                                         {NULL, 0, NULL, NULL}};
