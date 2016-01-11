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
	Sfio_t	*f;

	if(argc > 1)
	{	if(sfopen(sfstdin,argv[1],"r") != sfstdin)
			terror("Can't reopen stdin");
		sfmove(sfstdin,sfstdout,(Sfoff_t)(-1),-1);
		return 0;
	}

	if(!(f = sfopen((Sfio_t*)0,tstfile("sf", 0),"w")))
		terror("Opening to write");
	if(sfputc(f,'a') != 'a')
		terror("sfputc");
	if(sfgetc(f) >= 0)
		terror("sfgetc");
	
	if(!(f = sfopen(f,tstfile("sf", 0),"r")))
		terror("Opening to read");
	if(sfgetc(f) != 'a')
		terror("sfgetc2");
	if(sfputc(f,'b') >= 0)
		terror("sfputc2");

	if(!(f = sfopen(f,tstfile("sf", 0),"r+")))
		terror("Opening to read/write");

	if(sfgetc(f) != 'a')
		terror("sfgetc3");
	if(sfputc(f,'b') != 'b')
		terror("sfputc3");
	if(sfclose(f) < 0)
		terror("sfclose");

	if(!(f = sfpopen(NIL(Sfio_t*),sfprints("%s %s", argv[0], tstfile("sf", 0)),"r")))
		terror("sfpopen");
	if(sfgetc(f) != 'a')
		terror("sfgetc4");
	if(sfgetc(f) != 'b')
		terror("sfgetc5");
	if(sfgetc(f) >= 0)
		terror("sfgetc6");

	if(!(f = sfopen(f,tstfile("sf", 0),"w")) )
		terror("sfopen");
	if(sfputc(f,'a') != 'a')
		terror("sfputc1");
	sfsetfd(f,-1);
	if(sfputc(f,'b') >= 0)
		terror("sfputc2");
	if(sfclose(f) < 0)
		terror("sfclose");

	if(!(f = sfopen(NIL(Sfio_t*),tstfile("sf", 0),"a+")) )
		terror("sfopen2");
	sfset(f,SF_READ,0);
	if(!sfreserve(f,0,-1) )
		terror("Failed on buffer getting");
	if(sfvalue(f) <= 0)
		terror("There is no buffer?");

	texit(0);
}
