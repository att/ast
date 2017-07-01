/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2012 AT&T Intellectual Property          *
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

static int	Write_error = 0;

#if __STD_C
static int except(Sfio_t* f, int type, Void_t* obj, Sfdisc_t* disc)
#else
static int except(f, type, obj, disc)
Sfio_t*	f;
int	type;
Void_t* obj;
Sfdisc_t* disc;
#endif
{
	if(type == SF_WRITE)
		Write_error = 1;
	return 0;
}

static Sfdisc_t Wdisc = {NIL(Sfread_f), NIL(Sfwrite_f), NIL(Sfseek_f), except};

tmain()
{
	int	fd[2];
	Sfio_t	*r, *w;
	char*	s;
	void(*	handler)_ARG_((int));

#define N_STR	10
	alarm(10);
	if(argc > 1) /* to act as a coprocess that read/write ten lines */
	{	int	i, n, rv;

		n = atoi(argv[1]);
		for(i = 0; i < n; ++i)
		{	if(!(s = sfgetr(sfstdin,'\n',1)) )
				terror("Failed to read from stdin");
			if((rv = sfputr(sfstdout, s, '\n')) != sfvalue(sfstdin))
				terror("Failed to write rv=%d stdin=%d",
					rv, sfvalue(sfstdin));
		}
		sfsync(sfstdout);
		texit(0);
	}

	signal(SIGPIPE,SIG_IGN);

	if(pipe(fd) < 0)
		terror("Opening pipes");

	if(!(w = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND, fd[1],SF_WRITE)) )
		terror("Opening write stream");
	if(!(r = sfnew(NIL(Sfio_t*),NIL(Void_t*),(size_t)SF_UNBOUND, fd[0],SF_READ)) )
		terror("Opening read stream");

	sfdisc(w,&Wdisc);

	if(sfputr(w,"abc",'\n') != 4)
		terror("sfputr failed");
	if(!(s = sfgetr(r,'\n',1)) || strcmp(s,"abc") != 0)
		terror("sfgetr failed");

	if(sfclose(r) < 0)
		terror("sfclose failed closing read stream");

	if(sfputr(w,"def",'\n') != 4)
		terror("sfputr failed2");

	if(Write_error)
		terror("Write exception should not have been raised");

	if(sfclose(w) >= 0)
		terror("sfclose should have failed closing write stream");

	if(!Write_error)
		terror("Write exception did not get raised");

	signal(SIGPIPE,SIG_DFL);
	if((w = sfpopen(NIL(Sfio_t*), sfprints("%s %d", argv[0], N_STR), "w+")) )
	{	int	i;

		if((handler = signal(SIGPIPE,SIG_IGN)) == SIG_DFL ||
		   handler == SIG_IGN)
			terror("Bad signal handler for SIGPIPE");
		signal(SIGPIPE,handler);

		Write_error = 0;
		sfdisc(w,&Wdisc);

		for(i = 0; i < N_STR*10; ++i)
			if(sfputr(w, "abc",'\n') != 4)
				terror("Writing to coprocess1");

		sfsync(w);
		sfset(w,SF_READ,1);
		i = sffileno(w);
		sfset(w,SF_WRITE,1);
		if (i != sffileno(w))
			close(sffileno(w));

		for(i = 0; i < N_STR; ++i)
			if(!(s = sfgetr(w,'\n',1)) || strcmp(s,"abc") != 0)
				terror("Reading coprocess [%s]", s);
		if((s = sfgetr(w,'\n',1)) )
			terror("sfgetr should have failed");

		if(sfputr(w, "abc",'\n') != 4)
			terror("Writing to coprocess2");

		if(Write_error)
			terror("Write exception should not have been raised yet");

		if(sfclose(w))
			terror("sfclose should not have returned error status");

		if(!Write_error)
			terror("Write exception should have been raised");
	}

	if(signal(SIGPIPE,SIG_DFL) != SIG_DFL)
		terror("SIGPIPE handler should have been SIG_DFL");

	/* test for stdio signal handling behavior */
	w = sfpopen((Sfio_t*)(-1), sfprints("%s %d 2>/dev/null",argv[0],N_STR), "w+");
	if(w && (handler = signal(SIGPIPE,SIG_DFL)) != SIG_DFL)
		terror("SIGPIPE handler should have been SIG_DFL");

	texit(0);
}
