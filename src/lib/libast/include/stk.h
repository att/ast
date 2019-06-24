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
 * David Korn
 * AT&T Research
 *
 * Interface definitions for a stack-like storage library
 *
 */
#ifndef _STK_H
#define _STK_H 1

#include "sfio.h"

#define _Stk_data _Stak_data

#define stkstd (&_Stk_data)

#define Stk_t Sfio_t

#define STK_SMALL 1 /* small stkopen stack              */
#define STK_NULL 2  /* return NULL on overflow  */

#define stkptr(sp, n) ((char *)((sp)->data) + (n))
#define stktop(sp) ((char *)(sp)->next)
// This is documented to return an int but on systems with 64 bit pointers it returns a long long.
// This causes lint warnings about narrowing implicit converions. Since the value will never exceed
// 2GB just do an explicit cast to eliminate the lint.
#define stktell(sp) (int)((sp)->next - (sp)->data)

extern Sfio_t _Stk_data;

extern Stk_t *stkopen(int);
extern Stk_t *stkinstall(Stk_t *, char *(*)(int));
extern int stkclose(Stk_t *);
extern int stklink(Stk_t *);
extern void *stkalloc(Stk_t *, size_t);
extern char *stkcopy(Stk_t *, const char *);
extern char *stkset(Stk_t *, char *, size_t);
extern void *stkseek(Stk_t *, ssize_t);
extern char *stkfreeze(Stk_t *, size_t);
extern int stkon(Stk_t *, char *);

#endif  // _STK_H
