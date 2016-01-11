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

int
unlink3d(register const char* path)
{
	register char*	sp;
	register int	r;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_remove, 0, path))
		return state.ret;
	mp = monitored();
#endif
	if (!(sp = pathreal(path, P_PATHONLY|P_SAFE, NiL)))
		return -1;
	if (state.path.level)
		return 0;
	if (!(r = LSTAT(sp, &state.path.st)))
	{
		if (S_ISLNK(state.path.st.st_mode) && !checklink(sp, &state.path.st, P_PATHONLY|P_LSTAT) && state.path.linksize > 0)
		{
			/*
			 * remove instance if not default
			 */

			r = strlen(sp) - (sizeof(state.vdefault) - 1);
			if (r > 3 && streq(sp + r, state.vdefault))
				return 0;
		}
		r = UNLINK(sp);
	}
	if (!r)
	{
#if FS
		if (mp)
			fscall(mp, MSG_remove, 0, path);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_remove))
				fscall(mp, MSG_remove, 0, path);
#endif
	}
	else if (errno == ENOENT && pathreal(path, 0, NiL))
		r = 0;
	return r;
}

#undef	remove

int
remove(const char* path)
{
	return unlink(path);
}
