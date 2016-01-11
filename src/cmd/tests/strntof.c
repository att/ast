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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * AT&T Research
 *
 * test harness for
 *
 *	strtod		strtold
 */

#if _PACKAGE_ast
#include <ast.h>
#else
#define _ast_fltmax_t	long double
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>
#include <float.h>

#ifndef ERANGE
#define ERANGE	EINVAL
#endif

#ifndef errno
extern int	errno;
#endif

#if !_PACKAGE_ast
#undef	printf
#endif

#ifndef LDBL_DIG
#define LDBL_DIG	DBL_DIG
#endif

int
main(int argc, char** argv)
{
	char*			s;
	char*			p;
	double			d;
	_ast_fltmax_t		ld;
	int			sep = 0;
	size_t			size = 0;

	if (argc <= 1)
	{
		printf("%u\n", LDBL_DIG);
		return 0;
	}
	while (s = *++argv)
	{
		if (!strncmp(s, "LC_ALL=", 7))
		{
			if (!setlocale(LC_ALL, s + 7))
			{
				printf("%s failed\n", s);
				return 0;
			}
			continue;
		}
		if (!size)
		{
			size = atoi(s);
			continue;
		}
		if (sep)
			printf("\n");
		else
			sep = 1;

		errno = 0;
		d = strntod(s, size, &p);
		printf("strntod   \"%s\" \"%s\" %.15e %s\n", s, p, d, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ld = strntold(s, size, &p);
		printf("strntold  \"%s\" \"%s\" %.31Le %s\n", s, p, ld, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");


		size = 0;
	}
	return 0;
}
