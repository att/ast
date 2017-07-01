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

/* Test segmented memory
** The idea is to have segmen() returns SEGN separate pieces of memory.
** The allocation method will have to handle the fact that these are not contiguous.
*/

#define	SEGN	4
#define SEGZ	(1024*1024)
static char	Seg0[SEGZ+16];
static char	Seg1[SEGZ+16];
static char	Seg2[SEGZ+16];
static char	Seg3[SEGZ+16];
static char	*Segm[SEGN] = {Seg0, Seg1, Seg2, Seg3};
static int	Segn = 0;

static Void_t* segmem(Vmalloc_t* vm, Void_t* ca, size_t cs, size_t ns, Vmdisc_t* dc)
{
	if(ns > SEGZ)
		return NIL(Void_t*);
	else if(ca && cs > 0)
		return ca;
	else if(Segn >= SEGN)
		return NIL(Void_t*);
	else	return (Void_t*)Segm[Segn++];
}

static Vmdisc_t Disc = { segmem, NIL(Vmexcept_f), 1024};

tmain()
{
	Vmalloc_t	*vm;
	Void_t		*data[SEGN][SEGN];
	int		n, k;

	if(!(vm = vmopen(&Disc,Vmbest,0)) )
		terror("Opening region0");
	for(n = 0; n < SEGN; ++n)
	{	if(!(data[n][0] = vmalloc(vm, (SEGZ/SEGN)/2 + 32)) )
			terror("vmalloc failed");
		if(!(data[n][1] = vmalloc(vm, (SEGZ/SEGN)/2 + 32)) )
			terror("vmalloc failed");
		if(!(data[n][2] = vmalloc(vm, (SEGZ/SEGN)/2 + 32)) )
			terror("vmalloc failed");
		if(!(data[n][3] = vmalloc(vm, 128)) )
			terror("vmalloc failed");

		vmfree(vm, data[n][0]);
		vmfree(vm, data[n][2]);
	}
	vmclose(vm);

	/* test the block allocation/free algorithm */
#define ZBLOCK		512
#define NBLOCK		(SEGZ/(ZBLOCK+64))
	{	Void_t		*big, *blk[NBLOCK];
		if(!(vm = vmopen(&Disc,Vmbest,0)) )
			terror("Opening region1");

		for(n = 0; n < NBLOCK; ++n)
			if(!(blk[n] = vmalloc(vm, ZBLOCK)) )
				terror("vmalloc failed");

		for(n = 1; n < NBLOCK; n += 2) /* free every other block to keep fragmented */
			vmfree(vm, blk[n]);
		for(n = 0; n < SEGN; ++n)
		{	if(!(big = vmalloc(vm, SEGZ/4)) )
			{	tinfo("vmalloc failed due to lack of memory as expected");
				break;
			}
		}

		for(n = 0; n < NBLOCK; n += 2) /* free all blocks to remerge free areas */
			vmfree(vm, blk[n]);
		if(!(big = vmalloc(vm, SEGZ/4)) )
			terror("vmalloc failed");

		vmclose(vm);
	}

	texit(0);
}
