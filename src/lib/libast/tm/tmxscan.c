/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2014 AT&T Intellectual Property          *
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
 * Time_t conversion support
 *
 * scan date expression in s using format
 * if non-null, e points to the first invalid sequence in s
 * if non-null, f points to the first unused format char
 * t provides default values
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "ast.h"
#include "sfio.h"
#include "tm.h"
#include "tmx.h"

typedef struct {
    int32_t nsec;
    int year;
    int mon;
    int week;
    int weektype;
    int yday;
    int mday;
    int wday;
    int hour;
    int min;
    int sec;
    int meridian;
    int zone;
} Set_t;

#define CLEAR(s)                                                                                \
    (s.year = s.mon = s.week = s.weektype = s.yday = s.mday = s.wday = s.hour = s.min = s.sec = \
         s.meridian = (-1),                                                                     \
     s.nsec = 1000000000L, s.zone = TM_LOCALZONE)

#define INDEX(m, x) (((n) >= ((x) - (m))) ? ((n) -= ((x) - (m))) : (n))

#define NUMBER(d, m, x)                                                                      \
    do {                                                                                     \
        n = 0;                                                                               \
        u = (char *)s;                                                                       \
        while (s < (const char *)(u + d) && *s >= '0' && *s <= '9') n = n * 10 + *s++ - '0'; \
        if (u == (char *)s || n < m || n > x) goto next;                                     \
    } while (0)

/*
 * generate a Time_t from tm + set
 */

static_fn Time_t tmx_gen(Tm_t *tm, Set_t *set) {
    int n;
    int z;
    Time_t t;

    if (set->year >= 0) tm->tm_year = set->year;
    if (set->mon >= 0) {
        if (set->year < 0 && set->mon < tm->tm_mon) tm->tm_year++;
        tm->tm_mon = set->mon;
        if (set->yday < 0 && set->mday < 0) tm->tm_mday = set->mday = 1;
    }
    if (set->week >= 0) {
        if (set->mon < 0) {
            tmweek(tm, set->weektype, set->week, set->wday);
            set->wday = -1;
        }
    } else if (set->yday >= 0) {
        if (set->mon < 0) {
            tm->tm_mon = 0;
            tm->tm_mday = set->yday + 1;
        }
    } else if (set->mday >= 0) {
        tm->tm_mday = set->mday;
    }
    if (set->hour >= 0) {
        if (set->hour < tm->tm_hour && set->yday < 0 && set->mday < 0 && set->wday < 0) {
            tm->tm_mday++;
        }
        tm->tm_hour = set->hour;
        tm->tm_min = (set->min >= 0) ? set->min : 0;
        tm->tm_sec = (set->sec >= 0) ? set->sec : 0;
    } else if (set->min >= 0) {
        tm->tm_min = set->min;
        tm->tm_sec = (set->sec >= 0) ? set->sec : 0;
    } else if (set->sec >= 0) {
        tm->tm_sec = set->sec;
    }
    if (set->nsec < 1000000000L) tm->tm_nsec = set->nsec;
    if (set->meridian > 0) {
        if (tm->tm_hour < 12) tm->tm_hour += 12;
    } else if (set->meridian == 0) {
        if (tm->tm_hour >= 12) tm->tm_hour -= 12;
    }
    t = tmxtime(tm, set->zone);
    if (set->yday >= 0) {
        z = 1;
        tm = tmxtm(tm, t, tm->tm_zone);
        tm->tm_mday += set->yday - tm->tm_yday;
    } else if (set->wday >= 0) {
        z = 1;
        tm = tmxtm(tm, t, tm->tm_zone);
        if ((n = set->wday - tm->tm_wday) < 0) n += 7;
        tm->tm_mday += n;
    } else {
        z = 0;
    }
    if (set->nsec < 1000000000L) {
        if (!z) {
            z = 1;
            tm = tmxtm(tm, t, tm->tm_zone);
        }
        tm->tm_nsec = set->nsec;
    }
    return z ? tmxtime(tm, set->zone) : t;
}

/*
 * the format scan workhorse
 */

