/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
 * *at() emulation -- if this code is active for your time critical app then upgrade your system
 */

#ifndef _AST_INTERCEPT
#define _AST_INTERCEPT	0
#endif

#include "astlib.h"
#include "FEATURE/atdev"

#include <aso.h>
#include <error.h>
#include <sig.h>

#ifndef ENOSYS
#define ENOSYS	EINVAL
#endif

static unsigned int	_at_lock;

#if defined(_fd_self_dir_fmt) || defined(_fd_pid_dir_fmt)

#if defined(_fd_self_dir_fmt)

#define ATBEG(cwd,path,works) \
	{ \
		int		_at_dot = -1; \
		int		_at_ret; \
		unsigned int	_at_tid; \
		int		_at_works; \
		char		_at_##path[PATH_MAX]; \
		if (_at_works = works) \
		{ \
			if (*path == '/') \
				cwd = AT_FDCWD; \
			else if (cwd != AT_FDCWD) \
				sfsprintf(_at_##path, sizeof(_at_##path), _fd_self_dir_fmt, cwd, "/", path); \
		} \
		else if (cwd != AT_FDCWD && *path != '/') \
		{ \
			sigcritical(SIG_REG_ALL); \
			_at_tid = asothreadid(); \
			if (_at_lock == _at_tid) \
				_at_tid = 0; \
			else \
				asolock(&_at_lock, _at_tid, ASO_LOCK); \
			if ((_at_dot = open(".", O_SEARCH|O_CLOEXEC)) < 0 || fchdir(cwd)) \
			{ \
				if (_at_tid) \
					asolock(&_at_lock, _at_tid, ASO_UNLOCK); \
				sigcritical(SIG_REG_POP); \
				return -1; \
			} \
		}

#define ATBEGL(lwd,link,works) \
	{ \
		int		_at_dot = -1; \
		int		_at_ret; \
		unsigned int	_at_tid; \
		int		_at_works; \
		char		_at_##link[PATH_MAX]; \
		if (_at_works = works) \
		{ \
			if (*link == '/') \
				lwd = AT_FDCWD; \
			else if (lwd != AT_FDCWD) \
				sfsprintf(_at_##link, sizeof(_at_##link), _fd_self_dir_fmt, lwd, "/", link); \
		} \
		else \
		{ \
			errno = EINVAL; \
			return -1; \
		}

#else

#define ATBEG(cwd,path,works) \
	{ \
		int		_at_dot = -1; \
		int		_at_ret; \
		int		_at_works; \
		unsigned int	_at_tid; \
		char		_at_##path[PATH_MAX]; \
		if (_at_works = works) \
		{ \
			if (*path == '/') \
				cwd = AT_FDCWD; \
			else if (cwd != AT_FDCWD) \
				sfsprintf(_at_##path, sizeof(_at_##path), _fd_pid_dir_fmt, getpid(), cwd, "/", path);
		} \
		else if (cwd != AT_FDCWD && *path != '/') \
		{ \
			sigcritical(SIG_REG_ALL); \
			_at_tid = asothreadid(); \
			if (_at_lock == _at_tid) \
				_at_tid = 0; \
			else \
				asolock(&_at_lock, _at_tid, ASO_LOCK); \
			if ((_at_dot = open(".", O_SEARCH|O_CLOEXEC)) < 0 || fchdir(cwd)) \
			{ \
				if (_at_tid) \
					asolock(&_at_lock, _at_tid, ASO_UNLOCK); \
				sigcritical(SIG_REG_POP); \
				return -1; \
			} \
		}

#define ATBEGL(lwd,path,works) \
	{ \
		int		_at_dot = -1; \
		int		_at_ret; \
		unsigned int	_at_tid; \
		int		_at_works; \
		char		_at_##link[PATH_MAX]; \
		if (_at_works = works) \
		{ \
			if (*link == '/') \
				lwd = AT_FDCWD; \
			else if (lwd != AT_FDCWD) \
				sfsprintf(_at_##link, sizeof(_at_##link), _fd_pid_dir_fmt, getpid(), lwd, "/", link); \
		} \
		else \
		{ \
			errno = EINVAL; \
			return -1; \
		}

#endif

#define ATPATH(cwd,path) \
		((_at_works)?((cwd==AT_FDCWD)?path:(const char*)_at_##path):(path))

#define ATEND() \
	}

#define ATENDL() \
	}

#else

#define ATBEG(cwd,path,works) \
	{ \
		int		_at_dot = -1; \
		int		_at_ret; \
		unsigned int	_at_tid; \
		if (cwd != AT_FDCWD && *path != '/') \
		{ \
			sigcritical(SIG_REG_ALL); \
			_at_tid = asothreadid(); \
			if (_at_lock == _at_tid) \
				_at_tid = 0; \
			else \
				asolock(&_at_lock, _at_tid, ASO_LOCK); \
			if ((_at_dot = open(".", O_SEARCH|O_CLOEXEC)) < 0 || fchdir(cwd)) \
			{ \
				if (_at_tid) \
					asolock(&_at_lock, _at_tid, ASO_UNLOCK); \
				sigcritical(SIG_REG_POP); \
				return -1; \
			} \
		}

#define ATPATH(cwd,path) \
		path

#define ATEND() \
		if (_at_dot >= 0) \
		{ \
			_at_ret = fchdir(_at_dot); \
			close(_at_dot); \
			if (_at_tid) \
				asolock(&_at_lock, _at_tid, ASO_UNLOCK); \
			sigcritical(SIG_REG_POP); \
			if (_at_ret) \
				return -1; \
		} \
	}

#endif

#define STUB	1

#if !_lib_faccessat

#undef	STUB

int
faccessat(int cwd, const char* path, mode_t mode, int flags)
{
	int	r;

	ATBEG(cwd, path, _fd_dir_access);
	r = (flags & AT_EACCESS) ? eaccess(ATPATH(cwd, path), mode) : access(ATPATH(cwd, path), mode);
	ATEND();
	return r;
}

#endif

#if !_lib_fchmodat

int
fchmodat(int cwd, const char* path, mode_t mode, int flags)
{
	int	r;

	if (flags & AT_SYMLINK_NOFOLLOW)
	{
#if _lib_lchmod
		ATBEG(cwd, path, _fd_dir_lchmod);
		r = lchmod(ATPATH(cwd, path), mode);
		ATEND();
#else
		errno = ENOSYS;
		r = -1;
#endif
	}
	else
	{
		ATBEG(cwd, path, _fd_dir_chmod);
		r = chmod(ATPATH(cwd, path), mode);
		ATEND();
	}
	return r;
}

#endif

#if !_lib_fchownat

#undef	STUB

int
fchownat(int cwd, const char* path, uid_t owner, gid_t group, int flags)
{
	int	r;

	if (flags & AT_SYMLINK_NOFOLLOW)
	{
#if _lib_lchown
		ATBEG(cwd, path, _fd_dir_lchown);
		r = lchown(ATPATH(cwd, path), owner, group);
		ATEND();
#else
		errno = ENOSYS;
		r = -1;
#endif
	}
	else
	{
		ATBEG(cwd, path, _fd_dir_chown);
		r = chown(ATPATH(cwd, path), owner, group);
		ATEND();
	}
	return r;
}

#endif

#if !_lib_fstatat

#undef	STUB

int
fstatat(int cwd, const char* path, struct stat* st, int flags)
{
	int	r;

	ATBEG(cwd, path, _fd_dir_stat);
	r = (flags & AT_SYMLINK_NOFOLLOW) ? lstat(ATPATH(cwd, path), st) : stat(ATPATH(cwd, path), st);
	ATEND();
	return r;
}

#endif

#if !_lib_linkat

#undef	STUB

int
linkat(int pwd, const char* path, int lwd, const char* linkpath, int flags)
{
	int	r;

	if (pwd == AT_FDCWD || *path == '/')
	{
		if (lwd == AT_FDCWD || *linkpath == '/')
			r = link(path, linkpath);
		else
		{
			ATBEG(lwd, linkpath, _fd_dir_link);
			r = link(path, ATPATH(lwd, linkpath));
			ATEND();
		}
	}
	else if (lwd == AT_FDCWD || *linkpath == '/')
	{
		ATBEG(pwd, path, _fd_dir_link);
		r = link(ATPATH(pwd, path), linkpath);
		ATEND();
	}
	else
	{
#ifdef ATBEGL
		ATBEGL(lwd, linkpath, _fd_dir_link);
		ATBEG(pwd, path, _fd_dir_link);
		r = link(ATPATH(pwd, path), ATPATH(lwd, linkpath));
		ATEND();
		ATEND();
#else
		errno = EINVAL;
		r = -1;
#endif
	}
	return r;
}

#endif

#if !_lib_mkdirat

#undef	STUB

int
mkdirat(int cwd, const char* path, mode_t mode)
{
	int	r;

	ATBEG(cwd, path, _fd_dir_mkdir);
	r = mkdir(ATPATH(cwd, path), mode);
	ATEND();
	return r;
}

#endif

#if !_lib_mkfifoat

#undef	STUB

int
mkfifoat(int cwd, const char* path, mode_t mode)
{
	int	r;

	ATBEG(cwd, path, _fd_dir_mkfifo);
	r = mkfifo(ATPATH(cwd, path), mode);
	ATEND();
	return r;
}

#endif

#if !_lib_mknodat

#undef	STUB

int
mknodat(int cwd, const char* path, mode_t mode, dev_t dev)
{
	int	r;

	ATBEG(cwd, path, _fd_dir_mknod);
	r = mknod(ATPATH(cwd, path), mode, dev);
	ATEND();
	return r;
}

#endif

#if !_lib_openat

#undef	STUB

int
openat(int cwd, const char* path, int flags, ...)
{
	int	r;
	mode_t	mode;
	va_list	ap;

	va_start(ap, flags);
	mode = (flags & O_CREAT) ? (mode_t)va_arg(ap, int) : (mode_t)0;
	va_end(ap);
	ATBEG(cwd, path, _fd_dir_open);
	r = open(ATPATH(cwd, path), flags, mode);
	ATEND();
	return r;
}

#endif

#if !_lib_readlinkat

#undef	STUB

ssize_t
readlinkat(int cwd, const char* path, char* buf, size_t size)
{
	int	r;

	ATBEG(cwd, path, _fd_dir_readlink);
	r = readlink(ATPATH(cwd, path), buf, size);
	ATEND();
	return r;
}

#endif

#if !_lib_renameat

#undef	STUB

int
renameat(int fwd, const char* fpath, int twd, const char* tpath)
{
	int	r;

	if (fwd == AT_FDCWD || *fpath == '/')
	{
		if (twd == AT_FDCWD || *tpath == '/')
			r = rename(fpath, tpath);
		else
		{
			ATBEG(twd, tpath, _fd_dir_rename);
			r = rename(fpath, ATPATH(twd, tpath));
			ATEND();
		}
	}
	else if (twd == AT_FDCWD || *tpath == '/')
	{
		ATBEG(fwd, fpath, _fd_dir_rename);
		r = rename(ATPATH(fwd, fpath), tpath);
		ATEND();
	}
	else
	{
#ifdef ATBEGL
		ATBEGL(twd, tpath, _fd_dir_rename);
		ATBEG(fwd, fpath, _fd_dir_rename);
		r = rename(ATPATH(fwd, fpath), ATPATH(twd, tpath));
		ATEND();
		ATEND();
#else
		errno = EINVAL;
		r = -1;
#endif
	}
	return r;
}

#endif

#if !_lib_symlinkat

#undef	STUB

int
symlinkat(const char* path, int cwd, const char* linkpath)
{
	int	r;

	ATBEG(cwd, linkpath, _fd_dir_symlink);
	r = symlink(path, ATPATH(cwd, linkpath));
	ATEND();
	return r;
}

#endif

#if !_lib_unlinkat

#undef	STUB

int
unlinkat(int cwd, const char* path, int flags)
{
	int	r;

	ATBEG(cwd, path, _fd_dir_unlink);
	r = (flags & AT_REMOVEDIR) ? rmdir(ATPATH(cwd, path)) : unlink(ATPATH(cwd, path));
	ATEND();
	return r;
}

#endif

#if STUB

void _STUB_at(){}

#endif
