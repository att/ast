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
#include	"vmtest.h"

#define RAND()		trandom()

typedef struct _obj_s	Obj_t;
struct _obj_s
{	Void_t*	obj;	/* allocated object	*/
	size_t	size;	/* its allocated size	*/
	Obj_t*	next;	/* linked list pointer 	*/
};

#define N_OBJ		500000	/* #iterations and #objects */
static Obj_t		Obj[N_OBJ], *List[N_OBJ+1];

#define Z_HUGE		5000		/* huge block size increment	*/
#define Z_BIG		10000		/* size range of large objects	*/
#define Z_MED		1000		/* size range of medium objects	*/
#define Z_TINY		10		/* size range of small objects	*/

#define ALLOCSIZE()	(RAND()%100 == 0 ? (RAND()%Z_BIG + Z_BIG/2 ) : \
			 RAND()%10  == 0 ? (RAND()%Z_MED + Z_MED ) : \
					   (RAND()%Z_TINY + 1) )

/* when to resize or free */
#define TM_BIG		1000		/* life range of a big block 	*/
#define TM_MED		10		/* life range of a medium block	*/
#define TM_TINY		5		/* life range of a tiny block	*/
#define TMRANGE(zz)	(zz >= Z_BIG ? TM_BIG : zz >= Z_MED ? TM_MED : TM_TINY )
#define TIME(nk,kk,zz)	((nk = kk + (RAND() % TMRANGE(zz)) + 1), \
			 (nk = nk <= kk ? kk+1 : nk > N_OBJ ? N_OBJ : nk ) )

#define STAT(kk)	((kk % (N_OBJ/10)) == 0 ? 1 : 0)
#define RESIZE(kk)	((kk % (N_OBJ/500)) == 0 ? 1 : 0)

tmain()
{
	Obj_t		*o, *next;
	Void_t		*huge;
	size_t		hugesz;
	Vmstat_t	sb;
	ssize_t		k, p;

	srandom(0);

	hugesz = Z_HUGE; /* one huge block to be resized occasionally */
	if(!(huge = vmalloc(Vmregion, hugesz)) )
		terror("Can't allocate block");

	for(k = 0; k < N_OBJ; ++k)
	{	
		/* free/resize all on this list */
		for(o = List[k]; o; o = next)
		{	next = o->next;

			if((RAND()%2) == 0 ) /* flip a coin to see if freeing */
				vmfree(Vmregion, o->obj);
			else /* resizing */
			{	o->size = ALLOCSIZE();
				if(!(o->obj = vmresize(Vmregion,o->obj,o->size,VM_RSMOVE)) )
					terror("Vmresize failed");
				TIME(p, k, o->size); /* add to a future list */
				o->next = List[p]; List[p] = o;
			}
		}

		if(STAT(k))
		{	if(vmstat(Vmregion, &sb) < 0)
				terror("Vmstat failed");
			tinfo("Arena: busy=(%u,%u) free=(%u,%u) extent=%u #segs=%d",
				sb.n_busy,sb.s_busy, sb.n_free,sb.s_free, sb.extent, sb.n_seg);
		}

		if(RESIZE(k)) /* make the huge block bigger */
		{	hugesz += Z_HUGE;
			if(!(huge = vmresize(Vmregion, huge, hugesz, VM_RSMOVE)) )
				terror("Bad resize of huge block");
		}

		o = Obj+k; /* allocate a new block */
		o->size = ALLOCSIZE();
		if(!(o->obj = vmalloc(Vmregion, o->size)) )
			terror("Vmalloc failed");
		TIME(p, k, o->size);
		o->next = List[p]; List[p] = o;
	}

	if(vmdbcheck(Vmregion) < 0)
		terror("Corrupted region");

	if(vmstat(Vmregion, &sb) < 0)
		terror("Vmstat failed");
	tinfo("After simulation loop: Busy=(%u,%u) Free=(%u,%u) Extent=%u #segs=%d",
		sb.n_busy, sb.s_busy, sb.n_free, sb.s_free, sb.extent, sb.n_seg);

	/* now free all left-overs */
	for(o = List[N_OBJ]; o; o = o->next)
		vmfree(Vmregion,o->obj);
	vmfree(Vmregion,huge);

	if(vmstat(Vmregion, &sb) < 0)
		terror("Vmstat failed");
	tinfo("After freeing all: Busy=(%u,%u) Free=(%u,%u) Extent=%u #segs=%d",
		sb.n_busy, sb.s_busy, sb.n_free, sb.s_free, sb.extent, sb.n_seg);

	if(!(huge = vmalloc(Vmregion, 10)))
		terror("Vmalloc failed");
	if(vmstat(Vmregion, &sb) < 0)
		terror("Vmstat failed");
	tinfo("Allocating a small block: Busy=(%u,%u) Free=(%u,%u) Extent=%u #segs=%d",
		sb.n_busy, sb.s_busy, sb.n_free, sb.s_free, sb.extent, sb.n_seg);

	texit(0);
}
