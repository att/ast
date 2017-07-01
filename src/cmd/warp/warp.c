/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2013 AT&T Intellectual Property          *
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
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * time related system call intercept
 * the env var WARP controls the seconds offset added to each time_t
 * and the time progression factor
 */

static const char id[] = "\n@(#)$Id: warp (AT&T Research) 2013-07-29 $\0\n";

#define _AST_INTERCEPT_IMPLEMENT	2

#if defined(_SCO_C_DIALECT) || defined(_SCO_ELF) || defined(_SCO_XPG_VERS)
#define _NO_STATIC	1
#define _WARP_stat32	1
#endif

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide alarm _alarm __alarm clock_gettime _clock_gettime __clock_gettime gettimeofday _gettimeofday __gettimeofday getitimer _getitimer __getitimer poll _poll __poll select _select __select setitimer _setitimer __setitimer time times utime _utime __utime utimensat _utimensat __utimensat utimes _utimes __utimes utimets _utimets __utimets fstat64 lstat64
__STDPP__directive pragma pp:hide execlp _execlp __execlp execve _execve __execve execvp _execvp __execvp execvpe _execvpe __execvpe
#else
#define alarm		______alarm
#define _alarm		_______alarm
#define __alarm		________alarm
#define clock_gettime	______clock_gettime
#define _clock_gettime	_______clock_gettime
#define __clock_gettime	________clock_gettime
#define gettimeofday	______gettimeofday
#define _gettimeofday	_______gettimeofday
#define __gettimeofday	________gettimeofday
#define getitimer	______getitimer
#define _getitimer	_______getitimer
#define __getitimer	________getitimer
#define poll		______poll
#define _poll		_______poll
#define __poll		________poll
#define select		______select
#define _select		_______select
#define __select	________select
#define setitimer	______setitimer
#define _setitimer	_______setitimer
#define __setitimer	________setitimer
#define time		______time
#define times		______times
#define utime		______utime
#define _utime		_______utime
#define __utime		________utime
#define utimes		______utimes
#define _utimes		_______utimes
#define __utimes	________utimes
#define utimensat	______utimensat
#define _utimensat	_______utimensat
#define __utimensat	________utimensat
#define utimets		______utimets
#define _utimets	_______utimets
#define __utimets	________utimets
#define fstat64		______fstat64
#define lstat64		______lstat64
#define execlp		______execlp
#define _execlp		_______execlp
#define __execlp	________execlp
#define execve		______execve
#define _execve		_______execve
#define __execve	________execve
#define execvp		______execvp
#define _execvp		_______execvp
#define __execvp	________execvp
#define execvpe		______execvpe
#define _execvpe	_______execvpe
#define __execvpe	________execvpe
#endif

#ifdef __EXPORT__
#define extern	__EXPORT__
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#undef	extern
#endif

#include <ast.h>
#include <dlldefs.h>
#include <ls.h>
#include <times.h>

#include "FEATURE/lib"

#if _hdr_utime
#include <utime.h>
#else
struct utimbuf
{
	time_t		actime;
	time_t		modtime;
};
#endif

#if !_typ_struct_timespec
struct timespec
{
	time_t		tv_sec;
	time_t		tv_nsec;
};
#endif

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide alarm _alarm __alarm clock_gettime _clock_gettime __clock_gettime gettimeofday _gettimeofday __gettimeofday getitimer _getitimer __getitimer poll _poll __poll select _select __select setitimer _setitimer __setitimer time times utime _utime __utime utimensat _utimensat __utimensat utimes _utimes __utimes utimets _utimets __utimets fstat64 lstat64
__STDPP__directive pragma pp:nohide execlp _execlp __execlp execve _execve __execve execvp _execvp __execvp execvpe _execvpe __execvpe
#else
#undef	alarm
#undef	_alarm
#undef	__alarm
#undef	clock_gettime
#undef	_clock_gettime
#undef	__clock_gettime
#undef	gettimeofday
#undef	_gettimeofday
#undef	__gettimeofday
#undef	getitimer
#undef	_getitimer
#undef	__getitimer
#undef	poll
#undef	_poll
#undef	__poll
#undef	select
#undef	_select
#undef	__select
#undef	setitimer
#undef	_setitimer
#undef	__setitimer
#undef	time
#undef	times
#undef	utime
#undef	_utime
#undef	__utime
#undef	utimensat
#undef	_utimensat
#undef	__utimensat
#undef	utimes
#undef	_utimes
#undef	__utimes
#undef	fstat64
#undef	lstat64
#undef	execlp
#undef	_execlp
#undef	__execlp
#undef	execve
#undef	_execve
#undef	__execve
#undef	execvp
#undef	_execvp
#undef	__execvp
#undef	execvpe
#undef	_execvpe
#undef	__execvpe
#endif

#ifdef __EXPORT__
#define extern	__EXPORT__
#endif

#if defined(LINUX_STAT_VERSION) && !defined(_STAT_VER)
#define _STAT_VER	LINUX_STAT_VERSION
#endif

#undef	fstat
#undef	lstat
#undef	stat

typedef void* (*Syscall_f)(void*);

typedef unsigned int (*Alarm_f)(unsigned int);
typedef int (*Gettimeofday_f)(struct timeval*, void*);
typedef int (*Poll_f)(void*, int, int);
typedef int (*Select_f)(int, void*, void*, void*, const struct timeval*);
typedef time_t (*Time_f)(time_t*);
typedef clock_t (*Times_f)(struct tms*);
typedef int (*Utime_f)(const char*, const struct utimbuf*);
typedef int (*Utimensat_f)(int, const char*, const struct timespec*, int);
typedef int (*Utimes_f)(const char*, const struct timeval*);
typedef int (*Utimets_f)(const char*, const struct timespec*);

typedef int (*Close_f)(int);
typedef int (*Execlp_f)(const char*, const char*, ...);
typedef int (*Execve_f)(const char*, char* const[], char* const[]);
typedef int (*Execvp_f)(const char*, char* const[]);
typedef int (*Execvpe_f)(const char*, char* const[], char* const[]);

#if defined(_STAT_VER)
typedef int (*Xstat_f)(int, const char*, struct stat*);
typedef int (*Fxstat_f)(const int, int, struct stat*);
typedef int (*Lxstat_f)(const int, const char*, struct stat*);
#if defined(_lib__xstat64) || defined(_lib___xstat64)
typedef int (*Xstat64_f)(int, const char*, struct stat64*);
typedef int (*Fxstat64_f)(const int, int, struct stat64*);
typedef int (*Lxstat64_f)(const int, const char*, struct stat64*);
#endif
#else
typedef int (*Fstat_f)(int, struct stat*);
typedef int (*Lstat_f)(const char*, struct stat*);
typedef int (*Stat_f)(const char*, struct stat*);
#endif

