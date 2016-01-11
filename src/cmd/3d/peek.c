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

#include <cs.h>

/*
 * peek siz chars from fd into buf
 * if not peekable but seekable then chars are peeked at offset 0
 */

ssize_t
peek(int fd, void* buf, size_t siz)
{
	register int	n;

	n = cspeek(&cs, fd, buf, siz);
	if (n >= 0)
		return(n);
	cspeek(&cs, -1, NiL, 0);
	if (!FSTAT(fd, &state.path.st) && state.path.st.st_size <= siz)
	{
		while ((n = read(fd, buf, siz)) != state.path.st.st_size && lseek(fd, 0L, 1) > 0 && lseek(fd, 0L, 0) == 0);
		if (n == state.path.st.st_size || n > 0 && !state.path.st.st_size)
			return(n);
	}
	return(-1);
}
