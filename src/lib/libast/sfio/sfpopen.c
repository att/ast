/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2012 AT&T Intellectual Property          *
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
 *                    David Korn <dgkorn@gmail.com>                     *
 *                     Phong Vo <phongvo@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
#include "config_ast.h"  // IWYU pragma: keep

#include <stddef.h>

#include "sfhdr.h"  // IWYU pragma: keep
#include "sfio.h"

/*      Create a coprocess.
**      Written by Kiem-Phong Vo.
*/

#include "proc.h"

Sfio_t *sfpopen(Sfio_t *f, const char *command, const char *mode) {
    Proc_t *proc;
    int sflags;
    long flags;
    int pflags;
    char *av[4];

    if (!command || !command[0] || !mode) return 0;
    sflags = _sftype(mode, NULL, NULL, NULL);

    if (f == (Sfio_t *)(-1)) { /* stdio compatibility mode */
        f = NULL;
        pflags = 1;
    } else {
        pflags = 0;
    }

    flags = 0;
    if (sflags & SF_READ) flags |= PROC_READ;
    if (sflags & SF_WRITE) flags |= PROC_WRITE;
    av[0] = "sh";
    av[1] = "-c";
    av[2] = (char *)command;
    av[3] = 0;
    if (!(proc = procopen(0, av, 0, 0, flags))) return 0;
    if (!(f = sfnew(f, NULL, (size_t)SF_UNBOUND, (sflags & SF_READ) ? proc->rfd : proc->wfd,
                    sflags | ((sflags & SF_RDWR) ? 0 : SF_READ))) ||
        _sfpopen(f, (sflags & SF_READ) ? proc->wfd : -1, proc->pid, pflags) < 0) {
        if (f) sfclose(f);
        procclose(proc);
        return 0;
    }
    procfree(proc);
    return f;
}
