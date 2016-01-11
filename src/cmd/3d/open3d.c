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

#include "3d.h"

/*
 * this does the actual open, creating versions as required
 */

static int
vcreate(register const char* path, int flags, int mode)
{
	register char*	cp;
	register char*	ep;
	int		dirlen;
	int		namlen;
	int		fd;
	int		r;
	char		buf[PATH_MAX+1];

	if (!state.vmap.size || !(flags & O_CREAT))
		return OPEN(path, flags, mode);
	ep = cp = (char*)path + strlen(path);
	if (!instance(state.path.name, cp, &state.path.st, 1))
		return OPEN(path, flags, mode);
	while (cp > (char*)path && *--cp != '/');
	if (*cp == '/') cp++;
	namlen = ep - cp;

	/*
	 * construct pathname for version instance name text into buf
	 */

	memcpy(buf, path, dirlen = cp - path);
	memcpy(buf + dirlen, state.opaque, 4);
	*ep = '/';
	strcpy(buf + dirlen + 4, cp);
	if ((fd = OPEN(buf, flags, mode)) >= 0)
		return fd;
	if (errno != ENOTDIR && errno != ENOENT)
		return -1;
	buf[dirlen + namlen + 4] = 0;
	if (MKDIR(buf, VERMODE))
	{
		buf[dirlen + 3] = 0;
		if (MKDIR(buf, VERMODE))
		{
			if (errno != EEXIST)
				return -1;
			if (LSTAT(buf, &state.path.st))
				return -1;

			/*
			 * check for old style opaque, can be removed soon
			 */

			if (S_ISREG(state.path.st.st_mode) && !(state.path.st.st_mode & S_IPERM))
			{
				char	savebuf[8];

				memcpy(savebuf, cp, sizeof(savebuf));
				strcpy(cp, "....");
				if (RENAME(buf, path))
				{
					errno = EEXIST;
					return -1;
				}
				if (MKDIR(buf, VERMODE))
					return -1;
				buf[dirlen + 3] = '.';
				buf[dirlen + 4] = 0;
				strcpy(cp, state.opaque);
				if (RENAME(buf, path))
				{
					errno = EEXIST;
					return -1;
				}
				memcpy(cp, savebuf, 8);
				buf[dirlen + 3] = 0;
				buf[dirlen + 4] = *cp;
			}
			else
			{
				errno = EEXIST;
				return -1;
			}
		}
		buf[dirlen + 3] = '/';
		if (MKDIR(buf, VERMODE))
			return -1;
	}
	buf[dirlen + namlen + 4] = '/';

	/*
	 * now create the file
	 */

	if ((fd = OPEN(buf, flags, mode)) < 0)
		return -1;
	strcpy(buf + dirlen + namlen + 5, state.vdefault);
	*ep = 0;
	r = RENAME(path, buf);
	if (SYMLINK(buf + dirlen, path))
		return fd;
	if (r)
	{
		strcpy(buf + dirlen + namlen + 5, ep + 1);
		LINK(buf, path);
	}
	return fd;
}

int
open3d(const char* path, int oflag, ...)
{
	register char*	sp = 0;
	register int	r;
	int		fd = -1;
	int		level;
	int		synthesize;
	mode_t		mode;
	struct stat	st;
	va_list		ap;
#if FS
	Mount_t*	mp;
#endif

	va_start(ap, oflag);
	mode = (oflag & O_CREAT) ? va_arg(ap, int) : 0;
	va_end(ap);
#if FS
	if (!fscall(NiL, MSG_open, 0, path, oflag, mode, 0))
	{
		message((-1, "DEBUG: fs open fd=%d", state.ret));
		return state.ret;
	}
	mp = monitored();
#endif
	if (state.in_2d)
	{
		if ((r = OPEN((!path || *path) ? path : state.dot, oflag, mode)) < 0 || !state.call.monitor || state.in_2d == 1 || FSTAT(r, &st))
			return r;
		goto done;
	}
	for (;;)
	{
		if (!(sp = pathreal(path, (oflag & O_CREAT) ? P_NOOPAQUE : 0, &st)))
		{
			if (oflag & O_CREAT)
			{
				Path_t	save;

				save = state.path;
				state.real++;
				sp = pathreal(path, P_PATHONLY|P_DOTDOT, NiL);
				state.real--;
				if (!sp)
				{
					state.path = save;
					errno = EROFS;
					return -1;
				}
				if (state.path.level && LSTAT(sp, &st))
				{
					if (LSTAT(state.dot, &st) || fs3d_mkdir(sp, st.st_mode & S_IPERM))
					{
						state.path = save;
						return -1;
					}
					state.path = save;
					continue;
				}
				state.path = save;
				sp = 0;
			}
			st.st_mode = 0;
		}
		else if (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode))
		{
			r = OPEN(path, oflag, mode);
			goto done;
		}
		break;
	}
	level = state.path.level;
	synthesize = state.path.synthesize;
