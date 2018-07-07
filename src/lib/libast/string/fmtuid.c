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
 * uid number -> name
 */
#include "config_ast.h"  // IWYU pragma: keep

#define getpwuid ______getpwuid

#include <pwd.h>
#include <stddef.h>
#include <string.h>

#include "ast.h"
#include "cdt.h"
#include "sfio.h"

#undef getpwuid

extern struct passwd *getpwuid(uid_t);

typedef struct Id_s {
    Dtlink_t link;
    int id;
    char name[1];
} Id_t;

/*
 * return uid name given uid number
 */

char *fmtuid(int uid) {
    Id_t *ip;
    char *name;
    struct passwd *pw;
    int z;

    static Dt_t *dict;
    static Dtdisc_t disc;

    if (!dict) {
        disc.key = offsetof(Id_t, id);
        disc.size = sizeof(int);
        dict = dtopen(&disc, Dtset);
    } else {
        ip = (Id_t *)dtmatch(dict, &uid);
        if (ip) return ip->name;
    }
    pw = getpwuid(uid);
    if (pw) {
        name = pw->pw_name;
#if __CYGWIN__
        if (streq(name, "Administrator")) name = "root";
#endif
    } else if (uid == 0) {
        name = "root";
    } else {
        name = fmtbuf(z = sizeof(uid) * 3 + 1);
        sfsprintf(name, z, "%I*d", sizeof(uid), uid);
    }
    if (dict && (ip = newof(0, Id_t, 1, strlen(name)))) {
        ip->id = uid;
        strcpy(ip->name, name);
        dtinsert(dict, ip);
        return ip->name;
    }
    return name;
}
