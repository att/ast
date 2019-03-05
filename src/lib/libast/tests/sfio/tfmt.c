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
#include "config_ast.h"  // IWYU pragma: keep

#include <string.h>

#include "sfio.h"
#include "terror.h"

static char *Mystr = "abc";
int myprint(Sfio_t *f, void *v, Sffmt_t *fe) {
    UNUSED(f);

    if (fe->fmt == 's') {
        *((char **)v) = Mystr;
        fe->flags |= SFFMT_VALUE;
    }

    return 0;
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    char buf1[1024], buf2[1024];
    Sffmt_t fe;

    memset(&fe, 0, sizeof(Sffmt_t));
    fe.version = SFIO_VERSION;
    fe.form = "%1$s";
    fe.extf = myprint;

    sfsprintf(buf1, sizeof(buf1), "%s", Mystr);
    sfsprintf(buf2, sizeof(buf2), "%!", &fe);
    if (strcmp(buf1, buf2) != 0) terror("Failed testing $position");

    return 0;
}
