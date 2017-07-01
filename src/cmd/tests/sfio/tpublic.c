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
	Sfio_t*	g;
	char*	s;
	char	buf[1024];
	int	n, i;

	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "w")) )
		terror("Can't open file to write");
	sfputr(f,"1111",'\n');
	sfputr(f,"2222",'\n');
	sfputr(f,"3333",'\n');
	sfputr(f,"4444",'\n');

	if(!(f = sfopen(f, tstfile("sf", 0), "r")) )
		terror("Can't open file to read1");
	if(!(g = sfnew(NIL(Sfio_t*),
			NIL(Void_t*),(size_t)SF_UNBOUND,dup(sffileno(f)),SF_READ)) )
		terror("Can't open file to read2");

	sfset(f,SF_SHARE|SF_PUBLIC,1);
	sfset(g,SF_SHARE|SF_PUBLIC,1);

	if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"1111") != 0)
		terror("Wrong data1");
	sfsync(f);
	if(!(s = sfgetr(g,'\n',1)) || strcmp(s,"2222") != 0)
		terror("Wrong data2");
	sfsync(g);
	if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"3333") != 0)
		terror("Wrong data3");
	sfsync(f);
	if(!(s = sfgetr(g,'\n',1)) || strcmp(s,"4444") != 0)
		terror("Wrong data4");
	sfsync(g);

	sfclose(f);
	sfclose(g);
	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "r+")) )
		terror("Can't open file to write2");
	if(!(g = sfnew(NIL(Sfio_t*),
			NIL(Void_t*),(size_t)SF_UNBOUND,dup(sffileno(f)),SF_READ)) )
		terror("Can't open file to read3");

	sfset(f,SF_SHARE|SF_PUBLIC,1);
	sfset(g,SF_SHARE|SF_PUBLIC,1);

	if(sfputr(f,"1111",'\n') <= 0)
		terror("bad write1");
	sfsync(f);
	if(!(s = sfgetr(g,'\n',1)) || strcmp(s,"2222") != 0)
		terror("Wrong data5");
	sfsync(g);
	if(sfputr(f,"3333",'\n') <= 0)
		terror("bad write2");
	sfsync(f);
	if(!(s = sfgetr(g,'\n',1)) || strcmp(s,"4444") != 0)
		terror("Wrong data6");
	sfsync(g);

	if(!(f = sfopen(f, tstfile("sf", 0), "w")) )
		terror("Can't open file to write3");

	for(i = 0; i < sizeof(buf); ++i)
		buf[i] = 0;
	for(i = 0; i < 256; ++i)
		if(sfwrite(f,buf,sizeof(buf)) != sizeof(buf))
			terror("Writing buffer0");

	for(i = 0; i < sizeof(buf); ++i)
		buf[i] = 1;
	for(i = 0; i < 256; ++i)
		if(sfwrite(f,buf,sizeof(buf)) != sizeof(buf))
			terror("Writing buffer1");

	if(!(f = sfopen(f, tstfile("sf", 0), "r")) )
		terror("Can't open file to read3");
	sfset(f,SF_SHARE|SF_PUBLIC,1);

	for(n = 0; n < 256; ++n)
	{	if(!(s = sfreserve(f,sizeof(buf),0)) )
			terror("Can't reserve buffer1");
		for(i = 0; i < sizeof(buf); ++i)
			if(s[i] != 0)
				terror("Bad data1");
	}

	for(n = 0; n < 256; ++n)
	{	if(!(s = sfreserve(f,sizeof(buf),0)) )
			terror("Can't reserve buffer2");
		for(i = 0; i < sizeof(buf); ++i)
			if(s[i] != 1)
				terror("Bad data2");
	}

	if((s = sfreserve(f,1,0)) )
		terror("Reading beyond eof");

	if(!(f = sfopen(f, tstfile("sf", 0), "w")) )
		terror("Can't open to write");
	if(sfwrite(f,"aaa\nbbb\nccc\n",12) != 12)
		terror("Can't write");
	sfclose(f);
	if(sfopen(sfstdin,tstfile("sf", 0),"r") != sfstdin)
		terror("Can't open file as sfstdin");
	if((n = (int)sfmove(sfstdin,NIL(Sfio_t*),(Sfoff_t)SF_UNBOUND,'\n')) != 3)
		terror("sfmove wrong number of lines %d",n);
	if(sfseek(sfstdin,(Sfoff_t)0,0) != 0)
		terror("Can't seek back to 0");
	if((n = (int)sfmove(sfstdin,NIL(Sfio_t*),(Sfoff_t)2,'\n')) != 2)
		terror("sfmove2 wrong number of lines %d",n);
	if((n = (int)sfmove(sfstdin,NIL(Sfio_t*),(Sfoff_t)SF_UNBOUND,'\n')) != 1)
		terror("sfmove3 wrong number of lines %d",n);

	texit(0);
}
