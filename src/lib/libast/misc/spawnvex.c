/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2014 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * son of spawnveg()
 * more cooperative than posix_spawn()
 *
 * this code looks worse than it is because of the DEBUG_* emulations
 * the DEBUG_* emulations allow { cygwin ibm } debugging on linux
 */

#define _SPAWNVEX_PRIVATE_ \
	unsigned int	max; \
	unsigned int	set; \
	unsigned int	flags; \
	unsigned int	frame; \
	pid_t		pgrp; \
	int		debug; \
	int		noexec; \
	Spawnvex_u*	op;

union _Spawnvex_u;
typedef union _Spawnvex_u Spawnvex_u;

#include <ast.h>
#include <error.h>
#include <wait.h>
#include <sig.h>

#include "FEATURE/spawn"

#ifndef ENOSYS
#define ENOSYS	EINVAL
#endif

#ifndef O_CLOEXEC
#undef	_lib_pipe2
#endif

#if DEBUG_spawn

/*
 * DEBUG_spawn provides an emulation of ibm i5/OS spawn()
 */

#undef	_lib_spawn
#undef	_lib_spawnve
#undef	_lib_spawnvex
#undef	_hdr_spawn
#undef	_mem_pgroup_inheritance

#define	_lib_spawn		1
#define _hdr_spawn		1
#define _mem_pgroup_inheritance	1

#define SPAWN_FDCLOSED		(-1)

#define SPAWN_NEWPGROUP		0

#define SPAWN_SETGROUP		0x01
#define SPAWN_SETSIGDEF		0x02
#define SPAWN_SETSIGMASK	0x04

struct inheritance
{
	int		flags;
	pid_t		pgroup;
	sigset_t	sigmask;
	sigset_t	sigdefault;
};

static pid_t
spawn(const char* path, int nmap, const int map[], const struct inheritance* inherit, char* const argv[], char* const envv[])
{
#if _lib_fork || _lib_vfork
	pid_t			pid;
	pid_t			pgid;
	int			i;
	int			n;
	int			m;
#if _real_vfork
	volatile int		exec_errno;
	volatile int* volatile	exec_errno_ptr;
#else
	int			j;
	int			k;
	int			msg[2];
#endif
#endif

#if _lib_fork || _lib_vfork
	pgid = inherit && (inherit->flags & SPAWN_SETGROUP) ? inherit->pgroup : -1;
	n = errno;
#if _real_vfork
	exec_errno = 0;
	exec_errno_ptr = &exec_errno;
#else
#if _lib_pipe2
	if (pipe2(msg, O_CLOEXEC) < 0)
		msg[0] = -1;
#else
	if (pipe(msg) < 0)
		msg[0] = -1;
	else
	{
		fcntl(msg[0], F_SETFD, FD_CLOEXEC);
		fcntl(msg[1], F_SETFD, FD_CLOEXEC);
	}
#endif
#endif
	sigcritical(SIG_REG_EXEC|SIG_REG_PROC);
#if _lib_vfork
	pid = vfork();
#else
	pid = fork();
#endif
	if (pid == -1)
		n = errno;
	else if (!pid)
	{
		sigcritical(0);
		for (i = 0; i < nmap; i++)
		{
#if !_real_vfork
			for (j = 0; j < elementsof(msg); j++)
				if (i == msg[j] && (k = fcntl(i, F_DUPFD, 0)) >= 0)
				{
					fcntl(k, F_SETFD, FD_CLOEXEC);
					msg[j] = k;
				}
#endif
			if (map[i] == i)
				fcntl(i, F_SETFD, 0);
			else
			{
				close(i);
				fcntl(map[i], F_DUPFD, i);
				close(map[i]);
			}
		}
		if (pgid >= 0 && setpgid(0, pgid) < 0 && pgid && errno == EPERM)
			setpgid(pgid, 0);
		execve(path, argv, envv);
#if _real_vfork
		*exec_errno_ptr = errno;
#else
		if (msg[0] != -1)
		{
			m = errno;
			write(msg[1], &m, sizeof(m));
		}
#endif
		_exit(errno == ENOENT ? EXIT_NOTFOUND : EXIT_NOEXEC);
	}
#if _real_vfork
	if (pid != -1 && (m = *exec_errno_ptr))
	{
		while (waitpid(pid, NiL, 0) == -1 && errno == EINTR);
		pid = -1;
		n = m;
	}
#else
	if (msg[0] != -1)
	{
		close(msg[1]);
		if (pid != -1)
		{
			m = 0;
			while (read(msg[0], &m, sizeof(m)) == -1)
				if (errno != EINTR)
				{
					m = errno;
					break;
				}
			if (m)
			{
				while (waitpid(pid, &n, 0) && errno == EINTR);
				pid = -1;
				n = m;
			}
		}
		close(msg[0]);
	}
#endif
	sigcritical(0);
	if (pgid >= 0 && setpgid(pid, pgid) < 0 && pgid && errno == EPERM)
		setpgid(pid, pid);
	errno = n;
	return pid;
#else
	errno = ENOSYS;
	return -1;
#endif
}

