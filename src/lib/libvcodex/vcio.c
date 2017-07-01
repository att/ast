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
#include	"vchuff.h"

/*	Functions to read/write integers in various portable formats.
**	Some of these are stolen from the Sfio library and modified 
**	to deal only with memory buffers.
**
**	Written by Kiem-Phong Vo
*/

/* base-128 unsigned coding */
#if __STD_C
ssize_t _vcioputu(Vcio_t* io, Vcint_t v)
#else
ssize_t _vcioputu(io, v)
Vcio_t*	io;
Vcint_t	v;
#endif
{
	Vcchar_t	*ptr, *code;
	ssize_t		n;
	Vcchar_t	data[2*sizeof(Vcint_t)];

	code = &data[sizeof(data)-1];
	*code = v&127;
	while((v >>= 7) > 0)
		*--code = (v&127) | 128;
	n = &data[sizeof(data)] - code;

	ptr = io->next;
	switch(n)
	{ default:	memcpy(ptr, code, n); ptr += n; break;
	  case 7 :	*ptr++ = *code++;
	  case 6 :	*ptr++ = *code++;
	  case 5 :	*ptr++ = *code++;
	  case 4 :	*ptr++ = *code++;
	  case 3 :	*ptr++ = *code++;
	  case 2 :	*ptr++ = *code++;
	  case 1 :	*ptr++ = *code++;
	}
	io->next = ptr;

	return n;
}

#if __STD_C
Vcint_t _vciogetu(Vcio_t* io)
#else
Vcint_t _vciogetu(io)
Vcio_t*	io;
#endif
{
	int		n;
	Vcint_t		v;
	Vcchar_t	*ptr;

	ptr = io->next;
	v = (n = *ptr++)&127;
	while(n & 128)
		v = (v << 7) | ((n = *ptr++)&127);
	io->next = ptr;

	return v;
}

/* base 256 coding */
#if __STD_C
ssize_t _vcioputm(Vcio_t* io, Vcint_t v, Vcint_t max)
#else
ssize_t _vcioputm(io, v, max)
Vcio_t*	io;
Vcint_t	v;
Vcint_t	max;
#endif
{
	ssize_t		n;
	Vcchar_t	data[sizeof(Vcint_t)];
	Vcchar_t	*code = &data[sizeof(data) - 1];
	Vcchar_t	*ptr;

	*code = v&255;
	while((max >>= 8) > 0)
		*--code = (v >>= 8)&255;
	n = &data[sizeof(data)] - code;

	if(io)
	{	ptr = io->next;
		switch(n)
		{ default:	memcpy(ptr, code, n); ptr += n; break;
		  case 7 :	*ptr++ = *code++;
		  case 6 :	*ptr++ = *code++;
		  case 5 :	*ptr++ = *code++;
		  case 4 :	*ptr++ = *code++;
		  case 3 :	*ptr++ = *code++;
		  case 2 :	*ptr++ = *code++;
		  case 1 :	*ptr++ = *code++;
		}
		io->next = ptr;
	}

	return n;
}

#if __STD_C
Vcint_t _vciogetm(Vcio_t* io, Vcint_t max)
#else
Vcint_t _vciogetm(io, max)
Vcio_t*	io;
Vcint_t	max;
#endif
{
	Vcint_t		v;
	Vcchar_t	*ptr;

	ptr = io->next;
	v = *ptr++;
	while((max >>= 8) > 0)
		v = (v << 8) | *ptr++;
	io->next = ptr;

	return v;
}

/* A value v can be coded using two letters A and Z by treating v-1
** as the rank of a string of A's and Z's listed in lexicographic order.
** This coding is useful for coding runs of a letter, say A, using just
** another companion letter. Below are the codes of the first fourteen
** (2+4+8) integers using A and Z:
**
**	0	A		2	AA		6	AAA
**	1	Z		3	ZA		7	ZAA
**				4	AZ		8	AZA
**				5	ZZ		9	ZZA
**							10	AAZ
**							11	ZAZ
**							12	AZZ
**							13	ZZZ
*/