#if defined(_lib_stat64)
typedef int (*Fstat64_f)(int, struct stat64*);
typedef int (*Lstat64_f)(const char*, struct stat64*);
typedef int (*Stat64_f)(const char*, struct stat64*);
#endif

#if _lib_clock_gettime
typedef int (*Clock_gettime_f)(int, struct timespec*);
#endif

#if _lib_getitimer
typedef int (*Getitimer_f)(int, struct itimerval*);
#endif

#if _lib_setitimer
typedef int (*Setitimer_f)(int, const struct itimerval*, struct itimerval*);
#endif

#define WARP_ABS(t)	((t)+=(t)?(state.warp+(state.factor?mix((t)-state.base):0)):0)
#define UNWARP_ABS(t)	((t)=(t)?(((t)-state.warp)/(state.factor+1)+state.base/(state.factor+1)*state.factor):0)

#define WARP_REL(t)	((t)=(t)?(state.factor?mix(t):(t)):0)
#define UNWARP_REL(t)	((t)=(t)?(state.factor?((t)/state.factor):(t)):0)

typedef struct
{
	const char*	name;
	const char*	mangled;
	Syscall_f	call;
	int		warped;
	unsigned long	level;
} Call_t;

static struct
{
	long		base;
	void*		dll;
	long		factor;
	unsigned long	level;
	unsigned long	mix;
	int		trace;
	long		warp;
	char*		env[32];
} state;

#ifndef environ
extern char**		environ;
#endif

/*
 * synthesize the low order seconds in s
 * under the influence of a warp factor
 */

static unsigned long
mix(unsigned long s)
{
	state.mix = state.mix * 0x63c63cd9L + 0x9c39c33dL;
	return s * state.factor + state.mix % state.factor;
}

/*
 * initialize the warp environment
 * and intercept call p
 */

static void
intercept(register Call_t* p)
{
	register int		c;
	register char*		s;
	register long		n;
	register int		v;
	char*			o;

	static const char	msg[] = "warp: panic: dllnext() failed\n";

	if (p->warped)
		return;
	if (!state.dll)
	{
#ifdef __EXPORT__
		o = getenv("WARP");
#else
		register char**	e;
		register char**	p;
		register char**	x;
		char*		t;

		e = environ;
if (!e) { s = "warp: AHA: no env\n"; write(2, s, strlen(s)); }
		p = state.env;
		x = &state.env[elementsof(state.env) - 1];
		o = 0;
		while (s = *e++)
		{
			switch (s[0])
			{
			case 'L':
				if (s[1] != 'D' || s[2] != '_' || s[3] != 'P' || s[4] != 'R' || s[5] != 'E' || s[6] != 'L' || s[7] != 'O' || s[8] != 'A' || s[9] != 'D' || s[10] != '=')
					continue;
				break;
			case 'W':
				if (s[1] != 'A' || s[2] != 'R' || s[3] != 'P' || s[4] != '=')
					continue;
				o = s + 5;
				break;
			case '_':
				if (s[1] != 'R' || s[2] != 'L' || s[3] != 'D' || s[4] != '_' && s[4] != 'N')
					continue;
				break;
			default:
				continue;
			}
			if (p >= x)
				break;
			if (!(t = malloc(strlen(s) + 1)))
			{
				s = "warp: out of space [env]\n";
				write(2, s, strlen(s));
				_exit(125);
			}
			*p++ = strcpy(t, s);
		}
#endif
		if (s = o)
		{
			v = 'w';
			for (;;)
			{
				switch (*s++)
				{
				case 0:
					break;
				case ' ':
				case '\t':
				case '-':
				case '+':
					v = 'w';
					continue;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					s--;
					c = s > o && *(s - 1) == '-';
					n = 0;
					while (*s >= '0' && *s <= '9')
						n = n * 10 + *s++ - '0';
					if (n)
					{
						if (c)
							n = -n;
						switch (v)
						{
						case 'b':
							state.base = n;
							break;
						case 'f':
							state.factor = n;
							break;
						case 'w':
							state.warp = n;
							break;
						}
						v = 'w';
					}
					continue;
				case 't':
					state.trace = 1;
					continue;
				default:
					v = *(s - 1);
					continue;
				}
				break;
			}
			if (state.trace)
			{
				write(2, id + 10, strlen(id + 10));
				write(2, " ", 1);
				write(2, o, strlen(o));
				write(2, "\n", 1);
			}
		}
		if (state.factor == 1)
			state.factor = 0;
		state.mix = state.base;
		if (!(state.dll = dllnext(RTLD_LAZY)))
		{
			write(2, msg, sizeof(msg) - 1);
			exit(2);
		}
	}
	if (!p->call && !(p->call = (Syscall_f)dlsym(state.dll, p->mangled)))
	{
		write(2, "warp: ", 6);
		write(2, p->mangled, strlen(p->mangled));
		write(2, ": cannot intercept\n", 1);
		exit(2);
	}
	if (state.trace)
	{
		write(2, "warp: ", 6);
		write(2, p->mangled, strlen(p->mangled));
		write(2, "\n", 1);
	}
	p->level = ++state.level;
}

static unsigned int
warp_alarm(register Call_t* p, unsigned int s)
{
	unsigned int	r;

	intercept(p);
	if (p->warped || !state.factor)
		return (*(Alarm_f)p->call)(s);
	UNWARP_REL(s);
	r = (*(Alarm_f)p->call)(s);
	if (p->level != state.level)
		p->warped = 1;
	else
		WARP_REL(r);
	return r;
}

extern unsigned int
alarm(unsigned int sec)
{
	static Call_t	call = { "alarm", "alarm" };

	return warp_alarm(&call, sec);
}

#ifndef __EXPORT__

extern unsigned int
_alarm(unsigned int sec)
{
	static Call_t	call = { "alarm", "_alarm" };

	return warp_alarm(&call, sec);
}

extern unsigned int
__alarm(unsigned int sec)
{
	static Call_t	call = { "alarm", "__alarm" };

	return warp_alarm(&call, sec);
}

extern unsigned int
_libc_alarm(unsigned int sec)
{
	static Call_t	call = { "alarm", "_libc_alarm" };

	return warp_alarm(&call, sec);
}

extern unsigned int
__libc_alarm(unsigned int sec)
{
	static Call_t	call = { "alarm", "__libc_alarm" };

	return warp_alarm(&call, sec);
}

#endif

static int
warp_gettimeofday(register Call_t* p, struct timeval* tv, void* tz)
{
	int	r;

	intercept(p);
	if ((r = (*(Gettimeofday_f)p->call)(tv, tz)) != -1 && !p->warped)
	{
		if (p->level != state.level)
			p->warped = 1;
		else if (tv)
			WARP_ABS(tv->tv_sec);
	}
	return r;
}

extern int
gettimeofday(struct timeval* tv, void* tz)
{
	static Call_t	call = { "gettimeofday", "gettimeofday" };

	return warp_gettimeofday(&call, tv, tz);
}

