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
 * Header file for Huffman coding
 * The coding is the same as that used with the System V pack program
 *
 *   David Korn
 *   AT&T Laboratories
 */

#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_	1

#include	<ast.h>

#define HUFFLEV		32	/* maximum number of bits per code */
#define HUFFMAG1	037	/* ascii <US> */
#define HUFFMAG2	036	/* ascii <RS> */

typedef struct
{
	char		length[(1<<CHAR_BIT)+1];
	unsigned char	levcount[HUFFLEV+1];
	Sfoff_t		insize;
	Sfoff_t		outsize;
	long		buffer;
	long		id;
	int		left;
	int		maxlev;
	int		nchars;
	int		excess;
} Huff_t;

Huff_t*		huffinit(Sfio_t*,Sfoff_t);
Huff_t*		huffgethdr(Sfio_t*);
int	 	huffputhdr(Huff_t*,Sfio_t*);
Sfoff_t		huffencode(Huff_t*,Sfio_t*,Sfio_t*,int);
Sfoff_t		huffdecode(Huff_t*,Sfio_t*,Sfio_t*,int);
Sfio_t*		huffdisc(Sfio_t*);

#define huffend(hp)	free((void*)(hp))
#define huffisize(hp)	((hp)->insize)
#define huffosize(hp)	((hp)->outsize)
#define huffhsize(hp)	((hp)->maxlev+(hp)->nchars+7)

#endif /* !_HUFFMAN_H_ */