#if __STD_C
ssize_t _vcioput2(Vcio_t* io, Vcint_t v, Vcchar_t a, Vcchar_t z)
#else
ssize_t _vcioput2(io, v, a, z)
Vcio_t*		io;
Vcint_t		v;	/* value to encode	*/
Vcchar_t	a;	/* 1st coding letter	*/
Vcchar_t	z;	/* 2nd coding letter	*/
#endif
{
	Vcchar_t	*ptr = io->next;
	ssize_t		n;

	for(;;)
	{	*ptr++ = (v&1) == 0 ? a : z;
		if((v -= 2) < 0)
			break;
		else	v >>= 1;
	}
	n = ptr - io->next;
	io->next = ptr;

	return n;
}

#if __STD_C
Vcint_t _vcioget2(Vcio_t* io, Vcchar_t a, Vcchar_t z)
#else
Vcint_t _vcioget2(io, a, z)
Vcio_t*		io;
Vcchar_t	a;	/* 1st coding letter	*/
Vcchar_t	z;	/* 2nd coding letter	*/
#endif
{
	int		d;
	Vcint_t		v;
	Vcchar_t	*ptr, *endp;

	v = -1; d = 1;
	for(ptr = io->next, endp = io->endb; ptr < endp; ++ptr)
	{	if(*ptr == a)
		{	v += d;
			d <<= 1;
		}
		else if(*ptr == z)
		{	d <<= 1;
			v += d;
		}
		else	break;
	}

	io->next = ptr;

	return v;
}


/* Elias Gamma code for POSITIVE integers
** Gamma code	Value	Base-2 bits
** 1		1	1
** 00 1		2	10
** 01 1		3	11
** 00 00 1	4	100
** 01 00 1	5	101
** ...
*/
static Vcbit_t Gfour[16] = /* table of gamma codes mapping 4 bits at a time */
{	/* 0000 -> 00 00 00 00	*/	0x00000000,
	/* 0001 -> 01 00 00 00	*/	0x40000000,
	/* 0010 -> 00 01 00 00	*/	0x10000000,
	/* 0011 -> 01 01 00 00	*/	0x50000000,
	/* 0100 -> 00 00 01 00	*/	0x04000000,
	/* 0101 -> 01 00 01 00	*/	0x44000000,
	/* 0110 -> 00 01 01 00	*/	0x14000000,
	/* 0111 -> 01 01 01 00	*/	0x54000000,
	/* 1000 -> 00 00 00 01	*/	0x01000000,
	/* 1001 -> 01 00 00 01	*/	0x41000000,
	/* 1010 -> 00 01 00 01	*/	0x11000000,
	/* 1011 -> 01 01 00 01	*/	0x51000000,
	/* 1100 -> 00 00 01 01	*/	0x05000000,
	/* 1101 -> 01 00 01 01	*/	0x45000000,
	/* 1110 -> 00 01 01 01	*/	0x15000000,
	/* 1111 -> 01 01 01 01	*/	0x55000000
};
static Vcbit_t Glast[16] = /* table of gamma codes for last <=4 bits */
{	/* 0    -> 0		*/	0x00000000,
	/* 1    -> 1    	*/	0x80000000,
	/* 10   -> 00 1 	*/	0x20000000,
	/* 11   -> 01 1  	*/	0x60000000,
	/* 100  -> 00 00 1	*/	0x08000000,
	/* 101  -> 01 00 1	*/	0x48000000,
	/* 110  -> 00 01 1	*/	0x18000000,
	/* 111  -> 01 01 1	*/	0x58000000,
	/* 1000 -> 00 00 00 1	*/	0x02000000,
	/* 1001 -> 01 00 00 1	*/	0x42000000,
	/* 1010 -> 00 01 00 1	*/	0x12000000,
	/* 1011 -> 01 01 00 1	*/	0x52000000,
	/* 1100 -> 00 00 01 1	*/	0x06000000,
	/* 1101 -> 01 00 01 1	*/	0x46000000,
	/* 1110 -> 00 01 01 1	*/	0x16000000,
	/* 1111 -> 01 01 01 1	*/	0x56000000
};

#if __STD_C
ssize_t _vcioputg(Vcio_t* io, Vcint_t v)
#else
ssize_t _vcioputg(io, v)
Vcio_t*		io;
Vcint_t		v;
#endif
{
	ssize_t	n;

	for(n = 0; v > 0xf; v >>= 4, n += 8)
		vcioaddb(io, io->bits, io->nbits, Gfour[v&0xf], 8);

	if(v <= 0x3)
	{	if(v <= 0x1)
		{	vcioaddb(io, io->bits, io->nbits, Glast[v], 1);
			return n+1;
		}
		else
		{	vcioaddb(io, io->bits, io->nbits, Glast[v], 3);
			return n+3;
		}
	}
	else
	{	if(v <= 0x7)
		{	vcioaddb(io, io->bits, io->nbits, Glast[v], 5);
			return n+5;
		}
		else
		{	vcioaddb(io, io->bits, io->nbits, Glast[v], 7);
			return n+7;
		}
	}
}

