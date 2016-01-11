/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2013 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped

#include <cmd.h>
#include <ls.h>

static const char optsync[] =
"[-?\n@(#)$Id: sync (AT&T Research) 2013-09-22 $\n]"
USAGE_LICENSE
"[+NAME?sync - schedule file/file system updates]"
"[+DESCRIPTION?\bsync\b(1) transfers buffered modifications of file "
    "metadata and data to the storage device for a specific file, a specific "
    "filesystem, or all filesystems.]"
"[+?At minimum \bsync\b (with no options) should be called before "
    "halting the system. Most systems provide graceful shutdown procedures "
    "that include \bsync\b -- use them if possible.]"

"[f:sfsync?Calls \bsfsync\b(3) to flush all buffered \asfio\a stream data.]"
"[s:fsync?Calls \bfsync\b(2) using the open file descriptor \afd\a to "
    "transfer all data associated with \afd\a to the storage device. "
    "\bsync\b waits until the transfer completes or an error is "
    "detected.]#[fd]"
"[S:syncfs?Calls \bsyncfs\b(2) using the open file descriptor \afd\a to "
    "transfer all data for the file system containing the file referred to "
    "by \afd\a. Depending on the native system implementation \bsync\b may "
    "return before the data is actually written. Implies \b--sfsync\b.]#[fd]"
"[X:sync|all?Calls \bsync\b(2) to transfer all data for all filesystems. "
    "Depending on the native system implementaion the writing, although "
    "scheduled, is not necessarily complete upon return from \bsync\b. Since "
    "\bsync\b(2) has no failure indication, \bsync\b only fails for "
    "option/operand syntax errors, or when \bsync\b(2) does not return, in "
    "which case \bsync\b(1) also does not return. Implies \b--sfsync\b. This "
    "is the default when no options are specified.]"

"[+SEE ALSO?\bfsync\b(2), \bsync\b(2), \bsyncfs\b(2), \bsfsync\b(3), \bshutdown\b(8)]"
;

#ifndef ENOSYS
#define ENOSYS	EINVAL
#endif

#if !_lib_fsync
int
fsync(int fd)
{
	if (fcntl(fd, F_GETFL) >= 0)
		errno = ENOSYS;
	return -1;
}
#endif

#if !_lib_sync
int
sync(void)
{
	errno = ENOSYS;
	return -1;
}
#endif

#if !_lib_syncfs
int
syncfs(int fd)
{
	if (fcntl(fd, F_GETFL) >= 0)
		errno = ENOSYS;
	return -1;
}
#endif

int
b_sync(int argc, char** argv, Shbltin_t* context)
{
	int	fsync_fd = -1;
	int	syncfs_fd = -1;
	bool	do_sfsync = 0;
	bool	do_sync = 0;

	NoP(context);
	for (;;)
	{
		switch (optget(argv, optsync))
		{
		case 'f':
			do_sfsync = 1;
			continue;
		case 's':
			fsync_fd = opt_info.num;
			break;
		case 'S':
			syncfs_fd = opt_info.num;
			do_sfsync = 1;
			break;
		case 'X':
			do_sync = 1;
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || *argv)
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (fsync_fd == -1 && syncfs_fd == -1)
		do_sync = do_sfsync = 1;
	if (do_sfsync && sfsync(NiL) < 0)
		error(ERROR_system(0), "sfsync(0) failed");
	if (fsync_fd >= 0 && fsync(fsync_fd) < 0)
		error(ERROR_system(0), "fsync(%d) failed", fsync_fd);
	if (syncfs_fd >= 0 && syncfs(syncfs_fd) < 0)
		error(ERROR_system(0), "syncfs(%d) failed", syncfs_fd);
	if (do_sync)
		sync();
	return error_info.errors != 0;
}
