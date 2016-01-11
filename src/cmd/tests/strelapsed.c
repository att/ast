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
 * test harness for strelapsed
 */

#include <ast.h>

int
main(int argc, char** argv)
{
	char*		s;
	char*		e;
	unsigned long	t;
	int		n;

	while (s = *++argv)
	{
		n = (int)strtol(s, &e, 0);
		if (*e)
		{
			sfprintf(sfstderr, "%s: number expected", s);
			break;
		}
		if (!(s = *++argv))
		{
			sfprintf(sfstderr, "elapsed time expression expected");
			break;
		}
		t = strelapsed(s, &e, n);
		sfprintf(sfstdout, "strelapsed   \"%s\" \"%s\" %lu %d\n", s, e, t, n);
	}
	return 0;
}
