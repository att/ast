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
	int	fd[2];
	Sfio_t	*fr, *fw;
	char	*s;
	int	n, w;

	if(pipe(fd) < 0)
		terror("Can't open pipe");

	if(!(fr = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,fd[0],SF_READ)) ||
	   !(fw = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,fd[1],SF_WRITE)) )
		terror("Can't open pipe streams");
	sfset(fr,SF_SHARE,1);

	if(sfopen(sfstdout,tstfile("sf", 0),"w") != sfstdout)
		terror("Can't open for write");
	if(sfopen(sfstdin,tstfile("sf", 0),"r") != sfstdin)
		terror("Can't open for read");

	for(n = 0; n < 100; ++n)
		if((w = sfwrite(fw,"123456789\n",10)) != 10)
			terror("Writing to pipe w=%d",w);

	if((n = (int)sfmove(fr,sfstdout,(Sfoff_t)100,'\n')) != 100)
		terror("sfmove failed n=%d", n);
	sfclose(sfstdout);

	for(n = 0; n < 100; ++n)
	{	if(!(s = sfgetr(sfstdin,'\n',1)) )
			terror("Can't read data");
		if(strcmp(s,"123456789") != 0)
			terror("Wrong data");
	}

	texit(0);
}