#ifndef __EXPORT__

extern int
_gettimeofday(struct timeval* tv, void* tz)
{
	static Call_t	call = { "gettimeofday", "_gettimeofday" };

	return warp_gettimeofday(&call, tv, tz);
}

extern int
__gettimeofday(struct timeval* tv, void* tz)
{
	static Call_t	call = { "gettimeofday", "__gettimeofday" };

	return warp_gettimeofday(&call, tv, tz);
}

extern int
_libc_gettimeofday(struct timeval* tv, void* tz)
{
	static Call_t	call = { "gettimeofday", "_libc_gettimeofday" };

	return warp_gettimeofday(&call, tv, tz);
}

extern int
__libc_gettimeofday(struct timeval* tv, void* tz)
{
	static Call_t	call = { "gettimeofday", "__libc_gettimeofday" };

	return warp_gettimeofday(&call, tv, tz);
}

#endif

static int
warp_poll(register Call_t* p, void* f, int n, int to)
{
	int	r;

	intercept(p);
	if (p->warped || !state.factor || to <= 1000)
		return (*(Poll_f)p->call)(f, n, to);
	to /= 1000;
	UNWARP_REL(to);
	to *= 1000;
	if ((r = (*(Poll_f)p->call)(f, n, to)) != -1 && !p->warped && p->level != state.level)
		p->warped = 1;
	return r;
}

static int
poll(void* f, int n, int to)
{
	static Call_t	call = { "poll", "poll" };

	return warp_poll(&call, f, n, to);
}

#ifndef __EXPORT__

static int
_poll(void* f, int n, int to)
{
	static Call_t	call = { "poll", "_poll" };

	return warp_poll(&call, f, n, to);
}

static int
__poll(void* f, int n, int to)
{
	static Call_t	call = { "poll", "__poll" };

	return warp_poll(&call, f, n, to);
}

static int
_libc_poll(void* f, int n, int to)
{
	static Call_t	call = { "poll", "_libc_poll" };

	return warp_poll(&call, f, n, to);
}

static int
__libc_poll(void* f, int n, int to)
{
	static Call_t	call = { "poll", "__libc_poll" };

	return warp_poll(&call, f, n, to);
}

#endif

static int
warp_select(register Call_t* p, int n, void* rp, void* wp, void* ep, register const struct timeval* tv)
{
	int		r;
	struct timeval	x;

	intercept(p);
	if (p->warped || !state.factor || !tv || !tv->tv_sec && !tv->tv_usec)
		return (*(Select_f)p->call)(n, rp, wp, ep, tv);
	x = *tv;
	if (x.tv_sec)
	{
		UNWARP_REL(x.tv_sec);
		if (x.tv_sec)
			x.tv_usec = 0;
		else if (!(x.tv_usec /= state.factor))
			x.tv_usec = 10;
	}
	if ((r = (*(Select_f)p->call)(n, rp, wp, ep, &x)) != -1 && !p->warped && p->level != state.level)
		p->warped = 1;
	return r;
}

static int
select(int n, void* rp, void* wp, void* ep, const struct timeval* tv)
{
	static Call_t	call = { "select", "select" };

	return warp_select(&call, n, rp, wp, ep, tv);
}

#ifndef __EXPORT__

static int
_select(int n, void* rp, void* wp, void* ep, const struct timeval* tv)
{
	static Call_t	call = { "select", "_select" };

	return warp_select(&call, n, rp, wp, ep, tv);
}

static int
__select(int n, void* rp, void* wp, void* ep, const struct timeval* tv)
{
	static Call_t	call = { "select", "__select" };

	return warp_select(&call, n, rp, wp, ep, tv);
}

static int
_libc_select(int n, void* rp, void* wp, void* ep, const struct timeval* tv)
{
	static Call_t	call = { "select", "_libc_select" };

	return warp_select(&call, n, rp, wp, ep, tv);
}

static int
__libc_select(int n, void* rp, void* wp, void* ep, const struct timeval* tv)
{
	static Call_t	call = { "select", "__libc_select" };

	return warp_select(&call, n, rp, wp, ep, tv);
}

#endif

static time_t
warp_time(register Call_t* p, time_t* clock)
{
	time_t	t;

	intercept(p);
	if ((t = (*(Time_f)p->call)(clock)) != (time_t)(-1) && !p->warped)
	{
		if (p->level != state.level)
			p->warped = 1;
		else
		{
			WARP_ABS(t);
			if (clock)
				WARP_ABS(*clock);
		}
	}
	return t;
}

extern time_t
time(time_t* clock)
{
	static Call_t	call = { "time", "time" };

	return warp_time(&call, clock);
}

#ifndef __EXPORT__

extern time_t
_time(time_t* clock)
{
	static Call_t	call = { "time", "_time" };

	return warp_time(&call, clock);
}

extern time_t
__time(time_t* clock)
{
	static Call_t	call = { "time", "__time" };

	return warp_time(&call, clock);
}

extern time_t
_libc_time(time_t* clock)
{
	static Call_t	call = { "time", "_libc_time" };

	return warp_time(&call, clock);
}

extern time_t
__libc_time(time_t* clock)
{
	static Call_t	call = { "time", "__libc_time" };

	return warp_time(&call, clock);
}

#endif

static clock_t
warp_times(register Call_t* p, struct tms* tv)
{
	clock_t	t;

	intercept(p);
	if ((t = (*(Times_f)p->call)(tv)) != (clock_t)(-1) && !p->warped)
	{
		if (p->level != state.level)
			p->warped = 1;
		else
			WARP_REL(t);
	}
	return t;
}

extern clock_t
times(struct tms* tv)
{
	static Call_t	call = { "times", "times" };

	return warp_times(&call, tv);
}

#ifndef __EXPORT__

extern clock_t
_times(struct tms* tv)
{
	static Call_t	call = { "times", "_times" };

	return warp_times(&call, tv);
}

extern clock_t
__times(struct tms* tv)
{
	static Call_t	call = { "times", "__times" };

	return warp_times(&call, tv);
}

extern clock_t
_libc_times(struct tms* tv)
{
	static Call_t	call = { "times", "_libc_times" };

	return warp_times(&call, tv);
}

extern clock_t
__libc_times(struct tms* tv)
{
	static Call_t	call = { "times", "__libc_times" };

	return warp_times(&call, tv);
}

#endif

static int
warp_utime(register Call_t* p, const char* path, const struct utimbuf* tv)
{
	int		r;
	struct utimbuf	tb;

	intercept(p);
	if (!tv || p->warped)
		return (*(Utime_f)p->call)(path, tv);
	tb = *tv;
	UNWARP_ABS(tb.actime);
	UNWARP_ABS(tb.modtime);
	r = (*(Utime_f)p->call)(path, &tb);
	if (p->level != state.level)
	{
		p->warped = 1;
		r = (*(Utime_f)p->call)(path, tv);
	}
	return r;
}

