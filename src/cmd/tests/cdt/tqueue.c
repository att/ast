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

	/* testing Dtqueue */
	if(!(dt = dtopen(&Disc,Dtqueue)) )
		terror("dtopen queue");
	if((long)dtinsert(dt,1L) != 1)
		terror("Dtqueue insert 1");
	if((long)dtinsert(dt,3L) != 3)
		terror("Dtqueue insert 3.1");
	if((long)dtinsert(dt,2L) != 2)
		terror("Dtqueue insert 2.1");
	if((long)dtinsert(dt,3L) != 3)
		terror("Dtqueue insert 3.2");
	if((long)dtinsert(dt,2L) != 2)
		terror("Dtqueue insert 2.2");
	if((long)dtinsert(dt,3L) != 3)
		terror("Dtqueue insert 3.3");

	if((long)dtlast(dt) != 3)
		terror("Dtqueue dtlast");
	if((long)dtprev(dt,3L) != 2)
		terror("Dtqueue dtprev 3.3");
	if((long)dtprev(dt,2L) != 3)
		terror("Dtqueue dtprev 2.2");
	if((long)dtprev(dt,3L) != 2)
		terror("Dtqueue dtprev 3.2");
	if((long)dtprev(dt,2L) != 3)
		terror("Dtqueue dtprev 2.1");
	if((long)dtprev(dt,3L) != 1)
		terror("Dtqueue dtprev 3.1");
	if((long)dtprev(dt,1L) != 0)
		terror("Dtqueue dtprev 1");

	if((long)dtdelete(dt,NIL(Void_t*)) != 1)
		terror("Dtqueue pop 1");
	if((long)dtdelete(dt,NIL(Void_t*)) != 3)
		terror("Dtqueue delete 3.1");
	if((long)dtdelete(dt,NIL(Void_t*)) != 2)
		terror("Dtqueue delete 2");
	if((long)dtdelete(dt,NIL(Void_t*)) != 3)
		terror("Dtqueue delete 3.2");
	if((long)dtdelete(dt,NIL(Void_t*)) != 2)
		terror("Dtqueue delete 2.1");
	if((long)dtdelete(dt,NIL(Void_t*)) != 3)
		terror("Dtqueue delete 3.3");

	if(dtsize(dt) != 0)
		terror("Dtqueue size");

	texit(0);
}
