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
#include	"vchhdr.h"

/* Construct the code bits given their lengths.
**
** Written by Kiem-Phong Vo
*/

/* sort by code lengths */
#if __STD_C
static int sizecmp(Void_t* one, Void_t* two, Void_t* disc)
#else
static int sizecmp(one,two,disc)
Void_t*	one;
Void_t*	two;
Void_t*	disc;
#endif
{
	int	d;
	ssize_t	*o = *((ssize_t**)one), *t = *((ssize_t**)two);

	return (d = *o - *t) != 0 ? d : o < t ? -1 : o > t ? 1 : 0;
}

#if __STD_C
ssize_t vchbits(ssize_t nsym, ssize_t* size, Vcbit_t* bits)
#else
ssize_t vchbits(nsym, size, bits)
ssize_t		nsym;	/* alphabet size or #symbols	*/
ssize_t*	size;	/* encoding lengths of bytes	*/
Vcbit_t*	bits;	/* encoding bits to be computed	*/
#endif
{
	int		i, notz, k, s;
	ssize_t		**sort;
	Vcbit_t		b = 0;

	if(!(sort = (ssize_t**)malloc(nsym*sizeof(ssize_t*))) )
		return -1;

	for(notz = 0, i = 0; i < nsym; ++i)
	{	if(size[i] == 0)
			continue;
		sort[notz++] = size+i;
	}
	vcqsort(sort, notz, sizeof(ssize_t*), sizecmp, 0);

	for(i = 0; i < notz; i = k)
	{	s = *sort[i];
		for(k = i; k < notz && *sort[k] == s; ++k)
			bits[sort[k]-size] = (b++) << (VC_BITSIZE - s);
		if(k < notz)
			b <<= (*sort[k] - s);
	}

	/* return the maximum size of any code */
	s = notz <= 0 ? 0 : *sort[notz-1];

	free(sort);
	return s;
}
