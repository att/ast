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
/*
 * AT&T Research
 * return terminal rows and cols
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <fcntl.h>      // IWYU pragma: keep
#include <stdlib.h>     // IWYU pragma: keep
#include <sys/ioctl.h>  // IWYU pragma: keep
#include <termios.h>    // IWYU pragma: keep
#include <unistd.h>     // IWYU pragma: keep

#if _hdr_stream && _hdr_ptem
#include <sys/ptem.h>
#include <sys/stream.h>
#endif

static_fn int ttctl(int, int, void *);

void astwinsize(int fd, int *rows, int *cols) {
#ifdef TIOCGWINSZ
#define NEED_ttctl
    struct winsize ws;

    if (!ttctl(fd, TIOCGWINSZ, &ws) && ws.ws_col > 0 && ws.ws_row > 0) {
        if (rows) *rows = ws.ws_row;
        if (cols) *cols = ws.ws_col;
    } else
#else
#ifdef TIOCGSIZE
#define NEED_ttctl
    struct ttysize ts;

    if (!ttctl(fd, TIOCGSIZE, &ts) && ts.ts_lines > 0 && ts.ts_cols > 0) {
        if (rows) *rows = ts.ts_lines;
        if (cols) *cols = ts.ts_cols;
    } else
#else
#ifdef JWINSIZE
#define NEED_ttctl
    struct winsize ws;

    if (!ttctl(fd, JWINSIZE, &ws) && ws.bytesx > 0 && ws.bytesy > 0) {
        if (rows) *rows = ws.bytesy;
        if (cols) *cols = ws.bytesx;
    } else
#endif
#endif
#endif
    {
        char *s;

        if (rows) *rows = (s = getenv("LINES")) ? strtol(s, NULL, 0) : 0;
        if (cols) *cols = (s = getenv("COLUMNS")) ? strtol(s, NULL, 0) : 0;
    }
}

#ifdef NEED_ttctl

//
// tty ioctl() -- no cache
//
static_fn int ttctl(int fd, int op, void *tt) {
    if (!ioctl(fd, op, tt)) return 0;
    return -1;
}

#endif
