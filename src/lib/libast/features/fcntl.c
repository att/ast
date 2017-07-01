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
 * generate POSIX fcntl.h
 */

#include "FEATURE/standards"	/* iffe --include-first */
#include "FEATURE/lib"

#include <sys/types.h>

#define getdtablesize	______getdtablesize
#define getpagesize	______getpagesize
#define ioctl		______ioctl

#if _typ_off64_t
#undef	off_t
#ifdef __STDC__
#define	off_t		off_t
#endif
#endif

#if _hdr_fcntl
#include <fcntl.h>
#endif
#if _hdr_unistd
#include <unistd.h>
#endif
#if _sys_socket
#include <sys/socket.h>
#endif

#include <sys/stat.h>

#include "FEATURE/fs"

#undef	getdtablesize   
#undef	getpagesize
#undef	ioctl

#include "FEATURE/tty"

#if _typ_off64_t
#undef	off_t
#define	off_t	off64_t
#endif

/*
 * some bit macros may in fact be bits
 */

static unsigned int
hibit(unsigned int m)
{
	unsigned int	b;

	while (b = m & (m - 1))
		m = b;
	return m;
}

#define ORIGIN_EXTENSION	"ast extension"
#define ORIGIN_FAILSAFE		"just in case _*_SOURCE circumvented"
#define ORIGIN_IGNORE		"not implemented"

