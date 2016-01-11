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
 * AT&T Bell Laboratories
 *
 * make abstract machine definitions
 */

#ifndef _MAM_H
#define _MAM_H

#include <ast.h>
#include <hash.h>

#define getrule(p,n)	(struct rule*)hashget((p)->rules,(n))
#define putrule(p,n,r)	hashput((p)->rules,(char*)(n),(char*)(r))
#define getvar(p,n)	(struct var*)hashget((p)->vars,(n))
#define putvar(p,n,v)	hashput((p)->vars,(char*)(n),(char*)(v))

#define A_archive	(1<<0)		/* archive target		*/
#define A_dontcare	(1<<1)		/* don't care if error		*/
#define A_metarule	(1<<2)		/* metarule info		*/
#define A_virtual	(1<<3)		/* a file not			*/
#define A_LAST		(1<<7)		/* last A_* bit used		*/

#define B_LAST		(1<<7)		/* last B_* bit used		*/

#define V_LAST		(1<<7)		/* last V_* bit used		*/

struct rule				/* rule info			*/
{
	char*		name;		/* rule name			*/
	char*		bound;		/* bound name			*/
	time_t		time;		/* modify time			*/
	long		attributes;	/* A_* attributes		*/
	int		status;		/* action exit status		*/
	struct list*	prereqs;	/* explicit prerequisite list	*/
	struct list*	implicit;	/* implicit prerequisite list	*/
	struct block*	action;		/* action			*/
	union
	{
		char*	pointer;
		long	number;
	}		local;		/* user defined			*/

#ifdef _MAM_RULE_PRIVATE
	_MAM_RULE_PRIVATE
#endif

};

struct var				/* variable info		*/
{
	char*		name;		/* var name			*/
	char*		value;		/* var value			*/
	long		attributes;	/* V_* attributes		*/
	union
	{
		char*	pointer;
		long	number;
	}		local;		/* user defined			*/

#ifdef _MAM_VAR_PRIVATE
	_MAM_VAR_PRIVATE
#endif

};

struct list				/* prereq list			*/
{
	struct list*	next;		/* next prereq			*/
	struct rule*	rule;		/* this prereq			*/
};

struct block				/* data block list		*/
{
	struct block*	next;		/* next item			*/
	char*		data;		/* this item			*/
	long		attributes;	/* B_* attributes		*/
};

struct proc				/* mam process trace		*/
{
	struct rule*	root;		/* root target			*/
	struct proc*	parent;		/* parent proc			*/
	struct proc*	child;		/* child proc list		*/
	struct proc*	sibling;	/* sibling procs		*/
	char*		pwd;		/* pwd				*/
	char*		view;		/* 3d view			*/
	Hash_table_t*	rules;		/* rule hash			*/
	Hash_table_t*	vars;		/* variable hash		*/
	long		pid;		/* pid (invalid now)		*/
	int		status;		/* exit status			*/
	time_t		start;		/* start time			*/
	time_t		finish;		/* finish time			*/

#ifdef _MAM_PROC_PRIVATE
	_MAM_PROC_PRIVATE
#endif

};

struct mam				/* mam state			*/
{
	const char*	id;		/* library id			*/
	char*		version;	/* input version		*/
	struct proc*	main;		/* main proc			*/

#ifdef _MAM_MAM_PRIVATE
	_MAM_MAM_PRIVATE
#endif

};

/*
 * library globals
 */

extern struct mam*	mamalloc(void);
extern void		mamfree(struct mam*);
extern int		mamscan(struct mam*, const char*);
extern struct rule*	mamrule(struct proc*, const char*);
extern struct var*	mamvar(struct proc*, const char*, const char*);
extern void		mamprereq(struct proc*, struct rule*, struct rule*, struct list**);

#endif
