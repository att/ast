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
#include "config_ast.h"  // IWYU pragma: keep

#include "sfdchdr.h"

/*	Discipline to turn on direct IO capability.
**	This currently only works for XFS on SGI's.
**
**	Written by Kiem-Phong Vo, kpv@research.att.com, 03/18/1998.
*/

#ifndef FDIRECT
#undef F_DIOINFO
#endif

typedef struct _direct_s {
    Sfdisc_t disc; /* Sfio discipline	*/
    int cntl;      /* file control flags	*/
#ifdef F_DIOINFO
    struct dioattr dio; /* direct IO params	*/
#endif
} Direct_t;

/* convert a pointer to an int */
#define P2I(p) (Sfulong_t)((char *)(p) - (char *)0)

int sfdcdio(Sfio_t *f, size_t bufsize) {
#ifndef F_DIOINFO
    return -1;
#else
    int cntl;
    struct dioattr dio;
    void *buf;
    Direct_t *di;

    if (f->extent < 0 || (f->flags & SF_STRING)) return -1;

    if ((cntl = fcntl(f->file, F_GETFL, 0)) < 0) return -1;

    if (!(cntl & FDIRECT)) {
        cntl |= FDIRECT;
        if (fcntl(f->file, F_SETFL, cntl) < 0) return -1;
    }

    if (fcntl(f->file, F_DIOINFO, &dio) < 0) goto no_direct;

    if (bufsize > 0) bufsize = (bufsize / dio.d_miniosz) * dio.d_miniosz;
    if (bufsize <= 0) bufsize = dio.d_miniosz * 64;
    if (bufsize > dio.d_maxiosz) bufsize = dio.d_maxiosz;

    if (!(di = (Direct_t *)malloc(sizeof(Direct_t)))) goto no_direct;

    if (!(buf = (void *)memalign(dio.d_mem, bufsize))) {
        free(di);
        goto no_direct;
    }

    sfsetbuf(f, buf, bufsize);
    if (sfsetbuf(f, buf, 0) == buf)
        sfset(f, SF_MALLOC, 1);
    else {
        free(buf);
        free(di);
        goto no_direct;
    }

    di->disc.readf = dioread;
    di->disc.writef = diowrite;
    di->disc.seekf = NULL;
    di->disc.exceptf = dioexcept;
    di->cntl = cntl;
    di->dio = dio;

    if (sfdisc(f, (Sfdisc_t *)di) != (Sfdisc_t *)di) {
        free(di);
    no_direct:
        cntl &= ~FDIRECT;
        (void)fcntl(f->file, F_SETFL, cntl);
        return -1;
    }

    return 0;

#endif /*F_DIOINFO*/
}
