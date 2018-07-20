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

#include <stdlib.h>
#include <sys/types.h>

#include "sfdisc.h"
#include "sfio.h"

/*	A discipline to tee the output to a stream to another stream.
**	This is similar to what the "tee" program does. As implemented
**	this discipline only works with file streams.
**
**	Written by Kiem-Phong Vo, kpv@research.att.com, 03/18/1998.
*/

/* the discipline structure for tee-ing */
typedef struct _tee_s {
    Sfdisc_t disc; /* the sfio discipline structure */
    Sfio_t *tee;   /* the stream to tee to */
    int status;    /* if tee stream is still ok */
} Tee_t;

/*	write to the teed stream.  */
static ssize_t teewrite(Sfio_t *f, const void *buf, size_t size, Sfdisc_t *disc) {
    Tee_t *te = (Tee_t *)disc;

    /* tee data if still ok */
    if (te->status == 0 && sfwrite(te->tee, buf, size) != (ssize_t)size) te->status = -1;

    /* do the actual write */
    return sfwr(f, buf, size, disc);
}

/* on close, remove the discipline */
static int teeexcept(Sfio_t *f, int type, void *data, Sfdisc_t *disc) {
    UNUSED(f);
    UNUSED(data);

    if (type == SF_FINAL || type == SF_DPOP) free(disc);

    return 0;
}

int sfdctee(Sfio_t *f, Sfio_t *tee) {
    Tee_t *te;

    if (!(te = (Tee_t *)malloc(sizeof(Tee_t)))) return -1;

    te->disc.readf = NULL;
    te->disc.seekf = NULL;
    te->disc.writef = teewrite;
    te->disc.exceptf = teeexcept;
    te->tee = tee;
    te->status = 0;

    if (sfdisc(f, (Sfdisc_t *)te) != (Sfdisc_t *)te) {
        free(te);
        return -1;
    }

    return 0;
}
