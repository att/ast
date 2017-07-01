/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
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

/*
 * archive scan/touch/extract interface
 */

#ifndef _ARDIR_H
#define _ARDIR_H	1

#include <ast.h>
#include <ls.h>

#define ARDIR_VERSION		20020501L

/*
 * retain this order!
 */

#define ARDIR_CREATE		0x0001
#define ARDIR_DELETE		0x0002
#define ARDIR_REPLACE		(ARDIR_CREATE|ARDIR_DELETE)

#define ARDIR_FORCE		0x0004
#define ARDIR_LOCAL		0x0008
#define ARDIR_NEWER		0x0010
#define ARDIR_OUTOFDATE		0x0020
#define ARDIR_RANLIB		0x0040
#define ARDIR_SEQUENTIAL	0x0080
#define ARDIR_UPDATE		0x0100

struct Ardir_s; typedef struct Ardir_s Ardir_t;
struct Ardirent_s; typedef struct Ardirent_s Ardirent_t;
struct Ardirmeth_s; typedef struct Ardirmeth_s Ardirmeth_t;

struct Ardirmeth_s
{
	const char*	name;
	const char*	description;
	int		(*openf)(Ardir_t*, char*, size_t);
	Ardirent_t*	(*nextf)(Ardir_t*);
	int		(*changef)(Ardir_t*, Ardirent_t*);
	int		(*insertf)(Ardir_t*, const char*, int);
	const char*	(*specialf)(Ardir_t*);
	int		(*closef)(Ardir_t*);
	Ardirmeth_t*	next;
};

struct Ardirent_s
{
	char*			name;
	time_t			mtime;
	off_t			offset;
	off_t			size;
	uid_t			uid;
	gid_t			gid;
	mode_t			mode;
#ifdef _ARDIRENT_PRIVATE_
	_ARDIRENT_PRIVATE_
#endif
};

struct Ardir_s
{
	unsigned long		version;
	char*			path;
	struct stat		st;
	unsigned long		symtime;
	Ardirmeth_t*		meth;
	void*			data;
	unsigned int		flags;
	int			error;
	int			fd;
	int			truncate;
	Ardirent_t		dirent;
#ifdef _ARDIR_PRIVATE_
	_ARDIR_PRIVATE_
#endif
};

extern Ardirmeth_t*	ardirmeth(const char*);
extern Ardirmeth_t*	ardirlist(Ardirmeth_t*);
extern Ardir_t*		ardiropen(const char*, Ardirmeth_t*, int);
extern Ardirent_t*	ardirnext(Ardir_t*);
extern off_t		ardircopy(Ardir_t*, Ardirent_t*, int);
extern int		ardirchange(Ardir_t*, Ardirent_t*);
extern int		ardirinsert(Ardir_t*, const char*, int);
extern const char*	ardirspecial(Ardir_t*);
extern int		ardirclose(Ardir_t*);

#endif