extern int
utime(const char* path, const struct utimbuf* tv)
{
	static Call_t	call = { "utime", "utime" };

	return warp_utime(&call, path, tv);
}

#ifndef __EXPORT__

extern int
_utime(const char* path, const struct utimbuf* tv)
{
	static Call_t	call = { "utime", "_utime" };

	return warp_utime(&call, path, tv);
}

extern int
__utime(const char* path, const struct utimbuf* tv)
{
	static Call_t	call = { "utime", "__utime" };

	return warp_utime(&call, path, tv);
}

extern int
_libc_utime(const char* path, const struct utimbuf* tv)
{
	static Call_t	call = { "utime", "_libc_utime" };

	return warp_utime(&call, path, tv);
}

extern int
__libc_utime(const char* path, const struct utimbuf* tv)
{
	static Call_t	call = { "utime", "__libc_utime" };

	return warp_utime(&call, path, tv);
}

#endif

static int
warp_utimensat(int dirfd, register Call_t* p, const char* path, const struct timespec* tv, int flags)
{
	int		r;
	register int	i;
	struct timespec	tb[2];

	intercept(p);
	if (!tv || p->warped)
		return (*(Utimensat_f)p->call)(dirfd, path, tv, flags);
	for (i = 0; i < elementsof(tb); i++)
	{
		tb[i] = tv[i];
		UNWARP_ABS(tb[i].tv_sec);
	}
	r = (*(Utimensat_f)p->call)(dirfd, path, tb, flags);
	if (p->level != state.level)
	{
		p->warped = 1;
		r = (*(Utimensat_f)p->call)(dirfd, path, tv, flags);
	}
	return r;
}

extern int
utimensat(int dirfd, const char* path, const struct timespec* tv, int flags)
{
	static Call_t	call = { "utimensat", "utimensat" };

	return warp_utimensat(dirfd, &call, path, tv, flags);
}

#ifndef __EXPORT__

extern int
_utimensat(int dirfd, const char* path, const struct timespec* tv, int flags)
{
	static Call_t	call = { "utimensat", "_utimensat" };

	return warp_utimensat(dirfd, &call, path, tv, flags);
}

extern int
__utimensat(int dirfd, const char* path, const struct timespec* tv, int flags)
{
	static Call_t	call = { "utimensat", "__utimensat" };

	return warp_utimensat(dirfd, &call, path, tv, flags);
}

extern int
_libc_utimensat(int dirfd, const char* path, const struct timespec* tv, int flags)
{
	static Call_t	call = { "utimensat", "_libc_utimensat" };

	return warp_utimensat(dirfd, &call, path, tv, flags);
}

extern int
__libc_utimensat(int dirfd, const char* path, const struct timespec* tv, int flags)
{
	static Call_t	call = { "utimensat", "__libc_utimensat" };

	return warp_utimensat(dirfd, &call, path, tv, flags);
}

#endif

static int
warp_utimes(register Call_t* p, const char* path, const struct timeval* tv)
{
	int		r;
	register int	i;
	struct timeval	tb[2];

	intercept(p);
	if (!tv || p->warped)
		return (*(Utimes_f)p->call)(path, tv);
	for (i = 0; i < elementsof(tb); i++)
	{
		tb[i] = tv[i];
		UNWARP_ABS(tb[i].tv_sec);
	}
	r = (*(Utimes_f)p->call)(path, tb);
	if (p->level != state.level)
	{
		p->warped = 1;
		r = (*(Utimes_f)p->call)(path, tv);
	}
	return r;
}

extern int
utimes(const char* path, const struct timeval* tv)
{
	static Call_t	call = { "utimes", "utimes" };

	return warp_utimes(&call, path, tv);
}

#ifndef __EXPORT__

extern int
_utimes(const char* path, const struct timeval* tv)
{
	static Call_t	call = { "utimes", "_utimes" };

	return warp_utimes(&call, path, tv);
}

extern int
__utimes(const char* path, const struct timeval* tv)
{
	static Call_t	call = { "utimes", "__utimes" };

	return warp_utimes(&call, path, tv);
}

extern int
_libc_utimes(const char* path, const struct timeval* tv)
{
	static Call_t	call = { "utimes", "_libc_utimes" };

	return warp_utimes(&call, path, tv);
}

extern int
__libc_utimes(const char* path, const struct timeval* tv)
{
	static Call_t	call = { "utimes", "__libc_utimes" };

	return warp_utimes(&call, path, tv);
}

#endif

static int
warp_utimets(register Call_t* p, const char* path, const struct timespec* tv)
{
	int		r;
	register int	i;
	struct timespec	tb[2];

	intercept(p);
	if (!tv || p->warped)
		return (*(Utimets_f)p->call)(path, tv);
	for (i = 0; i < elementsof(tb); i++)
	{
		tb[i] = tv[i];
		UNWARP_ABS(tb[i].tv_sec);
	}
	r = (*(Utimets_f)p->call)(path, tb);
	if (p->level != state.level)
	{
		p->warped = 1;
		r = (*(Utimets_f)p->call)(path, tv);
	}
	return r;
}

extern int
utimets(const char* path, const struct timespec* tv)
{
	static Call_t	call = { "utimets", "utimets" };

	return warp_utimets(&call, path, tv);
}

#ifndef __EXPORT__

extern int
_utimets(const char* path, const struct timespec* tv)
{
	static Call_t	call = { "utimets", "_utimets" };

	return warp_utimets(&call, path, tv);
}

extern int
__utimets(const char* path, const struct timespec* tv)
{
	static Call_t	call = { "utimets", "__utimets" };

	return warp_utimets(&call, path, tv);
}

extern int
_libc_utimets(const char* path, const struct timespec* tv)
{
	static Call_t	call = { "utimets", "_libc_utimets" };

	return warp_utimets(&call, path, tv);
}

extern int
__libc_utimets(const char* path, const struct timespec* tv)
{
	static Call_t	call = { "utimets", "__libc_utimets" };

	return warp_utimets(&call, path, tv);
}

#endif

#if defined(_STAT_VER)

static void
warp_xst(register Call_t* p, const int ver, register struct stat* st)
{
	static const char	msg[] = "warp: _STAT_VER\n";

	if (!p->warped)
	{
		if (p->level != state.level)
			p->warped = 1;
#if defined(_STAT64_VER)
		else if (ver >= _STAT64_VER)
		{
			register struct stat64*	st64 = (struct stat64*)st;
			static const char	msg64[] = "warp: _STAT64_VER\n";

			if (state.trace)
				write(2, msg64, sizeof(msg64) - 1);
			WARP_ABS(st64->st_atime);
			WARP_ABS(st64->st_ctime);
			WARP_ABS(st64->st_mtime);
		}
#endif
		else if (ver == _STAT_VER)
		{
			if (state.trace)
				write(2, msg, sizeof(msg) - 1);
			WARP_ABS(st->st_atime);
			WARP_ABS(st->st_ctime);
			WARP_ABS(st->st_mtime);
		}
		else if (state.trace)
		{
			static char	msg[] = "warp: stat version 0\n";

			msg[19] = '0' + ver;
			write(2, msg, sizeof(msg) - 1);
		}
	}
}

