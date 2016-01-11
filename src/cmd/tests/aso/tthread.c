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

#include <pthread.h>
#include <sys/time.h>

#ifndef elementsof
#define elementsof(x)		(sizeof(x)/sizeof(x[0]))
#endif
#ifndef integralof
#define integralof(x)	(((char*)(x))-((char*)0))
#endif

#define N_OBJS		1000000
#define N_THREADS	8

typedef struct _obj_s
{	struct _obj_s*	next;
	int		value;
} Obj_t;

static Obj_t*		Objlist;	/* list of objects	*/
 
static unsigned int	Asolock;

static unsigned int	N_free;
 
static void* consumer(void* arg)
{
	Obj_t*		obj;
	unsigned int	id = (unsigned int)integralof(arg);
 
	while (1)
	{	asolock(&Asolock, id, ASO_LOCK);

		if((obj = Objlist))
		{	Objlist = obj->next;
			free(obj);
			N_free += 1;
		}

		asolock(&Asolock, id, ASO_UNLOCK);

		if(!obj)
			return NULL;
	}
}
 
tmain()
{
	int		i, m;
	Obj_t		*obj;
	char		*lockid;
	pthread_t	thread[N_THREADS];
	struct timeval	tv1, tv2;
 
	topts();
	lockid = tstfile("aso", 0);
	tinfo("testing with %d threads", N_THREADS);
	Asolock = 0;
	/* create object list */
	for (i = 0; i < N_OBJS; i++)
	{	if(!(obj = malloc(sizeof(Obj_t))) )
			terror("malloc failed");
		obj->value = i;
		obj->next = Objlist;
		Objlist = obj;
	}
 
	/* time before starting the threads... */
	gettimeofday(&tv1, NULL);

	N_free = 0;
	for(i = 0; i < N_THREADS; ++i)
		pthread_create(&thread[i], NULL, consumer, (char*)0 + i + 1);
 
	for(i = 0; i < N_THREADS; ++i)
		pthread_join(thread[i], NULL);
 
	/* time after threads finished... */
	gettimeofday(&tv2, NULL);

	if(N_free != N_OBJS)
		terror("free() call error -- expected %d, got %d", N_OBJS, N_free);
 
	if (tv1.tv_usec > tv2.tv_usec)
	{
    		tv2.tv_sec--;
    		tv2.tv_usec += 1000000;
	}
 
	tinfo("elapsed time %ld.%lds",
		tv2.tv_sec - tv1.tv_sec, tv2.tv_usec - tv1.tv_usec);
 
	texit(0);
}
