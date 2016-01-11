/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2012 AT&T Intellectual Property          *
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
 * connect stream server support
 */

#ifndef _CSSLIB_H
#define _CSSLIB_H

#include "cslib.h"

#include <error.h>
#include <sig.h>
#include <tok.h>

#define EXPIRE		60
#define KEYEXPIRE	(60*5)
#define KEYMASK		0x7fffffff
#define TOSS(k)		while(CSTOSS(k,cs.time)<=CS_KEY_MAX)

typedef struct
{
	int		seq;
	unsigned long	expire;
	time_t		atime;
	time_t		mtime;
	Csid_t		id;
} Auth_t;

typedef struct
{
	dev_t		dev;
	ino_t		ino;
} Fid_t;

struct Common_s;
typedef struct Common_s Common_t;

#define _CSS_FD_PRIVATE_		\
	Css_t*		css;		\
	int		events;		\
	int		set;

#define _CSS_PRIVATE_			\
	Cssdisc_t*	disc;		\
	Css_t*		next;		\
	char		buf[PATH_MAX];	\
	char		tmp[PATH_MAX];	\
	int		auth;		\
	int		challenge;	\
	int		fdpending;	\
	int		fdpolling;	\
	int		fdlistening;	\
	int		fduser;		\
	Fid_t		fid[2];		\
	gid_t		gid;		\
	uid_t		uid;		\
	unsigned long	newkey;		\
	unsigned long	oldkey;		\
	unsigned long	conkey;		\
	unsigned long	timeout_last;	\
	unsigned long	timeout_remain;	\
	unsigned long	wakeup_last;	\
	unsigned long	wakeup_remain;

#include <css.h>

struct Common_s
{
	Css_t*		servers;
	Css_t*		main;
	Auth_t*		auth;
	int		nauth;
	Cspoll_t*	fdpoll;
	Cssfd_t*	fdinfo;
	unsigned long	flags;
	int		fdpending;
	int		fdpolling;
	int		fdbefore;
	int		fdloop;
	int		fdnext;
	int		fdmax;
	int		pid;
	int		polling;
	unsigned long	expire;
};

#endif
