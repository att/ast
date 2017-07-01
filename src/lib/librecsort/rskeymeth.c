/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * rskey method by name
 */

#include "rskeyhdr.h"

static Rsmethod_t**	methods[] =
{
	&Rsrasp,
	&Rsradix,
	&Rssplay,
	&Rsverify,
	&Rscopy,
};

/*
 * return rs method given name
 */

Rsmethod_t*
#if __STD_C
rskeymeth(register Rskey_t* kp, const char* name)
#else
rskeymeth(kp, name)
register Rskey_t*	kp;
char*			name;
#endif
{
	register int	n;

	if (!name || !*name || streq(name, "-") || streq(name, "default"))
		return Rsrasp;
	for (n = 0; n < elementsof(methods); n++)
		if (streq(name, (*methods[n])->name))
			return *methods[n];
	return 0;
}

/*
 * list rs method names on fp
 */

int
#if __STD_C
rskeylist(register Rskey_t* kp, Sfio_t* fp, int usage)
#else
rskeylist(kp, fp, usage)
register Rskey_t*	kp;
Sfio_t*			fp;
int			usage;
#endif
{
	register int	i;
	register int	n;

	n = 0;
	for (i = 0; i < elementsof(methods); i++)
		if (usage)
			n += sfprintf(fp, "[+%s?%s]", (*methods[i])->name, (*methods[i])->desc);
		else
			n += sfprintf(fp, "%-10s %s\n", (*methods[i])->name, (*methods[i])->desc);
	return n;
}
