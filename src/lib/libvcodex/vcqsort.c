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

/*	Like qsort() but allows a struct to hold add'l data describing objects.
**
**	Written by Kiem-Phong Vo.
*/

#define SWAP(le, re, ne) \
do {	Vcchar_t *ll = (Vcchar_t*)(le); \
	Vcchar_t *rr = (Vcchar_t*)(re); \
	ssize_t   nn = (ne); \
	for(; nn > 0; --nn, ++ll, ++rr) \
		{ int ss = *ll; *ll = *rr; *rr = ss; } \
} while(0)

#if __STD_C
void vcqsort(Void_t* list, ssize_t n, ssize_t size, Vccompare_f comparf, Void_t* disc)
#else
void vcqsort(list, n, size, sortf, disc)
Void_t*		list;	/* list of objects to be sorted	*/
ssize_t		n;	/* number of objects in list[]	*/
ssize_t		size;	/* size in byte of each object	*/
Vccompare_f	comparf; /* comparison function		*/
Void_t*		disc;	/* adjunct struct for sortf()	*/
#endif
{
	ssize_t		l, r;
	Vcchar_t	*base = (Vcchar_t*)list;

	if(n <= 1)
		return;

	if(n == 2)
	{	if((*comparf)(base, base+size, disc) > 0)
			SWAP(base, base+size, size);
		return;
	}

	for(l = 1, r = n; l < r; ) /* pivot on element 0 */
	{	if((*comparf)(base, base + l*size, disc) >= 0)
			l += 1;
		else if((r -= 1) > l)
			SWAP(base + l*size, base + r*size, size);
	}

	if((l -= 1) > 0) /* move the pivot into its final place */
		SWAP(base, base + l*size, size);

	if(l > 1)
		vcqsort(base, l, size, comparf, disc);

	if((n -= r) > 1)
		vcqsort(base + r*size, n, size, comparf, disc);
}
