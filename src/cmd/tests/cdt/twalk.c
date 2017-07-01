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

typedef struct _obj_s
{	Dtlink_t	link;
	long		key;
	long		ord;
} Obj_t;

static int objcmp(Dt_t* dt, Void_t* arg1, Void_t* arg2, Dtdisc_t* disc)
{
	Obj_t	*o1 = (Obj_t*)arg1, *o2 = (Obj_t*)arg2;

	return (int)(o1->key - o2->key);
}

static unsigned int objhash(Dt_t* dt, Void_t* arg, Dtdisc_t* disc)
{
	Obj_t	*o = (Obj_t*)arg;
	return dtstrhash(0, (char*)(&o->key), sizeof(long));
}

static char* objprint(Void_t* arg)
{
	Obj_t		*obj = (Obj_t*)arg;
	static char	buf[1024];

	sprintf(buf,"%ld,%ld", obj->key, obj->ord);
	return buf;
}

Dtdisc_t Disc = { 0, 0, 0, 0, 0, objcmp, objhash, 0, 0 };

#define N_OBJ	100000	/* total number of elements	*/
#define	N_PATH	100	/* need N_OBJ%NPATH == 0	*/
static Obj_t	Obj[N_OBJ];
static Obj_t	*Ord[N_OBJ];

tmain()
{
	int		i, k, p, meth;
	char		*name;
	Obj_t		*o, *obj;
	Void_t		*walk, *path[N_PATH];
	Dt_t		*dt;
	Dtstat_t	stat;

	/* construct objects */
	for(i = 0; i < N_OBJ; ++i)
	{	Obj[i].key = i;
		Obj[i].ord = i;
	}

	for(meth = 0; meth < 4; ++meth)
	{	switch(meth)
		{ case 0:
			name = "Dtoset";
			if(!(dt = dtopen(&Disc, Dtoset)) )
				terror("%s: Can't open dictionary", name);
			break;
		  case 1:
			name = "Dtset";
			if(!(dt = dtopen(&Disc, Dtset)) )
				terror("%s: Can't open dictionary", name);
			break;
		  case 2:
			name = "Dtlist";
			if(!(dt = dtopen(&Disc, Dtlist)) )
				terror("%s: Can't open dictionary", name);
			break;
		  case 3:
			name = "Dtrhset";
			if(!(dt = dtopen(&Disc, Dtrhset)) )
				terror("%s: Can't open dictionary", name);
			break;
		  default: terror("Unknown storage method");
			break;
		}
		tinfo("Testing method %s:", name);
		dtcustomize(dt, DT_SHARE, 1); /* make it more interesting */

		/* add all objects into dictionary */
		for(i = 0; i < N_OBJ; ++i)
		{	obj = Obj + i;
			o = (meth == 2 /* Dtlist */) ? dtappend(dt,obj) : dtinsert(dt,obj);
			if(o != obj)
				terror("%s: Inserting (key=%d,ord=%d) failed", name, obj->key, obj->ord);
		}

		/* construct the list of objects in walk order */
		tresource(-1,0);
		if(!(walk = dtstart(dt, NIL(Void_t*))) )
			terror("%s: Can't open walk", name);
		for(k = 0, o = dtstep(dt,walk); o; )
		{	Ord[k++] = o;
			o = dtstep(dt,walk);
		}
		if(k != N_OBJ)
			terror("%s: walk count is %d != expected %d", k, N_OBJ);
		tinfo("%s: Timing for walk of all %d objects in dictionary:", name, N_OBJ);
		tresource(0,0);

		/* now walk different parts of the list */
		tresource(-1,0);
		for(p = 0; p < N_PATH; ++p)
			if(!(path[p] = dtstart(dt, Ord[p*(N_OBJ/N_PATH)])) )
				terror("%s: Can't create a path at pos=%d", name, p*(N_OBJ/N_PATH) ); 
		for(k = 0; k < N_OBJ/N_PATH; ++k)
		{	for(p = 0;  p < N_PATH; ++p)
			{	if(!(o = dtstep(dt, path[p])) )
					terror("%s: missing an object", name);
				if(o != Ord[p*(N_OBJ/N_PATH) + k] )
					terror("%s: object=%d != expected %d", name,
						o->key, Ord[p*(N_OBJ/N_PATH) + k]->key);
			}
		}
		for(p = 0; p < N_PATH; ++p)
			dtstop(dt, path[p]);
		tinfo("%s: Timing for walk of %d paths:", name, N_PATH);
		tresource(0,0);

		dtstop(dt, walk);

		dtclose(dt);
	}

	texit(0);
}
