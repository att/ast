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
 * huffman coding encoding
 *
 *   David Korn
 *   AT&T Laboratories
 */

#include	"huffman.h"

#define END	(1<<CHAR_BIT)

static long	lastid = 1;
static long	id = 0x8000;
static int	bits[END+1];
static unsigned char *outend;
static unsigned char *outbuff;
static int 	putbuff(Sfio_t*,unsigned char*);

#define putbits(buff,left,bits,n)	(left+=(n),buff=((buff<<(n))|(bits)))
#define outchars(fp,buff,left,outp,n)	while(left>=CHAR_BIT)\
				{ \
					left-=CHAR_BIT; \
					*outp++ = buff>>left; \
					if(outp>=outend) \
					{ \
						if((n=putbuff(fp,outp)) < 0) \
							return(-1); \
						hp->outsize += n; \
						outp = outbuff; \
					} \
				}

/*
 * encode <size> bytes of <infile> using <hp> encoding and write
 * result to <outfile>
 * encode until end-of-file of <size> < 0.
 */

Sfoff_t huffencode(register Huff_t *hp,Sfio_t *infile,Sfio_t *outfile,int size)
{
	register long buffer;
	register int left, i, c;
	register unsigned char *inbuff;
	register int n;
	register unsigned char *outp;
	register Sfio_t *fp = outfile;
	if(hp->id != lastid)
	{
		/* compute the bit patterns for each character */
		for (n=0, i=hp->maxlev; i>0; i--)
		{
			for (c=0; c<=END; c++)
				if (hp->length[c] == i)
					bits[c] = n++;
			n >>= 1;
		}
		if(!hp->id)
			hp->id = id++;
		lastid = hp->id;
	}
	buffer = hp->buffer;
	left = hp->left;
	hp->outsize = 0;
	if(!(outp=outbuff=(unsigned char*)sfreserve(fp,SF_UNBOUND,SF_LOCKR)))
		return(-1);
	outend = outp + sfvalue(fp);
	do
	{
		if(!(inbuff=(unsigned char*)sfreserve(infile,SF_UNBOUND,0)))
		{
			if((n=sfvalue(infile))==0)
			{
				c = END;
				goto endof;
			}
			return(-1);
		}
		n = sfvalue(infile);
		if(size>=0 && size<n)
			n = size;
		while(n-- > 0)
		{
			c = *inbuff++;
		endof:
			i = hp->length[c];
			putbits(buffer,left,bits[c],i);
			outchars(fp,buffer,left,outp,i);
		}
		if(size>0 && n && (size -=n)<=0)
			goto done;
	}
	while (c != END);
	if(left)
	{
		i = CHAR_BIT-left;
		putbits(buffer,left,0,i);
		outchars(fp,buffer,left,outp,i);
		buffer = 0;
	}
done:
	hp->buffer = buffer;
	hp->left = left;
	n = outp - outbuff; 
	hp->outsize += n; 
	if(sfwrite(fp,outbuff,n)<0) 
		return(-1);
	return(hp->outsize);
}

static int putbuff(register Sfio_t *fp,register unsigned char *outp)
{
	register int n = outp - outbuff;
	if(sfwrite(fp,outbuff,n)< 0)
		return(-1);
	if(!(outbuff=(unsigned char*)sfreserve(fp,SF_UNBOUND,SF_LOCKR)))
		return(-1);
	outend = outbuff+(n=sfvalue(fp));
	return(n);
}

