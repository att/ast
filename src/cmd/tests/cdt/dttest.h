/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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
#include "cdt.h"
#include "terror.h"

static int compare(Dt_t *dt, void *o1, void *o2, Dtdisc_t *disc) {
    return (int)((char *)o1 - (char *)o2);
}

static int rcompare(Dt_t *dt, void *o1, void *o2, Dtdisc_t *disc) {
    return (int)((char *)o2 - (char *)o1);
}

static void *newint(Dt_t *dt, void *o, Dtdisc_t *disc) { return o; }

static unsigned int hashint(Dt_t *dt, void *o, Dtdisc_t *disc) {
    return (unsigned int)((char *)o - (char *)0);
}
