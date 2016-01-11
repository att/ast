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

/*
 * 3d mounted fs support
 *
 * NOTE: be vewwwy careful with errno
 */

#include "3d.h"

#if FS

#include <cs.h>

#define DEVFD		"/dev/fd"
#define MNTFD		"path\n"
#define MNTNAM		"/dev/tcp/*"

/*
 * initialize mount channel and return fs fd for mp
 */

int
fschannel(register Mount_t* mp)
{
	register Fs_t*	fs;
	register int	fd;

	if (mp->channel == -1)
		return -1;
	fs = mp->fs;
	if (mp->channel && fs->fd)
		return fs->fd;
	if ((fd = fsfd(fs)) <= 0)
		return -1;
	if (mp->channel)
		return fs->fd;
	mp->channel = 1;
	mp->flags |= MOUNT_PRIMARY;
	return fd;
}

/*
 * generate phony monitor open for fd inherited from parent
 */

static void
fsphony(register Mount_t* mp, register File_t* fp, int fd)
{
	register ssize_t	n;
	int			nd;
	Mount_t*		np;
	char			buf[64];

	state.kernel++;
	fp->flags |= FILE_LOCK;
	*buf = 0;
	if (!(np = getmount(MNTNAM, NiL)) || (nd = fsfd(np->fs)) < 0)
		sfsprintf(buf, sizeof(buf), "%s/%d", DEVFD, fd);
	else if (!(np->fs->flags & FS_LOCK) && WRITE(nd, MNTFD, sizeof(MNTFD) - 1) == sizeof(MNTFD) - 1)
	{
		np->fs->flags |= FS_LOCK;
		if (!cssend(&cs, nd, &fd, 1) && (n = READ(nd, buf, sizeof(buf))) > 1)
			buf[n - 1] = 0;
		np->fs->flags &= ~FS_LOCK;
	}
	state.kernel--;
	if (*buf)
	{
		fp->open |= (1<<(mp-state.mount));
		message((-3, "fs: phony: %s", buf));
		fscall(mp, MSG_open, fd, buf, fp->oflag, 0, 0);
	}
	fp->flags &= ~FILE_LOCK;
}

/*
 * return real fd for up under mount mp to be accessed by call
 * return value stuck in tail portion of state.path.name
 */

char*
fsreal(register Mount_t* mp, long call, const char* up)
{
	char		dev[2 * PATH_MAX + 3];
	register char*	s = dev;
	register Fs_t*	fs = mp->fs;
	register int	n;
	register int	fd;
	int		lz;
	int		pz;

	if (!(lz = mp->logicalsize))
		lz = strlen(mp->logical);
	if (!(pz = mp->physicalsize) && mp->physical)
		pz = strlen(mp->physical);
	n = sfsprintf(s, sizeof(dev) - 1, "%s %-*s %s%s%-*s pwd=%s%s%s", msgname(call), lz, mp->logical, up, pz ? " physical=" : "", pz, mp->physical ? mp->physical : "", state.pwd, mp->fs->attr, mp->attr);
	s[n] = 0;
	message((-2, "fs: %s: real: service=%-*s request=\"%s\"", fs->special, fs->servicesize, fs->service, s));
	s[n++] = '\n';
	if ((fd = fsfd(fs)) <= 0)
		return 0;
	s = state.path.name + PATH_MAX;
	if (write(fd, dev, n) != n || (n = csread(&cs, fd, s, PATH_MAX, CS_LINE)) <= 0)
	{
		fsdrop(fs, 0);
		return 0;
	}
	if (n <= 1)
		return 0;
	s[n - 1] = 0;
	if (s[0] >= 'A' && s[0] <= 'Z' && s[1] == ' ')
	{
		message((-2, "fs: %s: real: %s", fs->special, s + 2));
		return 0;
	}
	message((-2, "fs: %s: real: path=%s", fs->special, s));
	return s;
}

/*
 * do fs call
 * -1 returned if not mounted
 * 0 returned if mounted with state.ret call return value
 * state.path.monitor set if monitor or name service mount
 */

