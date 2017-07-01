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

#if __STD_C
int walkf(Sfio_t* f, Void_t* cntp)
#else
int walkf(f, cntp)
Sfio_t*	f;
Void_t* cntp;
#endif
{
	return (*((int*)cntp) += 1);
}

tmain()
{
	Sfio_t	*f;
	int	c, count = 0;

	if((c = sfwalk(walkf, &count, 0)) != 3) /* counting sfstdin/out/err */
		terror("Bad count c=%d", c);

	if(!(f = sfopen(0, 0, "sw")) )
		terror("Sfopen failed");

	if((c = sfwalk(walkf, &count, 0)) != 7) /* recount std-streams and 1 more */
		terror("Bad count c=%d", c);

	texit(0);
}
