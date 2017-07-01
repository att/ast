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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#include	"vchhdr.h"

/*	Recursively partitioning a data string to improve Huffman coding
**
**	Written by Binh Dao Vo and Kiem-Phong Vo
*/

#define PT_NBIT		(3.00)	/* est. # of bits per code for table 	*/
#define PT_NONE		(1.00)	/* est. # of bits per nonappearing byte	*/
#define PT_MIN		(512)	/* minimum part size for part()	*/
#define PT_DIV		(3)	/* factor for subdividing a part	*/
#define PTMINSZ(sz)	((sz/PT_DIV) < PT_MIN ? PT_MIN : (sz/PT_DIV) )

typedef struct _part_s
{	Vcodex_t*	vch;	/* Huffman coder handle			*/
	size_t		ctab;	/* est. cost to encode a Huffman table	*/
	Vcio_t*		io;	/* where to write out compressed data	*/
} Part_t;

/* compute the byte frequencies and encoding cost of a string */
#if __STD_C
static double entropy(ssize_t* freq, Vcchar_t* data, size_t size)
#else
static double entropy(freq, data, size)
ssize_t*	freq;
Vcchar_t*	data;
size_t		size;
#endif
{
	double	e;
	int	i;

	CLRTABLE(freq, VCH_SIZE);
	ADDFREQ(freq, Vcchar_t*, data, size);

	for(e = 0., i = 0; i < VCH_SIZE; ++i)
		e += freq[i]*vclog(freq[i]);

	return e;
}

/* update a frequency array given another one adding/robbing it */
#if __STD_C
static double eupdate(double e, ssize_t* freq, ssize_t* fr, int add)
#else
static double eupdate(e, freq, fr, add)
double		e;	/* current entropy value	*/
ssize_t*	freq;	/* current frequency array	*/
ssize_t*	fr;	/* the one adding/robbing it	*/
int		add;	/* 1/0 for adding/subtracting	*/
#endif
{
	int	b;

	for(b = 0; b < VCH_SIZE; ++b)
	{	if(fr[b] == 0)
			continue;
		e -= freq[b]*vclog(freq[b]);
		if(add)
			freq[b] += fr[b];
		else	freq[b] -= fr[b];
		e += freq[b]*vclog(freq[b]);
	}

	return e;
}

#if __STD_C
static int addpart(Part_t* pt, Void_t* data, size_t size, ssize_t* freq)
#else
static int addpart(pt, data, size, freq)
Part_t*		pt;
Void_t*		data;	/* data to compress	*/
size_t		size;	/* size of data		*/
ssize_t*	freq;	/* frequency of bytes	*/
#endif
{
	Vcchar_t	*buf;
	ssize_t		sz;

	vchcopy(pt->vch, freq, NIL(ssize_t*), 0);
	if((sz = vcapply(pt->vch, data, size, &buf)) <= 0)
		return -1;
	vcioputu(pt->io, sz);
	vcioputs(pt->io, buf, sz);
	return 0;
}

/* recursively partition a dataset into parts with different statistical models */
#if __STD_C
static int part(Part_t* pt, Vcchar_t* data, size_t size, ssize_t* rfreq, double re)
#else
static int part(pt, data, size, pos, rfreq, re)
Part_t*		pt;	/* partition handle		*/
Vcchar_t*	data;	/* dataset to be partitioned	*/
size_t		size;	/* size of data			*/
ssize_t*	rfreq;	/* frequencies of all bytes	*/
double		re;	/* entropy of this dataset	*/
#endif
{
	double	le, lc, rc, cost;
	ssize_t	b, lfreq[VCH_SIZE];
	ssize_t	p, endp, minsz;

	/* cost of coding the entire data set */
	cost = pt->ctab + size*vclog(size) - re;

	/* see if large enough to split */
	minsz = PTMINSZ(size);
	if(size < 2*minsz)
		return addpart(pt, data, size, rfreq);

	/* first partition is at minsz */
	le = entropy(lfreq, data, minsz);
	re = eupdate(re, rfreq, lfreq, 0);
	
	/* loop adding 1 to left part and subtracting 1 from right part */
	for(p = minsz, endp = size-minsz; p < endp; )
	{	b = data[p++];
	
		/* update the costs of left&right parts */
		le -= lfreq[b]*vclog(lfreq[b]);
		re -= rfreq[b]*vclog(rfreq[b]);
		lfreq[b] += 1; rfreq[b] -= 1;
		le += lfreq[b]*vclog(lfreq[b]);
		re += rfreq[b]*vclog(rfreq[b]);

		lc = pt->ctab + p*vclog(p) - le;
		rc = pt->ctab + (size-p)*vclog(size-p) - re;
		if((lc + rc) < cost)
		{	/* good partition, recurse to do the two parts */
			if(part(pt, data, p, lfreq, le) < 0 )
				return -1;
			if(part(pt, data+p, size-p, rfreq, re) < 0)
				return -1;
			return 0;
		}
	}

	eupdate(re, rfreq, lfreq, 1); /* retotal the frequencies */
	return addpart(pt, data, size, rfreq);
}

