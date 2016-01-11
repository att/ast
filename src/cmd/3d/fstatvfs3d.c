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

#if defined(fstatvfs3d) && defined(_sys_statvfs)

#include <sys/statvfs.h>

int
fstatvfs3d(int fd, struct statvfs* fs)
{
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_fstatfs, 0, fd, fs))
		return(state.ret);
	mp = monitored();
#endif
	if (FSTATVFS(fd, fs))
		return(-1);
#if FS
	if (mp)
		fscall(mp, MSG_fstatfs, 0, fd, fs);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_fstatfs))
			fscall(mp, MSG_fstatfs, 0, fd, fs);
#endif
	return(0);
}

#else

NoN(fstatvfs)

#endif
