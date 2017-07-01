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
#ifndef _ASO_H
#define _ASO_H		1

#define ASO_UNLOCK	0	/* unlock if key matches  		*/
#define ASO_TRYLOCK	1	/* matched key means successful attempt	*/
#define ASO_LOCK	2	/* matched key first, then spin-lock	*/
#define ASO_SPINLOCK	3	/* no matching of key before locking	*/

#define asogetint(p)	(*(p))

extern unsigned int	asoaddint(unsigned int*, unsigned int);
extern unsigned int	asosubint(unsigned int*, unsigned int);
extern void*		asocasptr(void*, void*, void*);
extern int		asolock(unsigned int volatile*, unsigned int, int);

#endif
