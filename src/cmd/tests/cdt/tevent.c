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

static int	Event[8];
static int	Index;

static int event(Dt_t* dt, int type, Void_t* obj, Dtdisc_t* disc)
{
	if(Index >= sizeof(Event)/sizeof(Event[0]))
		Index = 0;
	Event[Index++] = type;

	if(type == DT_HASHSIZE)
	{	*(ssize_t*)obj = 1024;
		return 1;
	}

	return 0;
}

static int chkevent(int type)
{
	int	i, cnt;

	if(type < 0)
	{	memset(Event, 0, sizeof(Event));
		Index = 0;
		return 0;
	}
	else
	{	cnt = 0;
		for(i = 0; i < sizeof(Event)/sizeof(Event[0]); ++i )
			if(Event[i] == type)
				cnt += 1;
		return cnt;
	}
}

static Void_t* memory(Dt_t* dt, Void_t* obj, size_t size, Dtdisc_t* disc)
{
	if(!obj)
		return malloc(size);
	else
	{	free(obj);
		return NIL(Void_t*);
	}
}

Dtdisc_t Disc =
{	0, sizeof(long), -1,
  	newint, NIL(Dtfree_f), compare, hashint, memory, event
};

tmain()
{
	Dt_t		*dt;
	long		k;

	chkevent(-1);
	if(!(dt = dtopen(&Disc,Dtset)) )
		terror("Opening Dtset");
	if(chkevent(DT_OPEN) != 1 )
		terror("Bad count of DT_OPEN event");
	if(chkevent(DT_ENDOPEN) != 1 )
		terror("Bad count of DT_ENDOPEN event");

	dtinsert(dt, 1);
	if(chkevent(DT_HASHSIZE) != 1 )
		terror("No hash table size event");

	chkevent(-1);
	dtmethod(dt,Dtoset);
	if(chkevent(DT_METH) < 1 )
		terror("No meth event");
	
	chkevent(-1);
	dtdisc(dt,&Disc,0);
	if(chkevent(DT_DISC) < 1 )
		terror("No disc event");

	chkevent(-1);
	dtclose(dt);
	if(chkevent(DT_CLOSE) != 1 )
		terror("Bad count of DT_CLOSE event");
	if(chkevent(DT_ENDCLOSE) != 1 )
		terror("Bad count of DT_ENDCLOSE event");

	texit(0);
}
