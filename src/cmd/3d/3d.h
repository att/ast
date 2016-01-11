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

/*
 * Glenn Fowler
 * David Korn
 * Eduardo Krell
 *
 * AT&T Research
 *
 * 3d file system implementation interface
 */

#ifndef _3D_H
#define _3D_H

#undef	_BLD_DEBUG

#define _def_map_ast	1
#define _std_strtol	1

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide fchown ftruncate mount readlink sbrk strmode truncate utimes __utimes
#else
#undef	fchown
#define fchown		______fchown
#undef	ftruncate
#define ftruncate	______ftruncate
#undef	mount
#define mount		______mount
#undef	readlink
#define readlink	______readlink
#undef	sbrk
#define sbrk		______sbrk
#undef	strmode
#define strmode		______strmode
#undef	truncate
#define truncate	______truncate
#undef	utimes
#define utimes		______utimes
#undef	__utimes
#define __utimes	________utimes
#endif

#if defined(__STDPP__directive) && defined(__STDPP__note)
__STDPP__directive pragma pp:note off64_t
#endif

#include <ast_std.h>

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide strmode
#else
#undef	strmode
#endif

#include "lib_3d.h"

#include <ls.h>
#include <sig.h>
#include <times.h>
#include <error.h>
#include <debug.h>
#include <hashkey.h>
#include <dlldefs.h>

#if _lib_strerror && defined(strerror)
#undef	strerror
extern char*		strerror();
#endif

#undef	mount
#define mount		______mount
#include <fs3d.h>
#undef	mount

#include "msg.h"

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide fchown ftruncate mount readlink sbrk strmode truncate utimes __utimes
#else
#undef	fchown
#undef	ftruncate
#undef	mount
#undef	readlink
#undef	sbrk
#undef	strmode
#undef	truncate
#undef	utimes
#undef	__utimes
#endif

#if FS && defined(MIX)
#define LICENSED	1
#endif

#if defined(__STDPP__directive) && defined(__STDPP__note)
#if !_typ_off64_t && !noticed(off64_t)
#undef	off64_t
#if _typ_int64_t
#define off64_t		int64_t
#else
#define off64_t		long
#endif
#endif
#endif

struct stat64;

#undef	fcntl
#undef	open
#undef	stat
#undef	strtol
#undef	strtoul
#undef	strtoll
#undef	strtoull

#define MSG_read3d	(MSG_read|MSG_EXT(MSG_ARG_number,4))
#define MSG_write3d	(MSG_write|MSG_EXT(MSG_ARG_number,4))

struct Fs;

typedef long (*Sysfunc_t)();

typedef int (*Fs_get_t)(struct Fs*, char*, const char*, int);

typedef int (*Fs_set_t)(struct Fs*, const char*, int, const char*, int);

#define _3D_LINK_MAX	999		/* for nice ls -l */

#ifndef _3D_LINK_MAX
#ifdef LINK_MAX
#define _3D_LINK_MAX	LINK_MAX
#else
#define _3D_LINK_MAX	30000
#endif
#endif

#define ATTR_MAX	121

#include "FEATURE/peek"
#include "FEATURE/syscall"
#include "FEATURE/syslib"

typedef struct Fs
{
	long		flags;
	short		fd;
	short		retry;
	short		specialsize;
	Fs_get_t	get;
	Fs_set_t	set;
	unsigned long	key;
	unsigned long	ack;
	unsigned long	call;
	unsigned long	terse;
	char		special[10];
	char		attr[ATTR_MAX];
	struct stat	st;
	char*		match;
	char*		service;
	short		matchsize;
	short		servicesize;
} Fs_t;

#if FS

typedef struct Mount
{
	long		channel;
	Fs_t*		fs;
	struct Mount*	global;
	char*		logical;
	char*		physical;
	char		attr[ATTR_MAX];
	short		logicalsize;
	short		physicalsize;
	short		flags;
} Mount_t;

typedef struct
{
	dev_t		dev;
	ino_t		ino;
	char		path[1];
} Dir_t;

#endif

typedef struct
{
#if FS
	Mount_t*	mount;
	Dir_t*		dir;
#endif
	short*		reserved;
	Msg_file_t	id;
	unsigned long	open;
	unsigned char	flags;
	unsigned char	oflag;
} File_t;

#if VCS
#include "vcs_3d.h"
#endif

#define NOSYS		nosys

