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

static int Bufcount = 0;

#if __STD_C
ssize_t readbuf(Sfio_t* f, Void_t* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t readbuf(f,buf,n,disc)
Sfio_t*		f;
Void_t*		buf;
size_t		n;
Sfdisc_t*	disc;
#endif
{
	Bufcount += 1;
	return sfrd(f,buf,n,disc);
}

Sfdisc_t Disc = {readbuf, (Sfwrite_f)0, (Sfseek_f)0, (Sfexcept_f)0, (Sfdisc_t*)0};

tmain()
{
	Sfio_t	*f, *sf;
	char	*ss, *s, *tmp;
	int	n, i;
	char	zero[SF_BUFSIZE*2];
	char	buf[SF_BUFSIZE], little[512];
	Sfoff_t	one, two;

	s = "123456789\n";
	n = strlen(s);
	if(!(f = sfopen((Sfio_t*)0, tstfile("sf", 0),"w")))
		terror("Opening file to write");
	for(i = 0; i < 1000; ++i)
		if(sfwrite(f,s,n) != n)
			terror("Writing data");

	if(!(f = sfopen(f, tstfile("sf", 0),"r")))
		terror("Opening file to read");

	if(sfseek(f,(Sfoff_t)128,0) != (Sfoff_t)128)
		terror("Bad seek to 128");
	if(sfseek(f,(Sfoff_t)0,1) != (Sfoff_t)128)
		terror("Bad seek(0,1) to 128");

	if(sfseek(f,(Sfoff_t)0,2) != (i*n))
		terror("Bad file length");
	if(sftell(f) != (i*n))
		terror("sftell");
	for(; i > 0; --i)
	{	sfseek(f,(Sfoff_t)(-i*n),2);
		if(!(ss = sfgetr(f,'\n',1)))
			terror("sfgetr");
		if(strncmp(ss,s,sfvalue(f)-1) != 0)
			terror("Expect=%s",s);
	}

	if(!(f = sfopen(f,tstfile("sf", 0),"w")) )
		terror("Open to write");
	for(n = sizeof(zero)-1; n >= 0; --n)
		zero[n] = 0;
	if(sfwrite(f,zero,sizeof(zero)) != sizeof(zero))
		terror("Writing data");
	one = sfseek(f,(Sfoff_t)0,2);
	two = (Sfoff_t)lseek(sffileno(f), (off_t)0, 2);
	if(one != two)
		terror("seeking1");
	if(sfseek(f,(Sfoff_t)(-1),2) != (Sfoff_t)lseek(sffileno(f), (off_t)(-1), 2))
		terror("seeking2");

	if(!(f = sfopen(f,tstfile("sf", 0),"w")))
		terror("Open to write2");
	for(n = 0; n < sizeof(buf); n++)
		buf[n] = n;
	for(n = 0; n < 256; n++)
		if(sfwrite(f,buf,sizeof(buf)) != sizeof(buf))
			terror("Writing data 2");
	if(!(f = sfopen(f,tstfile("sf", 0),"r")))
		terror("Open to read2");
	if(sfgetc(f) != 0 && sfgetc(f) != 1)
		terror("Get first 2 bytes");

	if(sfseek(f,(Sfoff_t)(128*sizeof(buf)),0) != (Sfoff_t)128*sizeof(buf) )
		terror("Seeking ");
	for(n = 0; n < 128; ++n)
		if(sfread(f,buf,sizeof(buf)) != sizeof(buf))
			terror("Reading data");

	if(!(f = sfopen(f,tstfile("sf", 0),"r")))
		terror("Open to read3");
	sfsetbuf(f,little,sizeof(little));
	if(sfread(f, buf, 10) != 10)
		terror("sfread failed");
	if(sftell(f) != (Sfoff_t)10)
		terror("sftell failed");
	if(sfseek(f, (Sfoff_t)10, SEEK_CUR|SF_PUBLIC) != (Sfoff_t)20)
		terror("sfseek failed");
	sfseek(f, (Sfoff_t)0, SEEK_SET);

	if(!(sf = sfnew((Sfio_t*)0, little, sizeof(little), sffileno(f), SF_READ)) )
		terror("sfnew failed");
	if(sfread(f, buf, 10) != 10)
		terror("sfread failed2");
	if(sftell(f) != 10)
		terror("sftell failed2");

	if(sfseek(sf, (Sfoff_t)4000, SEEK_SET) != (Sfoff_t)4000)
		terror("sfseek failed on sf");
	sfsync(sf);

	if(sfseek(f, (Sfoff_t)10, SEEK_CUR|SF_PUBLIC) != (Sfoff_t)4010)
		terror("sfseek public failed");

	/* test to see if the buffering algorithm does the right thing */
	if(!(f = sfopen(NIL(Sfio_t*),tstfile("sf", 0),"w")) )
		terror("Opening test file to write");
	for(i = 0; i < 8192; ++i)
		if(sfputr(f,"123456789",'\n') != 10)
			terror("writing test data");
	if(!(f = sfopen(f,tstfile("sf", 0),"r")) )
		terror("Opening test file to read");
	sfdisc(f,&Disc);
	sfsetbuf(f,NIL(Void_t*),8192);
	for(i = 0; i < 8192; ++i)
	{	sfseek(f, (Sfoff_t)(i*10), 0);
		if(!(s = sfgetr(f, '\n', SF_STRING)) )
			terror("Reading data");
		if(strcmp(s,"123456789") != 0)
			terror("Bad data");
	}
	if(Bufcount != 10)
		terror("Bad buffer filling count");
	sfclose(f);

	/* test buffer alignment for read streams - from a Daytona case */
	tmp = tstfile("sf", 0); /* create a small file of data */
	if(!(f = sfopen(NIL(Sfio_t*), tmp, "w")) )
		terror("Opening to write");
	for(i = 0; i < 500; ++i)
		if(sfputr(f,"123456789",'\n') != 10)
			terror("writing test data");
	sfclose(f);

	if(!(f = sfopen(NIL(Sfio_t*), tmp, "r")) )
		terror("Opening to read");
	sfsetbuf(f, buf, 512);
	if(sfseek(f, (Sfoff_t)2741, SEEK_SET) != (Sfoff_t)2741)
		terror("Bad seek");
	if(!(s = sfreserve(f, 100, -1)) )
		terror("Bad sfreserve");
	if(sfseek(f, (Sfoff_t)0, SEEK_CUR) != (Sfoff_t)2841)
		terror("Bad file position");
	if(sfseek(f, (Sfoff_t)3224, SEEK_SET) != (Sfoff_t)3224)
		terror("Bad seek");
	if(!(s = sfreserve(f, 6, -1)) )
		terror("Bad sfreserve");
	if(strncmp(s, "56789\n", 6) != 0)
		terror("Bad reserved data");

	texit(0);
}
