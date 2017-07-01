/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
 * codex method name comparison
 * strcmp() semantics
 */

#include <codex.h>

int
codexcmp(register const char* s, register const char* t)
{
	for (;;)
	{
		if (!*s)
		{
			if (!*t || *t == '-' || *t == '+' || *t == '<' || *t == '>' || *t == '|' || *t == '^')
				return 0;
			break;
		}
		if (!*t)
		{
			if (*s == '-' || *s == '+' || *s == '<' || *s == '>' || *s == '|' || *s == '^')
				return 0;
			break;
		}
		if (*s != *t)
			break;
		s++;
		t++;
	}
	return *((unsigned char*)s) - *((unsigned char*)t);
}
