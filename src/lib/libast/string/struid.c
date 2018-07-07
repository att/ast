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
 *               Glenn Fowler <glenn.s.fowler@gmail.com>                *
 *                    David Korn <dgkorn@gmail.com>                     *
 *                     Phong Vo <phongvo@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * uid name -> number
 */
#include "config_ast.h"  // IWYU pragma: keep

#define getpwnam ______getpwnam
#define getpwuid ______getpwuid

#include <pwd.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "cdt.h"

#undef getpwnam
#undef getpwuid

extern struct passwd *getpwnam(const char *);
extern struct passwd *getpwuid(uid_t);

typedef struct Id_s {
    Dtlink_t link;
    int id;
    char name[1];
} Id_t;

/*
 * return uid number given uid name
 * -1 on first error for a given name
 * -2 on subsequent errors for a given name
 */

int struid(const char *name) {
    Id_t *ip;
    struct passwd *pw;
    int id;
    char *e;

    static Dt_t *dict;
    static Dtdisc_t disc;

    if (!dict) {
        disc.key = offsetof(Id_t, name);
        dict = dtopen(&disc, Dtset);
    } else {
        ip = (Id_t *)dtmatch(dict, name);
        if (ip) return ip->id;
    }
    pw = getpwnam(name);
    if (pw) {
        id = pw->pw_uid;
    } else {
        id = strtol(name, &e, 0);
#if __CYGWIN__
        if (!*e) {
            if (!getpwuid(id)) id = -1;
        } else if (streq(name, "root") && (pw = getpwnam("Administrator")))
            id = pw->pw_uid;
        else
            id = -1;
#else
        if (*e || !getpwuid(id)) id = -1;
#endif
    }
    if (dict && (ip = newof(0, Id_t, 1, strlen(name)))) {
        strcpy(ip->name, name);
        ip->id = id >= 0 ? id : -2;
        dtinsert(dict, ip);
    }
    return id;
}
