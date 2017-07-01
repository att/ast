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

/* this tests to see if SF_SYNC is raised at the right time */
static int	Count = 0;
#if __STD_C
static int except(Sfio_t* f, int type, Void_t* arg, Sfdisc_t* disc)
#else
static int except(f, type, arg, disc)
Sfio_t*		f;
int		type;
Void_t*		arg;
Sfdisc_t*	disc;
#endif
{
	if(type == SF_SYNC && integralof(arg) == 1)
		Count += 1;
	return 0;
}
Sfdisc_t	Disc = { NIL(Sfread_f), NIL(Sfwrite_f), NIL(Sfseek_f), except };

/* this tests to see if data is written correctly */
typedef struct _mydisc_s
{
	Sfdisc_t	disc;
	int		sync;
} Mydisc_t;
#if __STD_C
int event(Sfio_t* f, int type, Void_t* arg,  Sfdisc_t* disc)
#else
int event(f,type,arg,disc)
Sfio_t*		f;
int		type;
Void_t*		arg;
Sfdisc_t*	disc;
#endif
{
	if(type == SF_SYNC)
		((Mydisc_t*)disc)->sync = integralof(arg);
	return 0;
}

#if __STD_C
ssize_t newline(Sfio_t* f, const Void_t* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t newline(f,buf,n,disc)
Sfio_t*		f;
Void_t*		buf;
size_t		n;
Sfdisc_t*	disc;
#endif
{
	size_t	i;
	char*	s = (char*)buf;

	if(((Mydisc_t*)disc)->sync) /* for synchronization, write everything */
		i = n;
	else /* only write entire lines */
	{	for(i = n; i > 0; --i)
			if(s[i-1] == '\n')
				break;
		if(i <= 0)
			return 0;
	}

	return sfwr(f,buf,i,disc);
}

Mydisc_t Mydisc = {{(Sfread_f)0, newline, (Sfseek_f)0, event, (Sfdisc_t*)0}, 0};

tmain()
{
	int	p1[2], p2[2];
	int	n, dupf2;
	Sfoff_t	off;
	char	buf[1024], *s;
	Sfio_t	*f, *f1, *f2;

	/* redirect stdout to a pipe */
	if(pipe(p1) < 0 || close(1) < 0 || dup(p1[1]) != 1)
		terror("Creating pipe1");

	/* redirect stdin to a pipe with some input */
	close(0);
	if(pipe(p2) < 0)
		terror("Creating a pipe2");
	if(write(p2[1],"foo\n",4) != 4)
		terror("Writing to pipe");
	close(p2[1]);

	sfsetbuf(sfstdout,buf,sizeof(buf));
	if(sfset(sfstdout,0,0)&SF_LINE)
		terror("Line mode on unexpectedly");
	sfset(sfstdout,SF_SHARE,0);

	if(sfeof(sfstdin) )
		terror("Premature eof");
	if(sferror(sfstdout) )
		terror("Weird error1");
	if((off = sfmove(sfstdin,sfstdout,(Sfoff_t)SF_UNBOUND,-1)) != 4)
		terror("Wrong # of bytes %lld", off);
	if(!sfeof(sfstdin) )
		terror("Should be eof");
	if(sferror(sfstdout) )
		terror("Weird error2");
	if(sfpurge(sfstdout) < 0)
		terror("Purging stdout");

	if(!(f1 = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "w")) )
		terror("Opening file to write");
	if(!(f2 = sfopen(NIL(Sfio_t*), tstfile("sf", 0),"r")) )
		terror("Opening file to read");

	sfset(f1,SF_IOCHECK,1);
	sfdisc(f1,&Disc);
	sfsetbuf(f1,NIL(char*),4);
	if(Count != 1)
		terror("No sfsync call?");

	Count = 0;
	sfputc(f1,'1');
	sfputc(f1,'2');
	sfputc(f1,'3');
	sfputc(f1,'4');
	sfputc(f1,'5');
	sfputc(f1,'6');

	if((n = sfread(f2,buf,sizeof(buf))) != 4)
		terror("Did not get all data n=%d", n);
	if(Count != 0)
		terror("Should not have seen SF_SYNC yet");
	sfsync(NIL(Sfio_t*));
	if(Count != 1)
		terror("Should have seen SF_SYNC");

	sfputc(f1,'7');
	sfputc(f1,'8');

	sfsync(f1);
	if(Count != 2)
		terror("Bad SF_SYNC count=%d, expecting 2", Count);
	sfsync(f1);
	if(Count != 3)
		terror("Bad SF_SYNC count=%d, expecting 3", Count);

	sfdisc(f1,NIL(Sfdisc_t*));

	sfseek(f2,(Sfoff_t)0,0);
	sfgetc(f2);
	if((off = sftell(f2)) != 1)
		terror("Wrong sfseek location %lld", off);
	sfsync(0);
	if((off = (Sfoff_t)lseek(sffileno(f2), (off_t)0, 1)) != 1)
		terror("Wrong lseek location %lld", off);

	dupf2 = dup(sffileno(f2));
	sfclose(f2);
	if((off = (Sfoff_t)lseek(dupf2, (off_t)0, 1)) != 1)
		terror("Wrong lseek location %lld", off);

	/* test to see if data is written correctly */
	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0),"w+")))
		terror("Creating temp file");
	sfdisc(f, &Mydisc.disc);
	sfsetbuf(f, NIL(Void_t*), 95);
	sfset(f, SF_IOCHECK, 1);

	for(n = 0; n < 10; ++n)
		if(sfwrite(f, "0123456789\n", 11) != 11)
			terror("Bad sfwrite call");

	if(sfwrite(f, "abcdefgh", 8) != 8)
		terror("Bad sfwrite call 2");

	if(sfsync(f) != 0)
		terror("Bad sfsync call");

	if(sfseek(f,(Sfoff_t)0,0) != (Sfoff_t)0)
		terror("Bad sfseek call");

	for(n = 0; n < 10; ++n)
	{	if(!(s = sfgetr(f, '\n', SF_STRING)) )
			terror("Bad sfgetr call");
		if(strcmp(s, "0123456789") != 0)
			terror("Bad read data");
	}

	texit(0);
}
