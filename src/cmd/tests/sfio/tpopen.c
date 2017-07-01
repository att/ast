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

tmain()
{
	Sfio_t	*f;
	char	*s, *endos, *os = "one\ntwo\nthree\n";
	int	n;
	void(* handler)_ARG_((int));

	if(argc > 1)
	{	sfmove(sfstdin,sfstdout,(Sfoff_t)(-1),-1);
		return 0;
	}

	if(!(f = sfpopen((Sfio_t*)0, sfprints("%s -p > %s", argv[0], tstfile("sf", 0)), "w")))
		terror("Opening for write");
	if(sfwrite(f,os,strlen(os)) != (ssize_t)strlen(os))
		terror("Writing");

#ifdef SIGPIPE
	if((handler = signal(SIGPIPE,SIG_DFL)) == SIG_DFL)
		terror("Wrong signal handler");
	if((handler = signal(SIGPIPE,handler)) != SIG_DFL)
		terror("Weird signal handling");
#endif

	sfclose(f);

	if(!(f = sfpopen((Sfio_t*)0, sfprints("%s -p < %s", argv[0], tstfile("sf", 0)), "r")))
		terror("Opening for read");
	sleep(1);

	endos = os + strlen(os);
	while(s = sfgetr(f,'\n',0))
	{	n = sfvalue(f);
		if(strncmp(s,os,n) != 0)
		{	s[n-1] = os[n-1] = 0;
			terror("Input=%s, Expect=%s",s,os);
		}
		os += n;
	}

	if(os != endos)
		terror("Does not match all data, left=%s",os);

	texit(0);
}
