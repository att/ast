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

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide chmod
#else
#define chmod        ______chmod
#endif
#define _def_syscall_3d	1
#define _LS_H		1

#include "3d.h"

#undef	_def_syscall_3d
#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide chmod
#else
#undef  chmod
#endif

#include "FEATURE/syscall"

int
chmod3d(const char* path, mode_t mode)
{
	register char*	sp;
	register int	r;

#if FS
	register Mount_t*	mp;

	if (!fscall(NiL, MSG_chmod, 0, path, mode))
		return(state.ret);
	mp = monitored();
#endif
	if (!(sp = pathreal(path, state.safe ? (P_SAFE|P_TOP) : P_TOP, NiL)))
		return(-1);
	r = CHMOD(sp, mode);
#if FS
	if (!r)
	{
		if (mp)
			fscall(mp, MSG_chmod, 0, state.path.name, mode);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_chmod))
				fscall(mp, MSG_chmod, 0, state.path.name, mode);
	}
#endif
	return(r);
}
