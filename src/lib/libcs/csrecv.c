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
 * receive up to n fds from stream fd
 * number of fds received is returned
 *
 * NOTE: cssend() only supported for local host
 *       for local host Csid_t.hid==0, Csid_t.pid not trusted
 *	 for remote host csauth() can authenticate Csid_t.[ug]id
 */

#include "cslib.h"

#if CS_LIB_SOCKET_RIGHTS && !CS_LIB_STREAM

static int
sockrecv(int fd, Csid_t* id, int* fds, int n)
{
	struct iovec		iov;
	struct msghdr		msg;
#if _mem_msg_control_msghdr
	struct
	{
	struct cmsghdr		hdr;
	int			fds[OPEN_MAX + 1];
	}			ctl;
#else
#define msg_control	msg_accrights
#define msg_controllen	msg_accrightslen
#endif

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	iov.iov_base = (caddr_t)id;
	iov.iov_len = sizeof(*id);
#if _mem_msg_control_msghdr
	msg.msg_control = (caddr_t)&ctl;
	msg.msg_controllen = (char*)&ctl.fds[n] - (char*)&ctl;
	ctl.hdr.cmsg_len = msg.msg_controllen;
	ctl.hdr.cmsg_level = SOL_SOCKET;
	ctl.hdr.cmsg_type = SCM_RIGHTS;
#else
	msg.msg_control = (caddr_t)fds;
	msg.msg_controllen = n * sizeof(fds[0]);
#endif
	if (recvmsg(fd, &msg, 0) < 0) return -1;
	id->hid = id->hid;
	id->pid = id->pid;
	id->uid = id->uid;
	id->gid = id->gid;
#if _mem_msg_control_msghdr
	if (ctl.hdr.cmsg_level != SOL_SOCKET || ctl.hdr.cmsg_type != SCM_RIGHTS)
		return -1;
	n = (ctl.hdr.cmsg_len - sizeof(ctl.hdr)) / sizeof(fds[0]);
	for (fd = 0; fd < n; fd++)
		fds[fd] = ctl.fds[fd];
	return n;
#else
	return msg.msg_controllen / sizeof(fds[0]);
#endif
}

#endif

