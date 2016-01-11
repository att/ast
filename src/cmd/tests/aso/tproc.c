/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2013 AT&T Intellectual Property          *
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
#include "terror.h"

#ifndef N_PROC
#define N_PROC		8
#endif

#define N_STEP		1000	/* #working steps	*/
#define N_REP		1000	/* #repeats per step	*/

static unsigned int	*Lock;	/* Asolock */

/* all threads wait until this is == N_PROC */
static unsigned int	*Active; /* active processes	*/

/* Count is the shared resource updated by different threads. */
static unsigned int	*Count;

static void workload(unsigned int pid)
{
	int		k, r;

	asoincint(Active); /* indicate that we are active */
	while(*Active < N_PROC) /* wait for all to be ready */
		asorelax(1);

	for(k = 0; k < N_STEP; ++k)
		for(r = 0; r < N_REP; ++r)
			asoincint(Count);
}

tmain()
{
	ssize_t		k;
	int		status;
	int		type;
	char		*lockid;
	pid_t		pid, cpid[N_PROC];
	struct timeval	tv1, tv2;

	tchild();

	Lock = (unsigned int*)tshared(3*sizeof(unsigned int)); /* this is used by asolock() */
	Count = Lock+1; /* this is the shared counter to be updated asynchronously */
	Active = Count+1; /* this counter sets all processes to work at the same time */

	lockid = tstfile("aso", 0);
	tinfo("testing with %d processes", N_PROC);
	gettimeofday(&tv1, 0); /* start the timer */

	*Lock = 0;
	*Count = 0;
	*Active = 0;

#if N_PROC > 1
	for(k = 0; k < N_PROC; ++k)
		if((pid = fork()) < 0)
			terror("Can't create a child process");
		else if(pid > 0) /* parent process */
		{	cpid[k] = pid;
			continue;
		}
		else /* child process */
		{	workload((unsigned int)getpid()); /* now start working concurrently */
			texit(0);
		}

	if (twait(cpid, N_PROC))
		terror("workload subprocess error");
#else
	workload((unsigned int)getpid());
#endif

	if(*Lock != 0)
		terror("Aso lock is still held by %d", *Lock);
	if(*Count != N_PROC*N_STEP*N_REP)
		terror("count error -- expected %d, got %d", N_PROC*N_STEP*N_REP, *Count);

	gettimeofday(&tv2, 0); /* get end of time */

	if (tv1.tv_usec > tv2.tv_usec)
	{	tv2.tv_sec -= 1;
    		tv2.tv_usec += 1000000;
	}
 
	tinfo("elapsed time %ld.%lds",
		tv2.tv_sec - tv1.tv_sec, tv2.tv_usec - tv1.tv_usec);

	texit(0);
}
