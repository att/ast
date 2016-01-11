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
#include	"sftest.h"

#if _PACKAGE_ast
#ifndef _hdr_wchar
#define _hdr_wchar	1
#endif
#include	<stdio.h>
#endif

#if _hdr_wchar
#include	<wchar.h>
#endif

tmain()
{
	int		n;
	wchar_t		wuf[256];
	char		buf[256];
	char		str[256];

	static char	tst[] = "hello-world";

#if defined(__STDC__) && defined(_hdr_wchar)

#if _PACKAGE_ast

	swprintf(wuf, sizeof(wuf), L"%ls", L"hello-world");
	wcstombs(str, wuf, sizeof(str));
	if (strcmp(tst, str))
		terror("swprintf %%ls \"%s\" expected, \"%s\" returned", tst, str);

	swprintf(wuf, sizeof(wuf), L"%S", L"hello-world");
	wcstombs(str, wuf, sizeof(str));
	if (strcmp(tst, str))
		terror("swprintf %%S \"%s\" expected, \"%s\" returned", tst, str);

	n = swscanf(L" hello-world ", L"%ls", wuf);
	if (n != 1)
		terror("swscanf %%ls %d expected, %d returned", 1, n);
	else
	{
		wcstombs(str, wuf, sizeof(str));
		if (strcmp(tst, str))
			terror("swscanf %%ls \"%s\" expected, \"%s\" returned", tst, str);
	}

	n = swscanf(L" hello-world ", L"%S", wuf);
	if (n != 1)
		terror("swscanf %%S %d expected, %d returned", 1, n);
	else
	{
		wcstombs(str, wuf, sizeof(str));
		if (strcmp(tst, str))
			terror("swscanf %%S \"%s\" expected, \"%s\" returned", tst, str);
	}

	swprintf(wuf, sizeof(wuf), L"%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc",
		L'h',L'e',L'l',L'l',L'o',L'-',L'w',L'o',L'r',L'l',L'd');
	wcstombs(str, wuf, sizeof(str));
	if (strcmp(tst, str))
		terror("swprintf %%lc \"%s\" expected, \"%s\" returned", tst, str);

	swprintf(wuf, sizeof(wuf), L"%C%C%C%C%C%C%C%C%C%C%C",
		L'h',L'e',L'l',L'l',L'o',L'-',L'w',L'o',L'r',L'l',L'd');
	wcstombs(str, wuf, sizeof(str));
	if (strcmp(tst, str))
		terror("swprintf %%C \"%s\" expected, \"%s\" returned", tst, str);

#endif

	sfsprintf(buf, sizeof(buf), "%ls", L"hello-world");
	if (strcmp(tst, buf))
		terror("sfsprintf %%ls \"%s\" expected, \"%s\" returned", tst, buf);

	sfsprintf(buf, sizeof(buf), "%S", L"hello-world");
	if (strcmp(tst, buf))
		terror("sfsprintf %%S \"%s\" expected, \"%s\" returned", tst, buf);

	n = sfsscanf(" hello-world ", "%ls", wuf);
	if (n != 1)
		terror("sfsscanf %%ls %d expected, %d returned", 1, n);
	else
	{
		wcstombs(str, wuf, sizeof(str));
		if (strcmp(tst, str))
			terror("sfsscanf %%ls \"%s\" expected, \"%s\" returned", tst, str);
	}

	n = sfsscanf(" hello-world ", "%S", wuf);
	if (n != 1)
		terror("sfsscanf %%S %d expected, %d returned", 1, n);
	else
	{
		wcstombs(str, wuf, sizeof(str));
		if (strcmp(tst, str))
			terror("sfsscanf %%S \"%s\" expected, \"%s\" returned", tst, str);
	}

	sfsprintf(buf, sizeof(buf), "%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc",
		L'h',L'e',L'l',L'l',L'o',L'-',L'w',L'o',L'r',L'l',L'd');
	if (strcmp(tst, buf))
		terror("sfsprintf %%lc \"%s\" expected, \"%s\" returned", tst, buf);

	sfsprintf(buf, sizeof(buf), "%C%C%C%C%C%C%C%C%C%C%C",
		L'h',L'e',L'l',L'l',L'o',L'-',L'w',L'o',L'r',L'l',L'd');
	if (strcmp(tst, buf))
		terror("sfsprintf %%C \"%s\" expected, \"%s\" returned", tst, buf);

#endif

	texit(0);
}
