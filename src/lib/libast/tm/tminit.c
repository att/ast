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
 * time conversion support
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "ast.h"
#include "sfio.h"
#include "tm.h"

/*
 * 2007-03-19 move tm_info from _tm_info_ to (*_tm_infop_)
 *            to allow future Tm_info_t growth
 *            by 2009 _tm_info_ can be static
 */

Tm_info_t _tm_info_ = {.flags = 0};
Tm_info_t *_tm_infop_ = &_tm_info_;

static char TZ[256];
static char *ZZ[2];

struct tm *tmlocaltime(const time_t *t) {
    struct tm *r;
    char *e = NULL;
    char **v = environ;

    if (TZ[0]) {
        if (!environ || !*environ) {
            environ = ZZ;
        } else {
            e = environ[0];
        }
        environ[0] = TZ;
    }
    r = localtime(t);
    if (TZ[0]) {
        if (environ != v) {
            environ = v;
        } else {
            environ[0] = e;
        }
    }
    return r;
}

/*
 * return minutes west of GMT for local time clock
 *
 * isdst will point to non-zero if DST is in effect
 * this routine also kicks in the local initialization
 */
static_fn int tzwest(time_t *clock, int *isdst) {
    struct tm *tp;
    int n;
    int m;
    int h;
    time_t epoch;

    /*
     * convert to GMT assuming local time
     */
    if (!(tp = gmtime(clock))) {
        /*
         * some systems return 0 for negative time_t
         */
        epoch = 0;
        clock = &epoch;
        tp = gmtime(clock);
    }
    n = tp->tm_yday;
    h = tp->tm_hour;
    m = tp->tm_min;

    /*
     * tmlocaltime() handles DST and GMT offset
     */

    tp = tmlocaltime(clock);
    n = tp->tm_yday - n;
    if (n) {
        if (n > 1) {
            n = -1;
        } else if (n < -1) {
            n = 1;
        }
    }
    *isdst = tp->tm_isdst;
    return (h - tp->tm_hour - n * 24) * 60 + m - tp->tm_min;
}

/*
 * initialize the local timezone
 */

static_fn void tmlocal(void) {
    Tm_zone_t *zp;
    int n;
    char *s;
    char *e = NULL;
    int i;
    int m;
    int isdst;
    char *t;
    struct tm *tp;
    time_t now;
    char buf[16];

    static Tm_zone_t local;

    {
        char **v = environ;

        s = getenv("TZ");
        if (s) {
            sfsprintf(TZ, sizeof(TZ), "TZ=%s", s);
            if (!environ || !*environ) {
                environ = ZZ;
            } else {
                e = environ[0];
            }
            environ[0] = TZ;
        } else {
            TZ[0] = 0;
            e = NULL;
        }
        tzset();
        if (environ != v) {
            environ = v;
        } else if (e) {
            environ[0] = e;
        }
    }

    tmlocale();

    /*
     * tm_info.local
     */

    tm_info.zone = tm_info.local = &local;
    time(&now);
    n = tzwest(&now, &isdst);

    /*
     * compute local DST offset by roaming
     * through the last 12 months until tzwest() changes
     */

    for (i = 0; i < 12; i++) {
        now -= 31 * 24 * 60 * 60;
        if ((m = tzwest(&now, &isdst)) != n) {
            if (!isdst) {
                isdst = n;
                n = m;
                m = isdst;
            }
            m -= n;
            break;
        }
    }
    local.west = n;
    local.dst = m;

    /*
     * now get the time zone names
     */

    if (tzname[0]) {
        /*
         * POSIX
         */

        local.standard = strdup(tzname[0]);
        local.daylight = strdup(tzname[1]);
    } else if ((s = getenv("TZNAME")) && *s && (s = strdup(s))) {
        // BSD
        local.standard = s;
        s = strchr(s, ',');
        if (s) {
            *s++ = 0;
        } else {
            s = "";
        }
        local.daylight = s;
    } else if ((s = getenv("TZ")) && *s && *s != ':' && (s = strdup(s))) {
        /*
         * POSIX style but skipped by tmlocaltime()
         */

        local.standard = s;
        if (*++s && *++s && *++s) {
            *s++ = 0;
            tmgoff(s, &t, 0);
            for (s = t; isalpha(*t); t++) {
                ;
            }
            *t = 0;
        } else {
            s = "";
        }
        local.daylight = s;
    } else {
        //
        // tm_data.zone table lookup.
        //
        t = NULL;
        for (zp = tm_data.zone; zp->standard; zp++) {
            if (zp->type) t = zp->type;
            if (zp->west == n && zp->dst == m) break;
        }
        if (zp->standard) {
            local.type = t;
            local.standard = zp->standard;
            s = zp->daylight;
            if (!s) {
                s = buf;
                e = s + sizeof(buf);
                s = tmpoff(s, e - s, zp->standard, 0, 0);
                if (s < e - 1) {
                    *s++ = ' ';
                    tmpoff(s, e - s, tm_info.format[TM_DT], m, TM_DST);
                }
                s = strdup(buf);
            }
            local.daylight = s;
        } else {
            //
            // Not in the table.
            //
            s = buf;
            e = s + sizeof(buf);
            s = tmpoff(s, e - s, tm_info.format[TM_UT], n, 0);
            local.standard = strdup(buf);
            if (s < e - 1) {
                *s++ = ' ';
                tmpoff(s, e - s, tm_info.format[TM_UT], m, TM_DST);
                local.daylight = strdup(buf);
            }
        }
    }
    if (!*local.standard && !local.west && !local.dst && (s = getenv("TZ"))) {
        if ((zp = tmzone(s, &t, NULL, NULL)) && !*t) {
            local.standard = strdup(zp->standard);
            if (zp->daylight) local.daylight = strdup(zp->daylight);
            local.west = zp->west;
            local.dst = zp->dst;
        } else {
            local.standard = strdup(s);
        }
        if (!local.standard) local.standard = "";
        if (!local.daylight) local.daylight = "";
    }

    /*
     * the time zone type is probably related to the locale
     */

    if (!local.type) {
        s = local.standard;
        t = 0;
        for (zp = tm_data.zone; zp->standard; zp++) {
            if (zp->type) t = zp->type;
            if (tmword(s, NULL, zp->standard, NULL, 0)) {
                local.type = t;
                break;
            }
        }
    }

    /*
     * tm_info.flags
     */

    if (!(tm_info.flags & TM_ADJUST)) {
        now = (time_t)78811200; /* Jun 30 1972 23:59:60 */
        tp = tmlocaltime(&now);
        if (tp->tm_sec != 60) tm_info.flags |= TM_ADJUST;
    }
    if (!(tm_info.flags & TM_UTC)) {
        s = local.standard;
        zp = tm_data.zone;
        if (local.daylight) zp++;
        for (; !zp->type && zp->standard; zp++) {
            if (tmword(s, NULL, zp->standard, NULL, 0)) {
                tm_info.flags |= TM_UTC;
                break;
            }
        }
    }
}

/*
 * initialize tm data
 */

void tminit(Tm_zone_t *zp) {
    static uint32_t serial = ~(uint32_t)0;

    if (serial != ast.env_serial) {
        serial = ast.env_serial;
        if (tm_info.local) {
            memset(tm_info.local, 0, sizeof(*tm_info.local));
            tm_info.local = 0;
        }
    }
    if (!tm_info.local) tmlocal();
    if (!zp) zp = tm_info.local;
    tm_info.zone = zp;
}
