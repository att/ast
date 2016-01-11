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

#include "ptvlib.h"

#define S	"----------"

/*
 * list table a stats on sp
 */

int
ptvstats(Ptv_t* a, Sfio_t* sp)
{
	int		i;
	int		n;
	int		pc;
	int		tc;
	int		bits;
	unsigned char*	v;
	unsigned char*	total_addresses;
	unsigned char*	total_prefixes;
	unsigned char**	addresses;
	unsigned char**	prefixes;

	static char	prt[] = "==================================================";
	static char	tot[] = "**************************************************";

	bits = a->size * 8;
	if (!(addresses = (unsigned char**)newof(0, unsigned char, (2 * (bits + 1)) * (sizeof(unsigned char*) + a->size), 0)))
	{
		if (a->disc->errorf)
			(*a->disc->errorf)(NiL, a->disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	prefixes = addresses + bits + 1;
	total_addresses = (unsigned char*)(prefixes + bits + 1);
	total_prefixes = total_addresses + a->size;
	v = total_prefixes + a->size;
	for (i = 0; i <= bits; i++)
	{
		addresses[i] = v;
		v += a->size;
		prefixes[i] = v;
		v += a->size;
	}
	PTVSCAN(a, b,

		if (fvcmp(a->size, a->r[3], a->r[1]) < 0)
		{
			fvnot(a->size, a->r[3], a->r[3]);
			fvadd(a->size, addresses[b], addresses[b], a->r[1]);
			fvadd(a->size, total_addresses, total_addresses, a->r[1]);
		}
		fvadd(a->size, addresses[b], addresses[b], a->r[3]);
		fvadd(a->size, total_addresses, total_addresses, a->r[3]);
		fvadd(a->size, prefixes[b], prefixes[b], a->r[1]);

		);
	fvset(a->size, a->r[0], 0);
	fvnot(a->size, a->r[0], a->r[0]);
	fvset(a->size, a->r[2], 256);
	for (i = n = 0; i <= bits; i++)
		if (fvcmp(a->size, prefixes[i], a->r[1]) >= 0)
		{
			n++;
			fvadd(a->size, total_prefixes, total_prefixes, prefixes[i]);
			if (fvcmp(a->size, addresses[i], a->r[1]) >= 0)
			{
				fvdiv(a->size, a->r[3], a->r[5], a->r[0], addresses[i]);
				if (fvcmp(a->size, a->r[3], a->r[2]) < 0)
					tc = (int)((double)(sizeof(tot) - (sizeof(S) - 10)) / (double)a->r[3][a->size - 1]);
				else
					tc = 0;
			}
			else
				tc = 0;
			if (fvcmp(a->size, addresses[i], a->r[1]) >= 0)
			{
				fvdiv(a->size, a->r[3], a->r[5], total_addresses, addresses[i]);
				if (fvcmp(a->size, a->r[3], a->r[2]) < 0)
					pc = (int)((double)(sizeof(prt) - (sizeof(S) - 10)) / (double)a->r[3][a->size - 1]);
				else
					pc = 0;
			}
			else
				pc = 0;
			sfprintf(sp, "/%-3d %16s %16s  %s%s\n", i, fmtfv(a->size, prefixes[i], 10, ',', 3), sizeof(S) - 1, fmtfv(a->size, addresses[i], 10, ',', 3), prt + sizeof(prt) - pc - 1, tot + sizeof(tot) - tc - 1);
		}
	sfprintf(sp, "---- ------ %s\n", S);
	sfprintf(sp, " %3d %s %s  ranges %I*u  entries %I*u\n", n, fmtfv(a->size, total_prefixes, 10, ',', 3), sizeof(S) - 1, fmtfv(a->size, total_addresses, 10, ',', 3), sizeof(Ptvcount_t), (Ptvcount_t)dtsize(a->dict), sizeof(a->entries), a->entries);
	return sfsync(sp);
}
