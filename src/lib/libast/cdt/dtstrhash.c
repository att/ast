/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#include	"dthdr.h"

/* Hashing a string into an unsigned integer.
** This is the FNV (Fowler-Noll-Vo) hash function.
** Written by Kiem-Phong Vo, phongvo@gmail.com (01/10/2012)
*/

uint dtstrhash(uint h, char* args, int n)
{
	unsigned char	*s = (unsigned char*)args;

#if _ast_sizeof_int == 8 /* 64-bit hash */
#define	FNV_PRIME	((1<<40) + (1<<8) + 0xb3)
#define FNV_OFFSET	0x8000000000000000
#else /* 32-bit hash */
#define	FNV_PRIME	((1<<24) + (1<<8) + 0x93)
#define FNV_OFFSET	0x811c9dc5
#endif
	h = (h == 0 || h == (uint)~0) ? FNV_OFFSET : h;
	if(n <= 0) /* see discipline key definition for == 0 */
	{	for(; *s != 0; ++s )
			h = ((h ^ s[0]) * FNV_PRIME) ^ (h >> (_ast_sizeof_int-1)*8);
	}
	else
	{	unsigned char	*ends;
		for(ends = s+n; s < ends; ++s)
			h = ((h ^ s[0]) * FNV_PRIME) ^ (h >> (_ast_sizeof_int-1)*8);
	}

	return h;
}

