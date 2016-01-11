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
#include	"terror.h"
#include	<pthread.h>

#if VMALLOC
#include	"vmalloc.h"
#undef malloc
#undef free
#undef realloc
#define malloc(z)	vmalloc(Vmregion, z)
#define free(d)		vmfree(Vmregion, d)
#define realloc(d,z)	vmresize(Vmregion, d, z, VM_RSCOPY)
#endif

/* This runs a simulation to test the performance of different malloc implementations.
** In general, a simulation will run Nalloc steps. The life of an allocated object
** will be measured in these steps.
**
** For testing performance under concurrency, a number of threads, Nthread, can be
** specified. Then, each thread will perform Nalloc/Nthread steps with objects that
** are completely separated from others.
** In applications such as a database server, there are also emphemeral threads that
** pop up to do some service then go away. To simulate this, a parameter, Empperiod,
** can be specified. Then, for every Empperiod steps, an emphemeral thread will be
** created to perform Empcount allocation operations. All emphemeral threads store
** allocated objects in a shared array of size Empcount. Objects that must be vacated
** for new ones are freed.
**
** The simulation attempts to represent some real class of application situations.
** So, it assumes that there are two types of blocks being allocated: Small and Large
** with the distribution of 'Small'% Small and (100-'Small')% Large. Small blocks
** represent typical objects in a program such as strings, structures representing
** some information to be processed or constructed, etc. On the other hand,
** large objects represent buffers or arrays holding small objects. Allocation
** strategies for these types of block are as follows:
**
** 1. Small block sizes are picked at random from a range [Smalllo, Smallhi].
**    A small block lives for some time (picked randomly in [Smalllf/2,Smalllf]),
**    then either freed or reallocated with a different size. The probability
**    of being reallocated is set at Smallre.  If current size is 'z',
**    the new size is randomly picked in [0, 2*z]. The new block will then
**    be treated in exactly the same way as any other small block.
**
** 2. Large block sizes are treated similarly to Small blocks except that their
**    life time is in the range [Largelf/2, Largelf]) where Largelf far larger
**    than Smalllf and their probability of being reallocated is set at a larger
**    value, Largere. If current size is 'z', the new size is randomly picked
**    in [z/2, 3*z/2]. This is typical for a container array that stores objects
**    whose number dynamically changes.
**
**	Written by Kiem-Phong Vo (01/23/2013).
*/

#define N_EMPHE		10000		/* max #allocs for emphemeral threads	*/

/* below are parameters and default values that drive the simulation */
static size_t		Nthread = N_THREAD;/* number of main threads		*/
static size_t		Nalloc = 10000; /* total number of allocations	*/

static size_t		Smalllo = 10;	/* low end of size range		*/
static size_t		Smallhi = 200;	/* high end of size range		*/
static size_t		Smalllf = 100;	/* #steps to live before free/realloc	*/
static size_t		Smallre = 10;	/* chance of getting reallocated	*/

static size_t		Small = 95;	/* probability of being a Small block	*/

static size_t		Largelo = 4000;	/* low end of size range		*/
static size_t		Largehi = 8000;	/* high end of size range		*/
static size_t		Largelf = 100000; /* life before free or realloc	*/
static size_t		Largere = 80;	/* probability of being reallocated	*/

static size_t		Empperiod = 1000; /* period to make emphemeral threads	*/
static size_t		Empcount = 1000; /* #allocations on each invovation	*/
static Void_t		**Emp;		/* shared array of allocated blocks	*/

/* accounting for space usage */
static size_t		Maxbusy = 0;	/* size of max busy space at any time	*/
static size_t		Curbusy = 0;	/* size of busy space at current time	*/

#ifndef NIL
#define	NIL(t)		((t)0)
#endif

/* 'Rand' is defined locally in each routine that uses it. In this way, the RNG
** runs exactly the same way for each thread. Makes it easy to debug stuff.
*/
#define RANDOM()	(Rand = Rand*((1<<24)+(1<<8)+0x93) + 0xbadbeef)

/* define alignment requirement */
typedef union type_u
{	int		i, *ip;
	short		s, *sp;
	long		l, *lp;
	long long	ll, *llp;
	float		f, *fp;
	double		d, *dp;
	long double	dd, *ddp;
} Type_t;

typedef struct _align_s
{	char	c;
	Type_t	type;
} Align_t;

#define ALIGNMENT	(sizeof(Align_t) - sizeof(Type_t))

