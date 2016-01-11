/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2012 AT&T Intellectual Property          *
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

#ifdef	getdents3d

ssize_t
getdents3d(int fd, void* buf, size_t n)
{
	ssize_t		r;

#if FS
	register Mount_t*	mp;

	if (!fscall(NiL, MSG_getdents, 0, fd, buf, n))
		return(state.ret);
	mp = monitored();
#endif
	if ((r = GETDENTS(fd, buf, n)) >= 0)
	{
#if FS
		if (mp)
			fscall(mp, MSG_getdents, r, fd, buf, n);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_getdents))
				fscall(mp, MSG_getdents, r, fd, buf, n);
#endif
	}
	return(r);
}

#else

NoN(getdents)

#endif

#ifdef	getdents643d

ssize_t
getdents643d(int fd, void* buf, size_t n)
{
	ssize_t		r;

#if FS
	register Mount_t*	mp;

	if (!fscall(NiL, MSG_getdents, 0, fd, buf, n))
		return(state.ret);
	mp = monitored();
#endif
	if ((r = GETDENTS64(fd, buf, n)) >= 0)
	{
#if FS
		if (mp)
			fscall(mp, MSG_getdents, r, fd, buf, n);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_getdents))
				fscall(mp, MSG_getdents, r, fd, buf, n);
#endif
	}
	return(r);
}

#else

NoN(getdents64)

#endif
