/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "cslib.h"

#if _socket_peek && !CS_LIB_SOCKET
#include <sys/socket.h>
#endif

#if _stream_peek && !CS_LIB_STREAM
#include <stropts.h>
#endif

/*
 * peek up to siz chars from fd into buf
 * -1 returned if fd not peekable
 *
 * NOTE: cspeek() caches the peek state of fd
 *	 call cspeek(-1,...) to flush the cache
 */

ssize_t
cspeek(register Cs_t* state, int fd, void* buf, size_t siz)
{
	register int	n;
#if _stream_peek
	struct strpeek	pk;
#endif

	if (fd == -1)
	{
#if _stream_peek
		state->nostream = -1;
#endif
#if _socket_peek
		state->nosocket = -1;
#endif
		return -1;
	}
#if _stream_peek
	if (fd != state->nostream)
	{
		pk.flags = 0;
		pk.ctlbuf.maxlen = -1;
		pk.ctlbuf.len = 0;
		pk.ctlbuf.buf = 0;
		pk.databuf.maxlen = siz;
		pk.databuf.buf = (char*)buf;
		pk.databuf.len = 0;
		if ((n = ioctl(fd, I_PEEK, &pk)) >= 0)
		{
			state->nostream = -1;
			if (n > 0) n = pk.databuf.len;
			return n;
		}
		messagef((state->id, NiL, -1, "peek: %d: ioctl I_PEEK error", fd));
		state->nostream = fd;
	}
#endif
#if _socket_peek
	if (fd != state->nosocket)
	{
		if ((n = recv(fd, (char*)buf, siz, MSG_PEEK)) >= 0)
		{
			state->nosocket = -1;
			return n;
		}
		messagef((state->id, NiL, -1, "peek: %d: recv MSG_PEEK error", fd));
		state->nosocket = fd;
	}
#endif
	messagef((state->id, NiL, -1, "peek: %d: no peek", fd));
	return -1;
}

ssize_t
_cs_peek(int fd, void* buf, size_t siz)
{
	return cspeek(&cs, fd, buf, siz);
}
