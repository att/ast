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
#include	"vchdr.h"

/*	Transforming rows by xor-ing.
**
**	Written by Kiem-Phong Vo
*/

#define SIZE		2	/* #bytes coding a record size	*/
#define MAXSIZE		(1 << 16) /* max record size		*/
#define GETSIZE(dt)	(((dt)[0]<<8)+(dt)[1]) /* get AMA size	*/
#define PUTSIZE(dt,v)	(((dt)[0] = (((v)>>8)&0377)), ((dt)[1] = ((v)&0377)) )

/* Fast string xor-ing. Assumption: sizeof(type) is a power-of-2 <= 8 */
#define ALIGN(ptr,type)	(((unsigned char*)ptr - (unsigned char*)0) & (sizeof(type)-1) )
#define XOR(sx,s1,s2,sk,sn,algn,type) \
	do { if((sk+algn+sizeof(type)) <= sn) \
	     {	reg unsigned type *tx, *t1, *t2; \
	      	if(algn > 0) for(algn = sk+sizeof(type)-algn; sk < algn; ++sk) \
		    	*sx++ = *s1++ ^ *s2++; \
		tx = (unsigned type*)sx; t1 = (unsigned type*)s1; t2 = (unsigned type*)s2; \
		for(sn -= sizeof(type); sk <= sn; sk += sizeof(type)) \
		  	*tx++ = *t1++ ^ *t2++; \
		sn += sizeof(type); sx = (Vcchar_t*)tx; s1 = (Vcchar_t*)t1; s2 = (Vcchar_t*)t2; \
	     } \
	     for(; sk < sn; ++sk) \
		*sx++ = *s1++ ^ *s2++; \
	} while(0)

#if __STD_C
static ssize_t amadiff(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t amadiff(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	reg Vcchar_t	*dt, *df, *pd;
	ssize_t		k, a, ncols, sz, hdsz;
	Vcchar_t	*enddt, *output;
	Vcchar_t	*rcdt[MAXSIZE];
	Vcio_t		io;
	/**/DEBUG_DECLARE(int, i_align = 0)
	/**/DEBUG_DECLARE(int, s_align = 0)

	vc->undone = 0;
	if(size == 0)
		return 0;

	ncols = vc->disc ? vc->disc->size : 0;
	hdsz = vcsizeu(ncols);
	sz = ((hdsz + sizeof(int)-1)/sizeof(int))*sizeof(int);
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), size, sz)) )
		return -1;

	df = output; enddt = (dt = (Vcchar_t*)data) + size;
	if(ncols > 0) /* fixed-length rows */
	{	if((sz = (size/ncols)*ncols) > 0)
		{	memcpy(df, dt, ncols);
			pd = dt; df += ncols; dt += ncols;
			if((sz -= ncols) > 0)
			{	k = 0;
				if((a = ALIGN(df,int)) == ALIGN(dt,int) && a == ALIGN(pd,int) )
					XOR(df, dt, pd, k, sz,  a, int);
				else if((a = ALIGN(df,short)) == ALIGN(dt,short) && a == ALIGN(pd,short) )
					XOR(df, dt, pd, k, sz,  a, short);
				else for(; k < sz; ++k)
					*df++ = *dt++ ^ *pd++;
			}
		}
	}
	else /* ama data */
	{	for(k = 0; k < MAXSIZE; ++k)
			rcdt[k] = NIL(Vcchar_t*);
		for(;;)
		{	if((dt+SIZE) > enddt)
				break;
			ncols = GETSIZE(dt);
			if((dt+ncols) > enddt)
				break;

			pd = rcdt[ncols]; rcdt[ncols] = dt+SIZE;
			if(!pd)
			{	for(k = 0; k < ncols; ++k) /* first record is kept clear */
					*df++ = *dt++;
			}
			else
			{	for(k = 0; k < SIZE; ++k) /* record size is kept clear */
					*df++ = *dt++;

				/* xor-transforming the rest of the data */
				if((a = ALIGN(df,int)) == ALIGN(dt,int) && a == ALIGN(pd,int))
					XOR(df, dt, pd, k, ncols, a, int);
				else if((a = ALIGN(df,short)) == ALIGN(dt,short) && a == ALIGN(pd,short) )
					XOR(df, dt, pd, k, ncols, a, short);
				else for(; k < ncols; ++k)
					*df++ = *dt++ ^ *pd++;
			}
		}

		ncols = 0;
	}

	vc->undone = enddt-dt;
	if((sz = dt - (Vcchar_t*)data) == 0)
		return 0;

	dt = output;
	if(vcrecode(vc, &output, &sz, hdsz, 0) < 0 )
		return -1;
	if(dt != output)
		vcbuffer(vc, dt, -1, -1);

	output -= hdsz; sz += hdsz;
	vcioinit(&io, output, hdsz);
	vcioputu(&io, ncols);

	if(out)
		*out = output;

	return sz;
}

#if __STD_C
static ssize_t unamadiff(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t unamadiff(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*dt, *df, *savdf, *pd, *enddt, *output;
	ssize_t		k, a, ncols, sz;
	Vcchar_t	*rcdt[MAXSIZE];
	Vcio_t		io;

	vc->undone = 0;
	if(size == 0)
		return 0;

	vcioinit(&io, data, size);
	ncols = vciogetu(&io);

	df = savdf = vcionext(&io);
	sz = vciomore(&io);
	if(vcrecode(vc, &df, &sz, 0, 0) < 0 )
		return -1;

	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
		return -1;

	/* undo the transform */
	enddt = (dt = output) + sz;
	if(ncols > 0)
	{	if(sz%ncols != 0)
			return -1;
		memcpy(dt, df, ncols);
		for(pd = dt, df += ncols, dt += ncols; dt < enddt; )
			*dt++ = *df++ ^ *pd++;	
	}
	else
	{	for(k = 0; k < MAXSIZE; ++k)
			rcdt[k] = NIL(Vcchar_t*);
		for(;;)
		{	if((dt+SIZE) > enddt)
				break;
			ncols = GETSIZE(df);
			if((dt+ncols) > enddt)
				return -1;

			pd = rcdt[ncols]; rcdt[ncols] = dt+SIZE;
			if(!pd)
			{	for(k = 0; k < ncols; ++k)	
					*dt++ = *df++;
			}
			else
			{	for(k = 0; k < SIZE; ++k)
					*dt++ = *df++;

				/* xor-transforming the rest of the data */
				if((a = ALIGN(df,int)) == ALIGN(dt,int) && a == ALIGN(pd,int))
					XOR(dt, df, pd, k, ncols, a, int);
				else if((a = ALIGN(df,short)) == ALIGN(dt,short) && a == ALIGN(pd,short) )
					XOR(dt, df, pd, k, ncols, a, short);
				else for(; k < ncols; ++k)
					*dt++ = *df++ ^ *pd++;
			}
		}
	}

	vcbuffer(vc, savdf, -1, -1);

	if(out)
		*out = output;
	return sz;
}

Vcmethod_t _Vcamadiff =
{	amadiff,
	unamadiff,
	0,
	"amadiff", "Xor-ing rows in an AMA table.",
	"[-?\n@(#)$Id: vcodex-amadiff (AT&T Research) 2003-01-01 $\n]" USAGE_LICENSE,
	0,
	1024*1024,
	0
};

VCLIB(Vcamadiff)