extern int
_fxstat(const int ver, int fd, struct stat* st)
{
	int		r;

	static Call_t	call = { "_fxstat", "_fxstat" };

	intercept(&call);
	if ((r = (*(Fxstat_f)call.call)(ver, fd, st)) != -1)
		warp_xst(&call, ver, st);
	return r;
}

extern int
__fxstat(const int ver, int fd, struct stat* st)
{
	int		r;

	static Call_t	call = { "_fxstat", "__fxstat" };

	intercept(&call);
	if ((r = (*(Fxstat_f)call.call)(ver, fd, st)) != -1)
		warp_xst(&call, ver, st);
	return r;
}

extern int
_lxstat(const int ver, const char* path, struct stat* st)
{
	int		r;

	static Call_t	call = { "_lxstat", "_lxstat" };

	intercept(&call);
	if ((r = (*(Lxstat_f)call.call)(ver, path, st)) != -1)
		warp_xst(&call, ver, st);
	return r;
}

extern int
__lxstat(const int ver, const char* path, struct stat* st)
{
	int		r;

	static Call_t	call = { "_lxstat", "__lxstat" };

	intercept(&call);
	if ((r = (*(Lxstat_f)call.call)(ver, path, st)) != -1)
		warp_xst(&call, ver, st);
	return r;
}

extern int
_xstat(const int ver, const char* path, struct stat* st)
{
	int		r;

	static Call_t	call = { "_xstat", "_xstat" };

	intercept(&call);
	if ((r = (*(Xstat_f)call.call)(ver, path, st)) != -1)
		warp_xst(&call, ver, st);
	return r;
}

extern int
__xstat(const int ver, const char* path, struct stat* st)
{
	int		r;

	static Call_t	call = { "_xstat", "__xstat" };

	intercept(&call);
	if ((r = (*(Xstat_f)call.call)(ver, path, st)) != -1)
		warp_xst(&call, ver, st);
	return r;
}

#if defined(_lib__xstat64) || defined(_lib___xstat64)

static void
warp_xst64(register Call_t* p, const int ver, register struct stat64* st)
{
	static const char	msg[] = "warp: _STAT_VER\n";

	if (!p->warped)
	{
		if (p->level != state.level)
			p->warped = 1;
#if defined(_STAT64_VER)
		else if (ver >= _STAT64_VER)
		{
			register struct stat64*	st64 = (struct stat64*)st;
			static const char	msg64[] = "warp: _STAT64_VER\n";

			if (state.trace)
				write(2, msg64, sizeof(msg64) - 1);
			WARP_ABS(st64->st_atime);
			WARP_ABS(st64->st_ctime);
			WARP_ABS(st64->st_mtime);
		}
#endif
		else if (ver == _STAT_VER)
		{
			if (state.trace)
				write(2, msg, sizeof(msg) - 1);
			WARP_ABS(st->st_atime);
			WARP_ABS(st->st_ctime);
			WARP_ABS(st->st_mtime);
		}
		else if (state.trace)
		{
			static char	msg[] = "warp: stat version 0\n";

			msg[19] = '0' + ver;
			write(2, msg, sizeof(msg) - 1);
		}
	}
}

extern int
_fxstat64(const int ver, int fd, struct stat64* st)
{
	int		r;

	static Call_t	call = { "_fxstat64", "_fxstat64" };

	intercept(&call);
	if ((r = (*(Fxstat64_f)call.call)(ver, fd, st)) != -1)
		warp_xst64(&call, ver, st);
	return r;
}

extern int
__fxstat64(const int ver, int fd, struct stat64* st)
{
	int		r;

	static Call_t	call = { "_fxstat64", "__fxstat64" };

	intercept(&call);
	if ((r = (*(Fxstat64_f)call.call)(ver, fd, st)) != -1)
		warp_xst64(&call, ver, st);
	return r;
}

extern int
_lxstat64(const int ver, const char* path, struct stat64* st)
{
	int		r;

	static Call_t	call = { "_lxstat64", "_lxstat64" };

	intercept(&call);
	if ((r = (*(Lxstat64_f)call.call)(ver, path, st)) != -1)
		warp_xst64(&call, ver, st);
	return r;
}

extern int
__lxstat64(const int ver, const char* path, struct stat64* st)
{
	int		r;

	static Call_t	call = { "_lxstat64", "__lxstat64" };

	intercept(&call);
	if ((r = (*(Lxstat64_f)call.call)(ver, path, st)) != -1)
		warp_xst64(&call, ver, st);
	return r;
}

extern int
_xstat64(const int ver, const char* path, struct stat64* st)
{
	int		r;

	static Call_t	call = { "_xstat64", "_xstat64" };

	intercept(&call);
	if ((r = (*(Xstat64_f)call.call)(ver, path, st)) != -1)
		warp_xst64(&call, ver, st);
	return r;
}

extern int
__xstat64(const int ver, const char* path, struct stat64* st)
{
	int		r;

	static Call_t	call = { "_xstat64", "__xstat64" };

	intercept(&call);
	if ((r = (*(Xstat64_f)call.call)(ver, path, st)) != -1)
		warp_xst64(&call, ver, st);
	return r;
}

#endif
#else

static void
warp_st(register Call_t* p, register struct stat* st)
{
	if (!p->warped)
	{
		if (p->level != state.level)
			p->warped = 1;
		else
		{
			WARP_ABS(st->st_atime);
			WARP_ABS(st->st_ctime);
			WARP_ABS(st->st_mtime);
		}
	}
}

static int
warp_fstat(register Call_t* p, int fd, struct stat* st)
{
	int	r;

	intercept(p);
	if ((r = (*(Fstat_f)p->call)(fd, st)) != -1)
		warp_st(p, st);
	return r;
}

extern int
fstat(int fd, struct stat* st)
{
	static Call_t	call = { "fstat", "fstat" };

	return warp_fstat(&call, fd, st);
}

#ifndef __EXPORT__

extern int
_fstat(int fd, struct stat* st)
{
	static Call_t	call = { "fstat", "_fstat" };

	return warp_fstat(&call, fd, st);
}

extern int
__fstat(int fd, struct stat* st)
{
	static Call_t	call = { "fstat", "__fstat" };

	return warp_fstat(&call, fd, st);
}

extern int
_libc_fstat(int fd, struct stat* st)
{
	static Call_t	call = { "fstat", "_libc_fstat" };

	return warp_fstat(&call, fd, st);
}

extern int
__libc_fstat(int fd, struct stat* st)
{
	static Call_t	call = { "fstat", "__libc_fstat" };

	return warp_fstat(&call, fd, st);
}