typedef struct _piece_s	Piece_t;
struct _piece_s
{	Piece_t*	next;
	Piece_t*	free;
	void*		data;
	size_t		size;
	int		type; /* small or large */
};

typedef struct _thread_s
{	size_t		nalloc;
	Piece_t*	list;
} Tdata_t;

Tdata_t	Tdata[N_THREAD];

void* emphemeral(void* arg) /* an empheral thread, allocate a few times, free then return */
{
	ssize_t	k, sz;
	void	*blk, *emp;
	unsigned int	Rand = 1001; /* use a local RNG so that threads work uniformly */

	for(k = 0; k < Empcount; ++k)
	{	sz = (RANDOM() % (Smallhi-Smalllo+1)) + Smalllo;
		sz = sz > sizeof(size_t) ? sz : sizeof(size_t);

		if(!(blk = (void*)malloc(sz)) )
			terror("Malloc failed");

		*((size_t*)blk) = sz;
		asoaddsize(&Curbusy, sz);
		asomaxsize(&Maxbusy, Curbusy);

		emp = Emp[k]; /* make an attempt to put blk in array */
		if(asocasptr(&Emp[k], emp, blk) == emp)
		{	if(emp) /* free previous block if any */
			{	sz = *((size_t*)emp);
				free(emp);
			}
		}
		else	free(blk); /* couldn't place it so free it */

		asosubsize(&Curbusy, sz);
	}

	return (void*)0;
}

void* simulate(void* arg)
{
	int		k, p, a, ty;
	size_t		sz, nalloc, lf;
	Piece_t		*list, *up, *next;
	void		*data;
	int		thread = (int)((long)arg);
	pthread_t	ethread;
	Tdata_t		*tdata = &Tdata[thread];
	int		warn_align = 1;
	unsigned int	Rand = 1001; /* use a local RNG so that threads work uniformly */

	list = tdata->list;
	nalloc = tdata->nalloc;
	for(k = 0; k < nalloc; ++k)
	{	
		if(Empperiod > 0 && k > 0 && k%Empperiod == 0)
			if(pthread_create(&ethread, NULL, emphemeral, (void*)0) != 0 )
				terror("Can't create emphemeral thread");

		/* update all that needs updating */
		for(up = list[k].free; up; up = next)
		{	next = up->next;

			if(k == nalloc-1 ||
			   (up->type == 0 && (RANDOM()%100) >= Smallre) ||
			   (up->type == 1 && (RANDOM()%100) >= Largere) )
			{	free(up->data);

				asosubsize(&Curbusy, up->size);
				up->data = NIL(void*);
				up->size = 0;
			}
			else /* realloc: new size's range is [up->size/2, 2*up->size] */
			{	if(up->type == 0)
					sz = RANDOM()%(2*up->size) + 1;
				else	sz = up->size/2 + RANDOM()%up->size + 1;

				if(!(up->data = realloc(up->data, sz)) )
					terror("Thread %d: bad realloc(org=%d sz=%d)", up->size, sz);
				if((a = (unsigned long)(list[k].data) % ALIGNMENT) != 0)
					terror("Thread %d: block=%#0x mod %d == %d",
						thread, list[k].data, ALIGNMENT, a);

				asosubsize(&Curbusy, up->size);
				asoaddsize(&Curbusy, sz);
				asomaxsize(&Maxbusy, Curbusy);
				up->size = sz;

				/* get a random point in the future to update */
				if(up->type == 0)
					lf = Smalllf/2 + RANDOM()%(Smalllf/2);
				else	lf = Largelf/2 + RANDOM()%(Largelf/2);
				if((p = k+1 + lf) >= nalloc)
					p = nalloc-1;
				up->next = list[p].free;
				list[p].free = up;
			}
		}

		/* get a random size in given range */
		if(RANDOM()%100 < Small)
		{	sz = RANDOM()%(Smallhi-Smalllo+1) + Smalllo;
			lf = Smalllf/2 + RANDOM()%(Smalllf/2);
			ty = 0; /* small block */
		}
		else
		{	sz = RANDOM()%(Largehi-Largelo+1) + Largelo;
			lf = Largelf/2 + RANDOM()%(Largelf/2);
			ty = 1; /* large block */
		}

		if(!(list[k].data = malloc(sz)) )
			terror("Thread %d: failed to malloc(%d)", thread, sz);
		if((a = (unsigned long)(list[k].data) % ALIGNMENT) != 0)
		{	if(warn_align)
				tinfo("Thread %d: block=%#0x mod %d == %d", thread, list[k].data, ALIGNMENT, a);
			warn_align = 0;
		}
			
		asoaddsize(&Curbusy, sz);
		asomaxsize(&Maxbusy, Curbusy);
		list[k].size = sz;
		list[k].type = ty;

		/* set time to update */
		if((p = k+1 + lf) >= nalloc )
			p = nalloc-1;
		list[k].next = list[p].free;
		list[p].free = &list[k];

		if(Empperiod > 0 && k > 0 && k%Empperiod == 0)
			if(pthread_join(ethread, 0) != 0 )
				terror("Can't wait for emphemeral thread");
	}

	return (void*)0;
}

