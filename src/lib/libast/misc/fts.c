/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2011 AT&T Intellectual Property          *
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
 *                 Glenn Fowler <gsf@research.att.com>                  *
 *                  David Korn <dgk@research.att.com>                   *
 *                   Phong Vo <kpv@research.att.com>                    *
 *                                                                      *
 ***********************************************************************/
/*
 * Phong Vo
 * Glenn Fowler
 * AT&T Research
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>
#include <sys/stat.h>   // IWYU pragma: keep
#include <sys/types.h>  // IWYU pragma: keep

#include <fts.h>  // OpenBSD and possibly others require the above includes first

#include "ast.h"

/*
 * return default (FTS_LOGICAL|FTS_META|FTS_PHYSICAL|FTS_SEEDOTDIR) flags
 * conditioned by astconf()
 */

int fts_flags(void) {
    char *s;

    s = astconf("PATH_RESOLVE", 0, 0);
    if (!strcmp(s, "logical")) return FTS_LOGICAL;
    if (!strcmp(s, "physical")) return FTS_PHYSICAL | FTS_SEEDOT;
    return FTS_COMFOLLOW | FTS_PHYSICAL | FTS_SEEDOT;
}