#if __STD_C
static ssize_t pthuff(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t pthuff(vc, data, size, out)
Vcodex_t*	vc;	/* Vcodex handle		*/
Void_t*		data;	/* target data to be compressed	*/
size_t		size;	/* data size			*/
Void_t**	out;	/* to return output buffer 	*/
#endif
{
	ssize_t		n, c, freq[VCH_SIZE];
	double		e;
	Vcchar_t	*output, *dt;
	Vcio_t		io;
	Part_t		*pt = vcgetmtdata(vc, Part_t*);

	if(size == 0)
		return 0;

	/* estimate table entropy */
	e = entropy(freq, (Vcchar_t*)data, size);

	/* estimate the cost of emitting a Huffman table */
	for(c = 0, n = 0; n < VCH_SIZE; ++n)
		if(freq[n])
			c += 1;
	pt->ctab = c*PT_NBIT + (VCH_SIZE-c)*PT_NONE;

	/* get buffer space to write data */
	n = size + VCH_SIZE;
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), n, 0)) )
		return -1;
	vcioinit(&io, output, n);
	pt->io = &io;

	vcioputu(&io, size); /* write out the original size */

	/* recursively partition into smaller segments and compress */
	if(part(pt, (Vcchar_t*)data, size, freq, e) < 0)
		return -1;

	pt->io = NIL(Vcio_t*);

	dt = output;
	n = vciosize(&io); /* compressed data size */
	if(vcrecode(vc, &output, &n, 0, 0) < 0)
		return -1;
	if(dt != output) /* free unused buffers */
		vcbuffer(vc, dt, -1, -1);
	vcbuffer(pt->vch, NIL(Vcchar_t*), -1, -1);

	if(out)
		*out = output;

	return n;
}

#if __STD_C
static ssize_t ptunhuff(Vcodex_t* vc, const Void_t* orig, size_t size, Void_t** out)
#else
static ssize_t ptunhuff(vc, orig, size, out)
Vcodex_t*	vc;	/* Vcodex handle		*/
Void_t*		orig;	/* target data to be matched	*/
size_t		size;	/* data size			*/
Void_t**	out;	/* to return output buffer 	*/
#endif
{
	Vcchar_t	*output, *dt, *data;
	ssize_t		sz, os, s, n;
	Vcio_t		rd, wr;
	Part_t		*pt = vcgetmtdata(vc, Part_t*);

	if(size == 0)
		return 0;

	data = (Vcchar_t*)orig; sz = (ssize_t)size;
	if(vcrecode(vc, &data, &sz, 0, 0) < 0 )
		return -1;
	size = sz;

	/* get the size of uncompressed data */
	vcioinit(&rd, data, size);
	if((sz = vciogetu(&rd)) <= 0)
		return -1;

	/* set up buffer */
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
		return -1;
	vcioinit(&wr, output, sz);

	for(os = 0; os < sz; )
	{	/* get the size and compressed data of this segment */
		if((n = vciogetu(&rd)) <= 0)
			return -1;
		dt = vcionext(&rd);
		vcioskip(&rd, n);
		
		/* decompress the data */
		if((s = vcapply(pt->vch, dt, n, &dt)) <= 0)
			return -1;

		/* write out data */
		if((os += s) > sz)
			return -1;
		vcioputs(&wr, dt, s);
	}

	/* free unused buffers */
	if(data != orig)
		vcbuffer(vc, data, -1, -1);
	vcbuffer(pt->vch, NIL(Vcchar_t*), -1, -1);

	sz = vciosize(&wr);

	if(out)
		*out = output;
	return sz;
}

/* Event handler */
#if __STD_C
static int ptevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int ptevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Part_t	*pt;

	if(type == VC_OPENING)
	{	if(!(pt = (Part_t*)malloc(sizeof(Part_t))) )
			return -1;

		/* open the entropy coder handle */
		if(!(pt->vch = vcopen(NIL(Vcdisc_t*), Vchuffman, 0, 0, vc->flags)) )
		{	free(pt);
			return -1;
		}

		pt->ctab = 0;
		pt->io = NIL(Vcio_t*);

		vcsetmtdata(vc, pt);
		return 0;
	}
	else if(type == VC_CLOSING)
	{	if((pt = vcgetmtdata(vc, Part_t*)) )
		{	if(pt->vch)
				vcclose(pt->vch);
			free(pt);
		}
		vcsetmtdata(vc, NIL(Part_t*));
		return 0;
	}
	else if(type == VC_FREEBUFFER)
	{	if((pt = vcgetmtdata(vc, Part_t*)) && pt->vch)
			vcbuffer(pt->vch, NIL(Vcchar_t*), -1, -1);
		return 0;
	}
	else	return 0;
}

Vcmethod_t	_Vchuffpart =
{	pthuff,
	ptunhuff,
	ptevent,
	"huffpart", "Huffman encoding by partitioning.",
	"[-version?huffpart (AT&T Research) 2003-01-01]" USAGE_LICENSE,
	NIL(Vcmtarg_t*),
	1024*1024,
	0
};

VCLIB(Vchuffpart)
