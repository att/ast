/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2011 AT&T Intellectual Property          *
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
//  This installs a hook to allow the processing of events when the shell is waiting for input and
//  when the shell is waiting for job completion. The previous waitevent hook function is returned.
//
#include "config_ast.h"  // IWYU pragma: keep

#include "defs.h"

void *sh_waitnotify(int (*newevent)(int, long, int)) {
    int (*old)(int, long, int);
    old = shgd->waitevent;
    shgd->waitevent = newevent;
    return old;
}
