/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * connect stream interface definitions
 */

#ifndef _CS_H
#define _CS_H

#define CS_VERSION	19970717L

#include <ast.h>
#include <ast_cs.h>
#include <hashpart.h>
#include <ls.h>
#include <times.h>

#define cs		_cs_info_
#define Cs		(&cs)

#define CSTIME()	(cs.time=(unsigned long)time(NiL))
#define CSTOSS(n,c)	HASHPART(n,c)

#define CS_LOCAL	(-1L)		/* local address		*/

#define CS_NEVER	(-1L)		/* no timeout			*/

#define CS_EXACT	0x0001		/* csread() exact amount	*/
#define CS_LIMIT	0x0002		/* csread() amount limit	*/
#define CS_LINE		0x0004		/* csread() one or more lines	*/
#define CS_RESTART	0x1000		/* restart on interrupt		*/

#define CS_PORT_NORMAL		0x00000000	/* alloc normal port	*/
#define CS_PORT_MIN		0x00000001	/* min real port	*/
#define CS_PORT_MAX		0x0000ffff	/* max real port	*/
#define CS_PORT_INVALID		0xffffffff	/* invalid port		*/
#define CS_PORT_RESERVED	0xfffffffe	/* alloc reserved port	*/

#define CS_NORMAL	CS_PORT_NORMAL
#define CS_RESERVED	CS_PORT_RESERVED

#define CS_HOST_GATEWAY	"ftp"		/* local net gateway host	*/
#define CS_HOST_LOCAL	"local"		/* local host			*/
#define CS_HOST_PROXY	"proxy"		/* local net proxy host		*/
#define CS_HOST_SHARE	"share"		/* any local net host		*/
#define CS_HOST_UNKNOWN	"unknown"	/* unknown host			*/

#define CS_STAT_DAEMON	"lib/ssd"	/* system stat daemon rel name	*/
#define CS_STAT_DIR	"share/lib/ss"	/* system stat dir rel path	*/
#define CS_STAT_DOWN	(3*CS_STAT_FREQ)/* assumed down after . secs	*/
#define CS_STAT_FREQ	50		/* avg update frequency in secs	*/
#define CS_STAT_IGNORE	(24*60*60)	/* ignore user after . idle	*/

#define CS_SVC_ACCESS	"access"	/* local host access		*/
#define CS_SVC_DIR	"lib/cs"	/* service directory		*/
#define CS_SVC_DORMANT	(5*60)		/* min dormant secs		*/
#define CS_SVC_HOSTS	"hosts"		/* hosts that can run service	*/
#define CS_SVC_INFO	"local"		/* local host info		*/
#define CS_SVC_REMOTE	"remote"	/* remote gateway info		*/
#define CS_SVC_SUFFIX	".svc"		/* service daemon suffix	*/

#define CS_MNT_MAX	14		/* max mount file name length	*/
#define CS_NAME_MAX	256		/* misc name max length		*/

#define CS_MNT_TAIL	"-cs.mnt"	/* mount file name tail		*/

#define CS_MNT_AUTH	'A'		/* client authentication	*/
#define CS_MNT_LOCK	'X'		/* mount lock			*/
#define CS_MNT_LOG	'L'		/* error log			*/
#define CS_MNT_OLDLOG	'O'		/* previous error log		*/
#define CS_MNT_OTHER	'#'		/* no auth suffix		*/
#define CS_MNT_PROCESS	'P'		/* process id			*/
#define CS_MNT_STREAM	'S'		/* connect stream		*/
#define CS_MNT_TMP	'Z'		/* temporary			*/