int
fscall(register Mount_t* mp, long call, int ret, ...)
{
	register Fs_t*		fs;
	register int		retry;
	register int		tries;
	const char*		up;
	const char*		sp;
	int			cd;
	int			fd;
	int			oerrno;
	int			m;
	long			n;
	File_t*			fp;
	Handler_t		handler;
	Msg_return_t*		rp;
	Msg_return_t		rv;
	void**			xp;
	void*			xv[2];
	int*			ip;
	Msg_file_t		iv[2];
	va_list			ap;

	oerrno = errno;
	initialize();
	state.ret = -1;
	if (state.in_2d)
		return -1;
	up = 0;
	/* proto workaround */
	va_start(ap, ret); va_end(ap);
	if (!mp)
	{
		state.path.monitor = 0;
		state.path.mount = 0;
		sp = 0;
		va_start(ap, ret);
		switch (MSG_ARG(call, 1))
		{
		case MSG_ARG_file:
			fd = va_arg(ap, int);
			if (fd < 0 || fd >= elementsof(state.file))
				goto nope;
			if (!state.kernel && (mp = state.global) && !((fp = &state.file[fd])->flags & FILE_LOCK)) do
			{
				if (fssys(mp, MSG_open) && ((fp->flags & FILE_OPEN) || !fileinit(fd, NiL, NiL, 0)))
				{
					fs = mp->fs;
					if ((!(fs->flags & FS_REGULAR) || (fp->flags & FILE_REGULAR)) && (!(fs->flags & FS_WRITE) || (fp->flags & FILE_WRITE)) && ((mp->flags & MOUNT_PRIMARY) || fd > 2) && !(fp->open & (1<<(mp-state.mount))))
						fsphony(mp, fp, fd);
				}
			} while (mp = mp->global);
			mp = fgetmount(fd);
			n = FS_ERROR|FS_INIT|FS_LOCK|FS_NAME|FS_ON;
			break;
		case MSG_ARG_string:
			sp = va_arg(ap, const char*);
			if (sp != state.path.name)
				sp = pathreal(sp, P_PATHONLY|P_ABSOLUTE, NiL);
			if (sp) mp = getmount(sp, &up);
			n = FS_ERROR|FS_INIT|FS_LOCK|FS_ON;
			break;
		}
		va_end(ap);
		if (!mp || ((fs = mp->fs)->flags & n) != FS_ON)
			goto nope;
		if (fs->flags & FS_MONITOR)
		{
			if (!state.kernel && (fs->call & MSG_MASK(call)))
			{
				if (sp && fs->match)
				{
					if (fs->matchsize)
					{
						cd = fs->match[fs->matchsize];
						fs->match[fs->matchsize] = 0;
					}
					if (strmatch(sp, fs->match))
						state.path.monitor = mp;
					if (fs->matchsize)
						fs->match[fs->matchsize] = cd;
				}
				else state.path.monitor = mp;
			}
			goto nope;
		}
	}
	else if (((fs = mp->fs)->flags & (FS_ERROR|FS_INIT|FS_LOCK|FS_ON)) != FS_ON)
		goto nope;
	if (!(fs->call & MSG_MASK(call)))
		goto nope;
	if (fs->flags & FS_MONITOR)
	{
		if (state.kernel)
			goto nope;
		if (MSG_ARG(call, 1) == MSG_ARG_file)
		{
			va_start(ap, ret);
			fd = va_arg(ap, int);
			va_end(ap);
			if (fd < 0 || fd >= elementsof(state.file))
				goto nope;
			fp = &state.file[fd];
			if (fp->flags & FILE_LOCK)
				goto nope;
			if (!(fp->flags & FILE_OPEN) && fileinit(fd, NiL, NiL, 0))
				goto nope;
			if ((fs->flags & FS_REGULAR) && !(fp->flags & FILE_REGULAR) || (fs->flags & FS_WRITE) && !(fp->flags & FILE_WRITE))
				goto nope;
			if (fssys(mp, MSG_open) && ((mp->flags & MOUNT_PRIMARY) || fd > 2) && !(fp->open & (1<<(mp-state.mount))))
				fsphony(mp, fp, fd);
		}
		else if (call == MSG_open)
		{
			if (fsmount(mp) < 0)
				goto nope;
			if (fs->flags & FS_WRITE)
			{
				va_start(ap, ret);
				va_arg(ap, const char*);
				n = va_arg(ap, int);
				va_end(ap);
				if ((n & O_ACCMODE) == O_RDONLY)
					goto nope;
			}
		}
		if (MSG_ARG(call, 0) == MSG_ARG_file)
		{
			fp = &state.file[ret];
			fp->open |= (1<<(mp-state.mount));
			rv.file = fp->id;
		}
		else rv.number = ret;
		rp = &rv;
	}
	else if (call == MSG_open)
	{
		int	oflag;
		int	mode;
		int	level;

		fs->flags |= FS_LOCK;
		va_start(ap, ret);
		sp = va_arg(ap, const char*);
		oflag = va_arg(ap, int);
		mode = va_arg(ap, int);
		level = va_arg(ap, int);
		va_end(ap);
		message((-3, "fs: %s: open: path=%s", fs->special, sp));
		if (fs == &state.fs[FS_fd])
		{
			const char*	ep;

			if ((fd = OPEN(sp, oflag, mode)) >= 0)
			{
				state.ret = fd;
				goto unlock;
			}
			fd = strtol(up, (char**)&ep, 0);
			if (*ep)
			{
				oerrno = ENOENT;
				goto unlock;
			}
			if ((n = FCNTL(fd, F_GETFL, 0)) < 0)
			{
				oerrno = errno;
				goto unlock;
			}
			n &= O_ACCMODE;
			oflag &= O_ACCMODE;
			if (n == O_RDONLY && oflag == O_WRONLY || n == O_WRONLY && oflag == O_RDONLY)
			{
				oerrno = EPERM;
				goto unlock;
			}
			if ((state.ret = FCNTL(fd, F_DUPFD, 0)) < 0)
				oerrno = errno;
		}
		else if ((sp = (const char*)fsreal(mp, MSG_open, up)) && (fd = fsfd(mp->fs)) > 0)
		{
			/*
			 * /#<id>/[#]<path> for active fd's
			 * /#<id>\n written back to initialize
			 */

			if (sp[0] == '/' && sp[1] == '#')
			{
				up = sp;
				if (!(sp = strchr(sp + 2, '/')))
				{
					sp = up;
					up = 0;
				}
				else
				{
					m = sp - up;
					if (sp[1] == '#')
						sp += 2;
				}
			}
			else up = 0;
			if (streq(sp, DEVFD))
			{
				cd = -1;
				while (csrecv(&cs, fd, NiL, &cd, 1) != 1 && errno == EINTR);
				fd = cd;
			}
			else if ((fd = fs3d_open(sp, oflag, mode)) == -1)
				oerrno = errno;
			if (fd >= 0 && up)
			{
				*((char*)up + m++) = '\n';
				if (write(fd, up, m) != m)
					fd = -1;
				else
				{
					if (fd > state.cache)
						state.cache = fd;
					fp = &state.file[fd];
					fp->open = ~0;
					fp->flags = FILE_OPEN;
					fp->mount = mp;
				}
			}
			state.ret = fd;
		}
		goto unlock;
	}
	else
		rp = 0;
	if (fs->flags & FS_NAME)
	{
		if (up && fs != &state.fs[FS_fd])
		{
			state.path.monitor = mp;
			state.path.mount = (char*)up;
		}
		goto nope;
	}
	if (MSG_MASK(call) & (MSG_MASK(MSG_close)|MSG_MASK(MSG_dup)))
		goto nope;
	message((-3, "fs: %s: %s: call", fs->special, msgname(call)));
	fs->flags |= FS_LOCK;
	if (fs->terse & MSG_MASK(call))
	{
		tries = MSG_ARG_CALL;
		for (;;)
		{
			tries += MSG_ARG_TYPE;
			switch ((call >> tries) & ((1 << MSG_ARG_TYPE) - 1))
			{
			case 0:
				break;
			case MSG_ARG_output:
				if (!(fs->flags & FS_MONITOR)) break;
				/*FALLTHROUGH*/
			case MSG_ARG_input:
			case MSG_ARG_vector:
				call = (call & ~(((1 << MSG_ARG_TYPE) - 1) << tries)) | (MSG_ARG_number << tries);
				continue;
			default:
				continue;
			}
			break;
		}
	}
	if (fs->flags & FS_ACTIVE)
	{
		if (!(fs->flags & FS_MONITOR))
			call |= MSG_RETURN;
		else if (fs->ack & MSG_MASK(call))
			call |= (fs->flags & FS_INTERACTIVE) ? MSG_RETURN : MSG_ACK;
		retry = fs->retry;
	}
	else retry = 0;
	if ((fs->flags & FS_FLUSH) && (call |= MSG_FLUSH) || retry)
		handler = signal(SIGPIPE, SIG_IGN);
	tries = 1;
	for (;;)
	{
		if ((cd = fsmount(mp)) < 0)
		{
			message((-2, "fs: %s: %s: connect error on try %d", fs->special, msgname(call), tries));
			goto unlock;
		}
		va_start(ap, ret);
		xp = xv;
		switch (MSG_ARG(call, 1))
		{
		case MSG_ARG_file:
			fd = va_arg(ap, int);
			if (!(fs->flags & FS_MONITOR))
				cd = fd;
			*xp++ = (void*)&state.file[fd].id;
			break;
		case MSG_ARG_string:
			sp = va_arg(ap, const char*);
			if (MSG_VAR(call) == MSG_VAR_FILE)
			{
				if (!(fs->flags & FS_MONITOR))
					sp = up;
				else if (sp != state.path.name && !(sp = (const char*)pathreal(sp, P_PATHONLY|P_ABSOLUTE, NiL)))
					goto unlock;
			}
			*xp++ = (void*)sp;
			break;
		case MSG_ARG_output:
			if (call == MSG_pipe)
			{
				ip = va_arg(ap, int*);
				for (n = 0; n < 2; n++)
				{
					fp = &state.file[ip[n]];
					if (!(fp->flags & FILE_OPEN))
						fileinit(ip[n], NiL, NiL, 0);
					fp->open |= (1<<(mp-state.mount));
					iv[n] = fp->id;
					*xp++ = (void*)iv;
				}
			}
			break;
		default:
			xp = 0;
			break;
		}
		if (xp)
		{
			*xp = 0;
			xp = xv;
		}
		n = msgvcall(cd, MSG_CHANNEL(state.pid, mp->channel), call, rp, xp, ap);
		va_end(ap);
		if (n != -1)
			break;
		if (errno != EMSGIO)
		{
			if (!(fs->flags & FS_MONITOR))
				oerrno = errno;
			break;
		}
		message((-2, "fs: %s: %s: error on try %d", fs->special, msgname(call), tries));
		if (tries++ > retry)
			break;
		fsdrop(fs, 0);
	}
	if ((fs->flags & FS_FLUSH) || retry)
		signal(SIGPIPE, handler);
	if (fs->flags & FS_ACTIVE)
	{
		if ((state.ret = n) > 0 && (fs->flags & (FS_ACTIVE|FS_INTERACTIVE|FS_MONITOR)) == (FS_ACTIVE|FS_INTERACTIVE|FS_MONITOR) && (fs->ack & MSG_MASK(call)))
		{
			char	buf[TABLE_MAX];

			if ((n = READ(cd, buf, sizeof(buf))) > 1)
			{
				buf[n - 1] = 0;
				mapinit(buf, 0);
			}
			else message((-3, "fs: %s: %s: interactive ack failed", fs->special, msgname(call)));
		}
	}
	else if (!(fs->flags & FS_MONITOR))
	{
		oerrno = errno = ENODEV;
		message((-3, "fs: %s: %s: return: passive fs", fs->special, msgname(call)));
	}
	else if (fs->ack & MSG_MASK(call))
	{
		oerrno = errno = ENODEV;
		message((-3, "fs: %s: %s: ack: passive fs", fs->special, msgname(call)));
	}
	else state.ret = 0;
 unlock:
	fs->flags &= ~FS_LOCK;
	errno = oerrno;
	return 0;
 nope:
	errno = oerrno;
	return -1;
}

