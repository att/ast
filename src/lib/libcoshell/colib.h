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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * coshell library private definitions
 */

#ifndef _COLIB_H
#define _COLIB_H

#include <ast.h>
#include <dt.h>
#include <vmalloc.h>

#define _CO_JOB_PRIVATE_		/* Cojob_t private additions	*/ \
	Cojob_t*	next;		/* next in list			*/ \
	Coservice_t*	service;	/* service 			*/ \
	int		pid;		/* pid				*/ \
	char*		out;		/* serialized stdout file	*/ \
	char*		err;		/* serialized stderr file	*/ \
					/* end of private additions	*/

#define _CO_SHELL_PRIVATE_		/* Coshell_t private additions	*/ \
	Vmalloc_t*	vm;		/* Coshell_t vm			*/ \
	Coshell_t*	next;		/* next in list			*/ \
	Cojob_t*	jobs;		/* job list			*/ \
	Coservice_t*	service;	/* service 			*/ \
	Dt_t*		export;		/* coexport() dictionary	*/ \
	Dtdisc_t*	exdisc;		/* coexport() discipline	*/ \
	struct Coinit_s			/* initialization script state	*/ \
	{								   \
	char*		script;		/* initialization script	*/ \
	dev_t		pwd_dev;	/* previous pwd dev		*/ \
	ino_t		pwd_ino;	/* previous pwd inode number	*/ \
	int		mask;		/* previous umask		*/ \
	int		sync;		/* sync script			*/ \
	}		init;						   \
	int		cmdfd;		/* command pipe fd		*/ \
	int		gsmfd;		/* msgfp child write side	*/ \
	int		mask;		/* CO_* flags to clear		*/ \
	int		mode;		/* connection modes		*/ \
	int		svc_outstanding;/* outstanding service intercepts */ \
	int		svc_running;	/* running service intercepts	*/ \
	int		pid;		/* pid				*/ \
	int		index;		/* coshell index		*/ \
	int		slots;		/* number of job slots		*/ \
					/* end of private additions	*/

typedef struct Coexport_s
{
	Dtlink_t	link;
	char*		value;
	char		name[1];
} Coexport_t;

struct Coservice_s;
typedef struct Coservice_s Coservice_t;

struct Coservice_s			/* service info			*/
{
	Coservice_t*	next;		/* next in list			*/
	char*		name;		/* instance name		*/
	char*		path;		/* coexec() command path	*/
	char*		db;		/* state/db path		*/
	int		fd;		/* command pipe			*/
	int		pid;		/* pid				*/
	char*		argv[16];	/* coexec() command argv[]	*/
};

#include <coshell.h>
#include <error.h>
#include <sig.h>
#include <wait.h>

#define state		_coshell_info_	/* hide external symbol		*/

#define CO_MODE_ACK		(1<<0)	/* wait for coexec() ack	*/
#define CO_MODE_INDIRECT	(1<<1)	/* indirect CO_SERVER		*/
#define CO_MODE_SEPARATE	(1<<2)	/* 1 shell+wait per action	*/

#define CO_INIT		(CO_USER>>1)	/* initial command		*/

#define CO_PID_FREE	(-3)		/* free job slot		*/
#define CO_PID_WARPED	(-2)		/* exit before start message	*/
#define CO_PID_ZOMBIE	(-1)		/* ready for wait		*/

#define CO_BUFSIZ	(PATH_MAX/2)	/* temporary buffer size	*/
#define CO_MAXEVAL	(PATH_MAX*8)	/* max eval'd action size	*/

typedef struct Costate_s		/* global coshell state		*/
{
	const char*	lib;		/* library id			*/
	Coshell_t*	coshells;	/* list of all coshells		*/
	Coshell_t*	current;	/* current coshell		*/
	Coshell_t*	generic;	/* generic coshell for coinit()	*/
	char*		pwd;		/* pwd				*/
	char*		sh;		/* sh from first coopen()	*/
	char*		type;		/* CO_ENV_TYPE value		*/
	int		init;		/* 0 if first coopen()		*/
	int		index;		/* last coshell index		*/
} Costate_t;

extern char 		coident[];	/* coshell ident script		*/
extern char 		cobinit[];	/* bsh initialition script	*/
extern char 		cokinit[];	/* ksh initialition script	*/
extern char* 		co_export[];	/* default export var list	*/

extern Costate_t	state;		/* global coshell info		*/

#ifndef errno
extern int		errno;
#endif

extern char*		costash(Sfio_t*);
extern char*		coinitialize(Coshell_t*, int);

#endif
