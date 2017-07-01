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
 * AT&T Bell Laboratories
 *
 * remote coshell service definitions
 */

#define LABELLEN	64		/* max label string length	*/
#define NAMELEN		64		/* max name string length	*/
#define TYPELEN		64		/* max type string length	*/
#define MISCLEN		256		/* max misc string length	*/

#define _CO_JOB_PRIVATE_		/* Cojob_t private additions	*/ \
	char		label[LABELLEN];/* optional label		*/ \
	int		pid;		/* pid				*/ \
	int		rid;		/* user request id		*/ \
	int		sig;		/* last signal sent to job	*/ \
	int		fd;		/* con USER fd			*/ \
	int		ref;		/* drop reference count		*/ \
	char*		cmd;		/* cmd msg text			*/ \
	Coshell_t*	shell;		/* controlling shell		*/ \
	unsigned long	busy;		/* time when job becomes hog	*/ \
	unsigned long	lost;		/* time when job is lost	*/ \
	unsigned long	start;		/* start time			*/ \
					/* end of private additions	*/

#define _CO_SHELL_PRIVATE_		/* Coshell_t private additions	*/ \
	Coshell_t*	next;		/* next in ring			*/ \
	char		name[NAMELEN];	/* host name			*/ \
	char		type[TYPELEN];	/* host type			*/ \
	char		misc[MISCLEN];	/* host misc attributes		*/ \
	char		remote[MISCLEN];/* remote shell path		*/ \
	char		shell[MISCLEN];	/* shell path			*/ \
	char*		access;		/* host access expression	*/ \
	char*		bypass;		/* idle bypass expression	*/ \
	unsigned long	addr;		/* host address			*/ \
	unsigned long	rank;		/* scheduling rank		*/ \
	unsigned long	temp;		/* scheduling temperature	*/ \
	int		mode;		/* extra flags			*/ \
	int		bias;		/* scheduling bias		*/ \
	int		cpu;		/* cpu count			*/ \
	int		errors;		/* csstat() error count		*/ \
	int		fd;		/* con SHELL fd			*/ \
	int		home;		/* # users calling this home	*/ \
	int		idle;		/* min # login idle secs	*/ \
	int		idle_override;	/* restore idle override	*/ \
	int		pid;		/* shell pid			*/ \
	int		open;		/* number of successful opens	*/ \
	int		rating;		/* cpu rating			*/ \
	int		scale;		/* load average scale		*/ \
	Cs_stat_t	stat;		/* csstat() status		*/ \
	unsigned long	override;	/* override expiration		*/ \
	unsigned long	start;		/* connect start time		*/ \
	unsigned long	update;		/* time stat last updated	*/ \
					/* end of private additions	*/

#include <ast.h>
#include <cs.h>
#include <coshell.h>
#include <ctype.h>
#include <debug.h>
#include <error.h>
#include <sig.h>
#include <tm.h>
#include <tok.h>
#include <wait.h>

#define match(p,a,o)	((!((a)->set&SETNAME)||strmatch((p)->name,(a)->name))&&((((a)->set|o)&(SETMISC|DEF|NEW|SET))==SETMISC?miscmatch(p,(a)->misc):((a)->set&SETNAME)?1:streq((p)->type,state.home->type)))

#define SHELL_CLOSE	(1<<0)			/* marked for close	*/
#define SHELL_DENIED	(1<<1)			/* access denied	*/
#define SHELL_DISABLE	(1<<2)			/* disabled for UPDATE	*/
#define SHELL_OVERRIDE	(1<<3)			/* override on open	*/

#define ACCESS_SEARCH	(60*60)			/* CS_SVC_ACCESS search	*/
#define ACCESS_UPDATE	(10*60)			/* CS_SVC_ACCESS update	*/
#define BIAS		100			/* default bias		*/
#define BUSY		((UPDATE)*2)		/* default max busy	*/
#define CHUNK		1024			/* allocation chunk	*/
#define ERRORS		8			/* max csstat() errors	*/
#define FORGET		(5*LOST)		/* forget lost shell	*/
#define GRACE		BUSY			/* default busy grace	*/
#define HOG		(~0L)			/* hog job busy time	*/
#define HOME		(3*OVERRIDE)		/* home host reprieve	*/
#define LOAD		100			/* default load		*/
#define LOST		(1*60)			/* job/shell lost secs	*/
#define OVERRIDE	(2*60)			/* override reprieve	*/
#define POOL		8			/* default proc pool	*/
#define RANK		30000			/* worst rank w/o toss	*/
#define RATING		100			/* default rating	*/
#define TEMPBASE	32			/* temp file name base	*/
#define TOSS		CSTOSS(state.toss,0)	/* tiebreaker		*/
#define UPDATE		((6*CS_STAT_FREQ)/5)	/* csstat() update freq	*/

