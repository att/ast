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

#ifdef fpathconf3d

long
fpathconf3d(int fd, int op)
{
	long		r;
	int		oerrno;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_fpathconf, 0, fd, op))
		return(state.ret);
	mp = monitored();
#endif
	oerrno = errno;
	errno = 0;
	r = FPATHCONF(fd, op);
	if (!errno)
	{
		errno = oerrno;
#if FS
		if (mp)
			fscall(mp, MSG_fpathconf, 0, fd, op);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_fpathconf))
				fscall(mp, MSG_fpathconf, 0, fd, op);
#endif
	}
	return(r);
}

#else

NoN(fpathconf)

#endif
