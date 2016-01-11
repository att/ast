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
 * send n fds to stream fd
 */

#include "cslib.h"

int
cssend(register Cs_t* state, int fd, int* fds, int n)
{

#if CS_LIB_STREAM || CS_LIB_V10

	register int	i;
	struct csfdhdr	hdr;

	if (n > 0)
	{
#if CS_LIB_STREAM
		struct strbuf	dat;
#endif
		csprotect(&cs);
		hdr.count = n;
		hdr.pid = getpid();
#if CS_LIB_STREAM
		dat.buf = "";
		dat.len = 0;
		if (putmsg(fd, NiL, &dat, 0) < 0 || write(fd, &hdr, sizeof(hdr)) != sizeof(hdr))
#else
		if (write(fd, &hdr, sizeof(hdr)) != sizeof(hdr))
#endif
		{
			messagef((state->id, NiL, -1, "send: %d: hdr write error", fd));
			return -1;
		}
		for (i = 0; i < n; i++)
			if (ioctl(fd, I_SENDFD, FDARG(fds[i])))
			{
				messagef((state->id, NiL, -1, "send: %d: ioctl I_SENDFD error", fd));
				return -1;
			}
	}
	return 0;

#else

#if CS_LIB_SOCKET_RIGHTS

	register char*	s;
	register int	i;
	struct iovec	iov;
	Csid_t		id;
	struct msghdr	msg;
	struct
	{
#if _mem_msg_control_msghdr
	struct cmsghdr	hdr;
#else
#define msg_control	msg_accrights
#define msg_controllen	msg_accrightslen
#endif
	int		fds[OPEN_MAX + 1];
	}		ctl;
	char		tmp[PATH_MAX + 1];

	csprotect(&cs);
	if (n >= elementsof(ctl.fds))
	{
		errno = EINVAL;
		return -1;
	}

	/*
	 * the first fd is to verify uid,gid in csrecv()
	 */

	s = csvar(state, CS_VAR_LOCAL, 0);
	if (eaccess(s, X_OK) && (mkdir(s, S_IRWXU|S_IRWXG|S_IRWXO) || chmod(s, S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)))
	{
		messagef((state->id, NiL, -1, "send: %d: %s: invalid authentication directory ", fd, s));
		return -1;
	}
	if (!(s = pathtemp(tmp, sizeof(tmp), s, "cs", 0)))
	{
		messagef((state->id, NiL, -1, "send: %d: authentication tmp file error", fd));
		return -1;
	}
	if ((i = open(s, O_WRONLY|O_CREAT|O_TRUNC, CS_AUTH_MODE)) < 0 || chmod(s, CS_AUTH_MODE) < 0)
	{
		messagef((state->id, NiL, -1, "send: %d: %s: authentication file creat error", fd, s));
		return -1;
	}
	if (remove(s))
	{
		messagef((state->id, NiL, -1, "send: %d: %s: authentication file remove error", fd, s));
		close(i);
		errno = EPERM;
		return -1;
	}
	ctl.fds[0] = i;
	for (i = 0; i < n; i++)
		ctl.fds[i + 1] = fds[i];
	id.hid = 0;
	id.pid = getpid();
	id.uid = geteuid();
	id.gid = getegid();
	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	iov.iov_base = (caddr_t)&id;
	iov.iov_len = sizeof(id);
	msg.msg_control = (caddr_t)&ctl;
	msg.msg_controllen = (char*)&ctl.fds[n + 1] - (char*)&ctl;
#if _mem_msg_control_msghdr
	ctl.hdr.cmsg_len = msg.msg_controllen;
	ctl.hdr.cmsg_level = SOL_SOCKET;
	ctl.hdr.cmsg_type = SCM_RIGHTS;
#endif
#ifdef EMSGSIZE
	n = 0;
	while ((i = sendmsg(fd, &msg, 0) < 0 ? -1 : 0) < 0 && errno == EMSGSIZE && n++ < 5)
		sleep(1);
#else
	i = sendmsg(fd, &msg, 0) < 0 ? -1 : 0;
#endif
	if (i) messagef((state->id, NiL, -1, "send: %d: sendmsg error", fd));
	close(ctl.fds[0]);
	return i;

#else

	messagef((state->id, NiL, -8, "send(%d,%d) call", fd, n));
	if (!access(CS_PROC_FD_TST, F_OK))
	{
		register int	i;
		register int	j;
		register char*	s;

		static struct
		{
			int*	fds;
			int	num;
			int	max;
		}		hold;

		/*
		 * the sent fd's must be kept open until
		 * the other side receives them, so we dup
		 * and hold them open until the next cssend()
		 */

		if (hold.fds)
			for (i = 0; i < hold.num; i++)
				if (hold.fds[i] >= 0)
				{
					close(hold.fds[i]);
					hold.fds[i] = 0;
				}
		if (n > hold.max)
		{
			hold.max = roundof(n, 8);
			hold.fds = newof(hold.fds, int, hold.max, 0);
		}
		s = state->temp;
		s += sfsprintf(s, sizeof(state->temp), "%lu %d", getpid(), n);
		if (hold.fds)
		{
			hold.num = n;
			for (i = 0; i < n; i++)
			{
				if ((j = hold.fds[i] = dup(fds[i])) < 0)
					j = fds[i];
				else fcntl(j, F_SETFD, FD_CLOEXEC);
				s += sfsprintf(s, sizeof(state->temp) - (s - state->temp), " %d", j);
			}
		}
		else for (i = 0; i < n; i++)
			s += sfsprintf(s, sizeof(state->temp) - (s - state->temp), " %d", fds[i]);
		s += sfsprintf(s, sizeof(state->temp) - (s - state->temp), "\n");
		n = s - state->temp;
		messagef((state->id, NiL, -8, "send(%d, %-*.*s) call", fd, n - 1, n - 1, state->temp));
		i = send(fd, state->temp, n, 0) == n ? 0 : -1;
		if (i) messagef((state->id, NiL, -1, "send: %d: write error", fd));
		return i;
	}
	errno = EINVAL;
	messagef((state->id, NiL, -1, "send: %d: not supported", fd));
	return -1;

#endif

#endif

}

int
_cs_send(int fd, int* fds, int n)
{
	return cssend(&cs, fd, fds, n);
}
