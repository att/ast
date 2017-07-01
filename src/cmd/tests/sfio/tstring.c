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

/* test to make sure sfputr() does not cause too many buffer reallocation */
static int	Putrextend = 0;
static char	Putrbuf[1024*1024];
static int putrextend(Sfio_t* f, int type, Void_t* arg, Sfdisc_t* disc)
{
	if(type == SF_WRITE)
		Putrextend += 1;
	return 0;
}
static Sfdisc_t	Putrdisc = { 0, 0, 0, putrextend, 0 };

tmain()
{
	Sfio_t	*f;
	int	n;
	char	*s, *os, *endos;
	char	buf[8192];

	if(!(f = sfopen((Sfio_t*)0, (char*)0, "srw")) )
		terror("Can't open string stream");
	if(!sfdisc(f, &Putrdisc) )
		terror("Can't insert putr discipline");
	for(n = 0; n < sizeof(Putrbuf)-1; ++n)
		Putrbuf[n] = '0' + n%10;
	if((n = sfputr(f, Putrbuf, '\n')) != sizeof(Putrbuf))
		terror("Wrong sfputr return size=%d", n);
	if(Putrextend != 1)
		terror("Putrextend=%d, should be 1", Putrextend);
	sfseek(f, (Sfoff_t)0, 0);
	if(!(s = sfgetr(f, '\n', 1)) )
		terror("Can't read output string");
	if(strcmp(s, Putrbuf) != 0 )
		terror("Wrong data");
	sfclose(f);

	os = "123\n456\n789\n";
	if(!(f = sfopen((Sfio_t*)0,os,"s")))
		terror("Opening string");

	endos = os + strlen(os);
	while((s = sfgetr(f,'\n',0)) )
	{	
		if(s != os)
			terror("Did not get string");
		os += sfvalue(f);
	}

	if(os != endos)
		terror("Did not match everything");

	if(sfgetc(f) >= 0 || !sfeof(f))
		terror("Stream should have exhausted");

	if(!(f = sfopen(f,(char*)0,"s+")))
		terror("Opening string for r/w");
	for(n = 0; n < 26; ++n)
		if((sfputc(f,'a'+n)) != 'a'+n)
			terror("Output");
	if(sfgetc(f) >= 0)
		terror("Read a non-existent byte");
	sfseek(f,(Sfoff_t)0,0);
	if(!(s = sfreserve(f,26,0)) )
		terror("Didnot get the right amount of data");
	for(n = 0; n < 26; ++n)
		if((sfputc(f,'a'+n)) != 'a'+n)
			terror("Output2");
	sfseek(f,(Sfoff_t)2,0);
	if(!(s = sfreserve(f,50,0)) )
		terror("Didnot get the right amount of data2");

	if(!(f = sfopen(f,(char*)0,"s+")))
		terror("Opening string for r/w");
	sfset(f,SF_READ,0);
	sfseek(f,(Sfoff_t)0,0);
	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) || (n = sfvalue(f)) <= 0 ||
	   sfwrite(f,s,0) != 0)
		terror("Buffer size should be positive");
	sfseek(f,(Sfoff_t)(n+8192),0);
	sfseek(f,(Sfoff_t)0,0);
	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) || sfvalue(f) != (n+8192) ||
	   sfwrite(f,s,0) != 0)
		terror("Bad buffer size");

	if(!(f = sfopen(f,(char*)0,"s+")))
		terror("Opening string for r/w");
	if(sfwrite(f,buf,sizeof(buf)) != sizeof(buf))
		terror("Can't write large buffer");

	if(!(f = sfopen((Sfio_t*)0,(char*)0,"s+")))
		terror("Opening string for r/w");
	sfset(f,SF_READ,0);
	for(n = 0; n < 16*1024; ++n)
	{
         	if((n%1024) == 0)
		{	Sfoff_t a = sfseek(f,(Sfoff_t)1024,1);
			sfputc(f,'a');
			sfseek(f,(Sfoff_t)(-1025),1);
		}
                sfputc(f,'a');
	}
	sfseek(f,(Sfoff_t)0,0);
	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) || sfvalue(f) != n+1024 ||
	   sfwrite(f,s,0) != 0)
		terror("Wrong buffer size");
	while(n-- > 0)
		if(*s++ != 'a')
			terror("Wrong data");

	if(!(f = sfopen((Sfio_t*)0,(char*)0,"s+")))
		terror("Opening r/w string");
	for(n = 0; n < 10; n++)
		sfputc(f,'a'+n);
	sfputc(f,'\n');
	sfseek(f,(Sfoff_t)0,0);
	for(n = 0; n <= 11 ; ++n)
		if(sfgetc(f) != 'a'+n)
			break;
	if(n != 10)
		terror("Get too many");
	if(sfgetc(f) >= 0)
		terror("Reading non-existent data");
	if(!sfeof(f))
		terror("Didn't get eof");

	texit(0);
}
