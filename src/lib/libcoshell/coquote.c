/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Research
 *
 * single quote s into sp
 * if type!=0 then /<getenv(<CO_ENV_TYPE>)/ translated to /$<CO_ENV_TYPE>/
 */

#include "colib.h"

void
coquote(register Sfio_t* sp, register const char* s, int type)
{
	register int	c;

	if (type && (!state.type || !*state.type))
		type = 0;
	while (c = *s++)
	{
		sfputc(sp, c);
		if (c == '\'')
		{
			sfputc(sp, '\\');
			sfputc(sp, '\'');
			sfputc(sp, '\'');
		}
		else if (type && c == '/' && *s == *state.type)
		{
			register const char*	x = s;
			register char*		t = state.type;

			while (*t && *t++ == *x) x++;
			if (!*t && *x == '/')
			{
				s = x;
				sfprintf(sp, "'$%s'", CO_ENV_TYPE);
			}
		}
	}
}