#define COMMAND		1
#define DEFER		2
#define SERVER		3

#define ANON		1
#define DEST		2
#define IDENT		3
#define INIT		4
#define MESG		5
#define PASS		6
#define POLL		7
#define PUMP		8
#define SCHED		9
#define SHELL		10
#define UCMD		11
#define UERR		12
#define UOUT		13
#define USER		14

#define USER_DUP	(1<<0)		/* Couser_t stdout==stderr	*/
#define USER_IDENT	(1<<1)		/* Couser_t INIT needs ident	*/
#define USER_INIT	(1<<2)		/* Couser_t INIT hit once	*/

#define DEF		(1<<0)		/* default attributes		*/
#define GET		(1<<1)		/* get existing shell		*/
#define IGN		(1<<2)		/* ignore pool member		*/
#define JOB		(1<<3)		/* schedule job			*/
#define NEW		(1<<4)		/* make new shell if not found	*/
#define SET		(1<<5)		/* set attributes		*/

#define SETACCESS	(1<<6)		/* access was set		*/
#define SETBIAS		(1<<7)		/* bias was set			*/
#define SETBYPASS	(1<<8)		/* bypass was set		*/
#define SETCPU		(1<<9)		/* cpu count was set		*/
#define SETIDLE		(1<<10)		/* idle was set			*/
#define SETIGNORE	(1<<11)		/* ignore was set		*/
#define SETLABEL	(1<<12)		/* label was set		*/
#define SETMISC		(1<<13)		/* misc attribute was set	*/
#define SETNAME		(1<<14)		/* name was set			*/
#define SETRATING	(1L<<15)	/* rating was set		*/
#define SETSCALE	(1L<<16)	/* scale was set		*/
#define SETTYPE		(1L<<17)	/* type was set			*/

#define SETBUSY		(1<<0)		/* global.busy was set		*/
#define SETDEBUG	(1<<1)		/* global.debug was set		*/
#define SETDISABLE	(1<<2)		/* global.disable was set	*/
#define SETFILE		(1<<3)		/* global.file was set		*/
#define SETGRACE	(1<<4)		/* global.grace was set		*/
#define SETIDENTIFY	(1<<5)		/* global.identify was set	*/
#define SETLOAD		(1<<6)		/* stat.load was set		*/
#define SETMAXIDLE	(1<<7)		/* global.maxidle was set	*/
#define SETMAXLOAD	(1<<8)		/* global.maxload was set	*/
#define SETMIGRATE	(1<<9)		/* global.migrate was set	*/
#define SETPERCPU	(1<<10)		/* global.percpu was set	*/
#define SETPERHOST	(1<<11)		/* global.perhost was set	*/
#define SETPERSERVER	(1<<12)		/* global.perserver was set	*/
#define SETPERUSER	(1<<13)		/* global.peruser was set	*/
#define SETPOOL		(1<<14)		/* global.pool was set		*/
#define SETPROFILE	(1L<<15)	/* global.profile was set	*/
#define SETSCHEDULE	(1L<<16)	/* global.schedule was set	*/
#define SETUPDATE	(1L<<17)	/* stat.update was set		*/
#define SETUSERS	(1L<<18)	/* stat.users was set		*/

#define SETREMOTE	(1L<<29)	/* (global) remote was set	*/
#define SETSHELL	(1L<<30)	/* (global) shell was set	*/

#define QUEUE		(-1)		/* waiting for shell to open	*/
#define START		(-2)		/* started but no pid		*/
#define WARP		(-3)		/* exit before start message	*/

