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
#ifdef SF_APPEND
#undef SF_APPEND
#endif

Sfdisc_t Disc;

tmain()
{
	int	n, fd;
	Sfio_t	*f;
	char	*s, buf[1024];
	int	fdv[100];

	buf[0] = 0;
	sfsetbuf(sfstdout,buf,sizeof(buf));
	if(!sfstdout->pool)
		terror("No pool");
	sfdisc(sfstdout,&Disc);
	sfset(sfstdout,SF_SHARE,0);
	sfputr(sfstdout,"123456789",0);
	if(strcmp(buf,"123456789") != 0)
		terror("Setting own buffer for stdout");
	if(sfpurge(sfstdout) < 0)
		terror("Purging sfstdout");

	if((fd = creat(tstfile("sf", 0),0666)) < 0)
		terror("Creating file");

	if(write(fd,buf,sizeof(buf)) != sizeof(buf))
		terror("Writing to file");
	if(lseek(fd, (off_t)0, 0) < 0)
		terror("Seeking back to origin");

	if(!(f = sfnew((Sfio_t*)0,buf,sizeof(buf),fd,SF_WRITE)))
		terror("Making stream");

	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) || s != buf)
		terror("sfreserve1 returns the wrong pointer");
	sfwrite(f,s,0);

#define NEXTFD	12
	if((fd+NEXTFD) < (sizeof(fdv)/sizeof(fdv[0])) )
	{	struct stat	st;
		int		i;
		for(i = 0; i < fd+NEXTFD; ++i)
			fdv[i] = fstat(i,&st);
	}
	if((n = sfsetfd(f,fd+NEXTFD)) != fd+NEXTFD)
		terror("Try to set file descriptor to %d but get %d",fd+NEXTFD,n);
	if((fd+NEXTFD) < (sizeof(fdv)/sizeof(fdv[0])) )
	{	struct stat	st;
		int		i;
		for(i = 0; i < fd+NEXTFD; ++i)
			if(i != fd && fdv[i] != fstat(i,&st))
				terror("Fd %d changes status after sfsetfd %d->%d",
					i, fd, fd+NEXTFD);
	}

	if(!(s = sfreserve(f,SF_UNBOUND,SF_LOCKR)) || s != buf)
		terror("sfreserve2 returns the wrong pointer");
	sfwrite(f,s,0);

	if(sfsetbuf(f,NIL(Void_t*),(size_t)SF_UNBOUND) != buf)
		terror("sfsetbuf didnot returns last buffer");

	sfsetbuf(f,buf,sizeof(buf));

	if(sfreserve(f,SF_UNBOUND,SF_LOCKR) != buf || sfvalue(f) != sizeof(buf) )
		terror("sfreserve3 returns the wrong value");
	sfwrite(f,s,0);

	texit(0);
}
