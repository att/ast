/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2012 AT&T Intellectual Property          *
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

#include "3d.h"

#ifndef MAXSYMLINKS
#define MAXSYMLINKS	20
#endif

/*
 * if <sp> is a hard link to state.opaque in the same directory then the file
 * is opaque and the errno should be ENOENT
 */

static int
checkopaque(register char* path, struct stat* st)
{
	register char*	basesp;
	int		oerrno = errno;
	struct stat	statb;
	char		savebuf[sizeof(state.opaque)];
	register int	r = 0;

	if (st->st_nlink <= 1)
		return 0;
	if (st->st_mode & (S_IRWXU|S_IRWXG|S_IRWXO))
		return 0;

	/*
	 * change the basename to state.opaque
	 */

	if (basesp = strrchr(path, '/'))
		basesp++;
	else
		basesp = path;
	memcpy(savebuf, basesp, sizeof(state.opaque));
	strcpy(basesp, state.opaque);
	if (LSTAT(path, &statb))
	{
		/*
		 * for backward compatability
		 */

		basesp[3] = 0;
		if (LSTAT(path, &statb))
			goto not_opaque;
	}
	if (statb.st_ino == st->st_ino && statb.st_dev == st->st_dev)
	{
		errno = ENOENT;
		r = statb.st_ino;
	}
	else
		errno = oerrno;
 not_opaque:
	memcpy(basesp, savebuf, sizeof(state.opaque));
	return r;
}

/*
 * return real path name for path
 */

#if DEBUG

static char*	_pathreal(const char*, int, struct stat*);

char*
pathreal(const char* apath, register int type, struct stat* st)
{
	char*	path = (char*)apath;

	initialize();
	message((-5, "pathreal: ++ %s type=|%s%s%s%s%s%s%s%s%s%s%s", path
		, (type & P_ABSOLUTE) ? "ABSOLUTE|" : state.null
		, (type & P_DOTDOT) ? "DOTDOT|" : state.null
		, (type & P_LSTAT) ? "LSTAT|" : state.null
		, st ? "MYSTAT|" : state.null
		, (type & P_NOOPAQUE) ? "NOOPAQUE|" : state.null
		, (type & P_NOSLASH) ? "NOSLASH|" : state.null
		, (type & P_PATHONLY) ? "PATHONLY|" : state.null
		, (type & P_READLINK) ? "READLINK|" : state.null
		, (type & P_SAFE) ? "SAFE|" : state.null
		, (type & P_SLASH) ? "SLASH|" : state.null
		, (type & P_TOP) ? "TOP|" : state.null
		));
	path = _pathreal(path, type, st);
	message((-5, "pathreal: -- %s level=%d links=%d", path, state.path.level, state.path.nlinks));
	return path;
}

#undef	initialize
#define initialize()
#undef	pathreal
#define pathreal _pathreal
static

#endif

