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

/*
 * AT&T Research
 *
 * test harness for
 *
 *	strtol		strtoul		strton
 *	strtoll		strtoull	strtonll
 *	strntol		strntoul	strnton
 *	strntoll	strntoull	strntonll
 */

#if _PACKAGE_ast
#include <ast.h>
#else
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <locale.h>

#ifndef ERANGE
#define ERANGE	EINVAL
#endif

#ifndef errno
extern int	errno;
#endif

#if !_PACKAGE_ast

static int
base(const char* s)
{
	const char*	t;
	int		n;

	for (t = s; *t == '-' || *t == '+' || *t == ' ' || *t == '\t'; t++);
	if (!*t)
		return 10;
	if (*t == '0')
		return (*++t == 'x' || *t == 'X') ? (*(t + 1) ? 16 : 10) : *t ? 8 : 0;
	n = 0;
	while (*t >= '0' && *t <= '9')
		n = n * 10 + *t++ - '0';
	return *t == '#' ? (*(t + 1) ? n : 10) : 0;
}

static long
strton(const char* s, char** e, char* b, int m)
{
	int		o;

	o = *b;
	*b = base(s);
	return strtol(s, e, o);
}

static intmax_t
strtonll(const char* s, char** e, char* b, int m)
{
	int		o;

	o = *b;
	*b = base(s);
	return strtoll(s, e, o);
}

#endif

int
main(int argc, char** argv)
{
	char*			s;
	char*			p;
	unsigned long		l;
	uintmax_t		ll;
	char			b;
	int			m;
	int			decimal;
	int			sep = 0;
#if _PACKAGE_ast
	int			n;
#endif

	if (argc <= 1)
	{
		printf("%u/%u\n", sizeof(l) * 8, sizeof(ll) * 8);
		return 0;
	}
	decimal = *localeconv()->decimal_point;
	while (s = *++argv)
	{
		if (!strncmp(s, "LC_ALL=", 7))
		{
#if _PACKAGE_ast
			p = s + 7;
#else
			p = "de_DE";
#endif
			if (!setlocale(LC_ALL, p))
			{
				printf("%s failed\n", s);
				return 0;
			}
			decimal = *localeconv()->decimal_point;
			continue;
		}
		if (sep)
			printf("\n");
		else
			sep = 1;
		m = strchr(s, decimal) ? 100 : 0;

		errno = 0;
		l = strtol(s, &p, 0);
		printf("strtol    \"%s\" \"%s\" %ld %s\n", s, p, l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		b = 0;
		l = strton(s, &p, &b, m);
		printf("strton    \"%s\" \"%s\" %ld %s %d\n", s, p, l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR", b);

		errno = 0;
		l = strtoul(s, &p, 0);
		printf("strtoul   \"%s\" \"%s\" %lu %s\n", s, p, l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ll = strtoll(s, &p, 0);
		printf("strtoll   \"%s\" \"%s\" %lld %s\n", s, p, ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		b = 0;
		ll = strtonll(s, &p, &b, m);
		printf("strtonll  \"%s\" \"%s\" %lld %s %d\n", s, p, ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR", b);

		errno = 0;
		ll = strtoull(s, &p, 0);
		printf("strtoull  \"%s\" \"%s\" %llu %s\n", s, p, ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

#if _PACKAGE_ast
		n = strlen(s);

		errno = 0;
		l = strntol(s, n, &p, 0);
		printf("strntol    %2d \"%s\" \"%s\" %ld %s\n", n, s, p, l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		l = strntol(s, n - 1, &p, 0);
		printf("strntol    %2d \"%s\" \"%s\" %ld %s\n", n - 1, s, p, l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		b = 0;
		l = strnton(s, n, &p, &b, m);
		printf("strnton    %2d \"%s\" \"%s\" %ld %s %d\n", n, s, p, l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR", b);

		errno = 0;
		b = 0;
		l = strnton(s, n - 1, &p, &b, m);
		printf("strnton    %2d \"%s\" \"%s\" %ld %s %d\n", n - 1, s, p, l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR", b);

		errno = 0;
		l = strntoul(s, n, &p, 0);
		printf("strntoul   %2d \"%s\" \"%s\" %lu %s\n", n, s, p, l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		l = strntoul(s, n - 1, &p, 0);
		printf("strntoul   %2d \"%s\" \"%s\" %lu %s\n", n - 1, s, p, l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ll = strntoll(s, n, &p, 0);
		printf("strntoll   %2d \"%s\" \"%s\" %lld %s\n", n, s, p, ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ll = strntoll(s, n - 1, &p, 0);
		printf("strntoll   %2d \"%s\" \"%s\" %lld %s\n", n - 1, s, p, ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		b = 0;
		ll = strntonll(s, n, &p, &b, m);
		printf("strntonll %2d \"%s\" \"%s\" %lld %s %d\n", n, s, p, ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR", b);

		errno = 0;
		b = 0;
		ll = strntonll(s, n - 1, &p, &b, m);
		printf("strntonll %2d \"%s\" \"%s\" %lld %s %d\n", n - 1, s, p, ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR", b);

		errno = 0;
		ll = strntoull(s, n, &p, 0);
		printf("strntoull %2d \"%s\" \"%s\" %llu %s\n", n, s, p, ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ll = strntoull(s, n - 1, &p, 0);
		printf("strntoull %2d \"%s\" \"%s\" %llu %s\n", n - 1, s, p, ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");
#endif
	}
	return 0;
}
