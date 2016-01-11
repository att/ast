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

int	Code_line = 30; /* line number of CALL(sfclose(0)) */

#if defined(__LINE__)
#define CALL(x)	((Code_line = __LINE__), (x))
#else
#define CALL(x)	((Code_line += 1),(x))
#endif

#if __STD_C
void handler(int sig)
#else
void handler(sig)
int	sig;
#endif
{
	terror("Bad argument handling on code line %d", Code_line);
}


tmain()
{	
	signal(SIGILL,handler);
	signal(SIGBUS,handler);
	signal(SIGSEGV,handler);

	CALL(sfclose(0));
	CALL(sfclrlock(0));
	CALL(sfopen(0,0,0));
	CALL(sfdisc(0,0));
	CALL(_sffilbuf(0,0));
	CALL(_sfflsbuf(0,0));
	CALL(sfgetd(0));
	CALL(sfgetl(0));
	CALL(sfgetm(0,0));
	CALL(sfgetr(0,0,0));
	CALL(sfgetu(0));
	CALL(sfmove(0,0,0,0));
	CALL(sfmutex(0,0));
	CALL(sfnew(0,0,0,0,0));
	CALL(sfnputc(0,0,0));
	CALL(sfopen(0,0,0));
	CALL(sfpoll(0,0,0));
	CALL(sfpool(0,0,0));
	CALL(sfpopen(0,0,0));
	CALL(sfprintf(0,0));
	CALL(sfvsprintf(0,0,0,0));
	CALL(sfsprintf(0,0,0));
	CALL(sfprints(0));
	CALL(sfpurge(0));
	CALL(sfputd(0,0));
	CALL(sfputl(0,0));
	CALL(sfputm(0,0,0));
	CALL(sfputr(0,0,0));
	CALL(sfputu(0,0));
	CALL(sfraise(0,0,0));
	CALL(sfrd(0,0,0,0));
	CALL(sfread(0,0,0));
	CALL(sfreserve(0,0,0));
	CALL(sfresize(0,0));
	CALL(sfscanf(0,0));
	CALL(sfvsscanf(0,0,0));
	CALL(sfsscanf(0,0));
	CALL(sfseek(0,0,0));
	CALL(sfset(0,0,0));
	CALL(sfsetbuf(0,0,0));
	CALL(sfsetfd(0,0));
	CALL(sfsize(0));
	CALL(sfsk(0, 0, 0, 0));
	CALL(sfstack(0, 0));
	CALL(sfswap(0, 0));
	CALL(sfsync(0));
	CALL(sftell(0));
	CALL(sftmp(0));
	CALL(sfungetc(0,0));
	CALL(sfvprintf(0,0,0));
	CALL(sfvscanf(0,0,0));
	CALL(sfwr(0,0,0,0));
	CALL(sfwrite(0,0,0));

	texit(0);
}
