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

/*	Read/write the code lengths of a Huffman code.
**
**	Written by Kiem-Phong Vo
*/

#if __STD_C
static ssize_t rlcode(ssize_t nsym, ssize_t* clen, ssize_t cmax, Vcchar_t* rle, ssize_t rlsz, int encode)
#else
static ssize_t rlcode(nsym, clen, cmax, rle, rlsz, encode)
ssize_t		nsym;	/* alphabet size		*/
ssize_t*	clen;	/* code lengths to be encoded	*/
ssize_t		cmax;	/* max size of any code		*/
Vcchar_t*	rle;	/* buffer to store rle sequence	*/
ssize_t		rlsz;
int		encode;
#endif
{
	ssize_t		k, n, r, d, esc1, esc2;
	Vcchar_t	*rl = rle, *endr = rle+rlsz;
	Vcio_t		io;

	esc1 = cmax+1; esc2 = cmax+2;
	if(encode)
	{	for(k = 0; k < nsym; k = n)
		{	d = clen[k];
			for(n = k+1; n < nsym; ++n)
				if(clen[n] != d)
					break;
			if((r = n-k) >= 3)
			{	vcioinit(&io, rl, 8*sizeof(ssize_t));
				vcioput2(&io, r-3, esc1, esc2);
				rl = vcionext(&io);
				*rl++ = d;
			}
			else for(r = k; r < n; ++r)
				*rl++ = clen[r];
		}
	}
	else
	{	for(k = 0; k < nsym && rl < endr; )
		{	if((d = *rl++) == esc1 || d == esc2)
			{	vcioinit(&io, rl-1, (endr-rl)+1);
				r = vcioget2(&io, esc1, esc2) + 3;
				rl = vcionext(&io);
				for(d = *rl++; k < nsym && r > 0; --r)
					clen[k++] = d;
				if(r > 0)
					return -1;
			}
			else if(d <= cmax)
				clen[k++] = d;
			else	return -1;
		}

		if(k != nsym || rl != endr)
			return -1;
	}

	return rl-rle;
}

#if __STD_C
ssize_t vchputcode(ssize_t nsym, ssize_t* clen, ssize_t maxs, Vcchar_t* data, size_t dtsz)
#else
ssize_t vchputcode(nsym, clen, maxs, data, dtsz)
ssize_t		nsym;	/* alphabet size or #symbols	*/
ssize_t*	clen;	/* code lengths to be output	*/
ssize_t		maxs;	/* max length of any code	*/
Vcchar_t*	data;	/* data buffer for output	*/
size_t		dtsz;	/* buffer size (need >= 256)	*/ 
#endif
{
	reg ssize_t	n, k, nl, cs;
	Vcchar_t	*len;
	Vcbit_t		b;
	Vcio_t		io;

	nl = 2*nsym;
	if(!(len = (Vcchar_t*)malloc(nl*sizeof(Vcchar_t*))) )
		return -1;

	/* # of bits used to output the code table for this coding */
	cs = maxs+2;
	cs = cs < 2 ? 1 : cs < 4 ? 2 : cs < 8 ? 3 : cs < 16 ? 4 : cs < 32 ? 5 : 6;

	/* run-length-encode the code table in the alphabet [0...maxs+2] */
	if((nl = rlcode(nsym, clen, maxs, len, nl, 1)) < 0 )
	{	free(len);
		return -1;
	}

	vcioinit(&io,data,dtsz);
	vcioputu(&io, nl); /**/DEBUG_PRINT(2,"Runlength=%d\n",nl);

	vciosetb(&io, b, n, VC_ENCODE);
	for(k = 0; k < nl; ++k)
	{	if((n + cs) > VC_BITSIZE)
			vcioflsb(&io, b, n);
		vcioaddb(&io, b, n, (((Vcbit_t)len[k]) << (VC_BITSIZE-cs)), cs);
	}
	vcioendb(&io, b, n, VC_ENCODE);

	free(len);
	return vciosize(&io);
}

#if __STD_C
ssize_t vchgetcode(ssize_t nsym, ssize_t* clen, ssize_t maxs, Vcchar_t* data, size_t dtsz)
#else
ssize_t vchgetcode(nsym, clen, maxs, data, dtsz)
ssize_t		nsym;	/* alphabet size or #symbols		*/
ssize_t*	clen;	/* code lengths to be reconstructed	*/
ssize_t		maxs;	/* max length of any code		*/
Vcchar_t*	data;	/* data that encodes the code table	*/
size_t		dtsz;	/* size of above data buffer		*/
#endif
{
	ssize_t		i, n, k, nl, cs;
	Vcchar_t	*len;
	Vcbit_t		b;
	Vcio_t		io;

	if(!(len = (Vcchar_t*)malloc(nsym*sizeof(Vcchar_t*))) )
		return -1;

	/* # of bits used to output the code table for this coding */
	cs = maxs+2;
	cs = cs < 2 ? 1 : cs < 4 ? 2 : cs < 8 ? 3 : cs < 16 ? 4 : cs < 32 ? 5 : 6;

	/* The length of the rle sequence should have been coded with vcioputu()
	** so it could be decoded by simply using vciogetu(). However, an older
	** and buggy version used vcioputc() and could not handle values >= 256.
	** The below loop tries to detect and handle those old cases.
	*/
	for(i = 0; i < 2; i += 1)
	{	vcioinit(&io, data, dtsz);
		if(i == 0) /* try the current coding first */
		{	if((nl = vciogetu(&io)) > nsym || nl < 0 /* nl is a signed int */)
				continue; /* restart to do the below case */
		}
		else /* must be the old buggy coding noted above */
		{	if((nl = vciogetc(&io)) == 0)
				nl = 256; /* fix a simple byte wrap-around value */
		}

		vciosetb(&io, b, n, VC_DECODE);
		for(k = 0; k < nl; ++k) /* read the rle sequence */
		{	vciofilb(&io, b, n, cs);
			len[k] = b >> (VC_BITSIZE-cs);
			vciodelb(&io, b, n, cs);
		}
		vcioendb(&io, b, n, VC_DECODE);

		/* now see if clen[] can be reconstructed */
		if(rlcode(nsym, clen, maxs, len, nl, 0) >= 0)
		{	free(len);
			return vciosize(&io);
		}
	}

	free(len);
	return -1;
}
