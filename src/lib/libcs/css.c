/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2012 AT&T Intellectual Property          *
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
 * connect stream server support
 */

#include "csslib.h"

#include <dirent.h>

#define OPEN_MIN	20

#if !defined(O_NONBLOCK) && defined(FNDELAY)
#define O_NONBLOCK	FNDELAY
#endif

static Common_t		state;

static int	signals[] =		/* caught by interrupt()	*/
{
	SIGINT,
	SIGQUIT,
	SIGALRM,
	SIGTERM,
#ifdef SIGCHLD
	SIGCHLD,
#endif
};

/*
 * catch interrupts
 */

static void
interrupt(int sig)
{
#if defined(SIGCHLD) && !_WINIX
	if (sig != SIGCHLD)
#endif
	signal(sig, interrupt);
#ifdef SIGCHLD
	if (cs.interrupt != SIGCHLD)
#endif
	cs.interrupt = sig;
}

/*
 * called on exit
 */

static void
done(void)
{
	cssclose(NiL);
}

/*
 * drop fd ip
 */

static void
drop(Css_t* css, register Cspoll_t* pp)
{
	register Cssfd_t*	ip;
	register int		fd;
	register int		user;

	if (!css)
		css = state.main;
	fd = pp->fd;
	ip = &state.fdinfo[fd];
	if (ip->css == css)
	{
		user = (ip->events & CS_POLL_USER) != 0;
		ip->events = 0;
		if (css->auth)
			state.auth[fd].seq = 0;
		if (ip->actionf)
		{
			ip->status = CS_POLL_CLOSE;
			if ((*ip->actionf)(css, ip, css->disc) > 0)
				fd = -1;
		}
		if (state.fdpolling > 0)
		{
			state.fdpolling--;
			*pp = state.fdpoll[state.fdpolling];
		}
		css->fdpolling--;
		if (ip->events & CS_POLL_CONNECT)
			css->fdlistening--;
		if (user)
			css->fduser--;
		else if (fd >= 0)
			close(fd);
	}
}

/*
 * open the service
 */

