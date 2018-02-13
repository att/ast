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
#include "config_ast.h"  // IWYU pragma: keep

#include <time.h>

#include "tm.h"
#include "tv.h"

#if _lib_clock_gettime && defined(CLOCK_REALTIME)

int tvgettime(Tv_t *tv) {
    struct timespec s;

    clock_gettime(CLOCK_REALTIME, &s);
    tv->tv_sec = s.tv_sec;
    tv->tv_nsec = s.tv_nsec;
    return 0;
}

#elif _lib_gettimeofday

#include <sys/time.h>

int tvgettime(Tv_t *tv) {
    struct timeval v;

    gettimeofday(&v, NULL);
    tv->tv_sec = v.tv_sec;
    tv->tv_nsec = v.tv_usec * 1000;
    return 0;
}

#else

int tvgettime(Tv_t *tv) {
    time_t s = 0;
    uint32_t n = 0;

    tv->tv_sec = time(NULL);
    if (tv->tv_sec != s) {
        s = tv->tv_sec;
        n = 0;
    }
    else {
        n += 1000;
    }
    tv->tv_nsec = n;
    return 0;
}

#endif