tmain()
{
	int		i, arg1, arg2, arg3, arg4, nalloc, rv;
	size_t		sz;
	void		*status;
	pthread_t	th[N_THREAD];

	for(; argc > 1; --argc, ++argv)
	{	if(argv[1][0] != '-')
			continue;
		else if(argv[1][1] == 'a') /* number of allocation steps */
			Nalloc = atoi(argv[1]+2);
		else if(argv[1][1] == 't') /* number of threads */
			Nthread = atoi(argv[1]+2);
		else if(argv[1][1] == 's') /* probability of being Small vs Large */
			Small = atoi(argv[1]+2);
		else if(argv[1][1] == 'z')
		{	sscanf(argv[1]+2, "%d,%d,%d,%d", &arg1, &arg2, &arg3, &arg4);
			Smalllo = arg1; /* lo of size range */
			Smallhi = arg2; /* hi of size range */
			Smalllf = arg3; /* life before free or realloc */
			Smallre = arg4; /* probability of being realloced */
		}
		else if(argv[1][1] == 'Z')
		{	sscanf(argv[1]+2, "%d,%d,%d,%d", &arg1, &arg2, &arg3, &arg4);
			Largelo = arg1;
			Largehi = arg2;
			Largelf = arg3;
			Largere = arg4;
		}
		else if(argv[1][1] == 'e')
		{	sscanf(argv[1]+2, "%d,%d", &arg1, &arg2);
			Empperiod = arg1; /* number of alloc steps between an emphemeral thread */
			Empcount = arg2; /* number of allocations to perform */
		}
	}

	if(Nthread <= 0 || Nthread > N_THREAD)
		terror("Nthread=%d must be in 1..%d", Nthread, N_THREAD);

	tresource(-1, 0);

	nalloc = Nalloc/Nthread; /* #of allocations per thread */
	Nalloc = nalloc*Nthread;

	/* space for emphemeral threads */
	if(Empperiod > 0)
	{	sz = Empcount*sizeof(Void_t*);
		if(!(Emp = malloc(sz)) )
			terror("Failed allocating shared list of emphemeral objects nalloc=%d", nalloc );
		memset(Emp, 0, sz);
		Curbusy += sz;
	}
		
	for(i = 0; i < Nthread; ++i)
	{	Tdata[i].nalloc = nalloc;
		sz = Tdata[i].nalloc*sizeof(Piece_t);
		if(!(Tdata[i].list = (Piece_t*)malloc(sz)) )
			terror("Failed allocating list of objects nalloc=%d", Tdata[i].nalloc);
		memset(Tdata[i].list, 0, Tdata[i].nalloc*sizeof(Piece_t));
		Curbusy += sz;
	}
	Maxbusy = Curbusy;

	tinfo("Nstep=%d Nthread=%d ProbSmall=%d, Emphe(p=%d,n=%d)",
		Nalloc, Nthread, Small, Empperiod,Empcount);
	tinfo("\tSmall(min=%d,max=%d,life=%d,probre=%d) Large(min=%d,max=%d,life=%d,probre=%d)",
		Smalllo, Smallhi, Smalllf, Smallre, Largelo, Largehi, Largelf, Largere);

	for(i = 0; i < Nthread; ++i)
	{	if((rv = pthread_create(&th[i], NULL, simulate, (void*)((long)i))) != 0 )
			terror("Failed to create simulation thread %d", i);
	}

	for(i = 0; i < Nthread; ++i)
	{	if((rv = pthread_join(th[i], &status)) != 0 )
			terror("Failed waiting for simulation thread %d", i);
	}

	tresource(0, Maxbusy);

#if VMALLOC
	{	Vmstat_t	vmst;
		vmstat(Vmregion, &vmst);
		tinfo(vmst.mesg);
	}
#endif

	texit(0);
}
