/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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
 *                                                                      *
 ***********************************************************************/
#include "config_ast.h"  // IWYU pragma: keep

#include <unistd.h>

#include "sfio.h"
#include "terror.h"

static int Type;
static int Mttype;

static void notify(Sfio_t *f, int type, void *data) {
    UNUSED(f);
    UNUSED(data);

    switch (type) {
        case SF_NEW:
        case SF_CLOSING:
        case SF_SETFD:
        case SF_READ:
        case SF_WRITE:
            Type = type;
            return;
        case SF_MTACCESS:
            Mttype = type;
            return;
        default:
            terror("Unexpected nofity-type: %d", type);
            break;  // this is actually unreachable but here to silence lint warning
    }
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);

    sfnotify(notify);

    Sfio_t *f = sfopen(NULL, tstfile("sf", 0), "w");
    if (!f) terror("sfopen() failed");
    if (Type != SF_NEW) terror("Notify did not announce SF_NEW event");

    int fd = sffileno(f);
    close(fd + 5);
    if (sfsetfd(f, fd + 5) != fd + 5 || Type != SF_SETFD) {
        terror("Notify did not announce SF_SETFD event");
    }
    if (sfclose(f) < 0 || Type != SF_CLOSING) terror("Notify did not announce SF_CLOSING event");

    if (sfputc(sfstdin, 'a') >= 0 || Type != SF_WRITE) {
        terror("Notify did not announce SF_WRITE event");
    }

    if (sfgetc(sfstdout) >= 0 || Type != SF_READ) terror("Notify did not announce SF_READ event");

    texit(0);
}
