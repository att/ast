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

/* to test handling of writing whole lines */
static ssize_t writef(Sfio_t* f, const void* data, size_t n, Sfdisc_t* disc)
{
	char	*dt;
	ssize_t	k;

	for(dt = (char*)data + n-1; dt >= (char*)data; --dt)
		if(*dt == '\n')
			break;

	if((k = (dt - (char*)data) + 1) == 0 )
		tinfo("Processing a partial line, ok");

	return k;
}

Sfdisc_t Disc = { 0, writef, 0, 0, 0 };

tmain()
{
	char	buf[100];
	Sfio_t	*fp;
	int	i;
	char	*s;

	if(!(fp = sftmp(8)))
		terror("Can't open temp file");

	sfset(fp,SF_LINE,1);
	for(i = 0; i < 1000; ++i)
	{	sfsprintf(buf,sizeof(buf),"Number: %d",i);
		if(sfputr(fp,buf,'\n') <= 0)
			terror("Writing %s",buf);
	}

	sfseek(fp,(Sfoff_t)0,0);

	for(i = 0; i < 1000; ++i)
	{	sfsprintf(buf,sizeof(buf),"Number: %d",i);
		if(!(s = sfgetr(fp,'\n',1)))
			terror("Reading %s",buf);
		if(strcmp(s,buf) != 0)
			terror("Input=%s, Expect=%s",s,buf);
	}

	sfseek(fp,(Sfoff_t)0,0);
	s = sfgetr(fp,'\0',1);
	if(s)
		terror("Expecting a null string");
	s = sfgetr(fp,'\0',-1);
	if(!s)
		terror("Expecting a non-null string");
	if(sfvalue(fp) != sfsize(fp))
		terror("Wrong size");

	sfclose(fp);
	if(!(fp = sfnew(0, buf, 12, 1, SF_WRITE)) )
		terror("Opening a test stream");
	sfsetbuf(fp, buf, 12);
	sfset(fp, SF_LINE, 0);
	sfdisc(fp, &Disc);
	if(sfputr(fp, "0123456789", '\n') != 11)
		terror("Sfputr failed1");
	if(sfputr(fp, "0", -1) != 1)
		terror("Sfputr failed2");
	if(sfputr(fp, "1", -1) != 1)
		terror("Sfputr failed3");

	texit(0);
}
