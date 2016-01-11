/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "ptlib.h"

#define S	"----------"

/*
 * list table a stats on sp
 */

int
ptstats(Pt_t* a, Sfio_t* sp)
{
	int		i;
	int		n;
	int		pc;
	int		tc;
	Ptcount_t	total_addresses;
	Ptcount_t	total_prefixes;
	Ptcount_t	addresses[PTBITS+1];
	Ptcount_t	prefixes[PTBITS+1];

	static char	prt[] = "==================================================";
	static char	tot[] = "**************************************************";

	total_addresses = total_prefixes = 0;
	for (i = 0; i < elementsof(addresses); i++)
		addresses[i] = prefixes[i] = 0;
	PTSCAN(a, x, b, m,

		if (!m)
		{
			m = ~0;
			addresses[b]++;
			total_addresses++;
		}
		addresses[b] += m;
		total_addresses += m;
		prefixes[b]++;

		);
	for (i = n = 0; i < elementsof(prefixes); i++)
		if (prefixes[i])
		{
			n++;
			total_prefixes += prefixes[i];
			tc = (int)((double)(sizeof(tot) - (sizeof(S) - 10)) * ((double)((intmax_t)addresses[i]) / (double)((Ptaddr_t)~0)));
			pc = total_addresses ? ((int)((double)(sizeof(prt) - (sizeof(S) - 10)) * ((double)((intmax_t)addresses[i]) / (double)((intmax_t)total_addresses))) - tc) : 0;
			sfprintf(sp, "/%-2d %6I*u %*I*u  %s%s\n", i, sizeof(prefixes[i]), prefixes[i], sizeof(S) - 1, sizeof(addresses[i]), addresses[i], prt + sizeof(prt) - pc - 1, tot + sizeof(tot) - tc - 1);
		}
	sfprintf(sp, "--- ------ %s\n", S);
	sfprintf(sp, " %2d %6I*u %*I*u  ranges %I*u  entries %I*u\n", n, sizeof(total_prefixes), total_prefixes, sizeof(S) - 1, sizeof(total_addresses), total_addresses, sizeof(Ptcount_t), (Ptcount_t)dtsize(a->dict), sizeof(a->entries), a->entries, sizeof(Ptcount_t));
	return sfsync(sp);
}
