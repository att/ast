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
/*
 * Glenn Fowler
 * AT&T Research
 *
 * 8 bit character code map name/id lookup support
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <sys/types.h>

#include "ast.h"
#include "ast_api.h"
#include "ast_ccode.h"
#include "ccode.h"

static const Ccmap_t maps[] = {
    {"ascii", "a|ascii|?(iso)?(-)646|?(iso)?(-)8859|latin", "8 bit ascii", "ISO-8859-%s", "1",
     CC_ASCII, NULL},

    {"ebcdic", "e|ebcdic?(-)?([1e])", "X/Open ebcdic", "EBCDIC", NULL, CC_EBCDIC_E, NULL},

    {"ebcdic-o", "o|ebcdic?(-)[3o]|?(cp|ibm)1047|open?(-)edition", "mvs OpenEdition ebcdic",
     "EBCDIC-O", NULL, CC_EBCDIC_O, NULL},

    {"ebcdic-h", "h|ebcdic?(-)h|?(cp|ibm)?(00)37|[oa]s?(/-)400", "ibm OS/400 AS/400 ebcdic",
     "EBCDIC-H", NULL, CC_EBCDIC_H, NULL},

    {"ebcdic-s", "s|ebcdic?(-)s|siemens|posix-bc", "siemens posix-bc ebcdic", "EBCDIC-S", NULL,
     CC_EBCDIC_S, NULL},

    {"ebcdic-i", "i|ebcdic?(-)[2i]|ibm", "X/Open ibm ebcdic (not idempotent)", "EBCDIC-I", NULL,
     CC_EBCDIC_I, NULL},

    {"ebcdic-m", "m|ebcdic?(-)m|mvs", "mvs ebcdic", "EBCDIC-M", NULL, CC_EBCDIC_M, NULL},

    {"ebcdic-u", "u|ebcdic?(-)(u|mf)|microfocus", "microfocus cobol ebcdic", "EBCDIC-U", NULL,
     CC_EBCDIC_U, NULL},

    {"native", "n|native|local", "native code set", NULL, NULL, CC_NATIVE, NULL},

    {NULL, NULL, NULL, NULL, NULL, 0, NULL},
};

/*
 * ccode map list iterator
 */

Ccmap_t *ccmaplist(Ccmap_t *mp) { return !mp ? (Ccmap_t *)maps : (++mp)->name ? mp : (Ccmap_t *)0; }

/*
 * return ccode map id given name
 */

int ccmapid(const char *name) {
    const Ccmap_t *mp;
    int c;
    const Ccmap_t *bp;
    int n;
    ssize_t sub[2];

    bp = 0;
    n = 0;
    for (mp = maps; mp->name; mp++)
        if (strgrpmatch(name, mp->match, sub, elementsof(sub) / 2,
                        STR_MAXIMAL | STR_LEFT | STR_ICASE)) {
            if (!(c = name[sub[1]])) return mp->ccode;
            if (sub[1] > n && !isalpha(c)) {
                n = sub[1];
                bp = mp;
            }
        }
    return bp ? bp->ccode : -1;
}

/*
 * return ccode map name given id
 */

char *ccmapname(int id) {
    const Ccmap_t *mp;

    for (mp = maps; mp->name; mp++)
        if (id == mp->ccode) return (char *)mp->name;
    return 0;
}
