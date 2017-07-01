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

Dtdisc_t Rdisc =
	{ 0, sizeof(long), -1,
	  newint, NIL(Dtfree_f), rcompare, hashint,
	  NIL(Dtmemory_f), NIL(Dtevent_f)
	};

tmain()
{
	Dt_t*		dt;
	Dtlink_t*	link;
	long		x, g, i, k, count[10];

	/* testing Dtobag */
	dt = dtopen(&Disc,Dtobag);
	x = 5;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	x = 2;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	x = 5;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	for(k = 0, i = (long)dtfirst(dt); i; k = i, i = (long)dtnext(dt,i))
		if(i < k)
			terror("Disorder %ld >= %ld", k, i);
	x = 3;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	x = 5;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	for(k = 0, i = (long)dtfirst(dt); i; k = i, i = (long)dtnext(dt,i))
		if(i < k)
			terror("Disorder %ld >= %ld", k, i);
	x = 4;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	x = 1;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	for(k = 0, i = (long)dtfirst(dt); i; k = i, i = (long)dtnext(dt,i))
		if(i < k)
			terror("Disorder %ld >= %ld", k, i);
	x = 2;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	x = 5;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	x = 4;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	x = 3;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	for(k = 0, i = (long)dtfirst(dt); i; k = i, i = (long)dtnext(dt,i))
		if(i < k)
			terror("Disorder %ld >= %ld", k, i);
	x = 4;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	x = 5;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	for(k = 0, i = (long)dtfirst(dt); i; k = i, i = (long)dtnext(dt,i))
		if(i < k)
			terror("Disorder %ld >= %ld", k, i);
	x = 3;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	x = 4;
	if((g = (char*)dtinsert(dt,x) - (char*)0) != x)
		terror("Insert -- expected %ld, got %ld", x, g);
	for(k = 0, i = (long)dtfirst(dt); i; k = i, i = (long)dtnext(dt,i))
		if(i < k)
			terror("Disorder %ld >= %ld", k, i);
	for(i = 0; i <= 5; ++i)
		count[i] = 0;
	for(i = (long)dtfirst(dt); i; i = (long)dtnext(dt,i))
		count[i] += 1;
	for(i = 0; i <= 5; ++i)
		if(count[i] != i)
			terror("dtnext count failed -- expected %d, got %ld", i, count[i]);
	for(i = 0; i <= 5; ++i)
		count[i] = 0;
	for(i = (long)dtlast(dt); i; i = (long)dtprev(dt,i))
		count[i] += 1;
	for(i = 0; i <= 5; ++i)
		if(count[i] != i)
			terror("dtprev count failed -- expected %d, got %ld", i, count[i]);
	for(k = 0, i = (long)dtfirst(dt); i; k = i, i = (long)dtnext(dt,i))
		if(i < k)
			terror("Disorder %ld >= %ld", k, i);

	for(link = dtflatten(dt), i = 1; link; ++i)
		for(k = 1; k <= i; ++k, link = dtlink(dt,link))
			if(i != (long)dtobj(dt,link))
				terror("Bad element");

	dtclear(dt);
	if(dtsize(dt) > 0 )
		terror("Non empty dictionary after clearing");

	for(i = 1; i <= 10; ++i)
	for(k = 1; k <= 10; ++k)
		if((long)dtinsert(dt, k) != k)
			terror("Can't insert k=%d at iteration %d", k, i);

	for(k = 0, i = (long)dtatmost(dt,5L); i == 5; i = (long)dtnext(dt,i) )
		k += 1;
	if(k != 10)
		terror("Did not see all 5's k=%d", k);

	for(k = 0, i = (long)dtatleast(dt,3L); i == 3; i = (long)dtprev(dt,i) )
		k += 1;
	if(k != 10)
		terror("Did not see all 3's k=%d", k);

	texit(0);
}
