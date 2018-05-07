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

#include "sfdchdr.h"

/*	Discipline to invoke UNIX processes as data filters.
**	These processes must be able to fit in pipelines.
**
**	Written by Kiem-Phong Vo, kpv@research.att.com, 03/18/1998.
*/

typedef struct _filter_s {
    Sfdisc_t disc;  /* discipline structure	*/
    Sfio_t *filter; /* the filter stream	*/
    char *next;     /* data unwritten 	*/
    char *endb;     /* end of data		*/
    char raw[4096]; /* raw data buffer	*/
} Filter_t;

/* read data from the filter */
static ssize_t filterread(Sfio_t *f, void *buf, size_t n, Sfdisc_t *disc) {
    Filter_t *fi;
    ssize_t r, w;

    fi = (Filter_t *)disc;
    for (;;) {
        /* get some raw data to stuff down the pipe */
        if (fi->next && fi->next >= fi->endb) {
            if ((r = sfrd(f, fi->raw, sizeof(fi->raw), disc)) > 0) {
                fi->next = fi->raw;
                fi->endb = fi->raw + r;
            } else { /* eof, close write end of pipes */
                sfset(fi->filter, SF_READ, 0);
                close(sffileno(fi->filter));
                sfset(fi->filter, SF_READ, 1);
                fi->next = fi->endb = NULL;
            }
        }

        if (fi->next && (w = fi->endb - fi->next) > 0) { /* see if pipe is ready for write */
            sfset(fi->filter, SF_READ, 0);
            r = sfpoll(&fi->filter, 1, 1);
            sfset(fi->filter, SF_READ, 1);

            if (r == 1) /* non-blocking write */
            {
                errno = 0;
                if ((w = sfwr(fi->filter, fi->next, w, 0)) > 0)
                    fi->next += w;
                else if (errno != EAGAIN)
                    return 0;
            }
        }

        /* see if pipe is ready for read */
        sfset(fi->filter, SF_WRITE, 0);
        w = sfpoll(&fi->filter, 1, fi->next ? 1 : -1);
        sfset(fi->filter, SF_WRITE, 1);

        if (!fi->next || w == 1) /* non-blocking read */
        {
            errno = 0;
            if ((r = sfrd(fi->filter, buf, n, 0)) > 0) return r;
            if (errno != EAGAIN) return 0;
        }
    }
}

static ssize_t filterwrite(Sfio_t *f, const void *buf, size_t n, Sfdisc_t *disc) { return -1; }

/* for the duration of this discipline, the stream is unseekable */
static Sfoff_t filterseek(Sfio_t *f, Sfoff_t addr, int offset, Sfdisc_t *disc) {
    f = NULL;
    addr = 0;
    offset = 0;
    disc = NULL;
    return (Sfoff_t)(-1);
}

/* on close, remove the discipline */
static int filterexcept(Sfio_t *f, int type, void *data, Sfdisc_t *disc) {
    if (type == SF_FINAL || type == SF_DPOP) {
        sfclose(((Filter_t *)disc)->filter);
        free(disc);
    }

    return 0;
}

int sfdcfilter(Sfio_t *f, const char *cmd) {
    reg Filter_t *fi;
    reg Sfio_t *filter;

    /* open filter for read&write */
    if (!(filter = sfpopen(NULL, cmd, "r+"))) return -1;

    /* unbuffered stream */
    sfsetbuf(filter, NULL, 0);

    if (!(fi = (Filter_t *)malloc(sizeof(Filter_t)))) {
        sfclose(filter);
        return -1;
    }

    fi->disc.readf = filterread;
    fi->disc.writef = filterwrite;
    fi->disc.seekf = filterseek;
    fi->disc.exceptf = filterexcept;
    fi->filter = filter;
    fi->next = fi->endb = fi->raw;

    if (sfdisc(f, (Sfdisc_t *)fi) != (Sfdisc_t *)fi) {
        sfclose(filter);
        free(fi);
        return -1;
    }

    return 0;
}
