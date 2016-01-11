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
 * huffman coding initialization
 *
 *   David Korn
 *   AT&T Laboratories
 */

#include	"huffman.h"
#include	<error.h>

#define END		(1<<CHAR_BIT)

/* the heap */
typedef struct
{
	long int count;
	int node;
} Heap_t;

static long	count[END+1];
static int	lastnode;
static void	heapify(Heap_t*, int, int);

Huff_t *huffinit(Sfio_t *infile, Sfoff_t insize)
{
	register int		n;
	register unsigned char *inbuff;
	register int		i, c;
	register Huff_t		*hp;
	register Sfoff_t	size = insize;
	int			parent[2*END+1];
	Heap_t 			heap[END+2];
	if(!(hp=newof(0, Huff_t, 1, 0)))
	{
		errno = ENOMEM;
		return((Huff_t*)0);
	}
	for (i=0; i<END; i++)
		count[i] = 0;
	while(inbuff=(unsigned char*)sfreserve(infile,SF_UNBOUND,0))
	{
		n = sfvalue(infile);
		if(size>=0)
		{
			if(n > size)
				n = size;
			size -= n;
		}
		hp->insize +=n;
		while (n > 0)
			count[inbuff[--n]]++;
	}
	if(n < 0)
	{
		huffend(hp);
		return((Huff_t*)0);
	}
	for (i=0; i<END; i++)
		count[i] += count[i];
	count[END] = 1;
	/* put occurring chars in heap with their counts */
	for (i=END; i>=0; i--)
	{
		parent[i] = 0;
		if (count[i] > 0)
		{
			heap[++n].count = count[i];
			heap[n].node = i;
		}
	}
	hp->nchars = n-1;
	for (i=n/2; i>=1; i--)
		heapify(heap,i,n);
	/* build Huffman tree */
	lastnode = END;
	while (n > 1)
	{
		parent[heap[1].node] = ++lastnode;
		size = heap[1].count;
		heap[1] = heap[n];
		n--;
		heapify(heap,1,n);
		parent[heap[1].node] = lastnode;
		heap[1].node = lastnode;
		heap[1].count += size;
		heapify(heap,1,n);
	}
	parent[lastnode] = 0;
	/* assign lengths to encoding for each character */
	size = hp->maxlev = 0;
	for (i=1; i<=HUFFLEV; i++)
		hp->levcount[i] = 0;
	for (i=0; i<=END; i++)
	{
		c = 0;
		for(n=parent[i]; n!=0; n=parent[n])
			c++;
		hp->levcount[c]++;
		hp->length[i] = c;
		if (c > hp->maxlev)
			hp->maxlev = c;
		size += c*(count[i]>>1);
	}
	hp->outsize =  (size+CHAR_BIT-1)/CHAR_BIT;
	return(hp);
}

/* makes a heap out of heap[i],...,heap[n] */
static void heapify (register Heap_t *heap,register int i,register int n)
{
	register int k;
	register int lastparent = n/2;
	Heap_t heapsubi;
	heapsubi = heap[i];
	while (i <= lastparent)
	{
		k = 2*i;
		if (heap[k].count > heap[k+1].count && k < n)
			k++;
		if (heapsubi.count < heap[k].count)
			break;
		heap[i] = heap[k];
		i = k;
	}
	heap[i] = heapsubi;
}
