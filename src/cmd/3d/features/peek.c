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

#include <sys/types.h>

#if _stream_peek
#include <ast_tty.h>
#include <stropts.h>
#endif

#if _socket_peek
#include <sys/socket.h>
#endif

int
main()
{
	int		fds[2];
	static char	msg[] = "test";
#if _stream_peek
	struct strpeek	pk;
#endif

	if (pipe(fds) || write(fds[1], msg, sizeof(msg)) != sizeof(msg))
		return(0);
#if _stream_peek
	pk.flags = 0;
	pk.ctlbuf.maxlen = -1;
	pk.ctlbuf.len = 0;
	pk.ctlbuf.buf = 0;
	pk.databuf.maxlen = sizeof(msg);
	pk.databuf.buf = msg;
	pk.databuf.len = 0;
	if (ioctl(fds[0], I_PEEK, &pk) > 0 && pk.databuf.len == sizeof(msg))
	{
		printf("#undef	_socket_peek\n");
		return(0);
	}
#endif
#if _socket_peek
	if (recv(fds[0], msg, sizeof(msg), MSG_PEEK) == sizeof(msg))
	{
		printf("#undef	_stream_peek\n");
		return(0);
	}
#endif
	return(0);
}
