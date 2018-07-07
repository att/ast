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

void tvgettime(Tv_t *tv) {
    struct timespec s;

    clock_gettime(CLOCK_REALTIME, &s);
    tv->tv_sec = s.tv_sec;
    tv->tv_nsec = s.tv_nsec;
}

#elif _lib_gettimeofday

#include <sys/time.h>

void tvgettime(Tv_t *tv) {
    struct timeval v;

    gettimeofday(&v, NULL);
    tv->tv_sec = v.tv_sec;
    tv->tv_nsec = v.tv_usec * 1000;
}

#else

void tvgettime(Tv_t *tv) {
    tv->tv_sec = time(NULL);
    tv->tv_nsec = 0;
}

#endif