#else

#if DEBUG_spawnve

/*
 * DEBUG_spawnve provides an emulation of cygwin spawnve()
 * where spawnvex() must do a dup dance before and after calling spawn()
 */

#undef	_lib_posix_spawn
#undef	_lib_spawn
#undef	_lib_spawnve
#undef	_lib_spawnvex
#undef	_lib_spawn_mode
#undef	_hdr_spawn
#undef	_mem_pgroup_inheritance

#define	_lib_spawn_mode		1

#define P_DETACH		1
#define P_NOWAIT		2

static pid_t
spawnve(int mode, const char* path, char* const argv[], char* const envv[])
{
#if _lib_fork || _lib_vfork
	pid_t			pid;
	int			i;
	int			n;
	int			m;
#if _real_vfork
	volatile int		exec_errno;
	volatile int* volatile	exec_errno_ptr;
#else
	int			msg[2];
#endif
#endif

#if _lib_fork || _lib_vfork
	n = errno;
#if _real_vfork
	exec_errno = 0;
	exec_errno_ptr = &exec_errno;
#else
#if _lib_pipe2
	if (pipe2(msg, O_CLOEXEC) < 0)
		msg[0] = -1;
#else
	if (pipe(msg) < 0)
		msg[0] = -1;
	else
	{
		fcntl(msg[0], F_SETFD, FD_CLOEXEC);
		fcntl(msg[1], F_SETFD, FD_CLOEXEC);
	}
#endif
#endif
	sigcritical(SIG_REG_EXEC|SIG_REG_PROC);
#if _lib_vfork
	pid = vfork();
#else
	pid = fork();
#endif
	if (pid == -1)
		n = errno;
	else if (!pid)
	{
		sigcritical(0);
		if (mode == P_DETACH)
			setpgid(0, 0);
		execve(path, argv, envv);
#if _real_vfork
		*exec_errno_ptr = errno;
#else
		if (msg[0] != -1)
		{
			m = errno;
			write(msg[1], &m, sizeof(m));
		}
#endif
		_exit(errno == ENOENT ? EXIT_NOTFOUND : EXIT_NOEXEC);
	}
#if _real_vfork
	if (pid != -1 && (m = *exec_errno_ptr))
	{
		while (waitpid(pid, NiL, 0) == -1 && errno == EINTR);
		pid = -1;
		n = m;
	}
#else
	if (msg[0] != -1)
	{
		close(msg[1]);
		if (pid != -1)
		{
			m = 0;
			while (read(msg[0], &m, sizeof(m)) == -1)
				if (errno != EINTR)
				{
					m = errno;
					break;
				}
			if (m)
			{
				while (waitpid(pid, &n, 0) && errno == EINTR);
				pid = -1;
				n = m;
			}
		}
		close(msg[0]);
	}
#endif
	sigcritical(0);
	errno = n;
	return pid;
#else
	errno = ENOSYS;
	return -1;
#endif
}

#endif
#endif

#if _lib_spawnvex

NoN(spawnvex)

#else

#define VEXCHUNK	8
#define VEXFLAG(x)	(1<<(-(x)))
#define VEXINIT(p)	((p)->cur=(p)->frame,(p)->set=0)

union _Spawnvex_u
{
	intmax_t	number;
	void*		handle;
	Spawnvex_f	callback;
};

#if _lib_spawn_mode

#ifndef P_NOWAIT

#include <process.h>

#ifndef P_NOWAIT
#define P_NOWAIT	_P_NOWAIT
#endif
#ifndef P_DETACH
#define P_DETACH	_P_DETACH
#endif

#endif

#define SPAWN_cloexec	(-99)

#else

#if _lib_spawn && _hdr_spawn && _mem_pgroup_inheritance

#include <spawn.h>

#else

#if _lib_posix_spawn

