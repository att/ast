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

#ifndef _JCL_H
#define _JCL_H		1

#include <ast.h>
#include <dt.h>
#include <error.h>
#include <vmalloc.h>

#define JCL_VERSION		20050214L

#define JCL_AUTO		"JCL_AUTO_"
#define JCL_MAPFILE		"prefix"

#define JCL_CREATE		0x00000001
#define JCL_EXEC		0x00000002
#define JCL_IMPORT		0x00000004
#define JCL_LISTAUTOEDITS	0x00000008
#define JCL_LISTCOUNTS		0x00000010
#define JCL_LISTDD		0x00000020
#define JCL_LISTEXEC		0x00000040
#define JCL_LISTINPUTS		0x00000080
#define JCL_LISTJOBS		0x00000100
#define JCL_LISTOUTPUTS		0x00000200
#define JCL_LISTPARMS		0x00000400
#define JCL_LISTPROGRAMS	0x00000800
#define JCL_LISTSCRIPTS		0x00001000
#define JCL_LISTVARIABLES	0x00002000
#define JCL_MAPPED		0x00004000
#define JCL_MARKLENGTH		0x00008000
#define JCL_PARAMETERIZE	0x00010000
#define JCL_RECURSE		0x00020000
#define JCL_SCOPE		0x00040000
#define JCL_STANDARD		0x00080000
#define JCL_SUBDIR		0x00100000
#define JCL_SUBTMP		0x00200000
#define JCL_TRACE		0x00400000
#define JCL_VERBOSE		0x00800000
#define JCL_WARN		0x01000000
#define JCL_GDG			0x02000000

#define JCL_JOB			0x20000000
#define JCL_PGM			0x40000000
#define JCL_PROC		0x80000000

#define JCL_INHERIT		(~(JCL_SCOPE))

#define JCL_DD_SYSIN		0x0001
#define JCL_DD_SYSOUT		0x0002
#define JCL_DD_SYSERR		0x0004
#define JCL_DD_REFERENCE	0x0008
#define JCL_DD_INCLUDE		0x0010
#define JCL_DD_DUMMY		0x0020
#define JCL_DD_DIR		0x0040
#define JCL_DD_ALIAS		0x0080
#define JCL_DD_MARKED		0x0100

#define JCL_RECFM_A		0x01
#define JCL_RECFM_B		0x02
#define JCL_RECFM_D		0x04
#define JCL_RECFM_F		0x08
#define JCL_RECFM_M		0x10
#define JCL_RECFM_S		0x20
#define JCL_RECFM_U		0x40
#define JCL_RECFM_V		0x80

#define JCL_SYM_EXPORT		0x01
#define JCL_SYM_IMPORT		0x02
#define JCL_SYM_READONLY	0x04
#define JCL_SYM_SET		0x08

#define JCL_DISP_NEW		'N'
#define JCL_DISP_OLD		'O'
#define JCL_DISP_SHR		'S'
#define JCL_DISP_MOD		'M'

#define JCL_DISP_DELETE		'D'
#define JCL_DISP_KEEP		'K'
#define JCL_DISP_PASS		'P'
#define JCL_DISP_CATLG		'C'
#define JCL_DISP_UNCATLG	'U'

#define JCL_COND_ONLY		1
#define JCL_COND_EVEN		2
#define JCL_COND_LT		3
#define JCL_COND_LE		4
#define JCL_COND_EQ		5
#define JCL_COND_NE		6
#define JCL_COND_GE		7
#define JCL_COND_GT		8
#define JCL_COND_OR		9
#define JCL_COND_AND		10

struct Jclcat_s; typedef struct Jclcat_s Jclcat_t;
struct Jclcond_s; typedef struct Jclcond_s Jclcond_t;
struct Jcldd_s; typedef struct Jcldd_s Jcldd_t;
struct Jcldir_s; typedef struct Jcldir_s Jcldir_t;
struct Jcldisc_s; typedef struct Jcldisc_s Jcldisc_t;
struct Jclout_s; typedef struct Jclout_s Jclout_t;
struct Jcloutput_s; typedef struct Jcloutput_s Jcloutput_t;
struct Jclstep_s; typedef struct Jclstep_s Jclstep_t;
struct Jclsym_s; typedef struct Jclsym_s Jclsym_t;
struct Jcl_s; typedef struct Jcl_s Jcl_t;

