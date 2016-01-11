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

#if __STD_C
ssize_t upper(Sfio_t* f, Void_t* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t upper(f,buf,n,disc)
Sfio_t*		f;
Void_t*		buf;
size_t		n;
Sfdisc_t*	disc;
#endif
{
	size_t	i;
	char*	s = (char*)buf;

	if(sfgetc(f) >= 0)
		terror("Stream should be locked");

	if((n = sfrd(f,buf,n,disc)) <= 0)
		return n;
	for(i = 0; i < n; ++i)
		if(s[i] >= 'a' && s[i] <= 'z')
			s[i] = (s[i]-'a') + 'A';
	return n;
}

#if __STD_C
ssize_t wupper(Sfio_t* f, const Void_t* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t wupper(f,buf,n,disc)
Sfio_t*		f;
Void_t*		buf;
size_t		n;
Sfdisc_t*	disc;
#endif
{
	size_t	i;
	char*	s = (char*)buf;

	if(sfputc(f,0) >= 0)
		terror("Stream should be locked2");

	for(i = 0; i < n; ++i)
		if(s[i] >= 'a' && s[i] <= 'z')
			s[i] = (s[i]-'a') + 'A';
	return sfwr(f,buf,n,disc);
}

#if __STD_C
ssize_t lower(Sfio_t* f, Void_t* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t lower(f,buf,n,disc)
Sfio_t*		f;
Void_t*		buf;
size_t		n;
Sfdisc_t*	disc;
#endif
{
	size_t	i;
	char*	s = (char*)buf;

	if(sfgetc(f) >= 0)
		terror("Stream should be locked");

	if((n = sfrd(f,buf,n,disc)) <= 0)
		return n;
	for(i = 0; i < n; ++i)
		if(s[i] >= 'A' && s[i] <= 'Z')
			s[i] = (s[i]-'A') + 'a';
	return n;
}

#if __STD_C
ssize_t once(Sfio_t* f, Void_t* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t once(f,buf,n,disc)
Sfio_t*		f;
Void_t*		buf;
size_t		n;
Sfdisc_t*	disc;
#endif
{
	static int	called = 0;

	if(!called)
	{	called = 1;
		strcpy(buf,"123456");
		return 6;
	}
	else
	{	terror("Discipline is called twice");
		return -1;
	}
}

static char	External[128], *Enext = External;
#if __STD_C
ssize_t external(Sfio_t* f, const Void_t* buf, size_t n, Sfdisc_t* disc)
#else
ssize_t external(f,buf,n,disc)
Sfio_t*		f;
Void_t*		buf;
size_t		n;
Sfdisc_t*	disc;
#endif
{
	memcpy(Enext,buf,n);
	Enext += n;
	return n;
}


Sfdisc_t Wdisc = {(Sfread_f)0, wupper, (Sfseek_f)0, (Sfexcept_f)0, (Sfdisc_t*)0};
Sfdisc_t Udisc = {upper, wupper, (Sfseek_f)0, (Sfexcept_f)0, (Sfdisc_t*)0};
Sfdisc_t Ldisc = {lower, (Sfwrite_f)0, (Sfseek_f)0, (Sfexcept_f)0, (Sfdisc_t*)0};
Sfdisc_t Odisc = {once, (Sfwrite_f)0, (Sfseek_f)0, (Sfexcept_f)0, (Sfdisc_t*)0};
Sfdisc_t Edisc = {(Sfread_f)0, external, (Sfseek_f)0, (Sfexcept_f)0, (Sfdisc_t*)0};

tmain()
{
	Sfio_t	*f, *fs;
	char	*l, *u, *s;
	int	n, r, fd[2];
	int	flags;
	char	buf[1024], low[64], up[64];

	sfsetbuf(sfstdin,buf,sizeof(buf));
	flags = sfset(sfstdin,0,0);
	sfdisc(sfstdin,&Ldisc);
	if(sfset(sfstdin,0,0) != flags)
		terror("Flags changed after discipline setting");

	l = low;
	strcpy(l, "abcdefghijklmnopqrstuvwxyz");
	u = up;
	strcpy(u, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	n = strlen(l);

	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0),"w+")))
		terror("Creating temp file");
	if((r = sfwrite(f,l,n)) != n)
		terror("Writing data %d",r);
	sfsync(f);

	sfseek(f,(Sfoff_t)0,0);
	sfdisc(f,&Udisc);
	if(!(s = sfreserve(f,n,0)) )
		terror("Reading string1");
	if(strncmp(s,u,n) != 0)
	{	s[n-1] = 0;
		terror("Input1=%s, Expect=%s",s,u);
	}

	sfseek(f,(Sfoff_t)0,0);
	sfdisc(f,&Ldisc);
	if(!(s = sfreserve(f,n,0)) )
		terror("Reading string2");
	if(strncmp(s,l,n) != 0)
	{	s[n-1] = 0;
		terror("Input2=%s, Expect=%s",s,l);
	}
	sfclose(f);

	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0), "w+")) )
		terror("Opening file");
	sfdisc(f,&Wdisc);
	if(sfputr(f,low,'\n') < 0)
		terror("Writing data");
	if(sfseek(f,(Sfoff_t)0,0) != 0)
		terror("Seeking");
	if(!(s = sfgetr(f,'\n',1)))
		terror("sfgetr");
	if(strcmp(s,up) != 0)
		terror("Bad data");
	sfclose(f);

	/* read-once discipline */
	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0),"r")) )
		terror("Opening file");
	sfdisc(f,&Odisc);
	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) )
		terror("Sfreserve failed1");
	if(sfvalue(f) != 6)
		terror("Wrong reserved length1");
	sfread(f,s,3);

	if(!(fs = sfopen(NIL(Sfio_t*),"abcdef","s")) )
		terror("String open failed");
	sfstack(f,fs);
	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) )
		terror("Sfreserve failed2");
	if(sfvalue(f) != 6)
		terror("Wrong reserved length2");
	sfread(f,s,6);

	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) )
		terror("sfreserve failed 3");
	if(sfvalue(f) != 3)
		terror("Wrong reserved length3");
	sfclose(f);
	sfclose(fs);

	if(!(f = sfopen(NIL(Sfio_t*), tstfile("sf", 0),"w")) )
		terror("Opening file to write");
	sfdisc(f,&Edisc);
	if(sfwrite(f, "one", 3) != 3)
		terror("Bad sfwrite");
	if(sfwr(f, "two", 4, NIL(Sfdisc_t*)) != 4)
		terror("Bad sfwr");
	if(strcmp(External,"onetwo") != 0)
		terror("Bad call of sfwr");
	sfclose(f);

	/* test for setting discipline on an unseekable device */
	if(pipe(fd) < 0)
		terror("Bad pipe call");
	if(!(f = sfnew(0, 0, -1, fd[0], SF_READ)) )
		terror("Can't create stream to read");
	if(write(fd[1], "1234ABCD", 8) != 8)
		terror("Can't write to pipe");
	close(fd[1]);
	if(!(s = sfreserve(f, -1, SF_LOCKR)) )
		terror("Can't reserve for data");
	if(s[0] != '1' || s[1] != '2' || s[2] != '3' || s[3] != '4')
		terror("Bad reserved data");
	if(sfread(f, s, 4) != 4)
		terror("Bad reopening of stream");
	if(!sfdisc(f, &Ldisc) )
		terror("Can't insert new discipline");
	if(sfread(f, buf, sizeof(buf)) != 4)
		terror("Read wrong data size");
	if(buf[0] != 'a' || buf[1] != 'b' || buf[2] != 'c' || buf[3] != 'd')
		terror("Bad read data");

	texit(0);
}
