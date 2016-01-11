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

/* Test memory obtained by vmmopen.
**
** Written by Kiem-Phong Vo
*/

#define M_SIZE	(4*1024) /* max size of a blk	*/
#define	N_ALLOC	(512)	/* #allocations 	*/
#define N_FREE	(256)	/* #frees <= #allocs	*/

typedef struct _piece_s
{
	Void_t*	addr;	/* allocated address	*/
	size_t	size;	/* size to be allocated	*/
	int	free;	/* 1: to be freed	*/
} Piece_t;
Piece_t	Piece[N_ALLOC];

static int working(char* store, int type, ssize_t size)
{
	ssize_t		k, f, nbusy, nfree;
	Vmdisc_t	*dc;
	Vmalloc_t	*vm;
	Vmstat_t	vmst;

	tinfo("About to reopen region based on file %s", store);

	if(!(dc = vmdcshare(store, type, size, 1)) ||
	   !(vm = vmopen(dc, Vmbest, 0)) )
		terror("Can't open region based on %s", store);
	tinfo("Region successfully opened", store);
	if(vmstat(vm, &vmst) < 0)
		terror("vmstat failed");

	nbusy = nfree = 0;
	for(k = f = 0; k < N_ALLOC; ++k)
	{	if(!(Piece[k].addr = vmalloc(vm, Piece[k].size)) )
			terror("Vmalloc[k=%d,size=%d] failed ", k, Piece[k].size);
		nbusy += 1;
		if(k < (N_ALLOC-1) && (random()%100) != 0 )
			continue;

		for(; f <= k; ++f ) /* free anything that should be freed */
		{	if(!Piece[f].free)
				continue;
			if(vmfree(vm, Piece[f].addr) < 0)
				terror("Vmfree [f=%d] failed", f);
			nfree += 1;
			nbusy -= 1;
			Piece[f].free = 0;
			Piece[f].addr = (Void_t*)0;
		}
	}

	if(vmstat(vm, &vmst) < 0)
		terror("vmstat failed");
	tinfo("extent=%d busy=(expect=%d,actual=%d,z=%d) free=(n=%d,z=%d)",
		vmst.extent, nbusy, vmst.n_busy, vmst.s_busy, vmst.n_free, vmst.s_free);
	if(nbusy != vmst.n_busy)
		terror("Number of busy pieces is wrong nbusy=%d != vmst.n_busy=%d", nbusy, vmst.n_busy);

	vmclose(vm);

	return 0;
}

tmain()
{
	ssize_t		k, f, size;
	Vmstat_t	vmst;
	Vmdisc_t	*dc;
	Vmalloc_t	*vm;
	char		*store;

	size = 0; /* make up list of pieces for allocation */
	srandom(0); /* make it easier to debug */
	for(k = 0; k < N_ALLOC; ++k)
	{	Piece[k].size = (random()%M_SIZE) + 1;
		size += Piece[k].size + 32; /* add slop for malloc header */
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
	size *= 2; /* should be big enough */

	tinfo("Testing mmap shared memory");

	store = tstfile("map", -1);
	unlink(store); /* start with a fresh file */
	if(!(dc = vmdcshare(store, -1, size, -1)) ||
	   !(vm = vmopen(dc, Vmbest, 0)) )
		terror("Can't open shared region");

	if(working(store, -1, size) < 0 )
		terror("Failed simulation");
		
	if(vmstat(vm, &vmst) < 0 )
		terror("Can't get statistics for region");
	tinfo("Statistics: extent=%d busy=(n=%d,z=%d) free=(n=%d,z=%d)",
		vmst.extent, vmst.n_busy, vmst.s_busy, vmst.n_free, vmst.s_free);
	if(vmst.n_busy != (N_ALLOC-N_FREE))
		terror("Number of busy block %d is not the expected %d",
			vmst.n_busy, (N_ALLOC-N_FREE) );

	vmclose(vm);

	texit(0);
}