#if __STD_C
Vcint_t _vciogetg(Vcio_t* io)
#else
Vcint_t _vciogetg(io)
Vcio_t*		io;
#endif
{
	Vcint_t		v;
	int		k, b, g, s;
	static int	Ifour[256], Ilast[256];

	/* Bit strings of the below forms are terminal and map to # of sig bits.
	**	1xxx xxxx 	|-1| bits.
	**	xx1x xxxx	|-3| bits.
	**	xxxx 1xxx	|-5| bits.
	**	xxxx xx1x	|-7| bits.
	** Otherwise, they would be of the below form and inversely map Gfour[].
	**	0x0x 0x0x
	*/
	if(Ifour[255] == 0) /* initialize the inversion arrays */
	{	for(k = 0; k < 256; ++k)
		{	for(b = 7; b >= 1; b -= 2) /* find the high odd bit */
				if(k & (1<<b) )
					break;
			if(b >= 1)
			{	Ifour[k] = b-8; /* set # of bits needed */

				if((k & ~((1<<b)-1)) == k ) /* set value in Ilast[] */
				{	for(g = 0; g < 16; ++g)
						if((Glast[g]>>(VC_BITSIZE-8)) == k)
							break;
					for(s = (1<<b)-1; s >= 0; --s)
						Ilast[k|s] = g;
				}
			}
		}
		for(k = 0; k < 16; ++k) /* inverse of Gfour[] */
			Ifour[Gfour[k] >> (VC_BITSIZE-8)] = k;
	}

	for(v = 0, s = 0;; s += 4)
	{	vciofilb(io, io->bits, io->nbits, 8);
		if(io->nbits == 0)
			return -1;

		if((k = io->nbits) >= 8)
			b = (int)(io->bits >> (VC_BITSIZE-8));
		else	b = (int)((io->bits >> (VC_BITSIZE-k)) << (8-k));

		if((g = Ifour[b]) >= 0)
		{	if(io->nbits < 8)
				return -1;
			k = 8;
		}
		else
		{	k = -g;
			g = Ilast[b];
		}

		v |= ((Vcint_t)g) << s;
		vciodelb(io, io->bits, io->nbits, k);
		if(k < 8)
			break;
	}

	return v;
}


/* use binary search to get # of significant bits in a given integer */
static ssize_t _Nbits4[16] =
{	1,	/*  0: 0000	*/
	1,	/*  1: 0001	*/
	2,	/*  2: 0010	*/
	2,	/*  3: 0011	*/
	3,	/*  4: 0100	*/
	3,	/*  5: 0101	*/
	3,	/*  6: 0110	*/
	3,	/*  7: 0111	*/
	4,	/*  8: 1000	*/
	4,	/*  9: 1001	*/
	4,	/* 10: 1010	*/
	4,	/* 11: 1011	*/
	4,	/* 12: 1100	*/
	4,	/* 13: 1101	*/
	4,	/* 14: 1110	*/
	4	/* 15: 1111	*/
};
#define NBITS4(v)	_Nbits4[v]
#define NBITS8(v)	((v) > 0xf    ? (((v) >>=  4), (NBITS4(v)+4))   : NBITS4(v) )
#define NBITS16(v)	((v) > 0xff   ? (((v) >>=  8), (NBITS8(v)+8))   : NBITS8(v) )
#define NBITS(v)	((v) > 0xffff ? (((v) >>= 16), (NBITS16(v)+16)) : NBITS16(v) )

