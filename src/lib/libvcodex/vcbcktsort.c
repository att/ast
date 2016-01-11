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
#include	"vchdr.h"

/*	Counting bucket sort.
**	Return the number of distinct bytes. The bckt[] argument returns
**	the cumulative counts of all bytes. Thus, bckt[0] is the count
**	of byte 0, bckt[1] is the cumulative count of 0 and 1, and so on.
**
**	Written by Kiem-Phong Vo.
*/

#if __STD_C
ssize_t vcbcktsort(ssize_t* indx, ssize_t* list, ssize_t n, Vcchar_t* data, ssize_t* bckt)
#else
ssize_t vcbcktsort(indx, list, n, data, bckt)
ssize_t*	indx;	/* output sorted indxes	*/
ssize_t*	list;	/* indices to be sorted	*/
ssize_t		n;	/* # of indices		*/
Vcchar_t*	data;	/* data used to sort	*/
ssize_t*	bckt;	/* [256] buckets	*/
#endif
{
	ssize_t		i, p, c;
	ssize_t		distinct = 0;

	/* count byte frequencies */
	memset(bckt, 0, 256*sizeof(ssize_t));
	if(list) /* sort using secondary predictor */
	{	for(p = 0; p < n; ++p)
			bckt[data[list[p]]] += 1;
	}
	else /* unsorted permutation was the identity */
	{	for(p = 0; p < n; ++p)
			bckt[data[p]] += 1;
	}

	for(p = 0, i = 0; i < 256; ++i) /* starting positions */
	{	if((c = bckt[i]) > 0)
			distinct += 1;
		bckt[i] = p;
		p += c;
	}

	if(list) /* sorting a sublist of indices */
	{	for(p = 0; p < n; ++p)
			indx[bckt[data[list[p]]]++] = list[p];
	}
	else /* sorting all indices */
	{	for(p = 0; p < n; ++p)
			indx[bckt[data[p]]++] = p;
	}

	return distinct;
}


#if 0
/* some intermediate versions of Vcodex used this implementation and became
** incompatible with earlier version of Vctable. So this is saved here in case
** we ever need to deal with data compressed with such versions.
*/
#if __STD_C
ssize_t vcbcktsort(ssize_t* sort, ssize_t* list, ssize_t n, Vcchar_t* data, ssize_t* bckt)
#else
ssize_t vcbcktsort(sort, list, n, data, bckt)
ssize_t*	sort;	/* to output sorted elements	*/
ssize_t*	list;	/* != NULL: list to be sorted	*/
			/* == NULL: sorting 0,1,...n-1	*/
ssize_t		n;	/* # of elements to be sorted	*/
Vcchar_t*	data;	/* data identifying elements	*/
ssize_t*	bckt;	/* temp space of [256] buckets 	*/
#endif
{
	ssize_t		p, i, c, minc, maxc;
	ssize_t		distinct = 0;

	/* count byte frequencies */
	memset(bckt, 0, 256*sizeof(ssize_t));
	minc = 256; maxc = -1;
	for(p = 0; p < n; ++p)
	{	c = data[list ? list[p] : p];
		minc = c < minc ? c : minc;
		maxc = c > maxc ? c : maxc;
		bckt[c] += 1;
	}

	/* set starting positions for each bucket */
	for(p = 0, c = minc; c <= maxc; ++c)
	{	if((i = bckt[c]) <= 0)
			continue;
		distinct += 1; /* count distinct letters */
		bckt[c] = p; p += i;
	}

	/* now sort them into place */
	for(p = 0; p < n; ++p)
	{	i = list ? list[p] : p;
		sort[bckt[data[i]]++] = i;
	}

	return distinct;
}
#endif
