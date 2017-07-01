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
 * syscall message user side call and return
 *
 * NOTE: errno and the st_mode bits are not yet canonical
 */

#include "msglib.h"

#include <cs.h>

static char	msg_buf[MSG_SIZE_BUF];

/*
 * ap version of msgcall()
 * if xp!=0 then it replaces the first MSG_ARG_file or MSG_ARG_string args
 * message size is returned
 */

ssize_t
msgvcall(int fd, unsigned long channel, unsigned long call, Msg_return_t* ret, void** xp, va_list ap)
{
	register long		at;
	register long		n;
	register char*		p;
	unsigned long		rtime;
	int			i;
	int			nv;
	ssize_t			r;
	Msg_file_t*		ip;
	char*			b;
	char*			e;
	char**			vp;
	void**			oxp;
	int*			zp;
	long*			np;
	struct dirent*		dp;
	struct dirent*		de;
	struct statvfs*		fp;
	struct stat*		sp;
	Cspoll_t		ack;
	va_list			oap;

	static struct
	{
	unsigned long		addr;
	unsigned long		port;
	short			fd;
	}			msg_ack;
	static unsigned long	msg_stamp;
	static int		msg_write;

	oxp = xp;
	va_copy(oap, ap);
	if (MSG_CALL(call) == MSG_write && !msg_write && (MSG_PURE(call) == MSG_PURE(MSG_write) || MSG_PURE(call) == (MSG_write|MSG_EXT(MSG_ARG_number, 4))))
	{
		size_t		c;
		ssize_t		z;
		ssize_t		t;
		off_t		o;

		/*
		 * long MSG_write messages may be split up
		 */

		msg_write = 1;
		ip = xp ? (Msg_file_t*)(*xp) : va_arg(ap, Msg_file_t*);
		b = va_arg(ap, char*);
		c = va_arg(ap, size_t);
		o = MSG_ARG(call, 4) == MSG_ARG_number ? va_arg(ap, off_t) : (off_t)0;
		r = 0;
		while (c > 0)
		{
			z = c > MSG_SIZE_IO ? MSG_SIZE_IO : c;
			if ((t = msgcall(fd, channel, call, ret, ip, b, z, o)) == -1)
			{
				r = -1;
				break;
			}
			c -= z;
			b += z;
			o += z;
			r += t;
		}
		msg_write = 0;
		return r;
	}
	b = msg_buf + MSG_SIZE_SIZE;
	e = msg_buf + sizeof(msg_buf);
	if (ret) call |= MSG_VALUE;
	msgputu(&b, e, MSG_VERSION);
	msgputu(&b, e, channel);
	msgputu(&b, e, call);
	msgputu(&b, e, msg_info.timestamp ? (msg_stamp = CSTIME()) : ++msg_stamp);
	if (call & MSG_ACK)
	{
		if (msg_ack.fd < 0)
			return -1;
		if (!msg_ack.fd)
		{
			if ((msg_ack.fd = csbind(&cs, "udp", 0, CS_PORT_NORMAL, 0L)) < 0)
				return -1;
			fcntl(msg_ack.fd, F_SETFD, FD_CLOEXEC);
			msgreserve(&msg_ack.fd);
			msg_ack.addr = cs.addr;
			msg_ack.port = cs.port;
		}
		msgputu(&b, e, msg_ack.addr);
		msgputu(&b, e, msg_ack.port);
	}
	nv = 0;
	at = call >> MSG_ARG_CALL;
	if (call & MSG_VALUE) switch (at & ((1 << MSG_ARG_TYPE) - 1))
	{
	case 0:
		break;
	case MSG_ARG_file:
		if (ret)
		{
			ip = &ret->file;
			for (n = 0; n < sizeof(*ip) / sizeof(ip->fid[0]); n++)
				msgputu(&b, e, ip->fid[n]);
		}
		else for (n = 0; n < sizeof(*ip) / sizeof(ip->fid[0]); n++)
			msgputu(&b, e, 0);
		break;
	default:
		msgputu(&b, e, ret ? ret->number : 0);
		break;
	}
	for (;;)
	{
		switch ((at >>= MSG_ARG_TYPE) & ((1 << MSG_ARG_TYPE) - 1))
		{
		case MSG_ARG_array:
			n = va_arg(ap, int);
			msgputu(&b, e, n);
			np = va_arg(ap, long*);
			while (n-- > 0)
				msgputu(&b, e, *np++);
			continue;
		case MSG_ARG_file:
			ip = (xp && *xp) ? (Msg_file_t*)(*xp++) : va_arg(ap, Msg_file_t*);
			for (n = 0; n < sizeof(*ip) / sizeof(ip->fid[0]); n++)
				msgputu(&b, e, ip->fid[n]);
			continue;
		case MSG_ARG_input:
			p = va_arg(ap, char*);
			n = va_arg(ap, long);
			msgputz(&b, e, p, n);
			at >>= MSG_ARG_TYPE;
			continue;
		case MSG_ARG_number:
			n = va_arg(ap, long);
			msgputu(&b, e, n);
			continue;
		case MSG_ARG_output:
			if (call & MSG_VALUE) switch (MSG_CALL(call))
			{
			case MSG_CALL(MSG_getdents):
				dp = va_arg(ap, struct dirent*);
				n = va_arg(ap, size_t);
				if (ret) n = ret->number;
				at >>= MSG_ARG_TYPE;
				de = (struct dirent*)((char*)dp + n);
				while (dp < de)
				{
					i = D_NAMLEN(dp);
					msgputz(&b, e, dp->d_name, i + 1);
					msgputu(&b, e, D_FILENO(dp));
#if _mem_d_reclen_dirent
					i = dp->d_reclen;
#else
					i = D_RECSIZ(dp, i);
#endif
					dp = (struct dirent*)((char*)dp + i);
				}
				msgputu(&b, e, 0);
				msgputu(&b, e, n);
				break;
			case MSG_CALL(MSG_pipe):
				ip = (xp && *xp) ? (Msg_file_t*)(*xp++) : va_arg(ap, Msg_file_t*);
				for (i = 0; i < 2; i++)
				{
					for (n = 0; n < sizeof(*ip) / sizeof(ip->fid[0]); n++)
						msgputu(&b, e, ip->fid[n]);
					ip++;
				}
				break;
			case MSG_CALL(MSG_stat):
				sp = va_arg(ap, struct stat*);
				msgputu(&b, e, sp->st_dev);
				msgputu(&b, e, sp->st_ino);
				msgputu(&b, e, sp->st_mode);
				msgputu(&b, e, sp->st_nlink);
				msgputu(&b, e, sp->st_uid);
				msgputu(&b, e, sp->st_gid);
				msgputu(&b, e, sp->st_size);
				msgputu(&b, e, sp->st_atime);
				msgputu(&b, e, sp->st_mtime);
				msgputu(&b, e, sp->st_ctime);
#if _mem_st_blksize_stat
				msgputu(&b, e, sp->st_blksize);
#else
				msgputu(&b, e, 1024);
#endif
#if _mem_st_blocks_stat
				msgputu(&b, e, sp->st_blocks);
#else
				msgputu(&b, e, sp->st_size ? ((sp->st_size - 1) / 1024 + 1) : 0);
#endif
				break;
			case MSG_CALL(MSG_statfs):
				fp = va_arg(ap, struct statvfs*);
				msgputu(&b, e, fp->f_bsize);
				msgputu(&b, e, fp->f_frsize);
				msgputu(&b, e, fp->f_blocks);
				msgputu(&b, e, fp->f_bfree);
				msgputu(&b, e, fp->f_bavail);
				msgputu(&b, e, fp->f_files);
				msgputu(&b, e, fp->f_ffree);
				msgputu(&b, e, fp->f_favail);
#if _mem_f_fsid_statvfs
				msgputu(&b, e, fp->f_fsid);
#else
				msgputu(&b, e, 0);
#endif
#if _mem_f_basetype_statvfs
				msgputz(&b, e, fp->f_basetype, strlen(fp->f_basetype) + 1);
#else
				msgputz(&b, e, "ufs", 4);
#endif
				msgputu(&b, e, fp->f_flag);
				msgputu(&b, e, fp->f_namemax);
#if _mem_f_fstr_statvfs
				msgputz(&b, e, fp->f_fstr, strlen(fp->f_fstr) + 1);
#endif
				break;
			default:
				p = va_arg(ap, char*);
				if (((at >>= MSG_ARG_TYPE) & ((1 << MSG_ARG_TYPE) - 1)) == MSG_ARG_output)
					n = (zp = va_arg(ap, int*)) ? *zp : 0;
				else n = va_arg(ap, size_t);
				msgputz(&b, e, p, n);
				break;
			}
			else va_arg(ap, char*);
			continue;
		case MSG_ARG_string:
			p = (xp && *xp) ? (char*)(*xp++) : va_arg(ap, char*);
			if (p) msgputz(&b, e, p, strlen(p) + 1);
			else msgputu(&b, e, 0);
			continue;
		case MSG_ARG_vector:
			if (vp = va_arg(ap, char**))
			{
				int		m;
				static char*	coexport;

				if (nv++ && !coexport && !(coexport = getenv("COEXPORT")))
					coexport = "";
				n = 0;
				while ((p = *vp++) && (n += (m = strlen(p)) + 4) < MSG_SIZE_IO / 2)
				{
					if (nv > 1)
					{
						char*	tx;
						char*	tp;

						tx = coexport;
					again:
						tp = p;
						while (*tp == *tx && *tx)
						{
							tp++;
							tx++;
						}
						if (*tp != '=' || *tx != ':' && *tx)
						{
							while (*tx)
								if (*tx++ == ':')
									goto again;
							continue;
						}
					}
					msgputz(&b, e, p, m + 1);
				}
			}
			msgputz(&b, e, NiL, 0);
			continue;
		}
		break;
	}
	n = b - msg_buf;
	msgsetsize(msg_buf, n);
	if (cswrite(&cs, fd, msg_buf, n) != n)
	{
		errno = EMSGIO;
		return -1;
	}
	r = n;
	if (call & MSG_FLUSH)
	{
		msgsetsize(msg_buf, 0);
		if (cswrite(&cs, fd, msg_buf, MSG_SIZE_SIZE) != MSG_SIZE_SIZE)
		{
			errno = EMSGIO;
			return -1;
		}
	}
	if (call & MSG_ACK)
	{
		ack.fd = msg_ack.fd;
		ack.events = CS_POLL_READ;
		for (;;)
		{
			switch (cspoll(&cs, &ack, 1, msg_info.timeout))
			{
			case -1:
				if (errno == EINTR) continue;
				/*FALLTHROUGH*/
			case 0:
				r = -1;
				errno = ENXIO;
				break;
			default:

				if ((rtime = msgvreturn(ack.fd, call, oxp, oap)) == -1)
				{
					r = -1;
					errno = ENXIO;
					break;
				}
				if (rtime == msg_stamp) break;
				if (rtime == ~msg_stamp)
				{
					r = -1;
					errno = EIO;
				}
				break;
			}
			break;
		}
	}
	else if (call & MSG_RETURN)
		r = msgvreturn(fd, call, oxp, oap);
	return r;
}