/* Coding a list of non-negative integers using a variable-length bit coding.
** Each integer is coded by a prefix telling the # of significant bits followed
** by the significant bits themselves. The #'s of significant bits are coded
** using a Huffman code.
*/
#if __STD_C
ssize_t vcioputlist(Vcio_t* io, Vcint_t* list, ssize_t nlist)
#else
ssize_t vcioputlist(io, list, nlist)
Vcio_t*		io;
Vcint_t*	list;
ssize_t		nlist;
#endif
{
	reg Vcbit_t	b, v, e;
	reg ssize_t	n, s, i;
	ssize_t		freq[VC_INTSIZE], size[VC_INTSIZE];
	Vcbit_t		bits[VC_INTSIZE];
	Vcchar_t	*begs;
	int		run;

	/* compute the frequencies of the sizes of significant bits */
	for(i = 0; i < VC_INTSIZE; ++i)
		freq[i] = 0;
	for(i = 0; i < nlist; ++i)
	{	v = list[i];
		freq[NBITS(v)-1] += 1;
	}

	/* compute the Huffman code for these #'s of significant bits */
	if((s = vchsize(VC_INTSIZE, freq, size, &run)) < 0 ||
	   (s > 0 && vchbits(VC_INTSIZE, size, bits) < 0) )
		return -1;

	begs = vcionext(io);
	if(s == 0) /* all integers have the same size */
	{	s = run+1;
		vcioputc(io, s);

		vciosetb(io, b, n, VC_ENCODE);
		for(i = 0; i < nlist; ++i)
		{	v = ((Vcint_t)list[i]) << (VC_INTSIZE-s);
			vcioaddb(io, b, n, v, s);
		}
		vcioendb(io, b, n, VC_ENCODE);
	}
	else
	{	vcioputc(io, s|(1<<7)); /* the max size of any integer */	
		if((s = vchputcode(VC_INTSIZE, size, s, vcionext(io), vciomore(io))) < 0)
			return -1;
		else	vcioskip(io, s);

		vciosetb(io, b, n, VC_ENCODE);
		for(i = 0; i < nlist; ++i) 
		{	v = (Vcint_t)list[i]; s = NBITS(v)-1;
			vcioaddb(io, b, n, bits[s], size[s]);

			for(v = (Vcint_t)list[i], s += 1;; )
			{	if(s > 8)
				{	e = (v&0xff) << (VC_INTSIZE - 8);
					vcioaddb(io, b, n, e, 8);
					v >>= 8; s -= 8;
				}
				else
				{	e = v << (VC_INTSIZE - s);
					vcioaddb(io, b, n, e, s);
					break;
				}
			}
		}
		vcioendb(io, b, n, VC_ENCODE);
	}

	return vcionext(io)-begs;
}

#if __STD_C
ssize_t vciogetlist(Vcio_t* io, Vcint_t* list, ssize_t nlist)
#else
ssize_t vciogetlist(io, list, nlist)
Vcio_t*		io;
Vcint_t*	list;
ssize_t		nlist;
#endif
{
	reg Vcbit_t	b;
	reg ssize_t	n, s, p, ntop, nl, d;
	ssize_t		cdsz[VC_INTSIZE];
	Vcint_t		v;
	Vcbit_t		bits[VC_INTSIZE];
	Vchtrie_t	*trie;
	short		*node, *size;

	if((s = vciogetc(io)) < 0)
		return -1;

	vciosetb(io, b, n, VC_DECODE); /* start bit stream */

	if(!(s & (1<<7)) ) /* all integers have the same size */
	{	for(nl = 0; nl < nlist; ++nl)
		{	vciofilb(io, b, n, s);
			list[nl] = (Vcint_t)(b >> (VC_BITSIZE-s));
			vciodelb(io, b, n, s);
		}
	}
	else
	{	s &= ~(1<<7);
		if((s = vchgetcode(VC_INTSIZE, cdsz, s, vcionext(io), vciomore(io))) < 0 )
			return -1;
		else	vcioskip(io, s);
		if(vchbits(VC_INTSIZE, cdsz, bits) < 0)
			return -1;
		if(!(trie = vchbldtrie(VC_INTSIZE, cdsz, bits)) )
			return -1;
		node = trie->node;
		size = trie->size;
		ntop = trie->ntop;
		vciosetb(io, b, n, VC_DECODE);
		for(s = ntop, p = 0, nl = 0;; )
		{	vciofilb(io, b, n, s);

			p += (b >> (VC_BITSIZE-s)); /* slot to look into */
			if(size[p] > 0) /* length is found */
			{	s = (int)node[p] + 1; /* get the actual length */
				vciodelb(io, b, n, size[p]); /* consume bits */

				for(v = 0, d = 0;; )
				{	if(s > 8)
					{	vciofilb(io, b, n, 8);
						v |= (b >> (VC_BITSIZE-8)) << d;
						vciodelb(io, b, n, 8);
						d += 8; s -= 8;
					}
					else
					{	vciofilb(io, b, n, s);
						v |= (b >> (VC_BITSIZE-s)) << d;
						vciodelb(io, b, n, s);
						break;
					}
				}

				list[nl] = v;
				if((nl += 1) >= nlist)
					break;

				s = ntop; p = 0; /* restart at trie top for next integer */
			}
			else if(size[p] == 0) /* corrupted data */
				return -1;
			else 
			{	vciodelb(&io, b, n, s); /* consume bits */
				s = -size[p]; p = node[p]; /* trie recursion */
			}
		}
	}

	vcioendb(io, b, n, VC_DECODE); /* finish bit stream */

	return nl;
}

