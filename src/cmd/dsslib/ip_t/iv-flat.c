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

/* This method allows a new interval to simply overwrite
** any portion of older intervals that it overlaps with.
** Adjacent intervals with the same data are merged.
**
** Written by Kiem-Phong Vo and Glenn Fowler.
*/

typedef struct Flseg_s	Flseg_t;
typedef struct Flat_s	Flat_t;

struct Flat_s
{
	Dtdisc_t	dc;	/* discipline structure for dictionary	*/
	Ivfree_f	freef;	/* user data free */
	Dt_t*		dt;
	Iv_t*		iv;
	int		search;	/* during search, use a faster compare	*/
};

struct Flseg_s
{
	Ivseg_t		seg;	/* the actual segment	*/
	Dtlink_t	link;
};

/* create a new segment identical to "obj" */
static void*
flatmake(Dt_t* dt, void* obj, Dtdisc_t* disc)
{
	Flseg_t*	sg;
	int		size = ((Flat_t*)disc)->iv->size;

	/* the use of sizeof(Flseg_t) is deliberate */
	if (!(sg = newof(0, Flseg_t, 1, 2 * size)))
		return 0;
	fvcpy(size, sg->seg.lo = (unsigned char*)(sg + 1), ((Ivseg_t*)obj)->lo);
	fvcpy(size, sg->seg.hi = sg->seg.lo + size, ((Ivseg_t*)obj)->hi);
	sg->seg.data = ((Ivseg_t*)obj)->data;
	return sg;
}

/* free a segment */
static void
flatfree(Dt_t* dt, void* obj, Dtdisc_t* disc)
{
	if (((Flat_t*)disc)->freef && ((Ivseg_t*)obj)->data)
		((Flat_t*)disc)->freef(((Flat_t*)disc)->iv, ((Ivseg_t*)obj)->data);
	free(obj);
}

/* during build, segments are sorted by "lo" values only */
static int
flatbldcmp(Dt_t* dt, void* o1, void* o2, Dtdisc_t* disc)
{
	int	size = ((Flat_t*)disc)->iv->size;

	return fvcmp(size, ((Ivseg_t*)o1)->lo, ((Ivseg_t*)o2)->lo);
}

/* during search, we are looking for a segment containing some point.
** since a point is a segment with identical ends, the being compared
** objects must be either distinct or have a containing relationship.
*/
static int
flatsrchcmp(Dt_t* dt, void* o1, void* o2, Dtdisc_t* disc)
{
	int	size = ((Flat_t*)disc)->iv->size;

	if (fvcmp(size, ((Ivseg_t*)o1)->hi, ((Ivseg_t*)o2)->lo) < 0)
		return -1;
	else if (fvcmp(size, ((Ivseg_t*)o1)->lo, ((Ivseg_t*)o2)->hi) > 0)
		return 1;
	else
		return 0;
}

