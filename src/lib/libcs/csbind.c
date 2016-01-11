/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2013 AT&T Intellectual Property          *
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
 * internet address/port binding
 */

#include "cslib.h"

/*
 * low level for csbind()
 */

static int
portbind(register Cs_t* state, const char* type, unsigned long addr, unsigned int port)
{

#if CS_LIB_V10

	int		fd;
	struct tcpuser	tcp;

	if (streq(type, "udp"))
	{
		if (!addr)
		{
			if ((fd = udp_datagram(port)) < 0)
				messagef((state->id, NiL, -1, "bind: %s: udp_datagram error", type));
			return fd;
		}
		if (addr == CS_LOCAL) addr = 0;
		if ((fd = udp_connect(0, addr, port)) < 0)
			messagef((state->id, NiL, -1, "bind: %s: udp_connect error", type));
		return fd;
	}
	else if (streq(type, "tcp"))
	{
		if ((fd = tcp_sock()) < 0)
		{
			messagef((state->id, NiL, -1, "bind: %s: tcp_sock error", type));
			return -1;
		}
		memzero(&tcp, sizeof(tcp));
		tcp.fport = port;
		if (addr != CS_LOCAL) tcp.faddr = addr;
		if (addr ? !tcp_connect(fd, &tcp) : !tcp_listen(fd, &tcp))
		{
			state->addr = tcp.laddr;
			state->port = tcp.lport;
			return fd;
		}
		messagef((state->id, NiL, -1, "bind: %s %s error", type, addr ? "tcp_connect" : "tcp_listen"));
		close(fd);
	}

#else

#if CS_LIB_SOCKET

	int			fd;
	int			sock;
	int			r;
	struct sockaddr_in	nam;
	Sock_size_t		namlen = sizeof(nam);

	if (streq(type, "tcp")) sock = SOCK_STREAM;
	else if (streq(type, "udp")) sock = SOCK_DGRAM;
	else
	{
		messagef((state->id, NiL, -1, "bind: %s: invalid type", type));
		return -1;
	}
	if ((fd = socket(AF_INET, sock, 0)) < 0)
	{
		messagef((state->id, NiL, -1, "bind: %s: AF_INET socket error", type));
		return -1;
	}
	memzero(&nam, sizeof(nam));
	nam.sin_family = AF_INET;
	if (addr != CS_LOCAL) nam.sin_addr.s_addr = addr;
	state->addr = addr;
	state->port = port;
	nam.sin_port = htons(port);
	if (addr)
	{
#if defined(O_NONBLOCK) || defined(FNDELAY)
		int	fl;
#endif
#if defined(IPPROTO_TCP) && defined(TCP_NODELAY)
		int	sl;
#endif
		if (state->flags & CS_ADDR_NOW)
		{
#if defined(O_NONBLOCK) || defined(FNDELAY)
			if ((fl = fcntl(fd, F_GETFL, 0)) != -1)
#if defined(FNDELAY)
				fcntl(fd, F_SETFL, fl|FNDELAY);
#else
				fcntl(fd, F_SETFL, fl|O_NONBLOCK);
#endif
#endif
#if defined(IPPROTO_TCP) && defined(TCP_NODELAY)
			if (sock == SOCK_STREAM)
			{
				sl = 1;
				setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&sl, sizeof(sl));
			}
#endif
		}
#undef	connect
		if (!(r = connect(fd, (struct sockaddr*)&nam, sizeof(nam)))
#ifdef EINPROGRESS
		    || errno == EINPROGRESS
#endif
			)
		{
			if (!r && (state->flags & CS_ADDR_NOW))
			{
#if defined(O_NONBLOCK) || defined(FNDELAY)
				if (fl != -1)
					fcntl(fd, F_SETFL, fl);
#endif
#if defined(IPPROTO_TCP) && defined(TCP_NODELAY)
				if (sock == SOCK_STREAM)
				{
					sl = 0;
					setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&sl, sizeof(sl));
				}
#endif
			}
			state->addr = nam.sin_addr.s_addr;
			state->port = port;
			return fd;
		}
		messagef((state->id, NiL, -1, "bind: %s: connect error", type));
		if (errno == EADDRNOTAVAIL || errno == ECONNREFUSED)
			errno = ENOENT;
	}
	else
	{
#ifdef SO_REUSEADDR
		if (state->port != CS_PORT_NORMAL)
		{
			int	n = 1;

			setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&n, sizeof(n));
		}
#endif
		if (!bind(fd, (struct sockaddr*)&nam, sizeof(nam)) && (sock != SOCK_STREAM || !listen(fd, 32)))
		{
			if (!getsockname(fd, (struct sockaddr*)&nam, &namlen) && namlen == sizeof(nam))
			{
				state->addr = nam.sin_addr.s_addr;
				state->port = ntohs((unsigned short)nam.sin_port);
			}
			else messagef((state->id, NiL, -1, "bind: %s: getsockname error", type));
			return fd;
		}
		messagef((state->id, NiL, -1, "bind: %s: bind error", type));
		if (errno == EADDRINUSE) errno = EEXIST;
	}
	close(fd);

#else

	errno = ENOTDIR;
	messagef((state->id, NiL, -1, "bind: %s: not supported", type));

#endif

#endif

	return -1;
}

/*
 * create [addr==0] or open stream fd for <type,addr,port>
 * for create
 *
 *	addr	CS_LOCAL for local address
 *
 *	port	CS_PORT_NORMAL for normal port allocation
 *		CS_PORT_RESERVED for reserved port allocation
 */

int
csbind(register Cs_t* state, const char* type, unsigned long addr, unsigned long port, unsigned long clone)
{
	int	fd;

	messagef((state->id, NiL, -8, "bind(%s,%s,%u,%lu) call", type, csntoa(state, addr), port, clone));
	if (port == CS_PORT_INVALID)
		return -1;
	if (port == CS_PORT_RESERVED)
	{
		static unsigned int	last = IPPORT_RESERVED;

		if (addr)
		{
			errno = EROFS;
			messagef((state->id, NiL, -1, "bind: %s: explicit reserved port invalid", csntoa(state, addr)));
			return -1;
		}
		port = last;
		do
		{
			if (--last <= IPPORT_RESERVED / 2)
				port = IPPORT_RESERVED - 1;
			if (last == port)
			{
				errno = ENOSPC;
				messagef((state->id, NiL, -1, "bind: no more reserved ports"));
				break;
			}
			if ((fd = portbind(state, type, 0L, last)) >= 0)
				goto ok;
		} while (errno != EACCES);
		messagef((state->id, NiL, -1, "bind: reserved port allocation error"));
		return -1;
	}
	if (port == CS_PORT_NORMAL && addr)
	{
		errno = EROFS;
		return -1;
	}
	if ((fd = portbind(state, type, addr, port)) < 0)
		return -1;
	if (clone)
	{
		int	n;
		char	buf[16];

		n = sfsprintf(buf, sizeof(buf), "%d %lu\n", CS_KEY_CLONE, clone);
		cswrite(state, fd, buf, n);
	}
 ok:
	messagef((state->id, NiL, -8, "bind(%s,%s,%u,%lu) = %d", type, csntoa(state, addr), port, clone, fd));
	return fd;
}

int
_cs_bind(const char* type, unsigned long addr, unsigned long port, unsigned long clone)
{
	return csbind(&cs, type, addr, port, clone);
}