#if _WARP_stat32

extern int
__fstat32(int fd, struct stat* st)
{
	static Call_t	call = { "__fstat32", "__fstat32" };

	return warp_fstat(&call, fd, st);
}

#endif

#endif

static int
warp_lstat(register Call_t* p, const char* path, struct stat* st)
{
	int	r;

	intercept(p);
	if ((r = (*(Lstat_f)p->call)(path, st)) != -1)
		warp_st(p, st);
	return r;
}

extern int
lstat(const char* path, struct stat* st)
{
	static Call_t	call = { "lstat", "lstat" };

	return warp_lstat(&call, path, st);
}

#ifndef __EXPORT__

extern int
_lstat(const char* path, struct stat* st)
{
	static Call_t	call = { "lstat", "_lstat" };

	return warp_lstat(&call, path, st);
}

extern int
__lstat(const char* path, struct stat* st)
{
	static Call_t	call = { "lstat", "__lstat" };

	return warp_lstat(&call, path, st);
}

extern int
_libc_lstat(const char* path, struct stat* st)
{
	static Call_t	call = { "lstat", "_libc_lstat" };

	return warp_lstat(&call, path, st);
}

extern int
__libc_lstat(const char* path, struct stat* st)
{
	static Call_t	call = { "lstat", "__libc_lstat" };

	return warp_lstat(&call, path, st);
}

#if _WARP_stat32

extern int
__lstat32(const char* path, struct stat* st)
{
	static Call_t	call = { "__lstat32", "__lstat32" };

	return warp_lstat(&call, path, st);
}

extern int
__statlstat32(const char* path, struct stat* st)
{
	static Call_t	call = { "__statlstat32", "__statlstat32" };

	return warp_lstat(&call, path, st);
}

#endif

#endif

static int
warp_stat(register Call_t* p, const char* path, struct stat* st)
{
	int	r;

	intercept(p);
	if ((r = (*(Lstat_f)p->call)(path, st)) != -1)
		warp_st(p, st);
	return r;
}

extern int
stat(const char* path, struct stat* st)
{
	static Call_t	call = { "stat", "stat" };

	return warp_stat(&call, path, st);
}

#ifndef __EXPORT__

extern int
_stat(const char* path, struct stat* st)
{
	static Call_t	call = { "stat", "_stat" };

	return warp_stat(&call, path, st);
}

extern int
__stat(const char* path, struct stat* st)
{
	static Call_t	call = { "stat", "__stat" };

	return warp_stat(&call, path, st);
}

extern int
_libc_stat(const char* path, struct stat* st)
{
	static Call_t	call = { "stat", "_libc_stat" };

	return warp_stat(&call, path, st);
}

extern int
__libc_stat(const char* path, struct stat* st)
{
	static Call_t	call = { "stat", "__libc_stat" };

	return warp_stat(&call, path, st);
}

#if _WARP_stat32

extern int
__stat32(const char* path, struct stat* st)
{
	static Call_t	call = { "__stat32", "__stat32" };

	return warp_stat(&call, path, st);
}

#endif

#endif

#endif

#if defined(_lib_stat64)

static void
warp_st64(register Call_t* p, register struct stat64* st)
{
	if (!p->warped)
	{
		if (p->level != state.level)
			p->warped = 1;
		else
		{
			WARP_ABS(st->st_atime);
			WARP_ABS(st->st_ctime);
			WARP_ABS(st->st_mtime);
		}
	}
}

static int
warp_fstat64(register Call_t* p, int fd, struct stat64* st)
{
	int	r;

	intercept(p);
	if ((r = (*(Fstat64_f)p->call)(fd, st)) != -1)
		warp_st64(p, st);
	return r;
}

extern int
fstat64(int fd, struct stat64* st)
{
	static Call_t	call = { "fstat64", "fstat64" };

	return warp_fstat64(&call, fd, st);
}

#ifndef __EXPORT__

extern int
_fstat64(int fd, struct stat64* st)
{
	static Call_t	call = { "fstat64", "_fstat64" };

	return warp_fstat64(&call, fd, st);
}

extern int
__fstat64(int fd, struct stat64* st)
{
	static Call_t	call = { "fstat64", "__fstat64" };

	return warp_fstat64(&call, fd, st);
}

extern int
_libc_fstat64(int fd, struct stat64* st)
{
	static Call_t	call = { "fstat64", "_libc_fstat64" };

	return warp_fstat64(&call, fd, st);
}

extern int
__libc_fstat64(int fd, struct stat64* st)
{
	static Call_t	call = { "fstat64", "__libc_fstat64" };

	return warp_fstat64(&call, fd, st);
}

#endif

static int
warp_lstat64(register Call_t* p, const char* path, struct stat64* st)
{
	int	r;

	intercept(p);
	if ((r = (*(Lstat64_f)p->call)(path, st)) != -1)
		warp_st64(p, st);
	return r;
}

extern int
lstat64(const char* path, struct stat64* st)
{
	static Call_t	call = { "lstat64", "lstat64" };

	return warp_lstat64(&call, path, st);
}

#ifndef __EXPORT__

extern int
_lstat64(const char* path, struct stat64* st)
{
	static Call_t	call = { "lstat64", "_lstat64" };

	return warp_lstat64(&call, path, st);
}

extern int
__lstat64(const char* path, struct stat64* st)
{
	static Call_t	call = { "lstat64", "__lstat64" };

	return warp_lstat64(&call, path, st);
}

extern int
_libc_lstat64(const char* path, struct stat64* st)
{
	static Call_t	call = { "lstat64", "_libc_lstat64" };

	return warp_lstat64(&call, path, st);
}

extern int
__libc_lstat64(const char* path, struct stat64* st)
{
	static Call_t	call = { "lstat64", "__libc_lstat64" };

	return warp_lstat64(&call, path, st);
}

#endif

static int
warp_stat64(register Call_t* p, const char* path, struct stat64* st)
{
	int	r;

	intercept(p);
	if ((r = (*(Stat64_f)p->call)(path, st)) != -1)
		warp_st64(p, st);
	return r;
}

extern int
stat64(const char* path, struct stat64* st)
{
	static Call_t	call = { "stat64", "stat64" };

	return warp_stat64(&call, path, st);
}

#ifndef __EXPORT__

extern int
_stat64(const char* path, struct stat64* st)
{
	static Call_t	call = { "stat64", "_stat64" };

	return warp_stat64(&call, path, st);
}

extern int
__stat64(const char* path, struct stat64* st)
{
	static Call_t	call = { "stat64", "__stat64" };

	return warp_stat64(&call, path, st);
}

extern int
_libc_stat64(const char* path, struct stat64* st)
{
	static Call_t	call = { "stat64", "_libc_stat64" };

	return warp_stat64(&call, path, st);
}