#define TABLE_FD	(OPEN_MAX>22?22:(OPEN_MAX-1))	/* 0 for env	*/
#define TABLE_MAX	(1<<(TABBITS+7))/* max 3d table image size	*/
#define TABLE_PREFIX	"vpath "	/* for external consistency	*/
#define TABLE_VERSION	1		/* to verify TABLE_FD		*/

#define TABBITS		5	/* max vpath,vmap table index bits (<6)	*/
#define TABSIZE		(1<<TABBITS)			/* v* tab size	*/
#define	VERMODE		(S_IRWXU|S_IRWXG|S_IRWXO)	/* ver dir mode	*/
#define INSTANCE	TABSIZE				/* under ver	*/

#define INTERCEPT_MAX	8		/* max # call intercept layers	*/

#define LICENSE_ALL	'+'		/* everything licensed		*/
#define LICENSE_NONE	'-'		/* nothing licensed		*/
#define LICENSE_SEP	','		/* feature separator		*/

/*
 * File_t flags
 */

#define RESERVED_FD	10		/* reserved fds >= this		*/

#define FILE_CLOEXEC	(1<<0)
#define FILE_ERROR	(1<<1)
#define FILE_LOCK	(1<<2)
#define FILE_OPEN	(1<<3)
#define FILE_REGULAR	(1<<4)
#define FILE_WRITE	(1<<5)
#define FILE_VIRTUAL	(1<<6)

#define FILE_LOCAL	(1<<8)

/*
 * map flags
 */

#define MAP_EXEC	(1<<0)		/* get info for exec		*/
#define MAP_INIT	(1<<1)		/* get info for init		*/

/*
 * Mount_t flags
 */

#define MOUNT_PRIMARY	(1<<0)

/*
 * pathreal() flags
 */

#define P_ABSOLUTE	(1<<0)		/* no relative conversion	*/
#define P_DOTDOT	(1<<1)		/* add /.. to end of path	*/
#define P_LSTAT		(1<<2)		/* lstat() calling		*/
#define P_NOOPAQUE	(1<<3)		/* remove if opaque object	*/
#define P_NOSLASH	(1<<4)		/* remove trailing /'s		*/
#define P_PATHONLY	(1<<5)		/* just canonicalize path	*/
#define P_READLINK	(1<<6)		/* readlink() calling		*/
#define P_SAFE		(1<<7)		/* must be in safe tree		*/
#define P_SLASH		(1<<8)		/* add trailing /		*/
#define P_TOP		(1<<9)		/* copy to top level		*/

#ifndef ENOTEMPTY
#define ENOTEMPTY	EEXIST
#endif

#define T_ALLOCATE	(1<<15)
#define T_PREFIX	(1<<14)
#define T_DELETE	(T_ALLOCATE|T_PREFIX)
#define T_SIZE		(T_PREFIX-1)

#define T_VALSIZE(p)	(((p)->valsize&T_SIZE)?((p)->valsize&T_SIZE):strlen((p)->val))

#define T_MOUNT		(T_SIZE-0)

#define FS_null		0
#define FS_option	1
#define FS_view		2
#define FS_pwd		3
#define FS_fs		4
#define FS_map		5
#define FS_safe		6
#define FS_fd		7
#define FS_intercept	8

#define FS_ACTIVE	(1<<0)
#define FS_BOUND	(1<<1)
#define FS_CLOSE	(1<<2)
#define FS_ERROR	(1<<3)
#define FS_FLUSH	(1<<4)
#define FS_FORK		(1<<5)
#define FS_FS		(1<<6)
#define FS_GLOBAL	(1<<7)
#define FS_INIT		(1<<8)
#define FS_INTERACTIVE	(1<<9)
#define FS_INTERNAL	(1<<10)
#define FS_LICENSED	(1<<11)
#define FS_LOAD		(1<<12)
#define FS_LOCK		(1<<13)
#define FS_MAGIC	(1<<14)
#define FS_MONITOR	(1L<<15)
#define FS_NAME		(1L<<16)
#define FS_ON		(1L<<17)
#define FS_OPEN		(1L<<18)
#define FS_RAW		(1L<<19)
#define FS_RECEIVE	(1L<<20)
#define FS_REFERENCED	(1L<<21)
#define FS_REGULAR	(1L<<22)
#define FS_UNIQUE	(1L<<23)
#define FS_VALIDATED	(1L<<24)
#define FS_WRITE	(1L<<25)

