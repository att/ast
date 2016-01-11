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

static char	Serial[128], *S = Serial;
#if __STD_C
ssize_t writef(Sfio_t* f, const Void_t* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t writef(f, buf, n, disc)
Sfio_t*		f;
Void_t*		buf;
size_t		n;
Sfdisc_t*	disc;
#endif
{
	memcpy((Void_t*)S,buf,n);
	S += n;
	return n;
}
Sfdisc_t	Serialdc = {NIL(Sfread_f), writef, NIL(Sfseek_f), NIL(Sfexcept_f) };

tmain()
{
	int	i, n, on;
	char	*s, *os, *s1, *s2, *s3;
	char	poolbuf[1024];
	Sfio_t	*f, *f1, *f2, *f3, *f4;

	if(!(f1 = sfopen((Sfio_t*)0,tstfile("sf", 0),"w+")) ||
	   !(f2 = sfopen((Sfio_t*)0,tstfile("sf", 1),"w"))  ||
	   !(f3 = sfopen((Sfio_t*)0,tstfile("sf", 2),"w")))
		terror("Opening files");

	if(!(f4 = sfopen((Sfio_t*)0,tstfile("sf", 0),"r+")) )
		terror("Opening file to read");
	sfungetc(f1,'a');
	sfungetc(f4,'b');
	sfpool(f1,f4,0);
	sfungetc(f1,'a');
	sfpool(f1,NIL(Sfio_t*),0);

	sfsetbuf(f2,poolbuf,sizeof(poolbuf));
	sfsetbuf(f3,poolbuf,sizeof(poolbuf));
	if(!sfpool(f2,f3,0) )
		terror("Setting pool");

	os = "1234567890\n";
	on = strlen(os);
	for(i = 0; i < 100; ++i)
		if(sfputr(f1,os,-1) < 0)
			terror("Writing data");
	sfseek(f1,(Sfoff_t)0,0);
	for(i = 0; i < 100; ++i)
	{	if(!(s = sfgetr(f1,'\n',0)) || (n = sfvalue(f1)) != on)
			terror("Reading data");
		if(sfwrite(f2,s,n) != n)
			terror("Writing1");
		if(sfwrite(f3,s,n) != n)
			terror("Writing2");
	}

	/* see if data matches */
	if(!(f1 = sfopen(f1, tstfile("sf", 0), "r")) ||
	   !(f2 = sfopen(f2, tstfile("sf", 1), "r")) ||
	   !(f3 = sfopen(f3, tstfile("sf", 2), "r")) )
		terror("sfopen for file comparison");

	if(sfsize(f1) != sfsize(f2) || sfsize(f2) != sfsize(f3))
		terror("Files don't match sizes");

	n = (int)sfsize(f1);
	if(!(s1 = sfreserve(f1,n,0)) ||
	   !(s2 = sfreserve(f2,n,0)) ||
	   !(s3 = sfreserve(f3,n,0)) )
		terror("Fail reserving data");

	if(memcmp(s1,s2,n) != 0 || memcmp(s2,s3,n) != 0)
		terror("Data do not match");
	sfclose(f1);
	sfclose(f2);
	sfclose(f3);

	f1 = sfstdout; f2 = sfstderr;
	sfdisc(sfstdout,&Serialdc);
	sfdisc(sfstderr,&Serialdc);
	sfset(sfstdout,SF_LINE,0);
	sfset(sfstderr,SF_LINE,0);
	if(sfpool(sfstdout,sfstderr,0) != sfstderr )
		terror("sfpool1");
	sfputc(sfstdout,'1');
	sfputc(sfstderr,'2');
	sfputc(sfstdout,'3');
	sfputc(sfstderr,'4');
	sfsync(sfstderr);
	if(strcmp(Serial,"1234") != 0)
		terror("Pool not serializing output");
	sfdisc(sfstdout,NIL(Sfdisc_t*));
	sfdisc(sfstderr,NIL(Sfdisc_t*));

	sfclose(sfstdout);
	if(!(f1 = sfopen((Sfio_t*)0,tstfile("sf", 0),"r")))
		terror("sfopen");
	if(!sfpool(f1,sfstderr,0) )
		terror("sfpool2");

	if(!(f1 = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "w+")) ||
	   !(f2 = sfopen(NIL(Sfio_t*), tstfile("sf", 1), "w+")) ||
	   !(f3 = sfopen(NIL(Sfio_t*), tstfile("sf", 2), "w+")) )
		terror("sfopen3");
	if(sfpool(f1,f2,SF_SHARE) != f2)
		terror("sfpool3 f1");
	if(sfpool(f3,f2,SF_SHARE) != f2 )
		terror("sfpool3 f3");
	if(sfputc(f3,'x') < 0)
		terror("sfputc to f3");
	if(sfputc(f2,'y') < 0)
		terror("sfputc to f2");
	if(sfputc(f1,'z') < 0)
		terror("sfputc to f1");
	if(sfseek(f1,(Sfoff_t)0,0) != 0)
		terror("sfseek failed on f1");
	if(!(s = sfreserve(f1,3,SF_LOCKR)) || sfvalue(f1) != 3)
		terror("sfreserve failed on f1");
	if(memcmp(s,"xyz",3) != 0)
		terror("Wrong data");

	if((os = sfreserve(f2,SF_UNBOUND,0)) )
		terror("sfreserve should have failed on f2");

	if(sfpool(NIL(Sfio_t*),f2,0) != f1)
		terror("Didn't get right pool head for f2");

	if(sfread(f1,s,3) != 3)
		terror("Wrong read on f1");

	if(!(f = sfpool(f3,NIL(Sfio_t*),0)) )
		terror("sfpool to delete f3");
	if(f != f1 && f != f2)
		terror("sfpool delete did not return a stream from old pool");

	if(sfpool(f1,NIL(Sfio_t*),0) != f2 )
		terror("sfpool delete did not return a stream from pool");

	if(sfpool(f1,NIL(Sfio_t*),0) != f1 )
		terror("sfpool delete of a lone stream did not return self");

	if(!(f1 = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "w+")) ||
	   !(f2 = sfopen(NIL(Sfio_t*), tstfile("sf", 1), "w")) )
		terror("sfopen4");
	sfputc(f1,'a');
	sfputc(f1,'b');
	sfputc(f1,'c');
	sfset(f1,SF_WRITE,0);	/* off write mode */
	if(sfpool(f1,f2,SF_SHARE) )
		terror("sfpool should fail pooling read+write streams");
	if(sfseek(f1,(Sfoff_t)0,0) != (Sfoff_t)0)
		terror("sfseek failed");
	if(!(s = sfreserve(f1,3,SF_LOCKR)) || memcmp(s,"abc",3) != 0)
		terror("Can't get data from f1");

	texit(0);
}