int
csrecv(register Cs_t* state, int fd, Csid_t* id, int* fds, int n)
{
	int			oerrno;
	Csid_t			ignore;

#if CS_LIB_SOCKET

	Sock_size_t		namlen;
	struct sockaddr_in	nam;

#if CS_LIB_SOCKET_UN && !CS_LIB_STREAM

#if CS_LIB_SOCKET_RIGHTS
	int			rfd[OPEN_MAX + 1];
#endif
	struct stat		st;

#endif

#endif

#if CS_LIB_STREAM || CS_LIB_V10

	int			m;
	struct strrecvfd	rcv;
	struct csfdhdr		hdr;

#if CS_LIB_V10

	struct tcpuser		tcp;

#endif

#endif

#if CS_LIB_STREAM || CS_LIB_V10 || CS_LIB_SOCKET_RIGHTS

	register int		i;

#endif

	messagef((state->id, NiL, -8, "recv(%d,%d) call", fd, n));
	if (n < 1)
	{
		errno = EINVAL;
		return -1;
	}
	if (!id) id = &ignore;
	memzero(id, sizeof(*id));
	oerrno = errno;
	csprotect(&cs);

#if CS_LIB_V10

	if (!ioctl(fd, TCPGETADDR, &tcp))
	{
		if ((fds[0] = tcp_accept(fd, &tcp)) < 0)
		{
			messagef((state->id, NiL, -1, "recv: %d: tcp accept error", fd));
			return -1;
		}
		id->hid = tcp.faddr;
		return 1;
	}
	messagef((state->id, NiL, -1, "recv: %d: ioctl TCPGETADDR error", fd));

#else

#if CS_LIB_SOCKET

	namlen = sizeof(nam);
	if ((fds[0] = accept(fd, (struct sockaddr*)&nam, &namlen)) >= 0)
	{

#if CS_LIB_SOCKET_UN && !CS_LIB_STREAM

#if defined(__linux__) && defined(AF_UNSPEC)
		if (nam.sin_family == AF_UNSPEC)
			nam.sin_family = AF_UNIX;
#endif
		if (nam.sin_family == AF_UNIX)
		{
#if CS_LIB_SOCKET_RIGHTS
			if (write(fds[0], "", 1) != 1)
			{
				messagef((state->id, NiL, -1, "recv: %d: ping write error", fd));
				close(fds[0]);
				return -1;
			}
			if (sockrecv(fds[0], id, rfd, 1) != 1)
			{
				messagef((state->id, NiL, -1, "recv: %d: sockrecv error", fd));
				goto eperm;
			}
			if (fstat(rfd[0], &st))
			{
				messagef((state->id, NiL, -1, "recv: %d: %d: authentication stat error", fd, rfd[0]));
			drop:
				close(rfd[0]);
			eperm:
				close(fds[0]);
				errno = EPERM;
				return -1;
			}
			if ((st.st_mode & CS_AUTH_MASK) != CS_AUTH_MODE)
			{
				messagef((state->id, NiL, -1, "recv: %d: %d: invalid authentication mode %04o", fd, rfd[0], st.st_mode & CS_AUTH_MASK));
				goto drop;
			}
			close(rfd[0]);
#else
			if (fstat(fds[0], &st))
			{
				st.st_uid = geteuid();
				st.st_gid = getegid();
			}
#endif
			id->uid = st.st_uid;
			id->gid = st.st_gid;
		}
		else

#endif

		if (nam.sin_family == AF_INET)
			id->hid = (unsigned long)nam.sin_addr.s_addr;
		return 1;
	}
	messagef((state->id, NiL, -1, "recv: %d: accept error", fd));

#endif

#endif

#if CS_LIB_STREAM || CS_LIB_V10

	if (ioctl(fd, I_RECVFD, &rcv) < 0)
	{
		messagef((state->id, NiL, -1, "recv: %d: ioctl I_RECVFD error", fd));
		switch (errno)
		{
		case EIO:
#ifdef EBADMSG
		case EBADMSG:
#endif
			break;
		default:
			return -1;
		}
#if CS_LIB_STREAM
		if (!(m = read(fd, &hdr, sizeof(hdr))))
#endif
		m = read(fd, &hdr, sizeof(hdr));
		if (m != sizeof(hdr))
		{
			errno = EINVAL;
			messagef((state->id, NiL, -1, "recv: %d: hdr read error", fd));
			return -1;
		}
		if (hdr.count <= 0)
		{
			errno = EINVAL;
			messagef((state->id, NiL, -1, "recv: %d: invalid hdr count %d", fd, hdr.count));
			return -1;
		}
		for (i = 0; i < hdr.count; i++)
		{
			if (ioctl(fd, I_RECVFD, &rcv) < 0)
			{
				messagef((state->id, NiL, -1, "recv: %d: ioctl I_RECVFD #%d error", fd, i + 1));
				while (--i >= 0) close(fds[i]);
				return -1;
			}
			fds[i] = rcv.fd;
		}
		id->pid = hdr.pid;
	}

#ifdef I_ACCEPT

	else if (ioctl(rcv.fd, I_ACCEPT, NiL) < 0)
	{
		messagef((state->id, NiL, -1, "recv: %d: ioctl I_ACCEPT error", fd));
		close(rcv.fd);
		return -1;
	}

#endif

	else
	{

		i = 1;
		fds[0] = rcv.fd;
	}
	id->uid = rcv.uid;
	id->gid = rcv.gid;
	errno = oerrno;
	return i;

#else

#if CS_LIB_SOCKET_RIGHTS

	if ((i = sockrecv(fd, id, rfd, n + 1)) <= 1)
	{
		messagef((state->id, NiL, -1, "recv: %d: sockrecv error", fd));
	nope:
		if (i >= 0)
		{
			errno = EPERM;
			while (--i >= 0) close(rfd[i]);
		}
		return -1;
	}
	if (fstat(rfd[0], &st))
	{
		messagef((state->id, NiL, -1, "recv: %d: %d: authentication stat error", fd, rfd[0]));
		goto nope;
	}
	if ((st.st_mode & CS_AUTH_MASK) != CS_AUTH_MODE)
	{
		messagef((state->id, NiL, -1, "recv: %d: %d: invalid authentication mode %04o", fd, rfd[0], st.st_mode & CS_AUTH_MASK));
		goto nope;
	}
	close(rfd[0]);
	for (n = --i; i > 0; i--) fds[i - 1] = rfd[i];
	id->uid = st.st_uid;
	id->gid = st.st_gid;
	return n;

#else

	if (!access(CS_PROC_FD_TST, F_OK))
	{
		register int	i;
		register int	j;
		char*		s;
		unsigned long	pid;
		struct stat	st;

		s = state->temp;
		if ((i = recv(fd, s, sizeof(state->temp), 0)) <= 0)
		{
			messagef((state->id, NiL, -1, "recv: %d: read error", fd));
			return -1;
		}
		if (i >= sizeof(state->temp))
			i = sizeof(state->temp) - 1;
		s[i] = 0;
		messagef((state->id, NiL, -8, "recv: `%s'", s));
		pid = strtol(s, &s, 0);
		i = strtol(s, &s, 0);
		if (i < n)
			n = i;
		for (i = 0; i < n; i++)
			fds[i] = strtol(s, &s, 0);
		s = state->temp;
		for (i = j = 0; i < n; i++)
		{
			sfsprintf(s, sizeof(state->temp), CS_PROC_FD_FMT, pid, fds[i]);
			if (!stat(s, &st) && (fds[i] = open(s, (st.st_mode & (S_IRUSR|S_IWUSR)) == (S_IRUSR|S_IWUSR) ? O_RDWR : (st.st_mode & S_IWUSR) ? O_WRONLY : O_RDONLY)) >= 0)
				j++;
		}
		if (id)
		{
			id->hid = 0;
			id->pid = pid;
			if (!stat(CS_PROC_FD_TST, &st))
			{
				id->uid = st.st_uid;
				id->gid = st.st_gid;
			}
			else
			{
				id->uid = geteuid();
				id->gid = getegid();
			}
		}
		messagef((state->id, NiL, -8, "recv: %d fds", j));
		return j;
	}
	errno = EINVAL;
	return -1;

#endif

#endif

}

int
_cs_recv(int fd, Csid_t* id, int* fds, int n)
{
	return csrecv(&cs, fd, id, fds, n);
}