Css_t*
cssopen(const char* path, Cssdisc_t* disc)
{
	register Css_t*		css;
	int			type;
	int			n;
	char*			s;
	char*			t;
	struct stat		st;

	if (!state.servers)
	{
		csprotect(&cs);
		if ((n = (int)strtol(astconf("OPEN_MAX", NiL, NiL), NiL, 0)) < OPEN_MIN)
			n = OPEN_MIN;
		if (!(s = newof(0, char, n * (sizeof(Cssfd_t) + sizeof(Cspoll_t) + sizeof(Auth_t)), 0)))
			return 0;
		state.fdpoll = (Cspoll_t*)(s);
		state.fdinfo = (Cssfd_t*)(state.fdpoll + n);
		state.auth = (Auth_t*)(state.fdinfo + n);
		state.fdmax = n;
		state.fdnext = -1;
		state.pid = getpid();
		if (!(css = newof(0, Css_t, 1, sizeof(Cssdisc_t))))
			return 0;
		css->id = cs.id;
		css->disc = (Cssdisc_t*)(css + 1);
		css->disc->version = CSS_VERSION;
		state.servers = state.main = css;
		atexit(done);
	}
	if (!path || (n = strlen(path) + 2) < 2 * CS_NAME_MAX)
		n = 2 * CS_NAME_MAX;
	if (!(css = newof(0, Css_t, 1, n)))
		return 0;
	n = n / 2 + 1;
	css->next = state.servers;
	state.servers = css;
	if (!(css->state = csalloc(NiL)))
		goto bad;
	css->service = (char*)(css + 1);
	css->path = css->service + n;
	css->id = state.main->id;
	css->disc = disc;
	css->fdmax = state.fdmax;
	if (path && strchr(path, '/'))
	{
		strcpy(css->path, path);
		if (tokscan(css->path, NiL, "/dev/%s/%s/%s", &s, NiL, &t) != 3)
		{
			if (css->disc->errorf)
				(*css->disc->errorf)(css, css->disc, 3, "%s: invalid connect stream path", path);
			goto bad;
		}
		type = *s;
		if (s = strchr(t, '/'))
			*s = 0;
		strncpy(css->service, t, n);
		errno = EBADF;
		if ((css->fd = csopen(css->state, path, CS_OPEN_CREATE)) < 0)
		{
			if (css->disc->errorf)
			{
				if (errno == EEXIST)
					(*css->disc->errorf)(css, css->disc, 3, "%s: server already running", path);
				else
					(*css->disc->errorf)(css, css->disc, ERROR_SYSTEM|3, "%s: cannot create connect stream", path);
			}
			goto bad;
		}
		css->disc->flags &= ~CSS_AUTHENTICATE;
	}
	else
	{
		sfsprintf(css->service, n, "/dev/fdp/local/%s", path);
		path = (char*)cspath(css->state, 0, 0);
		type = strmatch(path, "/dev/??p/*/*") ? path[5] : 'f';
		css->perm = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
		if ((css->disc->flags & CSS_AUTHENTICATE) && csopen(css->state, css->service, CS_OPEN_CREATE|CS_OPEN_MOUNT))
		{
			if (css->disc->errorf)
				(*css->disc->errorf)(css, css->disc, ERROR_SYSTEM|3, "%s: cannot create authentication directory", css->service);
			goto bad;
		}
	}
	strcpy(css->path, path);
	if ((css->disc->flags & CSS_DAEMON) && csdaemon(css->state, (css->disc->flags & CSS_PRESERVE) ? ~0L : ((1<<css->fd)|(1<<2))))
	{
		if (css->disc->errorf)
			(*css->disc->errorf)(css, css->disc, 3, "cannot dive into background");
		goto bad;
	}
	if (css->state->control)
	{
		strcpy(css->mount, css->state->mount);
		css->control = strrchr(css->mount, '/') + 1;
		strcpy(css->control + 1, CS_MNT_TAIL);
		s = css->control - 1;
		*s = 0;
		if (stat(css->mount, &st))
		{
			if (css->disc->errorf)
				(*css->disc->errorf)(css, css->disc, 3, "%s: cannot stat service directory", css->mount);
			goto bad;
		}
		if ((css->disc->flags & CSS_DAEMON) && chdir(css->mount))
		{
			if (css->disc->errorf)
				(*css->disc->errorf)(css, css->disc, 3, "%s: cannot set service directory", css->mount);
			goto bad;
		}
		*s = '/';

		/*
		 * get the fid for the global and local authentication dirs
		 * authentication files can only be in these dirs and
		 * must not be symlinks -- symlinks can point to untrusted
		 * places and other dirs may get untrusted filesystems
		 * mounted via the automounter
		 */

		n = 0;
		while (s > css->mount)
			if (*--s == '/' && ++n >= 3)
				break;
		if (n != 3)
		{
			if (css->disc->errorf)
				(*css->disc->errorf)(css, css->disc, 3, "%s: invalid mount directory", css->mount);
			goto bad;
		}
		*s = 0;
		if (lstat(css->mount, &st))
		{
			if (css->disc->errorf)
				(*css->disc->errorf)(css, css->disc, 3, "%s: invalid global authentication directory", css->mount);
			goto bad;
		}
		*s = '/';
		css->fid[0].dev = st.st_dev;
		css->fid[0].ino = st.st_ino;
		s = csvar(css->state, CS_VAR_LOCAL, 0);
		if (lstat(s, &st) && (mkdir(s, S_IRWXU|S_IRWXG|S_IRWXO) || chmod(s, S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO) || lstat(s, &st)))
		{
			if (css->disc->errorf)
				(*css->disc->errorf)(css, css->disc, 3, "%s: invalid local authentication directory", s);
			goto bad;
		}
		css->fid[1].dev = st.st_dev;
		css->fid[1].ino = st.st_ino;
		css->uid = st.st_uid;
		css->gid = st.st_gid;
		if (type == 't' && *(css->control - 2) != CS_MNT_OTHER && *(css->control - 2) != '.' && *(css->control - 2) != '*')
		{
			css->auth = 1;
			state.nauth++;
			CSTIME();
			state.expire = cs.time + KEYEXPIRE;
			css->conkey = state.pid + getppid() + css->uid + st.st_ino;
			TOSS(css->conkey);
			*css->control = CS_MNT_AUTH;
			remove(css->mount);
			css->newkey = css->conkey & KEYMASK;
			if (close(open(css->mount, O_WRONLY|O_CREAT|O_TRUNC, st.st_mode & (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))) ||
			    chmod(css->mount, st.st_mode & (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) ||
			    cschallenge(css->state, css->mount, NiL, &css->newkey))
			{
				if (css->disc->errorf)
					(*css->disc->errorf)(css, css->disc, 3, "%s: cannot create service authentication file", css->mount);
				goto bad;
			}
			chown(css->mount, -1, css->gid);
			css->oldkey = css->newkey;
			for (n = 0; n < (css->newkey & 0xff); n++)
				CSTOSS(css->conkey, n);
			css->challenge = (st.st_mode & S_IXOTH) ? 2 : 0;
		}
		*css->control = CS_MNT_PROCESS;
		remove(css->mount);
		s = css->buf;
		if (type != 'f') s += sfsprintf(s, sizeof(css->buf) - (s - css->buf), "/n/%s", csname(css->state, 0));
		sfsprintf(s, sizeof(css->buf) - (s - css->buf), "/proc/%d", getpid());
		pathsetlink(css->buf, css->mount);
		if (css->disc->flags & CSS_LOG)
		{
			*css->control = CS_MNT_OLDLOG;
			strcpy(css->buf, css->mount);
			*css->control = CS_MNT_LOG;
			rename(css->mount, css->buf);
			close(2);
			open(css->mount, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		}
		*css->control = CS_MNT_AUTH;
		css->perm = st.st_mode & (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
		if (css->disc->flags & CSS_DAEMON)
			umask(~css->perm & (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH));
	}
	if ((css->disc->flags & CSS_INTERRUPT) && !(state.flags & CSS_INTERRUPT))
	{
		state.flags |= CSS_INTERRUPT;
		for (n = 0; n < elementsof(signals); n++)
			if (signal(signals[n], SIG_IGN) != SIG_IGN)
				signal(signals[n], interrupt);
	}
	cssfd(css, css->fd, type == 'u' ? CS_POLL_READ : (CS_POLL_AUTH|CS_POLL_CONNECT|CS_POLL_READ));
	CSTIME();
	return css;
 bad:
	cssclose(css);
	return 0;
}

/*
 * close the service
 */

int
cssclose(register Css_t* css)
{
	register Css_t*		pss;
	register Css_t*		xss;
	register Css_t*		oss;
	register int		i;
	register int		k;

	static char		mnt[] =
	{
		CS_MNT_AUTH,
		CS_MNT_LOCK,
		CS_MNT_PROCESS,
		CS_MNT_STREAM
	};

	pss = 0;
	xss = state.servers;
	while (xss)
	{
		if (xss == css || !css)
		{
			if (xss->disc->exceptf && (xss->disc->flags & CSS_CLOSE))
				(*xss->disc->exceptf)(css, CSS_CLOSE, 0, xss->disc);
			if (state.fdinfo)
			{
				for (i = k = 0; i < state.fdpolling;)
				{
					if (state.fdinfo[state.fdpoll[i].fd].css == xss)
						drop(xss, &state.fdpoll[i]);
					else
						state.fdpoll[k++] = state.fdpoll[i++];
				}
				state.fdpolling = k;
			}
			if (state.pid == getpid() && xss->control)
				for (i = 0; i < elementsof(mnt); i++)
				{
					*xss->control = mnt[i];
					remove(xss->control);
				}
			if (xss->auth)
				state.nauth--;
			oss = xss;
			xss = xss->next;
			if (pss)
				pss->next = oss->next;
			else
				state.servers = oss->next;
			if (oss->state)
				csfree(oss->state);
			free(oss);
			if (css)
				return 0;
		}
		else
		{
			pss = xss;
			xss = xss->next;
		}
	}
	if (css)
		return -1;
	if (!state.servers && state.fdpoll)
	{
		free(state.fdpoll);
		state.fdpoll = 0;
	}
	return 0;
}

/*
 * file descriptor manipulation
 *
 *	CS_POLL_DUP(n)	move fd to n
 *	CS_POLL_MOVE(n)	move fd to n
 *	CS_POLL_PRI(n)	set fd priority (not yet)
 *	CS_POLL_CLOSE	close+delete fd from table
 *	CS_POLL_AUTH|*	authenticate then poll
 *	CS_POLL_*	poll
 *	CS_POLL_USER|	don't close unless CS_POLL_CLOSE
 */

Cssfd_t*
cssfd(register Css_t* css, register int fd, unsigned long op)
{
	register Cssfd_t*	ip;
	register Cspoll_t*	pp;
	register int		n;
	register int		nfd;
	register int		cmd;
	int			flags;
#if DEBUG
	int			ofd = fd;
	unsigned long		oop = op;
#endif

	if (!css)
		css = state.main;
 again:
	if (fd < 0 || fd >= state.fdmax)
		return 0;
	ip = state.fdinfo + fd;
	if (op == 0)
		return ip->events ? ip : (Cssfd_t*)0;
	if (op & CS_POLL_ARG)
	{
		cmd = op & CS_POLL_MASK;
		nfd = (op >> CS_POLL_SHIFT) & CS_POLL_MASK;
		op = nfd == CS_POLL_MASK ? CS_POLL_CLOSE : 0;
	}
	else
		cmd = 0;
	if (op != CS_POLL_CLOSE)
	{
		if (op & CS_POLL_AUTH)
		{
			if (!css->auth)
				op &= ~CS_POLL_AUTH;
		}
		if (op & CS_POLL_CONNECT)
		{
			op &= ~(CS_POLL_CONTROL|CS_POLL_WRITE);
			op |= CS_POLL_READ;
		}
		for (n = 0;; n++)
		{
			if (n >= state.fdpolling)
			{
				if (!(op & CS_POLL_AUTH|CS_POLL_CONNECT|CS_POLL_READ|CS_POLL_WRITE))
					return 0;
				if (fcntl(fd, F_SETFD, FD_CLOEXEC) && errno == EBADF)
					return 0;
				if ((op & (CS_POLL_AUTH|CS_POLL_CONNECT)) == CS_POLL_AUTH)
				{
					state.auth[fd].seq = 1;
					state.auth[fd].expire = cs.time + EXPIRE;
				}
				pp = state.fdpoll + state.fdpolling++;
				pp->fd = fd;
				pp->events = op;
				pp->status = 0;
				ip->css = css;
				ip->fd = fd;
				ip->actionf = css->disc->actionf;
				ip->data = 0;
				ip->events = op;
				ip->set = 0;
				css->fdpolling++;
				if (ip->events & CS_POLL_BEFORE)
					state.fdbefore++;
				if (ip->events & CS_POLL_CONNECT)
					css->fdlistening++;
				if (ip->events & CS_POLL_USER)
					css->fduser++;
#if DEBUG
				goto show;
#else
				return ip;
#endif
			}
			if (state.fdpoll[n].fd == fd)
			{
				switch (cmd)
				{
				case 0:
					if (ip->css != css)
						return 0;
					if (css->fd != fd)
					{
						if (ip->events & CS_POLL_BEFORE)
							state.fdbefore--;
						if (ip->events & CS_POLL_CONNECT)
							css->fdlistening--;
						if (ip->events & CS_POLL_USER)
							css->fduser--;
						state.fdpoll[n].events = ip->events = op;
						if (ip->events & CS_POLL_BEFORE)
							state.fdbefore++;
						if (ip->events & CS_POLL_CONNECT)
							css->fdlistening++;
						if (ip->events & CS_POLL_USER)
							css->fduser++;
						if (op & CS_POLL_WRITE)
						{
#ifdef O_NONBLOCK
							if (!(ip->set & CS_POLL_WRITE))
							{
								ip->set |= CS_POLL_WRITE;
								if ((flags = fcntl(fd, F_GETFL, 0)) >= 0)
								{
									flags |= O_NONBLOCK;
									fcntl(fd, F_SETFL, flags);
									messagef((state.main->id, NiL, -4, "cssfd: %d O_NONBLOCK", fd));
								}
							}
#endif
							messagef((state.main->id, NiL, -4, "cssfd: %d CS_POLL_WRITE", fd));
						}
					}
					break;
				case CS_POLL_DUP:
					/*ip->actionf = 0;*/
					css = ip->css;
					fd = nfd;
					op = state.fdpoll[n].events;
					goto again;
				case CS_POLL_MOVE:
					if (fcntl(nfd, F_SETFD, FD_CLOEXEC) && errno == EBADF)
						return 0;
					state.fdpoll[n].fd = nfd;
					if (fd == css->fd)
						css->fd = nfd;
					state.fdinfo[nfd] = *ip;
					ip = state.fdinfo + nfd;
					ip->fd = nfd;
					break;
				case CS_POLL_PRI:
					break;
				}
#if DEBUG
 show:
				if (error_info.trace <= -9)
				{
					errorf(state.main->id, NiL, -9, "cssfd: pending=%d polling=%d next=%d fd=%d op=0x%08lx", state.fdpending, state.fdpolling, state.fdnext, ofd, oop);
 					for (n = 0; n < state.fdpolling; n++)
						errorf(state.main->id, NiL, -9, "cssfd: index=%d fd=%d events=%06o actionf=%p", n, state.fdpoll[n].fd, state.fdpoll[n].events, state.fdinfo[state.fdpoll[n].fd].actionf);
				}
#endif
				return ip;
			}
		}
	}
	else if (css->fd != fd)
	{
		for (n = 0; n < state.fdpolling; n++)
			if (state.fdpoll[n].fd == fd)
			{
				drop(state.fdinfo[fd].css, &state.fdpoll[n]);
				break;
			}
		if (n >= state.fdpolling)
			close(fd);
#if DEBUG
		ip = 0;
		goto show;
#endif
	}
	return 0;
}

/*
 * poll for the next fd event
 * some events may be pending from the last call
 */

Cssfd_t*
csspoll(unsigned long ms, unsigned long flags)
{
	register Css_t*		css;
	register Cspoll_t*	pp;
	register Cssfd_t*	ip;
	register Auth_t*	ap;
	register int		fd;
	char*			s;
	char*			t;
	int			n;
	int			fdnew;
	int			status;
	int			to;
	int			oerrno;
	int			err;
	int			clrerr;
	int			sig;
	int			clrsig;
	unsigned long		key;
	unsigned long		z;
	unsigned long		timeout;
	Csid_t			id;
	struct stat		st;
	char**			vp;
	char*			va[8];

	if (state.polling && !(flags & CSS_RECURSIVE))
	{
		errno = EBUSY;
		return 0;
	}
	state.polling++;
	if (ms == CS_NEVER)
		timeout = 0;
	else
		timeout = CSTIME() + ((ms + 999) / 1000);
	oerrno = errno;
	for (;;)
	{
		while (state.fdpending <= 0)
		{
			CSTIME();
			if (timeout && timeout >= cs.time)
			{
				errno = oerrno;
				return 0;
			}
			if (state.nauth && cs.time > state.expire)
			{
				state.expire = cs.time + KEYEXPIRE;
				for (css = state.servers; css; css = css->next)
					if (css->auth)
					{
						css->oldkey = css->newkey;
						n = cs.time & 077;
						do TOSS(css->newkey); while (n--);
						css->newkey &= KEYMASK;
						*css->control = CS_MNT_AUTH;
						if (cschallenge(css->state, css->mount, NiL, &css->newkey))
							css->newkey = css->oldkey;
					}
				for (pp = state.fdpoll; pp < state.fdpoll + state.fdpolling;)
				{
					ap = state.auth + pp->fd;
					if (ap->seq && cs.time > ap->expire)
						drop(state.fdinfo[pp->fd].css, pp);
					else
						pp++;
				}
			}
			to = ms;
			for (css = state.servers; css; css = css->next)
			{
				css->fdpending = 0;
				if (css->disc->timeout)
				{
					if (css->timeout_last != css->disc->timeout)
						css->timeout_last = css->timeout_remain = css->disc->timeout;
					if (to > css->timeout_remain)
						to = css->timeout_remain;
				}
				if (css->disc->wakeup)
				{
					if (css->wakeup_last != css->disc->wakeup)
						css->wakeup_last = css->wakeup_remain = css->disc->wakeup;
					if (to > css->wakeup_remain)
						to = css->wakeup_remain;
				}
			}
			if (state.fdbefore)
				for (pp = state.fdpoll; pp < state.fdpoll + state.fdpolling;)
				{
					if (pp->events & CS_POLL_BEFORE)
					{
						ip = state.fdinfo + pp->fd;
						ip->status = CS_POLL_BEFORE;
						if (ip->actionf && (*ip->actionf)(ip->css, ip, ip->css->disc) < 0)
						{
							drop(ip->css, pp);
							continue;
						}
					}
					pp++;
				}
			z = cs.time;
			if ((state.fdpending = cspoll(&cs, state.fdpoll, state.fdpolling, to)) < 0)
			{
				err = errno;
				sig = (err == EINTR) ? cs.interrupt : 0;
				clrsig = 0;
				clrerr = 0;
			}
			else
			{
				err = 0;
				sig = 0;
			}
			state.fdloop = 1;
			CSTIME();
			if (state.fdpending)
				z = (cs.time - z) * 1000;
			else
				z = to;
			if (ms != CS_NEVER)
			{
				if (ms > z)
					ms -= z;
				else
					timeout = cs.time;
			}
			if (state.fdpending > 0)
				for (pp = state.fdpoll; pp < state.fdpoll + state.fdpolling; pp++)
					if (pp->status & (CS_POLL_READ|CS_POLL_WRITE))
						state.fdinfo[pp->fd].css->fdpending++;
			for (css = state.servers; css; css = css->next)
				if (css->disc->exceptf)
				{
					if (css->disc->timeout)
					{
						if (css->fdpending)
							css->timeout_remain = css->disc->timeout;
						else if (css->timeout_remain <= z)
						{
							if ((css->disc->flags & CSS_DORMANT) && css->fdpolling <= (css->fdlistening + css->fduser))
								(*css->disc->exceptf)(css, CSS_DORMANT, 0, css->disc);
							else if (css->disc->flags & CSS_TIMEOUT)
								(*css->disc->exceptf)(css, CSS_TIMEOUT, 0, css->disc);
							css->timeout_last = css->timeout_remain = css->disc->timeout;
						}
						else
							css->timeout_remain -= z;
					}
					if (css->disc->wakeup)
					{
						messagef((state.main->id, NiL, -4, "csspoll: pending=%d polling=%d z=%I*d wakeup=%I*d remain=%I*d", state.fdpending, state.fdpolling, sizeof(z), z, sizeof(css->disc->wakeup), css->disc->wakeup, sizeof(css->wakeup_remain), css->wakeup_remain));
						if (css->wakeup_remain <= z)
						{
							(*css->disc->exceptf)(css, CSS_WAKEUP, 0, css->disc);
							css->wakeup_last = css->wakeup_remain = css->disc->wakeup;
						}
						else
							css->wakeup_remain -= z;
					}
					if (err)
					{
						if ((css->disc->flags & CSS_INTERRUPT) && (sig || err == EINTR))
						{
							if (!sig)
								clrerr = 1;
							else if ((*css->disc->exceptf)(css, CSS_INTERRUPT, sig, css->disc) >= 0)
								clrsig = 1;
						}
						else if (css->disc->flags & CSS_ERROR)
						{
							if ((*css->disc->exceptf)(css, CSS_ERROR, err, css->disc) >= 0)
								clrerr = 1;
						}
					}
				}
			if (err && (ms != CS_NEVER || (flags & CSS_INTERRUPT) && (sig || err == EINTR) && !clrsig || (flags & CSS_ERROR) && !sig && err != EINTR && !clrerr))
			{
				errno = err;
				state.polling--;
				return 0;
			}
		}
		do
		{
			if (++state.fdnext >= state.fdpolling)
			{
				if (state.fdloop-- < 0)
					break;
				state.fdnext = 0;
			}
			pp = state.fdpoll + state.fdnext;
			fd = pp->fd;
			messagef((state.main->id, NiL, -9, "csspoll: pending=%d polling=%d next=%d fd=%d status=%06o", state.fdpending, state.fdpolling, state.fdnext, fd, pp->status));
			ip = state.fdinfo + fd;
			css = ip->css;
			status = pp->status & ~(CS_POLL_BEFORE|CS_POLL_USER);
			pp->status = 0;
			switch (status)
			{
			case CS_POLL_AUTH|CS_POLL_CONNECT|CS_POLL_READ:
				if (css->auth)
				{
					if (csrecv(css->state, fd, &id, &fdnew, 1) == 1)
					{
						state.auth[fdnew].id = id;
						cssfd(css, fdnew, CS_POLL_AUTH|CS_POLL_READ);
					}
					break;
				}
				/*FALLTHROUGH*/
			case CS_POLL_CONNECT|CS_POLL_READ:
				if (csrecv(css->state, fd, &id, &fdnew, 1) == 1 &&
					(ip = cssfd(css, fdnew, CS_POLL_READ)) &&
					css->disc->acceptf &&
					(fdnew = (*css->disc->acceptf)(css, ip, &id, NiL, css->disc)) != ip->fd)
						cssfd(css, ip->fd, fdnew >= 0 ? CS_POLL_CMD(fdnew, CS_POLL_MOVE) : CS_POLL_CLOSE);
				break;
			case CS_POLL_AUTH|CS_POLL_READ:
				switch ((ap = state.auth + fd)->seq)
				{
				case 1:
					if ((n = csread(css->state, fd, css->buf, sizeof(css->buf), CS_LINE)) <= 0)
						goto reject;
					css->buf[n - 1] = 0;
					if (tokscan(css->buf, &t, "%lu", &key) != 1)
						goto reject;
					switch (key)
					{
					case CS_KEY_SEND:
						n = sfsprintf(css->tmp, sizeof(css->tmp), "%s\n", css->mount);
						if (cswrite(css->state, fd, css->tmp, n) != n)
							goto reject;
						break;
					default:
						if (key <= CS_KEY_MAX || key != css->newkey && key != css->oldkey || tokscan(t, NiL, "%ld", &ap->id.pid) != 1)
							goto reject;
						if (!(ap->seq = css->challenge))
						{
							st.st_uid = css->uid;
							st.st_gid = css->gid;
							goto ok;
						}
						css->conkey ^= cs.time ^ ap->id.pid;
						n = cs.time & 017;
						do TOSS(css->conkey); while (n--);
						ap->atime = css->conkey & KEYMASK;
						n = cs.time & 07;
						do TOSS(css->conkey); while (n--);
						ap->mtime = css->conkey & KEYMASK;
						n = sfsprintf(css->tmp, sizeof(css->tmp), "%lu %lu\n", (unsigned long)ap->atime, (unsigned long)ap->mtime);
						if (cswrite(css->state, fd, css->tmp, n) != n)
							goto reject;
						ap->expire = cs.time + EXPIRE;
						break;
					}
					break;
				case 2:
					ap->seq = 0;
					if (cs.time > ap->expire || (n = csread(css->state, fd, css->buf, sizeof(css->buf), CS_LINE)) <= 1)
						goto reject;
					css->buf[n - 1] = 0;
					if (t = strchr(css->buf, ' '))
						*t++ = 0;
					if (!(s = strrchr(css->buf, '/')))
						goto reject;
					*s = 0;
					if (lstat(css->buf, &st))
						goto reject;
					*s = '/';
					for (n = 0; n < elementsof(css->fid); n++)
#if _UWIN
						if (css->fid[n].dev == st.st_dev)
#else
						if (css->fid[n].dev == st.st_dev && css->fid[n].ino == st.st_ino)
#endif
							break;
					if (n >= elementsof(css->fid))
						goto reject;
					if (lstat(css->buf, &st))
					{
						/*
						 * kick the NonFS cache
						 */

						if (!(s = strrchr(css->buf, '.')))
							goto reject;
						*s = 0;
						close(open(css->buf, O_WRONLY|O_CREAT|O_TRUNC, 0));
						remove(css->buf);
						*s = '.';
						if (lstat(css->buf, &st))
							goto reject;
					}
					if ((st.st_mode & CS_AUTH_MODE) != CS_AUTH_MODE ||
						ap->atime != st.st_atime ||
						ap->mtime != st.st_mtime)
							goto reject;
				ok:
					if (cswrite(css->state, fd, "\n", 1) != 1)
						goto reject;
					pp->events &= ~CS_POLL_AUTH;
					if (css->disc->acceptf)
					{
						ip = state.fdinfo + pp->fd;
						ap->id.uid = st.st_uid;
						ap->id.gid = st.st_gid;
						vp = va;
						while ((vp < &va[elementsof(va) - 1]) &&
							(*vp++ = t) && (t = strchr(t, ' ')))
								*t++ = 0;
						*vp = 0;
						if ((fdnew = (*css->disc->acceptf)(css, ip, &ap->id, *va ? va : (char**)0, css->disc)) != ip->fd)
							cssfd(css, ip->fd, fdnew >= 0 ? CS_POLL_CMD(fdnew, CS_POLL_MOVE) : CS_POLL_CLOSE);
					}
					break;
				default:
					goto reject;
				}
				break;
			case CS_POLL_READ|CS_POLL_WRITE:
			case CS_POLL_WRITE:
				pp->events &= ~CS_POLL_WRITE;
				/*FALLTHROUGH*/
			case CS_POLL_READ:
				ip->status = status;
				messagef((state.main->id, NiL, -9, "csspoll: status=%06o fd=%d actionf=%p", status, fd, ip->actionf));
				if (!ip->actionf || (n = (*ip->actionf)(css, ip, css->disc)) == 0)
				{
					state.fdpending--;
					state.polling--;
					return ip;
				}
				if (n < 0)
					goto reject;
				break;
			case CS_POLL_AUTH:
			case CS_POLL_AUTH|CS_POLL_CONNECT:
			case CS_POLL_CONNECT:
				break;
			case 0:
				continue;
			default:
			reject:
				drop(css, pp);
				state.fdnext--;
				break;
			}
			state.fdpending--;
		} while (state.fdpending);
	}
}
