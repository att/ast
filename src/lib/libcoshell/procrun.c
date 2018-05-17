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
//
// Glenn Fowler
// AT&T Research
//
// Coshell procrun(3)
//
#include "config_ast.h"  // IWYU pragma: keep

#include "colib.h"

#include <proc.h>

int coprocrun(const char *path, char **argv, int flags) {
    char *s;
    char **a;
    Sfio_t *tmp;
    int n;
    a = argv;
    if (!a) {
        return procclose(
            procopen(path, a, NULL, NULL, PROC_FOREGROUND | PROC_GID | PROC_UID | flags));
    }
    tmp = sfstropen();
    if (!tmp) return -1;
    sfputr(tmp, path ? path : "sh", -1);
    while (s = *++a) {
        sfputr(tmp, " '", -1);
        coquote(tmp, s, 0);
        sfputc(tmp, '\'');
    }
    s = costash(tmp);
    if (!s) return -1;
    n = cosystem(s);
    sfstrclose(tmp);
    return n;
}
