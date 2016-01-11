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
 * connect stream server interface definitions
 */

#ifndef _CSS_H
#define _CSS_H

#define CSS_VERSION	19970717L

#ifndef CS_INTERFACE
#define CS_INTERFACE	2
#endif

#define CSS_AUTHENTICATE (1<<10)	/* authenticate connections	*/
#define CSS_DAEMON	(1<<0)		/* become daemon proc		*/
#define CSS_LOG		(1<<1)		/* stderr to daemon log file	*/
#define CSS_PRESERVE	(1<<2)		/* preserve daemon fd's		*/
#define CSS_RECURSIVE	(1<<3)		/* allow recursive csspoll()	*/

#define CSS_CLOSE	(1<<4)		/* server close exception	*/
#define CSS_DORMANT	(1<<5)		/* dormant timeout exception	*/
#define CSS_ERROR	(1<<6)		/* error exception		*/
#define CSS_INTERRUPT	(1<<7)		/* interrupt exception		*/
#define CSS_TIMEOUT	(1<<8)		/* timeout exception		*/
#define CSS_WAKEUP	(1<<9)		/* wakeup exception		*/

#define CS_POLL_DUP	1
#define CS_POLL_MOVE	2
#define CS_POLL_PRI	3

#define CS_POLL_ARG	0x80000000
#define CS_POLL_SHIFT	16
#define CS_POLL_MASK	0x00007fff

#define CS_POLL_CMD(f,c)(CS_POLL_ARG|(c)|(((long)(f)&CS_POLL_MASK)<<CS_POLL_SHIFT))

#include <cs.h>

struct Css_s;
struct Cssdisc_s;
struct Cssfd_s;

typedef struct Css_s Css_t;
typedef struct Cssdisc_s Cssdisc_t;
typedef struct Cssfd_s Cssfd_t;

struct Cssdisc_s			/* user discipline		*/
{
	unsigned long	version;	/* CSS_VERSION			*/
	unsigned long	flags;		/* CSS_* flags			*/
	unsigned long	timeout;	/* timeout in ms, 0 if none	*/
	unsigned long	wakeup;		/* wakeup in ms, 0 if none	*/
	Error_f		errorf;		/* error message handler	*/
	int		(*acceptf)(Css_t*, Cssfd_t*, Csid_t*, char**, Cssdisc_t*);
					/* accept new connection/fd	*/
	int		(*actionf)(Css_t*, Cssfd_t*, Cssdisc_t*);
					/* fd state change action	*/
	int		(*exceptf)(Css_t*, unsigned long, unsigned long, Cssdisc_t*);
					/* poll exception action	*/
};

struct Cssfd_s				/* fd info			*/
{
	int		fd;		/* fd				*/
	int		status;		/* action status		*/
	int		(*actionf)(Css_t*, Cssfd_t*, Cssdisc_t*);
					/* css.actionf by default	*/
	void*		data;		/* user data, 0 by default	*/

#ifdef _CSS_FD_PRIVATE_
	_CSS_FD_PRIVATE_
#endif

};

struct Css_s				/* connect stream server state	*/
{
	char*		id;		/* library identifier		*/
	char*		service;	/* service name			*/
	char*		path;		/* service path			*/
	int		fd;		/* service fd			*/
	int		fdmax;		/* max # serviceable fd's	*/
	int		perm;		/* service permissions		*/
	char		mount[PATH_MAX];/* service mount path		*/
	char*		control;	/* CS_MNT_* in css.mount	*/
	Cs_t*		state;		/* from csalloc()		*/

#ifdef _CSS_PRIVATE_
	_CSS_PRIVATE_
#endif

};

#if _BLD_cs && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Css_t*		cssopen(const char*, Cssdisc_t*);
extern Cssfd_t*		cssfd(Css_t*, int, unsigned long);
extern Cssfd_t*		csspoll(unsigned long, unsigned long);
extern int		cssclose(Css_t*);

#undef	extern

#endif