#define CS_REMOTE_CONTROL	"cs"
#define CS_REMOTE_DEBUG		'd'
#define CS_REMOTE_DELAY		8
#define CS_REMOTE_OPEN		'O'
#define CS_REMOTE_OPEN_AGENT	'A'
#define CS_REMOTE_OPEN_LOCAL	'L'
#define CS_REMOTE_OPEN_NOW	'N'
#define CS_REMOTE_OPEN_READ	'R'
#define CS_REMOTE_OPEN_SHARE	'S'
#define CS_REMOTE_OPEN_TEST	'T'
#define CS_REMOTE_OPEN_TRUST	'X'
#define CS_REMOTE_PROFILE	"set true; $1 eval 'unset true eval; source ./.login >& /dev/null < /dev/null'; set eval CsH=true; $1 $CsH 'shift; shift; . ./.profile >/dev/null 2>&1 </dev/null'"

#define CS_FD_ALLOCATE	(-2)		/* csfd() table alloc		*/

#define CS_OPEN_READ	0		/* initiate and open for read	*/
#define CS_OPEN_CREATE	1		/* create			*/

#define CS_OPEN_AGENT	(1<<1)		/* initiate, no authenticate	*/
#define CS_OPEN_LOCAL	(1<<2)		/* CS_OPEN_READ local affinity	*/
#define CS_OPEN_MOUNT	(1<<3)		/* no open, mount namespace only*/
#define CS_OPEN_NOW	(1<<4)		/* non blocking open		*/
#define CS_OPEN_REMOTE	(1<<5)		/* force CS_ADDR_REMOTE		*/
#define CS_OPEN_SHARE	(1<<6)		/* share affinity		*/
#define CS_OPEN_SLAVE	(1<<7)		/* no daemon fork() on create	*/
#define CS_OPEN_TEST	(1<<8)		/* open for read, no initiate	*/
#define CS_OPEN_TRUST	(1<<9)		/* trusted service		*/

#define CS_PATH_NAME	(1<<0)		/* cspath addr name vs. n.n.n.n	*/

#define CS_VAR_LOCAL	0		/* csvar for local mount dir	*/
#define CS_VAR_PROXY	1		/* csvar for proxy service	*/
#define CS_VAR_SHARE	2		/* csvar for share mount dir	*/
#define CS_VAR_TRUST	3		/* csvar for trusted root dirs	*/

/*
 * cs.flags -- CS_<function>_<flags>
 */

#define CS_ADDR_FULL	(1<<0)		/* retain fully qualified names	*/
#define CS_ADDR_LOCAL	(1<<1)		/* local host			*/
#define CS_ADDR_NOW	(1<<2)		/* for CS_OPEN_NOW		*/
#define CS_ADDR_NUMERIC	(1<<3)		/* n.n.n.n addr			*/
#define CS_ADDR_REMOTE	(1<<4)		/* remote host			*/
#define CS_ADDR_SHARE	(1<<5)		/* share host			*/
#define CS_ADDR_TEST	(1<<6)		/* share host			*/
#define CS_ADDR_TRUST	(1<<7)		/* for CS_OPEN_TRUST		*/
#define CS_DAEMON_SLAVE	(1<<8)		/* no daemon fork()		*/
#define CS_PIPE_BLOCKED	(1<<9)		/* SIGPIPE blocked		*/
#define CS_TEST		(1<<10)		/* enable test			*/

#define CS_CLIENT_ARGV	(1<<0)		/* just process argv		*/
#define CS_CLIENT_RAW	(1<<1)		/* tty raw mode input		*/
#define CS_CLIENT_SEP	(1<<2)		/* argv ':' line sep		*/

typedef struct
{
	unsigned long	addr[3];	/* address cookie		*/
} Csaddr_t;

typedef struct
{
	unsigned long	hid;		/* host id			*/
	unsigned long	pid;		/* process id			*/
	unsigned long	uid;		/* user id			*/
	unsigned long	gid;		/* group id			*/
} Csid_t;

typedef struct
{
	unsigned long	idle;		/* min user idle secs		*/
	long		up;		/* up (down<0) time in secs	*/
	int		load;		/* load average			*/
	int		pctsys;		/* % time for sys		*/
	int		pctusr;		/* % time for usr		*/
	int		users;		/* # interesting users		*/
} Csstat_t;

