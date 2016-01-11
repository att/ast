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
	Sfio_t	*f, *f2;
	char*	s;
	int	i, n;
	char	buf[16*1024];

	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "w+") ) )
		terror("Can't open file");

	if(sfnputc(f,'a',1000) != 1000)
		terror("Writing");

	if(sfseek(f,(Sfoff_t)0,0) != 0)
		terror("Seeking");

	if((n = (int)sfsize(f)) != 1000)
		terror("Wrong size %d", n);

	if(!(f2 = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,
			sffileno(f),SF_WRITE)) )
		terror("Can't open stream");

	if(sfseek(f2,(Sfoff_t)1999,0) != (Sfoff_t)1999)
		terror("Seeking2");
	sfputc(f2,'b');
	sfsync(f2);
	if((n = (int)sfsize(f2)) != 2000)
		terror("Wrong size2 %d", n);

	if((n = (int)sfsize(f)) != 1000)
		terror("Wrong size3 %d", n);

	sfputc(f,'a');
	sfset(f,SF_SHARE,1);
	if((n = (int)sfsize(f)) != 2000)
		terror("Wrong size4 %d", n);

	if(!(f = sfopen(f,NIL(char*),"srw")) )
		terror("Can't open string stream");

	sfwrite(f,"0123456789",10);
	if(sfsize(f) != 10)
		terror("String size is wrong1");
	sfseek(f,(Sfoff_t)19,0);
	if(sfsize(f) != 10)
		terror("String size is wrong2");
	sfputc(f,'a');
	if(sfsize(f) != 20)
		terror("String size is wrong3");
	sfseek(f,(Sfoff_t)0,0);
	if(sfsize(f) != 20)
		terror("String size is wrong4");
	sfseek(f,(Sfoff_t)0,0);
	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) && sfvalue(f) != 20)
		terror("String size is wrong5");
	sfread(f,s,5);
	if(sfsize(f) != 20)
		terror("String size is wrong6");
	sfwrite(f,"01234567890123456789",20);
	if(sfsize(f) != 25)
		terror("String size is wrong7");

	strcpy(buf,"0123456789");
	if(!(f = sfopen(f,buf,"s+")) )
		terror("Can't open string stream2");
	if(sfset(f,0,0)&SF_MALLOC)
		terror("SF_MALLOC should not have been set");
	if(sfsize(f) != 10)
		terror("String size is wrong8");
	sfread(f,buf,5);
	if((n = (int)sfwrite(f,"0123456789",10)) != 5)
		terror("Write wrong amount %d", n);
	if(sfsize(f) != 10)
		terror("String size is wrong9");

	if(!(f = sfopen(f, tstfile("sf", 0),"w") ) )
		terror("Reopening file1");
	for(i = 0; i < 10000; ++i)
		if(sfputc(f,'0'+(i%10)) != '0'+(i%10) )
			terror("sfputc failed");

	if(!(f = sfopen(f, tstfile("sf", 0),"r+") ) )
		terror("Reopening file2");
	if(sfsize(f) != 10000)
		terror("Bad size of file1");

	sfsetbuf(f,buf,1024);
	for(i = 0; i < 20; ++i)
		if(!sfreserve(f,100,0))
			terror("Reserve failed");

	s = buf+1024;
	for(i = 0; i < 20; ++i)
		s[i] = '0' + i%10;
	sfseek(f,(Sfoff_t)(10000-10),0);
	if(sfwrite(f,s,20) != 20)
		terror("Write failed");
	if(sfsize(f) != 10010)
		terror("Bad size of file2");
	sfseek(f,(Sfoff_t)0,0);
	for(i = 0; i < 10; ++i)
	{	if(!(s = sfreserve(f,1001,0)) )
			terror("Reserve failed2");
		if(s[0] != '0'+i)
			terror("Bad data1");
	}
	for(n = 0; n < 1001; ++n)
		if(s[n] != ((n+i-1)%10 + '0') )
			terror("Bad data");

	/* test to see if a string stream extends ok during writes */
	s = malloc(5);
	f = sfnew((Sfio_t*)0, (void*)s, 5, -1, SF_STRING|SF_READ|SF_WRITE|SF_MALLOC);
	if(!f)
		terror("Can't create string stream");
	if(sfwrite(f,"01",2) != 2)
		terror("Bad write to string stream");
	if(sfwrite(f,"2345678",7) != 7)
		terror("Bad write to string stream2");
	if(sfputc(f,0) != 0)
		terror("sfputc failed");
	if(sfseek(f, (Sfoff_t)0, 0) != 0)
		terror("sfseek failed");
	if((n = (int)sfread(f, buf, 100)) != 10)
		terror("sfread gets wrong amount of data %d", n);
	if(strcmp(buf, "012345678") != 0)
		terror("Get wrong data");

	texit(0);
}
