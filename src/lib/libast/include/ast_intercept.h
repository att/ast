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
 * ast function intercepts
 */

#if !defined(_AST_INTERCEPT) && !_BLD_ast && _API_ast && _API_ast < 20130625 /* <ast_api.h> not in scope yet */
#define _AST_INTERCEPT			0
#endif

#ifndef _AST_INTERCEPT
#define _AST_INTERCEPT			1

#define AST_SERIAL_ENVIRON		1
#define AST_SERIAL_LOCALE		2
#define AST_SERIAL_RESTART		3

#define AST_SERIAL_get			0
#define AST_SERIAL_inc			1
#define AST_SERIAL_always		0xffffffff
#define AST_SERIAL_except		(AST_SERIAL_always-1)
#define AST_SERIAL_max			(AST_SERIAL_except-1)

#ifndef _AST_INTERCEPT_IMPLEMENT
#define _AST_INTERCEPT_IMPLEMENT	1

/* at*() intercepts */

#undef	faccessat
#define faccessat	ast_faccessat

#undef	fchmodat
#define	fchmodat	ast_fchmodat

#undef	fchownat
#define fchownat	ast_fchownat

#undef	fstatat
#define fstatat		ast_fstatat

#undef	linkat
#define linkat 		ast_linkat

#undef	mkdirat
#define mkdirat		ast_mkdirat

#undef	mkfifoat
#define mkfifoat	ast_mkfifoat

#undef	mknodat
#define mknodat		ast_mknodat

#undef	openat
#define openat		ast_openat

#undef	readlinkat
#define readlinkat	ast_readlinkat

#undef	renameat
#define renameat 	ast_renameat

#undef	symlinkat
#define symlinkat	ast_symlinkat

#undef	unlinkat
#define unlinkat	ast_unlinkat

/* restart intercepts */

#undef	access
#define access		ast_access

#undef	chdir
#define chdir		ast_chdir

#undef	chmod
#define chmod		ast_chmod

#undef	chown
#define chown		ast_chown

#undef	close
#define close		ast_close

#undef	creat
#define creat		ast_creat

#undef	dup
#define dup		ast_dup

#undef	dup2
#define dup2		ast_dup2

#undef	eaccess
#define eaccess		ast_eaccess

#undef	fchdir
#define fchdir		ast_fchdir

#undef	fchmod
#define fchmod		ast_fchmod

#undef	fchown
#define fchown		ast_fchown

/* XXX ast_fcntl() will at least work for all ast usage */

#undef	fcntl
#define fcntl		ast_fcntl

#undef	fstat
#define fstat		ast_fstat

#undef	ftruncate
#define ftruncate	ast_ftruncate

/* ast_ioctl brought in scope by <ast_ioctl.h> */

#undef	lchmod
#define lchmod		ast_lchmod

#undef	lchown
#define lchown		ast_lchown

#undef	link
#define link		ast_link

#undef	lstat
#define lstat		ast_lstat

#undef	mkdir
#define mkdir		ast_mkdir

#undef	mkfifo
#define mkfifo		ast_mkfifo

#undef	mknod
#define mknod		ast_mknod

#undef	open
#define open		ast_open

#undef	pipe
#define pipe		ast_pipe

#undef	pipe2
#define pipe2		ast_pipe2

#undef	readlink
#define readlink	ast_readlink

#undef	remove
#define remove		ast_remove

#undef	rename
#define rename		ast_rename

#undef	rmdir
#define rmdir		ast_rmdir

/* stat vs stat64 vs struct stat makes life interesting */
#ifdef stat
#undef	stat64
#define stat64(p,s)	ast_stat(p,s)
#else
#undef	stat
#define stat(p,s)	ast_stat(p,s)
#endif

#undef	symlink
#define symlink		ast_symlink

#undef	unlink
#define unlink		ast_unlink

/* socket intercepts done by <sys/socket.h> intercept header */

#endif

#if _AST_INTERCEPT_IMPLEMENT > 0

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern uint32_t		astserial(int, uint32_t);

extern int		ast_access(const char*, int);
extern int		ast_chdir(const char*);
extern int		ast_chmod(const char*, mode_t);
extern int		ast_chown(const char*, uid_t, gid_t);
extern int		ast_close(int);
extern int		ast_creat(const char*, mode_t);
extern int		ast_dup(int);
extern int		ast_dup2(int, int);
extern int		ast_eaccess(const char*, int);
extern int		ast_fchdir(int);
extern int		ast_fchmod(int, mode_t);
extern int		ast_fchown(int, gid_t, uid_t);
extern int		ast_fcntl(int, int, ...);
extern int		ast_fstat(int, struct stat*);
extern int		ast_ftruncate(int, off_t);
extern int		ast_lchmod(const char*, mode_t);
extern int		ast_lchown(const char*, uid_t, gid_t);
extern int		ast_link(const char*, const char*);
extern int		ast_mkdir(const char*, mode_t);
extern int		ast_mkfifo(const char*, mode_t);
extern int		ast_mknod(const char*, mode_t, dev_t);
extern int		ast_open(const char*, int, ...);
extern int		ast_pipe(int[2]);
extern int		ast_pipe2(int[2], int);
extern ssize_t		ast_readlink(const char*, char*, size_t);
extern int		ast_remove(const char*);
extern int		ast_rename(const char*, const char*);
extern int		ast_rmdir(const char*);
extern int		ast_symlink(const char*, const char*);
extern int		ast_stat(const char*, struct stat*);
extern int		ast_unlink(const char*);

extern int		ast_faccessat(int, const char*, mode_t, int);
extern int		ast_fchmodat(int, const char*, mode_t, int);
extern int		ast_fchownat(int, const char*, uid_t, gid_t, int);
extern int		ast_fstatat(int, const char*, struct stat*, int);
extern int		ast_linkat(int, const char*, int, const char*, int);
extern int		ast_lstat(const char*, struct stat*);
extern int		ast_mkdirat(int, const char*, mode_t);
extern int		ast_mkfifoat(int, const char*, mode_t);
extern int		ast_mknodat(int, const char*, mode_t, dev_t);
extern int		ast_openat(int, const char*, int, ...);
extern int		ast_renameat(int, const char*, int, const char*);
extern ssize_t		ast_readlinkat(int, const char*, char*, size_t);
extern int		ast_symlinkat(const char*, int, const char*);
extern int		ast_unlinkat(int, const char*, int);

#undef	extern

#endif

#endif