int
main(int argc, char** argv)
{
	unsigned int	f_local = 0;
	unsigned int	f_local_use;
	unsigned int	f_lck = 0;
	unsigned int	o_local = 2;
	unsigned int	o_local_min;
	unsigned int	o_local_use;

	printf("#pragma prototyped\n");
	printf("\n");
	printf("#if _typ_off64_t\n");
	printf("#undef	off_t\n");
	printf("#ifdef __STDC__\n");
	printf("#define	off_t		off_t\n");
	printf("#endif\n");
	printf("#endif\n");
	printf("\n");
	printf("#include <ast_fs.h>\n");
	printf("\n");
	printf("#if _typ_off64_t\n");
	printf("#undef	off_t\n");
	printf("#ifdef __STDC__\n");
	printf("#define	off_t		off_t\n");
	printf("#endif\n");
	printf("#endif\n");
	printf("\n");
	printf("#include <fcntl.h>\n");
#if _hdr_mman
	printf("#include <mman.h>\n");
#else
#if _sys_mman
	printf("#include <sys/mman.h>\n");
#endif
#endif
	printf("\n");
#ifndef	FD_CLOEXEC
	printf("#define FD_CLOEXEC	1\n");
	printf("\n");
#endif

#ifdef	F_DUPFD
	if (F_DUPFD > f_local) f_local = F_DUPFD;
#endif
#ifdef	F_DUPFD_CLOEXEC
	if (F_DUPFD_CLOEXEC > f_local) f_local = F_DUPFD_CLOEXEC;
#else
#define NEED_F	1
#endif
#ifdef	F_GETFD
	if (F_GETFD > f_local) f_local = F_GETFD;
#endif
#ifdef	F_GETFL
	if (F_GETFL > f_local) f_local = F_GETFL;
#endif
#ifdef	F_GETLK
	if (F_GETLK > f_local) f_local = F_GETLK;
#endif
#ifdef	F_RDLCK
	if (F_RDLCK > f_lck) f_lck = F_RDLCK;
#endif
#ifdef	F_SETFD
	if (F_SETFD > f_local) f_local = F_SETFD;
#endif
#ifdef	F_SETFL
	if (F_SETFL > f_local) f_local = F_SETFL;
#endif
#ifdef	F_SETLK
	if (F_SETLK > f_local) f_local = F_SETLK;
#endif
#ifdef	F_SETLKW
	if (F_SETLKW > f_local) f_local = F_SETLKW;
#endif
#ifdef	F_UNLCK
	if (F_UNLCK > f_lck) f_lck = F_UNLCK;
#endif
#ifdef	F_WRLCK
	if (F_WRLCK > f_lck) f_lck = F_WRLCK;
#endif
#ifdef	F_EXLCK
	if (F_EXLCK > f_local) f_local = F_EXLCK;
#endif
#ifdef	F_GETLEASE
	if (F_GETLEASE > f_local) f_local = F_GETLEASE;
#endif
#ifdef	F_GETLK64
	if (F_GETLK64 > f_local) f_local = F_GETLK64;
#endif
#ifdef	F_GETOWN
	if (F_GETOWN > f_local) f_local = F_GETOWN;
#endif
#ifdef	F_GETOWN_EX
	if (F_GETOWN_EX > f_local) f_local = F_GETOWN_EX;
#endif
#ifdef	F_GETSIG
	if (F_GETSIG > f_local) f_local = F_GETSIG;
#endif
#ifdef	F_NOTIFY
	if (F_NOTIFY > f_local) f_local = F_NOTIFY;
#endif
#ifdef	F_SETLEASE
	if (F_SETLEASE > f_local) f_local = F_SETLEASE;
#endif
#ifdef	F_SETLK64
	if (F_SETLK64 > f_local) f_local = F_SETLK64;
#endif
#ifdef	F_SETLKW64
	if (F_SETLKW64 > f_local) f_local = F_SETLKW64;
#endif
#ifdef	F_SETOWN
	if (F_SETOWN > f_local) f_local = F_SETOWN;
#endif
#ifdef	F_SETOWN_EX
	if (F_SETOWN_EX > f_local) f_local = F_SETOWN_EX;
#endif
#ifdef	F_SETSIG
	if (F_SETSIG > f_local) f_local = F_SETSIG;
#endif
#ifdef	F_SHLCK
	if (F_SHLCK > f_local) f_local = F_SHLCK;
#endif
#ifdef	F_SHARE
	if (F_SHARE > f_local) f_local = F_SHARE;
#endif
#ifdef	F_UNSHARE
	if (F_UNSHARE > f_local) f_local = F_UNSHARE;
#endif
#ifdef	F_BADFD /* Create Poison FD */
	if (F_BADFD > f_local) f_local = F_BADFD;
#endif

#if	NEED_F
#if	_lib_fcntl
	printf("#define _lib_fcntl	1\n");
#endif

	/*
	 * pick a range for ast F_* local extensions that has a chance
	 * of remaining constant over time
	 */

	if (f_local < 30000)
		f_local = 30000;
	else if (f_local < 1000000)
		f_local = 1000000;
	else
		f_local++;
#ifndef F_DUPFD_CLOEXEC
	f_local_use = f_local + 0;
	printf("#define F_DUPFD_CLOEXEC	(%d)	/* %s */\n", f_local, ORIGIN_EXTENSION);
#endif
	printf("#define _ast_F_LOCAL	%d	/* %s up to %d */\n", f_local, ORIGIN_EXTENSION, f_local_use);
#endif
#ifdef F_DUPFD_CLOEXEC
	printf("#ifndef F_DUPFD_CLOEXEC\n");
	printf("#define F_DUPFD_CLOEXEC	(%d)	/* %s */\n", (int)F_DUPFD_CLOEXEC, ORIGIN_FAILSAFE);
	printf("#endif\n");
#endif
	printf("\n");

#ifdef	O_APPEND
	if (O_APPEND > o_local) o_local = O_APPEND;
#endif
#ifdef	O_CREAT
	if (O_CREAT > o_local) o_local = O_CREAT;
#endif
#ifdef	O_EXCL
	if (O_EXCL > o_local) o_local = O_EXCL;
#endif
#ifdef	O_NOCTTY
	if (O_NOCTTY > o_local) o_local = O_NOCTTY;
#endif
#ifdef	O_NONBLOCK
	if (O_NONBLOCK > o_local) o_local = O_NONBLOCK;
#endif
#ifdef	O_TRUNC
	if (O_TRUNC > o_local) o_local = O_TRUNC;
#endif
#ifdef O_BLKSEEK
	if (O_BLKSEEK > o_local) o_local = O_BLKSEEK;
#endif
#ifdef O_LARGEFILE
	if (O_LARGEFILE > o_local) o_local = O_LARGEFILE;
#endif
#ifdef O_LARGEFILE128
	if (O_LARGEFILE128 > o_local) o_local = O_LARGEFILE128;
#endif
#ifdef O_NOATIME
	if (O_NOATIME > o_local) o_local = O_NOATIME;
#endif
#ifdef O_NOFOLLOW
	if (O_NOFOLLOW > o_local) o_local = O_NOFOLLOW;
#endif
#ifdef O_NOLINKS
	if (O_NOLINKS > o_local) o_local = O_NOLINKS;
#endif
#ifdef O_PRIV
	if (O_PRIV > o_local) o_local = O_PRIV;
#endif
#ifdef O_DSYNC
	if (O_DSYNC > o_local) o_local = O_DSYNC;
#endif
#ifdef O_RSYNC
	if (O_RSYNC > o_local) o_local = O_RSYNC;
#endif
#ifdef O_SYNC
	if (O_SYNC > o_local) o_local = O_SYNC;
#endif
#ifdef O_TEMPORARY
	if (O_TEMPORARY > o_local) o_local = O_TEMPORARY;
#endif
#ifdef O_TMPFILE
	if (O_TMPFILE > o_local) o_local = O_TMPFILE;
#endif
#ifdef O_XATTR
	if (O_XATTR > o_local) o_local = O_XATTR;
#endif
#ifdef O_DIRECT
	if (O_DIRECT > o_local) o_local = O_DIRECT;
#endif
#ifndef O_DIRECTORY
#ifdef	O_OPENDIR
#define O_DIRECTORY		O_OPENDIR
#define O_DIRECTORY_origin	"O_OPENDIR"
#endif
#else
	if (O_DIRECTORY > o_local) o_local = O_DIRECTORY;
#endif
#ifdef O_OPENDIR
	if (O_OPENDIR > o_local) o_local = O_OPENDIR;
#endif
#ifndef O_SEARCH
#ifdef	O_PATH
#define O_SEARCH	O_PATH
#define O_SEARCH_origin	"O_PATH"
#endif
#else
	if (O_SEARCH > o_local) o_local = O_SEARCH;
#endif
#ifdef O_PATH
	if (O_PATH > o_local) o_local = O_PATH;
#endif
#ifdef O_EXEC
	if (O_EXEC > o_local) o_local = O_EXEC;
#endif
#ifdef O_CLOEXEC
	if (O_CLOEXEC > o_local) o_local = O_CLOEXEC;
#endif
#ifdef O_NDELAY
	if (O_NDELAY > o_local) o_local = O_NDELAY;
#endif
#ifdef O_TTY_INIT
	if (O_TTY_INIT > o_local) o_local = O_TTY_INIT;
#endif

	/*
	 * O_DIRECTORY and O_SEARCH are problematic and get special treatment.
	 * For some reason some systems make it hard to coax these values
	 * from the standard headers. This rigmarole makes sure they appear
	 * in the ast headers if they are implemented by the kernel, even absent
	 * native definitions. If you touch this code make sure you get it right.
	 *
	 * O_*_native are native implementation specific values
	 * that this file compilation may have failed to coax
	 * out of the native compilation system which ast
	 * nontheless would still like to use
	 *
	 * based on the assumption that the native system
	 * honors binary compatibility, subsequent code uses
	 * these values to probe the actual system call
	 * to determine if the feature is implemented anyway
	 */

#if __linux__
#define O_DIRECTORY_native	00200000
#define O_SEARCH_native		010000000,020000000,0100000000
/* #elif __some_other_os_that_wont_implement_posix_by_default__ */
#endif

	{
		unsigned int	o_directory = 0;
		unsigned int	o_search = 0;
		char*		o_directory_origin = ORIGIN_EXTENSION;
		char*		o_search_origin = ORIGIN_IGNORE;
#ifdef	O_SEARCH
		o_search = O_SEARCH;
#ifndef O_SEARCH_origin
#define O_SEARCH_origin		ORIGIN_FAILSAFE
#endif
		o_search_origin = O_SEARCH_origin;
#elif defined(O_SEARCH_native)
		{
			int		i;
			char		tmp[256];
			unsigned int	o_search_native[] = { O_SEARCH_native };
			snprintf(tmp, sizeof(tmp), "%s.s", argv[1]);
			if (!mkdir(tmp, S_IXUSR|S_IXGRP|S_IXOTH))
				for (i = 0; i < (int)(sizeof(o_search_native)/sizeof(o_search_native[0])); i++)
					if (!close(open(tmp, o_search_native[i])))
					{
						o_search_origin = "kernel bits otherwise undefined";
						o_search = o_search_native[i];
						if (o_search > o_local)
							o_local = o_search;
						break;
					}
		}
#endif
#ifdef	O_DIRECTORY
		o_directory = O_DIRECTORY;
#ifndef O_DIRECTORY_origin
#define O_DIRECTORY_origin	ORIGIN_FAILSAFE
#endif
		o_directory_origin = O_DIRECTORY_origin;
#elif defined(O_DIRECTORY_native)
		{
			int		i;
			char		tmp[256];
			unsigned int	o_directory_native[] = { O_DIRECTORY_native };
			snprintf(tmp, sizeof(tmp), "%s.d", argv[1]);
			if (!mkdir(tmp, S_IXUSR|S_IXGRP|S_IXOTH))
				for (i = 0; i < (int)(sizeof(o_directory_native)/sizeof(o_directory_native[0])); i++)
					if (!close(open(tmp, O_DIRECTORY_native)))
					{
						snprintf(tmp, sizeof(tmp), "%s.f", argv[1]);
						if (!close(open(tmp, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR)) && close(open(tmp, o_directory_native[i]|O_RDONLY))))
						{
							o_directory_origin = "kernel bits otherwise undefined";
							o_directory = o_directory_native[i];
							if (o_directory > o_local)
								o_local = o_directory;
						}
			}
		}
#endif
		o_local_min = hibit(o_local);
		o_local_use = o_local = 020000000000;

		printf("#define O_INTERCEPT		%012o	/* %s */\n", o_local, ORIGIN_EXTENSION);
		printf("#ifndef O_SEARCH\n");
		if (o_search)
			printf("#define O_SEARCH		%012o	/* %s */\n", o_search, o_search_origin);
		else
			printf("#define O_SEARCH		0		/* %s */\n", o_search_origin);
		printf("#endif\n");
		if (!o_directory)
			o_local_use = o_directory = o_local >> 1;
		printf("#ifndef O_DIRECTORY\n");
		printf("#define O_DIRECTORY		%012o	/* %s */\n", o_directory, o_directory_origin);
		printf("#endif\n");

	}

	printf("#ifndef O_CLOEXEC\n");
