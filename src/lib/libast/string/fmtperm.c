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
//
// Glenn Fowler
// AT&T Bell Laboratories
//
// Return strperm() expression for perm
//
#include "config_ast.h"  // IWYU pragma: keep

#include <sys/stat.h>

// This function always returns same static buffer
char *fmtperm(int perm) {
    static char strbuf[32];
    char *s = strbuf;

    //
    //  User permissions
    //
    *s++ = 'u';
    *s++ = '=';
    if (perm & S_ISVTX) *s++ = 't';
    if (perm & S_ISUID) *s++ = 's';
    if (perm & S_IRUSR) *s++ = 'r';
    if (perm & S_IWUSR) *s++ = 'w';
    if (perm & S_IXUSR) *s++ = 'x';
    if ((perm & (S_ISGID | S_IXGRP)) == S_ISGID) *s++ = 'l';

    //
    // Group Permissions
    //
    *s++ = ',';
    *s++ = 'g';
    *s++ = '=';
    if ((perm & (S_ISGID | S_IXGRP)) == (S_ISGID | S_IXGRP)) *s++ = 's';
    if (perm & S_IRGRP) *s++ = 'r';
    if (perm & S_IWGRP) *s++ = 'w';
    if (perm & S_IXGRP) *s++ = 'x';

    //
    // Other permissions
    //
    *s++ = ',';
    *s++ = 'o';
    *s++ = '=';
    if (perm & S_IROTH) *s++ = 'r';
    if (perm & S_IWOTH) *s++ = 'w';
    if (perm & S_IXOTH) *s++ = 'x';
    *s = 0;
    return strbuf;
}
