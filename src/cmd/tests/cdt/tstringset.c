/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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
#include	"dttest.h"

#define strcmp(a,b)	teststrcmp(a,b)

int	N_cmp = 0; /* count number of compares */
int strcmp(const char* s1, const char* s2)
{	int	d;
	N_cmp++;
	for(;; s1++, s2++)
	{	if((d = s1[0] - s2[0]) )
			return d;
		if(s1[0] == 0)
			return 0;
	}
}

/* print statistics */
#define N_OBJ	100000
static int	N_obj = N_OBJ, Count = 0, Brief = 0;
static int	Perm[N_OBJ];
static char	Key[N_OBJ][8]; /* max 7-digit numbers */
static char	*Pat = "%07d";

tmain()
{
	int		i, k, t;
	Dt_t*		dt;
	Dtdisc_t	disc;
	char		*ks, tmp[16]; /* max 7-digit numbers */

	/* create a permutation of size N_obj */
	for(i = 0; i < N_obj; ++i)
		Perm[i] = i;
#if RANDOM
	for(i = N_obj; i > 1; --i)
	{	k = ((unsigned int)rand())%i;
		t = Perm[k]; Perm[k] = Perm[i-1]; Perm[i-1] = t;
	}
#endif

	/* make key */
	for(i = 0; i < N_obj; ++i)
		sprintf(Key[i], Pat, Perm[i]);

	disc.key = disc.size = 0; disc.link = -1;
	disc.makef = 0; disc.freef = 0;
	disc.comparf = 0; disc.hashf = 0;
	disc.memoryf = 0; disc.eventf = 0;
	dt = dtopen(&disc, Dtset);

	/* insert into table */
	for(i = 0; i < N_obj; ++i)
	{	dtinsert(dt, Key[i]);
		if(((i+1)%1000) == 0 )
		{	if(dtsize(dt) != i+1)
				terror("Bad size=%d, should be %d", k, i+1);
			for(k = 0; k < 1000; ++k)
			{	sprintf(tmp, Pat, ((unsigned int)rand())%(i+1));
				if(!(ks = (char*)dtsearch(dt,tmp)) )
					terror("Not finding '%s'", tmp);
			}
		}
	}

	/* search in order of insertion */
	for(Count = 0, i = 0; i < N_obj; ++i)
	{	if(!(ks = (char*)dtsearch(dt,Key[i])) )
			terror("Not finding '%s'", Key[i]);
		if(strcmp(ks,Key[i]) == 0)
			Count += 1;
	}
	if(Count != N_obj)
		terror("Count=%d but should be %d", Count, N_obj);

	/* search in a random order */
	for(Count = 0, i = 0; i < N_obj; ++i)
	{	sprintf(tmp, Pat, ((unsigned int)rand())%N_obj);
		if(!(ks = (char*)dtsearch(dt,tmp)) )
			terror("Not finding '%s'", Key[i]);
		if(strcmp(ks,tmp) == 0)
			Count += 1;
	}
	if(Count != N_obj)
		terror("Count=%d but should be %d", Count, N_obj);

	/* search in increasing order */
	for(Count = 0, i = 0; i < N_obj; ++i)
	{	sprintf(tmp, Pat, i);
		if(!(ks = (char*)dtsearch(dt,tmp)) )
			terror("Not finding '%s'", tmp);
		if(strcmp(ks,tmp) == 0)
			Count += 1;
	}
	if(Count != N_obj)
		terror("Count=%d but should be %d", Count, N_obj);

	texit(0);
}
