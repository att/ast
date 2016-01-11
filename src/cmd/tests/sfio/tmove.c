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
	char	*s = "1234567890\n";
	Sfoff_t	n, i;
	Sfio_t	*f;
	char	buf[1024];
	char*	addr;

	if(sfopen(sfstdout,tstfile("sf", 0),"w+") != sfstdout)
		terror("Opening output file");
	for(i = 0; i < 10000; ++i)
		if(sfputr(sfstdout,s,-1) < 0)
			terror("Writing data");

	if(!(f = sfopen((Sfio_t*)0,tstfile("sf", 1),"w")))
		terror("Opening output file ");

	sfseek(sfstdout,(Sfoff_t)0,0);
	if((n = sfmove(sfstdout,f,(Sfoff_t)SF_UNBOUND,'\n')) != i)
		terror("Move %d lines, Expect %d",n,i);

	sfseek(sfstdout,(Sfoff_t)0,0);
	sfseek(f,(Sfoff_t)0,0);
	sfsetbuf(sfstdout,buf,sizeof(buf));
	if((n = sfmove(sfstdout,f,(Sfoff_t)SF_UNBOUND,'\n')) != i)
		terror("Move %d lines, Expect %d",n,i);

	sfopen(sfstdin,tstfile("sf", 0),"r");
	sfopen(sfstdout,tstfile("sf", 1),"w");
	sfmove(sfstdin,sfstdout,(Sfoff_t)SF_UNBOUND,-1);
	if(!sfeof(sfstdin))
		terror("Sfstdin is not eof");
	if(sferror(sfstdin))
		terror("Sfstdin is in error");
	if(sferror(sfstdout))
		terror("Sfstdout is in error");

	sfseek(sfstdin,(Sfoff_t)0,0);
	sfseek(sfstdout,(Sfoff_t)0,0);
	sfsetbuf(sfstdin,buf,sizeof(buf));

	addr = (char*)sbrk(0);
	sfmove(sfstdin,sfstdout,(Sfoff_t)((unsigned long)(~0L)>>1),-1);
	if((ssize_t)((char*)sbrk(0)-addr) > 256*1024)
		terror("Too much space allocated in sfmove");

	if(!sfeof(sfstdin))
		terror("Sfstdin is not eof2");
	if(sferror(sfstdin))
		terror("Sfstdin is in error2");
	if(sferror(sfstdout))
		terror("Sfstdout is in error2");

	texit(0);
}
