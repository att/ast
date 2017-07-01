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

tmain()
{
	Sfoff_t	m;

	if(sfopen(sfstdout, tstfile("sf", 0), "w") != sfstdout)
		terror("Can't open %s to write", tstfile("sf", 0));
	if(sfputr(sfstdout,"012345678\n",-1) != 10)
		terror("Can't write to %s", tstfile("sf", 0));

	if(sfopen(sfstdout, tstfile("sf", 1),"w") != sfstdout)
		terror("Can't open %s to write", tstfile("sf", 1));

	if(sfopen(sfstdin, tstfile("sf", 0), "r") != sfstdin)
		terror("Can't open %s to read", tstfile("sf", 0));

	if((m = sfmove(sfstdin,sfstdout, (Sfoff_t)SF_UNBOUND, -1)) != 10)
		terror("Moving data from %s to %s m=%lld",
			tstfile("sf", 0), tstfile("sf", 1), (Sflong_t)m);

	texit(0);
}
