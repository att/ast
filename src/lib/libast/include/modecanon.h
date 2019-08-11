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
 * Glenn Fowler
 * AT&T Research
 *
 * canonical mode_t representation
 */
#ifndef _MODECANON_H
#define _MODECANON_H 1

#define X_ISUID 0004000
#define X_ISGID 0002000
#define X_ISVTX 0001000
#define X_IRUSR 0000400
#define X_IWUSR 0000200
#define X_IXUSR 0000100
#define X_IRGRP 0000040
#define X_IWGRP 0000020
#define X_IXGRP 0000010
#define X_IROTH 0000004
#define X_IWOTH 0000002
#define X_IXOTH 0000001

#endif  // _MODECANON_H