static int
flatset(Iv_t* iv, unsigned char* lo, unsigned char* hi, void* data)
{
	unsigned char*	pt;
	Ivseg_t		seg;
	Ivseg_t*	sg;
	Flat_t*		fl;
	Dt_t*		dt;
	unsigned char*	unmatched = iv->disc->unmatched;
	int		size = iv->size;

	if (!iv || !(fl = (Flat_t*)iv->data) || !(dt = fl->dt))
		return -1;
	if (fl->search)
	{
		fl->dc.comparf = flatbldcmp;
		dtdisc(dt, &fl->dc, DT_SAMECMP/* not truthful but ok */);
		fl->search = 0;
	}
	seg.lo = lo;
	seg.hi = hi;
	seg.data = data;

	/* the possibilities for this case are:
	** ........       ........      ........  ...  ...
	**   ++++           ++++++          ++++++++++++++++
	*/
	if ((sg = dtprev(dt, &seg)) && fvcmp(size, fvcpy(size, pt = iv->r2, sg->hi), lo) >= 0)
	{
		if (fvcmp(size, pt, hi) >= 0)
		{
			if (data == sg->data) /* no need for new segment	*/
				return 0;
			/* set left side of old segment	*/
			fvsub(size, sg->hi, lo, iv->unit);
			if (data != unmatched && !dtinsert(dt, &seg))
				return -1;
			if (fvcmp(size, pt, hi) > 0)
			{
				/* set right side of old segment	*/
				fvadd(size, seg.lo = iv->r1, hi, iv->unit);
				seg.hi = pt;
				seg.data = sg->data;
				if (!dtinsert(dt, &seg))
					return -1;
			}
			return 0;
		}
		else
			/* set left side of old segment	*/
			fvsub(size, sg->hi, lo, iv->unit);
	}

	/* the possibilities for this case are:
	** ........      ........      ........  ...   ....
	** ++++          ++++++++      +++++++++++++++++++++++
	*/
	if ((sg = dtsearch(dt, &seg)))
	{
		if (fvcmp(size, sg->hi, hi) > 0)
		{
			if (data == sg->data)
				return 0;
			/* right side of old seg */
			fvadd(size, sg->lo, hi, iv->unit);
		}
		else
			dtdelete(dt, sg);
	}

	/* the possibilities for this case are:
	**    ........      ........      ........   ...  ...
	**  +++++++       ++++++++++    ++++++++++++++++++++++++
	*/
	while((sg = dtnext(dt, &seg)) && fvcmp(size, sg->lo, hi) <= 0)
		if (fvcmp(size, sg->hi, hi) > 0)
		{
			fvadd(size, sg->lo, hi, iv->unit);
			break;
		}
		else
			dtdelete(dt, sg);

	/* mergeable with previous segment */
	if ((sg = dtprev(dt,&seg)) && data == sg->data && fvcmp(size, (fvadd(size, iv->r1, sg->hi, iv->unit), iv->r1), lo) == 0)
	{
		seg.lo = fvcpy(size, iv->r2, sg->lo);
		dtdelete(dt, sg);
	}
	/* mergeable with next segment */
	if ((sg = dtnext(dt, &seg)) && data == sg->data && fvcmp(size, (fvadd(size, iv->r1, hi, iv->unit), iv->r1), sg->lo) == 0)
	{
		seg.hi = fvcpy(size, iv->r1, sg->hi);
		dtdelete(dt, sg);
	}
	/* a new segment */
	return data != unmatched && !dtinsert(dt, &seg) ? -1 : 0;
}

static int
flatdel(Iv_t* iv, unsigned char* lo, unsigned char* hi)
{
	return flatset(iv, lo, hi, iv->disc->unmatched);
}

static unsigned char*
flatget(Iv_t* iv, unsigned char* pt)
{
	Ivseg_t		seg;
	Ivseg_t*	sg;
	Flat_t*		fl;
	Dt_t*		dt;

	if (!(fl = (Flat_t*)iv->data) || !(dt = fl->dt))
		return 0;
	if (!fl->search)
	{
		fl->dc.comparf = flatsrchcmp;
		dtdisc(dt, &fl->dc, DT_SAMECMP/* not truthful but ok */);
		fl->search = 1;
	}
	/* see if inside some segment */
	seg.lo = seg.hi = pt;
	sg = (Ivseg_t*)dtsearch(dt, &seg);
	return sg ? sg->data : iv->disc->unmatched;
}

static Ivseg_t*
flatseg(Iv_t* iv, unsigned char* pt)
{
	Ivseg_t		seg;
	Flat_t*		fl;
	Dt_t*		dt;

	if (!(fl = (Flat_t*)iv->data) || !(dt = fl->dt))
		return 0;
	if (!fl->search)
	{
		fl->dc.comparf = flatsrchcmp;
		dtdisc(dt, &fl->dc, DT_SAMECMP/* not truthful but ok */);
		fl->search = 1;
	}
	/* find the segment containing pt or just beyond it */
	seg.lo = seg.hi = pt;
	return (Ivseg_t*)dtsearch(dt, &seg);
}

static int
flatevent(Iv_t* iv, int type, void* data)
{
	Flat_t*	fl;

	switch (type)
	{
	case IV_OPEN:
		if (!(fl = (Flat_t*)malloc(sizeof(Flat_t))))
			return -1;
		fl->search = 0;
		DTDISC(&fl->dc,0,0,offsetof(Flseg_t,link),flatmake,flatfree,flatbldcmp,0,0,0);
		if (!(fl->dt = dtopen(&fl->dc, Dtoset)))
		{
			free(fl);
			return -1;
		}
		fl->freef = iv->disc->freef;
		fl->iv = iv;
		iv->data = (void*)fl;
		break;
	case IV_CLOSE:
		if (!(fl = (Flat_t*)iv->data))
			return -1;
		if (fl->dt)
			dtclose(fl->dt);
		free(fl);
		iv->data = 0;
		break;
	}
	return 0;
}

static Ivmeth_t _Ivflat =
{
	"flat",
	"The flat interval method allows a new interval to simply overwrite any portion of older intervals that it overlaps with. Adjacent intervals with the same data are merged.",
	0,
	flatget,
	flatset,
	flatdel,
	flatseg,
	flatevent,
};

IVLIB(flat)
