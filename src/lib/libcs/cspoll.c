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
 * poll for read/write/control on fds with ms timeout
 * number of selected fd's returned
 */

#include "cslib.h"

#ifdef POLLIST
#undef	CS_LIB_SOCKET
#undef	CS_LIB_STREAM
#undef	CS_LIB_V10
#undef	_sys_select
#define CS_LIB_STREAM	1
#endif

#if _sys_select && !defined(FD_SET)
#include <sys/select.h>
#endif

int
cspoll(Cs_t* state, Cspoll_t* fds, int num, int ms)
{
	register Cspoll_t*	pp;
	register Cspoll_t*	mp;

#if CS_LIB_SOCKET || CS_LIB_V10
	register int		events;
	register int		width;
	fd_set			rd;
	register fd_set*	rp = &rd;
	fd_set			wr;
	register fd_set*	wp = &wr;
#if CS_LIB_SOCKET
	fd_set			ex;
	register fd_set*	ep = &ex;
	struct timeval*		tp;
	struct timeval		tv;

	if (ms >= 0)
	{
		tv.tv_sec = ms / 1000;
		tv.tv_usec = (ms % 1000) * 1000;
		tp = &tv;
	}
	else tp = 0;
	FD_ZERO(ep);
#endif
	FD_ZERO(rp);
	FD_ZERO(wp);
	events = width = 0;
	for (mp = (pp = fds) + num; pp < mp; pp++)
		if (pp->fd >= 0)
		{
			if (pp->fd > width)
				width = pp->fd;
			if (pp->events & CS_POLL_READ)
			{
				events |= CS_POLL_READ;
				FD_SET(pp->fd, rp);
			}
			if (pp->events & CS_POLL_WRITE)
			{
				events |= CS_POLL_WRITE;
				FD_SET(pp->fd, wp);
			}
			if (pp->events & CS_POLL_CONTROL)
			{
				events |= CS_POLL_CONTROL;
#if CS_LIB_SOCKET
				FD_SET(pp->fd, ep);
#endif
			}
		}
	if (!(events & CS_POLL_WRITE))
		wp = 0;
#if CS_LIB_SOCKET
	if (!(events & CS_POLL_READ))
		rp = 0;
	if (!(events & CS_POLL_CONTROL))
		ep = 0;
	messagef((state->id, NiL, -6, "poll: %s num=%d ms=%d sec=%d usec=%d", fmttime("%K", CSTIME()), num, ms, tp ? tp->tv_sec : 0, tp ? tp->tv_usec : 0));
	num = select(width + 1, rp, wp, ep, tp);
#else
	if (!(events & (CS_POLL_READ|CS_POLL_CONTROL)))
		rp = 0;
	messagef((state->id, NiL, -6, "poll: %s num=%d ms=%d", fmttime("%K", CSTIME()), num, ms);
	num = select(width + 1, rp, wp, ms);
#endif
	messagef((state->id, NiL, -6, "poll: %s num=%d", fmttime("%K", CSTIME()), num));
	if (num < 0)
		messagef((state->id, NiL, -1, "poll: select error"));
	else
		for (num = 0, pp = fds; pp < mp; pp++)
		{
			pp->status = 0;
			if (pp->fd >= 0)
			{
				if (rp && FD_ISSET(pp->fd, rp))
				{
#if CS_LIB_V10
					long	n;

					if ((pp->event & CS_POLL_CONTROL) && ioctl(fd, FIONREAD, &n))
						pp->status |= CS_POLL_CLOSE;
					else if (!n)
						pp->status |= CS_POLL_CONTROL;
					else
#endif
					pp->status |= CS_POLL_READ;
				}
				if (wp && FD_ISSET(pp->fd, wp))
					pp->status |= CS_POLL_WRITE;
#if CS_LIB_SOCKET
				if (ep && FD_ISSET(pp->fd, ep))
					pp->status |= CS_POLL_CONTROL;
#endif
				if (pp->status)
				{
					pp->status |= pp->events & (CS_POLL_AUTH|CS_POLL_CONNECT|CS_POLL_USER);
					num++;
				}
			}
		}
	return num;
	
#else

#if CS_LIB_STREAM

	int	n;

#if _lib_poll_fd_2
	n = poll(num, fds, ms);
#else
	n = poll(fds, num, ms);
#endif
	if (n < 0)
		messagef((state->id, NiL, -1, "poll: poll error"));
	else if (n > 0)
	{
		int		i;
#ifdef RS_HIPRI
		struct strbuf	buf;

		buf.maxlen = 0;
#endif
		for (mp = (pp = fds) + num, i = n; pp < mp; pp++)
		{
			if (pp->status)
			{
				pp->status |= pp->events & (CS_POLL_AUTH|CS_POLL_CONNECT|CS_POLL_USER);
#ifdef RS_HIPRI
				if (pp->status & CS_POLL_CONTROL)
				{
					int	f = RS_HIPRI;

					if (getmsg(pp->fd, NiL, &buf, &f))
						pp->status &= ~CS_POLL_CONTROL;
				}
#endif
				if (--i <= 0)
					break;
			}
		}
	}
	return n;

#else

	errno = EINVAL;
	messagef((state->id, NiL, -1, "poll: not supported"));
	return -1;

#endif

#endif

}

int
_cs_poll(Cspoll_t* fds, int num, int ms)
{
	return cspoll(&cs, fds, num, ms);
}
