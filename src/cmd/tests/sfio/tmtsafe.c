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

#if vt_threaded

#include	<vthread.h>

#define N_STR	1000

static Sfio_t*	Sf;
static char	Bigz[10*N_STR];
static char*	Str[26] =
	{	"aaaaaaaaa",
	 	"bbbbbbbbb",
	 	"ccccccccc",
	 	"ddddddddd",
	 	"eeeeeeeee",
	 	"fffffffff",
	 	"ggggggggg",
	 	"hhhhhhhhh",
	 	"iiiiiiiii",
	 	"jjjjjjjjj",
	 	"kkkkkkkkk",
	 	"lllllllll",
	 	"mmmmmmmmm",
	 	"nnnnnnnnn",
	 	"ooooooooo",
	 	"ppppppppp",
	 	"qqqqqqqqq",
	 	"rrrrrrrrr",
	 	"sssssssss",
	 	"ttttttttt",
	 	"uuuuuuuuu",
	 	"vvvvvvvvv",
	 	"wwwwwwwww",
	 	"xxxxxxxxx",
	 	"yyyyyyyyy",
	 	"zzzzzzzzz"
	};

static int	Inverted;	/* failure means success */

#if _STD_C
void* writesmall(void* arg)
#else
void* writesmall(arg)
void*	arg;
#endif
{
	char*	s;
	int	n;

	s = Str[integralof(arg)];
	for(n = 0; n < N_STR; ++n)
	{	if(sfputr(Sf, s, '\n') != 10)
		{	if(Inverted)
				tsuccess("sfputr failed as expected");
			else	terror("sfputr failed");
		}
	}

	return arg;
}

#if _STD_C
void* writebig(void* arg)
#else
void* writebig(arg)
void*	arg;
#endif
{
	int	r = (rand()%3) + 1;	sleep(r);

	if(sfwrite(Sf,Bigz,sizeof(Bigz)) != sizeof(Bigz))
		terror("Writing bigz");

	return arg;
}

#if __STD_C
void sighandler(int sig)
#else
void sighandler(sig)
int     sig;
#endif
{
	tmesg("\tSignal %d.\n", sig);
	texit(0);
}

tmain()
{
	int		count[26];
	char*		s;
	int		i, k, n;
	Vthread_t*	thread[26];

	/* make the big z string */
	for(i = 0, s = Bigz; i < N_STR; ++i, s += 10)
		strcpy(s, "zzzzzzzzz\n");

	signal(SIGQUIT,sighandler);
	signal(SIGINT,sighandler);

do_inverted: /* get back to here when trying to make things fail */

	if(!Inverted)
		tmesg("\tTesting thread-safe streams.\n");
	else	tmesg("\tTesting unsafe streams: if hung, send INTR or QUIT.\n");

	/* spin threads writing small chunks */
	Sf = sfopen(NIL(Sfio_t*),tstfile("sf", 0), Inverted ? "w+" : "mw+");

	for(i = 0; i < 26; ++i)
	{	if(!(thread[i] = vtopen(0, 0)) )
			terror("Creating thread handle[%d]", i);
 		if(vtrun(thread[i], writesmall, (Void_t*)i) < 0)
			terror("Running thread [%d]", i);
	}

	for(i = 0; i < 26; ++i)
	{	count[i] = 0;
		vtwait(thread[i]);
	}

	if(sfseek(Sf,(Sfoff_t)0,SEEK_SET) != (Sfoff_t)0)
	{	if(Inverted)
			tsuccess("Rewinding failed as expected");
		else	terror("Rewinding");
	}

	for(n = 0;; ++n)
	{	if(!(s = sfgetr(Sf,'\n',1)) )
			break;

		i = s[0] - 'a';
		if(i < 0 || i >= 26 || sfvalue(Sf) != 10)
		{	if(Inverted)
				tsuccess("Bad data as expected");
			else	terror("Bad data s='%s' n=%d", s, n);
		}
		if(strcmp(s, Str[i]) != 0)
		{	if(Inverted)
				tsuccess("Bad str as expected");
			else	terror("Bad str s='%s' i=%d Str[i]='%s' n=%d",
					s, i, Str[i], n);
		}

		count[i] += 1;
	}

	for(i = 0; i < 26; ++i)
	{	if(count[i] != N_STR)
		{	if(Inverted)
				tsuccess("Bad count as expected");
			else	terror("Bad count[%d] = %d", i, count[i]);
		}
	}

	/* spin threads with one writing a big chunk */
	Sf = sfopen(Sf,tstfile("sf", 0), Inverted ? "w+" : "mw+");

	for(i = 0; i < 25; ++i)
	{	if(!(thread[i] = vtopen(0, 0)) )
			terror("Creating thread %d", i);
		if(vtrun(thread[i], writesmall, (void*)i) < 0)
			terror("Running thread %d", i);
	}
	sleep(1);
	if(!(thread[i] = vtopen(0,0)))
		terror("Creating big thread z");
	if(vtrun(thread[i],writebig,(void*)i) < 0)
		terror("Running big thread z");

	for(i = 0; i < 26; ++i)
	{	count[i] = 0;
		vtwait(thread[i]);
	}

	if(sfseek(Sf,(Sfoff_t)0,SEEK_SET) != (Sfoff_t)0)
	{	if(Inverted)
			tsuccess("Rewinding failed as expected");
		else	terror("Rewinding");
	}

	for(n = 0; ; ++n)
	{	if(!(s = sfgetr(Sf,'\n',1)) )
			break;

		i = s[0] - 'a';
		if(i < 0 || i >= 26 || sfvalue(Sf) != 10)
		{	if(Inverted)
				tsuccess("Bad data as expected");
			else	terror("Bad data s='%s' n=%d", s, n);
		}
		if(strcmp(s, Str[i]) != 0)
		{	if(Inverted)
				tsuccess("Bad str as expected");
			else	terror("Bad str s='%s' i=%d Str[i]='%s' n=%d",
					s, i, Str[i], n);
		}
		count[i] += 1;

		if(i == 25) /* the 'z' */
		{	for(k = 1; k < N_STR; ++k, ++n)
			{	if(!(s = sfgetr(Sf,'\n',1)) )
				{	if(Inverted)
						tsuccess("Premature eof as expected");
					else	terror("Premature eof n=%d", n);
				}
				if(strcmp(s, Str[25]) != 0)
				{	if(Inverted)
						tsuccess("Bad str as expected");
					else	terror("Bad str s='%s' n=%d", s, n);
				}
				count[i] += 1;
			}
		}
	}

	for(i = 0; i < 26; ++i)
	{	if(count[i] != N_STR)
		{	if(Inverted)
				tsuccess("Bad count as expected");
			else	terror("Bad count[%d] = %d", i, count[i]);
		}
	}

	if(!Inverted)
	{	Inverted = 1;
		goto do_inverted;
	}
	else	tmesg("\tUnsafe streams work ok on this platform!\n");

	texit(0);
}

#else

tmain()
{
	texit(0);
}

#endif