#ifdef O_CLOEXEC
	printf("#define O_CLOEXEC		%012o	/* %s */\n", (int)O_CLOEXEC, ORIGIN_FAILSAFE);
#else
	o_local_use = o_local >> 2;
	printf("#define O_CLOEXEC		%012o	/* %s */\n", o_local_use, ORIGIN_EXTENSION);
#endif
	printf("#endif\n");

#ifndef	O_NOCTTY
#ifdef	TIOCNOTTY
	printf("#ifndef O_NOCTTY\n");
	o_local_use = o_local >> 3;
	printf("#define O_NOCTTY		%012o	/* %s */\n", o_local_use, ORIGIN_EXTENSION);
	printf("#endif\n");
#endif
#endif
#ifndef	O_NONBLOCK
	printf("#ifndef O_NONBLOCK\n");
	printf("#ifdef  O_NDELAY\n");
#ifdef O_NDELAY
	printf("#define O_NONBLOCK		%012o	/* O_NDELAY */\n", (int)O_NDELAY);
#else
	printf("#define O_NONBLOCK		O_NDELAY\n");
#endif
	printf("#else\n");
	printf("#ifdef  FNDELAY\n");
#ifdef FNDELAY
	printf("#define O_NONBLOCK		%012o	/* FNDELAY */\n", FNDELAY);
