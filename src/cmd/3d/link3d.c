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

#ifdef link3d

int
link3d(const char* path, const char* target)
{
	register char*	sp;
	register char*	tp;
	register int	r;
	char*		t;
	int		oerrno;
	char		buf[PATH_MAX+1];
#if FS
	char		tmp[PATH_MAX+1];
	Mount_t*	mp;
#endif

	if (state.in_2d)
		return(LINK(path, target));
#if FS
	if (!fscall(NiL, MSG_link, 0, path, target))
		return(state.ret);
	mp = monitored();
#endif
	if (!(sp = pathreal(path, state.safe ? (P_SAFE|P_TOP) : P_TOP, NiL)))
		return(-1);
	sp = strncpy(buf, sp, PATH_MAX);
	if (!(tp = pathreal(target, P_PATHONLY|P_NOOPAQUE|P_SAFE, NiL)))
		return(-1);
	oerrno = errno;
	if ((r = LINK(sp, tp)) && errno == ENOENT && (t = strrchr(tp, '/')))
	{
		*t = 0;
		r = fs3d_mkdir(tp, S_IRWXU|S_IRWXG|S_IRWXO);
		*t = '/';
		if (!r)
		{
			errno = oerrno;
			r = LINK(sp, tp);
		}
	}
#if FS
	if (!r)
	{
		if (mp && !LSTAT(tp, &state.path.st))
		{
			if (tp != tmp)
			{
				if (!(tp = pathreal(target, P_PATHONLY|P_NOOPAQUE|P_ABSOLUTE, NiL)))
					return(r);
				tp = strncpy(tmp, tp, PATH_MAX);
			}
			fscall(mp, MSG_link, 0, sp, tp);
		}
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_link))
			{
				if (tp != tmp)
				{
					if (!(tp = pathreal(target, P_PATHONLY|P_NOOPAQUE|P_ABSOLUTE, NiL)))
						return(r);
					tp = strncpy(tmp, tp, PATH_MAX);
				}
				fscall(mp, MSG_link, 0, sp, tp);
			}
	}
#endif
	return(r);
}

#else

NoN(link)

#endif
