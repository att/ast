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

static int	Count;
static int	Size;

#if __STD_C
ssize_t writef(Sfio_t* f, const Void_t* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t writef(f,buf,n,disc)
Sfio_t*		f;
Void_t*		buf;
size_t		n;
Sfdisc_t*	disc;
#endif
{
	Count += 1;
	if((n % Size) != 0)
		terror("Wrong record size");
	return write(sffileno(f),buf,n);
}

Sfdisc_t	Disc = {(Sfread_f)0, writef, (Sfseek_f)0, (Sfexcept_f)0, (Sfdisc_t*)0};

tmain()
{
	Sfio_t*	f;
	char	buf[550];
	int	i;
	char*	s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	Count = 0;
	Size = 52;

	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "w")) )
		terror("Opening to write");
	sfsetbuf(f,buf,sizeof(buf));
	sfset(f,SF_WHOLE,1);
	sfdisc(f,&Disc);

	for(i = 0; i < 100; ++i)
		if(sfwrite(f,s,52) != 52)
			terror("sfwrite failed");
	sfclose(f);
	if(Count != 10)
		terror("Wrong number of writes1");

	Count = 0;
	Size = 53;

	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0),"w")) )
		terror("Opening to write");
	sfsetbuf(f,buf,sizeof(buf));
	sfset(f,SF_WHOLE,1);
	sfdisc(f,&Disc);

	for(i = 0; i < 100; ++i)
		if(sfputr(f,s,'\n') != 53)
			terror("sfputr failed");
	sfclose(f);
	if(Count != 10)
		terror("Wrong number of writes2");

	texit(0);
}
