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
/*
 * G. S. Fowler
 * D. G. Korn
 * AT&T Bell Laboratories
 *
 * shell library support
 */
#include "config_ast.h"  // IWYU pragma: keep

#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ast.h"

//
// Return pointer to the full path name of the shell
//
// SHELL is read from the environment and must start with /
//
// if set-uid or set-gid then the executable and its containing
// directory must not be owned by the real user/group
//
// root/administrator has its own test
//
// "/bin/sh" is returned by default.
//
// NOTE: csh is rejected because the bsh/csh differentiation is
//       not done for `csh script arg ...'
//
char *pathshell(void) {
    char *shell;
    int real_uid;
    int effective_uid;
    int real_gid;
    int effective_gid;
    struct stat statbuf;

    static char *val = NULL;

    shell = getenv("SHELL");
    if (shell && *shell == '/' &&
        strmatch(shell, "*/(sh|*[!cC]sh)*([[:digit:]])?(-+([.[:alnum:]]))?(.exe)")) {
        real_uid = getuid();
        if (!real_uid || !eaccess("/bin", W_OK)) {
            if (stat(shell, &statbuf)) goto defshell;
            if (real_uid != statbuf.st_uid &&
                !strmatch(shell, "?(/usr)?(/local)/?([ls])bin/?([[:lower:]])sh?(.exe)")) {
                goto defshell;
            }
        } else {
            effective_uid = geteuid();
            real_gid = getgid();
            effective_gid = getegid();

            // Check if we are executing in setuid or setgid mode
            if (real_uid != effective_uid || real_gid != effective_gid) {
                char *s;
                char dir[PATH_MAX];

                s = shell;

                // Check the uid and gid on shell and it's parent directory
                for (;;) {
                    if (stat(s, &statbuf)) goto defshell;
                    if (real_uid != effective_uid && statbuf.st_uid == real_uid) goto defshell;
                    if (real_gid != effective_gid && statbuf.st_gid == real_gid) goto defshell;
                    if (s != shell) break;
                    if (strlen(s) >= sizeof(dir)) goto defshell;
                    strcpy(dir, s);
                    s = strrchr(dir, '/');
                    if (!s) break;
                    *s = 0;
                    s = dir;
                }
            }
        }
        return shell;
    }
defshell:
    shell = val;
    if (!shell) shell = "/bin/sh";
    return shell;
}