static_fn Time_t tmx_scan(const char *s, char **e, const char *format, char **f, Time_t t,
                          long flags) {
    int d;
    int n;
    char *p;
    Tm_t *tm;
    const char *b;
    char *stack[4];
    int m;
    int hi;
    int lo;
    int pedantic;
    Time_t x;
    Set_t set;
    Tm_zone_t *zp;
    Tm_t ts;
    char *u = NULL;
    char **sp = &stack[0];

    while (isspace(*s)) s++;
    b = s;
again:
    CLEAR(set);
    tm = tmxtm(&ts, t, NULL);
    pedantic = (flags & TM_PEDANTIC) != 0;
    for (;;) {
        if (!(d = *format++)) {
            if (sp <= &stack[0]) {
                format--;
                break;
            }
            format = (const char *)*--sp;
        } else if (!*s) {
            format--;
            break;
        } else if (d == '%' && (d = *format) && format++ && d != '%') {
        more:
            switch (d) {
                case 'a':
                    lo = TM_DAY_ABBREV;
                    hi = pedantic ? TM_DAY : TM_TIME;
                    goto get_wday;
                case 'A':
                    lo = pedantic ? TM_DAY : TM_DAY_ABBREV;
                    hi = TM_TIME;
                get_wday:
                    if ((n = tmlex(s, &u, tm_info.format + lo, hi - lo, NULL, 0)) < 0) goto next;
                    s = u;
                    INDEX(TM_DAY_ABBREV, TM_DAY);
                    set.wday = n;
                    continue;
                case 'b':
                case 'h':
                    lo = TM_MONTH_ABBREV;
                    hi = pedantic ? TM_MONTH : TM_DAY_ABBREV;
                    goto get_mon;
                case 'B':
                    lo = pedantic ? TM_MONTH : TM_MONTH_ABBREV;
                    hi = TM_DAY_ABBREV;
                get_mon:
                    if ((n = tmlex(s, &u, tm_info.format + lo, hi - lo, NULL, 0)) < 0) goto next;
                    s = u;
                    INDEX(TM_MONTH_ABBREV, TM_MONTH);
                    set.mon = n;
                    continue;
                case 'c':
                    p = "%a %b %e %T %Y";
                    break;
                case 'C':
                    NUMBER(2, 19, 99);
                    set.year = (n - 19) * 100 + tm->tm_year % 100;
                    continue;
                case 'd':
                    if (pedantic && !isdigit(*s)) goto next;
                    // FALLTHRU
                case 'e':
                    NUMBER(2, 1, 31);
                    set.mday = n;
                    continue;
                case 'D':
                    p = "%m/%d/%y";
                    break;
                case 'E':
                case 'O':
                    if (*format) {
                        d = *format++;
                        goto more;
                    }
                    continue;
                case 'F':
                    p = "%Y-%m-%d";
                    break;
                case 'H':
                case 'k':
                    NUMBER(2, 0, 23);
                    set.hour = n;
                    continue;
                case 'I':
                case 'l':
                    NUMBER(2, 1, 12);
                    set.hour = n;
                    continue;
                case 'j':
                    NUMBER(3, 1, 366);
                    set.yday = n - 1;
                    continue;
                case 'm':
                    NUMBER(2, 1, 12);
                    set.mon = n - 1;
                    continue;
                case 'M':
                    NUMBER(2, 0, 59);
                    set.min = n;
                    continue;
                case 'n':
                    if (pedantic) {
                        while (*s == '\n') s++;
                        continue;
                    }
                    // FALLTHRU
                case 't':
                    while (isspace(*s)) s++;
                    continue;
                case 'N':
                    NUMBER(9, 0, 999999999L);
                    set.nsec = n;
                    continue;
                case 'p':
                    if ((n = tmlex(s, &u, tm_info.format + TM_MERIDIAN, TM_UT - TM_MERIDIAN, NULL,
                                   0)) < 0) {
                        goto next;
                    }
                    set.meridian = n;
                    s = u;
                    continue;
                case 'r':
                    p = "%I:%M:%S %p";
                    break;
                case 'R':
                    p = "%H:%M:%S";
                    break;
                case 's':
                    x = strtoul(s, &u, 0);
                    if (s == u) goto next;
                    tm = tmxtm(tm, tmxsns(x, 0), tm->tm_zone);
                    s = u;
                    CLEAR(set);
                    continue;
                case 'S':
                    NUMBER(2, 0, 61);
                    set.sec = n;
                    continue;
                case 'u':
                    NUMBER(2, 1, 7);
                    set.wday = n % 7;
                    continue;
                case 'U':
                    NUMBER(2, 0, 52);
                    set.week = n;
                    set.weektype = 0;
                    continue;
                case 'V':
                    NUMBER(2, 1, 53);
                    set.week = n;
                    set.weektype = 2;
                    continue;
                case 'w':
                    NUMBER(2, 0, 6);
                    set.wday = n;
                    continue;
                case 'W':
                    NUMBER(2, 0, 52);
                    set.week = n;
                    set.weektype = 1;
                    continue;
                case 'x':
                    p = tm_info.format[TM_DATE];
                    break;
                case 'X':
                    p = tm_info.format[TM_TIME];
                    break;
                case 'y':
                    NUMBER(2, 0, 99);
                    if (n < TM_WINDOW) n += 100;
                    set.year = n;
                    continue;
                case 'Y':
                    NUMBER(4, 1969, 2100);
                    set.year = n - 1900;
                    continue;
                case 'Z':
                case 'q':
                    zp = tmtype(s, &u);
                    if (zp) {
                        s = u;
                        u = zp->type;
                    } else {
                        u = NULL;
                    }
                    if (d == 'q') continue;
                // FALLTHRU
                case 'z':
                    zp = tmzone(s, &u, u, &m);
                    if (zp) {
                        s = u;
                        set.zone = zp->west + m;
                        tm_info.date = zp;
                    }
                    continue;
                case '|':
                    s = b;
                    goto again;
                case '&':
                    (void)tmx_gen(tm, &set);
                    x = tmxdate(s, e, t);
                    if (s == (const char *)*e) goto next;
                    t = x;
                    s = (const char *)*e;
                    if (!*format || (*format == '%' && *(format + 1) == '|')) goto done;
                    goto again;
                default:
                    goto next;
            }
            if (sp >= &stack[elementsof(stack)]) goto next;
            *sp++ = (char *)format;
            format = (const char *)p;
        } else if (isspace(d)) {
            while (isspace(*s)) s++;
        } else if (*s != d) {
            break;
        } else {
            s++;
        }
    }
next:
    if (sp > &stack[0]) format = (const char *)stack[0];
    if (*format) {
        p = (char *)format;
        if (!*s && *p == '%' && *(p + 1) == '|') {
            format += strlen(format);
        } else {
            while (*p) {
                if (*p++ == '%' && *p && *p++ == '|' && *p) {
                    format = (const char *)p;
                    s = b;
                    goto again;
                }
            }
        }
    }
    t = tmx_gen(tm, &set);
done:
    if (e) {
        while (isspace(*s)) s++;
        *e = (char *)s;
    }
    if (f) {
        while (isspace(*format)) format++;
        *f = (char *)format;
    }
    return t;
}