#else
	printf("#define O_NONBLOCK		FNDELAY\n");
#endif
	printf("#else\n");
	o_local_use = o_local >> 4;
	printf("#define O_NONBLOCK		%012o	/* %s */\n", o_local_use, ORIGIN_EXTENSION);
	printf("#endif\n");
	printf("#endif\n");
	printf("#endif\n");
#endif

	printf("\n");
	printf("#define _ast_O_LOCAL		%012o	/* %s up to %012o%s */\n", o_local_use, ORIGIN_EXTENSION, o_local, o_local_use <= o_local_min ? " *** encroaches on native O_* flags ***" : "");
	printf("\n");

#ifndef	O_NDELAY
	printf("#ifndef O_NDELAY\n");
	printf("#define O_NDELAY		O_NONBLOCK\n");
	printf("#endif\n");
#endif
#ifndef	O_ACCMODE
	printf("#define O_ACCMODE		(O_RDONLY|O_WRONLY|O_RDWR)\n");
#endif
#ifndef	O_BINARY
	printf("#define O_BINARY		0		/* %s */\n", ORIGIN_IGNORE);
#endif
#ifdef O_DIRECT
	printf("#ifndef O_DIRECT\n");
	printf("#define O_DIRECT		%012o	/* %s */\n", (int)O_DIRECT, ORIGIN_FAILSAFE);
	printf("#endif\n");
