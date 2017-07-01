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
	int	i, n, k;
	Sfoff_t	o;
	char	buf[1024], *s;
	char	bigbuf[1024*8];
	int	fd[2];
	Sfio_t*	f;

	if(!(f = sfopen(0, tstfile("sf", 0), "w")) )
		terror("Opening file to write");
	if(sfwrite(f,"0123456789",10) != 10 || sfwrite(f,"abcdefgh",8) != 8)
		terror("Writing data");

	if(!(f = sfopen(f, tstfile("sf", 0), "r")) )
		terror("Opening file to read");
	sfsetbuf(f, buf, sizeof(buf));

	if(!(s = (char*)sfreserve(f,10,0)) )
		terror("sfreserve failed");
	if(strncmp(s,"0123456789",10) != 0)
		terror("Did not get correct data");

	if((s = (char*)sfreserve(f,10,0)) )
		terror("sfreserve should not have succeeded");
	if(sfvalue(f) != 8)
		terror("sfreserve should have left the right unread record length");

	if(!(s = (char*)sfreserve(f,4,0)) )
		terror("sfreserve should return 4 bytes");
	if(strncmp(s,"abcd",4) != 0)
		terror("Got wrong data");

	if((s = (char*)sfreserve(f,10,0)) )
		terror("sfreserve should not have succeeded2");
	if(sfvalue(f) != 4)
		terror("sfreserve should have left 4 bytes length");

	if(!(s = (char*)sfreserve(f,0,SF_LASTR)) )
		terror("sfreserve should have returned last unread record");
	if(strncmp(s,"efgh",4) != 0)
		terror("Record has wrong data");

	sfclose(f);

	sfsetbuf(sfstdout,buf,sizeof(buf));
	sfset(sfstdout,SF_SHARE|SF_PUBLIC,0);
	if((s = (char*)sfreserve(sfstdout,0,0)) != buf)
		terror("Wrong buffer");
	if((n = sfwrite(sfstdout,"foobar",6)) != 6)
		terror("Write failed");
	if((char*)sfreserve(sfstdout,0,0) != s+6)
		terror("Wrong reserved pointer");
	sfpurge(sfstdout);

	if(sfopen(sfstdout, tstfile("sf", 0),"w") != sfstdout)
		terror("Opening file");

	sfsetbuf(sfstdout,NIL(char*),0);
	if(!(s = sfreserve(sfstdout,0,SF_LOCKR)) )
		terror("Could not lock stdout");
	if(sfputc(sfstdout,'1') >= 0)
		terror("stdout wasn't locked");
	if(sfwrite(sfstdout,s,0) != 0)
		terror("stdout can't be unlocked");

	sfsetbuf(sfstdout,NIL(char*),sizeof(buf)/2);

	for(i = 0; i < sizeof(buf); ++i)
		buf[i] = (i%26) + 'a';

	n = 0;
	for(i = 0; i < 33; ++i)
	{	if(!(s = sfreserve(sfstdout,sizeof(buf),SF_LOCKR)) )
			terror("Can't reserve write buffer");

		memcpy(s,buf,sizeof(buf));

		if(sfwrite(sfstdout,s,sizeof(buf)) != sizeof(buf) )
			terror("Writing to file");
		else	n += sizeof(buf);
	}

	sfsync(sfstdout);

	if(sfopen(sfstdin, tstfile("sf", 0),"r") != sfstdin)
		terror("Opening file2");
	sfsetbuf(sfstdin,NIL(char*),8*sizeof(buf));
	if(sfsize(sfstdin) != n)
		terror("Wrong size for file");

	i = 0;
	for(;;)
	{	if(!(s = sfreserve(sfstdin,16*sizeof(buf),0)) )
			break;
		else	i += 16*sizeof(buf);
	}
	if(sfvalue(sfstdin) > 0)
		i += sfvalue(sfstdin);
	if(i != n)
		terror("Did not read data");

	if(sfseek(sfstdin,(Sfoff_t)0,0) != 0)
		terror("sfseek failed0");
	sfsetbuf(sfstdin,bigbuf,sizeof(bigbuf));
	i = 0;
	for(;;)
	{	if(!(s = sfreserve(sfstdin,16*sizeof(buf),0)) )
			break;
		else	i += 16*sizeof(buf);
	}
	if(sfvalue(sfstdin) > 0)
		i += sfvalue(sfstdin);
	if(i != n)
		terror("Did not read data2");
	sfsetbuf(sfstdin,NIL(Void_t*),(size_t)SF_UNBOUND);

	if(sfopen(sfstdout, tstfile("sf", 0), "w") != sfstdout)
		terror("Can't open to write");
	for(i = 0; i < 32; ++i)
	{	for(k = 0; k < sizeof(bigbuf); ++k)
			bigbuf[k] = '0' + (k+i)%10;
		if(sfwrite(sfstdout,bigbuf,sizeof(bigbuf)) != sizeof(bigbuf))
			terror("Writing to %s", tstfile("sf", 0));
	}
	sfclose(sfstdout);

	if(sfopen(sfstdin, tstfile("sf", 0), "r") != sfstdin)
		terror("Opening to read");
	sfsetbuf(sfstdin,NIL(Void_t*),8*1024);
	if(!(s = sfreserve(sfstdin,16*sizeof(bigbuf),0)) )
		terror("sfreserve failed");
	for(i = 0; i < 16; ++i)
	{	for(k = 0; k < sizeof(bigbuf); ++k)
			if(*s++ != ('0' + (k+i)%10))
				terror("Wrong data i=%d k=%d",i,k);
	}
	if((o = sfseek(sfstdin,-15*((Sfoff_t)sizeof(bigbuf)),1)) != sizeof(bigbuf))
		terror("sfseek failed o=%lld", (Sflong_t)o);
	if(sfread(sfstdin,bigbuf,sizeof(bigbuf)) != sizeof(bigbuf) )
		terror("sfread failed");
	s = bigbuf;
	for(i = 1; i < 2; ++i)
	{	for(k = 0; k < sizeof(bigbuf); ++k)
			if(*s++ != ('0' + (k+i)%10))
				terror("Wrong data2 i=%d k=%d",i,k);
	}
	if(!(s = sfreserve(sfstdin,16*sizeof(bigbuf),SF_LOCKR)) )
	{	sfsetbuf(sfstdin,NIL(Void_t*),16*sizeof(bigbuf));
		if(!(s = sfreserve(sfstdin,16*sizeof(bigbuf),SF_LOCKR)) )
			terror("sfreserve failed2");
	}

	sfread(sfstdin,s,0);
