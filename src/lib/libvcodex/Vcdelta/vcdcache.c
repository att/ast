/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#include	"vcdhdr.h"

/*	Functions to encode/decode COPY addresses based on the caches.
**
**	Written by Kiem-Phong Vo
*/

#if __STD_C
void vcdkaclose(Vcdcache_t* ka)
#else
void vcdkaclose(ka)
Vcdcache_t*	ka;
#endif
{
	if(ka)
		free(ka);
}

/* initialize address caches */
#if __STD_C
Vcdcache_t*  vcdkaopen(ssize_t s_near, ssize_t s_same)
#else
Vcdcache_t*  vcdkaopen(s_near, s_same)
ssize_t		s_near;
ssize_t		s_same;
#endif
{
	Vcdcache_t*	ka;
	ssize_t		sz;

	sz = sizeof(Vcdcache_t) + s_near*sizeof(ssize_t) + s_same*256*sizeof(ssize_t);

	if(!(ka = (Vcdcache_t*)calloc(1,sz)) )
		return NIL(Vcdcache_t*);
	ka->c_same = ka->c_near = NIL(ssize_t*);

	if((ka->s_near = s_near) > 0)
		ka->c_near = (ssize_t*)(ka+1);

	if((ka->s_same = s_same) > 0)
		ka->c_same = ((ssize_t*)(ka+1)) + s_near;

	return ka;
}

#if __STD_C
void vcdkaclear(Vcdcache_t* ka)
#else
void vcdkaclear(ka)
Vcdcache_t*	ka;
#endif
{
	ssize_t	i;

	if(ka)
	{	for(i = 0; i < ka->s_near; ++i)
			ka->c_near[i] = 0;
		ka->n = 0;
		for(i = 0; i < ka->s_same*256; ++i)
			ka->c_same[i] = 0;
	}
}

/* update address caches */
#if __STD_C
static void vcdkaupdate(Vcdcache_t* ka, ssize_t addr)
#else
static void vcdkaupdate(ka, addr)
Vcdcache_t*	ka;
ssize_t		addr;
#endif
{
	if(ka)
	{	if(ka->s_near > 0)
		{	ka->c_near[ka->n] = addr;
			if((ka->n += 1) >= ka->s_near)
				ka->n = 0;
		}
		if(ka->s_same > 0)
			ka->c_same[addr % (ka->s_same*256)] = addr;
	}
}

/* compute encoding for COPY addresses */
#if __STD_C
ssize_t vcdkasetaddr(Vcdcache_t* ka, ssize_t addr, ssize_t here, ssize_t* mode)
#else
ssize_t vcdkasetaddr(ka, addr, here, mode)
Vcdcache_t*	ka;
ssize_t		addr;	/* matching address to be encoded	*/
ssize_t		here;	/* current location			*/
ssize_t*	mode;	/* to return the coded address		*/
#endif
{
	ssize_t	i, d, sz, bestd, bestm, bestsz;

	bestd = addr;
	bestm = VCD_SELF;
	if((bestsz = vcsizeu(bestd)) == 1)
		goto done;

	d = here-addr;
	if((sz = vcsizeu(d)) < bestsz)
	{	bestd = d;
		bestm = VCD_HERE;
		if((bestsz = sz) == 1)
			goto done;
	}

	if(ka)
	{	for(i = 0; i < ka->s_near; ++i)
		{	if((d = addr - ka->c_near[i]) < 0)
				continue;
			if((sz = vcsizeu(d)) < bestsz)
			{	bestd = d;
				bestm = (VCD_HERE+1) + i;
				if((bestsz = sz) == 1)
					goto done;
			}
		}
		if(ka->s_same > 0 && ka->c_same[d = addr%(ka->s_same*256)] == addr)
		{	bestd = d%256;
			bestm = (VCD_HERE+1) + ka->s_near + d/256;
		}
	}

done:	vcdkaupdate(ka, addr);

	*mode = bestm;
	return bestd;
}


#if __STD_C
ssize_t vcdkagetaddr(Vcdcache_t* ka, Vcio_t* addr, ssize_t here, ssize_t mode)
#else
ssize_t vcdkagetaddr(ka, addr, here, mode)
Vcdcache_t*	ka;
Vcio_t*		addr;
ssize_t		here;
ssize_t		mode;
#endif
{
	ssize_t	a, m;

	if(mode == VCD_SELF)
		a = vciogetu(addr);
	else if(mode == VCD_HERE)
		a = here - vciogetu(addr);
	else if(ka)
	{	if((m = mode - (VCD_HERE+1)) >= 0 && m < ka->s_near)
			a = ka->c_near[m] + vciogetu(addr);
		else if((m = mode - (VCD_HERE+1+ka->s_near)) >= 0 && m < ka->s_same)
			a = ka->c_same[vciogetc(addr) + m*256];
		else	return -1;
	}
	else	return -1;

	vcdkaupdate(ka, a);
	return a;
}