/* Transform a list of integers into non-negative integers.
** 1. Keep a sign indicator and if the current element is of this sign,
**    code its absolute value; otherwise, code it as the negative value
**    with the same magnitude. This turns a run into all positives except
**    for the head of the run.
** 2. Then code all integers using only non-negative integers via
**    a proportional method. Suppose that we want n negatives for each p
**    positives, the codings for a negative x and a positive y would be:
**	x -> ((-x-1)/n)*(n+p) + (-x-1)%n + 1 + p
**	y -> ( (y-1)/p)*(n+p) + ( y-1)%p + 1
*/
ssize_t vcpositive(Vcint_t* list, ssize_t nlist, ssize_t* pos, ssize_t* neg, int type)
{
	ssize_t		k, p, n, s, g;
	Vcint_t		v;

	if(type == VC_ENCODE) /* encoding */
	{	
		/* transform runs to positives */
		for(type = 1, k = 0; k < nlist; ++k)
		{	if((v = list[k]) < 0)
			{	if(type > 0)
					type = -1;
				else	list[k] = -v;
			}
			else if(v > 0)
			{	if(type < 0)
				{	type = 1;	
					list[k] = -v;
				}
			}
		}

		/* count positives/negatives to decide proportion */
		for(n = p = 0, k = 0; k < nlist; ++k)
		{	if((v = list[k]) > 0)
				p += 1;
			else if(v < 0)
				n += 1;
		}

		if(n == 0) /* a sequence of non-negatives */
			p = 1;
		else if(p == 0) /* all negatives */
		{	for(k = 0; k < nlist; ++k)
				list[k] = -list[k];
			n = 1;
		}
		else
		{	/* reduce proportions */
			while(p >= 128 && n >= 128)
				{ p /= 2; n /= 2; }
			for(k = 127; k > 1; --k)
				if((p%k) == 0 && (n%k) == 0)
					{ p /= k; n /= k; }

			/* now do the coding */
			for(s = n+p, k = 0; k < nlist; ++k)
			{	if((v = list[k]) == 0 )
					continue;
				else if(v > 0)
					list[k] = (( v-1)/p)*s + ( v-1)%p + 1;
				else	list[k] = ((-v-1)/n)*s + (-v-1)%n + 1 + p;
			}
		}

		*pos = p; *neg = n;
	}
	else
	{	p = *pos; n = *neg;

		if(p == 0 && n > 0) /* all negatives */
		{	for(k = 0; k < nlist; ++k)
				list[k] = -list[k];
		}
		else if(p > 0 && n > 0) /* nontrivial coding */
		{	for(s = n+p, k = 0; k < nlist; ++k)
			{	if((v = list[k]) == 0)
					continue;
				if((g = (v-1)%s) < p)
					list[k] =  ( ((v-1)/s)*p + g + 1 );
				else	list[k] = -( ((v-1)/s)*n + g + 1 - p );
			}
		}

		/* undo the sign switching */
		for(type = 1, k = 0; k < nlist; ++k)
		{	v = list[k];
			if(type < 0)
				list[k] = -v;
			if(v < 0)
				type = -type;
		}
	}

	return nlist;
}


