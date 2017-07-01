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
#pragma prototyped

#include "asohdr.h"

#if defined(_UWIN) && defined(_BLD_ast)

NoN(asolock)

#else

int
asolock(unsigned int volatile* lock, unsigned int key, int type)
{
	if (key == 0 ) 
		return -1;
	else if (type & ASO_TRYLOCK)
		return asocasint(lock, 0, key) == 0 ? 1 : -1;
	else if (type & ASO_LOCK)
	{	for (;; asospinrest())
			if (asocasint(lock, 0, key) == 0)
				return 1;
	}
	else /*if(type & ASO_UNLOCK)*/
		return asocasint(lock, key, 0) == key ? 1 : -1;
}

#endif
