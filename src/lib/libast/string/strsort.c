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
#pragma prototyped
/*
 *  strsort - sort an array pointers using fn
 *
 *	fn follows strcmp(3) conventions
 *
 *   David Korn
 *   AT&T Bell Laboratories
 *
 *  derived from Bourne Shell
 */

#include <ast.h>

void
strsort(char** argv, int n, Strcmp_f cmp)
{
	register int 	i;
	register int 	j;
	register int 	m;
	register char**	ap;
	char*		s;
	int 		k;

	for (j = 1; j <= n; j *= 2);
	for (m = 2 * j - 1; m /= 2;)
		for (j = 0, k = n - m; j < k; j++)
			for (i = j; i >= 0; i -= m)
			{
				ap = &argv[i];
				if ((*cmp)(ap[m], ap[0]) >= 0)
					break;
				s = ap[m];
				ap[m] = ap[0];
				ap[0] = s;
			}
}

void
strsort_r(char** argv, size_t n, Strcmp_context_f cmp, void* handle)
{
	register size_t i;
	register size_t j;
	register size_t m;
	register char**	ap;
	char*		s;
	size_t 		k;

	for (j = 1; j <= n; j *= 2);
	for (m = 2 * j - 1; m /= 2;)
		for (j = 0, k = n - m; j < k; j++)
			for (i = j;; i -= m)
			{
				ap = &argv[i];
				if ((*cmp)(ap[m], ap[0], handle) >= 0)
					break;
				s = ap[m];
				ap[m] = ap[0];
				ap[0] = s;
				if (i < m)
					break;
			}
}