/* Addresses of successive matches in the Lempel-Ziv parser tend to be close
** to one another. So it is good to code an integer given another in such a
** way that the coded value is small.
*/
#if __STD_C
Vcint_t vcintcode(Vcint_t v, Vcint_t near, Vcint_t min, Vcint_t max, int type)
#else
Vcint_t vcintcode(v, near, min, max, type)
Vcint_t		v;	/* value to be de/coded	*/
Vcint_t		near;	/* like to be near this	*/
Vcint_t		min;	/* range is [min,max)	*/
Vcint_t		max;	/* excluded max value	*/
int		type;	/* VC_ENCODE/VC_DECODE	*/
#endif
{
	Vcint_t	a, n;

	if(min >= max || near < min || near >= max)
		return -1; /* an error in specification */

	if(type == VC_ENCODE)
	{	if(v < min || v >= max)
			return -1; /* out of range */

		a = (v -= near) < 0 ? -v : v;
		near -= min; max -= min;
		n = (n = max - near - 1) < near ? n : near;
		if(a <= n)
			return (a<<1) - (v <= 0 ? 0 : 1); 
		else	return a + n;
	}
	else if(type == VC_DECODE)
	{	near -= min; max -= min;
		if(v < 0 || v >= max)
			return -1; /* bad coded value */

		n = (n = max - near - 1) < near ? n : near;
		if(v <= (n<<1))
			a = near + ((v&1) ? ((v+1)>>1) : -(v>>1) );
		else	a = n == near ? v : near - (v - n);
		return a + min;
	}
	else	return -1;
}


/* print/de-print an integer */
#if __STD_C
ssize_t vcitoa(Vcint_t i, char* a, ssize_t z)
#else
ssize_t vcitoa(i, a, z)
Vcint_t		i;
char*		a;
ssize_t		z;
#endif
{
	int	k;
	char	buf[sizeof(Vcint_t)*4];

	for(k = sizeof(buf)-1; k >= 0; --k)
	{	buf[k] = '0' + (i%10);
		if((i /= 10) == 0 )
			break;
	}
	if((i = sizeof(buf) - k) >= z)
		return -1;
	else
	{	memcpy(a, buf+k, i);
		a[i] = 0;
		return i;
	}
}

#if __STD_C
Vcint_t vcatoi(char* a)
#else
Vcint_t vcatoi(a)
char*		a;
#endif
{
	Vcint_t	i;

	for(i = 0; *a && isdigit(*a); ++a)
		i = i*10 + (*a - '0');
	return i;
}