/*
 *  format==0   DATEMSK
 * *format==0   DATEMSK and tmxdate()
 * *format!=0   format
 */

Time_t tmxscan(const char *s, char **e, const char *format, char **f, Time_t t, long flags) {
    char *v;
    char **p;
    char *q;
    char *r;
    Time_t x;

    static int initialized = 0;
    static char **datemask = NULL;

    tmlocale();

    if (format && *format) return tmx_scan(s, e, format, f, t, flags);

    if (!initialized) {
        Sfio_t *sp;
        int n;
        off_t m;

        initialized = 1;
        if ((v = getenv("DATEMSK")) && *v && (sp = sfopen(NULL, v, "r"))) {
            for (n = 1; sfgetr(sp, '\n', 0); n++) {
                ;
            }
            m = sfseek(sp, 0L, SEEK_CUR);
            p = calloc(1, n * sizeof(char *) + m);
            if (p) {
                sfseek(sp, 0L, SEEK_SET);
                v = (char *)(p + n);
                if (sfread(sp, v, m) != m) {
                    free(p);
                    p = 0;
                } else {
                    datemask = p;
                    v[m] = 0;
                    while (*v) {
                        *p++ = v;
                        if (!(v = strchr(v, '\n'))) break;
                        *v++ = 0;
                    }
                    *p = 0;
                }
            }
        }
    }

    p = datemask;
    if (p) {
        while ((v = *p++)) {
            x = tmx_scan(s, &q, v, &r, t, flags);
            if (!*q && !*r) {
                if (e) *e = q;
                if (f) *f = r;
                return x;
            }
        }
    }
    if (f) *f = (char *)format;
    if (format) return tmxdate(s, e, t);
    if (e) *e = (char *)s;
    return 0;
}
