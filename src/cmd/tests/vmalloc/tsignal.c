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

#include	<setjmp.h>

#ifndef VMALLOC
#define VMALLOC	1
#endif
#if VMALLOC
#include	<vmalloc.h>
#endif

static int		Nalloc = 100000;
static int		Life = 100;
static size_t		Minsize = 100;
static size_t		Maxsize = 1000;

#ifndef NIL
#define	NIL(t)		((t)0)
#endif

typedef struct _piece_s	Piece_t;
struct _piece_s
{	Piece_t*	next;
	Piece_t*	free;
	void*		data;
	size_t		size;
};

static int		Step = 0; /* number of allocation steps done	*/
static int		Allocing = 0; /* longjmp while in a malloc call */
static int		Nlongjmp = 0; /* total number of such jumps	*/
static sigjmp_buf	Jmpbuf;

static void sighup(int sig)
{
	if(Allocing)
	{
		Allocing = 0; /* reset state now to reduce unnecessary longjmps */
#if VMALLOC
		vmclrlock(0); /* unnecessary when vmalloc wraps siglongjmp() */
#endif
		Nlongjmp += 1;

		signal(sig, sighup); /* restore signal handler */

		siglongjmp(Jmpbuf, SIGHUP);
	}
	else	signal(sig, sighup);
}

static void sigalarm(int sig)
{
#if VMALLOC
	{	Vmstat_t	vmst;
		vmstat(Vmregion, &vmst);
		tinfo(vmst.mesg);
	}
#endif

	terror("Process hang after running step=%d: SIGALRM was received", Step);
}

int simulate(Piece_t* list, size_t nalloc)
{
	int		k, p;
	unsigned int	rand;
	size_t		sz;
	Piece_t		*up, *next;
	void		*data;
	int		nfree, nresize;
	unsigned int	Rand = 1001; /* use a local RNG so that threads work uniformly */
#define RANDOM()	(Rand = Rand*((1<<24)+(1<<8)+0x93) + 0xbadbeef)

	nfree = nresize = 0;
	sigsetjmp(Jmpbuf, 1);
	if(Nlongjmp >= 1000) /* enough signal/longjmp */
		return 0;

	for(; Step < nalloc; ++Step)
	{	k = Step;
	
		/* update all that needs updating */
		for(up = list[k].free; up; up = next)
		{	next = up->next;

			if(!up->data || up->size <= 0)
				continue;

			if((rand = RANDOM()%100) < 75)
			{	
				Allocing = 1;
				free(up->data);
				Allocing = 0;

				nfree += 1;
				up->data = NIL(void*);
				up->size = 0;
			}
			else
			{	sz = RANDOM()%(2*up->size) + 1;

				Allocing = 1;
				up->data = realloc(up->data, sz);
				Allocing = 0;

				if(!up->data)
					terror("Failed to realloc(org=%d sz=%d)", up->size, sz);

				nresize += 1;
				up->size = sz;
			}
		}

		/* get a random size in given range */
		sz = (RANDOM() % (Maxsize-Minsize+1)) + Minsize;

		Allocing = 1;
		list[k].data = malloc(sz);
		Allocing = 0;

		if(!list[k].data)
		{
#if VMALLOC
			{	Vmstat_t	vmst;
				vmstat(Vmregion, &vmst);
				tinfo(vmst.mesg);
			}
#endif
			terror("Failed to malloc(%d) at step=%d", sz, Step);
		}
			
		list[k].size = sz;

		/* get a random point in the future to update */
		if((p = k+1 + RANDOM()%Life) < nalloc )
		{	list[k].next = list[p].free;
			list[p].free = &list[k];
		}
	}

	tinfo("nalloc=%d nfree=%d nresize=%d", nalloc, nfree, nresize);

	return 0;
}

tmain()
{
	int		i, rv;
	Piece_t		*list;
	size_t		sz;
	void		*status;
	pid_t		ppid, cpid;

	tinfo("Configuration: Nalloc=%d Life=%d Minsize=%d Maxsize=%d", Nalloc, Life, Minsize, Maxsize);

	ppid = getpid(); /* pid of parent process */
	cpid = 0;
	signal(SIGHUP, sighup);
	signal(SIGALRM,sigalarm);

#if 1
	switch((cpid = fork()) )
	{	case 0 : /* child process */
			for(;;) /* keep sending sighup to parent */
			{	asorelax(2012);
				kill(ppid, SIGHUP);
			}
			break;

		case -1:
			terror("Can't fork() subprocess");
			break;

		default: /* parent */
			break;
	}
#endif

	tresource(-1, 0);
	sz = Nalloc*sizeof(Piece_t);
	if(!(list = (Piece_t*)malloc(sz)) )
		terror("Failed allocating list of objects nalloc=%d", Nalloc);
	memset(list, 0, sz);

	alarm(2);
	simulate(list, Nalloc);
	alarm(0);

	if(cpid > 0)
		kill(cpid, SIGKILL);

	tinfo("ExecutedStep=%d vs. MaxStep=%d; Nlongjmp=%d", Step, Nalloc, Nlongjmp);
	tresource(0, 0);

#if VMALLOC
	{	Vmstat_t	vmst;
		vmstat(Vmregion, &vmst);
		tinfo(vmst.mesg);
	}
#endif

	texit(0);
}
