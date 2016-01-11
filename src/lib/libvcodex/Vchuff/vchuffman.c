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
#include	"vchhdr.h"

/*	Static Huffman encoding.
**
**	Written by Kiem-Phong Vo
*/

/* internal handle for Huffman coder and decoder */
typedef struct _vchuff_s
{	ssize_t		freq[VCH_SIZE]; /* frequencies of bytes	*/
	ssize_t		size[VCH_SIZE]; /* encoding lengths 	*/
	Vcbit_t		bits[VCH_SIZE]; /* encoding bits 	*/
	ssize_t		maxs;	   	/* max code size	*/
	Vchtrie_t*	trie;		/* re-trie for decoding	*/
	int		type;
} Vchuff_t;

/* sentinels to tell that application has set these tables */
#define VCH_HASFREQ	(1<<15)
#define VCH_HASSIZE	(1<<14)

/* Copy frequencies, code sizes and code bits into a Vcodex handle. */
#if __STD_C
int vchcopy(Vcodex_t* vc, ssize_t* freq, ssize_t* size , ssize_t maxs)
#else
int vchcopy(vc, freq, size, maxs )
Vcodex_t*	vc;
ssize_t*	freq; /* byte frequency table	*/
ssize_t*	size; /* code length table	*/
ssize_t		maxs; /* max length of any code	*/
#endif
{
	Vchuff_t	*huff = vcgetmtdata(vc, Vchuff_t*);

	huff->type = 0;
	huff->maxs = 0;
	if(freq)
	{	memcpy(huff->freq, freq, VCH_SIZE*sizeof(freq[0]));
		huff->type |= VCH_HASFREQ;
	}
	if(size && maxs >= 0 && maxs < 32)
	{	memcpy(huff->size, size, VCH_SIZE*sizeof(size[0]));
		huff->maxs =  maxs;
		huff->type |= VCH_HASSIZE;
	}

	return 0;
}

#if __STD_C
static ssize_t vchuff(Vcodex_t* vc, const Void_t* data, size_t dtsz, Void_t** out)
#else
static ssize_t vchuff(vc, data, dtsz, out)
Vcodex_t*	vc;
Void_t*		data;	/* data to be encoded	*/
size_t		dtsz;
Void_t**	out;	/* return encoded data 	*/
#endif
{
	reg Vcbit_t	b;
	reg ssize_t	n;
	ssize_t		s;
	Vcchar_t	*output, *dt, *enddt;
	Vcio_t		io;
	Vchuff_t	*huff = vcgetmtdata(vc, Vchuff_t*);
	ssize_t		*freq = huff->freq;
	ssize_t		*size = huff->size;
	Vcbit_t		*bits = huff->bits;

	if(dtsz == 0) /* no data to compress */
		return 0;

	/* compute code frequencies and lengths */
	if(!(huff->type&VCH_HASFREQ) )
	{	CLRTABLE(freq, VCH_SIZE);
		ADDFREQ(freq, Vcchar_t*, data, dtsz);
		huff->type = 0;
	}
	if(!(huff->type&VCH_HASSIZE) &&
	   (huff->maxs = vchsize(VCH_SIZE,freq,size,NIL(int*))) < 0)
		return -1;
	huff->type = 0;

	/* estimate compressed size */
	if(huff->maxs > 0)
	{	DOTPRODUCT(s, freq, size, VCH_SIZE);
		s = (s+7)/8; /* round up to byte count */
	}
	else	s = 1;
	s += vcsizeu(dtsz) + 1;

	/* set up buffer for output */
	s += VCH_SIZE;
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), s, 0)) )
		return -1;
	vcioinit(&io, output, s);

	huff->maxs = vchbits(VCH_SIZE, size, bits); /* compute bits per byte */
	vcioputu(&io, dtsz); /* size of raw data */
	vcioputc(&io, huff->maxs); /* the control code */

	/**/DEBUG_PRINT(2,"Vchuff: inputsz=%d ",dtsz);
	enddt = (dt = (Vcchar_t*)data) + dtsz;
	if(huff->maxs == 0)  /* a single run */
		vcioputc(&io, *dt);
	else
	{	/* output the code tree */
		if((s = vchputcode(VCH_SIZE, size, huff->maxs, vcionext(&io), vciomore(&io))) <= 0)
			return -1;
		else	vcioskip(&io,s);
		/**/DEBUG_PRINT(2,"headsz=%d ",vciosize(&io));

		vciosetb(io, b, n, VC_ENCODE);
		for(; dt < enddt; ++dt)
			vcioaddb(&io, b, n, bits[*dt], size[*dt]);
		vcioendb(&io, b, n, VC_ENCODE); /* flush any remaining bits */
	}

	dt = output;
	s = vciosize(&io); /**/DEBUG_PRINT(2,"cmpsz=%d\n",s);
	if(vcrecode(vc, &output, &s, 0, 0) < 0 )
		return -1;
	if(dt != output)
		vcbuffer(vc, dt, -1, -1);

	if(out) 
		*out = output;
	return s;
}

