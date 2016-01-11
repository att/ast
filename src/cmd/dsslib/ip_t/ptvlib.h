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

#ifndef _PTVPREFIX_PRIVATE_

#define _PTVPREFIX_PRIVATE_ \
	Dtlink_t	link;

#define _PTV_PRIVATE_ \
	int		nr; \
	unsigned char*	a;

#include <cdt.h>
#include <fv.h>
#include <ip6.h>
#include <error.h>

#include "ptv.h"

#define PTVSCAN(t,b,s)										\
	do											\
	{											\
		Ptvprefix_t*	_pt_p;								\
		unsigned int	b;								\
		fvset((t)->size, (t)->r[1], 1);							\
		for (_pt_p = (Ptvprefix_t*)dtfirst((t)->dict); _pt_p; _pt_p = (Ptvprefix_t*)dtnext((t)->dict, _pt_p))				\
		{										\
			fvcpy((t)->size, (t)->r[2], _pt_p->min);				\
			do									\
			{									\
				fvcpy((t)->size, (t)->r[0], (t)->r[2]);				\
				fvset((t)->size, (t)->r[3], 1);					\
				b = (t)->size * 8;						\
				for (;;)							\
				{								\
					if (fvcmp((t)->size, (t)->r[3], (t)->r[1]) < 0) 	\
						break;						\
					if (fvodd((t)->size, (t)->r[0]))			\
						break;						\
					fvlsh((t)->size, (t)->r[4], (t)->r[3], 1);		\
					fvsub((t)->size, (t)->r[5], (t)->r[4], (t)->r[1]);	\
					fvior((t)->size, (t)->r[4], (t)->r[0], (t)->r[5]);	\
					if (fvcmp((t)->size, (t)->r[4], _pt_p->max) > 0)	\
						break;						\
					fvrsh((t)->size, (t)->r[4], (t)->r[0], 1);		\
					fvcpy((t)->size, (t)->r[0], (t)->r[4]);			\
					fvlsh((t)->size, (t)->r[4], (t)->r[3], 1);		\
					fvcpy((t)->size, (t)->r[3], (t)->r[4]);			\
					b--;							\
				}								\
				fvcpy((t)->size, (t)->r[0], (t)->r[2]);				\
				s;								\
				if (!b)								\
					break;							\
				fvadd((t)->size, (t)->r[4], (t)->r[0], (t)->r[3]);		\
				fvcpy((t)->size, (t)->r[0], (t)->r[4]);				\
				if (fvcmp((t)->size, (t)->r[2], (t)->r[0]) < 0)			\
					break;							\
			} while (fvcmp((t)->size, (t)->r[2], _pt_p->max) <= 0);			\
		}										\
	} while (0)

#endif
