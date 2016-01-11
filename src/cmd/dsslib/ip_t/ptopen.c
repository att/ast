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
 * pt handle operations
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "ptlib.h"

/*
 * order by prefix
 */

static int
byprefix(Dt_t* dt, register Ptprefix_t* a, register Ptprefix_t* b, Dtdisc_t* disc)
{
	NoP(dt);
	NoP(disc);

	if (a->max < b->min)
		return -1;
	if (a->min > b->max)
		return 1;
	return 0;
}

static void*
makef(Dt_t* dt, register Ptprefix_t* a, Dtdisc_t* disc)
{
	register Ptprefix_t*	b;

	if (b = oldof(0, Ptprefix_t, 1, 0))
	{
		b->min = a->min;
		b->max = a->max;
		b->data.pointer = 0;
	}
	return b;
}

static void
freef(Dt_t* dt, Ptprefix_t* a, Dtdisc_t* disc)
{
	free(a);
}

/*
 * open a new table
 */

Pt_t*
ptopen(Ptdisc_t* disc)
{
	Pt_t*		a;

	static Dtdisc_t	prefixdisc;

	prefixdisc.link = offsetof(Ptprefix_t, link);
	prefixdisc.comparf = (Dtcompar_f)byprefix;
	prefixdisc.makef = (Dtmake_f)makef;
	prefixdisc.freef = (Dtfree_f)freef;
	if (!(a = newof(0, Pt_t, 1, 0)) || !(a->dict = dtopen(&prefixdisc, Dtoset)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		if (a)
			free(a);
		return 0;
	}
	a->disc = disc;
	return a;
}

/*
 * close a table
 */

int
ptclose(Pt_t* a)
{
	if (!a)
		return -1;
	dtclose(a->dict);
	free(a);
	return 0;
}

/*
 * insert prefix range min..max into tab
 * prefix pointer returned on success
 */

Ptprefix_t*
ptinsert(Pt_t* tab, Ptaddr_t min, Ptaddr_t max)
{
	register Ptprefix_t*	xp;
	register Ptprefix_t*	pp;
	Ptprefix_t		key;

	tab->entries++;
	key.min = key.max = min ? (min - 1) : min;
	if (!(xp = (Ptprefix_t*)dtsearch(tab->dict, &key)))
		xp = (Ptprefix_t*)dtnext(tab->dict, &key);
	key.min = min;
	key.max = max;
	pp = 0;
	if (xp)
	{
		if (key.min >= xp->min && key.max <= xp->max)
			return xp;
		if (key.max >= (xp->min ? (xp->min - 1) : 0))
		{
			if (key.min > xp->min)
				key.min = xp->min;
			do
			{
				max = xp->max;
				pp = xp;
				xp = (Ptprefix_t*)dtnext(tab->dict, xp);
				dtdelete(tab->dict, pp);
			} while (xp && key.max >= (xp->min - 1));
			if (key.max < max)
				key.max = max;
		}
	}
	return (Ptprefix_t*)dtinsert(tab->dict, &key);
}

/*
 * delete prefix range min..max from tab
 * 0 returned on success
 */

int
ptdelete(Pt_t* tab, Ptaddr_t min, Ptaddr_t max)
{
	register Ptprefix_t*	xp;
	Ptprefix_t		key;
	Ptprefix_t		cur;

	tab->entries++;
	key.min = min;
	key.max = max;
	if (xp = (Ptprefix_t*)dtsearch(tab->dict, &key))
	{
		do
		{
			cur.min = xp->min;
			cur.max = xp->max;
			dtdelete(tab->dict, xp);
			if (key.min > cur.min)
			{
				max = cur.max;
				cur.max = key.min - 1;
				if (!dtinsert(tab->dict, &cur))
					goto bad;
				if (key.max < max)
				{
					cur.min = key.max + 1;
					cur.max = max;
					if (!dtinsert(tab->dict, &cur))
						goto bad;
					break;
				}
			}
			else if (key.max < xp->max)
			{
				xp->min = key.max + 1;
				if (!dtinsert(tab->dict, xp))
					goto bad;
			}
		} while (xp = (Ptprefix_t*)dtnext(tab->dict, xp));
	}
	return 0;
 bad:
	if (tab->disc->errorf)
		(*tab->disc->errorf)(NiL, tab->disc, ERROR_SYSTEM|2, "out of space");
	return -1;
}
