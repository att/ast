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
	Sfio_t*	f;

	if(!(f = sfopen(NIL(Sfio_t*),"ab","sr")) )
		terror("Can't open stream");
	if(sfeof(f) || sferror(f))
		terror("Can't be eof or error yet");
	if(sfgetc(f) != 'a')
		terror("Got wrong data");
	if(sfeof(f) || sferror(f))
		terror("Can't be eof or error yet2");
	if(sfgetc(f) != 'b')
		terror("Got wrong data2");
	if(sfeof(f) || sferror(f))
		terror("Can't be eof or error yet3");
	if(sfgetc(f) >= 0)
		terror("Got wrong data2");
	if(!sfeof(f))
		terror("Should be eof now");
	if(sfseek(f,(Sfoff_t)(-1),2) != 1)
		terror("Seek error");
	if(sfeof(f))
		terror("Shouldn't be eof any more");

	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "w+")) )
		terror("Can't open stream2");
	if(sfeof(f) || sferror(f))
		terror("Can't be eof or error yet2");
	if(sfwrite(f,"ab",2) != 2)
		terror("Can't write data");
	if(sfseek(f,(Sfoff_t)0,0) != 0)
		terror("Can't seek back");
	if(sfgetc(f) != 'a')
		terror("Got wrong data3");
	if(sfeof(f) || sferror(f))
		terror("Can't be eof or error yet4");
	if(sfgetc(f) != 'b')
		terror("Got wrong data4");
	if(sfeof(f) || sferror(f))
		terror("Can't be eof or error yet5");
	if(sfgetc(f) >= 0)
		terror("Got wrong data5");
	if(!sfeof(f))
		terror("Should be eof now2");
	if(sfseek(f,(Sfoff_t)(-1),2) != 1)
		terror("Seek error2");
	if(sfeof(f))
		terror("Shouldn't be eof any more2");

	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0),"w+")) )
		terror("Reopening %s", tstfile("sf", 0));
	sfwrite(f,"1234567890",10);
	sfseek(f,(Sfoff_t)0,0);

	if(sfopen(sfstdout, tstfile("sf", 1), "w") != sfstdout)
		terror("Opening %s", tstfile("sf", 1));

	if(sfmove(f,sfstdout,(Sfoff_t)(-1),-1) != 10)
		terror("sfmove failed");
	if(!sfeof(f))
		terror("f should be eof");
	if(sferror(sfstdout))
		terror("sfstdout should not be in error");

	texit(0);
}
