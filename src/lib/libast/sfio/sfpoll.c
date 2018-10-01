/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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

#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#include "sfhdr.h"

/*	Poll a set of streams to see if any is available for I/O.
**	Ready streams are moved to front of array but retain the
**	same relative order.
**
**	Written by Kiem-Phong Vo.
*/

int sfpoll(Sfio_t **fa, int n, int tm) {
    int r, c, m, np, eintr;
    Sfio_t *f;
    int *check;

    if (n <= 0 || !fa) return -1;

    int *status = malloc(2 * n * sizeof(int));
    if (!status) return -1;
    check = status + n;  // streams that need polling

    // A SF_READ stream is ready if there is buffered read data.
#define RDREADY(f)                                 \
    (((f->mode & SF_READ) && f->next < f->endb) || \
     ((f->mode & SF_WRITE) && f->proc && f->proc->ndata > 0))

    /* a SF_WRITE stream is ready if there is no write data */
#define WRREADY(f) (!(f->mode & SF_WRITE) || f->next == f->data)

#define HASAUXFD(f) (f->proc && f->proc->file >= 0 && f->proc->file != f->file)

    for (r = c = eintr = 0; r < n; ++r) /* compute streams that must be checked */
    {
        f = fa[r];
        status[r] = 0;

        /* terminate poll on interrupt? */
        if (f->flags & SF_IOINTR) eintr++;
        /* check accessibility */
        m = f->mode & SF_RDWR;
        if ((int)f->mode != m && _sfmode(f, m, 0) < 0) continue;

        if ((f->flags & SF_READ) && RDREADY(f)) status[r] |= SF_READ;

        if ((f->flags & SF_WRITE) && WRREADY(f)) status[r] |= SF_WRITE;

        if ((f->flags & SF_RDWR) == status[r]) continue;

        /* has discipline, ask its opinion */
        if (f->disc && f->disc->exceptf) {
            if ((m = (*f->disc->exceptf)(f, SF_DPOLL, &tm, f->disc)) < 0)
                continue;
            else if (m > 0) {
                status[r] = m & SF_RDWR;
                continue;
            }
        }

        if (f->extent < 0) /* unseekable stream, must poll/select */
            check[c++] = r;
        else /* seekable streams are always ready */
        {
            if (f->flags & SF_READ) status[r] |= SF_READ;
            if (f->flags & SF_WRITE) status[r] |= SF_WRITE;
        }
    }
    /* terminate poll on interrupt only if all streams marked SF_IOINTR */
    eintr = eintr == n ? -1 : EINTR;

    np = -1;
    if (c > 0) {
        struct pollfd *fds;

        /* construct the poll array */
        for (m = 0, r = 0; r < c; ++r, ++m) {
            f = fa[check[r]];
            if (HASAUXFD(f)) m += 1;
        }
        fds = malloc(m * sizeof(struct pollfd));
        if (!fds) {
            free(status);
            return -1;
        }

        for (m = 0, r = 0; r < c; ++r, ++m) {
            f = fa[check[r]];

            fds[m].fd = f->file;
            fds[m].events = fds[m].revents = 0;

            if ((f->flags & SF_WRITE) && !WRREADY(f)) fds[m].events |= POLLOUT;

            if ((f->flags & SF_READ) &&
                !RDREADY(f)) { /* a sfpopen situation with two file descriptors */
                if ((f->mode & SF_WRITE) && HASAUXFD(f)) {
                    m += 1;
                    fds[m].fd = f->proc->file;
                    fds[m].revents = 0;
                }

                fds[m].events |= POLLIN;
            }
        }

        while ((np = SFPOLL(fds, m, tm)) < 0) {
            if (errno == eintr || errno == EAGAIN)
                errno = 0;
            else
                goto report;
        }
        if (np > 0) /* poll succeeded */
            np = c;

        for (m = 0, r = 0; r < np; ++r, ++m) {
            f = fa[check[r]];

            if ((f->flags & SF_WRITE) && !WRREADY(f)) {
                if (fds[m].revents & (POLLOUT | POLLHUP | POLLERR)) status[check[r]] |= SF_WRITE;
            }

            if ((f->flags & SF_READ) && !RDREADY(f)) {
                if ((f->mode & SF_WRITE) && HASAUXFD(f)) m += 1;
                if (fds[m].revents & (POLLIN | POLLHUP | POLLERR)) status[check[r]] |= SF_READ;
            }
        }

        free(fds);
    }

    if (np < 0 && c > 0) {
        fd_set rd, wr;
        struct timeval tmb, *tmp;

        FD_ZERO(&rd);
        FD_ZERO(&wr);
        m = 0;
        for (r = 0; r < c; ++r) {
            f = fa[check[r]];

            if (f->file > m) m = f->file;

            if ((f->flags & SF_WRITE) && !WRREADY(f)) FD_SET(f->file, &wr);

            if ((f->flags & SF_READ) && !RDREADY(f)) {
                if ((f->mode & SF_WRITE) && HASAUXFD(f)) {
                    if (f->proc->file > m) m = f->proc->file;
                    FD_SET(f->proc->file, &rd);
                } else
                    FD_SET(f->file, &rd);
            }
        }
        if (tm < 0)
            tmp = NULL;
        else {
            tmp = &tmb;
            tmb.tv_sec = tm / 1000;
            tmb.tv_usec = (tm % 1000) * 1000;
        }

        while ((np = select(m + 1, &rd, &wr, NULL, tmp)) < 0) {
            if (errno == eintr)
                errno = 0;
            else
                goto report;
        }
        if (np > 0) np = c;

        for (r = 0; r < np; ++r) {
            f = fa[check[r]];

            if ((f->flags & SF_WRITE) && !WRREADY(f)) {
                if (FD_ISSET(f->file, &wr)) status[check[r]] |= SF_WRITE;
            }

            if ((f->flags & SF_READ) && !RDREADY(f)) {
                if ((f->mode & SF_WRITE) && HASAUXFD(f)) {
                    if (FD_ISSET(f->proc->file, &rd)) status[check[r]] |= SF_READ;
                } else {
                    if (FD_ISSET(f->file, &rd)) status[check[r]] |= SF_READ;
                }
            }
        }
    }

report:
    for (r = c = 0; c < n; ++c) {
        if (status[c] == 0) continue;

        f = fa[c];
        f->val = (ssize_t)status[c];

        /* announce status */
        if (f->disc && f->disc->exceptf)
            (*f->disc->exceptf)(f, SF_READY, (void *)(long)status[c], f->disc);

        if (c > r) /* move to front of list */
        {
            fa[c] = fa[r];
            fa[r] = f;
        }
        r += 1;
    }

    free(status);
    return r ? r : np < 0 ? -1 : 0;
}
