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
__STDPP__directive pragma pp:hide mknod
#else
#define mknod        ______mknod
#endif
#define _def_syscall_3d	1
#define _LS_H		1

#include "3d.h"

#undef	_def_syscall_3d
#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide mknod
#else
#undef  mknod
#endif

#include "FEATURE/syscall"

#ifdef mknod3d

int
mknod3d(const char* path, mode_t mode, dev_t dev)
{
 	register char*	sp;
#if FS
	Mount_t*	mp;

	if (!fscall(NiL, MSG_mknod, 0, path, mode, dev))
		return(state.ret);
	mp = monitored();
#endif
 	if (!(sp = pathreal(path, P_PATHONLY, NiL)) || MKNOD(sp, mode, dev))
		return(-1);
#if FS
 	if (mp && !STAT(sp, &state.path.st))
 		fscall(mp, MSG_mknod, 0, path, state.path.st.st_mode, dev);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, MSG_mknod))
			fscall(mp, MSG_mknod, 0, path, mode, dev);
#endif
	return(0);
}

#else

NoN(mknod)

#endif
