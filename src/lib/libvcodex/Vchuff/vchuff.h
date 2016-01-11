/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
#ifndef _VCHUFF_H
#define _VCHUFF_H	1

/* Types and functions to construct a static Huffman codes.
**
** Written By Kiem-Phong Vo
*/

/* A Huffman decoding trie is stored in Vchtrie_t.node and Vchtrie_t.size.
** size[p] > 0: a data byte has been decoded. In this case, size[p]
**		is the number of bits that should be consumed to finish
**		the bits corresponding to this byte. node[p] is the byte.
** size[p] < 0: need to recurse to the next level of the trie. In this
**		case, -size[p] is the number of bits needed to index the
**		next level. node[p] is the base of the next level.
** size[p] == 0: an undecipherable bit string. Data is likely corrupted.
*/
typedef struct _vchtrie_s
{	short*		node;	/* data or next trie base to look up	*/
	short*		size;	/* >0: code sizes, 0: internal nodes 	*/
	short		ntop;	/* # of bits to index top trie level	*/
	short		trsz;	/* allocated memory for the trie	*/
	short		next;
} Vchtrie_t;

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif
#if !_BLD_vcodex && defined(__IMPORT__)
#define extern		extern __IMPORT__
#endif

extern Vcmethod_t*	Vchuffman;	/* Huffman compression		*/
extern Vcmethod_t*	Vchuffgroup;	/* Huffman with grouping	*/
extern Vcmethod_t*	Vchuffpart;	/* Huffman with partitioning	*/

#undef	extern

#if _BLD_vcodex && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern ssize_t		vchsize _ARG_((ssize_t, ssize_t*, ssize_t*, int*));
extern ssize_t		vchbits _ARG_((ssize_t, ssize_t*, Vcbit_t*));
extern Vchtrie_t*	vchbldtrie _ARG_((ssize_t, ssize_t*, Vcbit_t*));
extern Void_t		vchdeltrie _ARG_((Vchtrie_t*));
extern ssize_t		vchgetcode _ARG_((ssize_t, ssize_t*, ssize_t, Vcchar_t*, size_t));
extern ssize_t		vchputcode _ARG_((ssize_t, ssize_t*, ssize_t, Vcchar_t*, size_t));
extern int		vchcopy _ARG_((Vcodex_t*, ssize_t*, ssize_t*, ssize_t));

#undef	extern

_END_EXTERNS_

#endif /*_VCHUFF_H*/
