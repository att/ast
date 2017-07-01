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

#define N_REGION	64
#define N_SHMREG	3

typedef struct _region_s
{	Vmalloc_t	*vm;
	ssize_t		size;
} Region_t;

tmain()
{
	int		i, k, m;
	ssize_t		size;
	Vmdisc_t	*dc;
	Region_t	region[N_REGION];
	char		*shmfile, *mapfile, *warn;

	warn = (char*)0;
	size = 0;
	m = sizeof(char*) == 4 ? 2 : 16;
	for(k = 0; k < N_REGION; ++k)
	{
		region[k].size = (trandom()%m + m)*m;
		region[k].size *= 1024*1024;

		if(k%(N_SHMREG+1) != 0 ) /* do a bunch of shm memory */
		{
			shmfile = tstfile("shm", k);
			if(!(dc = vmdcshare(shmfile, 1, region[k].size, -1)) ||
			   !(region[k].vm = vmopen(dc, Vmbest, 0)) )
			{	warn = "shmem";
				break;
			}

			tinfo("Region-shmem[%d] size=%lu addr=%p",
				k, region[k].size/(1024*1024), region[k].vm->data);
			size += region[k].size;
		}
		else /* interspersed the above with mmap */
		{	
			mapfile = tstfile("map", k);
			if(!(dc = vmdcshare(mapfile, -1, region[k].size, -1)) ||
			   !(region[k].vm = vmopen(dc, Vmbest, 0)) )
			{	warn = "mmap";
				break;
			}

			tinfo("Region-mmap[%d] size=%lu addr=%p",
				k, region[k].size/(1024*1024), region[k].vm->data);
			size += region[k].size;
		}

		for(i = 0; i < k; ++i)
		{	if((char*)region[i].vm->data >= (char*)region[k].vm->data &&
			   (char*)region[i].vm->data <= ((char*)region[k].vm->data + region[k].size) )
				terror("Region[%d] and Region[%d] overlap", i, k);
		}
	}

	if(size > 0 )
		tinfo("#regions to try=%d #regions actually opened=%d total memory=%luM",
			N_REGION, k, size/(1024*1024) );

	for(i = 0; i < k; ++i)
		vmclose(region[i].vm);

	if(warn && k == 0)
		terror("Region-%s[%d] size=%luM failed", warn, k, region[k].size/(1024*1024) );

	texit(0);
}
