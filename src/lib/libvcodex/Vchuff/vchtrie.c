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


/*	Decoding Huffman-encoded data amounts to taking a sequence of
**	bits and computing the longest bit-string representing bytes that
**	matches a prefix of this bit sequence, then emitting the byte.
**	For fast prefix matching, we use a recursive trie similar to
**	the Retrie structure described in "Fast Prefix Matching of
**	Bounded Strings" (Proceedings of ALENEX 2003).
**
**	Written by Kiem-Phong Vo
*/

#define TRIE_TOP	8	/* max #bits for top level  */
#define	TRIE_SUB	4	/* max #bits for sub levels */

typedef struct _node_s
{	short	node;
	short	size;
} Node_t;

#if __STD_C
static int bldtrie(Vchtrie_t* trie, ssize_t* clen, Vcbit_t* bits,
		   Node_t* root, ssize_t len, Vcbit_t** sort, ssize_t ns, ssize_t lev)
#else
static int bldtrie(trie, clen, bits, root, len, sort, ns, lev)
Vchtrie_t*	trie;	/* handle holding entire trie	*/
ssize_t*	clen;	/* table of code lengths	*/
Vcbit_t*	bits;	/* table of prefix codes	*/
Node_t*		root;	/* parent node of the new table	*/
ssize_t		len;	/* prefix len already processed	*/
Vcbit_t**	sort;	/* list of codes sharing prefix	*/
ssize_t		ns;	/* # of elts in sort[]		*/
ssize_t		lev;	/* level in the trie		*/
#endif
{
	ssize_t		k, m, p, s, e, z, msk, olen;
	short		*node, *size;
	Node_t		nd;

	/* compute the max prefix length for this set */
	for(m = 0, k = 0; k < ns; ++k)
		if((s = clen[sort[k]-bits]) > m)
			m = s;

	/* # of bits to pick out at this level */
	if((p = lev == 0 ? TRIE_TOP : TRIE_SUB) > (m - len) )
		p = m - len;

	if((trie->next + (1<<p)) > trie->trsz)
	{	s = trie->next + ((1<<p) < (1<<8) ? (1<<8) : (1<<p));
		if(!(node = (short*)malloc(2*s*sizeof(short))) )
			return -1;
		size = node+s;
		memcpy(node, trie->node, trie->next*sizeof(short));
		memcpy(size, trie->size, trie->next*sizeof(short));
		if(trie->node)
			free(trie->node);
		trie->node = node;
		trie->size = size;
		trie->trsz = s;
	}

	/* set data for parent node of the new table */
	root->node = trie->next;
	root->size = -p;
	trie->next += (1<<p);

	/* now point trie to the new table */
	node = trie->node + root->node;
	size = trie->size + root->node;
	memset(size, 0, (1<<p)*sizeof(short));

	olen = len;
	len += p;
	msk  = (1<<p)-1;
	for(k = 0; k < ns; )
	{	/* starting index of this code in the table */
		s = (*sort[k] >> (VC_BITSIZE-len)) & msk;

		z = clen[m = sort[k]-bits];
		if((p = len - z) >= 0) /* data nodes */
		{	for(e = s + (1<<p); s < e; ++s)
			{	node[s] = m; /* the byte to be decoded */
				size[s] = z-olen; /* # bits to consume */
			}
			k += 1;
		}
		else	/* internal node, find all codes sharing same prefix */
		{	for(m = k+1; m < ns; ++m)
			{	if(len >= clen[sort[m]-bits])
					break;
				if(s != ((*sort[m] >> (VC_BITSIZE-len)) & msk) )
					break;
			}

			/* recurse to process this group */
			if(bldtrie(trie,clen,bits,&nd,len,sort+k,m-k,lev+1) < 0 )
				return -1;

			/* reset in case these were relocated in recursion */
			node = trie->node + root->node;
			size = trie->size + root->node;

			/* now set data for this internal node */
			node[s] = nd.node; /* base of next level table */
			size[s] = nd.size; /* #bits needed to index it */

			k = m;
		}
	}

	return 0;
}

/* sort by prefix bits */
#if __STD_C
static int bitscmp(Void_t* one, Void_t* two, Void_t* disc)
#else
static int bitscmp(one, two, disc)
Void_t*	one;
Void_t*	two;
Void_t*	disc;
#endif
{
	Vcbit_t	*o = *((Vcbit_t**)one), *t = *((Vcbit_t**)two);
	return *o < *t ? -1 : 1;
}

#if __STD_C
Vchtrie_t* vchbldtrie(ssize_t nsym, ssize_t* size, Vcbit_t* bits)
#else
Vchtrie_t* vchbldtrie(nsym, size, bits)
ssize_t		nsym;	/* alphabet size or #symbols	*/
ssize_t*	size;	/* array of code lengths	*/
Vcbit_t*	bits;	/* array of code bits		*/
#endif
{
	ssize_t		k, ns;
	Vcbit_t		**sort;
	Vchtrie_t	*trie;
	Node_t		root;

	if(!(sort = (Vcbit_t**)malloc(nsym*sizeof(Vcbit_t*))) )
		return NIL(Vchtrie_t*);

	if(!(trie = (Vchtrie_t*)malloc(sizeof(Vchtrie_t))) )
	{	free(sort);
		return NIL(Vchtrie_t*);
	}
	trie->next = 0;
	trie->trsz = 0;
	trie->node = NIL(short*);
	trie->size = NIL(short*);

	/* compute list of weighted elements */
	for(ns = 0, k = 0; k < nsym; ++k)
		if(size[k] > 0)
			sort[ns++] = bits+k;
	vcqsort(sort, ns, sizeof(Vcbit_t*), bitscmp, 0);

	if(bldtrie(trie, size, bits, &root, 0, sort, ns, 0) < 0 )
		return NIL(Vchtrie_t*);

	free(sort);

	trie->ntop = -root.size;
	return trie;
}

#if __STD_C
Void_t vchdeltrie(Vchtrie_t* trie)
#else
Void_t vchdeltrie(trie)
Vchtrie_t*	trie;
#endif
{
	if(trie)
	{	if(trie->node)
			free(trie->node);
		free(trie);
	}
}