char*
pathreal(const char* apath, register int type, struct stat* st)
{
	char*			path = (char*)apath;
	register char*		sp;
	register char*		cp;
	register char*		ip;
	Table_t*		safe;
	int			oerrno = errno;
	int			opaqued = 0;
	int			len;
	int			vir;
	int			safesize;
	int			safe_dir;
	long			visits;
	char			buf[PATH_MAX + 1];

	static struct stat	stbuf;
	static struct stat	tsbuf;

	state.path.level = state.path.synthesize = state.path.nlinks = 0;
	if (!path)
	{
		errno = EFAULT;
		return 0;
	}
	initialize();
	if (state.in_2d)
	{
		if (!st || (!state.level || *path == '/') && !LSTAT(path, st))
			return path;
		if (state.level && streq(path, ".") && !CHDIR(state.pwd))
		{
			state.level = 0;
			return path;
		}
		return 0;
	}
#if FS
	if (mounted() && (sp = fsreal(state.path.monitor, MSG_stat, state.path.mount)))
		apath = (char*)(path = sp);
#endif

	/*
	 * handle null path, . and / separately
	 */

	if (safe = state.safe ? &state.vsafe : (Table_t*)0)
	{
		type |= P_ABSOLUTE;
		if (!(safesize = state.safe->servicesize))
			safesize = strlen(state.safe->service);
	}
	else
		type &= ~P_SAFE;
 again:
	if (!*path)
	{
		errno = ENOENT;
		return 0;
	}
	cp = sp = path;
	state.path.synthesize = state.path.linksize = 0;
	if (!st)
		st = &stbuf;

	/*
	 * check if virtual dir has been created by another process
	 * only P_PATHONLY|P_TOP calls (usually create or modify link) and
	 * references to "." are checked for performance
	 */

	if (state.level > 0 && state.pwd && ((type & (P_PATHONLY|P_TOP)) && *sp != '/' || *sp == '.' && sp[1] == 0))
	{
		if (!CHDIR(state.pwd))
			state.level = 0;
		else if (!(type & (P_PATHONLY|P_TOP)))
		{
			len = 0;
			state.path.level += (state.path.synthesize = state.level);
			sp = strcpy(state.path.name, state.pwd);
			goto skip;
		}
	}
	if (!state.pwd || sp[1] == 0 && (*sp == '.' || *sp == '/' && !safe))
	{
		if (st != &stbuf && LSTAT(sp, st))
			return 0;
		if (*sp == '/' || !state.pwd && (type & P_PATHONLY))
			strncpy(state.path.name, sp, PATH_MAX);
		else if (!state.pwd)
		{
			/*
			 * treat the current directory as if were empty
			 */

			errno = ENOENT;
			return 0;
		}
		else
			strncpy(state.path.name, state.pwd, PATH_MAX);
		errno = oerrno;
		return state.path.name;
	}

	/*
	 * put absolute pathname into state.path
	 */

	safe_dir = 0;
	if (*path != '/')
	{
		strcpy(state.path.name, state.pwd);
		sp = state.path.name + state.pwdsize;
		*sp++ = '/';
		if (safe && state.pwdsize >= safesize && !strncmp(state.pwd, state.safe->service, safesize) && (!state.pwd[safesize] || state.pwd[safesize] == '/'))
			safe_dir = safesize;
	}
	else
		sp = state.path.name;
	ip = state.path.name + elementsof(state.path.name);
	while (sp < ip && (*sp = *cp++))
		sp++;
	if (type & P_DOTDOT)
		strcpy(sp, "/..");
	sp = state.path.name;
	if (!(ip = pathcanon(sp + safe_dir, sizeof(state.path.name) - safe_dir, 0)))
	{
		errno = ENOENT;
		return 0;
	}
	if (type & (P_DOTDOT|P_NOSLASH))
	{
		/*
		 * remove trailing slashes
		 */

		while (*--ip == '/');
		*++ip = 0;
	}
	else if ((type & P_SLASH) && *(ip - 1) != '/')
		*ip++ = '/';
	if (*(ip - 1) == '/' && ip - sp > 1)
	{
		/*
		 * trailing slash is equivalent to trailing slash-dot
		 * this forces the common-sense interpretation
		 */
#if DEBUG
		if (!(state.test & 010))
#endif
		*ip++ = '.';
		*ip = 0;
	}
	len = ip - sp;

	/*
	 * try to use relative path
	 */

	if (!(type & (P_LSTAT|P_READLINK)))
	{
		for (ip = state.pwd; *ip && *ip == *sp++; ip++);
		if (*ip != 0 || *sp && *sp != '/' || state.level < 0)
			sp = state.path.name;
		else
		{
			state.path.level += (state.path.synthesize = state.level);
			if (state.level && !(type & P_PATHONLY) && st == &stbuf)
			{
				sp = state.path.name;
				len -= state.pwdsize;
			}
			else if (type & P_ABSOLUTE)
				sp = state.path.name;
			else if (*sp == '/')
				sp++;
		}
		if (*sp == 0)
			sp = state.dot;
	}
 skip:
	if ((type & P_NOOPAQUE) && !LSTAT(sp, st) && checkopaque(sp, st))
	{
		message((-1, "%s: remove opaque", sp));
		UNLINK(sp);
		opaqued = 1;
	}
	if (safe && *sp == '/')
	{
		state.path.table = safe;
		cp = pathnext(sp, NiL, NiL);
		state.path.table = safe = 0;
		if (cp)
		{
			state.path.level = 0;
			path = strcpy(buf, sp);
			message((-5, "pathreal: == safe map %s", path));
			type &= ~(P_DOTDOT|P_SAFE);
			goto again;
		}
		if (!*(sp + 1))
		{
			strncpy(sp, state.safe->service, safesize);
			sp[safesize] = 0;
		}
		else if (strncmp(sp, state.safe->service, safesize) || sp[safesize] && sp[safesize] != '/')
		{
			if (*path != '/' && safe_dir)
			{
				errno = EPERM;
				return 0;
			}
			if (sp[1])
				strcpy(buf, sp);
			else
				*buf = 0;
			len = sfsprintf(sp, sizeof(state.path.name), "%-*s%s", safesize, state.safe->service, buf);
			message((-5, "pathreal: == safe next %s", sp));
			if (!pathnext(sp, NiL, NiL))
			{
				errno = EPERM;
				return 0;
			}
		}
		else
			type &= ~P_SAFE;
	}
	if ((type & P_SAFE) && state.path.level)
	{
		errno = EPERM;
		return 0;
	}
	if (type & P_PATHONLY)
	{
		errno = oerrno;
		return sp;
	}
	visits = 0;
	vir = 1;
	while (LSTAT(sp, st))
	{
		if (vir)
		{
			if (apath[0] == '.' && apath[1] == '.' && apath[2] == '.' && !apath[3])
			{
				if (state.level > 0)
				{
					message((-1, "pathreal: %s => %s", apath, sp));
					LSTAT(".", st);
					return sp;
				}
				errno = ENOENT;
				return 0;
			}
			vir = 0;
		}
		if (errno == ENOTDIR)
		{
			/*
			 * check for version instance
			 */

			cp = ip = sp + strlen(sp);
			while (ip > sp && *--ip != '/');
			if (ip < sp)
				return 0;
			while (ip > sp && *--ip == '/');
			if (ip < sp)
				return 0;
			while (ip > sp && *--ip != '/');
			if (*ip == '/')
				ip++;
			while (cp >= ip)
			{
				cp[4] = *cp;
				cp--;
			}
			memcpy(ip, state.opaque, 4);
			if (!LSTAT(sp, st))
				break;
			errno = ENOTDIR;
			return 0;
		}

		if (errno != ENOENT || opaqued)
			return 0;
#if FS
		/*
		 * check user mount
		 */

		if (visits)
		{
			Mount_t*	mp;
			const char*	up;

			if ((mp = getmount(sp, &up)) && (mp->fs->flags & FS_NAME) && (sp = fsreal(mp, MSG_open, (char*)up)) && !LSTAT(sp, st))
				break;
		}
#endif

		/*
		 * search down the viewpath
		 */

		if (type & P_SAFE)
		{
			errno = EPERM;
			return 0;
		}
		if (!pathnext(state.path.name, NiL, &visits))
			return 0;
		sp = state.path.name;
		if (!(type & P_ABSOLUTE))
		{
			/*
			 * try to use relative path
			 */

			for (ip = state.pwd; *ip && *ip == *sp++; ip++);
			if (*ip == 0 && *sp == '/')
				sp++;
			else
				sp = state.path.name;
		}
		if (*sp == 0)
			sp = state.dot;
	}
	if (st->st_nlink > 1 && checkopaque(sp, st))
		return 0;
	if ((type & P_TOP) && state.path.level)
	{
		int	rfd;
		int	wfd;

		if ((rfd = OPEN(sp, O_RDONLY, 0)) < 0)
			sp = 0;
		else
		{
			tsbuf = *st;
			wfd = open(apath, O_WRONLY|O_CREAT|O_TRUNC|O_cloexec, st->st_mode & S_IPERM);
			*st = tsbuf;
			if (wfd < 0)
				sp = 0;
			else 
			{
				if (fs3d_copy(rfd, wfd, st))
					sp = 0;
				CLOSE(wfd);
			}
			CLOSE(rfd);
		}
		if (!sp)
		{
			errno = EROFS;
			return 0;
		}
		if (st == &stbuf)
			st = 0;
		return pathreal(apath, P_PATHONLY, st);
	}
	IVIEW(st, state.path.level);
	if (state.path.synthesize)
	{
		if (state.path.level < state.level)
		{
			if (len)
			{
				ip  = state.path.name + strlen(state.path.name) - len;
				len = *ip;
				*ip = 0;
			}
			if (!CHDIR(state.path.name))
				state.level = state.path.level;
			message((-1, "chdir=%s level=%d", state.path.name, state.level));
			*ip = len;
		}
		else if (S_ISDIR(st->st_mode))
		{
			int		mask;
			static int	uid = -1;
			static int	gid;

			umask(mask = umask(0));
			st->st_mode = (st->st_mode | (S_IRWXU|S_IRWXG|S_IRWXO)) & ~(mask & (S_IRWXU|S_IRWXG|S_IRWXO));
			if (uid == -1)
			{
				uid = geteuid();
				gid = getegid();
			}
			st->st_uid = uid;
			st->st_gid = gid;
		}
	}
	ip = sp;

	/*
	 * symbolic links handled specially
	 * get filename from pathname
	 */

	if (S_ISLNK(st->st_mode) && (len = checklink(sp, st, type)) > 1 && !(type & (P_LSTAT|P_READLINK)) && state.path.nlinks++ < MAXSYMLINKS)
	{
		path = strcpy(buf, state.path.name);
		message((-1, "pathreal: == again %s", path));
		if (*path != '/')
			state.path.level = 0;
		type &= ~(P_DOTDOT|P_SAFE);
		goto again;
	}
#if VCS && defined(VCS_REAL)
	VCS_REAL(state.path.name, st);
#endif
	errno = oerrno;
	return sp;
}