#include <spawn.h>

#else

#include <ast_tty.h>
#include <ast_vfork.h>

#if _lib_spawnve && _hdr_process
#include <process.h>
#if defined(P_NOWAIT) || defined(_P_NOWAIT)
#undef	_lib_spawnve
#endif
#endif

#if !_lib_vfork
#undef	_real_vfork
#endif

#endif
#endif
#endif

#if _lib_spawn_mode || _lib_spawn && _hdr_spawn && _mem_pgroup_inheritance

/*
 * slide fd out of the way so native spawn child process doesn't see it
 */

static int
save(int fd, int* pf, int md)
{
	int	td;

	if ((*pf = fcntl(fd, F_GETFD)) < 0)
		return -1;
	if (md)
	{
#ifdef F_DUPFD_CLOEXEC
		if ((td = fcntl(fd, F_DUPFD_CLOEXEC, md)) < 0)
#else
		if ((td = fcntl(fd, F_DUPFD, md)) < 0 || fcntl(td, F_SETFD, FD_CLOEXEC) < 0 && (close(td), 1))
#endif
			return -2;
		close(fd);
		fd = td;
	}
	return fd;
}

/*
 * add save()'d fd restore ops to vex
 */

static int
restore(Spawnvex_t* vex, int i, int j, int fl)
{
	if (j >= 0)
	{
		if (spawnvex_add(vex, i, j, 0, 0) < 0)
			return -1;
		if (fl == 0 && spawnvex_add(vex, j, j, 0, 0) < 0)
			return -1;
	}
	if (spawnvex_add(vex, i, -1, 0, 0) < 0)
		return -1;
	return 0;
}

#endif

/*
 * apply Spawnvex_t.flags
 */