/* transform from a native string to its ASCII version for portability */
#if __STD_C
char* vcstrcode(char* s, char* a, ssize_t z)
#else
char* vcstrcode(s, a, z)
char*	s;	/* string to be made portable	*/
char*	a;	/* space to store asciized data	*/
ssize_t z;	/* size of 'a'			*/
#endif
{
	ssize_t		c;
	static int	type = 0; /* unknown: 0, native != ASCII: -1, else: 1 */
	static Vcchar_t	_ascii[256];

	if(type == 0) /* construct the map from native to ascii */
	{	for(c = 0; c < 256; ++c)
			_ascii[c] = (char)c;
		_ascii[(Vcchar_t)'\a'] = '\007';
		_ascii[(Vcchar_t)'\b'] = '\010';
		_ascii[(Vcchar_t)'\t'] = '\011';
		_ascii[(Vcchar_t)'\n'] = '\012';
		_ascii[(Vcchar_t)'\v'] = '\013';
		_ascii[(Vcchar_t)'\f'] = '\014';
		_ascii[(Vcchar_t)'\r'] = '\015';
		_ascii[(Vcchar_t)' ' ] = '\040';
		_ascii[(Vcchar_t)'!' ] = '\041';
		_ascii[(Vcchar_t)'\"'] = '\042';
		_ascii[(Vcchar_t)'#' ] = '\043';
		_ascii[(Vcchar_t)'$' ] = '\044';
		_ascii[(Vcchar_t)'%' ] = '\045';
		_ascii[(Vcchar_t)'&' ] = '\046';
		_ascii[(Vcchar_t)'\''] = '\047';
		_ascii[(Vcchar_t)'(' ] = '\050';
		_ascii[(Vcchar_t)')' ] = '\051';
		_ascii[(Vcchar_t)'*' ] = '\052';
		_ascii[(Vcchar_t)'+' ] = '\053';
		_ascii[(Vcchar_t)',' ] = '\054';
		_ascii[(Vcchar_t)'-' ] = '\055';
		_ascii[(Vcchar_t)'.' ] = '\056';
		_ascii[(Vcchar_t)'/' ] = '\057';
		_ascii[(Vcchar_t)'0' ] = '\060';
		_ascii[(Vcchar_t)'1' ] = '\061';
		_ascii[(Vcchar_t)'2' ] = '\062';
		_ascii[(Vcchar_t)'3' ] = '\063';
		_ascii[(Vcchar_t)'4' ] = '\064';
		_ascii[(Vcchar_t)'5' ] = '\065';
		_ascii[(Vcchar_t)'6' ] = '\066';
		_ascii[(Vcchar_t)'7' ] = '\067';
		_ascii[(Vcchar_t)'8' ] = '\070';
		_ascii[(Vcchar_t)'9' ] = '\071';
		_ascii[(Vcchar_t)':' ] = '\072';
		_ascii[(Vcchar_t)';' ] = '\073';
		_ascii[(Vcchar_t)'<' ] = '\074';
		_ascii[(Vcchar_t)'=' ] = '\075';
		_ascii[(Vcchar_t)'>' ] = '\076';
		_ascii[(Vcchar_t)'\?'] = '\077';
		_ascii[(Vcchar_t)'@' ] = '\100';
		_ascii[(Vcchar_t)'A' ] = '\101';
		_ascii[(Vcchar_t)'B' ] = '\102';
		_ascii[(Vcchar_t)'C' ] = '\103';
		_ascii[(Vcchar_t)'D' ] = '\104';
		_ascii[(Vcchar_t)'E' ] = '\105';
		_ascii[(Vcchar_t)'F' ] = '\106';
		_ascii[(Vcchar_t)'G' ] = '\107';
		_ascii[(Vcchar_t)'H' ] = '\110';
		_ascii[(Vcchar_t)'I' ] = '\111';
		_ascii[(Vcchar_t)'J' ] = '\112';
		_ascii[(Vcchar_t)'K' ] = '\113';
		_ascii[(Vcchar_t)'L' ] = '\114';
		_ascii[(Vcchar_t)'M' ] = '\115';
		_ascii[(Vcchar_t)'N' ] = '\116';
		_ascii[(Vcchar_t)'O' ] = '\117';
		_ascii[(Vcchar_t)'P' ] = '\120';
		_ascii[(Vcchar_t)'Q' ] = '\121';
		_ascii[(Vcchar_t)'R' ] = '\122';
		_ascii[(Vcchar_t)'S' ] = '\123';
		_ascii[(Vcchar_t)'T' ] = '\124';
		_ascii[(Vcchar_t)'U' ] = '\125';
		_ascii[(Vcchar_t)'V' ] = '\126';
		_ascii[(Vcchar_t)'W' ] = '\127';
		_ascii[(Vcchar_t)'X' ] = '\130';
		_ascii[(Vcchar_t)'Y' ] = '\131';
		_ascii[(Vcchar_t)'Z' ] = '\132';
		_ascii[(Vcchar_t)'[' ] = '\133';
		_ascii[(Vcchar_t)'\\'] = '\134';
		_ascii[(Vcchar_t)']' ] = '\135';
		_ascii[(Vcchar_t)'^' ] = '\136';
		_ascii[(Vcchar_t)'_' ] = '\137';
		_ascii[(Vcchar_t)'`' ] = '\140';
		_ascii[(Vcchar_t)'a' ] = '\141';
		_ascii[(Vcchar_t)'b' ] = '\142';
		_ascii[(Vcchar_t)'c' ] = '\143';
		_ascii[(Vcchar_t)'d' ] = '\144';
		_ascii[(Vcchar_t)'e' ] = '\145';
		_ascii[(Vcchar_t)'f' ] = '\146';
		_ascii[(Vcchar_t)'g' ] = '\147';
		_ascii[(Vcchar_t)'h' ] = '\150';
		_ascii[(Vcchar_t)'i' ] = '\151';
		_ascii[(Vcchar_t)'j' ] = '\152';
		_ascii[(Vcchar_t)'k' ] = '\153';
		_ascii[(Vcchar_t)'l' ] = '\154';
		_ascii[(Vcchar_t)'m' ] = '\155';
		_ascii[(Vcchar_t)'n' ] = '\156';
		_ascii[(Vcchar_t)'o' ] = '\157';
		_ascii[(Vcchar_t)'p' ] = '\160';
		_ascii[(Vcchar_t)'q' ] = '\161';
		_ascii[(Vcchar_t)'r' ] = '\162';
		_ascii[(Vcchar_t)'s' ] = '\163';
		_ascii[(Vcchar_t)'t' ] = '\164';
		_ascii[(Vcchar_t)'u' ] = '\165';
		_ascii[(Vcchar_t)'v' ] = '\166';
		_ascii[(Vcchar_t)'w' ] = '\167';
		_ascii[(Vcchar_t)'x' ] = '\170';
		_ascii[(Vcchar_t)'y' ] = '\171';
		_ascii[(Vcchar_t)'z' ] = '\172';
		_ascii[(Vcchar_t)'{' ] = '\173';
		_ascii[(Vcchar_t)'|' ] = '\174';
		_ascii[(Vcchar_t)'}' ] = '\175';
		_ascii[(Vcchar_t)'~' ] = '\176';

		for(c = 0; c < 256; ++c)
			if(_ascii[c] != c)
				break;
		type = c < 256 ? -1 : 1;
	}

	for(c = 0; c < z; ++c)
		if((a[c] = (char)_ascii[s[c]]) == 0 )
			break;
	return c < z ? a : NIL(char*);
}

