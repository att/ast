/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
 *               Glenn Fowler <glenn.s.fowler@gmail.com>                *
 *                                                                      *
 ***********************************************************************/
/*
 * Glenn Fowler
 * AT&T Research
 *
 * coshell system(3)
 */
#include "config_ast.h"  // IWYU pragma: keep

#include "colib.h"

int cosystem(const char *cmd) {
    Coshell_t *co;
    Cojob_t *cj;
    int status;

    if (!cmd) return !eaccess(pathshell(), X_OK);
    if (!(co = coopen(NULL, CO_ANY, NULL))) return -1;
    if (cj = coexec(co, cmd, CO_SILENT, NULL, NULL, NULL)) cj = cowait(co, cj, -1);
    if (!cj) return -1;

    /*
     * synthesize wait() status from shell status
     * lack of synthesis is the standard's proprietary sellout
     */

    status = cj->status;
    if (EXITED_TERM(status))
        status &= ((1 << (EXIT_BITS - 1)) - 1);
    else
        status = (status & ((1 << EXIT_BITS) - 1)) << EXIT_BITS;
    return status;
}
