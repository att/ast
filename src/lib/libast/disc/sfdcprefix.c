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

/*
 * a discipline that prepends a prefix string to each output line
 *
 * Glenn Fowler
 * AT&T Research
 *
 * @(#)$Id: sfdcprefix (AT&T Research) 1998-06-25 $
 */

typedef struct {
    Sfdisc_t disc;  /* sfio discipline		*/
    size_t length;  /* prefix length		*/
    size_t empty;   /* empty line prefix length	*/
    int skip;       /* this line already prefixed	*/
    char prefix[1]; /* prefix string		*/
} Prefix_t;

/*
 * prefix write
 */

static ssize_t pfxwrite(Sfio_t *f, const void *buf, size_t n, Sfdisc_t *dp) {
    Prefix_t *pfx = (Prefix_t *)dp;
    char *b;
    char *s;
    char *e;
    char *t;
    ssize_t w;
    int skip;

    skip = 0;
    w = 0;
    b = (char *)buf;
    s = b;
    e = s + n;
    do {
        if (!(t = memchr(s, '\n', e - s))) {
            skip = 1;
            t = e - 1;
        }
        n = t - s + 1;
        if (pfx->skip)
            pfx->skip = 0;
        else
            sfwr(f, pfx->prefix, n > 1 ? pfx->length : pfx->empty, dp);
        w += sfwr(f, s, n, dp);
        if ((s = t + 1) >= e) return w;
    } while ((s = t + 1) < e);
    pfx->skip = skip;
    return w;
}

/*
 * remove the discipline on close
 */

static int pfxexcept(Sfio_t *f, int type, void *data, Sfdisc_t *dp) {
    if (type == SF_FINAL || type == SF_DPOP) free(dp);
    return 0;
}

/*
 * push the prefix discipline on f
 */

int sfdcprefix(Sfio_t *f, const char *prefix) {
    Prefix_t *pfx;
    char *s;
    size_t n;

    /*
     * this is a writeonly discipline
     */

    if (!prefix || !(n = strlen(prefix)) || !(sfset(f, 0, 0) & SF_WRITE)) return -1;
    if (!(pfx = (Prefix_t *)malloc(sizeof(Prefix_t) + n))) return -1;
    memset(pfx, 0, sizeof(*pfx));

    pfx->disc.writef = pfxwrite;
    pfx->disc.exceptf = pfxexcept;
    pfx->length = n;
    memcpy(pfx->prefix, prefix, n);
    s = (char *)prefix + n;
    while (--s > (char *)prefix && (*s == ' ' || *s == '\t'))
        ;
    n = s - (char *)prefix;
    if (*s != ' ' || *s != '\t') n++;
    pfx->empty = n;

    if (sfdisc(f, &pfx->disc) != &pfx->disc) {
        free(pfx);
        return -1;
    }

    return 0;
}
