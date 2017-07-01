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

/*	Burrows-Wheeler transform.
**
**	Written by Kiem-Phong Vo
*/

#if __STD_C
static ssize_t vcbwt(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t vcbwt(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcsfx_t		*sfx;
	ssize_t		sz;
	Vcinx_t		hd, sp, *idx, k;
	Vcio_t		io;
	Vcchar_t	*dt, *output, *bw;
	Vcinx_t		rv = -1;

	if(size == 0)
		return 0;

	/* compute suffix array */
	if(!(sfx = vcsfxsort(data,size)) )
		RETURN(-1);
	idx = sfx->idx;

	/* compute the location of the sentinel */
	for(sp = 0; sp < size; ++sp)
		if(idx[sp] == 0)
			break;
	/**/DEBUG_ASSERT(sp < size);

	hd = vcsizeu(sp); /* encoding size of the 0th position after sorting */
	sz = size + 1; /* size of transformed data */
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, hd)) )
		goto done;

	/* compute the transform */
	dt = (Vcchar_t*)data; bw = output;
	for(k = 0; k < size; ++k)
	{	if(idx[k] == 0) /* special coding of the 0th position */
		{	/**/DEBUG_ASSERT(k == sp);
			bw[k] = k == 0 ? 0 : bw[k-1];
		}
		else
		{	/**/DEBUG_ASSERT(idx[k] < size);
			bw[k] = dt[idx[k] - 1];
		}
	}
	bw[size] = dt[size-1];

	/* filter data thru the continuation coder */
	dt = output;
	if(vcrecode(vc, &output, &sz, hd, 0) < 0 )
		goto done;
	if(dt != output) /* got a new buffer, free old one */
		vcbuffer(vc, dt, -1, -1);

	/* write out the sorted location of position-0 in data */
	output -= hd;
	vcioinit(&io, output, hd);
	vcioputu(&io, sp);
	rv = hd+sz;

	/* truncate buffer to size */
	if(!(output = vcbuffer(vc, output, rv, -1)) )
		RETURN(-1);
	if(out)
		*out = output;
done:	free(sfx);
	return rv;
}

#if __STD_C
static ssize_t vcunbwt(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t vcunbwt(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		n, k, b, sp, sz;
	Vcchar_t	*dt, *output;
	ssize_t		base[256], *pick;
	Vcio_t		io;

	if(size == 0)
		return 0;

	vcioinit(&io, data, size);

	if((sp = vciogetu(&io)) < 0) /* index of 0th position */
		RETURN(-1);

	/* retrieve transformed data */
	sz = vciomore(&io);
	dt = vcionext(&io);

	/* invert continuation coder if there was one */
	if(vcrecode(vc, &dt, &sz, 0, 0) < 0 )
		RETURN(-1);
	sz -= 1; /* actual data size */

	if(sp >= sz) /* corrupted data */
		RETURN(-1);

	/* get space to decode */
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
		RETURN(-1);

	if(!(pick = (ssize_t*)malloc((sz+1)*sizeof(ssize_t))) )
		RETURN(-1);

	for(k = 0; k < 256; ++k)
		base[k] = 0;
	for(n = 0; n <= sz; ++n)
	{	if(n == sp)
			continue;
		pick[n] = base[dt[n]];
		base[dt[n]] += 1;
	}

	for(b = 0, n = 0; n < 256; ++n)
	{	k = base[n];
		base[n] = b;
		b += k;
	}

	/* construct string from right to left */
	for(n = sz, k = sz-1; k >= 0; --k)
	{	output[k] = dt[n];
		n = base[dt[n]] + pick[n]; /**/DEBUG_ASSERT(n != sp || k == 0);
	}

	free(pick);
	vcbuffer(vc, dt, -1, -1);

	if(out)
		*out = output;
	return sz;
}


Vcmethod_t _Vcbwt =
{	vcbwt,
	vcunbwt,
	0,
	"bwt", "Burrows-Wheeler transform.",
	"[-?\n@(#)$Id: vcodex-bwt (AT&T Research) 2009-02-22 $\n]" USAGE_LICENSE,
	NIL(Vcmtarg_t*),
	2*1024*1024,
	0
};

VCLIB(Vcbwt)
