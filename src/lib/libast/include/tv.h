/* : : generated from tv by iffe version 2013-11-14 : : */
#pragma prototyped
#ifndef _TV_H
#define _TV_H	1
/*
 * AT&T Research
 *
 * high resolution Tv_t interface definitions
 */

#include <ast.h>

#define TV_NSEC_IGNORE		(1000000000L)
#define TV_TOUCH_RETAIN		((Tv_t*)1)

typedef struct Tv_s
{
	uint32_t	tv_sec;
	uint32_t	tv_nsec;
} Tv_t;


#if STAT_ST_MTIM
    #define ST_ATIME_NSEC_GET(st)	((st)->st_atim.tv_nsec)
    #define ST_CTIME_NSEC_GET(st)	((st)->st_ctim.tv_nsec)
    #define ST_MTIME_NSEC_GET(st)	((st)->st_mtim.tv_nsec)
#elif STAT_ST_MTIMESPEC
    #define ST_ATIME_NSEC_GET(st)   ((st)->st_atimespec.tv_nsec)
    #define ST_CTIME_NSEC_GET(st)   ((st)->st_ctimespec.tv_nsec)
    #define ST_MTIME_NSEC_GET(st)   ((st)->st_mtimespec.tv_nsec)
#endif

#define ST_ATIME_NSEC_SET(st,n)	(ST_ATIME_NSEC_GET(st)=(n))
#define ST_CTIME_NSEC_SET(st,n)	(ST_CTIME_NSEC_GET(st)=(n))
#define ST_MTIME_NSEC_SET(st,n)	(ST_MTIME_NSEC_GET(st)=(n))

#define tvgetatime(t,s)	((t)->tv_nsec=ST_ATIME_NSEC_GET(s),(t)->tv_sec=(s)->st_atime)
#define tvgetmtime(t,s)	((t)->tv_nsec=ST_MTIME_NSEC_GET(s),(t)->tv_sec=(s)->st_mtime)
#define tvgetctime(t,s)	((t)->tv_nsec=ST_CTIME_NSEC_GET(s),(t)->tv_sec=(s)->st_ctime)

#define tvsetatime(t,s)	(ST_ATIME_NSEC_SET(s,(t)->tv_nsec),(s)->st_atime=(t)->tv_sec)
#define tvsetmtime(t,s)	(ST_MTIME_NSEC_SET(s,(t)->tv_nsec),(s)->st_mtime=(t)->tv_sec)
#define tvsetctime(t,s)	(ST_CTIME_NSEC_SET(s,(t)->tv_nsec),(s)->st_ctime=(t)->tv_sec)

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int		tvgettime(Tv_t*);
extern int		tvsettime(const Tv_t*);
extern int		tvcmp(const Tv_t*, const Tv_t*);
extern int		tvtouch(const char*, const Tv_t*, const Tv_t*, const Tv_t*, int);
extern int		tvsleep(const Tv_t*, Tv_t*);

extern char*		fmttv(const char*, Tv_t*);

#endif
