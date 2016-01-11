/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2013 AT&T Intellectual Property          *
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

/* test to see if the Dtuser_t structure has the right elements */

Dtdisc_t Disc =
	{ 0, sizeof(long), -1,
	  newint, NIL(Dtfree_f), compare, hashint,
	  NIL(Dtmemory_f), NIL(Dtevent_f)
	};

tmain()
{
	Dt_t		*dt;

	if(!(dt = dtopen(&Disc,Dtset)) )
		terror("Opening Dtset");

	if(dtuserlock(dt, 0, 1) >= 0 )
		terror("dtuserlock() should have failed because key == 0");

	if(dtuserlock(dt, 1111, 1) <  0 )
		terror("dtuserlock() should have succeeded to lock");
	if(dt->user->lock != 1111 )
		terror("user->lock should be 1111");

	if(dtuserlock(dt, 1111, -1) <  0 )
		terror("dtuserlock() should have succeeded to unlock");
	if(dt->user->lock != 0 )
		terror("user->lock should be 0");

	if(dtuserdata(dt, (Void_t*)11, 1) != (Void_t*)0)
		terror("dtuserdata() should have returned NULL");
	if(dt->user->data != (Void_t*)11)
		terror("user->data should be 11");

	if(dtuserdata(dt, (Void_t*)0, 0) != (Void_t*)11)
		terror("dtuserdata() should have returned 11");

	if(dtuserdata(dt, (Void_t*)22, 1) != (Void_t*)11)
		terror("dtuserdata() should have returned 11");
	if(dt->user->data != (Void_t*)22)
		terror("user->data should be 22");

	texit(0);
}
