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
/*
 * Glenn Fowler
 * AT&T Research
 *
 * write datagram
 */

#include "cslib.h"

ssize_t
csto(register Cs_t* state, int fd, const void* buf, size_t siz, Csaddr_t* addr)
{

#if CS_LIB_V10

	struct udpaddr	udp;

	udp.host = addr->addr[0];
	udp.port = addr->addr[1];
	if (cswrite(state, fd, &udp, sizeof(udp)) != sizeof(udp))
	{
		messagef((state->id, NiL, -1, "to: %d: hdr write error", fd));
		return -1;
	}
	return cswrite(state, fd, buf, siz);

#else

#if CS_LIB_SOCKET

	struct sockaddr_in	nam;

	memzero(&nam, sizeof(nam));
	nam.sin_family = AF_INET;
	nam.sin_addr.s_addr = addr->addr[0];
	nam.sin_port = addr->addr[1];
	return sendto(fd, buf, siz, 0, (struct sockaddr*)&nam, sizeof(nam));

#else

	errno = EINVAL;
	messagef((state->id, NiL, -1, "to: %d: not supported", fd));
	return -1;

#endif

#endif

}

ssize_t
_cs_to(int fd, const void* buf, size_t siz, Csaddr_t* addr)
{
	return csto(&cs, fd, buf, siz, addr);
}
