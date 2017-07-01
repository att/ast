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

Dtdisc_t Disc =
	{ 0, sizeof(long), -1,
	  newint, NIL(Dtfree_f), compare, hashint,
	  NIL(Dtmemory_f), NIL(Dtevent_f)
	};

static int Count, See[10];

#if __STD_C
static int visit(Dt_t* dt, Void_t* obj, Void_t* data)
#else
static int visit(dt, obj, data)
Dt_t*	dt;
Void_t* obj;
Void_t*	data;
#endif
{
	See[(long)obj] = 1;
	Count += 1;
	return 0;
}

tmain()
{
	Dt_t		*dt1, *dt2, *dt3;
	long		i, k;

	if(!(dt1 = dtopen(&Disc,Dtoset)) )
		terror("Opening Dtoset");
	if(!(dt2 = dtopen(&Disc,Dtoset)) )
		terror("Opening Dtoset");
	if(!(dt3 = dtopen(&Disc,Dtoset)) )
		terror("Opening Dtoset");

	dtinsert(dt1,1L);
	dtinsert(dt1,3L);
	dtinsert(dt1,5L);
	dtinsert(dt1,2L);

	dtinsert(dt2,2L);
	dtinsert(dt2,4L);
	dtinsert(dt2,6L);
	dtinsert(dt2,3L);

	dtinsert(dt3,2L);
	dtinsert(dt3,7L);
	dtinsert(dt3,6L);
	dtinsert(dt3,8L);

	if((long)dtsearch(dt1,4L) != 0)
		terror("Finding 4 here?");

	dtview(dt1,dt2);
	if((long)dtsearch(dt1,4L) != 4)
		terror("Should find 4 here!");

	dtwalk(dt1,visit,NIL(Void_t*));
	if(Count != 6)
		terror("Walk wrong length");
	for(i = 1; i <= 6; ++i)
		if(!See[i] )
			terror("Bad walk");

	dtinsert(dt1,2L);

	Count = 0;
	for(i = (long)dtfirst(dt1); i; i = (long)dtnext(dt1,i))
		Count++;
	if(Count != 6)
		terror("Walk wrong length2");

	Count = 0;
	for(i = (long)dtlast(dt1); i; i = (long)dtprev(dt1,i))
		Count++;
	if(Count != 6)
		terror("Walk wrong length3");

	/* check union of elements in order across ordered sets */
	dtview(dt2,dt3);
	for(k = 1, i = (long)dtfirst(dt1); i; ++k, i = (long)dtnext(dt1,i) )
		if(i != k)
			terror("Elements not appearing in order");

	dtinsert(dt3, 10L);
	if((long)dtatmost(dt1,9L) != 8L)
		terror("dtatmost failed on an order set");
	if((long)dtatleast(dt1,9L) != 10L)
		terror("dtatleast failed on an order set");

	/* dt1: 1 3 5 2
	   dt2: 2 4 6 3
	   dt3: 2 7 6 8 10
	*/
	Count = 0;
	dtmethod(dt1,Dtset);
	dtmethod(dt2,Dtset);
	dtmethod(dt3,Dtset);
	for(i = (long)dtfirst(dt1); i; i = (long)dtnext(dt1,i))
		Count++;
	if(Count != 9)
		terror("Walk wrong length4");

	texit(0);
}