struct Cs_s;
typedef struct Cs_s Cs_t;

struct Csdisc_s;
typedef struct Csdisc_s Csdisc_t;

struct Csdisc_s				/* user discipline		*/
{
	unsigned long	version;	/* CS_VERSION			*/
	unsigned long	flags;		/* CS_* flags			*/
	Error_f		errorf;		/* error message handler	*/
};

struct Cs_s				/* thread state			*/
{
	char*		id;		/* library id			*/
	Csdisc_t*	disc;		/* discipline from csalloc()	*/
	unsigned long	addr;		/* addr from csbind()		*/
	unsigned long	port;		/* port from csbind()		*/
	unsigned long	time;		/* time updated by CSTIME()	*/
	unsigned long	flags;		/* misc flags			*/
	char*		control;	/* CS_MNT_* in cs.mount		*/
	char*		cs;		/* service connect stream	*/
	char		type[8];	/* csopen() stream type		*/
	char		qual[CS_NAME_MAX];/* csopen() qualifier		*/
	char		host[CS_NAME_MAX];/* csaddr() real host		*/
	char		mount[PATH_MAX];/* current mount path		*/
	char		user[CS_NAME_MAX];/* csaddr() user		*/

#ifdef _CS_PRIVATE_
	_CS_PRIVATE_
#endif

};

#if _BLD_cs && defined(__EXPORT__)
#define extern		__EXPORT__
#endif
#if !_BLD_cs && defined(__IMPORT__)
#define extern		extern __IMPORT__
#endif

extern Cs_t		cs;

#undef	extern

#if _BLD_cs && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

#if CS_INTERFACE <= 1

#define Cs_addr_t	Csaddr_t
#define Cs_id_t		Csid_t
#define Cs_info_t	Cs_t
#define Cs_poll_t	Cspoll_t
#define Cs_stat_t	Csstat_t

#define CSADDR		Csaddr_t
#define CSID		Csid_t
#define CSINFO		Csinfo_t
#define CSPOLL		Cspoll_t
#define CSSTAT		Csstat_t

#define csaddr		_cs_addr
#define csattach	_cs_attach
#define csattr		_cs_attr
#define csauth		_cs_auth
#define csbind		_cs_bind
#define csclient	_cs_client
#define csclone		_cs_clone
#define cschallenge	_cs_challenge
#define csdaemon	_cs_daemon
#define csfd		_cs_fd
#define csfrom		_cs_from
#define csfull		_cs_full
#define csinfo		_cs_info
#define cslocal		_cs_local
#define csname		_cs_name
#define csnote		_cs_note
#define csntoa		_cs_ntoa
#define csopen		_cs_open
#define cspath		_cs_path
#define cspeek		_cs_peek
#define csping		_cs_ping
#define cspipe		_cs_pipe
#define cspoll		_cs_poll
#define csport		_cs_port
#define csread		_cs_read
#define csrecv		_cs_recv
#define cssend		_cs_send
#define csserve		_cs_serve
#define csstat		_cs_stat
#define cstimeout	_cs_timeout
#define csto		_cs_to
#define csvar		_cs_var
#define cswakeup	_cs_wakeup
#define cswrite		_cs_write

#else

