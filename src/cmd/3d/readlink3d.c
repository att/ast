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

ssize_t
readlink3d(const char* path, char* buf, register size_t size)
{
	size_t		r;
#if FS
	Mount_t*	mp;
#endif

	if (state.in_2d)
		return(READLINK(path, buf, size));
#if FS
	if (!fscall(NiL, MSG_readlink, 0, path, buf, size))
		return(state.ret);
	mp = monitored();
#endif
	if (!pathreal(path, P_READLINK, NiL))
		return(-1);

	/*
	 * see if link text is already in memory
	 */

	if (r = state.path.linksize)
	{
		if (r > state.path.linksize)
			r = state.path.linksize;
		memcpy(buf, state.path.linkname, r);
#if FS
		if (mp) fscall(mp, MSG_readlink, r, path, buf, size);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_readlink))
				fscall(mp, MSG_readlink, r, path, buf, size);
#endif
		return(r);
	}

	/*
	 * exists but not a symbolic link
	 */

	errno = EINVAL;
	return(-1);
}