static void
apply_flags(unsigned int flags)
{
	if (flags & SPAWN_DAEMON)
	{
#ifdef SIGHUP
		signal(SIGHUP, SIG_IGN);
#endif
		signal(SIGTERM, SIG_IGN);
#ifdef SIGTSTP
		signal(SIGTSTP, SIG_IGN);
#endif
#ifdef SIGTTIN
		signal(SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTTOU
		signal(SIGTTOU, SIG_IGN);
#endif
	}
	if (flags & (SPAWN_BACKGROUND|SPAWN_DAEMON))
	{
		signal(SIGINT, SIG_IGN);
#ifdef SIGQUIT
		signal(SIGQUIT, SIG_IGN);
#endif
	}
	if (flags & SPAWN_DAEMON)
		setsid();
#ifdef SIGCHLD
	if (flags & SPAWN_ZOMBIE)
		signal(SIGCHLD, SIG_IGN);
#endif
}

Spawnvex_t*
spawnvex_open(unsigned int flags)
{
	Spawnvex_t*	vex;

	if (vex = newof(0, Spawnvex_t, 1, 0))
	{
		VEXINIT(vex);
		vex->flags = flags;
#ifdef F_DUPFD_CLOEXEC
		vex->debug = (flags & SPAWN_DEBUG) ? fcntl(2, F_DUPFD_CLOEXEC, 60) : -1;
#else
		if ((vex->debug = (flags & SPAWN_DEBUG) ? fcntl(2, F_DUPFD, 60) : -1) >= 0)
			fcntl(vex->debug, F_SETFD, FD_CLOEXEC);
#endif
	}
	return vex;
}

int
spawnvex_add(Spawnvex_t* vex, intmax_t op, intmax_t arg, Spawnvex_f callback, void* handle)
{
	if ((vex->cur + (callback ? 4 : 2)) >= vex->max)
	{
		vex->max += VEXCHUNK;
		if (!(vex->op = oldof(vex->op, Spawnvex_u, vex->max, 0)))
		{
			vex->max = 0;
			VEXINIT(vex);
			return -1;
		}
	}
	switch ((int)op)
	{
	case SPAWN_frame:
		arg = vex->frame;
		vex->frame = vex->cur;
		break;
	case SPAWN_pgrp:
#if OBSOLETE < 20150101
		if (arg == 1)
			arg = 0;
#endif
		break;
	}
	if (op < 0)
		vex->set |= VEXFLAG(op);
	op *= 2;
	if (callback)
	{
		if (op < 0)
			op--;
		else
			op++;
	}
	if (vex->debug > 0)
		error(ERROR_OUTPUT, vex->debug, "spawnvex add %4d %8d %p %4d %4I*d %4I*d %p %p", __LINE__, getpid(), vex, vex->cur, sizeof(op), op / 2, sizeof(arg), arg, callback, callback ? handle : NiL);
	vex->op[vex->cur++].number = op;
	vex->op[vex->cur++].number = arg;
	if (callback)
	{
		vex->op[vex->cur++].callback = callback;
		vex->op[vex->cur++].handle = handle;
	}
	return vex->cur;
}

int
spawnvex_apply(Spawnvex_t* vex, int cur, int flags)
{
	int		i;
	int		j;
	int		k;
	int		op;
	int		arg;
	int		err;
	int		ret;
	off_t		off;
	void*		handle;
	Spawnvex_f	callback;
	unsigned char	siz[512];

	if (cur < 0 || cur > vex->max)
		return EINVAL;
	ret = 0;
	if (!(flags & SPAWN_RESET))
	{
		vex->noexec = -1;
		vex->pgrp = -1;
		i = cur;
		if (flags & SPAWN_UNDO)
		{
			for (j = 0; i < vex->cur && j < elementsof(siz) - 1; j++)
				i += (siz[j] = (vex->op[i].number & 1) ? 4 : 2);
			siz[j] = 0;
		}
		for (;;)
		{
			k = i;
			if (flags & SPAWN_UNDO)
			{
				if (j < 1)
					break;
				i -= siz[j];
				i -= siz[--j];
			}
			else if (i >= vex->cur || !vex->op)
				break;
			op = vex->op[i++].number;
			arg = vex->op[i++].number;
			if (!(op & 1))
				callback = 0;
			else if (flags & SPAWN_NOCALL)
			{
				i += 2;
				callback = 0;
			}
			else
			{
				callback = vex->op[i++].callback;
				handle = vex->op[i++].handle;
			}
			op /= 2;
			if (vex->debug >= 0)
				error(ERROR_OUTPUT, vex->debug, "spawnvex app %4d %8d %p %4d %4I*d %4I*d %p %p", __LINE__, getpid(), vex, k, sizeof(op), op, sizeof(arg), arg, callback, callback ? handle : NiL);
			if (!(flags & SPAWN_CLEANUP))
			{
				err = 0;
				switch (op)
				{
				case SPAWN_noop:
				case SPAWN_frame:
					break;
				case SPAWN_cwd:
					if (fchdir(arg))
						err = errno;
					break;
#ifdef SPAWN_cloexec
				case SPAWN_cloexec:
					if (fcntl(arg, F_SETFD, FD_CLOEXEC) < 0)
						err = errno;
					break;
#endif
				case SPAWN_noexec:
					callback = 0;
					vex->noexec = k;
					break;
				case SPAWN_pgrp:
					/* parent may succeed and cause setpigid() to fail but that's ok */
					if (setpgid(0, arg) < 0 && arg && errno == EPERM)
						setpgid(arg, 0);
					vex->pgrp = (pid_t)arg;
					break;
				case SPAWN_resetids:
					if (arg == 1)
					{
						if (geteuid() == 0 && (setuid(geteuid()) < 0 || setgid(getegid()) < 0))
							err = errno;
					}
					else if (setuid(getuid()) < 0 || setgid(getgid()) < 0)
						err = errno;
					break;
				case SPAWN_sid:
					if (setsid() < 0)
						err = errno;
					break;
				case SPAWN_sigdef:
					err = ENOSYS;
					break;
				case SPAWN_sigmask:
					err = ENOSYS;
					break;
				case SPAWN_truncate:
					if (callback)
					{
						if ((err = (*callback)(handle, op, arg)) < 0)
							continue;
						callback = 0;
						if (err)
							break;
					}
					if ((off = lseek(arg, 0, SEEK_CUR)) < 0 || ftruncate(arg, off) < 0)
						err = errno;
					break;
				case SPAWN_umask:
					umask(arg);
					break;
				default:
					if (op < 0)
						err = EINVAL;
					else if (arg < 0)
					{
						if (callback)
						{
							if ((err = (*callback)(handle, op, arg)) < 0)
								continue;
							callback = 0;
							if (err)
								break;
						}
#if 0
						if (close(op))
							err = errno;
#else
						close(op);
#endif
					}
					else if (op == arg)
					{
						if (fcntl(op, F_SETFD, 0) < 0)
							err = errno;
					}
					else
					{
						close(arg);
						if (fcntl(op, F_DUPFD, arg) < 0)
							err = errno;
					}
					break;
				}
				if (err || callback && (err = (*callback)(handle, op, arg)) > 0)
				{
					if (!(flags & SPAWN_FLUSH))
						return err;
					ret = err;
				}
			}
			else if (op >= 0 && arg >= 0 && op != arg)
				close(op);
		}
	}
	if (!(flags & SPAWN_NOCALL))
	{
		if (!(flags & SPAWN_FRAME))
			vex->frame = 0;
		else if (vex->op && (vex->op[vex->frame].number / 2) == SPAWN_frame)
		{
			cur = vex->frame;
			vex->frame = (unsigned int)vex->op[vex->frame + 1].number;
		}
		if (!(vex->cur = cur))
			VEXINIT(vex);
	}
	return ret;
}

/*
 * return the fd that is redirected to fd
 * otherwise -1
 *
 * what is the fd that is redirected to 2? spawnvex_get(vex, 2, 0)
 */

intmax_t
spawnvex_get(Spawnvex_t* vex, int fd, int i)
{
	int		od;
	int		op;
	int		arg;
	int		k;
	int		m;

	if (i >= 0 && i < vex->max)
	{
		od = fd;
		m = vex->cur;
		while (i < m)
		{
			k = i;
			op = vex->op[i++].number;
			arg = vex->op[i++].number;
			if (op & 1)
				i += 2;
			op /= 2;
			if (op >= 0 && arg == fd)
			{
				fd = op;
				m = k;
				i = 0;
			}
		}
		if (fd != od)
			return fd;
	}
	return -1;
}

int
spawnvex_close(Spawnvex_t* vex)
{
	if (!vex)
		return -1;
	if (vex->op)
		free(vex->op);
	if (vex->debug >= 0)
		close(vex->debug);
	free(vex);
	return 0;
}

pid_t
spawnvex(const char* path, char* const argv[], char* const envv[], Spawnvex_t* vex)
{
	int				i;
	int				op;
	unsigned int			flags;
	pid_t				pid;
#if _lib_posix_spawn
	int				arg;
	int				err;
	int				fd;
	posix_spawnattr_t		ax;
	posix_spawn_file_actions_t	fx;
	Spawnvex_t*			xev = 0;
#endif
#if _lib_spawn_mode || _lib_spawn && _hdr_spawn && _mem_pgroup_inheritance
	pid_t				pgid;
	int				arg;
	int				j;
	int				k;
	int				m;
	int				ic;
	int				jc;
	Spawnvex_t*			xev;
#if !_lib_spawn_mode
	int*				map;
	int				a;
	struct inheritance		inherit;
#endif
#endif

	if (vex && vex->debug >= 0)
		error(ERROR_OUTPUT, vex->debug, "spawnvex exe %4d %8d %p %4d \"%s\"", __LINE__, getpid(), vex, vex->cur, path);
#if _lib_spawn_mode || _lib_spawn && _hdr_spawn && _mem_pgroup_inheritance
	if (!envv)
		envv = environ;
	pid = -1;
	m = 0;
	if (vex)
	{
		vex->noexec = -1;
		vex->pgrp = -1;
		flags = vex->flags;
		if (!(xev = spawnvex_open(0)))
			goto bad;
		j = -1;
		for (i = 0; i < vex->cur;)
		{
			op = vex->op[i++].number;
			arg = vex->op[i++].number;
			if (op & 1)
				i += 2;
			op /= 2;
			if (op >= 0)
			{
				if (m < op)
					m = op + 1;
				if (m < arg)
					m = arg + 1;
			}
			else if (op == SPAWN_cwd)
				j = arg;
		}
		if (j >= 0)
		{
			if ((i = open(".", O_RDONLY)) < 0)
				goto bad;
			if ((i = save(i, &ic, m)) < 0)
				goto bad;
			if (spawnvex_add(xev, SPAWN_cwd, i, 0, 0) < 0 || restore(xev, i, -1, 0) < 0)
			{
				close(i);
				goto bad;
			}
			if (fchdir(j) < 0)
				goto bad;
			if ((i = save(j, &jc, m)) < 0)
				goto bad;
			if (restore(xev, i, j, jc) < 0)
			{
				close(i);
				goto bad;
			}
		}
	}
	else
	{
		flags = 0;
		xev = 0;
	}
#if _lib_spawn_mode
	if (vex)
		for (i = 0; i < vex->cur;)
		{
			op = vex->op[i++].number;
			arg = vex->op[i++].number;
			if (op & 1)
				i += 2;
			switch (op /= 2)
			{
			case SPAWN_frame:
				vex->frame = (unsigned int)arg;
				break;
			case SPAWN_pgrp:
				vex->pgrp = (pid_t)arg;
				break;
			default:
				if (op >= 0)
				{
					if (arg < 0)
					{
						if ((i = save(op, &ic, m)) < 0)
						{
							if (i < -1)
								goto bad;
						}
						else if (restore(xev, i, op, ic) < 0)
						{
							close(i);
							goto bad;
						}
						else
							close(op);
					}
					else if (arg == op)
					{
						if (spawnvex_add(xev, SPAWN_cloexec, arg, 0, 0) < 0)
							goto bad;
						if (fcntl(arg, F_SETFD, 0) < 0)
							goto bad;
					}
					else if ((j = save(arg, &jc, m)) < -1)
						goto bad;
					else
					{
						close(arg);
						if (fcntl(op, F_DUPFD, arg) >= 0)
						{
							if ((i = save(op, &ic, m)) >= 0)
							{
								if (restore(xev, i, op, ic) >= 0)
								{
									close(op);
									if (j < 0 || restore(xev, j, arg, jc) >= 0)
										continue;
								}
								close(i);
							}
						}
						if (j >= 0)
						{
							fcntl(j, F_DUPFD, arg);
							close(j);
						}
						goto bad;
					}
				}
				break;
			}
		}
	pid = spawnve(vex && vex->pgrp >= 0 ? P_DETACH : P_NOWAIT, path, argv, envv);
#else
	inherit.flags = 0;
	map = 0;
	if (vex)
	{
		if (m)
		{
			if (!(map = newof(0, int, m, 0)))
				goto bad;
			for (i = 0; i < m; i++)
				map[i] = i;
		}
		for (i = 0; i < vex->cur;)
		{
			op = vex->op[i++].number;
			a = i;
			arg = vex->op[i++].number;
			if (op & 1)
				i += 2;
			switch (op /= 2)
			{
			case SPAWN_noop:
			case SPAWN_noexec:
				break;
			case SPAWN_frame:
				vex->frame = (unsigned int)arg;
				break;
			case SPAWN_pgrp:
				inherit.flags |= SPAWN_SETGROUP;
				inherit.pgroup = arg ? arg : SPAWN_NEWPGROUP;
				break;
			case SPAWN_sigdef:
				inherit.flags |= SPAWN_SETSIGDEF;
				sigemptyset(&inherit.sigdefault);
				for (j = 1; j < 8 * sizeof(vex->op[a].number); j++)
					if (vex->op[a].number & (1<<j))
						sigaddset(&inherit.sigdefault, j);
				break;
			case SPAWN_sigmask:
				inherit.flags |= SPAWN_SETSIGMASK;
				sigemptyset(&inherit.sigmask);
				for (j = 1; j < 8 * sizeof(vex->op[a].number); j++)
					if (vex->op[a].number & (1<<j))
						sigaddset(&inherit.sigmask, j);
				break;
			default:
				if (op < 0)
				{
					errno = EINVAL;
					goto bad;
				}
				else if (arg < 0)
					map[op] = SPAWN_FDCLOSED;
				else
					map[op] = arg;
				break;
			}
		}
	}
	pid = spawn(path, m, map, &inherit, (const char**)argv, (const char**)envv);
#endif
	if (pid >= 0 && vex)
		VEXINIT(vex);
 bad:
	if (xev)
	{
		spawnvex_apply(xev, 0, SPAWN_FLUSH|SPAWN_NOCALL);
		spawnvex_close(xev);
	}
#if !_lib_spawn_mode
	if (map)
		free(map);
#endif
	return pid;

#else

#if _lib_spawnve
#if _lib_fork || _lib_vfork
	if (!vex || !vex->cur && !vex->flags)
#endif
		return spawnve(path, argv, envv);
#endif
#if _lib_posix_spawn
	if (vex && ((vex->set & (0
#if !_lib_posix_spawnattr_setfchdir
		|VEXFLAG(SPAWN_cwd)
#endif
#if !_lib_posix_spawnattr_setsid
		|VEXFLAG(SPAWN_sid)
#endif
#if !_lib_posix_spawnattr_setumask
		|VEXFLAG(SPAWN_umask)
#endif
		))
#if _lib_posix_spawn < 2
		|| !(vex->flags & SPAWN_EXEC)
#endif
		))
#endif
	{
#if _lib_fork || _lib_vfork
		pid_t			pgid;
		int			n;
		int			m;
		Spawnvex_noexec_t	nx;
		int			msg[2];
#if _real_vfork
		volatile int		exec_errno;
		volatile int* volatile	exec_errno_ptr;
#endif

		if (!envv)
			envv = environ;
#if _lib_vfork
#if _lib_fork
		if (!vex || !(vex->flags & SPAWN_FORK))
#endif
			nx.flags = SPAWN_VFORK;
#if _lib_fork
		else
#endif
#endif
#if _lib_fork
			nx.flags = SPAWN_FORK;
#endif
		n = errno;
#if _real_vfork
		if (nx.flags & SPAWN_VFORK)
		{
			exec_errno = 0;
			exec_errno_ptr = &exec_errno;
			msg[0] = msg[1] = -1;
		}
		else
#endif
#if _lib_pipe2
		if (pipe2(msg, O_CLOEXEC) < 0)
			msg[0] = msg[1] = -1;
#else
		if (pipe(msg) < 0)
			msg[0] = msg[1] = -1;
		else
		{
			fcntl(msg[0], F_SETFD, FD_CLOEXEC);
			fcntl(msg[1], F_SETFD, FD_CLOEXEC);
		}
#endif
		if (!(flags & SPAWN_FOREGROUND))
			sigcritical(SIG_REG_EXEC|SIG_REG_PROC);
#if _lib_vfork
#if _lib_fork
		if (nx.flags & SPAWN_VFORK)
#endif
			pid = vfork();
#if _lib_fork
		else
#endif
#endif
#if _lib_fork
			pid = fork();
#endif
		if (pid == -1)
			n = errno;
		else if (!pid)
		{
			if (!(flags & SPAWN_FOREGROUND))
				sigcritical(0);
			if (vex && (n = spawnvex_apply(vex, 0, SPAWN_FRAME|SPAWN_NOCALL)))
				errno = n;
			else
			{
				if (vex && vex->debug >= 0)
					error(ERROR_OUTPUT, vex->debug, "spawnvex exe %4d %8d %p %4d \"%s\"", __LINE__, getpid(), vex, vex->cur, path);
				execve(path, argv, envv);
				if (vex && vex->debug >= 0)
					error(ERROR_OUTPUT, vex->debug, "spawnvex exe %4d %8d %p %4d \"%s\" FAILED", __LINE__, getpid(), vex, vex->cur, path);
				if (vex && (i = vex->noexec) >= 0)
				{
					nx.vex = vex;
					nx.handle = vex->op[i + 3].handle;
					nx.path = path;
					nx.argv = argv;
					nx.envv = envv;
#if _use_spawn_exec
					/*
					 * setting SPAWN_EXEC here means that it is more efficient to
					 * exec(interpreter) on script than to fork() initialize and
					 * read script -- highly subjective, based on some ksh
					 * implementaions, and probably won't be set unless its a
					 * noticable win
					 */

					nx.flags |= SPAWN_EXEC;
#endif
					nx.msgfd = msg[1];
					errno = (*vex->op[i + 2].callback)(&nx, SPAWN_noexec, errno);
				}
			}
#if _real_vfork
			if (nx.flags & SPAWN_VFORK)
				*exec_errno_ptr = errno;
			else
#endif
			if (msg[1] != -1)
			{
				m = errno;
				write(msg[1], &m, sizeof(m));
			}
			_exit(errno == ENOENT ? EXIT_NOTFOUND : EXIT_NOEXEC);
		}
#if _real_vfork
		if ((nx.flags & SPAWN_VFORK) && pid != -1 && (m = *exec_errno_ptr))
		{
			while (waitpid(pid, NiL, 0) == -1 && errno == EINTR);
			pid = -1;
			n = m;
		}
		else
#endif
		if (msg[0] != -1)
		{
			close(msg[1]);
			if (pid != -1)
			{
				m = 0;
				while (read(msg[0], &m, sizeof(m)) == -1)
					if (errno != EINTR)
					{
						m = errno;
						break;
					}
				if (m)
				{
					while (waitpid(pid, &n, 0) && errno == EINTR);
					pid = -1;
					n = m;
				}
			}
			close(msg[0]);
		}
		if (!(flags & SPAWN_FOREGROUND))
			sigcritical(0);
		if (pid != -1 && vex)
		{
			if (vex->pgrp >= 0 && setpgid(pid, vex->pgrp) < 0 && vex->pgrp && errno == EPERM)
				setpgid(pid, pid);
			VEXINIT(vex);
		}
		errno = n;
		return pid;
#else
		errno = ENOSYS;
		return -1;
#endif
	}
#if _lib_posix_spawn
	if (vex)
	{
		if (err = posix_spawnattr_init(&ax))
			goto nope;
		if (err = posix_spawn_file_actions_init(&fx))
		{
			posix_spawnattr_destroy(&ax);
			goto nope;
		}
		for (i = 0; i < vex->cur;)
		{
			op = vex->op[i++].number;
			arg = vex->op[i++].number;
			if (op & 1)
				i += 2;
			switch (op /= 2)
			{
			case SPAWN_noop:
			case SPAWN_noexec:
			case SPAWN_frame:
				break;
#if _lib_posix_spawnattr_setfchdir
			case SPAWN_cwd:
				if (err = posix_spawnattr_setfchdir(&ax, arg))
					goto bad;
				break;
#endif
			case SPAWN_pgrp:
				if (err = posix_spawnattr_setpgroup(&ax, arg))
					goto bad;
				if (err = posix_spawnattr_setflags(&ax, POSIX_SPAWN_SETPGROUP))
					goto bad;
				break;
			case SPAWN_resetids:
				if (err = posix_spawnattr_setflags(&ax, POSIX_SPAWN_RESETIDS))
					goto bad;
				break;
#if _lib_posix_spawnattr_setsid
			case SPAWN_sid:
				if (err = posix_spawnattr_setsid(&ax, arg))
					goto bad;
				break;
#endif
			case SPAWN_sigdef:
				break;
			case SPAWN_sigmask:
				break;
#if _lib_posix_spawnattr_setumask
			case SPAWN_umask:
				if (err = posix_spawnattr_setumask(&ax, arg))
					goto bad;
				break;
#endif
			default:
				if (op < 0)
				{
					err = EINVAL;
					goto bad;
				}
				else if (arg < 0)
				{
					if (err = posix_spawn_file_actions_addclose(&fx, op))
						goto bad;
				}
				else if (arg == op)
				{
#ifdef F_DUPFD_CLOEXEC
					if ((fd = fcntl(op, F_DUPFD_CLOEXEC, 0)) < 0)
#else
					if ((fd = fcntl(op, F_DUPFD, 0)) < 0 || fcntl(fd, F_SETFD, FD_CLOEXEC) < 0 && (close(fd), 1))
#endif
					{
						err = errno;
						goto bad;
					}
					if (!xev && !(xev = spawnvex_open(0)))
						goto bad;
					spawnvex_add(xev, fd, -1, 0, 0);
					if (err = posix_spawn_file_actions_adddup2(&fx, fd, op))
						goto bad;
				}
				else if (err = posix_spawn_file_actions_adddup2(&fx, op, arg))
					goto bad;
				break;
			}
		}
		if (err = posix_spawn(&pid, path, &fx, &ax, argv, envv ? envv : environ))
			goto bad;
		posix_spawnattr_destroy(&ax);
		posix_spawn_file_actions_destroy(&fx);
		if (xev)
		{
			spawnvex_apply(xev, 0, SPAWN_NOCALL);
			spawnvex_close(xev);
		}
		if (vex->flags & SPAWN_CLEANUP)
			spawnvex_apply(vex, 0, SPAWN_FRAME|SPAWN_CLEANUP);
		VEXINIT(vex);
	}
	else if (err = posix_spawn(&pid, path, NiL, NiL, argv, envv ? envv : environ))
		goto nope;
	if (vex && vex->debug >= 0)
		error(ERROR_OUTPUT, vex->debug, "spawnvex exe %4d %8d %p %4d \"%s\" %8d posix_spawn", __LINE__, getpid(), vex, vex->cur, path, pid);
	return pid;
 bad:
	posix_spawnattr_destroy(&ax);
	posix_spawn_file_actions_destroy(&fx);
#if _lib_posix_spawn
	if (xev)
	{
		spawnvex_apply(xev, 0, SPAWN_NOCALL);
		spawnvex_close(xev);
	}
#endif
 nope:
	errno = err;
	if (vex && vex->debug >= 0)
		error(ERROR_OUTPUT, vex->debug, "spawnvex exe %4d %8d %p %4d \"%s\" %8d posix_spawn FAILED", __LINE__, getpid(), vex, vex->cur, path, -1);
	return -1;
#endif
#endif
}

#endif
