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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"vmtest.h"
#include	<pthread.h>

#ifndef N_THREAD
#define N_THREAD	32
#endif
#if N_THREAD > 32
#undef	N_THREAD
#define N_THREAD	32
#endif

#define N_ALLOC		5000

typedef struct _thread_s
{	void*	list[N_ALLOC];
	size_t	size[N_ALLOC];
} Tdata_t;

Tdata_t	Tdata[N_THREAD];

void* allocate(void* arg)
{
	int		k, p;
	size_t		size, total;
	int		thread = (int)((long)arg);
	Tdata_t		*tdata = &Tdata[thread];

	total = 0;
	for(k = 0; k < N_ALLOC; ++k)
	{	size = random() % 1024 + 1;
		if(!(tdata->list[k] = malloc(size)) )
			terror("Thread %d: failed to malloc(%d), total=%d", thread, size, total);
		else
		{	tdata->size[k] = size;
			total += size;
			memset(tdata->list[k], 1, size);
		}

		if(k > 10 )
		{	p = random() % k;
			if(tdata->list[p])
			{	if(random()%4 == 0 )
				{	size = 2*tdata->size[p];
					if(!(tdata->list[p] = realloc(tdata->list[p], size)) )
						terror("Thread %d: failed to realloc(%d), total=%d",
							thread, size, total);
					else
					{	tdata->size[p] = size;
						total += size/2;
						memset(tdata->list[p], 255, size);
					}
				}
				else
				{	free(tdata->list[p]);
					tdata->list[p] = 0;
					tdata->size[p] = 0;
				}
			}
		}

		if(k > 0 && k%(N_ALLOC/4) == 0)
			tinfo("Thread %d: k=%d", thread, k);
	}

	return (void*)0;
}

tmain()
{
	int		i, rv;
	void		*status;
	pthread_t	th[N_THREAD];
	Vmstat_t	vmst;

	topts();
	taso(ASO_THREAD);

	if(argc > 1) /* set max number of regions to avoid collisions */
		setregmax(atoi(argv[1]));

	for(i = 0; i < N_THREAD; ++i)
	{	if((rv = pthread_create(&th[i], NULL, allocate, (void*)((long)i))) != 0 )
			terror("Failed to create thread %d", i);
		tinfo("Thread %d was created", i);
	}

	for(i = 0; i < N_THREAD; ++i)
	{	if((rv = pthread_join(th[i], &status)) != 0 )
			terror("Failed waiting for thread %d", i);
		tinfo("Thread %d return error %ld", i, (long)status);
	}

	vmstat((Vmalloc_t*)0, &vmst);
	tinfo("#regions=%d #open-regions=%d #busy-regions=%d #probes=%d",
		vmst.n_region, vmst.n_open, vmst.n_lock, vmst.n_probe);
	tinfo("n_busy=%d n_free=%d s_busy=%d s_free=%d n_seg=%d extent=%d",
		vmst.n_busy, vmst.n_free, vmst.s_busy, vmst.s_free, vmst.n_seg, vmst.extent );

	texit(0);
}
