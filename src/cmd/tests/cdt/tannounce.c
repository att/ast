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

static int	Event;

static int event(Dt_t* dt, int type, Void_t* obj, Dtdisc_t* disc)
{
	if(type&DT_ANNOUNCE)
		Event = type & ~DT_ANNOUNCE;
	return 0;
}

Dtdisc_t Disc =
{	0, sizeof(long), -1,
  	newint, NIL(Dtfree_f), compare, hashint, NIL(Dtmemory_f), event
};

tmain()
{
	int		k, i;
	Dt_t		*dt;
	Dtmethod_t	*Meth[8];
	
	Meth[0] = Dtoset;
	Meth[1] = Dtlist;
	Meth[2] = Dtset;
	Meth[3] = Dtrhset;
	Meth[4] = 0;

	for(k = 0; Meth[k]; ++k)
	{	if(!(dt = dtopen(&Disc, Meth[k])) )
			terror("Opening %s", Meth[k]->name);
		dtcustomize(dt, DT_ANNOUNCE, 1);

		dtinsert(dt, 1);
		if(Event != DT_INSERT)
			terror("Did not get dtinsert event %s", Meth[k]->name);

		dtinsert(dt, 2);
		if(Event != DT_INSERT)
			terror("Did not get dtinsert event %s", Meth[k]->name);

		i = (int)(long)dtfirst(dt);
		if(Event != DT_FIRST)
			terror("Did not get dtfirst event %s", Meth[k]->name);

		i = (int)(long)dtnext(dt,(long)i);
		if(Event != DT_NEXT)
			terror("Did not get dtnext event %s", Meth[k]->name);

		dtsearch(dt, 2);
		if(Event != DT_SEARCH)
			terror("Did not get dtsearch event %s", Meth[k]->name);

		dtdelete(dt, 2);
		if(Event != DT_DELETE)
			terror("Did not get dtdelete event %s", Meth[k]->name);

		if(dtsize(dt) != 1)
			terror("Wrong size %s", Meth[k]->name);

		dtclose(dt);
	}

	texit(0);
}
