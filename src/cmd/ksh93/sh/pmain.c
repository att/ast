/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2011 AT&T Intellectual Property          *
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
 *                    David Korn <dgkorn@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>

extern int sh_main(int, char **, void *);

int main(int argc, char *argv[]) {
    // Ensure stdin, stdout, stderr are open. See https://github.com/att/ast/issues/1117.
    for (int fd = 0; fd < 3; ++fd) {
        errno = 0;
        if (fcntl(fd, F_GETFD, NULL) == -1 || errno == EBADF) {
            // We don't bother to check the fd returned because there isn't anything we can do if
            // it unexpectedly fails. And that should be a "can't happen" situation.
            (void)open("/dev/null", O_RDWR);
        }
    }

    return sh_main(argc, argv, NULL);
}
