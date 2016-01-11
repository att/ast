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

#ifdef pipe3d

int
pipe3d(int* fds)
{
	int		r;
#if !_mangle_syscall
	int		fd;
#endif

	initialize();
#if !_mangle_syscall
	if (r = (state.fs[FS_option].flags & FS_ON) != 0) state.fs[FS_option].flags &= ~FS_ON;
	for (fd = 0; fd < OPEN_MAX; fd++)
		if ((fds[0] = DUP(fd)) >= 0)
		{
			fds[1] = DUP(fds[0]);
			CLOSE(fds[1]);
			CLOSE(fds[0]);
			break;
		}
	if (r) state.fs[FS_option].flags |= FS_ON;
#endif
	r = PIPE(fds);
#if FS
	if (!r && !state.in_2d)
	{
		Mount_t*	mp;

		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_pipe))
				fscall(mp, MSG_pipe, 0, fds);
	}
#endif
	return(r);
}

#else

NoN(pipe)

#endif
