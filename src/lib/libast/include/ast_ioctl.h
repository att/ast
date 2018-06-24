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
#ifndef _AST_IOCTL_H
#define _AST_IOCTL_H 1

#include <sys/ioctl.h>

#ifndef _AST_INTERCEPT_IMPLEMENT
#define _AST_INTERCEPT_IMPLEMENT 1
#endif

#if _AST_INTERCEPT_IMPLEMENT < 2

#undef ioctl
#define ioctl ast_ioctl

#endif

#if _AST_INTERCEPT_IMPLEMENT > 0

extern int ast_ioctl(int, int, ...);

#endif

#endif  // _AST_IOCTL_H
