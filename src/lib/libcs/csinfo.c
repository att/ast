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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * return Sfio_t stream pointer to host info file
 * if line!=0 then it points to current line number in file
 */

#include "cslib.h"

Sfio_t*
csinfo(register Cs_t* state, const char* file, int* line)
{
	int		n;
	Sfio_t*		sp = 0;
	char		buf[PATH_MAX];
	char		tmp[PATH_MAX];
	struct stat	st;

	messagef((state->id, NiL, -8, "info(%s) call", file));
	if (!file || streq(file, "-")) file = CS_SVC_INFO;
	if (strmatch(file, "*[ \t\n=]*")) sp = tokline(file, SF_STRING, line);
	else if (!strchr(file, '/') || stat(file, &st) || S_ISDIR(st.st_mode) || !(sp = tokline(file, SF_READ, line)))
		for (n = 0; n <= 1; n++)
		{
			sfsprintf(tmp, sizeof(tmp), "%s/%s", n ? csvar(state, CS_VAR_SHARE, 0) : CS_SVC_DIR, file);
			if (pathpath(tmp, "", PATH_REGULAR|PATH_READ, buf, sizeof(buf)))
			{
				sp = tokline(buf, SF_READ, line);
				break;
			}
		}
	if (!sp) messagef((state->id, NiL, -1, "info: %s: not found", file));
	return sp;
}

Sfio_t*
_cs_info(const char* file, int* line)
{
	return csinfo(&cs, file, line);
}
