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

/* This test was from the Hoard package. It tests cache locality. */

#define		OBJSIZE		10
#define		REPETITIONS	10
#define		ITERATIONS	10000

typedef struct _worker_s
{
	char*	object;
	int	objsize;
	int	repetitions;
	int	iterations;
} Worker_t;

static void* worker(void* arg)
{
	int		i, j, k;
	Worker_t	*w = (Worker_t*)arg;

	if(w->object)
		free(w->object);

	for(i = 0; i < w->iterations; ++i)
	{	char	*obj;

		if(!(obj = (char*)malloc(w->objsize)) )
			terror("malloc failed");

		/* write into obj a bunch of times */
		for(j = 0; j < w->repetitions; ++j)
		{	for(k = 0; k < w->objsize; ++k)
			{	volatile char	ch;
				obj[k] = (char)k;
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
	char		*objs[N_THREAD];
	pthread_t	thread[N_THREAD];
	int		nthreads = N_THREAD;
	int		iterations = ITERATIONS;
	int		objsize = OBJSIZE;
	int		repetitions = REPETITIONS;
#ifdef VMALLOC
	Vmstat_t	vmst;
#endif

	tresource(-1, 0);

	while(argc > 1)
	{	if(argv[1][0] == '-' && argv[1][1] == 't')
			nthreads = atoi(argv[1]+2);
		else if(argv[1][0] == '-' && argv[1][1] == 'i')
			iterations = atoi(argv[1]+2);
		else if(argv[1][0] == '-' && argv[1][1] == 'r')
			repetitions = atoi(argv[1]+2);
		else if(argv[1][0] == '-' && argv[1][1] == 'z')
			objsize = atoi(argv[1]+2);
		argc--; argv++;
	}

	if(nthreads <= 0 || nthreads > N_THREAD)
		terror("nthreads=%d must be in 1..%d", nthreads, N_THREAD);
	if(repetitions < nthreads)
		repetitions = 0;

	tinfo("nthreads=%d iterations=%d objsize=%d repetitions=%d",
		nthreads, iterations, objsize, repetitions );

	for(i = 0; i < nthreads; ++i)
		if(!(objs[i] = (char*)malloc(objsize)) )
			terror("Can't allocate objs[%d]", i);

	for(i = 0; i < nthreads; ++i)
	{	Worker_t	*w = (Worker_t*)malloc(sizeof(Worker_t));
		w->object = objs[i];
		w->objsize = objsize;
		w->iterations = iterations;
		w->repetitions = repetitions/nthreads;
		if((rv = pthread_create(&thread[i], NULL, worker, (void*)w)) != 0)
			terror("Failed to create thread %d", i);
	}
	for(i = 0; i < nthreads; ++i)
		if((rv = pthread_join(thread[i], &status)) != 0)
			terror("Failed waiting for thread %d", i);

	tresource(0, 0);

#ifdef VMALLOC
	vmstat(0, &vmst);
	twarn(vmst.mesg);
#endif

	texit(0);
}
