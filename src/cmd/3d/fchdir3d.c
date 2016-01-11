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

#if defined(fchdir3d)

int
fchdir3d(int fd)
{
	Dir_t*		dp;
	struct stat	st;

	if (fd >= 0 && fd < elementsof(state.file) && (dp = state.file[fd].dir) && !FSTAT(fd, &st) && dp->dev == st.st_dev && dp->ino == st.st_ino)
		chdir(dp->path);
	else if (FCHDIR(fd))
		return -1;
	return 0;
}

#else

NoP(fchdir)

#endif
