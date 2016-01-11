/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1993-2011 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * huffman coding decoding
 *
 *   David Korn
 *   AT&T Laboratories
 */

#include	"huffman.h"

#define CHUNK	9
#define END	(1<<CHAR_BIT)
#define fillbuff(b,left,bit,inp)	while(left<(bit))\
					{\
						if(inp>=inend && !(inp=getbuff())) \
							return(-1); \
						left+=CHAR_BIT;\
						b = (b<<CHAR_BIT)|*inp++; \
					}
/* read <n> bits from buffer <b> */
#define getbits(b,left,n)	(((b)>>(left-=(n)))&((1L<<(n))-1))
/* return <n> bits */
#define putbits(b,left,n)	(left+=(n))

static long	lastid = 1;
static long	id = 1;
static unsigned char outchar[(1<<CHUNK)];
static char	numbits[(1<<CHUNK)];
static short	intnodes[HUFFLEV+1];
static char	*tree[HUFFLEV+1];
static char	characters[END];
static char	*eof;
static unsigned char *inbuff,*inend;
static Sfio_t	*infile;
static void	decode_header(Huff_t *);
static unsigned	char *getbuff(void);

/*
 * decode <input> using huffman tree defined in <hp> onto <output>
 * if size>0, then decode until size bytes have been decoded
 * otherwise, the complete file is decoded.
 * The number of bytes written is returned or -1 on error
 */

Sfoff_t huffdecode(register Huff_t *hp,Sfio_t *input,Sfio_t *output,int size)
{
	register long buffer;
	register int left, i, n;
	register int lev = 0;
	register unsigned char *outp;
	register unsigned char *inp;
	unsigned char *outend;
	unsigned char *outbuff;
	Sfoff_t insize = hp->outsize;
	/* decode the header if called with different hp */
	if(lastid!=hp->id)
	{
		decode_header(hp);
		if(!hp->id)
			hp->id = id++;
		lastid = hp->id;
	}
	/* set up output buffers for faster access */
	if(!(outp=outbuff=(unsigned char*)sfreserve(output,SF_UNBOUND,SF_LOCKR)))
		return(sfvalue(output));
	n = sfvalue(output);
	if(size>=0)
	{
		if(n > size)
			n = size;
		size -= n;
	}
	outend = outp+n;
	/* set up input buffers for faster access */
	infile = input;
	if(!(inp=inbuff=(unsigned char*)sfreserve(infile,SF_UNBOUND,0)))
		return(sfvalue(infile));
	inend = inp + sfvalue(infile);
	buffer = hp->buffer;
	left = hp->left;
	/* main decoding loop */
	while (1)
	{
		if(lev==0)
		{
			fillbuff(buffer,left,hp->maxlev,inp);
			i = getbits(buffer,left,CHUNK);
			if((n=(numbits[i]-1)) >= 0)
			{
				putbits(buffer,left,n);
				*outp++ = outchar[i];
				goto pout;
			}
			if(hp->excess)
			{
				putbits(buffer,left,hp->excess);
				i >>= hp->excess;
			}
			lev = CHUNK-hp->excess;
		}
		i <<= 1;
		i += getbits(buffer,left,1);
		if ((n = i - intnodes[lev+1]) >= 0)
		{
			{
				register char *p = &tree[lev+1][n];
				if (p == eof)
				{
					size = 0;
					outend = outp;
				}
				else
					*outp++ = *p;
			}
		pout:
			if(outp >= outend)
			{
				n = outp - outbuff;
				hp->outsize +=n;
				if(sfwrite(output,outbuff,n)<0)
					return(-1);
				if(size==0)
				{
					hp->buffer = buffer;
					hp->left = left;
					sfread(infile, inbuff,inp-inbuff);
					return(hp->outsize-insize);
				}
				if(!(outp=outbuff=(unsigned char*)sfreserve(output,SF_UNBOUND,SF_LOCKR)))
					return(-1);
				n = sfvalue(output);
				if(size>0)
				{
					if(n > size)
						n = size;
					size -= n;
				}
				outp = outbuff;
				outend = outp + n;
			}
			lev = 0;
		}
		else
			lev++;
	}
}

static void decode_header(register Huff_t *hp)
{
	register int c, i, n, k;
	eof = &characters[0];
	for (i=1; i<=hp->maxlev; i++)
	{
		intnodes[i] = hp->levcount[i];
		tree[i] = eof;
		for (c=0; c < (1<<CHAR_BIT); c++)
		{
			if (hp->length[c] == i)
				*eof++ = c;
		}
	}
	intnodes[hp->maxlev] += 2;
	/*
	 * convert intnodes[i] to be number of
	 * internal nodes possessed by level i
	 */
	for (n=0, i=hp->maxlev; i>=1; i--)
	{
		c = intnodes[i];
		intnodes[i] = n /= 2;
		n += c;
	}
	/* compute output char and number of bits used for each CHUNK of bits */
	c = (1<<CHUNK) -1;
	for (i=1; i<=CHUNK; i++)
	{
		for(k = tree[i+1] - tree[i];--k>=0;)
		{
			for(n=0; n < 1<<(CHUNK-i); n++)
			{
				numbits[c] = CHUNK+1-i;
				outchar[c--] = tree[i][k];
			}
		}
	}
	if(hp->maxlev <= CHUNK)
	{
		hp->excess = CHUNK+1-hp->maxlev;
		hp->maxlev = CHUNK;
	}
	while(c >=0)
		numbits[c--] = 0;
}

static unsigned char *getbuff(void)
{
	register int n;
	register unsigned char *cp;
	if(cp=(unsigned char*)sfreserve(infile,SF_UNBOUND,0))
		inbuff = cp;
	n = sfvalue(infile);
	if(!cp && n<0)
		return(cp);
	inend = inbuff + n;
	return(inbuff);
}
