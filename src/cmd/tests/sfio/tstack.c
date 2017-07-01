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

static Sfio_t*	Fclose;

#if __STD_C
int exceptf(Sfio_t*f, int type, Void_t* data, Sfdisc_t* disc)
#else
int exceptf(f, type, data, disc)
Sfio_t*		f;
int		type;
Void_t*		data;
Sfdisc_t*	disc;
#endif
{
	if(type == SF_CLOSING || type == SF_FINAL)
	{	if(f != Fclose)
			return -1;
		if(type == SF_CLOSING && (f->mode&SF_RDWR) != f->mode)
			terror("Stream should be open");
		return 0;
	}

	if((f->mode&SF_RDWR) != f->mode )
		terror("Stream mode should be accessible in exceptf");

	return 0;
}

#if __STD_C
ssize_t readf(Sfio_t* f, Void_t* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t readf(f, buf, n, disc)
Sfio_t*		f;
Void_t*		buf;
size_t		n;
Sfdisc_t*	disc;
#endif
{
	if((f->mode&SF_RDWR) == f->mode )
		terror("Stream mode should be inaccessible in readf");
	return 0;
}

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
	if((f->mode&SF_RDWR) == f->mode )
		terror("Stream mode should be inaccessible in writef");
	return 0;
}

Sfdisc_t	Disc = { readf, writef, NIL(Sfseek_f), exceptf, 0 };

tmain()
{
	Sfio_t	*f1, *f2, *f3, *f;
	char	*s, *s1, *s2, *s3, *s4, str[1024], *ss;
	int	n;
	int	fd[2];

	if(!(f1 = sfopen(NIL(Sfio_t*), tstfile("sf", 0),"w+")) )
		terror("Opening file1");
	if(!(f2 = sfopen(NIL(Sfio_t*), tstfile("sf", 0),"w+")) )
		terror("Opening file2");
	Fclose = f2;
	sfdisc(f1,&Disc);
	sfdisc(f2,&Disc);
	sfstack(f1,f2);
	if((n = sfgetc(f1)) >= 0 || !sfeof(f1))
		terror("There should be no data n=%d",n);
	if(sfstacked(f1))
		terror("There should be no stack");
	Fclose = f1;
	if(sfclose(f1) < 0)
		terror("Can't close f1");
	tcleanup();

	s1 = "1234567890";
	s2 = "abcdefghijklmnopqrstuvwxyz";
	s3 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	s4 = "!@#$%^&*()_-+={}[]~`':;?/><,|";

	if(!(f1 = sfopen((Sfio_t*)0,s1,"s")) ||
	   !(f2 = sfopen((Sfio_t*)0,s2,"s")) ||
	   !(f3 = sfopen((Sfio_t*)0,s3,"s")))
		terror("Opening strings");

	sfdisc(sfstdin,&Disc);
	sfclose(sfstdin);
	if(sffileno(sfstdin) != 0)
		terror("Bad fd for stdin");

	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0),"w+")) )
		terror("Opening file");
	if(sfwrite(f,"0123456789",10) != 10)
		terror("Write file");
	if(sfseek(f,(Sfoff_t)0,0) != 0)
		terror("Seek file");
	
	if(sfstack(sfstdin,f) != sfstdin)
		terror("Stacking on stdin2");
	if(sfopen(sfstdout,"/dev/null","w") != sfstdout)
		terror("Opening sfstdout");
	if(sfmove(sfstdin,sfstdout,(Sfoff_t)SF_UNBOUND,-1) != 10 ||
	   !sfeof(sfstdin) || sferror(sfstdout))
		terror("Bad sfmove");

	tcleanup();

	if(!(f = sftmp(0)))
		terror("Opening temp file");
	if(sfputr(f,s4,-1) != (ssize_t)strlen(s4))
		terror("Writing s4");
	sfseek(f,(Sfoff_t)0,0);

#if FIX_THIS_TEST_2008_08_11
	if(sfstack(f,f3) != f)
		terror("Stacking s3");
	if(sfstack(f,f2) != f)
		terror("Stacking s2");
	if(sfstack(f,f1) != f)
		terror("Stacking s1");

	sfsprintf(str,sizeof(str),"%s%s%s%s",s1,s2,s3,s4);
	if((ss = sfgetr(f,'\n',1)) )
		terror("There shouldn't have been any new-line");
	else
	{	if(!(ss = sfgetr(f,'\n',-1)) )
			terror("Reading streams");
		n = sfvalue(f);
		if (ss[n])
			ss[n] = 0;
	}

	if(strcmp(ss,str) != 0)
		terror("Expect=%s Got=%s",str,ss);
#endif

	if(!(f1 = sfopen((Sfio_t*)0,s1,"s")) ||
	   !(f2 = sfopen((Sfio_t*)0,s2,"s")) ||
	   !(f3 = sfopen((Sfio_t*)0,s3,"s")))
		terror("Opening strings2");
	sfseek(f,(Sfoff_t)0,0);

	if(sfstack(f,f3) != f || sfstack(f,f2) != f || sfstack(f,f1) != f)
		terror("Stacking streams2");

	if(!(s = sfreserve(f,SF_UNBOUND,0)) || s != s1)
		terror("Sfpeek1");

	if(!(s = sfreserve(f,SF_UNBOUND,0)) || s != s2)
		terror("Sfpeek2");

	if(!(s = sfreserve(f,SF_UNBOUND,0)) || s != s3)
		terror("Sfpeek3");

	if(!(s = sfreserve(f,SF_UNBOUND,0)) || strncmp(s,s4,strlen(s4)) != 0)
		terror("Sfpeek4");

	/* test to see if hidden read data still accessible */
	if(pipe(fd) < 0)
		terror("Can't create pipe");
	if(!(f1 = sfnew(0, NIL(Void_t*), (size_t)SF_UNBOUND, fd[0], SF_READ|SF_WRITE)) )
		terror("Can't create stream");

	if(write(fd[1],"0123",4) != 4)
		terror("Can't write to pipe");
	if(sfgetc(f1) != '0')
		terror("sfgetc failed");

	/* hack to create hidden reserved buffer */
	f1->_file = fd[1];
	if(sfwrite(f1,"4",1) != 1)
		terror("Can't write to stream");
	sfsync(f1);
	f1->_file = fd[0];
	close(fd[1]);

	/* now stack stream */
	if(!(f2 = sfopen(0, "abcd\n", "s")))
		terror("Can't open string stream");

	sfstack(f2,f1);

	if(!(s = sfgetr(f2, '\n', 1)) )
		terror("sfgetr failed");

	if(strcmp(s, "1234abcd") != 0)
		terror("sfgetr got wrong data");

	texit(0);
}
