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

tmain()
{
	Dt_t*		dt;

	/* testing Dtlist */
	if(!(dt = dtopen(&Disc,Dtlist)) )
		terror("dtopen list");
	if((long)dtinsert(dt,1L) != 1)
		terror("Dtlist insert 1");
	if((long)dtappend(dt,2L) != 2)
		terror("Dtlist append 2");
	if((long)dtappend(dt,3L) != 3)
		terror("Dtlist append 3");
	if((long)dtappend(dt,1L) != 1)
		terror("Dtlist append 1");
	if((long)dtappend(dt,2L) != 2)
		terror("Dtlist append 2");
	if((long)dtappend(dt,3L) != 3)
		terror("Dtlist append 3");

	if((long)dtlast(dt) != 3)
		terror("Dtlist dtlast");
	if((long)dtprev(dt,3L) != 2)
		terror("Dtlist dtprev 2");
	if((long)dtprev(dt,2L) != 1)
		terror("Dtlist dtprev 1");
	if((long)dtprev(dt,1L) != 3)
		terror("Dtlist dtprev 3");
	if((long)dtprev(dt,3L) != 2)
		terror("Dtlist dtprev 2");
	if((long)dtprev(dt,2L) != 1)
		terror("Dtlist dtprev 1");

	/* search to the first 3 */
	if((long)dtfirst(dt) != 1)
		terror("Dtlist dtfirst 1");
	if((long)dtsearch(dt,3L) != 3)
		terror("Dtlist search 3");
	if((long)dtinsert(dt,4L) != 4)
		terror("Dtlist insert 4");
	if((long)dtnext(dt,4L) != 3)
		terror("Dtlist next 3");
	if((long)dtappend(dt,5L) != 5)
		terror("Dtlist append 5");

	if((long)dtfirst(dt) != 1)
		terror("Dtlist dtfirst 1");
	if((long)dtnext(dt,1L) != 2)
		terror("Dtlist next 2");
	if((long)dtnext(dt,2L) != 4)
		terror("Dtlist next 4");
	if((long)dtnext(dt,4L) != 3)
		terror("Dtlist next 3");
	if((long)dtnext(dt,3L) != 5)
		terror("Dtlist next 5");
	if((long)dtnext(dt,5L) != 1)
		terror("Dtlist next 1");
	if((long)dtnext(dt,1L) != 2)
		terror("Dtlist next 2");
	if((long)dtnext(dt,2L) != 3)
		terror("Dtlist next 3");

	texit(0);
}