#endif

/*
 * initialize mounted fs and return device service fd
 */

int
fsinit(register Fs_t* fs, int fd)
{
	int	n;
	int	oerrno;

	if (fd < 0 && (fs->flags & (FS_BOUND|FS_ERROR|FS_INIT|FS_ON)) != (FS_BOUND|FS_ON) || state.kernel && (fs->flags & FS_GLOBAL))
		return -1;
	oerrno = errno;
	fs->flags |= FS_INIT;
	if ((fs->flags & (FS_ON|FS_OPEN)) != (FS_ON|FS_OPEN))
	{
		state.kernel++;
		if ((fs->fd = fd) < 0)
		{
			char*	svc;
			char	buf[PATH_MAX];

			if (n = fs->servicesize)
			{
				if (n >= sizeof(buf)) n = sizeof(buf) - 1;
				svc = (char*)memcpy(buf, fs->service, n);
				svc[n] = 0;
			}
			else svc = fs->service;
			message((-3, "fs: %s: init#1: service=%s", fs->special, svc));
#if FS
			fs->fd = cslocal(&cs, svc);
			message((-3, "fs: %s: init#2: service=%s cslocal=%d", fs->special, svc, fs->fd));
			if (fs->fd >= 0)
			{
				if (fs->flags & FS_RECEIVE)
				{
					n = csrecv(&cs, fs->fd, NiL, &fd, 1);
					CLOSE(fs->fd);
					fs->fd = n == 1 ? fd : -1;
				}
			}
			else if (errno == ENOENT)
#endif
			fs->fd = fs3d_open(svc, O_CREAT|O_RDWR|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
		}
		if (fs->fd < 0 || FSTAT(fs->fd, &fs->st))
		{
			fs->fd = -1;
			fs->flags |= FS_ERROR;
		}
		else
		{
			if (fs->flags & FS_CLOSE)
				FCNTL(fs->fd, F_SETFD, FD_CLOEXEC);
			if (S_ISREG(fs->st.st_mode))
				fs->flags &= ~FS_ACTIVE;
			fs->flags |= FS_ON|FS_OPEN;
			reserve(&fs->fd);
			if (fd < 0)
				message((-3, "fs: %s: init#3: service=%-*s fd=%d cache=%d", fs->special, fs->servicesize ? fs->servicesize : strlen(fs->service), fs->service, fs->fd, state.cache));
		}
		state.kernel--;
	}
	fs->flags &= ~FS_INIT;
	errno = oerrno;
	return fs->fd;
}

/*
 * drop internal 3d mount
 * if clear!=0 then path binding also cleared
 */

void
fsdrop(register Fs_t* fs, int clear)
{
	int	oerrno;

	state.kernel++;
	oerrno = errno;
	message((-3, "fs: %s: drop:%s", fs->special, clear ? " clear" : state.null));
	if (fs->flags & FS_OPEN)
	{
		fs->flags &= ~FS_OPEN;
		cancel(&fs->fd);
	}
	fs->flags &= ~FS_ERROR;
	if (clear)
	{
#if FS
		if (fs->flags & FS_FS)
		{
			register int		n;
			register int		m;
			register Mount_t*	mp;

			for (n = 0; n <= state.cache; n++)
				if ((mp = state.file[n].mount) && mp->fs == fs)
					state.file[n].mount = 0;
			for (n = m = 0; n < state.vmount.size; n++)
			{
				if (((Mount_t*)state.vmount.table[n].val)->fs != fs)
				{
					if (n != m)
						state.vmount.table[m] = state.vmount.table[n];
					m++;
				}
				else if (state.vmount.table[n].valsize & T_ALLOCATE)
					free(state.vmount.table[n].key);
			}
			state.vmount.size = m;
			for (n = 0; n < elementsof(state.mount); n++)
				if (state.mount[n].fs == fs)
				{
					state.mount[n].fs = 0;
					if (state.mount[n].physical && !state.mount[n].physicalsize)
					{
						free(state.mount[n].physical);
						state.mount[n].physical = 0;
					}
				}
		}
#endif
		if (fs->flags & FS_INTERNAL) fs->flags &= ~(FS_BOUND|FS_ON);
		else
		{
			fs->flags = 0;
			if (fs->service && !fs->servicesize)
			{
				free(fs->service);
				fs->service = 0;
			}
		}
	}
	errno = oerrno;
	state.kernel--;
}
