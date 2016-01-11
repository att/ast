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
#pragma prototyped

#if _PACKAGE_ast
#include <ast.h>
#else
#include <string.h>
#endif

#include <stdio.h>
#include <ip6.h>

int
main(int argc, char** argv)
{
	char*		s;
	char*		e;
	char*		f;
	unsigned char	b;
	unsigned char	a[IP6ADDR];
	int		c;
	int		r;
	int		all = 1;

	while (s = *++argv)
	{
		if (*s == '-' && !*(s + 1))
		{
			all = !all;
			continue;
		}
		r = strtoip6(s, &e, a, &b);
		f = fmtip6(a, b);
		c = *e;
		*e = 0;
		if (r || all || strcmp(s, f))
		{
			*e = c;
			printf("%d %32s %32s %s\n", r, s, e, f);
		}
	}
	return 0;
}
