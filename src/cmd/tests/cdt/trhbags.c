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
	return (unsigned int)(o->key);
}

Dtdisc_t Disc = { 0, 0, 0, 0, 0, objcmp, objhash, 0, 0 };

#define N_OBJ	100	/* total number of elements	*/
#define R_OBJ	5	/* number of times to repeat	*/
#define N_CHK	10
static Obj_t	Obj[N_OBJ];

tmain()
{
	Dt_t*	dt;
	Obj_t	*o, proto;
	long	i, k, count, n;

	for(i = 0; i < N_OBJ; i = k)
	{	for(k = i; k < i+R_OBJ && k < N_OBJ; ++k)
		{	Obj[k].key = i;
			Obj[k].ord = k;
		}
	}

	for(k = 0; k < 2; ++k)
	{	if(!(dt = dtopen(&Disc, k == 0 ? Dtrhbag : Dtobag)) )
			terror("Opening dictionary");
		dtcustomize(dt, DT_SHARE, 1); /* turn on sharing */

		for(i = 0; i < N_OBJ; ++i)
		{	if(dtinsert(dt, Obj+i) != Obj+i)
				terror("Insert %d,%d", Obj[i].key, Obj[i].ord);

			if(i > 0 && (i%N_CHK) == 0)
				if((count = dtsize(dt)) != i+1)
					terror("Bad size %d (need %d)", count, i+1);
		}

		count = n = 0; /* count the group of elements with key == 0 */
		for(o = (Obj_t*)dtflatten(dt); o; o = (Obj_t*)dtlink(dt,o), count += 1)
			if(o->key == 0)
				n += 1;
		if(count != N_OBJ || n != R_OBJ)
			terror("flatten %s: count=%d(need=%d) n=%d(need=%d)",
				k == 0 ? "bag" : "obag", count, N_OBJ, n, R_OBJ);

		/* delete a bunch of objects */
		for(n = 0, i = 0; i < N_OBJ; i += R_OBJ, n += 1)
			if(!dtdelete(dt, Obj+i))
				terror("delete %s: i=%d",
					k == 0 ? "bag" : "obag", i);

		count = 0; /* count the left over */
		for(o = (Obj_t*)dtflatten(dt); o; o = (Obj_t*)dtlink(dt,o))
			count += 1;
		if(count != N_OBJ-n)
			terror("%s wrong count %d",
				k == 0 ? "bag" : "obag", count);

		dtclose(dt);
	}

	texit(0);
}
