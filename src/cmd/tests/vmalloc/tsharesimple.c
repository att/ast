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

/* Test safe concurrent accesses of memory obtained by
** either mmap or shmget.
**
** Written by Kiem-Phong Vo
*/

#define M_SIZE	(1024)		/* max size of an allocated blk	*/
#define	N_ALLOC	(4*1024)	/* #allocations in each process	*/
#define N_FREE	(2*1024)	/* #frees <= #allocations	*/

typedef struct _piece_s
{
	Void_t*	addr;	/* allocated address	*/
	size_t	size;	/* size to be allocated	*/
	int	free;	/* 1: to be freed	*/
} Piece_t;
Piece_t	Piece[N_ALLOC];

static int working(char* store, char* typeascii, char* pnumascii, ssize_t size)
{
	int		type, pnum;
	ssize_t		k, f;
	Vmdisc_t	*dc;
	Vmalloc_t	*vm;
	Vmstat_t	vmst;
	ssize_t		n_alloc = 0, n_free = 0, n_busy = 0;
	pid_t		pid = getpid();

	type = atoi(typeascii);
	pnum = atoi(pnumascii);

	tinfo("Process %d[pid=%d]: simulation starting with store %s", pnum, pid, store);
	if(!(dc = vmdcshare(store, type, size, 0)) ||
	   !(vm = vmopen(dc, Vmbest, 0)) )
		terror("Process %d[pid=%d]: can't open allocation region", pnum, pid);

	for(k = f = 0; k < N_ALLOC; ++k)
	{	if(!(Piece[k].addr = vmalloc(vm, Piece[k].size)) )
			terror("Process %d[pid=%d]: vmalloc failed", pnum, pid);
		else	n_alloc += 1;

		if(k < (N_ALLOC-1) && (random()%100) != 0 )
			continue;

		for(; f <= k; ++f ) /* free anything that should be freed */
		{	if(!Piece[f].free)
				continue;

			if(Piece[f].addr && vmfree(vm, Piece[f].addr) < 0)
				terror("Process %d[pid=%d]: vmfree failed", pnum, pid);
			else
			{	n_free += 1;
				Piece[f].free = 0;
				Piece[f].addr = (Void_t*)0;
			}
		}
	}

	n_busy = 0;
	for(k = 0; k < N_ALLOC; ++k)
		if(Piece[k].addr)
			n_busy += 1;

	tinfo("Process %d[pid=%d]: (n_alloc=%d, n_free=%d, n_busy=%d) simulation completed",
		pnum, pid, n_alloc, n_free, n_busy);

	vmclose(vm);

	return 0;
}

static pid_t makeprocess(char* program, char* store, int type, int pnum)
{
	pid_t	pid;
	char	pnumascii[64], typeascii[64],  *argv[6];

	sprintf(pnumascii, "%d", pnum);
	sprintf(typeascii, "%d", type);
	argv[0] = program;
	argv[1] = "--child";
	argv[2] = store;
	argv[3] = typeascii;
	argv[4] = pnumascii;
	argv[5] = (char*)0;

	if((pid = fork()) < 0 )
		terror("Could not fork() a subprocess");
	else if(pid > 0 ) /* return to parent process */
		return pid;
	else if(execv(program, argv) < 0 )
		terror("Process %d[pid=%d]: Could not execv() subprocess", pnum, pid);
	else	return 0; /* strictly speaking, never reached */
}

tmain()
{
	int		type;
	ssize_t		k, f, size;
	pid_t		pid;
	Vmstat_t	vmst;
	Vmdisc_t	*dc;
	Vmalloc_t	*vm;
	pid_t		proc[N_PROC];
	char		*store;

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
			else if(random()%(N_ALLOC/N_FREE) == 0)
			{	Piece[k].free = 1;
				if((f += 1) >= N_FREE)
					break;
			}
		}
	}
	size = size*N_PROC; /* this should be big enough for region */

	if(k = tchild())
		return working(argv[k], argv[k+1], argv[k+2], size);

	for(type = -1; type < 2; type += 2 ) /* -1 is mmap, 1 is shm */
	{	vm = NIL(Vmalloc_t*);

		tresource(-1, 0);

		if(type < 0)
		{	tinfo("Testing mmap shared memory");
			store = tstfile("map", -1);
			unlink(store);
			if((dc = vmdcshare(store, -1, size, -1)) )
				vm = vmopen(dc, Vmbest, 0);
		}
		else
		{	tinfo("Testing shm shared memory");
			store = tstfile("shm", -1);
			unlink(store);
			if((dc = vmdcshare(store, 1, size, -1)) )
				vm = vmopen(dc, Vmbest, 0);
		}

		if(!vm)
			terror("Can't open shared region");

		for(k = 0; k < N_PROC; ++k)
			proc[k] = makeprocess(argv[0], store, type, k);

		for(f = 0; f < N_PROC; ++f )
		{	if((pid = wait(0)) <= 0 )
				terror("wait failed");
			for(k = 0; k < N_PROC; ++k)
				if(proc[k] == pid )
					break;
			if(k >= N_PROC)
				terror("Unknown child process %d", pid);
		}

		if(vmstat(vm, &vmst) < 0 )
			terror("Can't get statistics for region");
		tinfo("Statistics: %s", vmst.mesg);

		tresource(1, 0);

		vmclose(vm);

		if(type < 0)
			tinfo("Test for map shared memory passed");
		else	tinfo("Test for shm shared memory passed");
	}

	texit(0);
}
