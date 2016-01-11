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

#include "ivlib.h"

/* This method manages nested intervals so that only points in
** the smallest containing intervals will be visible.
**
** Written by Kiem-Phong Vo.
*/

typedef struct Itvl_s	Itvl_t;
typedef struct Nest_s	Nest_t;

/* a segment [lo,hi] and associated "data". */
struct Itvl_s
{
	Dtlink_t	link;	/* splay tree holder	*/
	unsigned char*	lo;	/* low point		*/
	unsigned char*	hi;	/* high point		*/
	void*		data;	/* associated data	*/
};

struct Nest_s
{
	Dtdisc_t	dc;	/* dt discipline	*/
	Ivfree_f	freef;	/* user data free	*/
	Dt_t*		dt;	/* to keep intervals	*/
	Iv_t*		flat;	/* flat interval handle	*/
	Iv_t*		iv;	/* original interval	*/
	Ivdisc_t	disc;	/* flat discipline	*/
	int		changed;/* intervals changed	*/
};

/* create a new segment identical to p */
static void*
nestmake(Dt_t* dt, void* p, Dtdisc_t* disc)
{
	Itvl_t*	op = (Itvl_t*)p;
	Itvl_t*	np;
	int	size = ((Nest_t*)disc)->iv->size;

	if (!(np = newof(0, Itvl_t, 1, 2 * size)))
		return 0;
	fvcpy(size, np->lo = (unsigned char*)(np + 1), op->lo);
	fvcpy(size, np->hi = np->lo + size, op->hi);
	np->data = op->data;
	return np;
}

/* free a segment */
static void
nestfree(Dt_t* dt, void* obj, Dtdisc_t* disc)
{
	if (((Nest_t*)disc)->freef && ((Itvl_t*)obj)->data)
		((Nest_t*)disc)->freef(((Nest_t*)disc)->iv, ((Itvl_t*)obj)->data);
	free(obj);
}

/* Segments are sorted so that "inside" ones are consider larger */
static int
nestcmp(Dt_t* dt, void* o1, void* o2, Dtdisc_t* disc)
{
	Itvl_t*		i1;
	Itvl_t*		i2;
	int		size = ((Nest_t*)disc)->iv->size;
	int		l;
	int		h;

	if ((i1 = (Itvl_t*)o1) == (i2 = (Itvl_t*)o2))
		return  0;
	if (fvcmp(size, i1->hi, i2->lo) < 0)	/* i1 is to the left of i2	*/
		return -1;
	if (fvcmp(size, i1->lo, i2->hi) > 0)	/* i1 is to the right of i2	*/
		return  1;
	h = fvcmp(size, i1->hi, i2->hi);
	l = fvcmp(size, i1->lo, i2->lo);
	if (l < 0)
	{
		if (h >= 0)			/* i1 contains i2		*/
			return -1;
		else
			return  0;		/* i1 crossing i2 on the left	*/
	}
	else if (l == 0)
	{
		if (h < 0)			/* i1 is contained in i2	*/
			return  1;
		else if (h == 0) 		/* equal segments		*/
			return  0;
		else
			return -1;		/* i1 contains i2		*/
	}
	else /* if (l > 0) */
	{
		if (h <= 0)			/* i1 is contained in i2	*/
			return  1;
		else
			return  0;		/* i1 crossing i2 on the right	*/
	}
}

static int nestset(Iv_t* iv, unsigned char* lo, unsigned char* hi, void* data)
{
	Nest_t*		nst;
	Itvl_t		itvl;
	Itvl_t*		it;
	int		size = iv->size;

	if (!iv || !(nst = (Nest_t*)iv->data))
		return -1;
	itvl.lo = lo;
	itvl.hi = hi;
	itvl.data = data;
	if (!(it = (Itvl_t*)dtsearch(nst->dt, &itvl)))
	{
		nst->changed = 1;
		return dtinsert(nst->dt, &itvl) ? 0 : -1;
	}
	else if (fvcmp(size, it->lo, lo) || fvcmp(size, it->hi, hi))
		return -1; /* must have been a crossing element */
	else if (it->data != data) /* just update the data	*/
	{
		nst->changed = 1;
		it->data = data;
		return 0;
	}
	else
		return 0;
}

static int nestdel(Iv_t* iv, unsigned char* lo, unsigned char* hi)
{
	Nest_t*	nst;
	Itvl_t	itvl;
	Itvl_t*	it;
	int	size = iv->size;

	if (!(nst = (Nest_t*)iv->data))
		return -1;
	itvl.lo = lo;
	itvl.hi = hi;
	if (!(it = (Itvl_t*)dtsearch(nst->dt, &itvl)) || fvcmp(size, it->lo, lo) || fvcmp(size, it->hi, hi))
		return -1;
	nst->changed = 1;
	dtdelete(nst->dt, it);
	return 0;
}

/* create a fresh Ivflat handle to compute maximal open segments */
static int nest2flat(Iv_t* iv, Nest_t* nst)
{
	Itvl_t*	it;

	if (nst->flat)
		ivclose(nst->flat);
	if (!(nst->flat = ivopen(&nst->disc, ivmeth("flat"), iv->size, 0)))
		return -1;
	/* insert "in order" all intervals into the Ivflat handle */
	for(it = (Itvl_t*)dtfirst(nst->dt); it; it = (Itvl_t*)dtnext(nst->dt, it))
		ivset(nst->flat, it->lo, it->hi, it->data);
	nst->changed = 0;
	return 0;
}

static unsigned char*
nestget(Iv_t* iv, unsigned char* pt)
{
	Nest_t*	nst;

	if (!(nst = (Nest_t*)iv->data) || nst->changed && nest2flat(iv, nst) < 0)
		return 0;
	return nst->flat ? ivget(nst->flat, pt) : 0;
}

static Ivseg_t*
nestseg(Iv_t* iv, unsigned char* pt)
{
	Nest_t*	nst;

	if (!(nst = (Nest_t*)iv->data) || nst->changed && nest2flat(iv, nst) < 0)
		return 0;
	return nst->flat ? ivseg(nst->flat, pt) : 0;
}

static int
nestevent(Iv_t* iv, int type, void* data)
{
	Nest_t*	nst;

	switch (type)
	{
	case IV_OPEN:
		if (!(nst = newof(0, Nest_t, 1, 0)))
			return -1;
		nst->dc.makef = nestmake;
		nst->dc.freef = nestfree;
		nst->dc.comparf = nestcmp;
		if (!(nst->dt = dtopen(&nst->dc, Dtoset)))
		{
			free(nst);
			return -1;
		}
		nst->changed = 0;
		nst->flat = 0;
		nst->freef = iv->disc->freef;
		nst->iv = iv;
		nst->disc = *iv->disc;
		nst->disc.freef = 0;
		iv->data = (void*)nst;
		break;
	case IV_CLOSE:
		if ((nst = (Nest_t*)iv->data))
		{
			if (nst->flat)
				ivclose(nst->flat);
			if (nst->dt)
				dtclose(nst->dt);
			free(nst);
			iv->data = 0;
		}
		break;
	}
	return 0;
}

static Ivmeth_t _Ivnested =
{
	"nested",
	"The nested method manages nested intervals so that only points in the smallest containing intervals will be visible.",
	0,
	nestget,
	nestset,
	nestdel,
	nestseg,
	nestevent,
};

IVLIB(nested)
