/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
 * jcl interface definitions
 */

#ifndef _JCLLIB_H
#define _JCLLIB_H	1

#include <ast.h>
#include <dt.h>

#define IE_KEEP		01
#define IE_SKIP		02

#define JCL_LIST	(JCL_LISTAUTOEDITS|JCL_LISTEXEC|JCL_LISTINPUTS|JCL_LISTJOBS|JCL_LISTOUTPUTS|JCL_LISTPROGRAMS|JCL_LISTSCRIPTS|JCL_LISTVARIABLES)

struct Dir_s; typedef struct Dir_s Dir_t;
struct Dirlist_s; typedef struct Dirlist_s Dirlist_t;
struct Ie_s; typedef struct Ie_s Ie_t;
struct Include_s; typedef struct Include_s Include_t;
struct Redirect_s; typedef struct Redirect_s Redirect_t;
struct Rc_s; typedef struct Rc_s Rc_t;

struct Dir_s				/* directory list element	*/
{
	Dir_t*		next;		/* next in list			*/
	unsigned long	flags;		/* {JCL_JOB,JCL_PGM,JCL_PROC}	*/
	char		dir[1];		/* directory path		*/
};

struct Dirlist_s			/* directory list head/tail	*/
{
	Dir_t*		head;		/* directory list head		*/
	Dir_t*		tail;		/* directory list tail		*/
};

struct Include_s
{
	Include_t*	prev;
	Sfio_t*		sp;
	long		line;
	char*		file;
	char		path[1];
};

struct Rc_s
{
	Dtlink_t	link;
	int		rc;
	char		name[1];
};

struct Redirect_s
{
	const char*	file;
	int		fd;
};

struct Ie_s
{
	Ie_t*		prev;
	Ie_t*		next;
	int		flags;
	int		line;
};

#define _JCL_PUSH_ \
	char*		data; \
	char*		peek; \
	char*		peekpeek; \
	char*		last;

typedef struct Push_s
{
	_JCL_PUSH_
} Push_t;

#define CARD		72

#define DEFAULT		0x01
#define MUST		0x02

#define _JCL_PRIVATE_ \
	Jcl_t*		scope; \
	Jcl_t*		main; \
	Jclstep_t	current; \
	Jcldd_t*	lastdd; \
	Sfio_t*		sp; \
	Sfio_t*		cp; \
	Sfio_t*		dp; \
	Sfio_t*		rp; \
	Sfio_t*		tp; \
	Sfio_t*		vp; \
	Sfio_t*		xp; \
	Vmalloc_t*	vs; \
	Vmalloc_t*	vx; \
	Dt_t*		ds; \
	Dt_t*		rcs; \
	Dt_t*		ss; \
	Dt_t*		outdir; \
	Dtdisc_t	rcdisc; \
	Dtdisc_t	dddisc; \
	Dtdisc_t	outputdisc; \
	Dtdisc_t	outdirdisc; \
	Dtdisc_t	symdisc; \
	Dtdisc_t	dirdisc; \
	Dirlist_t	dirs; \
	Include_t*	include; \
	char*		file; \
	char*		ofile; \
	int		oline; \
	char*		card; \
	char*		peekcard; \
	char*		record; \
	char*		end; \
	_JCL_PUSH_ \
	Push_t		push[2]; \
	int		pushed; \
	int		abend; \
	int		rc; \
	int		eof; \
	int		canon; \
	int		redirect[elementsof(redirect)]; \
	Ie_t*		ie; \
	Ie_t*		iefree;

#define _JCLDD_PRIVATE_ \
	int		space; \
	Jclcat_t*	last;

#define diff		_jcl_diff
#define expand		_jcl_expand
#define lookup		_jcl_lookup
#define mark		_jcl_mark
#define marked		_jcl_marked
#define matched		_jcl_matched
#define nospace		_jcl_nospace
#define redirect	_jcl_redirect
#define stash		_jcl_stash
#define suflen		_jcl_suflen
#define uniq		_jcl_uniq

extern Redirect_t	redirect[2];

#include "jcl.h"

#include <ctype.h>
#include <debug.h>

extern int		diff(const char*, const char*, Jcldisc_t*);
extern char*		expand(Jcl_t*, const char*, int);
extern char*		lookup(Jcl_t*, const char*, const char*, int, int);
extern char*		mark(const char*, int, size_t, Jcldisc_t*);
extern int		marked(const char*, Jcldd_t*, Jcldisc_t*);
extern char*		matched(int, size_t*, Jcldisc_t*);
extern void		nospace(Jcl_t*, Jcldisc_t*);
extern char*		stash(Jcl_t*, Vmalloc_t*, const char*, int);
extern int		suflen(const char*);
extern void		uniq(const char*, const char*, unsigned long, Jcldisc_t*);

#endif
