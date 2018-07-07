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

#include "tv.h"

#if _lib_clock_gettime && defined(CLOCK_REALTIME)

int tvsettime(const Tv_t *tv) {
    struct timespec s;

    s.tv_sec = tv->tv_sec;
    s.tv_nsec = tv->tv_nsec;
    return clock_settime(CLOCK_REALTIME, &s);
}

#elif _lib_gettimeofday

#include <sys/time.h>

int tvsettime(const Tv_t *tv) {
    struct timeval v;

    v.tv_sec = tv->tv_sec;
    v.tv_usec = tv->tv_nsec / 1000;
    return settimeofday(&v, NULL);
}

#else

int tvsettime(const Tv_t *tv) {
    errno = EPERM;
    return -1;
}

#endif