#define FS_LOCAL	(1L<<26)

#define cancel(p)	(*(p)>=RESERVED_FD?(state.file[*(p)].reserved=0,CLOSE(*(p)),*(p)=0):(-1))
#define fsfd(p)		((((p)->flags&(FS_ERROR|FS_ON|FS_OPEN))==(FS_ON|FS_OPEN))?(p)->fd:((((p)->flags&(FS_BOUND|FS_ERROR|FS_INIT|FS_ON))==(FS_BOUND|FS_ON))?fsinit(p,-1):-1))
#define fson(p)		(((p)->flags&(FS_BOUND|FS_ERROR|FS_ON))==(FS_BOUND|FS_ON))
#define initialize()	do { if (!state.pid) init(0,0,0); else if (state.control.note) control(); } while(0)
#define monitored()	((state.path.monitor&&!state.path.mount)?state.path.monitor:(Mount_t*)0)
#define mounted()	((state.path.monitor&&state.path.mount)?state.path.monitor:(Mount_t*)0)
#if _hdr_alloca
#define reclaim()
#else
#define reclaim()	do { if (state.brk.beg) fix(); } while (0)
#endif
#define reserve(p)	((state.file[*(p)].flags|=FILE_LOCK,*(state.file[*(p)].reserved=(p)))<RESERVED_FD?close(*(p)):0)

#if FS
#define fgetmount(f)	((f)>=0&&(f)<=state.cache?state.file[f].mount:(Mount_t*)0)
#define fsmonitored(p)	((state.path.monitor=getmount((p),NiL))&&(state.path.monitor->fs->flags&(FS_ERROR|FS_MONITOR|FS_ON))==(FS_MONITOR|FS_ON))
#define fsmount(p)	(((p)->channel&&(p)->fs->fd)?((p)->channel==-1?-1:(p)->fs->fd):fschannel(p))
#define fssys(p,c)	(fson((p)->fs)&&(((p)->fs->call&MSG_MASK(c))||((p)->fs->flags&FS_GLOBAL)&&(fsmount(p),0)))
#endif

#define var_3d		"__="
#define var_pwd		"PWD="
#define var_shell	"SHELL="
#define var_view	"VPATH="

typedef struct
{
	char*		key;
	char*		val;
	unsigned short	keysize;
	unsigned short	valsize;
} Map_t;

typedef struct
{
	unsigned int	size;
	Map_t		table[TABSIZE];
} Table_t;

typedef struct
{
	int		level;
	int		open_level;
	int		synthesize;
	int		nlinks;
	char*		linkname;
	int		linksize;
#if FS
	Mount_t*	monitor;
#endif
	char*		mount;
	Table_t*	table;
#if VCS && defined(VCS_PATH_STATE)
	VCS_PATH_STATE;
#endif
	struct stat	st;
	char		name[4 * PATH_MAX];
} Path_t;

typedef struct
{
	Fs_t*		fs;
	char*		prefix;
	int		prelen;
} Visit_t;

struct Intercept_s; typedef struct Intercept_s Intercept_t;

typedef long (*Intercept_f)(Intercept_t*, int, int, void*, void*, void*, void*, void*, void*);

struct Intercept_s
{
	Intercept_f		call;
	unsigned long		mask;
};

#if _no_exit_exit
typedef void (*Exitfunc_t)(int);
#endif

