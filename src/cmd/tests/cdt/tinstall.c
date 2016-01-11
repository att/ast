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

/* test the various insertion strategies */

typedef struct _obj_s
{	Dtlink_t	link;
	int		key;
} Obj_t;

static int intcompare(Dt_t* dt, Void_t* arg1, Void_t* arg2, Dtdisc_t* disc)
{
	int	*o1 = (int*)arg1;
	int	*o2 = (int*)arg2;
	return *o1 - *o2;
}

Dtdisc_t	Disc =
{	DTOFFSET(Obj_t, key), sizeof(int),  
	DTOFFSET(Obj_t, link),
	0, 0, intcompare, 0, 0, 0
};

tmain()
{
	Obj_t	obj, chk1, chk2, chk22;
	Dt_t	*dt;

	obj.key = 1;
	chk1.key = 1;
	chk2.key = 2;
	chk22.key = 2;

	if(!(dt = dtopen(&Disc, Dtoset)) )
		terror("Can't open Dtoset dictionary");
	if(dtinsert(dt, &obj) != &obj)
		terror("dtinsert failed");
	if(dtinsert(dt, &chk1) != &obj)
		terror("dtinsert should have returned obj");
	if(dtinsert(dt, &chk2) != &chk2)
		terror("dtinsert should have returned chk2");
	if(dtinstall(dt, &chk1) != &chk1)
		terror("dtinstall should have returned chk1");

	if(!(dt = dtopen(&Disc, Dtobag)) )
		terror("Can't open Dtobag dictionary");
	if(dtinsert(dt, &obj) != &obj)
		terror("dtinsert failed");
	if(dtinsert(dt, &chk1) != &chk1)
		terror("dtinsert should have returned chk1");
	if(dtinsert(dt, &chk2) != &chk2)
		terror("dtinsert should have returned chk2");
	if(dtinstall(dt, &chk1) != &chk1)
		terror("dtinsert should have returned chk1");
	if(dtinstall(dt, &chk22) != &chk22)
		terror("dtinsert should have returned chk22");

	if(!(dt = dtopen(&Disc, Dtset)) )
		terror("Can't open Dtset dictionary");
	if(dtinsert(dt, &obj) != &obj)
		terror("dtinsert failed");
	if(dtinsert(dt, &chk1) != &obj)
		terror("dtinsert should have returned obj");
	if(dtinsert(dt, &chk2) != &chk2)
		terror("dtinsert should have returned chk2");
	if(dtinstall(dt, &chk1) != &chk1)
		terror("dtinsert should have returned chk1");

	if(!(dt = dtopen(&Disc, Dtbag)) )
		terror("Can't open Dtbag dictionary");
	if(dtinsert(dt, &obj) != &obj)
		terror("dtinsert failed");
	if(dtinsert(dt, &chk1) != &chk1)
		terror("dtinsert should have returned chk1");
	if(dtinsert(dt, &chk2) != &chk2)
		terror("dtinsert should have returned chk2");
	if(dtinstall(dt, &chk1) != &chk1)
		terror("dtinsert should have returned chk1");
	if(dtinstall(dt, &chk22) != &chk22)
		terror("dtinsert should have returned chk22");

	if(!(dt = dtopen(&Disc, Dtrhset)) )
		terror("Can't open Dtrhset dictionary");
	if(dtinsert(dt, &obj) != &obj)
		terror("dtinsert failed");
	if(dtinsert(dt, &chk1) != &obj)
		terror("dtinsert should have returned obj");
	if(dtinsert(dt, &chk2) != &chk2)
		terror("dtinsert should have returned chk2");
	if(dtinstall(dt, &chk1) != &chk1)
		terror("dtinsert should have returned chk1");

	if(!(dt = dtopen(&Disc, Dtrhbag)) )
		terror("Can't open Dtrhbag dictionary");
	if(dtinsert(dt, &obj) != &obj)
		terror("dtinsert failed");
	if(dtinsert(dt, &chk1) != &chk1)
		terror("dtinsert should have returned chk1");
	if(dtinsert(dt, &chk2) != &chk2)
		terror("dtinsert should have returned chk2");
	if(dtinstall(dt, &chk1) != &chk1)
		terror("dtinsert should have returned chk1");
	if(dtinstall(dt, &chk22) != &chk22)
		terror("dtinsert should have returned chk22");

	texit(0);
}
