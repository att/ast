/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#include	<vclib.h>

/*	Burrows-Wheeler transform.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
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
	ssize_t		hd, sp, sz, *idx, *endidx;
	Vcio_t		io;
	Vcchar_t	*dt, *output, *bw;
	ssize_t		rv = -1;

	if(size == 0)
		return 0;

	/* compute suffix array */
	if(!(sfx = vcsfxsort(data,size)) )
		RETURN(-1);

	/* compute the location of the sentinel */
	for(endidx = (idx = sfx->idx)+size; idx < endidx; ++idx)
		if(*idx == 0)
			break;
	sp = idx - sfx->idx;

	hd = vcsizeu(sp); /* encoding size of the 0th position after sorting */
	sz = size + 1; /* size of transformed data */
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, hd)) )
		goto done;

	/* compute the transform */
	dt = (Vcchar_t*)data; bw = output;
	for(idx = sfx->idx; idx < endidx; ++idx, ++bw)
	{	if(*idx == 0) /* special coding of the 0th position */
			*bw = idx == sfx->idx ? 0 : *(bw-1);
		else	*bw = dt[*idx - 1];
	}
	*bw = dt[size-1];

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
	ssize_t		n, sp, sz;
	Vcchar_t	*dt, *output, *o;
	ssize_t		base[256], *offset;
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

	if(!(offset = (ssize_t*)malloc((sz+1)*sizeof(ssize_t))) )
		RETURN(-1);

	/* base and offset vector for bytes */
	for(n = 0; n < 256; ++n)
		base[n] = 0;
	for(n = 0; n <= sz; ++n)
	{	if(n == sp)
			offset[n] = 0;
		else
		{	offset[n] = base[dt[n]];
			base[dt[n]] += 1;
		}
	}
	for(sp = 0, n = 0; n < 256; ++n)
	{	ssize_t	c = base[n];
		base[n] = sp;
		sp += c;
	}

	/* now invert the transform */
	for(n = sz, o = output+sz-1; o >= output; --o)
	{	*o = dt[n];
		n = base[*o] + offset[n];
	}

	free(offset);
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
	"[-version?bwt (AT&T Research) 2003-01-01]" USAGE_LICENSE,
	NIL(Vcmtarg_t*),
	4*1024*1024,
	0
};

VCLIB(Vcbwt)