#endif
#ifdef O_NOFOLLOW
	printf("#ifndef O_NOFOLLOW\n");
	printf("#define O_NOFOLLOW		%012o	/* %s */\n", (int)O_NOFOLLOW, ORIGIN_FAILSAFE);
	printf("#endif\n");
#endif
#ifdef O_NOATIME
	printf("#ifndef O_NOATIME\n");
	printf("#define O_NOATIME		%012o	/* %s */\n", (int)O_NOATIME, ORIGIN_FAILSAFE);
	printf("#endif\n");
#endif
#ifndef	O_TEMPORARY
	printf("#define O_TEMPORARY		0		/* %s */\n", ORIGIN_IGNORE);
#endif
#ifndef	O_TEXT
	printf("#define O_TEXT			0		/* %s */\n", ORIGIN_IGNORE);
#endif
#if !defined(SOCK_CLOEXEC) || !defined(SOCK_NONBLOCK)
	printf("\n");
#ifndef SOCK_CLOEXEC
	printf("#ifndef SOCK_CLOEXEC\n");
	printf("#define _ast_SOCK_CLOEXEC	1\n");
	printf("#define SOCK_CLOEXEC		02000000 /* %s */\n", ORIGIN_EXTENSION);
	printf("#endif\n");
#endif
#ifndef SOCK_NONBLOCK
	printf("#ifndef SOCK_NONBLOCK\n");
	printf("#define _ast_SOCK_NONBLOCK	1\n");
	printf("#define SOCK_NONBLOCK		04000	/* %s */\n", ORIGIN_EXTENSION);
	printf("#endif\n");
#endif
#endif

	printf("\n");
	printf("#define F_dupfd_cloexec		F_DUPFD_CLOEXEC /* OBSOLETE */\n");
	printf("#define O_cloexec		O_CLOEXEC /* OBSOLETE*/\n");
#if !defined(AT_FDCWD) || !defined(AT_SYMLINK_NOFOLLOW) || !defined(AT_REMOVEDIR) || !defined(AT_SYMLINK_FOLLOW) || !defined(AT_EACCESS)
	printf("\n");
#ifndef AT_FDCWD
	/* AT_FDCWD must be < -256 for portability reasons */
	printf("#define AT_FDCWD		-666	/* %s */\n", ORIGIN_EXTENSION);
