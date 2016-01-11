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
rename3d(const char* path, const char* target)
{
	register char*	sp;
	register int	r;
	char*		t;
	int		oerrno;
	char		buf[PATH_MAX+1];
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_rename, 0, path, target))
		return(state.ret);
	mp = monitored();
#endif
	if (!(sp = pathreal(path, P_TOP|P_LSTAT, NiL)))
		return(-1);
	strncpy(buf, sp, PATH_MAX);
	if (!(sp = pathreal(target, P_PATHONLY|P_NOOPAQUE, NiL)))
		return(-1);
	oerrno = errno;
	if ((r = RENAME(buf, sp)) && errno == ENOENT && (t = strrchr(sp, '/')))
	{
		*t = 0;
		r = fs3d_mkdir(sp, S_IRWXU|S_IRWXG|S_IRWXO);
		*t = '/';
		if (!r)
		{
			errno = oerrno;
			r = RENAME(buf, sp);
		}
	}
#if FS
	if (!r)
	{
		if (mp)
		{
			if (sp != buf)
			{
				if (!(sp = pathreal(target, P_PATHONLY|P_NOOPAQUE|P_ABSOLUTE, NiL)))
					return(r);
				sp = strncpy(buf, sp, PATH_MAX);
			}
			fscall(mp, MSG_rename, 0, path, sp);
		}
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_rename))
			{
				if (sp != buf)
				{
					if (!(sp = pathreal(target, P_PATHONLY|P_NOOPAQUE|P_ABSOLUTE, NiL)))
						return(r);
					sp = strncpy(buf, sp, PATH_MAX);
				}
				fscall(mp, MSG_rename, 0, path, sp);
			}
	}
#endif
	return(r);
}
