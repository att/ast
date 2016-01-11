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

static int	line;

#define SYNC	line = __LINE__;

#if __STD_C
void alrmf(int sig)
#else
void alrmf(sig)
int	sig;
#endif
{
	terror("blocked at line %d", line);
}

tmain()
{
	int	fd[2];
	Sfio_t	*fr, *fw;
	char	*s;
	int	i, j, n, to;
	char	buf[1024];

	to = argc > 1 ? 0 : 4;

	if(sfnew(sfstdout,buf,sizeof(buf),SF_UNBOUND,SF_STRING|SF_WRITE) != sfstdout)
		terror("Reopen sfstdout");

	if(pipe(fd) < 0)
		terror("Can't open pipe");
	if(!(fr = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,fd[0],SF_READ)) ||
	   !(fw = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,fd[1],SF_WRITE)) )
		terror("Can't open stream");
	if(to)
	{	signal(SIGALRM,alrmf);
		alarm(4);
	}
	SYNC sfwrite(fw,"0123456789",10);
	if(sfread(fr,buf,10) != 10)
		terror("Can't read data from pipe");
	SYNC sfwrite(fw,"0123456789",10);
	SYNC if(sfmove(fr,fw,(Sfoff_t)10,-1) != 10)
		terror("sfmove failed");
	if(to)
		alarm(0);
	sfpurge(fw);
	sfclose(fw);
	sfpurge(fr);
	sfclose(fr);

	if(pipe(fd) < 0)
		terror("Can't open pipe2");
	if(!(fr = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,fd[0],SF_READ)) ||
	   !(fw = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND,fd[1],SF_WRITE)) )
		terror("Can't open stream");
	sfset(fr,SF_SHARE|SF_LINE,1);
	sfset(fw,SF_SHARE,1);

	if(to)
		alarm(4);
	SYNC if(sfwrite(fw,"1\n2\n3\n",6) != 6)
		terror("sfwrite failed");
	i = j = -1;
	SYNC if(sfscanf(fr,"%d%d\n%n",&i,&j,&n) != 2 || i != 1 || j != 2 || n != 4)
		terror("sfscanf failed");
	SYNC if(sfscanf(fr,"%d\n%n",&i,&n) != 1 || i != 3 || n != 2)
		terror("sfscanf failed");
	if(to)
		alarm(0);

	if(to)
		alarm(4);
	SYNC if(sfwrite(fw,"123\n",4) != 4)
		terror("sfwrite failed");
	SYNC if(!(s = sfreserve(fr,4,0)) )
		terror("sfreserve failed");
	if(to)
		alarm(0);

	if(to)
		alarm(4);
	SYNC sfputr(fw,"abc",'\n');
	SYNC if(sfmove(fr,fw,(Sfoff_t)1,'\n') != 1)
		terror("sfmove failed");
	SYNC if(!(s = sfgetr(fr,'\n',1)) || strcmp(s,"abc"))
		terror("sfgetr failed");
	if(to)
		alarm(0);

	if(to)
		alarm(4);
	SYNC if(sfwrite(fw,"111\n222\n333\n444\n",16) != 16)
		terror("Bad write to pipe");
	SYNC if(!(s = sfgetr(fr,'\n',1)) )
		terror("sfgetr failed");
	if(to)
		alarm(0);
	if(strcmp(s,"111"))
		terror("sfgetr got wrong string");

	if(to)
		alarm(4);
	SYNC if(sfmove(fr,sfstdout,(Sfoff_t)2,'\n') != 2)
		terror("sfmove failed");
	SYNC sfputc(sfstdout,0);
	SYNC if(strcmp("222\n333\n",buf))
		terror("sfmove got wrong data");
	SYNC if(sfmove(fr,NIL(Sfio_t*),(Sfoff_t)1,'\n') != 1)
		terror("sfmove failed");
	if(to)
		alarm(0);

	if(to)
		alarm(4);
	SYNC if(sfwrite(fw,"0123456789",11) != 11)
		terror("Bad write to pipe2");
	SYNC if(!(s = sfreserve(fr,11,0)) )
		terror("Bad peek size %d, expect 11",sfvalue(fr));
	if(to)
		alarm(0);
	if(strncmp(s,"0123456789",10))
		terror("Bad peek str %s",s);

	/* test for handling pipe error */
	if(pipe(fd) < 0)
		terror("Can't create pipe");
	close(fd[0]);
	if(!(fw = sfnew(NIL(Sfio_t*),NIL(Void_t*),sizeof(buf),fd[1],SF_WRITE)) )
		terror("Can't open stream");
	signal(SIGPIPE,SIG_IGN);

	for(i = 0; i < sizeof(buf); ++i)
		buf[i] = 'a';
	buf[sizeof(buf)-1] = 0;
	for(i = 0; i < 3; ++i)
	{	if(to)
		{	signal(SIGALRM,alrmf); /* do this to avoid infinite loop */
			alarm(4);
		}
		sfprintf(fw, "%s\n", buf); /* this should not block */
		if(to)
			alarm(0);
	}

	texit(0);
}