#endif
#ifndef AT_SYMLINK_NOFOLLOW
	printf("#define AT_SYMLINK_NOFOLLOW	0x100	/* %s */\n", ORIGIN_EXTENSION);
#endif
#ifndef AT_REMOVEDIR
	printf("#define AT_REMOVEDIR		0x200	/* %s*/\n", ORIGIN_EXTENSION);
#endif
#ifndef AT_SYMLINK_FOLLOW
	printf("#define AT_SYMLINK_FOLLOW	0x400	/* %s*/\n", ORIGIN_EXTENSION);
#endif
#ifndef AT_EACCESS
	printf("#define AT_EACCESS		0x800	/* %s*/\n", ORIGIN_EXTENSION);
#endif
#endif
	printf("\n");
	printf("#include <ast_fs.h>\n");
	printf("#if _typ_off64_t\n");
	printf("#undef	off_t\n");
	printf("#define	off_t		off64_t\n");
	printf("#endif\n");
	printf("#if _lib_fstat64\n");
	printf("#define fstat		fstat64\n");
	printf("#endif\n");
	printf("#if _lib_fstatat64\n");
	printf("#define fstatat		fstatat64\n");
	printf("#endif\n");
	printf("#if _lib_lstat64\n");
	printf("#define lstat		lstat64\n");
	printf("#endif\n");
	printf("#if _lib_stat64\n");
	printf("#define stat		stat64\n");
	printf("#endif\n");
	printf("#if _lib_creat64\n");
	printf("#define creat		creat64\n");
	printf("#endif\n");
	printf("#if _lib_mmap64\n");
	printf("#define mmap		mmap64\n");
	printf("#endif\n");
	printf("#if _lib_open64\n");
	printf("#undef	open\n");
	printf("#define open		open64\n");
	printf("#endif\n");

	printf("\n");
	printf("#if _BLD_ast && defined(__EXPORT__)\n");
	printf("#define extern	__EXPORT__\n");
	printf("#endif\n");
	printf("#if !_lib_faccessat\n");
	printf("extern int	faccessat(int, const char*, mode_t, int);\n");
	printf("#endif\n");
	printf("#if !_lib_fchmodat\n");
	printf("extern int	fchmodat(int, const char*, mode_t, int);\n");
	printf("#endif\n");
	printf("#if !_lib_fchownat\n");
	printf("extern int	fchownat(int, const char*, uid_t, gid_t, int);\n");
	printf("#endif\n");
	printf("#if !_lib_fstatat\n");
	printf("struct stat;\n");
	printf("extern int	fstatat(int, const char*, struct stat*, int);\n");
	printf("#endif\n");
	printf("#if !_lib_linkat\n");
	printf("extern int	linkat(int, const char*, int, const char*, int);\n");
	printf("#endif\n");
	printf("#if !_lib_mkdirat\n");
	printf("extern int	mkdirat(int, const char*, mode_t);\n");
	printf("#endif\n");
	printf("#if !_lib_mkfifoat\n");
	printf("extern int	mkfifoat(int, const char*, mode_t);\n");
	printf("#endif\n");
	printf("#if !_lib_mknodat\n");
	printf("extern int	mknodat(int, const char*, mode_t, dev_t);\n");
	printf("#endif\n");
	printf("#if !_lib_openat\n");
	printf("extern int	openat(int, const char*, int, ...);\n");
	printf("#endif\n");
	printf("#if !_lib_readlinkat\n");
	printf("extern ssize_t	readlinkat(int, const char*, char*, size_t);\n");
	printf("#endif\n");
	printf("#if !_lib_renameat\n");
	printf("extern int	renameat(int, const char*, int, const char*);\n");
	printf("#endif\n");
	printf("#if !_lib_symlinkat\n");
	printf("extern int	symlinkat(const char*, int, const char*);\n");
	printf("#endif\n");
	printf("#if !_lib_unlinkat\n");
	printf("extern int	unlinkat(int, const char*, int);\n");
	printf("#endif\n");
	printf("\n");
	printf("#undef	extern\n");

	return 0;
}
