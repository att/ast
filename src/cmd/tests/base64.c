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
 * base64encode()/base64decode() tester
 */

#include <ast.h>

main()
{
	register int	i;
	int		testno;
	int		errors;
	ssize_t		l;
	ssize_t		r;
	ssize_t		t;
	unsigned char	dat[256];
	unsigned char	tst[sizeof(dat)];
	char		buf[1024];
	unsigned char	pat[16];

	static char	en[] = "YWJjZGVmZ2hpams=";
	static char	de[] = "abcdefghijk";

	testno = errors = 0;
	sfprintf(sfstdout, "TEST	base64\n");
	for (i = 0; i < sizeof(dat); i++)
		dat[i] = i;
	for (i = 0; i < sizeof(dat) - 1; i++)
	{
		testno++;
		if (i > 0)
		{
			l = base64encode(dat, i, NiL, buf, sizeof(buf), NiL);
			if (l < 0 || l > sizeof(buf))
			{
				errors++;
				sfprintf(sfstdout, "test %02d left buffer encode size %ld failed\n", testno, l);
				continue;
			}
			t = base64decode(buf, l, NiL, tst, sizeof(tst), NiL);
			if (t != i)
			{
				errors++;
				sfprintf(sfstdout, "test %02d left buffer decode size %ld failed\n", testno, t);
				continue;
			}
			if (memcmp(dat, tst, i))
			{
				errors++;
				sfprintf(sfstdout, "test %02d left buffer decode failed\n", testno);
				continue;
			}
		}
		else
			l = 0;
		r = base64encode(dat + i, sizeof(dat) - i, NiL, buf + l, sizeof(buf) - l, NiL);
		if (r < 0 || r > sizeof(buf) - l)
		{
			errors++;
			sfprintf(sfstdout, "test %02d right buffer encode size %ld failed\n", testno, r);
			continue;
		}
		t = base64decode(buf + l, r, NiL, tst, sizeof(tst), NiL);
		if (t != (sizeof(dat) - i))
		{
			errors++;
			sfprintf(sfstdout, "test %02d total buffer decode size %ld failed\n", testno, t);
			continue;
		}
		if (memcmp(dat + i, tst, sizeof(dat) - i))
		{
			errors++;
			sfprintf(sfstdout, "test %02d right buffer decode failed\n", testno);
			continue;
		}
		t = base64decode(buf, l + r, NiL, tst, sizeof(tst), NiL);
		if (t != sizeof(dat))
		{
			errors++;
			sfprintf(sfstdout, "test %02d total buffer decode size %ld failed\n", testno, t);
			continue;
		}
		if (memcmp(dat, tst, sizeof(dat)))
		{
			errors++;
			sfprintf(sfstdout, "test %02d total buffer decode failed\n", testno);
			continue;
		}
	}
	for (r = 0; r <= sizeof(pat); r++)
	{
		testno++;
		memset(tst, '*', sizeof(pat));
		tst[sizeof(pat)-1] = 0;
		t = base64decode(en, sizeof(en)-1, NiL, tst, r, NiL);
		if (t != (sizeof(de)-1))
		{
			sfprintf(sfstdout, "decode size %r failed, %r expected\n", t, sizeof(de)-1);
			errors++;
		}
		else
		{
			memset(pat, '*', sizeof(pat));
			pat[sizeof(pat)-1] = 0;
			memcpy(pat, de, r >= sizeof(de) ? sizeof(de) : r);
			if (memcmp(tst, pat, sizeof(pat)))
			{
				sfprintf(sfstdout, "decode failed, \"%s\" != \"%s\"\n", tst, pat);
				errors++;
			}
		}
	}
	sfprintf(sfstdout, "TEST	base64, %d test%s", testno, testno == 1 ? "" : "s");
	sfprintf(sfstdout, ", %d error%s\n", errors, errors == 1 ? "" : "s");
	return 0;
}