/*
 * construct a call and send it to fd
 * message size is returned
 */

ssize_t
msgcall(int fd, unsigned long channel, unsigned long call, Msg_return_t* ret, ...)
{
	ssize_t	n;
	va_list	ap;

	va_start(ap, ret);
	n = msgvcall(fd, channel, call, ret, NiL, ap);
	va_end(ap);
	return n;
}

/*
 * ap version of msgreturn
 * if xp!=0 then it replaces the first MSG_ARG_file or MSG_ARG_string args
 */

long
msgvreturn(int fd, unsigned long call, void** xp, va_list ap)
{
	register long		n;
	register unsigned long	at;
	int			i;
	long			r;
	char*			b;
	char*			e;
	char*			p;
	Msg_file_t*		ip;
	struct stat*		sp;
	struct dirent*		dp;
	struct statvfs*		vp;

	if ((n = msgread(fd, msg_buf, sizeof(msg_buf))) < 0)
	{
		errno = EMSGIO;
		return -1;
	}
	b = msg_buf + MSG_SIZE_SIZE;
	e = msg_buf + n;
	if (msggetu(&b, e) != call)
	{
		errno = EMSGIO;
		return -1;
	}
	if ((r = msggetu(&b, e)) == -1)
	{
		errno = msggetu(&b, e);
		return -1;
	}
	if (call & MSG_ACK)
		return r;
	at = call >> (MSG_ARG_CALL - MSG_ARG_TYPE);
	for (;;)
	{
		switch ((at >>= MSG_ARG_TYPE) & ((1 << MSG_ARG_TYPE) - 1))
		{
		case MSG_ARG_array:
			va_arg(ap, int);
			va_arg(ap, long*);
			continue;
		case MSG_ARG_file:
			if (xp && *xp) xp++;
			else va_arg(ap, Msg_file_t*);
			continue;
		case MSG_ARG_input:
			va_arg(ap, char*);
			continue;
		case MSG_ARG_number:
			va_arg(ap, long);
			continue;
		case MSG_ARG_output:
			switch (MSG_CALL(call))
			{
			case MSG_CALL(MSG_getdents):
				dp = va_arg(ap, struct dirent*);
				while (n = msggetz(&b, e, dp->d_name, sizeof(dp->d_name)))
				{
					n--;
#if _mem_d_namlen_dirent
					dp->d_namlen = n;
#endif
#if _mem_d_fileno_dirent
					dp->d_fileno =
#endif
					msggetu(&b, e);
#if _mem_d_off_dirent
					dp->d_off = 0;
#endif
					n = D_RECSIZ(dp, n);
#if _mem_d_reclen_dirent
					dp->d_reclen = n;
#endif
					dp = (struct dirent*)((char*)dp + n);
				}
				break;
			case MSG_CALL(MSG_pipe):
				ip = va_arg(ap, Msg_file_t*);
				for (i = 0; i < 2; i++)
				{
					for (n = 0; n < sizeof(*ip) / sizeof(ip->fid[0]); n++)
						ip->fid[n] = msggetu(&b, e);
					ip++;
				}
				break;
			case MSG_CALL(MSG_stat):
				sp = va_arg(ap, struct stat*);
				sp->st_dev = msggetu(&b, e);
				sp->st_ino = msggetu(&b, e);
				sp->st_mode = msggetu(&b, e);
				sp->st_nlink = msggetu(&b, e);
				sp->st_uid = msggetu(&b, e);
				sp->st_gid = msggetu(&b, e);
				sp->st_size = msggetu(&b, e);
				sp->st_atime = msggetu(&b, e);
				sp->st_mtime = msggetu(&b, e);
				sp->st_ctime = msggetu(&b, e);
#if _mem_st_blksize_stat
				sp->st_blksize = msggetu(&b, e);
#else
				msggetu(&b, e);
#endif
#if _mem_st_blocks_stat
				sp->st_blocks = msggetu(&b, e);
#else
				msggetu(&b, e);
#endif
				break;
			case MSG_CALL(MSG_statfs):
				vp = va_arg(ap, struct statvfs*);
				vp->f_bsize = msggetu(&b, e);
				vp->f_frsize = msggetu(&b, e);
				vp->f_blocks = msggetu(&b, e);
				vp->f_bfree = msggetu(&b, e);
				vp->f_bavail = msggetu(&b, e);
				vp->f_files = msggetu(&b, e);
				vp->f_ffree = msggetu(&b, e);
				vp->f_favail = msggetu(&b, e);
#if _mem_f_fsid_statvfs
				vp->f_fsid =
#endif
					msggetu(&b, e);
#if _mem_f_basetype_statvfs
				msggetz(&b, e, vp->f_basetype, sizeof(vp->f_basetype));
#else
				msggetz(&b, e, NiL, 0);
#endif
				vp->f_flag = msggetu(&b, e);
				vp->f_namemax = msggetu(&b, e);
#if _mem_f_fstr_statvfs
				msggetz(&b, e, vp->f_fstr, sizeof(vp->f_fstr));
#endif
				break;
			default:
				p = va_arg(ap, char*);
				if (e - b >= r)
				{
					memcpy(p, b, r);
					b += r;
				}
				break;
			}
			continue;
		case MSG_ARG_string:
			if (xp && *xp) xp++;
			else va_arg(ap, char*);
			continue;
		case MSG_ARG_vector:
			va_arg(ap, char**);
			continue;
		}
		break;
	}
	return r;
}

/*
 * read a return from a previous call on fd
 */

long
msgreturn(int fd, unsigned long call, ...)
{
	long	n;
	va_list	ap;

	va_start(ap, call);
	n = msgvreturn(fd, call, NiL, ap);
	va_end(ap);
	return n;
}