typedef struct
{
	/* the first elements are explicitly initialized */

	const char*	id;
	char*		cmd;
	char		dot[2];
	char		null[1];
	char		one[2];
	char*		binsh;
	char		env3d[sizeof(var_3d)];
	char		envpwd[sizeof(var_pwd) + 4 * PATH_MAX];
	char		envshell[sizeof(var_shell) + PATH_MAX];
	char		envview[sizeof(var_view)];
	Fs_t		fs[TABSIZE];
	char		vdefault[8];
	char		opaque[8];
	int		limit;

	/* the rest are implicitly initialized */

	struct
	{
	short		monitor;
	short		name;
	}		call;
	struct
	{
	short		fd;
	unsigned long	internal;
	}		channel;
	struct
	{
	int		note;
	int		pathsize;
	char*		path;
	}		control;
#if VCS && defined(VCS_STATE)
	VCS_STATE;
#endif
	int		boundary;
	struct
	{
	char*		beg;
	char*		end;
	}		brk;
	int		cache;
	char**		env;
	char*		envpath;
	void*		freedirp;
	int		in_2d;
	gid_t		gid;
	int		kernel;
	int		level;
	char		license[64];
	int		open;
	pid_t		pid;
	char*		pwd;
	int		pwdsize;
	char		pwdbuf[4 * PATH_MAX + 1];
	long		ret;
	Fs_t*		safe;
	char*		shell;
	uid_t		uid;
	struct
	{
	char*		value;
	char*		invert;
	char*		next;
	int		valsize;
	int		invertsize;
	}		key;
	struct
	{
	short		fd;
	short		version;
	int		size;
	char		buf[TABLE_MAX];
	}		table;
	unsigned long	test;
	struct
	{
	unsigned long	call;
	short		count;
	pid_t		pid;
	}		trace;
	Table_t		vmap;
	Table_t		vmount;
	Table_t		vpath;
	Table_t		vsafe;
	Table_t		vintercept;
	File_t		file[OPEN_MAX];
	Visit_t		visit;
	Path_t		path;
	int		real;
	struct
	{
	int		size;
	Intercept_t	intercept[INTERCEPT_MAX];
	}		trap;
#if _no_exit_exit
	Exitfunc_t	libexit;
#endif
#if FS
	Mount_t*	global;
	Mount_t		mount[2 * TABSIZE];
#endif
} State_t;

#include "std_3d.h"

extern State_t		state;

extern int		bprintf(char**, char*, const char*, ...);
extern ssize_t		bvprintf(char**, char*, const char*, va_list);
extern void		calldump(char**, char*);
extern void		callinit(void);
extern int		checklink(const char*, struct stat*, int);
extern void		control(void);
extern int		fs3d_copy(int, int, struct stat*);
extern int		fs3d_dup(int, int);
extern int		fs3d_mkdir(const char*, mode_t);
extern int		fs3d_open(const char*, int, mode_t);
extern void		fsdrop(Fs_t*, int);
extern int		fsinit(Fs_t*, int);
extern unsigned long	getkey(const char*, const char*, int);
extern int		init(int, const char*, int);
extern int		instance(char*, char*, struct stat*, int);
extern int		iterate(Table_t*, int(*)(Map_t*, char*, int), char*, int);
extern int		keep(const char*, size_t, int);
extern int		mapdump(Table_t*, char*, int);
extern int		mapget(Map_t*, char*, int);
extern int		mapinit(const char*, int);
extern int		mapset(Table_t*, const char*, int, const char*, int);
extern int		nosys(void);
extern char*		pathcanon(char*, size_t, int);
extern char*		pathcat(const char*, int, const char*, const char*, char*, size_t);
extern char*		pathreal(const char*, int, struct stat*);
extern ssize_t		peek(int, void*, size_t);
extern Map_t*		search(Table_t*, const char*, int, const char*, int);
extern Sysfunc_t	sysfunc(int);
extern int		intercept(Intercept_f, unsigned long);

#if FS
extern int		fileinit(int, struct stat*, Mount_t*, int);
extern int		fscall(Mount_t*, long, int, ...);
extern int		fschannel(Mount_t*);
extern char*		fsreal(Mount_t*, long, const char*);
extern int		getattr(const char*, char*);
extern Mount_t*		getmount(const char*, const char**);
extern const char*	setattr(char*, const char*, const char*);
#endif

/*
 * non-standard standards
 */

#if ! _UWIN
extern void*		sbrk(ssize_t);
#endif

/*
 * backwards compatibility fixups
 * these should go away in 94 (ha!)
 */

#if _int_st_spare1
#define NIVIEW(p,v)	((p)->st_spare1=(v))
#else
#if _ary_st_spare4
#define NIVIEW(p,v)	((p)->st_spare4[0]=(v))
#else
#if _ary_st_pad4
#define NIVIEW(p,v)	((p)->st_pad4[0]=(v))
#else
#define NIVIEW(p,v)
#endif
#endif
#endif

#undef	IVIEW

#if _mem_st_rdev_stat
#define IVIEW(p,v)	do{NIVIEW(p,(v)&(INSTANCE-1));if(!S_ISBLK((p)->st_mode)&&!S_ISCHR((p)->st_mode))(p)->st_rdev=(v)&(INSTANCE-1);}while(0)
#else
#define IVIEW(p,v)	NIVIEW(p,(v)&(INSTANCE-1))
#endif

#endif