extern int
__libc_stat64(const char* path, struct stat64* st)
{
	static Call_t	call = { "stat64", "__libc_stat64" };

	return warp_stat64(&call, path, st);
}

#endif

#endif /* defined(_lib_stat64) */

#if _lib_clock_gettime && defined(CLOCK_REALTIME)

static int
warp_clock_gettime(register Call_t* p, int clock, struct timespec* tv)
{
	int	r;

	intercept(p);
	if ((r = (*(Clock_gettime_f)p->call)(clock, tv)) != -1 && !p->warped)
	{
		if (p->level != state.level)
			p->warped = 1;
		else if (tv)
			WARP_ABS(tv->tv_sec);
	}
	return r;
}

extern int
clock_gettime(int i, struct timespec* v)
{
	static Call_t	call = { "clock_gettime", "clock_gettime" };

	return warp_clock_gettime(&call, i, v);
}

#ifndef __EXPORT__

extern int
_clock_gettime(int i, struct timespec* v)
{
	static Call_t	call = { "clock_gettime", "_clock_gettime" };

	return warp_clock_gettime(&call, i, v);
}

extern int
__clock_gettime(int i, struct timespec* v)
{
	static Call_t	call = { "clock_gettime", "__clock_gettime" };

	return warp_clock_gettime(&call, i, v);
}

extern int
_libc_clock_gettime(int i, struct timespec* v)
{
	static Call_t	call = { "clock_gettime", "_libc_clock_gettime" };

	return warp_clock_gettime(&call, i, v);
}

extern int
__libc_clock_gettime(int i, struct timespec* v)
{
	static Call_t	call = { "clock_gettime", "__libc_clock_gettime" };

	return warp_clock_gettime(&call, i, v);
}

#endif

#endif /* _lib_clock_gettime */

#if _lib_getitimer

static int
warp_getitimer(register Call_t* p, int i, struct itimerval* v)
{
	int	r;

	intercept(p);
	if ((r = (*(Getitimer_f)p->call)(i, v)) != -1 && !p->warped && state.factor)
	{
		if (p->level != state.level)
			p->warped = 1;
		else
		{
			WARP_REL(v->it_interval.tv_sec);
			WARP_REL(v->it_value.tv_sec);
		}
	}
	return r;
}

extern int
getitimer(int i, struct itimerval* v)
{
	static Call_t	call = { "getitimer", "getitimer" };

	return warp_getitimer(&call, i, v);
}

#ifndef __EXPORT__

extern int
_getitimer(int i, struct itimerval* v)
{
	static Call_t	call = { "getitimer", "_getitimer" };

	return warp_getitimer(&call, i, v);
}

extern int
__getitimer(int i, struct itimerval* v)
{
	static Call_t	call = { "getitimer", "__getitimer" };

	return warp_getitimer(&call, i, v);
}

extern int
_libc_getitimer(int i, struct itimerval* v)
{
	static Call_t	call = { "getitimer", "_libc_getitimer" };

	return warp_getitimer(&call, i, v);
}

extern int
__libc_getitimer(int i, struct itimerval* v)
{
	static Call_t	call = { "getitimer", "__libc_getitimer" };

	return warp_getitimer(&call, i, v);
}

#endif

#endif /* _lib_getitimer */

#if _lib_setitimer

static int
warp_setitimer(register Call_t* p, int i, const struct itimerval* v, struct itimerval* o)
{
	int			r;
	struct itimerval	x;

	intercept(p);
	if (p->warped || !state.factor)
		return (*(Setitimer_f)p->call)(i, v, o);
	x = *v;
	if (x.it_interval.tv_sec)
	{
		UNWARP_REL(x.it_interval.tv_sec);
		if (x.it_interval.tv_sec)
			x.it_interval.tv_usec = 0;
		else if (!(x.it_interval.tv_usec /= state.factor))
			x.it_interval.tv_usec = 10;
	}
	if (x.it_value.tv_sec)
	{
		UNWARP_REL(x.it_value.tv_sec);
		if (x.it_value.tv_sec)
			x.it_value.tv_usec = 0;
		else if (!(x.it_value.tv_usec /= state.factor))
			x.it_value.tv_usec = 10;
	}
	if ((r = (*(Setitimer_f)p->call)(i, &x, o)) != -1 && !p->warped)
	{
		if (p->level != state.level)
			p->warped = 1;
		else if (o)
		{
			WARP_REL(o->it_interval.tv_sec);
			WARP_REL(o->it_value.tv_sec);
		}
	}
	return r;
}

extern int
setitimer(int i, const struct itimerval* v, struct itimerval* o)
{
	static Call_t	call = { "setitimer", "setitimer" };

	return warp_setitimer(&call, i, v, o);
}

#ifndef __EXPORT__

extern int
_setitimer(int i, const struct itimerval* v, struct itimerval* o)
{
	static Call_t	call = { "setitimer", "_setitimer" };

	return warp_setitimer(&call, i, v, o);
}

extern int
__setitimer(int i, const struct itimerval* v, struct itimerval* o)
{
	static Call_t	call = { "setitimer", "__setitimer" };

	return warp_setitimer(&call, i, v, o);
}

extern int
_libc_setitimer(int i, const struct itimerval* v, struct itimerval* o)
{
	static Call_t	call = { "setitimer", "_libc_setitimer" };

	return warp_setitimer(&call, i, v, o);
}

extern int
__libc_setitimer(int i, const struct itimerval* v, struct itimerval* o)
{
	static Call_t	call = { "setitimer", "__libc_setitimer" };

	return warp_setitimer(&call, i, v, o);
}

#endif

#endif /* _lib_setitimer */

#ifndef __EXPORT__

static char**
warp_env(char* const arge[])
{
	register char**	e;
	register char**	x;
	register char*	s;
	register char*	t;
	register char*	u;
	char**		z;
	char**		env;
	int		n;

	env = (char**)arge;
	if (*state.env)
	{
		if (e = env)
		{
			while (s = *e++)
			{
				x = state.env;
				while (t = *x++)
				{
					u = s;
					while (*t++ == *u++)
						if (!*t)
							goto found;
				}
				goto missing;
			found:
				;
			}
		}
		else
			s = *state.env;
	missing:
		if (s)
		{
			if (state.trace)
			{
				s = "warp: execve env missing\n";
				write(2, s, strlen(s));
			}
			if (e = env)
			{
				while (*e++);
				n = e - env;
			}
			else
				n = 1;
			e = state.env;
			while (*e++);
			n += (e - state.env) - 1;
			if (!(z = (char**)malloc(n * sizeof(char**))))
			{
				s = "warp: execve env malloc error\n";
				write(2, s, strlen(s));
				_exit(125);
			}
			e = env;
			env = z;
			x = state.env;
			while (*z = *x++)
				z++;
			if (e)
				while (*z++ = *e++);
			if (state.trace)
			{
				z = env;
				while (t = *z++)
				{
					s = "warp: execve new env ";
					write(2, s, strlen(s));
					write(2, t, strlen(t));
					write(2, "\n", 1);
				}
			}
		}
		else if (state.trace)
		{
			s = "warp: execve env ok\n";
			write(2, s, strlen(s));
		}
	}
	else if (state.trace)
	{
		s = "warp: execve skip check\n";
		write(2, s, strlen(s));
	}
	return env;
}

