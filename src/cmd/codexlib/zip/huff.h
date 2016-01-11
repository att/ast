#pragma prototyped

/*
 * zip huffman codeing interface
 */

#ifndef _HUFF_H
#define _HUFF_H		1

#include "zip.h"

#include <vmalloc.h>

/* Huffman code lookup table entry--this entry is four bytes for machines
   that have 16-bit pointers (e.g. PC's in the small or medium model).
   Valid extra bits are 0..13.	e == 15 is EOB (end of block), e == 16
   means that v is a literal, 16 < e < 32 means that v is a pointer to
   the next table, which codes e - 16 bits, and lastly e == 99 indicates
   an unused code.  If a code with e == 99 is looked up, this implies an
   error in the data. */

struct Huff_s; typedef struct Huff_s Huff_t;

struct Huff_s
{
    uch			e;	/* number of extra bits or operation */
    uch			b;	/* number of bits in this code or subcode */
    union
    {
	ush		n;	/* literal, length base, or distance base */
	Huff_t*		t;	/* pointer to next level of table */
    } v;
};

extern int	huff(ulg*, ulg, ulg, ush*, ush*, Huff_t**, int*, Vmalloc_t*);

#endif
