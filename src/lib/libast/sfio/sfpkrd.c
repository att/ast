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

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

/*      Read/Peek a record from an unseekable device
**
**      Written by Kiem-Phong Vo.
*/

#define SOCKET_PEEK (1 << 0)
#if _stream_peek
#define STREAM_PEEK (1 << 1)
#else  // _stream_peek
#define STREAM_PEEK 0
#endif  // _stream_peek

#if _lib_poll  // platform appears to have a working poll() implementation
#include <poll.h>

// Use poll() to detect if input is available or we're at EOF.
static_fn int _sfpkrd_poll(int fd, long tm) {
    struct pollfd po;
    po.fd = fd;
    po.events = POLLIN;
    po.revents = 0;

    while (true) {
        int r = poll(&po, 1, tm);
        if (r != -1) return (po.revents & POLLIN) ? 1 : -1;
        if (errno == EINTR) return -2;
        if (errno != EAGAIN) abort();  // can't happen unless something is horribly wrong
    }
}

#else  // _lib_poll

// Use select() to detect if input is available or we're at EOF.
static_fn int _sfpkrd_poll(int fd, long tm) {
    while (true) {
        struct timeval tmb, *tmp;
        if (tm < 0) {
            tmp = NULL;
        } else {
            tmb.tv_sec = tm / 1000;
            tmb.tv_usec = (tm % 1000) * 1000;
            tmp = &tmb;
        }
        fd_set rd;
        FD_ZERO(&rd);
        FD_SET(fd, &rd);
        int r = select(fd + 1, &rd, NULL, NULL, tmp);
        if (r != -1) return FD_ISSET(fd, &rd) ? 1 : -1;
        if (errno == EINTR) return -2;
        if (errno != EAGAIN) abort();  // can't happen unless something is horribly wrong
    }
}

#endif  // _lib_poll

//
// Args:
// fd       file descriptor
// argbuf   buffer to read data
// n        buffer size
// rc       record character
// tm       time-out
// action   >0: peeking, if rc>=0, get action records,
//          <0: no peeking, if rc>=0, get -action records,
//          =0: no peeking, if rc>=0, must get a single record
//
ssize_t sfpkrd(int fd, void *argbuf, size_t n, int rc, long tm, int action) {
    ssize_t r;
    int ntry, t;
    char *buf = (char *)argbuf, *endbuf;

    if (rc < 0 && tm < 0 && action <= 0) return read(fd, buf, n);

    t = (action > 0 || rc >= 0) ? (STREAM_PEEK | SOCKET_PEEK) : 0;

    for (ntry = 0; ntry < 2; ++ntry) {
        r = -1;

#if _stream_peek
        if ((t & STREAM_PEEK) && (ntry == 1 || tm < 0)) {
            struct strpeek pbuf;
            pbuf.flags = 0;
            pbuf.ctlbuf.maxlen = -1;
            pbuf.ctlbuf.len = 0;
            pbuf.ctlbuf.buf = NULL;
            pbuf.databuf.maxlen = n;
            pbuf.databuf.buf = buf;
            pbuf.databuf.len = 0;

            if ((r = ioctl(fd, I_PEEK, &pbuf)) < 0) {
                if (errno == EINTR) return -1;
                t &= ~STREAM_PEEK;
            } else {
                t &= ~SOCKET_PEEK;
                if (r > 0 && (r = pbuf.databuf.len) <= 0) {
                    if (action <= 0) /* read past eof */
                        r = read(fd, buf, 1);
                    return r;
                }
                if (r == 0)
                    r = -1;
                else if (r > 0)
                    break;
            }
        }
#endif /* stream_peek */

        if (ntry == 1) break;

        // Poll or select to see if data is present.
        if (tm >= 0 || action > 0 ||
            // Block until there is data before peeking again.
            ((t & STREAM_PEEK) && rc >= 0) ||
            // Let select be interrupted instead of recv which autoresumes.
            (t & SOCKET_PEEK)) {
            r = _sfpkrd_poll(fd, tm);
            if (r == -2) return -1;             // EINTR
            if (r == -1 && tm >= 0) return -1;  // timeout exceeded
            if (r > 0) {                        // there is data now
                if (action <= 0 && rc < 0) return read(fd, buf, n);
            }
            r = -1;
        }

        if (!(t & SOCKET_PEEK)) continue;

        while ((t & SOCKET_PEEK) && (r = recv(fd, (char *)buf, n, MSG_PEEK)) < 0) {
            if (errno == EINTR) {
                return -1;
            } else if (errno == EAGAIN) {
                errno = 0;
            } else {
                t &= ~SOCKET_PEEK;
            }
        }
        if (r >= 0) {
            if (r > 0) {
                break;
            } else /* read past eof */
            {
                if (action <= 0) r = read(fd, buf, 1);
                return r;
            }
        }
    }

    if (r < 0) {
        if (tm >= 0 || action > 0) return -1;
        // Get here means: tm < 0 && action <= 0 && rc >= 0.
        // Number of records read at a time.
        if ((action = action ? -action : 1) > (int)n) action = n;
        r = 0;
        while ((t = read(fd, buf, action)) > 0) {
            r += t;
            for (endbuf = buf + t; buf < endbuf;) {
                if (*buf++ == rc) action -= 1;
            }
            if (action == 0 || (int)(n - r) < action) break;
        }
        return r == 0 ? t : r;
    }

    /* successful peek, find the record end */
    if (rc >= 0) {
        char *sp;

        t = action == 0 ? 1 : action < 0 ? -action : action;
        for (endbuf = (sp = buf) + r; sp < endbuf;) {
            if (*sp++ == rc) {
                if ((t -= 1) == 0) break;
            }
        }
        r = sp - buf;
    }

    /* advance */
    if (action <= 0) r = read(fd, buf, r);

    return r;
}