static int
warp_execve(register Call_t* p, const char* path, char* const argv[], char* const arge[])
{
	int		n;
	char**		env;

	static int	level;

	intercept(p);
	env = (!p->warped && !level) ? warp_env(arge) : (char**)arge;
	p->warped++;
	level++;
	n = (*(Execve_f)p->call)(path, argv, env);
	level--;
	p->warped--;
	return n;
}

static int
warp_execvpe(register Call_t* p, const char* path, char* const argv[], char* const arge[])
{
	int		n;
	char**		env;

	static int	level;

	intercept(p);
	env = (!p->warped && !level) ? warp_env(arge) : (char**)arge;
	p->warped++;
	level++;
	n = (*(Execvpe_f)p->call)(path, argv, env);
	level--;
	p->warped--;
	return n;
}

extern int
execve(const char* path, char* const argv[], char* const arge[])
{
	static Call_t	call = { "execve", "execve" };

	return warp_execve(&call, path, argv, arge);
}

extern int
_execve(const char* path, char* const argv[], char* const arge[])
{
	static Call_t	call = { "execve", "_execve" };

	return warp_execve(&call, path, argv, arge);
}

extern int
__execve(const char* path, char* const argv[], char* const arge[])
{
	static Call_t	call = { "execve", "__execve" };

	return warp_execve(&call, path, argv, arge);
}

extern int
_libc_execve(const char* path, char* const argv[], char* const arge[])
{
	static Call_t	call = { "execve", "_libc_execve" };

	return warp_execve(&call, path, argv, arge);
}

extern int
__libc_execve(const char* path, char* const argv[], char* const arge[])
{
	static Call_t	call = { "execve", "__libc_execve" };

	return warp_execve(&call, path, argv, arge);
}

extern int
execv(const char* path, char* const argv[])
{
	static Call_t	call = { "execv", "execv" };

	return warp_execve(&call, path, argv, environ);
}

extern int
_execv(const char* path, char* const argv[])
{
	static Call_t	call = { "execv", "_execv" };

	return warp_execve(&call, path, argv, environ);
}

extern int
__execv(const char* path, char* const argv[])
{
	static Call_t	call = { "execv", "__execv" };

	return warp_execve(&call, path, argv, environ);
}

extern int
_libc_execv(const char* path, char* const argv[])
{
	static Call_t	call = { "execv", "_libc_execv" };

	return warp_execve(&call, path, argv, environ);
}

extern int
__libc_execv(const char* path, char* const argv[])
{
	static Call_t	call = { "execv", "__libc_execv" };

	return warp_execve(&call, path, argv, environ);
}

extern int
execvp(const char* path, char* const argv[])
{
	static Call_t	call = { "execvp", "execvp" };

	return warp_execvpe(&call, path, argv, environ);
}

extern int
_execvp(const char* path, char* const argv[])
{
	static Call_t	call = { "execvp", "_execvp" };

	return warp_execvpe(&call, path, argv, environ);
}

extern int
__execvp(const char* path, char* const argv[])
{
	static Call_t	call = { "execvp", "__execvp" };

	return warp_execvpe(&call, path, argv, environ);
}

extern int
_libc_execvp(const char* path, char* const argv[])
{
	static Call_t	call = { "execvp", "_libc_execvp" };

	return warp_execvpe(&call, path, argv, environ);
}

extern int
__libc_execvp(const char* path, char* const argv[])
{
	static Call_t	call = { "execvp", "__libc_execvp" };

	return warp_execvpe(&call, path, argv, environ);
}

extern int
execvpe(const char* path, char* const argv[], char* const arge[])
{
	static Call_t	call = { "execvpe", "execvpe" };

	return warp_execvpe(&call, path, argv, arge);
}

extern int
_execvpe(const char* path, char* const argv[], char* const arge[])
{
	static Call_t	call = { "execvpe", "_execvpe" };

	return warp_execvpe(&call, path, argv, arge);
}

extern int
__execvpe(const char* path, char* const argv[], char* const arge[])
{
	static Call_t	call = { "execvpe", "__execvpe" };

	return warp_execvpe(&call, path, argv, arge);
}

extern int
_libc_execvpe(const char* path, char* const argv[], char* const arge[])
{
	static Call_t	call = { "execvpe", "_libc_execvpe" };

	return warp_execvpe(&call, path, argv, arge);
}

extern int
__libc_execvpe(const char* path, char* const argv[], char* const arge[])
{
	static Call_t	call = { "execvpe", "__libc_execvpe" };

	return warp_execvpe(&call, path, argv, arge);
}

extern int
execl(const char* path, const char* arg, ...)
{
	static Call_t	call = { "execl", "execl" };

	return warp_execve(&call, path, (char* const*)&arg, environ);
}

extern int
_execl(const char* path, const char* arg, ...)
{
	static Call_t	call = { "execl", "_execl" };

	return warp_execve(&call, path, (char* const*)&arg, environ);
}

extern int
__execl(const char* path, const char* arg, ...)
{
	static Call_t	call = { "execl", "__execl" };

	return warp_execve(&call, path, (char* const*)&arg, environ);
}

extern int
_libc_execl(const char* path, const char* arg, ...)
{
	static Call_t	call = { "execl", "_libc_execl" };

	return warp_execve(&call, path, (char* const*)&arg, environ);
}

extern int
__libc_execl(const char* path, const char* arg, ...)
{
	static Call_t	call = { "execl", "__libc_execl" };

	return warp_execve(&call, path, (char* const*)&arg, environ);
}

extern int
execlp(const char* path, const char* arg, ...)
{
	static Call_t	call = { "execlp", "execlp" };

	return warp_execvpe(&call, path, (char* const*)&arg, environ);
}

extern int
_execlp(const char* path, const char* arg, ...)
{
	static Call_t	call = { "execlp", "_execlp" };

	return warp_execvpe(&call, path, (char* const*)&arg, environ);
}

extern int
__execlp(const char* path, const char* arg, ...)
{
	static Call_t	call = { "execlp", "__execlp" };

	return warp_execvpe(&call, path, (char* const*)&arg, environ);
}

extern int
_libc_execlp(const char* path, const char* arg, ...)
{
	static Call_t	call = { "execlp", "_libc_execlp" };

	return warp_execvpe(&call, path, (char* const*)&arg, environ);
}

extern int
__libc_execlp(const char* path, const char* arg, ...)
{
	static Call_t	call = { "execlp", "__libc_execlp" };

	return warp_execvpe(&call, path, (char* const*)&arg, environ);
}

#endif
