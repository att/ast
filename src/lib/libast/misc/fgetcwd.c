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
 * Glenn Fowler
 * AT&T Research
 *
 * pwd library support
 */

#include <ast.h>

#if _UWIN

NoN(fgetcwd)

#else

#include <ast_dir.h>
#include <error.h>
#include <fs3d.h>

#ifndef ERANGE
#define ERANGE			E2BIG
#endif

#define ERROR(e)		{ errno = e; goto error; }

/*
 * return a pointer to the absolute path name of fd
 * fd must be an fd to a directory open for read 
 * the resulting path may be longer than PATH_MAX
 *
 * a few environment variables are checked before the search algorithm
 * return value is placed in buf of len chars
 * if buf is 0 then space is allocated via malloc() with
 * len extra chars after the path name
 * 0 is returned on error with errno set as appropriate
 *
 * NOTE: the !_lib_fdopendir version is neither thread nor longjump safe
 */

char*
fgetcwd(int fd, char* buf, size_t len)
{
	register char*	p;
	register char*	s;
	DIR*		dirp = 0;
	int		dd;
	int		f = FS3D_OFF;
	int		n;
	int		x;
	size_t		namlen;
	ssize_t		extra = -1;
	struct dirent*	entry;
	struct stat*	cur;
	struct stat*	par;
	struct stat*	tmp;
	struct stat	curst;
	struct stat	parst;
	struct stat	tstst;
	char		tst[PATH_MAX];

	static struct
	{
		char*	name;
		char*	path;
		dev_t	dev;
		ino_t	ino;
	}		env[] =
	{
		{ /*previous*/0	},
		{ "PWD"		},
		{ "HOME"	},
	};

	if (buf && !len)
		ERROR(EINVAL);
	cur = &curst;
	par = &parst;
	if (fstatat(fd, ".", par, 0))
		ERROR(errno);
	f = fs3d(f);
	for (n = 0; n < elementsof(env); n++)
		if ((env[n].name && (p = getenv(env[n].name)) || (p = env[n].path)) && *p == '/')
		{
			Pathdev_t	dev;

			if ((!pathdev(AT_FDCWD, p, NiL, 0, PATH_DEV, &dev) || dev.fd < 0) && !stat(p, cur))
			{
				env[n].path = p;
				env[n].dev = cur->st_dev;
				env[n].ino = cur->st_ino;
				if (cur->st_ino == par->st_ino && cur->st_dev == par->st_dev)
				{
					namlen = strlen(p);
					namlen++;
					if (buf)
					{
						if (len < namlen)
							ERROR(ERANGE);
					}
					else if (!(buf = newof(0, char, namlen, len)))
						ERROR(ENOMEM);
					if (f != FS3D_OFF)
						fs3d(f);
					return (char*)memcpy(buf, p, namlen);
				}
			}
		}
	if (!buf)
	{
		extra = len;
		len = PATH_MAX;
		if (!(buf = newof(0, char, len, extra)))
			ERROR(ENOMEM);
	}
	p = buf + len - 1;
	*p = 0;
	n = elementsof(env);
#if !_lib_fdopendir
	if ((dd = fd) != AT_FDCWD && fchdir(dd))
		ERROR(errno);
#endif
	for (;;)
	{
		tmp = cur;
		cur = par;
		par = tmp;
#if _lib_fdopendir
		if ((dd = openat(fd, "..", O_RDONLY|O_NONBLOCK|O_DIRECTORY|O_CLOEXEC)) < 0)
			ERROR(errno);
		if (fstat(dd, par))
			ERROR(errno);
		if (par->st_dev == cur->st_dev && par->st_ino == cur->st_ino)
		{
			close(dd);
			*--p = '/';
			break;
		}
		if (dirp)
			closedir(dirp);
		if (!(dirp = fdopendir(dd)))
			ERROR(errno);
		fd = dd;
#else
		if (stat("..", par))
			ERROR(errno);
		if (par->st_dev == cur->st_dev && par->st_ino == cur->st_ino)
		{
			*--p = '/';
			break;
		}
		if (chdir(".."))
			ERROR(errno);
		if (dirp)
			closedir(dirp);
		if (!(dirp = opendir(".")))
			ERROR(errno);
#endif
#ifdef D_FILENO
		if (par->st_dev == cur->st_dev)
		{
			while (entry = readdir(dirp))
				if (D_FILENO(entry) == cur->st_ino)
				{
					namlen = D_NAMLEN(entry);
					goto part;
				}

			/*
			 * this fallthrough handles logical naming
			 */

			rewinddir(dirp);
		}
#endif
		do
		{
			if (!(entry = readdir(dirp)))
				ERROR(ENOENT);
		} while (fstatat(fd, entry->d_name, &tstst, AT_SYMLINK_NOFOLLOW) || tstst.st_ino != cur->st_ino || tstst.st_dev != cur->st_dev);
		namlen = D_NAMLEN(entry);
	part:
		if (*p)
			*--p = '/';
		while ((p -= namlen) <= (buf + 1))
		{
			x = (buf + len - 1) - (p + namlen);
			s = buf + len;
			if (extra < 0 || !(buf = newof(buf, char, len += PATH_MAX, extra)))
				ERROR(ERANGE);
			p = buf + len;
			while (p > buf + len - 1 - x)
				*--p = *--s;
		}
		if (n < elementsof(env))
		{
			memcpy(p, env[n].path, namlen);
			break;
		}
		if (namlen == 1 && entry->d_name[0] == '.')
		{
			p = buf + len - 1;
			*p = 0;
			*--p = '/';
			break;
		}
		memcpy(p, entry->d_name, namlen);
		for (n = 0; n < elementsof(env); n++)
			if (env[n].ino == par->st_ino && env[n].dev == par->st_dev)
			{
				namlen = strlen(env[n].path);
				goto part;
			}
	}
	if (p != buf)
	{
		s = buf;
		while (*s++ = *p++);
		len = s - buf;
		if (extra >= 0 && !(buf = newof(buf, char, len, extra)))
			ERROR(ENOMEM);
	}
	if (env[0].path)
		free(env[0].path);
	env[0].path = strdup(buf);
#if !_lib_fdopendir
	if (dd != AT_FDCWD)
		fchdir(dd);
#endif
	if (dirp)
		closedir(dirp);
	if (f != FS3D_OFF)
		fs3d(f);
	return buf;
 error:
	if (buf && extra >= 0)
		free(buf);
	if (dirp)
		closedir(dirp);
#if !_lib_fdopendir
	if (dd != AT_FDCWD)
		fchdir(dd);
#endif
	if (f != FS3D_OFF)
		fs3d(f);
	return 0;
}

#endif
