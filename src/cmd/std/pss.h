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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * process status stream interface definitions
 */

#ifndef _PSS_H
#define _PSS_H		1

#include <ast.h>

#define PSS_VERSION		20101201L

#define PSS_ALL			(1<<0)		/* match all		*/
#define PSS_ATTACHED		(1<<1)		/* match attached	*/
#define PSS_DETACHED		(1<<2)		/* match detached	*/
#define PSS_LEADER		(1<<3)		/* match group leaders	*/
#define PSS_NOLEADER		(1<<4)		/* no group leaders	*/
#define PSS_PS			(1<<5)		/* force the ps method	*/
#define PSS_TTY			(1<<6)		/* match disc.tty	*/
#define PSS_UID			(1<<7)		/* match disc.uid	*/
#define PSS_UNMATCHED		(1<<8)		/* return unmatched too	*/
#define PSS_VERBOSE		(1<<9)		/* report all errors	*/

#define PSS_ANCESTOR		(1<<0)		/* ancestor matched	*/
#define PSS_CHILD		(1<<1)		/* on children chain	*/
#define PSS_EXPLICIT		(1<<2)		/* explicit		*/
#define PSS_MATCHED		(1<<3)		/* matched		*/
#define PSS_PARENT		(1<<4)		/* on parent chain	*/

#define PSS_FLAGS		0xff
#define PSS_NODEV		((Pss_dev_t)(-1))
#define PSS_SCAN		0
#define PSS_ZOMBIE		'Z'

#define PSS_addr		(1<<0)
#define PSS_args		(1<<1)
#define PSS_command		(1<<2)
#define PSS_cpu			(1<<3)
#define PSS_flags		(1<<4)
#define PSS_gid			(1<<5)
#define PSS_job			(1<<6)
#define PSS_nice		(1<<7)
#define PSS_npid		(1<<8)
#define PSS_pgrp		(1<<9)
#define PSS_pid			(1<<10)
#define PSS_ppid		(1<<11)
#define PSS_pri			(1<<12)
#define PSS_proc		(1<<13)
#define PSS_refcount		(1<<14)
#define PSS_rss			(1L<<15)
#define PSS_sched		(1L<<16)
#define PSS_sid			(1L<<17)
#define PSS_size		(1L<<18)
#define PSS_start		(1L<<19)
#define PSS_state		(1L<<20)
#define PSS_tgrp		(1L<<21)
#define PSS_time		(1L<<22)
#define PSS_tty			(1L<<23)
#define PSS_uid			(1L<<24)
#define PSS_wchan		(1L<<25)

#define PSS_all		(PSS_addr|PSS_args|PSS_command|PSS_cpu|PSS_flags|\
			PSS_gid|PSS_job|PSS_nice|PSS_npid|PSS_pgrp|PSS_pid|\
			PSS_ppid|PSS_pri|PSS_proc|PSS_refcount|PSS_rss|\
			PSS_sched|PSS_sid|PSS_size|PSS_start|PSS_state|\
			PSS_tgrp|PSS_time|PSS_tty|PSS_uid|PSS_wchan)
#define PSS_match	(PSS_gid|PSS_pgrp|PSS_sid|PSS_tty|PSS_uid)

#undef	hz			/* who gets the prize for this?		*/

typedef long Pss_dev_t;
typedef long Pss_id_t;

struct Pss_s; typedef struct Pss_s Pss_t;

typedef struct Pssent_s
{
	int		pss;

	void*		addr;
	void*		wchan;

	char*		args;
	char*		command;
	char*		sched;
	char*		ttyname;

	Pss_dev_t	tty;

	size_t		rss;
	size_t		size;

	time_t		start;

	unsigned long	time;

	Pss_id_t	job;
	Pss_id_t	npid;
	Pss_id_t	pid;
	Pss_id_t	pgrp;
	Pss_id_t	ppid;
	Pss_id_t	sid;
	Pss_id_t	tgrp;

	Pss_id_t	gid;
	Pss_id_t	uid;

	long		nice;
	long		pri;

	int		cpu;
	int		flags;
	int		proc;
	int		refcount;
	int		state;
} Pssent_t;

typedef struct Pssdata_s
{
	struct Pssdata_s*	next;
	unsigned long		data;
} Pssdata_t;

typedef struct Pssmatch_s
{
	struct Pssmatch_s*	next;
	unsigned long		field;
	struct Pssdata_s*	data;
} Pssmatch_t;

typedef struct Pssdisc_s
{
	unsigned long	version;	/* interface version		*/
	unsigned long	fields;		/* PSS_[a-z]* field requests	*/
	unsigned long	flags;		/* PSS_[A-Z]* flags		*/
	char*		command;	/* caller command path		*/
	Pss_dev_t	tty;		/* PSS_TTY match value		*/
	Pss_id_t	uid;		/* PSS_UID match value		*/
	Pssmatch_t*	match;		/* match these fields		*/
	Error_f		errorf;		/* error function		*/
} Pssdisc_t;

typedef struct Pssmeth_s
{
	const char*	name;		/* method name			*/
	const char*	usage;		/* method usage			*/
	unsigned long	fields;		/* supported fields		*/
	int		(*initf)(Pss_t*);
	int		(*readf)(Pss_t*, Pss_id_t);
	int		(*partf)(Pss_t*, Pssent_t*);
	int		(*fullf)(Pss_t*, Pssent_t*);
	Pss_dev_t	(*ttydevf)(Pss_t*, const char*);
	char*		(*ttynamef)(Pss_t*, Pssent_t*);
	Pss_dev_t	(*ttymapf)(Pss_t*, Pss_dev_t);
	int		(*donef)(Pss_t*);
} Pssmeth_t;

struct Pss_s
{
	const char*	id;		/* library id string		*/
	Pssmeth_t*	meth;		/* method			*/

#ifdef _PSS_PRIVATE_
	_PSS_PRIVATE_
#endif

};

#define pssinit(d,c,e)	(memset(d,0,sizeof(*(d))),(d)->version=PSS_VERSION,(d)->command=(char*)(c),(d)->errorf=(Error_f)(e))

extern Pss_t*		pssopen(Pssdisc_t*);
extern Pssent_t*	pssread(Pss_t*, Pss_id_t);
extern Pssent_t*	psssave(Pss_t*, Pssent_t*);
extern int		pssclose(Pss_t*);

extern int		pssttyadd(Pss_t*, const char*, Pss_dev_t);
extern Pss_dev_t	pssttydev(Pss_t*, const char*);
extern char*		pssttyname(Pss_t*, Pssent_t*);

#endif
