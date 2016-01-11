/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "3d.h"

#ifdef	brk3d

int
brk3d(void* p)
{
#if FS
	register Mount_t*	mp;
	int			r;

	initialize();
	if (!(r = BRK(p)))
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_break))
				fscall(mp, MSG_break, 0, p);
	return(r);
#else
	return(BRK(p));
#endif
}

#else

NoN(brk)

#endif