#if VCS && defined(VCS_OPEN)
	VCS_OPEN(path, oflag, mode, &state.path.st);
#endif
	if ((oflag & O_CREAT) && (!sp || level || (oflag & O_EXCL)) || (sp || level) && ((oflag & O_TRUNC) || (oflag & O_ACCMODE) != O_RDONLY))
	{
		if (sp)
		{
			if ((oflag & O_ACCMODE) != O_RDONLY)
			{
				if (!level) 
				{
					r = OPEN(sp, oflag, mode);
					goto done;
				}
				if (!(oflag & O_TRUNC) && (fd = OPEN(sp, O_RDONLY, 0)) < 0)
					return -1;
			}
			mode = ((S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)|(st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)));
		}
		if (!(sp = pathreal(path, (oflag & O_CREAT) ? (P_PATHONLY|P_SAFE) : 0, NiL)))
		{
			if (fd >= 0)
				CLOSE(fd);
			return -1;
		}
		if (synthesize)
			fs3d_mkdir(state.pwd, S_IRWXU|S_IRWXG|S_IRWXO);
	}
	else if (!sp)
	{
		r = -1;
		goto done;
	}
	r = vcreate(sp, ((oflag & O_ACCMODE) == O_RDWR) ? (oflag|O_CREAT) : oflag, mode);
	if (r < 0 && errno == ENOENT && ((oflag & (O_CREAT|O_TRUNC)) || (oflag & O_ACCMODE) != O_RDONLY))
	{
		if (!(sp = pathreal(path, P_PATHONLY|P_NOOPAQUE, NiL)))
			r = -1;
		else if (state.level <= 0 || *sp == '/')
			r = vcreate(sp, O_CREAT|oflag, mode);
		else errno = ENOENT;
		if (r < 0 && errno == ENOENT && (sp = pathreal(path, P_DOTDOT, NiL)) && state.path.level)
		{
			if (sp = pathreal(path, P_DOTDOT|P_PATHONLY|P_NOOPAQUE, NiL))
				r = fs3d_mkdir(sp, S_IRWXU|S_IRWXG|S_IRWXO);
			if (!r)
			{
				/*
				 * try again
				 */

				if (!(sp = pathreal(path, P_PATHONLY|P_SAFE, NiL)))
					r = -1;
				else r = vcreate(sp, O_CREAT|oflag, mode);
			}
			else errno = ENOENT;
		}
	}
	if (r > 0)
	{
		if (level && fd >= 0)
		{
			if (fs3d_copy(fd, r, NiL))
			{
				CLOSE(r);
				CLOSE(fd);
				return -1;
			}
			state.path.level = 0;
		}
		state.path.open_level = state.path.level;
		setfdview(r, state.path.open_level);
	}
	if (fd >= 0)
		CLOSE(fd);
 done:
#if FS || defined(fchdir3d)
	if (r >= 0)
	{
#if defined(fchdir3d)
		if (S_ISDIR(st.st_mode) && r < elementsof(state.file))
		{
			Dir_t*	dp;

			if (state.file[r].dir)
			{
				free(state.file[r].dir);
				state.file[r].dir = 0;
			}
			if (dp = newof(0, Dir_t, 1, strlen(sp)))
			{
				strcpy(dp->path, sp);
				dp->dev = st.st_dev;
				dp->ino = st.st_ino;
				state.file[r].dir = dp;
			}
		}
#endif
#if FS
		level = state.path.level;
		if (state.cache)
		{
			if (!st.st_mode)
				FSTAT(r, &st);
			fileinit(r, &st, mp, 1);
		}
		if (mp)
			fscall(mp, MSG_open, r, state.path.name, oflag, st.st_mode, level);
		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_open))
				fscall(mp, MSG_open, r, state.path.name, oflag, st.st_mode, level);
#endif
	}
#endif
	return r;
}

/*
 * use this open() within 3d
 * to save and restore the path state
 */

int
fs3d_open(const char* path, int oflag, mode_t mode)
{
	int	fd;
	Path_t	save;

	save = state.path;
	fd = open(path, oflag, mode);
	state.path = save;
	return fd;
}

#if !_nosys_open64

int
open643d(const char* path, int oflag, ...)
{
	mode_t		mode;
	va_list		ap;

	va_start(ap, oflag);
	mode = (oflag & O_CREAT) ? va_arg(ap, int) : 0;
	va_end(ap);
#if defined(O_LARGEFILE)
	oflag |= O_LARGEFILE;
#endif
	return open(path, oflag, mode);
}

#endif