#if __STD_C
static ssize_t vcunhuff(Vcodex_t* vc, const Void_t* orig, size_t dtsz, Void_t** out)
#else
static ssize_t vcunhuff(vc, orig, dtsz, out)
Vcodex_t*	vc;
Void_t*		orig;	/* data to be decoded	*/
size_t		dtsz;
Void_t**	out;	/* return decoded data	*/
#endif
{
	reg Vcbit_t	b;
	ssize_t		n, p, sz, ntop;
	short		*node, *size;
	Vcchar_t	*o, *endo, *output, *data;
	Vcio_t		io;
	Vchuff_t	*huff = vcgetmtdata(vc, Vchuff_t*);

	if(dtsz == 0)
		return 0;

	data = (Vcchar_t*)orig; sz = (ssize_t)dtsz;
	if(vcrecode(vc, &data, &sz, 0, 0) < 0 )
		return -1;
	dtsz = sz;

	vcioinit(&io, data, dtsz);
	sz = (ssize_t)vciogetu(&io);

	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), sz, 0)) )
		return -1;
	endo = (o = output)+sz;

	if((sz = vciogetc(&io)) == 0 )
	{	n = vciogetc(&io);
		while(o < endo)
			*o++ = n; 
	}
	else
	{	if((n = vchgetcode(VCH_SIZE, huff->size, sz, vcionext(&io), vciomore(&io))) < 0)
			return -1;
		else	vcioskip(&io,n);

		if(vchbits(VCH_SIZE, huff->size, huff->bits) < 0)
			return -1;
		if(huff->trie)
			vchdeltrie(huff->trie);
		if(!(huff->trie = vchbldtrie(VCH_SIZE, huff->size, huff->bits)) )
			return -1;
		node = huff->trie->node;
		size = huff->trie->size;
		ntop = huff->trie->ntop;

		vciosetb(&io, b, n, VC_DECODE); /* associate b,n as bit vector */
		for(sz = ntop, p = 0;; )
		{	vciofilb(&io, b, n, sz);

			p += (b >> (VC_BITSIZE-sz)); /* slot to look into */
			if(size[p] > 0) /* byte is found */
			{	vciodelb(&io, b, n, size[p]); /* consume bits */
				*o = (Vcchar_t)node[p]; /* decode the byte */
				if((o += 1) >= endo)
					break;
				sz = ntop; p = 0; /* restart at trie top */
			}
			else if(size[p] == 0) /* corrupted data */
				return -1;
			else 
			{	vciodelb(&io, b, n, sz); /* consume bits */
				sz = -size[p]; p = node[p]; /* trie recursion */
			}
		}
		vcioendb(&io, b, n, VC_DECODE);
	}

	if(data != orig)
		vcbuffer(vc, data, -1, -1);

	if(out)
		*out = output;
	return o-output;
}

#if __STD_C
static int huffevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int huffevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Vchuff_t	*huff;

	if(type == VC_OPENING)
	{	if(!(huff = (Vchuff_t*)malloc(sizeof(Vchuff_t))) )
			return -1;
		huff->trie = NIL(Vchtrie_t*);
		huff->maxs = 0;
		huff->type = 0;
		vcsetmtdata(vc, huff);
		return 0;
	}
	else if(type == VC_CLOSING)
	{	if((huff = vcgetmtdata(vc, Vchuff_t*)) )
		{	if(huff->trie)
				vchdeltrie(huff->trie);
			free(huff);
		}
		vcsetmtdata(vc, NIL(Vchuff_t*));
		return 0;
	}
	else	return 0;
}

Vcmethod_t _Vchuffman =
{	vchuff,
	vcunhuff,
	huffevent,
	"huffman", "Huffman encoding.",
	"[-version?huffman (AT&T Research) 2003-01-01]" USAGE_LICENSE,
	NIL(Vcmtarg_t*),
	1024*1024,
	0
};

VCLIB(Vchuffman)
