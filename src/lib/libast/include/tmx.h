/* : : generated from tmx by iffe version 2013-11-14 : : */
#ifndef _TMX_H
#define _TMX_H 1
/*
 * AT&T Research
 *
 * high resolution Time_t support
 */

#include "tm.h"
#include "tv.h"

// #define TMX_MAXDATE "2554-07-21+23:34:33.709551614 UTC"
#define TMX_MAXYEAR 2554
#define TMX_RESOLUTION 1000000000

typedef uint64_t Time_t;
typedef uint64_t Tmxsec_t;
typedef uint32_t Tmxnsec_t;

#define tmxsec(t) ((Tmxsec_t)((t) / 1000000000))
#define tmxnsec(t) ((Tmxnsec_t)((t) % 1000000000))
#define tmxsns(s, n) (((((Time_t)(s)) * 1000000000)) + ((Time_t)(n)))

#define TMX_NOTIME ((Time_t)(-1))
#define TMX_NOW tmxgettime()

// #define tmx2tv(t, v) ((v)->tv_nsec = tmxnsec(t), (v)->tv_sec = tmxsec(t))
// #define tv2tmx(v) tmxsns((v)->tv_sec, (v)->tv_nsec)

#define tmxclock(p) tmxsns(((p) ? *(p) : time(NULL)), 0)

#define tmxgetatime(s) tmxsns((s)->st_atime, ST_ATIME_NSEC_GET(s))
#define tmxgetmtime(s) tmxsns((s)->st_mtime, ST_MTIME_NSEC_GET(s))
// #define tmxgetctime(s) tmxsns((s)->st_ctime, ST_CTIME_NSEC_GET(s))

// #define tmxsetatime(s, t) ((s)->st_atime = tmxsec(t), ST_ATIME_NSEC_SET(s, tmxnsec(t)))
// #define tmxsetctime(s, t) ((s)->st_ctime = tmxsec(t), ST_CTIME_NSEC_SET(s, tmxnsec(t)))
// #define tmxsetmtime(s, t) ((s)->st_mtime = tmxsec(t), ST_MTIME_NSEC_SET(s, tmxnsec(t)))

extern Time_t tmxdate(const char *, char **, Time_t);
extern char *tmxfmt(char *, size_t, const char *, Time_t);
extern Tm_t *tmxmake(Time_t);
extern Time_t tmxscan(const char *, char **, const char *, char **, Time_t, long);
extern Time_t tmxtime(Tm_t *, int);
extern Tm_t *tmxtm(Tm_t *, Time_t, Tm_zone_t *);
extern struct tm *tmlocaltime(const time_t *);
extern Time_t tmxgettime(void);
extern char *fmttmx(const char *, Time_t);

#endif  // _TMX_H