#ifdef MAP_TYPE
	if(sfreserve(sfstdin,0,0) != s)
		terror("Reserve pointer changed?");
#endif
	for(i = 2; i < 17; ++i)
	{	for(k = 0; k < sizeof(bigbuf); ++k)
			if(*s++ != ('0' + (k+i)%10))
				terror("Wrong data3 i=%d k=%d",i,k);
	}

	if(sfopen(sfstdout, tstfile("sf", 0),"w") != sfstdout)
		terror("Opening for write");
	for(i = 0; i < 100; ++i)
		bigbuf[i] = 'a';
	for(i = 0; i < 101; ++i)
		if(sfwrite(sfstdout,bigbuf,100) != 100)
			terror("Bad write to file");
	sfsync(sfstdout);
	if(sfopen(sfstdin, tstfile("sf", 0),"r") != sfstdin)
		terror("Opening for read");
	sfsetbuf(sfstdin,buf,1024);
	sfset(sfstdin,SF_SHARE,0);
	for(i = 0; i < 10; ++i)
		if(!sfreserve(sfstdin,500,0) )
			terror("Can't reserve from file");
	for(i = 0; i < 5; ++i)
		if(sfwrite(sfstdout,bigbuf,100) != 100)
			terror("Bad write to file2");
	sfsync(sfstdout);
	n = 5000;
	while(sfreserve(sfstdin,500,0))
		n += 500;
	if(n+sfvalue(sfstdin) != sfsize(sfstdout))
		terror("Wrong reserve size from file");

	tcleanup();

	fd[0] = fd[1] = -1;
	if(pipe(fd) < 0 || fd[0] < 0 || fd[1] < 0)
		terror("Can't make pipe");
	if(write(fd[1],"abcdefghijklmnopqrstuvwxyz\n0123456789",37) != 37)
		terror("Can't write to pipe");
	close(fd[1]);

	sfclose(sfstdin);
	if(sfnew(sfstdin,NIL(Void_t*),(size_t)SF_UNBOUND,fd[0],SF_READ) != sfstdin)
		terror("Can't creat pipe stream");
	if(!(s = sfgetr(sfstdin,'\n',1) ) ||
	   strcmp(s,"abcdefghijklmnopqrstuvwxyz") != 0)
		terror("Get wrong string");
	if((s = sfreserve(sfstdin,16,SF_LOCKR)) )
		terror("There should not be enough data for this");
	if(!(s = sfreserve(sfstdin,10,0)) )
		terror("Fail to reserve remainder of stream");
	if(strncmp(s,"0123456789",10) != 0)
		terror("Reserved data was corrupted");

	for(i = 0; i < 18; i += 6)
	{
		if(!(f = sftmp(i)) )
			terror("Can't open tempfile");

		sfset(f,SF_READ,0);
		for(k = 0; k < 10; ++k)
			if(sfputc(f,'0'+k) != '0'+k)
				terror("Write %c to temp file", '0'+k);
		if(!sfreserve(f,0,-1) )
			terror("No write buffer?");

		if(i < 10 && (sfset(f,0,0)&SF_STRING) )
			terror("No file created");

		sfset(f,SF_READ,1);
		sfseek(f,(Sfoff_t)0,0);
		if(sfgetc(f) != '0')
			terror("Getting the 0");
		if(!(s = sfreserve(f,0,-1)) || sfvalue(f) != 9 ||
		   strncmp(s,"123456789",9) != 0)
			terror("Read reserved failed");
		if(sfgetc(f) != '1')
			terror("Getting the 1");
		sfclose(f);
	}

	if(!(f = sftmp(0)) )
		terror("Can't open file");
	for(i = 0; i < sizeof(bigbuf); ++i)
		bigbuf[i] = (i%10) + '0';
	if(sfwrite(f,bigbuf,1000) != 1000)
		terror("Writing to file");
	sfsetbuf(f,buf,100);
	sfseek(f,(Sfoff_t)0,0);
	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) )
		terror("sfreserve failed at bottom1");
	if(sfvalue(f) != 100)
		terror("Expecting1 100 bytes, get only %d",sfvalue(f));
	if(strncmp(s,bigbuf,100) != 0)
		terror("Wrong data at bottom");
	sfread(f,s,95);
	if(!(s = sfreserve(f,6,SF_LOCKR)) )
		terror("sfreserve failed at bottom2");
	if(sfvalue(f) != 100)
		terror("Expecting2 100 bytes, get only %d",sfvalue(f));
	if(strncmp(s,bigbuf+5,100) != 0)
		terror("Wrong data at bottom2");
	sfread(f,s,5);
	for(i = 1; i < 10; ++i)
	{	if(!(s = sfreserve(f,-96,SF_LOCKR)) )
			terror("sfreserve failed at bottom loop");
		if(sfvalue(f) != 100)
			terror("Expecting3 100 bytes, get only %d",sfvalue(f));
		if(strncmp(s,bigbuf,100) != 0)
			terror("Wrong data at bottom loop");
		sfread(f,s,100);
	}

	sfseek(f,(Sfoff_t)0,0);
	for(i = 0; i < 16; ++i)
		sfwrite(f,bigbuf,sizeof(bigbuf));
	sfseek(f,(Sfoff_t)0,0);
	sfset(f,SF_WRITE,0);
	sfsetbuf(f,NIL(Void_t*),4096);
	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) )
		terror("sfreserve failed 11");
	if((n = sfvalue(f)) < 4096)
		terror("sfvalue is wrong");
	if(sfread(f,s,n-16) != n-16)
		terror("sfread failed");
	if(!(s = sfreserve(f,-7,SF_LOCKR)) )
		terror("sfreserve failed 12");
	if(sfvalue(f) < 16 )
		terror("hmm");

	if(!(f = sfopen(0, "", "sr")) )
		terror("can't open a read string stream");
	if(!(s = sfreserve(f, 0, SF_LOCKR)) )
		terror("can't lock an empty string stream");
	if(sfread(f,s,0) != 0)
		terror("can't unlock");
	if((s = sfreserve(f, 0, 0)) )
		terror("reserve successful on an empty stream");

	if(!(f = sfopen(0, "", "sw")) )
		terror("can't open a write string stream");
	if(!(s = sfreserve(f, 0, SF_LOCKR)) )
		terror("can't lock an empty string stream");
	if(sfwrite(f,s,0) != 0)
		terror("can't unlock");
	if((s = sfreserve(f, 0, 0)) )
		terror("reserve successful on an empty stream");

	texit(0);
}
