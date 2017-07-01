/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2013 AT&T Intellectual Property          *
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

/*
 * AT&T Research
 *
 * test harness for
 *
 *	strtod		strtold
 *	strntod		strntold
 */

#if _PACKAGE_ast
#include <ast.h>
#else
#ifndef _ISOC99_SOURCE
#define _ISOC99_SOURCE	1
#endif
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include <float.h>

#ifndef ERANGE
#define ERANGE	EINVAL
#endif

#ifndef errno
extern int	errno;
#endif

#ifndef LDBL_DIG
#define LDBL_DIG	DBL_DIG
#endif
#ifndef LDBL_MAX_EXP
#define LDBL_MAX_EXP	DBL_MAX_EXP
#endif

#if !_PACKAGE_ast

#define _ast_fltmax_t	long double

static char		buf[1024];

#undef	strntod
#define strntod		_tst_strntod

static double
strntod(const char* s, size_t n, char** e)
{
	double		r;
	char*		p;

	if (n >= sizeof(buf))
		n = sizeof(buf) - 1;
	memcpy(buf, s, n);
	buf[n] = 0;
	r = strtod(buf, &p);
	if (e)
		*e = (char*)s + (p - buf);
	return r;
}

#undef	strntold
#define strntold	_tst_strntold

static _ast_fltmax_t
strntold(const char* s, size_t n, char** e)
{
	_ast_fltmax_t		r;
	char*		p;

	if (n >= sizeof(buf))
		n = sizeof(buf) - 1;
	memcpy(buf, s, n);
	buf[n] = 0;
	r = strtold(buf, &p);
	if (e)
		*e = (char*)s + (p - buf);
	return r;
}

#endif

int
main(int argc, char** argv)
{
	char*			s;
	char*			p;
	double			d;
	_ast_fltmax_t		ld;
	int			sep = 0;
	int			n;

	if (argc <= 1)
	{
		printf("%u.%u.%u.%u-%u.%u.%u.%u-%u.%u.%u.%u\n", 8*sizeof(float), FLT_DIG, -(FLT_MIN_10_EXP), FLT_MAX_10_EXP, 8*sizeof(double), DBL_DIG, -(DBL_MIN_10_EXP), DBL_MAX_10_EXP, 8*sizeof(_ast_fltmax_t), LDBL_DIG, -(LDBL_MIN_10_EXP), LDBL_MAX_10_EXP);
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
		if (sep)
			printf("\n");
		else
			sep = 1;

		errno = 0;
		d = strtod(s, &p);
		printf("strtod   \"%s\" \"%s\" %.*e %s\n", s, p, DBL_DIG - 1, d, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ld = strtold(s, &p);
		printf("strtold  \"%s\" \"%s\" %.*Le %s\n", s, p, LDBL_DIG - 1, ld, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		n = strlen(s);

		errno = 0;
		d = strntod(s, n, &p);
		printf("strntod  %2d \"%-.*s\" \"%s\" %.*e %s\n", n, n, s, p, DBL_DIG - 1, d, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		d = strntod(s, n - 1, &p);
		printf("strntod  %2d \"%-.*s\" \"%s\" %.*e %s\n", n - 1, n - 1, s, p, DBL_DIG - 1, d, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ld = strntold(s, n, &p);
		printf("strntold %2d \"%-.*s\" \"%s\" %.*Le %s\n", n, n, s, p, LDBL_DIG - 1, ld, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ld = strntold(s, n - 1, &p);
		printf("strntold %2d \"%-.*s\" \"%s\" %.*Le %s\n", n - 1, n - 1, s, p, LDBL_DIG - 1, ld, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");
	}
	return 0;
}
