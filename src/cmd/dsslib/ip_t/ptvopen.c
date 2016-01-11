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

/*
 * ptv handle operations
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "ptvlib.h"

typedef struct Prefixdisc_s
{
	Dtdisc_t	dtdisc;
	int		size;
} Prefixdisc_t;

/*
 * order by prefix
 */

static int
byprefix(Dt_t* dt, register Ptvprefix_t* a, register Ptvprefix_t* b, Dtdisc_t* disc)
{
	register Prefixdisc_t*	p = (Prefixdisc_t*)disc;

	NoP(dt);
	NoP(disc);

	if (fvcmp(p->size, a->max, b->min) < 0)
		return -1;
	if (fvcmp(p->size, a->min, b->max) > 0)
		return 1;
	return 0;
}

static void*
makef(Dt_t* dt, register Ptvprefix_t* a, Dtdisc_t* disc)
{
	register Prefixdisc_t*	p = (Prefixdisc_t*)disc;
	register Ptvprefix_t*	b;

	if (b = oldof(0, Ptvprefix_t, 1, 2 * p->size))
	{
		fvcpy(p->size, b->min = (unsigned char*)(b + 1), a->min);
		fvcpy(p->size, b->max = b->min + p->size, a->max);
	}
	return b;
}

static void
freef(Dt_t* dt, Ptvprefix_t* a, Dtdisc_t* disc)
{
	free(a);
}

/*
 * open a new table
 */

Ptv_t*
ptvopen(Ptvdisc_t* disc, int size)
{
	Ptv_t*		a;
	int		i;
	Prefixdisc_t*	p;

	if (!(a = newof(0, Ptv_t, 1, sizeof(Prefixdisc_t) + size * PTV_REGISTERS)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	p = (Prefixdisc_t*)(a + 1);
	p->dtdisc.link = offsetof(Ptvprefix_t, link);
	p->dtdisc.comparf = (Dtcompar_f)byprefix;
	p->dtdisc.makef = (Dtmake_f)makef;
	p->dtdisc.freef = (Dtfree_f)freef;
	p->size = size;
	if (!(a->dict = dtopen(&p->dtdisc, Dtoset)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		free(a);
		return 0;
	}
	a->disc = disc;
	a->size = size;
	a->r[0] = (unsigned char*)(p + 1);
	for (i = 1; i < PTV_REGISTERS; i++)
		a->r[i] = a->r[i-1] + a->size;
	return a;
}

/*
 * close a table
 */

int
ptvclose(Ptv_t* a)
{
	if (!a)
		return -1;
	dtclose(a->dict);
	free(a);
	return 0;
}

/*
 * insert prefix range min..max into tab
 * 0 returned on success
 */

Ptvprefix_t*
ptvinsert(Ptv_t* tab, Ptvaddr_t min, Ptvaddr_t max)
{
	register Ptvprefix_t*	xp;
	register Ptvprefix_t*	pp;
	Ptvprefix_t		key;

	tab->entries++;
	fvset(tab->size, tab->r[2], 1);
	if (fvcmp(tab->size, min, tab->r[2]) >= 0)
	{
		fvsub(tab->size, tab->r[3], min, tab->r[2]);
		key.min = key.max = tab->r[3];
	}
	else
		key.min = key.max = min;
	if (!(xp = (Ptvprefix_t*)dtsearch(tab->dict, &key)))
		xp = (Ptvprefix_t*)dtnext(tab->dict, &key);
	key.min = min;
	key.max = max;
	pp = 0;
	if (xp)
	{
		if (fvcmp(tab->size, key.min, xp->min) >= 0 && fvcmp(tab->size, key.max, xp->max) <= 0)
			return xp;
		if (fvcmp(tab->size, xp->min, tab->r[2]) >= 0)
			fvsub(tab->size, tab->r[3], xp->min, tab->r[2]);
		else
			fvset(tab->size, tab->r[3], 0);
		if (fvcmp(tab->size, key.max, tab->r[3]) >= 0)
		{
			if (fvcmp(tab->size, key.min, xp->min) > 0)
				key.min = xp->min;
			do
			{
				max = xp->max;
				pp = xp;
				xp = (Ptvprefix_t*)dtnext(tab->dict, xp);
				dtdelete(tab->dict, pp);
				if (!xp)
					break;
				fvsub(tab->size, tab->r[3], xp->min, tab->r[1]);
			} while (fvcmp(tab->size, key.max, tab->r[3]) >= 0);
			if (fvcmp(tab->size, key.max, max) < 0)
				key.max = max;
		}
	}
	return (Ptvprefix_t*)dtinsert(tab->dict, &key);
}

/*
 * delete prefix range min..max from tab
 * 0 returned on success
 */

int
ptvdelete(Ptv_t* tab, Ptvaddr_t min, Ptvaddr_t max)
{
	register Ptvprefix_t*	xp;
	Ptvprefix_t		key;
	Ptvprefix_t		cur;

	tab->entries++;
	key.min = min;
	key.max = max;
	if (xp = (Ptvprefix_t*)dtsearch(tab->dict, &key))
	{
		fvset(tab->size, tab->r[1], 1);
		do
		{
			cur.min = xp->min;
			cur.max = xp->max;
			dtdelete(tab->dict, xp);
			if (fvcmp(tab->size, key.min, cur.min) > 0)
			{
				max = cur.max;
				fvsub(tab->size, tab->r[0], key.min, tab->r[1]);
				cur.max = tab->r[0];
				if (!dtinsert(tab->dict, &cur))
					goto bad;
				if (fvcmp(tab->size, key.max, max) < 0)
				{
					fvadd(tab->size, tab->r[2], key.max, tab->r[1]);
					cur.min = tab->r[2];
					cur.max = max;
					if (!dtinsert(tab->dict, &cur))
						goto bad;
					break;
				}
			}
			else if (fvcmp(tab->size, key.max, xp->max) < 0)
			{
				fvadd(tab->size, tab->r[3], key.max, tab->r[1]);
				xp->min = tab->r[3];
				if (!dtinsert(tab->dict, xp))
					goto bad;
			}
		} while (xp = (Ptvprefix_t*)dtnext(tab->dict, xp));
	}
	return 0;
 bad:
	if (tab->disc->errorf)
		(*tab->disc->errorf)(NiL, tab->disc, ERROR_SYSTEM|2, "out of space");
	return -1;
}