/*
 * check whether sp points to a version object and find version instance
 * sp is canonicalized and points into state.path
 * when called from unlink, (type & P_PATHONLY) is set
 *     -1 for non-existent link
 *     length of path for a relative link that is not a version object
 *     0  otherwise
 *    state.path.linkname and state.path.linksize are set for version object
 */

int
checklink(const char* asp, struct stat* st, int type)
{
	register char*	sp = (char*)asp;
	register char*	ip;
	register int	len;
	register int	n;
	register char*	bp;

	char		buf[PATH_MAX + 1];

	if (sp < state.path.name || sp >= state.path.name + sizeof(state.path.name))
	{
		message((-1, "AHA#%d checklink bounds sp=%p state.path.name=%p sp=%s", __LINE__, sp, state.path.name, sp));
		sp = strncpy(state.path.name, sp, sizeof(state.path.name) - 1);
	}
	while (S_ISLNK(st->st_mode))
	{
		/*
		 * go to the last component
		 */

		if (ip = strrchr(sp, '/'))
			ip++;
		else
			ip = sp;
		strcpy(buf, ip);
		len = (state.path.name + sizeof(state.path.name) - 1) - ip;
		if ((len = READLINK(sp, ip, len)) < 0)
		{
			message((-1, "%s: cannot readlink", sp));
			return 0;
		}
		state.path.linkname = ip;
		state.path.linksize = len;

		/*
		 * check for relative link
		 */

		if (*ip != '/')
		{
			ip[len] = 0;
			if (*ip == *state.opaque && !memcmp(ip, state.opaque, 4) && !memcmp(ip + 4, buf, n = strlen(buf)))
			{
				/*
				 * version object
				 */

				ip += n + 4;
				if (instance(state.path.name, ip, st, 0))
				{
					state.path.linksize = strlen(state.path.linkname);
					if (type & P_LSTAT)
					{
						st->st_size = state.path.linksize;
						st->st_mode &= S_IPERM;
#ifdef S_IFLNK
						st->st_mode |= S_IFLNK;
#endif
						return 0;
					}
					continue;
				}
				errno = ENOENT;
				return -1;
			}
			else if (!(type & (P_LSTAT|P_PATHONLY|P_READLINK)) && *ip == '.' && *(ip + 1) == '.' && (*(ip + 2) == '/' || *(ip + 2) == 0))
			{
				memcpy(buf, ip, len + 1);
				bp = state.path.name;
				while (ip > bp && *(ip - 1) == '/')
					ip--;
				for (;;)
				{
					*(sp = ip) = 0;
					while (ip > bp && *--ip != '/');
					while (ip > bp && *(ip - 1) == '/')
						ip--;
					if (*ip == '/')
						ip++;
					if ((n = READLINK(state.path.name, ip, PATH_MAX - (ip - state.path.name))) <= 0)
					{
						*sp++ = '/';
						state.path.linkname = (char*)memcpy(sp, buf, len + 1);
						return sp + len - state.path.name;
					}
					if (*ip == '/')
						ip = (char*)memcpy(bp = state.path.name, ip, n);
					else if (ip > bp)
						*(ip - 1) = '/';
					ip += n;
				}
			}
		}

		/*
		 * restore last component
		 */

		if (!(type & P_READLINK))
			strcpy(ip, buf);
		break;
	}
	return 0;
}
