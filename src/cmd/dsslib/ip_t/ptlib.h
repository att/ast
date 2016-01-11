/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * prefix table private interface
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _PTPREFIX_PRIVATE_

#define _PTPREFIX_PRIVATE_ \
	Dtlink_t	link;

#include <cdt.h>
#include <error.h>

#include "pt.h"

#define PTSCAN(t,x,b,m,s) \
	do								\
	{								\
		Ptprefix_t*	_pt_p;					\
		Ptaddr_t	_pt_min;				\
		Ptaddr_t	_pt_max;				\
		Ptaddr_t	m;					\
		Ptaddr_t	x;					\
		unsigned int	b;					\
		for (_pt_p = (Ptprefix_t*)dtfirst((t)->dict); _pt_p; _pt_p = (Ptprefix_t*)dtnext((t)->dict, _pt_p))				\
		{							\
			_pt_min = _pt_p->min;				\
			_pt_max = _pt_p->max;				\
			do						\
			{						\
				x = _pt_min;				\
				m = 1;					\
				b = PTBITS;				\
				while (m &&				\
				       !(x & ((1<<PTSHIFT)-1)) &&	\
				       (_pt_min|((m<<PTSHIFT)-1)) <= _pt_max) \
				{					\
					x >>= PTSHIFT;			\
					m <<= PTSHIFT;			\
					b -= PTSHIFT;			\
				}					\
				x = _pt_min;				\
				s;					\
				if (!b || (_pt_min += m) < x)		\
					break;				\
			} while (_pt_min <= _pt_max);			\
		}							\
	} while (0)

#endif
