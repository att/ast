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
#include	"vmtest.h"
#include	<pthread.h>

/* This test was from the Hoard package. It tests cache-locality.  */

#define		OBJSIZE		10
#define		REPETITION	10
#define		ITERATION	10000

#ifdef VMALLOC
#define		malloc(x)	vmalloc(Vmregion, (x))
#define		free(x)		vmfree(Vmregion, (x))
#define		realloc(x,z)	vmresize(Vmregion, (x), (z), VM_RSCOPY)
#endif

typedef struct _worker_s
{
	int	objsize;
	int	repetition;
	int	iteration;
	int	filler[16];
} Worker_t;

static void* worker(void* arg)
{
	int		i, j, k;
	Worker_t	*w = (Worker_t*)arg;

	for(i = 0; i < w->iteration; ++i)
	{	char	*obj;

		if(!(obj = (char*)malloc(w->objsize)) )
			terror("malloc failed");

		/* write into obj a bunch of times */
		for(j = 0; j < w->repetition; ++j)
		{	for(k = 0; k < w->objsize; ++k)
			{	volatile char	ch;
				obj[k] = 'a';
				ch = obj[k];
				ch += 1;
			}
		}

		free(obj);
	}

	free(w);

	return (void*)0;
}

tmain()
{
	int		i, rv;
	void		*status;
	pthread_t	thread[N_THREAD];
	int		nthreads = N_THREAD;
	int		iteration = ITERATION;
	int		objsize = OBJSIZE;
	int		repetition = REPETITION;

	for(; argc > 1; --argc, ++argv)
	{	if(argv[1][0] != '-')
			continue;
		else if(argv[1][1] == 't')
			nthreads = atoi(argv[1]+2);
		else if(argv[1][1] == 'z')
			objsize = atoi(argv[1]+2);
		else if(argv[1][1] == 'i')
			iteration = atoi(argv[1]+2);
		else if(argv[1][1] == 'r')
			repetition = atoi(argv[1]+2);
	}

	if(nthreads <= 0 || nthreads > N_THREAD)
		terror("nthreads=%d must be in 1..%d", nthreads, N_THREAD);
	if(repetition < nthreads)
		repetition = 0;

	tinfo("nthreads=%d iteration=%d objsize=%d repetition=%d",
		nthreads, iteration, objsize, repetition );

	tresource(-1, 0);

	for(i = 0; i < nthreads; ++i)
	{	Worker_t	*w = (Worker_t*)malloc(sizeof(Worker_t));
		w->objsize = objsize;
		w->iteration = iteration;
		w->repetition = repetition/nthreads;
		if((rv = pthread_create(&thread[i], NULL, worker, (void*)w)) != 0)
			terror("Failed to create thread %d", i);
	}
	for(i = 0; i < nthreads; ++i)
		if((rv = pthread_join(thread[i], &status)) != 0)
			terror("Failed waiting for thread %d", i);

	tresource(0, 0);

#ifdef VMALLOC
{	Vmstat_t vmst;
	vmstat(0, &vmst);
	tinfo(vmst.mesg);
}
#endif

	texit(0);
}
