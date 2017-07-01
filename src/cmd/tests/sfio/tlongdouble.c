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

#if _typ_long_double
#include	<float.h>
#endif

tmain()
{
#if _typ_long_double
	long double	ldval, ldmax;
	char		*s, *str;

	if(sfsscanf("Inf","%Le",&ldmax) != 1)
		terror("sfsscanf Inf failed");
	if(!(s = sfprints("%Le",ldmax)) )
		terror("sfprints failed1");
	if(!(str = malloc(strlen(s)+1)) )
		terror("Malloc failed");
	strcpy(str,s);

	if(sfsscanf(str,"%Le",&ldval) != 1)
		terror("sfsscanf failed");
	if(!(s = sfprints("%Le",ldval)) )
		terror("sfprints failed2");

	if(strcmp(s,str) != 0)
		terror("Bad conversion, expecting %s and getting %s",str,s);
#endif

	texit(0);
}
