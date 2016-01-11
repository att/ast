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

#ifdef	fcntl3d

#include <stdarg.h>

int
fcntl3d(int fd, int op, ...)
{
	register int	r;
	int		n;
	void*		arg;
	va_list		ap;

	initialize();
	va_start(ap, op);
	arg = va_arg(ap, void*);
	va_end(ap);
	n = (int)integralof(arg);
	if (op == F_DUPFD && state.file[n].reserved)
		close(n);
	r = FCNTL(fd, op, arg);
#if FS
	if (r >= 0 && r < elementsof(state.file))
		switch (op)
		{
		case F_DUPFD:
			fs3d_dup(fd, r);
			break;
		case F_SETFD:
			if (state.cache)
			{
				if (!(n & FD_CLOEXEC))
					state.file[fd].flags &= ~FILE_CLOEXEC;
				else if (!(state.file[fd].flags & FILE_OPEN))
					fileinit(fd, NiL, NiL, 1);
				else
					state.file[fd].flags |= FILE_CLOEXEC;
			}
			break;
		}
#endif
	return r;
}

#else

NoN(fcntl)

#endif