typedef int (*Jcloptset_f)(Jcl_t*, int, Jcldisc_t*);

struct Jcldisc_s
{
	unsigned long	version;		/* interface version	*/
	Error_f		errorf;			/* error function	*/
	const char*	usage;			/* option usage		*/
	Jcloptset_f	optsetf;		/* option function	*/
	time_t		date;			/* system date		*/
	time_t		odate;			/* original date	*/
	time_t		rdate;			/* current run date	*/
};

struct Jclcat_s
{
	Jclcat_t*	next;
	char*		path;
};

struct Jclout_s
{
	Jclout_t*	next;
	Jcloutput_t*	output;
};

struct Jclcond_s
{
	Jclcond_t*	next;
	char*		step;
	short		code;
	short		op;
};

struct Jcldd_s
{
	Dtlink_t	link;
	char*		name;
	char*		path;
	char*		here;
	char*		card;
	char		dlm[3];
	int		lrecl;
	int		reference;
	unsigned short	flags;
	unsigned char	recfm;
	unsigned char	disp[3];
	Jclcat_t*	cat;
	Jclout_t*	out;
#ifdef _JCLDD_PRIVATE_
	_JCLDD_PRIVATE_
#endif
};

struct Jcloutput_s
{
	Dtlink_t	link;
	char*		name;
	char*		parm;
#ifdef _JCLOUTPUT_PRIVATE_
	_JCLOUTPUT_PRIVATE_
#endif
};

struct Jcldir_s
{
	Dtlink_t	link;
	char		name[1];
};

struct Jclsym_s
{
	Dtlink_t	link;
	char*		value;
	int		flags;
	char		name[1];
};

struct Jclstep_s
{
	char*		name;
	char*		command;
	char*		parm;
	Dt_t*		dd;
	Jclcond_t*	cond;
	Dt_t*		syms;
	Vmalloc_t*	vm;
	unsigned long	flags;
};

struct Jcl_s
{
	char*		id;
	Jcldisc_t*	disc;
	Vmalloc_t*	vm;
	unsigned long	flags;
	unsigned long	roflags;
	unsigned long	steps;
	unsigned long	passed;
	unsigned long	failed;
	char*		name;
	char*		tmp;
	Jclcond_t*	cond;
	Dt_t*		dd;
	Dt_t*		output;
	Dt_t*		syms;
	Jclstep_t*	step;
#ifdef _JCL_PRIVATE_
	_JCL_PRIVATE_
#endif
};

#define jclinit(d,e)	(memset(d,0,sizeof(*(d))),(d)->version=JCL_VERSION,(d)->errorf=(Error_f)(e),(d)->date=(d)->odate=(d)->rdate=time(NiL))

extern int		jclmap(Jcl_t*, const char*, Jcldisc_t*);
extern int		jclstats(Sfio_t*, unsigned long, Jcldisc_t*);

extern int		jclclose(Jcl_t*);
extern int		jcleval(Jcl_t*, Jclcond_t*, int);
extern char*		jclfind(Jcl_t*, const char*, unsigned long, int, Sfio_t**);
extern int		jclinclude(Jcl_t*, const char*, unsigned long, Jcldisc_t*);
extern Jcl_t*		jclopen(Jcl_t*, const char*, unsigned long, Jcldisc_t*);
extern char*		jclparm(char**);
extern char*		jclpath(Jcl_t*, const char*);
extern int		jclpop(Jcl_t*);
extern int		jclpush(Jcl_t*, Sfio_t*, const char*, long);
extern int		jclrc(Jcl_t*, Jclstep_t*, int);
extern int		jclrun(Jcl_t*);
extern Jclstep_t*	jclstep(Jcl_t*);
extern Jclsym_t*	jclsym(Jcl_t*, const char*, const char*, int);

#endif
