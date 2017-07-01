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
#include "FEATURE/lib"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char** argv)
{
#if defined(_fd_self_dir_fmt) || defined(_fd_pid_dir_fmt)
	int		dd;
	int		fd;
	int		r;
	char*		fmt;
	char		dir[256];
	char		dev[256];
	char		tst[256];
	char		dev_old[256];
	char		dir_old[256];
	char		dev_new[256];
	char		dir_new[256];
	struct stat	st;

	static char*	res[] = { "fails", "works" };

	snprintf(dir, sizeof(dir), "%s.dir", argv[1]);
	if (mkdir(dir, S_IRUSR|S_IWUSR|S_IXUSR))
		return 1;
	if ((dd = open(dir, O_RDONLY)) < 0)
		return 1;
	snprintf(dir_old, sizeof(dir_old), "%s/old", dir);
	if ((fd = open(dir_old, O_CREAT, S_IRUSR|S_IWUSR)) < 0)
		return 1;
	close(fd);
	snprintf(dir_new, sizeof(dir_new), "%s/new", dir);
#ifdef _fd_pid_dir_fmt
	fmt = _fd_pid_dir_fmt;
	snprintf(dev, sizeof(dev), fmt, getpid(), dd, "", "");
#else
	fmt = _fd_self_dir_fmt;
	snprintf(dev, sizeof(dev), fmt, dd, "", "");
#endif
	snprintf(dev_old, sizeof(dev_old), "%s/old", dev);
	snprintf(dev_new, sizeof(dev_new), "%s/new", dev);
#if !_lib_faccessat
	r = access(dev_old, 0) >= 0;
	printf("#define _fd_dir_access	%d	/* access(%s/old) %s */\n", r, fmt, res[r]);
#endif
#if !_lib_fchmodat
	r = chmod(dev_old, S_IRUSR|S_IWUSR|S_IXUSR) >= 0;
	printf("#define _fd_dir_chmod	%d	/* chmod(%s/old) %s */\n", r, fmt, res[r]);
#if _lib_lchmod && _lib_symlink
	if (r = symlink(dir_old, dir_new) >= 0)
	{
		r = lchmod(dev_new, S_IRUSR|S_IWUSR|S_IXUSR) >= 0;
		unlink(dir_new);
	}
	printf("#define _fd_dir_chmod	%d	/* lchmod(%s/old) %s */\n", r, fmt, res[r]);
#endif
#endif
#if !_lib_fchownat
	r = chown(dev_old, -1, -1) >= 0;
	printf("#define _fd_dir_chown	%d	/* chown(%s/old) %s */\n", r, fmt, res[r]);
#if _lib_lchown && _lib_symlink
	if (r = symlink(dir_old, dir_new) >= 0)
	{
		r = lchown(dev_new, -1, -1) >= 0;
		unlink(dir_new);
	}
	printf("#define _fd_dir_lchown	%d	/* lchown(%s/old) %s */\n", r, fmt, res[r]);
#endif
#endif
#if !_lib_fstatat
	r = stat(dev_old, &st) >= 0;
	printf("#define _fd_dir_stat	%d	/* stat(%s/old) %s */\n", r, fmt, res[r]);
#endif
#if !_lib_linkat
	if (r = link(dev_old, dev_new) >= 0)
		unlink(dev_new);
	printf("#define _fd_dir_link	%d	/* link(%s/old,%s/new) %s */\n", r, fmt, fmt, res[r]);
#endif
#if !_lib_mkdirat
	if (r = mkdir(dev_new, S_IRUSR|S_IWUSR|S_IXUSR) >= 0)
		rmdir(dir_new);
	printf("#define _fd_dir_mkdir	%d	/* mkdir(%s/new) %s */\n", r, fmt, res[r]);
#endif
#if !_lib_mknodat
#if _lib_mknod
	if (r = mknod(dev_new, S_IFREG, 0) >= 0)
		unlink(dir_new);
#else
	r = 0;
#endif
	printf("#define _fd_dir_mknod	%d	/* mknod(%s/new) %s */\n", r, fmt, res[r]);
#endif
#if !_lib_mkfifoat
#if _lib_mkfifo
	if (r = mkfifo(dev_new, S_IRUSR|S_IWUSR|S_IXUSR) >= 0)
		unlink(dir_new);
#elif _lib_mknod
	if (r = mknod(dev_new, S_IFIFO, 0) >= 0)
		unlink(dir_new);
#else
	r = 0;
#endif
	printf("#define _fd_dir_mkfifo	%d	/* mkfifo(%s/new) %s */\n", r, fmt, res[r]);
#endif
#if !_lib_openat
	if (r = (fd = open(dev_old, O_RDONLY)) >= 0)
		close(fd);
	printf("#define _fd_dir_open	%d	/* open(%s/old) %s */\n", r, fmt, res[r]);
#endif
#if !_lib_readlinkat
#if _lib_symlink && _lib_readlink
	if (r = symlink(dir_old, dir_new) >= 0)
	{
		r = readlink(dev_new, tst, sizeof(tst)) > 0;
		unlink(dir_new);
	}
#else
	r = 0;
#endif
	printf("#define _fd_dir_readlink	%d	/* readlink(%s/old) %s */\n", r, fmt, res[r]);
#endif
#if !_lib_renameat
	if (r = rename(dev_old, dev_new) >= 0)
		unlink(dev_new);
	printf("#define _fd_dir_rename	%d	/* rename(%s/old,%s/new) %s */\n", r, fmt, fmt, res[r]);
#endif
#if !_lib_symlinkat
#if _lib_symlink
	if (r = symlink(dir_old, dev_new) >= 0)
		unlink(dir_new);
#else
	r = 0;
#endif
	printf("#define _fd_dir_symlink	%d	/* symlink(old,%s/new) %s */\n", r, fmt, res[r]);
#endif
#if !_lib_unlinkat
	if (r = unlink(dev_old) >= 0)
		close(open(dir_old, O_CREAT, S_IRUSR|S_IWUSR));
	printf("#define _fd_dir_unlink	%d	/* unlink(%s/old) %s */\n", r, fmt, res[r]);
#endif
#endif
	return 0;
}
