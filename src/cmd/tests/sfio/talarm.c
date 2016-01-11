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

#define HANDLER	"Handler"
char	Buf[16];
int	Except;

#if __STD_C
void alrmhandler(int sig)
#else
void alrmhandler(sig)
int	sig;
#endif
{
	strcpy(Buf,HANDLER);

	if(Except == 0)
		signal(sig,alrmhandler);
	else if(Except == 1) /* testing return on interrupt */
	{	Except = 2;
		signal(sig,alrmhandler);
		alarm(2);
	}
	else if(Except == 2)
	{	twarn("System call was automatically resumed by the OS");
		texit(0);
	}
	else	terror("Unexpected Except(%d) state", Except);
}

#if __STD_C
int exceptf(Sfio_t* f, int type, Void_t* data, Sfdisc_t* disc)
#else
int exceptf(f, type, data, disc)
Sfio_t* 	f;
int		type;
Void_t*		data;
Sfdisc_t*	disc;
#endif
{
	if(type == SF_ATEXIT || type == SF_DPOP)
		return 0;

	if(type != SF_READ)
		terror("Bad Io type %0o", type);
	if(errno != EINTR)
		terror("Bad exception %d", errno);
	Except = -1;

	return -1;
}

Sfdisc_t Disc = {NIL(Sfread_f), NIL(Sfwrite_f), NIL(Sfseek_f), exceptf};

tmain()
{
	int	fd[2];
	ssize_t	n;
	char	buf[128];

	if(pipe(fd) < 0)
		terror("Can't make pipe");
	if(sfnew(sfstdin,NIL(Void_t*),(size_t)SF_UNBOUND,fd[0],SF_READ) != sfstdin)
		terror("Can't renew stdin");
	sfdisc(sfstdin,&Disc);
	sfset(sfstdin,SF_SHARE,1);

	Except = 0;
	signal(SIGALRM,alrmhandler);
	alarm(2);
	if(sfreserve(sfstdin,1,SF_LOCKR))
		terror("Unexpected data");
	if(strcmp(Buf,HANDLER) != 0)
		terror("Handler wasn't called");
	if(Except >= 0)
		terror("Exception handler wasn't called1");

	Buf[0] = 0;
	Except = 0;
	signal(SIGALRM,alrmhandler);
	alarm(2);
	if(sfgetr(sfstdin,'\n',0))
		terror("Unexpected data2");
	if(strcmp(Buf,HANDLER) != 0)
		terror("Handler wasn't called2");
	if(Except >= 0)
		terror("Exception handler wasn't called2");

	Buf[0] = 0;
	Except = 1; /* testing return-on-interrupt feature */
	sfdisc(sfstdin, NIL(Sfdisc_t*)); /* pop discipline		*/
	sfset(sfstdin, SF_IOINTR, 1);	/* set to return on interrupt	*/
	signal(SIGALRM,alrmhandler);
	if(write(fd[1],"0123456789",10) != 10)
		terror("Writing to pipe");
	alarm(2);
	if((n = sfread(sfstdin,buf,sizeof(buf))) != 10)
		twarn("Wrong read size(%d) after an interrupt\n", n);

	texit(0);
}
