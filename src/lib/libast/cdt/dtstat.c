/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#include	"dthdr.h"
#include	<stdio.h>

/* Get statistics for a dictionary
**
** Written by Kiem-Phong Vo, phongvo@gmail.com
*/

ssize_t dtstat(Dt_t* dt, Dtstat_t* dtst)
{
	ssize_t	sz, k;
	char	*str;
	char	*end;

	sz = (ssize_t)(*dt->meth->searchf)(dt, (Void_t*)dtst, DT_STAT);

	end = &dtst->mesg[sizeof(dtst->mesg)] - 1;
	str = dtst->mesg;
	str += sfsprintf(str, end - str, "Objects=%ld #Toptablesize=%ld #Level=%ld\n",
		(long)dtst->size, (long)dtst->tslot, (long)(dtst->mlev+1) );

	/* print first 5 levels */
	for(k = 0; k < 5 && k <= dtst->mlev; ++k)
		str += sfsprintf(str, end - str, "\t\tlev[%ld]=%ld\n", (long)k, (long)dtst->lsize[k] );
	str[0] = 0;

	return sz;
}