/* decode a list of integers from a null-terminated string */
#if __STD_C
ssize_t vcstr2list(char* str, int comma, ssize_t** listp)
#else
ssize_t vcstr2list(str, comma, listp)
char*		str;	/* string to be parsed	*/
int		comma;	/* value separator	*/
ssize_t**	listp;	/* to return list	*/
#endif
{
	ssize_t	n, k, *list;

	for(n = 0, k = 0;; )
	{	while(str[k] == ' ' || str[k] == '\t' || str[k] == comma)
			k += 1;
		if(!isdigit(str[k]) )
			break;

		while(isdigit(str[k]) )
			k += 1;
		n += 1; /* a well-defined value */
	}

	if(n == 0)
		return n;

	if(!(list = (ssize_t*)malloc(n*sizeof(ssize_t))) )
		return -1;

	for(n = 0, k = 0;; )
	{	while(str[k] == ' ' || str[k] == '\t' || str[k] == comma)
			k += 1;
		if(!isdigit(str[k]) )
			break;

		list[n] = 0;
		while(isdigit(str[k]) )
		{	list[n] = list[n]*10 + (str[k] - '0');
			k += 1;
		}
		n += 1; /* a well-defined value */
	}

	*listp = list;
	return n;
}


/* transform data from/to bit-representation to/from hex-representation */
ssize_t vchexcode(Vcchar_t* byte, ssize_t bytez, Vcchar_t* hex, ssize_t hexz, int type)
{
	int		b, h, l, r;
	Vcchar_t	*dig;
	static Vcchar_t	Upper[16], Lower[16], Rev[256], Didinit = 0;

	if(!Didinit) /* initialize conversion tables */
	{	memcpy(Upper, (Vcchar_t*)("0123456789ABCDEF"), 16);
		memcpy(Lower, (Vcchar_t*)("0123456789abcdef"), 16);
		for(b = 0; b < 256; ++b)
			Rev[b] = (Vcchar_t)(~0);
		for(b = 0; b < 16; ++b)
		{	Rev[Upper[b]] = b; /* upper-case */
			Rev[Lower[b]] = b; /* lower-case */
		}
		Didinit = 1;
	}

	if(!byte || !hex)
		return -1;

	if(type >= 0) /* byte to hex */
	{	if(hexz < 2*bytez)
			return -1;

		/* 0 for lower-case, anything else upper-case */
		dig = type == 0 ? Lower : Upper;

		for(h = 0, b = 0; b < bytez; b += 1, h += 2)
		{	if(h >= hexz-1)
				return -1;
			hex[h]   = dig[(byte[b]>>4) & 0xf];
			hex[h+1] = dig[(byte[b]>>0) & 0xf];
		}

		if(h < hexz)
			hex[h] = 0;
		return h;
	}
	else /* hex to byte, allow mixed case */
	{	if(hexz%2 != 0 || bytez < hexz/2)
			return -1;
		for(b = 0, h = 0; h < hexz; h += 2, b += 1)
		{	if(b >= bytez ||
			   (l = Rev[hex[h+0]]) == (Vcchar_t)(~0) ||
			   (r = Rev[hex[h+1]]) == (Vcchar_t)(~0) )
				return -1;

			byte[b] = (Vcchar_t)((l<<4) | r);
		}

		if(b < bytez)
			byte[b] = 0;
		return b;
	}
}
