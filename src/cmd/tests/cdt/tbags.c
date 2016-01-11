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
	return (unsigned int)(o->key/8); /* cause hash collisions */
}

static char* objprint(Void_t* arg)
{
	Obj_t		*obj = (Obj_t*)arg;
	static char	buf[1024];

	sprintf(buf,"%ld,%ld", obj->key, obj->ord);
	return buf;
}

Dtdisc_t Disc = { 0, 0, 0, 0, 0, objcmp, objhash, 0, 0 };

#define N_OBJ	10000	/* total number of elements	*/
#define R_OBJ	10	/* repetition: N_OBJ%R_OBJ == 0	*/
static Obj_t	Obj[N_OBJ];

tmain()
{
	Dt_t		*dt;
	Dtstat_t	stat;
	Obj_t		*p, *o, *obj, proto;
	char		*name;
	long		i, k, mid, n_mid, n_obj, meth;

	/* construct repetitive objects */
	for(i = 0; i < N_OBJ; i += R_OBJ)
	{	for(k = 0; k < R_OBJ; ++k)
		{	Obj[i+k].key = i;
			Obj[i+k].ord = k;
		}
	}

	for(meth = 0; meth < 4; ++meth)
	{	switch(meth)
		{ case 0:
			name = "Dtobag";
			if(!(dt = dtopen(&Disc, Dtobag)) )
				terror("%s: Can't open dictionary", name);
			break;
		  case 1:
			name = "Dtbag";
			if(!(dt = dtopen(&Disc, Dtbag)) )
				terror("%s: Can't open dictionary", name);
			break;
		  case 2:
			name = "Dtrhbag";
			if(!(dt = dtopen(&Disc, Dtrhbag)) )
				terror("%s: Can't open dictionary", name);
			break;
		  case 3:
			name = "Dtlist";
			if(!(dt = dtopen(&Disc, Dtlist)) )
				terror("%s: Can't open dictionary", name);
			break;
		  default: terror("Unknown storage method");
			break;
		}
		tinfo("Testing method %s:", name);
		dtcustomize(dt, DT_SHARE, 1); /* make it more interesting */

		/* add all objects into dictionary */
		for(k = 0; k < R_OBJ; ++k)
		for(i = 0; i < N_OBJ/R_OBJ; ++i)
		{	obj = Obj + i*R_OBJ + k;
			o = (meth == 3 || i%2 == 0) ? dtappend(dt,obj) : dtinsert(dt,obj);
			if(o != obj)
				terror("%s: dtappend (key=%d,ord=%d) failed", name, obj->key, obj->ord);
		}

		mid = ((N_OBJ/R_OBJ)/2) * R_OBJ; /* key for middle group */
		proto.key = mid;
		proto.ord = -1;

		if(meth == 3) /* testing ATMOST/ATLEAST for Dtlist */
		{	/* note that dtappend() was used to keep objects in order of insertion */
			if(!(o = dtatmost(dt, &proto)) )
				terror("%s: dtatmost (key=%d) failed", name, mid);
			if(o->ord != 0)
				terror("%s: dtatmost (key=%d) but ord=%d > 0", name, o->key, o->ord);

			if(!(o = dtatleast(dt, &proto)) )
				terror("%s: dtatleast (key=%d) failed", name, mid);
			if(o->ord != R_OBJ-1)
				terror("%s: dtatleast (key=%d) but ord=%d > 0", name, o->key, o->ord);

			n_obj = 0; /* test ordering */
			for(p = NIL(Obj_t*), o = dtfirst(dt); o; p = o, o = dtnext(dt,o) )
			{	n_obj += 1;
				if(p && p->ord > o->ord)
					terror("%s: objects not ordered correctly p=%d > o=%d",
						name, p->ord, o->ord);
			}
			if(n_obj != N_OBJ)
				terror("%s: Bad object count %d != %d", n_obj, N_OBJ);
		}

		if(meth == 0) /* testing ordering properties of Dtobag */
		{	
			n_obj = 0; /* test atmost/next */
			for(o = dtatmost(dt, &proto); o; o = dtnext(dt,o) )
			{	if(o->key == mid)
					n_obj += 1;
				else	break;
			}
			if(n_obj != R_OBJ)
				terror("%s: dtatmost/dtnext count n_obj=%d != %d", name, n_obj, R_OBJ);

			n_obj = 0; /* test atleast/prev */
			for(o = dtatleast(dt, &proto); o; o = dtprev(dt,o) )
			{	if(o->key == mid)
					n_obj += 1;
				else	break;
			}
			if(n_obj != R_OBJ)
				terror("%s: dtatleast/dtprev count n_obj=%d != %d", name, n_obj, R_OBJ);

			n_obj = 0; /* test linear order */
			for(p = NIL(Obj_t*), o = dtfirst(dt); o; p = o, o = dtnext(dt,o) )
			{	n_obj += 1;
				if(p && p->key > o->key)
					terror("%s: objects not ordered correctly p=%d > o=%d",
						name, p->key, o->key);
			}
			if(n_obj != N_OBJ)
				terror("%s: Bad object count %d != %d", n_obj, N_OBJ);
		}

		n_mid = n_obj = 0; /* walk forward and count objects */
		for(o = dtfirst(dt); o; o = dtnext(dt,o))
		{	n_obj += 1;
			if(o->key == mid)
				n_mid += 1;
		}
		if(n_obj != N_OBJ)
			terror("%s: Walk forward n_obj=%d != %d", name, n_obj, N_OBJ);
		if(n_mid != R_OBJ)
			terror("%s: Walk forward n_mid=%d != %d", name, n_mid, R_OBJ);

		n_mid = n_obj = 0; /* walk backward and count objects */
		for(o = dtlast(dt); o; o = dtprev(dt,o))
		{	n_obj += 1;
			if(o->key == mid)
				n_mid += 1;
		}
		if(n_obj != N_OBJ)
			terror("%s: Walk backward n_obj=%d != %d", name, n_obj, N_OBJ);
		if(n_mid != R_OBJ)
			terror("%s: Walk backward n_mid=%d != %d", name, n_mid, R_OBJ);

		n_mid = n_obj = 0; /* walk flattened list and count objects */
		for(o = (Obj_t*)dtflatten(dt); o; o = (Obj_t*)dtlink(dt,o) )
		{	n_obj += 1;
			if(o->key == mid)
				n_mid += 1;
		}
		if(n_obj != N_OBJ)
			terror("%s: Walk flattened list n_obj=%d != %d", name, n_obj, N_OBJ);
		if(n_mid != R_OBJ)
			terror("%s: Walk flattened list n_mid=%d != %d", name, n_mid, R_OBJ);

		n_mid = 0; /* delete a bunch of objects */
		for(i = 0; i < N_OBJ-1; i += R_OBJ)
		{	obj = Obj + i + R_OBJ/2; /* use the one in the middle of group */

			if((o = dtremove(dt, obj)) == obj )
				n_mid += 1;
			else	terror("%s: dtremove (key=%d,ord=%d) wrongly yielded (key=%d,ord=%d)",
					name, obj->key, obj->ord, o->key, o->ord);

			if((o = dtremove(dt, obj)) != NIL(Obj_t*) )
				terror("%s: dtremove (key=%d,ord=%d) wrongly yielded (key=%d,ord=%d)",
					name, obj->key, obj->ord, o->key, o->ord);

			if((o = dtdelete(dt, obj)) != NIL(Obj_t*) )
				n_mid += 1;
			else	terror("%s: dtdelete matching object to (key=%d,ord=%d) failed",
					name, obj->key, obj->ord);
		}

		n_obj = 0; /* count the left over */
		for(o = (Obj_t*)dtflatten(dt); o; o = (Obj_t*)dtlink(dt,o))
			n_obj += 1;
		if((n_obj+n_mid) != N_OBJ)
			terror("%s: wrong count (n_obj=%d + n_del=%d) != %d", name, n_obj, n_mid, N_OBJ);

		dtcustomize(dt, DT_OPTIMIZE, 1); /* balance the tree in Dtobag to reduce depth */
		if(dtstat(dt, &stat) < 0 )
			terror("%s: Couldn't get statistics", name);
		if(stat.size != n_obj)
			terror("%s: stat.size=%d != %d (actual size)", name, stat.size, n_obj);

		dtclose(dt);
	}

	texit(0);
}
