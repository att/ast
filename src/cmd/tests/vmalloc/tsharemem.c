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

/* Test safe concurrent accesses of memory obtained by
** either mmap or shmget.
**
** Written by Kiem-Phong Vo
*/

#ifndef N_PROC
#define N_PROC		64
#endif

#define M_SIZE	32		/* max size of an allocated blk	*/
#define	N_ALLOC	(16*1024)	/* #allocations in each process	*/
#define N_FREE	(8*1024)	/* #frees <= #allocations	*/

typedef struct _piece_s
{
	Void_t*	addr;	/* allocated address	*/
	size_t	size;	/* size to be allocated	*/
	int	free;	/* 1: to be freed	*/
} Piece_t;
Piece_t	Piece[N_ALLOC];

static char	*Mapstore;
static char	*Shmstore;

static int working(char* store, char* type, char* num, ssize_t size)
{
	ssize_t		k, f;
	Vmalloc_t	*vm;
	Vmstat_t	vmst;
	pid_t		pid = getpid();

	tinfo("Process %s[pid=%d]: about to open region for %s", num, pid, store);
	if(!(vm = vmmopen(store, strcmp(type, "map") == 0 ? -1 : 1, size)) )
		terror("Process %s[pid=%d]: can't open %s allocation region on %s", num, pid, type, store);
	tinfo("Process %s[pid=%d]: %s region successfully opened for %s", num, pid, type, store);

	for(k = f = 0; k < N_ALLOC; ++k)
	{	if(!(Piece[k].addr = vmalloc(vm, Piece[k].size)) )
			terror("Process %s[pid=%d]: vmalloc failed", num, pid);

		if(k < (N_ALLOC-1) && (random()%100) != 0 )
			continue;

		for(; f <= k; ++f ) /* free anything that should be freed */
		{	if(!Piece[f].free)
				continue;
			if(vmfree(vm, Piece[f].addr) < 0)
				terror("Process %s[pid=%d]: vmfree failed", num, pid);
			Piece[f].free = 0;
			Piece[f].addr = (Void_t*)0;
		}
	}

	if(vmstat(vm, &vmst) < 0)
		terror("Process %s[pid=%d]: vmstat failed", num, pid);
	tinfo("Process %s[pid=%d]: extent=%d busy=(n=%d,z=%d) free=(n=%d,z=%d)",
		num, pid, vmst.extent, vmst.n_busy, vmst.s_busy, vmst.n_free, vmst.s_free);

	vmclose(vm);

	return 0;
}

static pid_t makeprocess(char* program, char* store, char* type, int p)
{
	pid_t	pid;
	char	num[64], *argv[6];

	sprintf(num, "%d", p);
	argv[0] = program;
	argv[1] = "--child";
	argv[2] = store;
	argv[3] = type;
	argv[4] = num;
	argv[5] = 0;

	if((pid = fork()) < 0 )
		terror("Could not fork() a subprocess");
	else if(pid > 0 ) /* return to parent process */
		return pid;
	else if(execv(program, argv) < 0 )
		terror("Process %d[pid=%d]: Could not execv() subprocess", p, pid);
	else	return 0; /* strictly speaking, never reached */
}

tmain()
{
	int		test, flag, code = 2;
	ssize_t		k, f, size;
	pid_t		pid;
	char		*store, *type;
	Vmstat_t	vmst;
	Vmalloc_t	*vm;
	pid_t		proc[N_PROC];

	taso(ASO_PROCESS);

	size = 0; /* make up list of pieces for allocation */
	srandom(0); /* make it easier to debug */
	for(k = 0; k < N_ALLOC; ++k)
	{	Piece[k].size = (random()%M_SIZE) + 1;
		size += Piece[k].size + 16; /* add slop for malloc header */
	}
	for(f = 0; f < N_FREE; ) /* set up what should be freed */
	{	for(k = 0; k < N_ALLOC; ++k)
		{	if(Piece[k].free == 1)
				continue;
			else if(random()%16 == 0)
			{	Piece[k].free = 1;
				if((f += 1) >= N_FREE)
					break;
			}
		}
	}
	size = 2*size*N_PROC; /* this should be big enough for region */

	if(k = tchild())
		return working(argv[k], argv[k+1], argv[k+2], size);

	Mapstore = tstfile("map", -1);
	Shmstore = tstfile("shm", -1);
	for(test = 0; test < 2; ++test )
	{	if (test)
		{	type = "shm";
			flag = 1;
			store = Shmstore;
		}
		else
		{	type = "map";
			flag = -1;
			store = Mapstore;
		}
		tinfo("Testing %s shared memory on %s", type, store);

		(void)unlink(Mapstore); /* remove any existing backing store */
		(void)unlink(Shmstore); /* remove any existing backing store */

		vm = vmmopen(store, flag, size);
		if(!vm)
			terror("%s: Can't open shared region", store);

		for(k = 0; k < N_PROC; ++k)
		{	tinfo("Creating subprocess %d", k);
			proc[k] = makeprocess(argv[0], store, type, k);
		}
		code = twait(proc, N_PROC);

		k = vmstat(vm, &vmst);
		vmmrelease(vm, 1); /* clean up file, shmid */
		vmclose(vm);

		if (code)
			break;
		if(k < 0 )
			terror("%s: Can't get statistics for region", store);
		tinfo("Statistics: store=%s extent=%d busy=(n=%d,z=%d) free=(n=%d,z=%d)",
			store, vmst.extent, vmst.n_busy, vmst.s_busy, vmst.n_free, vmst.s_free);
		if(vmst.n_busy != (N_ALLOC-N_FREE)*N_PROC)
			terror("%s: Number of busy blocks %d is not the expected %d",
				store, vmst.n_busy, (N_ALLOC-N_FREE)*N_PROC );

		tinfo("Test for %s share memory passed", type);
	}

	texit(code);
}
