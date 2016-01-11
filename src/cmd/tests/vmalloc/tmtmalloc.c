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
/*
 *  *  malloc-test: snarf from the web to test multithreaded mallocs. see also tsafemalloc.c
 *   *  cel - Thu Jan  7 15:49:16 EST 1999
 *    *
 *     *  Benchmark libc's malloc, and check how well it
 *      *  can handle malloc requests from multiple threads.
 *       *
 *        *  Syntax:
 *         *  malloc-test [ size [ iterations [ thread count ]]]
 *          *
 *           */

#include "terror.h"

#include <pthread.h>

#define USECSPERSEC		1000000
#define pthread_attr_default	NULL
#define MAX_THREADS		N_THREAD

static unsigned size = 512;
static unsigned iteration_count = 1000000;

static void* dummy(unsigned int i)
{
	return NULL;
}

static void run_test(void)
{
	unsigned int i;
	unsigned request_size = size;
	unsigned total_iterations = iteration_count;
	struct timeval start, end, null, elapsed, adjusted;

	/*
 	** Time a null loop.  We'll subtract this from the final
 	** malloc loop results to get a more accurate value.
 	*/
	gettimeofday(&start, NULL);

	for (i = 0; i < total_iterations; i++)
	{	void * buf;
		buf = dummy(i);
		buf = dummy(i);
	}

	gettimeofday(&end, NULL);

	null.tv_sec = end.tv_sec - start.tv_sec;
	null.tv_usec = end.tv_usec - start.tv_usec;
	if (null.tv_usec < 0)
	{	null.tv_sec--;
		null.tv_usec += USECSPERSEC;
	}

	/*
 	** Run the real malloc test
 	*/
	gettimeofday(&start, NULL);

	for (i = 0; i < total_iterations; i++)
	{	void * buf;
		buf = malloc(request_size);
		free(buf);
	}

	gettimeofday(&end, NULL);

	elapsed.tv_sec = end.tv_sec - start.tv_sec;
	elapsed.tv_usec = end.tv_usec - start.tv_usec;
	if (elapsed.tv_usec < 0)
	{	elapsed.tv_sec--;
		elapsed.tv_usec += USECSPERSEC;
	}

	/*
 	** Adjust elapsed time by null loop time
 	*/
	adjusted.tv_sec = elapsed.tv_sec - null.tv_sec;
	adjusted.tv_usec = elapsed.tv_usec - null.tv_usec;
	if (adjusted.tv_usec < 0)
	{	adjusted.tv_sec--;
		adjusted.tv_usec += USECSPERSEC;
	}
	tinfo("Thread %u adjusted timing: %d.%06d seconds for %d requests of %d bytes",
		pthread_self(), adjusted.tv_sec, adjusted.tv_usec,
		total_iterations, request_size);

	pthread_exit(NULL);
}

tmain()
{
	unsigned i;
	unsigned thread_count = N_THREAD;
	pthread_t thread[MAX_THREADS];

	/*
 	** Parse our arguments
	*/
	switch (argc)
	{
	case 4:
		/* size, iteration count, and thread count were specified */
		thread_count = atoi(argv[3]);
		if (thread_count > MAX_THREADS) thread_count = MAX_THREADS;
	case 3:
		/* size and iteration count were specified; others default */
		iteration_count = atoi(argv[2]);
	case 2:
		/* size was specified; others default */
		size = atoi(argv[1]);
	case 1:
		/* use default values */
		break;
	default:
		terror("Unrecognized arguments");
	}

	if(thread_count <= 0 || thread_count > N_THREAD)
		terror("thread_count=%d must be in 1..%d", thread_count, N_THREAD);

	/*
 	** Invoke the tests
 	*/
	tinfo("Starting test...");
	for (i=1; i<=thread_count; i++)
		if (pthread_create(&(thread[i]), pthread_attr_default, (void *) &run_test, NULL))
			terror("pthread_create() error");

	/*
 	** Wait for tests to finish
 	*/
	for (i=1; i<=thread_count; i++)
		pthread_join(thread[i], NULL);

	texit(0);
}