typedef struct
{
	char		name[NAMELEN];
	char		type[TYPELEN];
	char		misc[MISCLEN];
	char		label[LABELLEN];
	char		remote[MISCLEN];
	char		shell[MISCLEN];
	char*		access;
	char*		bypass;
	int		bias;
	int		cpu;
	int		idle;
	int		ignore;
	int		rating;
	int		scale;
	unsigned long	set;

	struct
	{
	char*		file;
	char*		identify;
	char*		migrate;
	char*		profile;
	char*		remote;
	char*		schedule;
	char*		shell;
	int		busy;
	int		debug;
	unsigned long	disable;
	int		grace;
	int		maxidle;
	int		maxload;
	int		percpu;
	int		perhost;
	int		perserver;
	int		peruser;
	int		pool;
	unsigned long	set;
	}		global;

	Cs_stat_t	stat;
} Coattr_t;

typedef struct
{
	Coshell_t*	shell;
	int		pid;
} Coident_t;

typedef struct
{
	Cojob_t*	job;
	Sfio_t*		serialize;
	short		fd;
} Copass_t;

typedef struct
{
	Coshell_t*	home;
	char*		pump;
	char*		expr;
	Coattr_t	attr;
	short		fds[3];
	short		flags;
	short		pid;
	short		running;
	short		total;
} Couser_t;

typedef struct
{
	union
	{
	Coident_t	ident;
	Copass_t	pass;
	Coshell_t*	shell;
	Couser_t	user;
	}		info;
	short		type;
	short		error;
	short		flags;
} Connection_t;

typedef struct
{
	int		con;
	int		cmd;
	int		err;
	int		msg;
	int		out;
	char*		pump;
} Indirect_t;

typedef struct
{
	unsigned long	access;
	char**		argv;
	char*		buf;
	int		buflen;
	unsigned long	busy;
	Pathcheck_t	check;
	unsigned long	clock;
	int		cmds;
	Connection_t*	con;
	int		connect;
	unsigned long	disable;
	int		fdtotal;
	gid_t*		gids;
	unsigned long	grace;
	Coshell_t*	home;
	char*		identify;
	Indirect_t	indirect;
	Cojob_t*	job;
	Cojob_t*	jobmax;
	Cojob_t*	jobnext;
	int		jobs;
	int		joblimit;
	int		jobwait;
	int		maxidle;
	int		maxload;
	char*		mesg;
	char*		migrate;
	int		open;
	int		override;
	int		percpu;
	int		perhost;
	int		perserver;
	int		peruser;
	int		pool;
	char*		profile;
	char*		pump;
	unsigned long	real;
	char*		remote;
	int		running;
	struct
	{
	char*		name;
	int		fd;
	}		scheduler;
	char*		service;
	int		set;
	char*		sh;
	Coshell_t*	shell;
	Coshell_t*	shellnext;
	int		shellc;
	int		shelln;
	int		shells;
	Coshell_t**	shellv;
	int		shellwait;
	unsigned long	start;
	Sfio_t*		string;
	unsigned long	sys;
	unsigned long	toss;
	unsigned long	user;
	unsigned long	wakeup;
	Tm_t*		tm;
	uid_t		uid;
	int		users;
	char*		version;
	Coshell_t	wait;
} State_t;

/*
 * coshell specific globals
 */

extern const char	corinit[];
extern State_t		state;

extern void		attributes(char*, Coattr_t*, Coattr_t*);
extern int		byname(const char*, const char*);
extern int		byrank(const char*, const char*);
extern int		bytemp(const char*, const char*);
extern int		command(int, char**);
extern void		drop(int);
extern char*		fmtfloat(int);
extern Coshell_t*	info(int, char*);
extern void		jobcheck(Coshell_t*);
extern void		jobdone(Cojob_t*);
extern void		jobkill(Cojob_t*, int);
extern void		miscadd(Coshell_t*, char*);
extern int		miscmatch(Coshell_t*, char*);
extern Coshell_t*	search(int, char*, Coattr_t*, Coattr_t*);
extern void		server(int, int, int, int, char*);
extern void		shellcheck(void);
extern void		shellclose(Coshell_t*, int);
extern void		shellexec(Cojob_t*, char*, int);
extern int		shellopen(Coshell_t*, int);
extern char*		stream(int, char*);
extern void		update(Coshell_t*);
