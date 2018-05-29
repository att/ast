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
// at&t Research
//
// coshell export var set/unset
//
#include "config_ast.h"  // IWYU pragma: keep

#include "colib.h"

//
// Set or unset coshell export variable
//

int coexport(Coshell_t *co, const char *name, const char *value) {
    Coexport_t *ex;
    char *v;

    if (!co->export) {
        co->exdisc = calloc(1, sizeof(Dtdisc_t));
        if (!co->exdisc) return -1;
        co->exdisc->link = offsetof(Coexport_t, link);
        co->exdisc->key = offsetof(Coexport_t, name);
        co->exdisc->size = 0;
        co->export = dtnew(co->exdisc, Dtset);
        if (!co->export) {
            free(co->exdisc);
            return -1;
        }
    }
    ex = (Coexport_t *)dtmatch(co->export, name);
    if (!ex) {
        if (!value) return 0;
        ex = calloc(1, sizeof(Coexport_t) + strlen(name));
        if (!ex) return -1;
        strcpy(ex->name, name);
        dtinsert(co->export, ex);
    }
    if (ex->value) {
        free(ex->value);
        ex->value = 0;
    }
    if (value) {
        if (!(v = strdup(value))) return -1;
        ex->value = v;
    } else {
        dtdelete(co->export, ex);
        free(ex);
    }
    co->init.sync = 1;
    return 0;
}
