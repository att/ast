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
 * huffman coding routine to read a pack format header
 *
 *   David Korn
 *   AT&T Laboratories
 */

#include	"huffman.h"
#include	<error.h>

#define END	(1<<CHAR_BIT)

Huff_t *huffgethdr(register Sfio_t *infile)
{
	register Huff_t*	hp;
	register int		i, j, c;
	/* allocate space for huffman tree */
	if(!(hp=newof(0, Huff_t, 1, 0)))
	{
		errno = ENOMEM;
		return((Huff_t*)0);
	}
	/* check two-byte header */
	if(sfgetc(infile)!=HUFFMAG1 || sfgetc(infile)!=HUFFMAG2)
		goto error;
	/* get size of original file, */
	for (i=0; i<4; i++)
		hp->insize = (hp->insize<<CHAR_BIT)+ sfgetc(infile);
	/* get number of levels in maxlev, */
	hp->maxlev = sfgetc(infile);
	if(hp->maxlev==0)
	{
		/* larger than  2**32 */
		for (i=0; i<4; i++)
			hp->insize = (hp->insize<<CHAR_BIT)+ sfgetc(infile);
		hp->maxlev = sfgetc(infile);
	}
	if(hp->maxlev < 0 || hp->maxlev > HUFFLEV)
		goto error;
	/* get number of leaves on level i  */
	for (i=1; i<=hp->maxlev; i++)
	{
		if((c=sfgetc(infile)) < 0)
			goto error;
		hp->levcount[i] = c;
	}
	/* read in the characters and compute number of bits for each */
	for(i=0; i <= END; i++)
		hp->length[i] = 0;
	hp->nchars = 0;
	for (i=1; i<=hp->maxlev; i++)
	{
		j = hp->levcount[i];
		if((hp->nchars += j) >=END)
			goto error;
		while (j-- > 0)
		{
			if((c=sfgetc(infile)) < 0)
				goto error;
			hp->length[c] = i;
		}
	}
	if((c=sfgetc(infile)) < 0)
		goto error;
	hp->length[c] = i-1;
	return(hp);

error:
	huffend(hp);
	return ((Huff_t*)0);
}