extern unsigned long	csaddr(Cs_t*, const char*);
extern Cs_t*		csalloc(Csdisc_t*);
extern int		csattach(Cs_t*, const char*, int, int);
extern char*		csattr(Cs_t*, const char*, const char*);
extern int		csauth(Cs_t*, int, const char*, const char*);
extern int		csbind(Cs_t*, const char*, unsigned long, unsigned long, unsigned long);
extern int		cschallenge(Cs_t*, const char*, unsigned long*, unsigned long*);
extern int		csclient(Cs_t*, int, const char*, const char*, char**, unsigned int);
extern unsigned long	csclone(Cs_t*, int);
extern int		csdaemon(Cs_t*, int);
extern int		csfd(Cs_t*, int, int);
extern int		csfree(Cs_t*);
extern ssize_t		csfrom(Cs_t*, int, void*, size_t, Csaddr_t*);
extern char*		csfull(Cs_t*, unsigned long);
extern Sfio_t*		csinfo(Cs_t*, const char*, int*);
extern int		cslocal(Cs_t*, const char*);
extern char*		csname(Cs_t*, unsigned long);
extern int		csnote(Cs_t*, const char*, Csstat_t*);
extern char*		csntoa(Cs_t*, unsigned long);
extern int		csopen(Cs_t*, const char*, int);
extern char*		cspath(Cs_t*, int, int);
extern ssize_t		cspeek(Cs_t*, int, void*, size_t);
extern int		csping(Cs_t*, const char*);
extern int		cspipe(Cs_t*, int*);
extern int		cspoll(Cs_t*, Cspoll_t*, int, int);
extern unsigned long	csport(Cs_t*, const char*, const char*);
extern ssize_t		csread(Cs_t*, int, void*, size_t, int);
extern int		csrecv(Cs_t*, int, Csid_t*, int*, int);
extern int		cssend(Cs_t*, int, int*, int);
extern void		csserve(Cs_t*, void*, const char*, void*(*)(void*, int), int(*)(void*, int), int(*)(void*, int, Csid_t*, int, char**), int(*)(void*, int), int(*)(void*, int), int(*)(void*));
extern int		csstat(Cs_t*, const char*, Csstat_t*);
extern unsigned long	cstimeout(Cs_t*, unsigned long);
extern ssize_t		csto(Cs_t*, int, const void*, size_t, Csaddr_t*);
extern char*		csvar(Cs_t*, int, int);
extern unsigned long	cswakeup(Cs_t*, unsigned long);
extern ssize_t		cswrite(Cs_t*, int, const void*, size_t);

#endif

#if CS_INTERFACE <= 1 || defined(_CS_PRIVATE_)

extern unsigned long	_cs_addr(const char*);
extern int		_cs_attach(const char*, int, int);
extern char*		_cs_attr(const char*, const char*);
extern int		_cs_auth(int, const char*, const char*);
extern int		_cs_bind(const char*, unsigned long, unsigned long, unsigned long);
extern int		_cs_client(int, const char*, const char*, char**, unsigned int);
extern unsigned long	_cs_clone(int);
extern int		_cs_daemon(int);
extern int		_cs_fd(int, int);
extern ssize_t		_cs_from(int, void*, size_t, Csaddr_t*);
extern char*		_cs_full(unsigned long);
extern Sfio_t*		_cs_info(const char*, int*);
extern int		_cs_local(const char*);
extern char*		_cs_name(unsigned long);
extern int		_cs_note(const char*, Csstat_t*);
extern char*		_cs_ntoa(unsigned long);
extern int		_cs_open(const char*, int);
extern char*		_cs_path(int, int);
extern ssize_t		_cs_peek(int, void*, size_t);
extern int		_cs_ping(const char*);
extern int		_cs_pipe(int*);
extern int		_cs_poll(Cspoll_t*, int, int);
extern unsigned long	_cs_port(const char*, const char*);
extern ssize_t		_cs_read(int, void*, size_t, int);
extern int		_cs_recv(int, Csid_t*, int*, int);
extern int		_cs_send(int, int*, int);
extern void		_cs_serve(void*, const char*, void*(*)(void*, int), int(*)(void*, int), int(*)(void*, int, Csid_t*, int, char**), int(*)(void*, int), int(*)(void*, int), int(*)(void*));
extern int		_cs_stat(const char*, Csstat_t*);
extern unsigned long	_cs_timeout(unsigned long);
extern ssize_t		_cs_to(int, const void*, size_t, Csaddr_t*);
extern char*		_cs_var(int, int);
extern unsigned long	_cs_wakeup(unsigned long);
extern ssize_t		_cs_write(int, const void*, size_t);

#endif

#undef	extern

#endif
