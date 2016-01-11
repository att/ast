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

#include <ast.h>

#include "FEATURE/lib"

#include <sys/types.h>

#if _hdr_poll
#include <poll.h>
#endif

#if _sys_socket && _mem_msg_control_msghdr
#include <sys/socket.h>
#ifdef SCM_RIGHTS
#undef	SCM_RIGHTS
#else
#define SCM_RIGHTS	1
#endif
#endif

int
main()
{
	register int	m;
	register int	i;

	sfprintf(sfstdout, "#define CS_REMOTE_SHELL	\"%s\"\n", CS_REMOTE_SHELL);
	sfprintf(sfstdout, "\n");
#if CS_LIB_SOCKET || CS_LIB_V10
	sfprintf(sfstdout, "#define CS_TCP	1\n");
	sfprintf(sfstdout, "#define CS_UDP	1\n");
#endif
#if CS_LIB_SOCKET_UN || CS_LIB_STREAM
	sfprintf(sfstdout, "#define CS_FDP	1\n");
#endif
	sfprintf(sfstdout, "\n");
#if _hdr_poll
#ifdef _nam_revents
	sfprintf(sfstdout, "#define %s status\n", _nam_revents_str);
#else
	sfprintf(sfstdout, "#define revents	status\n");
#endif
	sfprintf(sfstdout, "#include <poll.h>\n");
#ifdef _nam_revents
	sfprintf(sfstdout, "#undef	%s\n", _nam_revents_str);
#else
	sfprintf(sfstdout, "#undef	revents\n");
#endif
	sfprintf(sfstdout, "\n");
	sfprintf(sfstdout, "#define CS_POLL_CLOSE	POLLHUP\n");
	sfprintf(sfstdout, "#define CS_POLL_CONTROL	POLLPRI\n");
	sfprintf(sfstdout, "#define CS_POLL_ERROR	POLLERR\n");
	sfprintf(sfstdout, "#define CS_POLL_INVALID	POLLNVAL\n");
	sfprintf(sfstdout, "#define CS_POLL_READ	POLLIN\n");
	sfprintf(sfstdout, "#define CS_POLL_WRITE	POLLOUT\n");
	sfprintf(sfstdout, "\n");
	m = 0;
#ifdef POLLIN
	m |= POLLIN;
#endif
#ifdef POLLOUT
	m |= POLLOUT;
#endif
#ifdef POLLPRI
	m |= POLLPRI;
#endif
#ifdef POLLWRNORM
	m |= POLLWRNORM;
#endif
#ifdef POLLRDNORM
	m |= POLLRDNORM;
#endif
#ifdef POLLRDBAND
	m |= POLLRDBAND;
#endif
#ifdef POLLWRBAND
	m |= POLLWRBAND;
#endif
#ifdef POLLMSG
	m |= POLLMSG;
#endif
#ifdef POLLSYNC
	m |= POLLSYNC;
#endif
#ifdef POLLNVAL
	m |= POLLNVAL;
#endif
#ifdef POLLERR
	m |= POLLERR;
#endif
#ifdef POLLHUP
	m |= POLLHUP;
#endif
#ifdef POLLNORM
	m |= POLLNORM;
#endif
	sfprintf(sfstdout, "#define CS_POLL_PSEUDO	(CS_POLL_AUTH|CS_POLL_CONNECT|CS_POLL_USER|CS_POLL_BEFORE)\n");
	for (i = 0; m & (1<<i); i++);
	sfprintf(sfstdout, "#define CS_POLL_AUTH	(1<<%d)\n", i);
	for (i++; m & (1<<i); i++);
	sfprintf(sfstdout, "#define CS_POLL_CONNECT	(1<<%d)\n", i);
	for (i++; m & (1<<i); i++);
	sfprintf(sfstdout, "#define CS_POLL_USER	(1<<%d)\n", i);
	for (i++; m & (1<<i); i++);
	sfprintf(sfstdout, "#define CS_POLL_BEFORE	(1<<%d)\n", i);
	sfprintf(sfstdout, "\n");
	sfprintf(sfstdout, "typedef struct pollfd Cspoll_t;\n");
#else
	sfprintf(sfstdout, "#define CS_POLL_CLOSE	(1<<0)\n");
	sfprintf(sfstdout, "#define CS_POLL_CONTROL	(1<<1)\n");
	sfprintf(sfstdout, "#define CS_POLL_ERROR	(1<<2)\n");
	sfprintf(sfstdout, "#define CS_POLL_INVALID	(1<<3)\n");
	sfprintf(sfstdout, "#define CS_POLL_READ	(1<<4)\n");
	sfprintf(sfstdout, "#define CS_POLL_WRITE	(1<<5)\n");
	sfprintf(sfstdout, "\n");
	sfprintf(sfstdout, "#define CS_POLL_PSEUDO	(CS_POLL_AUTH|CS_POLL_CONNECT|CS_POLL_USER|CS_POLL_BEFORE)\n");
	sfprintf(sfstdout, "#define CS_POLL_AUTH	(1<<6)\n");
	sfprintf(sfstdout, "#define CS_POLL_CONNECT	(1<<7)\n");
	sfprintf(sfstdout, "#define CS_POLL_USER	(1<<8)\n");
	sfprintf(sfstdout, "#define CS_POLL_BEFORE	(1<<9)\n");
	sfprintf(sfstdout, "\n");
	sfprintf(sfstdout, "typedef struct\n");
	sfprintf(sfstdout, "{\n");
	sfprintf(sfstdout, "	short		fd;\n");
	sfprintf(sfstdout, "	unsigned char	events;\n");
	sfprintf(sfstdout, "	unsigned char	status;\n");
	sfprintf(sfstdout, "} Cspoll_t;\n");
#endif
#ifdef SCM_RIGHTS
	sfprintf(sfstdout, "\n#define SCM_RIGHTS	 %d\n", SCM_RIGHTS);
#endif
	exit(0);
}
