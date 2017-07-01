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
 * ibm dfsort support public interface
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _SS_H
#define _SS_H	1

#include <ast.h>
#include <error.h>
#include <recsort.h>
#include <recfmt.h>
#include <vmalloc.h>

#define SS_VERSION	20050611L

#define SS_MARKED	"*%+([a-z0-9])?(" SS_SUFFIX ")"
#define SS_SUFFIX	".*([!A-Z/])"

#define SS_V_IBM	REC_V_TYPE(4,0,2,0,1)

#define SS_DD_IN	"SORTIN"
#define SS_DD_OUT	"SORTOF"
#define SS_DD_XSUM	"SORTXSUM"

#define SS_ascii	'a'
#define SS_bcd		'p'
#define SS_be		'b'
#define SS_bit		'B'
#define SS_ebcdic	'c'
#define SS_ibm		'i'
#define SS_le		'l'
#define SS_ss		'S'
#define SS_void		'v'
#define SS_zd		'z'
#define SS_AC_alt	'Q'
#define SS_AC_dec	'D'
#define SS_AC_flt	'F'
#define SS_AC_hex	'H'
#define SS_AC_oct	'O'
#define SS_CH_dec	'd'
#define SS_CH_flt	'f'
#define SS_CH_hex	'h'
#define SS_CH_oct	'o'

#define SS_OP_false	0
#define SS_OP_true	1
#define SS_OP_field	2
#define SS_OP_value	3
#define SS_OP_lt	4
#define SS_OP_le	5
#define SS_OP_eq	6
#define SS_OP_ne	7
#define SS_OP_ge	8
#define SS_OP_gt	9
#define SS_OP_and	10
#define SS_OP_or	11
#define SS_OP_not	12

#define ssopdata(o)	((o)<=SS_OP_value)
#define ssopexpr(o)	((o)>=SS_OP_and)

typedef uintmax_t Sscount_t;

#ifndef _SSEXIT_H
typedef int (*Ssexit_f)(Rsobj_t*, Rsobj_t*, void**);
#endif
typedef int (*Ssintercept_f)(Ssexit_f, Rsobj_t*, Rsobj_t*, void**);

struct Ssdisc_s; typedef struct Ssdisc_s Ssdisc_t;
struct Ssexpr_s; typedef struct Ssexpr_s Ssexpr_t;
struct Ssfield_s; typedef struct Ssfield_s Ssfield_t;
struct Ssfile_s; typedef struct Ssfile_s Ssfile_t;
struct Ssgroup_s; typedef struct Ssgroup_s Ssgroup_t;
struct Ss_s; typedef struct Ss_s Ss_t;

union Ssop_u; typedef union Ssop_u Ssop_t;

struct Ssdisc_s				/* user discipline		*/
{
	unsigned long	version;	/* interface version		*/
	int		code;		/* default data codeset		*/
	Error_f		errorf;		/* error function		*/
};

struct Ssfield_s
{
	Ssfield_t*	next;		/* next in list			*/
	size_t		offset;		/* source record offset		*/
	size_t		size;		/* size in bytes		*/
	char*		value;		/* optional default value	*/
	unsigned char	type;		/* optional type		*/
	unsigned char	reverse;	/* reverse order		*/
};

union Ssop_u				/* expression operand		*/
{
	Ssfield_t*	field;		/* field			*/
	Ssexpr_t*	expr;		/* expression			*/
};

struct Ssexpr_s				/* expression node		*/
{
	Ssop_t		left;		/* left operand			*/
	Ssop_t		right;		/* right operand		*/
	unsigned char	op;		/* operation			*/
};

struct Ssgroup_s			/* file group circular list	*/
{
	Ssgroup_t*	next;		/* next in group (circular)	*/
	char*		id;		/* id name			*/
	char*		name;		/* file name			*/
	Sfio_t*		io;		/* file stream			*/
#ifdef _SS_GROUP_PRIVATE_
	_SS_GROUP_PRIVATE_
#endif
};

struct Ssfile_s				/* file info			*/
{
	Ssfile_t*	next;		/* next file group		*/
	Ssgroup_t*	group;		/* file group			*/
	Ssexpr_t*	expr;		/* include/omit			*/
	Ssfield_t*	out;		/* output fields		*/
	size_t		size;		/* output record size		*/
	Recfmt_t	format;		/* output record format		*/
	unsigned char	omit;		/* expr is omit			*/
	unsigned char	save;		/* include if omitted by others	*/
#ifdef _SS_FILE_PRIVATE_
	_SS_FILE_PRIVATE_
#endif
};

struct Ss_s				/* handle state			*/
{
	Ssfile_t*	file;		/* file list -- at least one	*/
	Ssfield_t*	in;		/* input record fields		*/
	Ssfield_t*	sort;		/* optional sort fields		*/
	Ssfield_t*	sum;		/* optional sum fields		*/
	Ssdisc_t*	disc;		/* user discipline		*/
	Vmalloc_t*	vm;		/* handle memory region		*/
	Sscount_t	skip;		/* skip this many records	*/
	Sscount_t	stop;		/* stop after this many records	*/
	size_t		size;		/* input record fixed size	*/
	size_t		insize;		/* in record size		*/
	Recfmt_t	format;		/* input record format		*/
	char*		suffix;		/* input file suffix		*/
	Ssexpr_t*	expr;		/* global include/omit		*/
	void*		exitstate;	/* user exit state		*/
	Ssexit_f	initexit;	/* RS_OPEN exit			*/
	Ssexit_f	doneexit;	/* RS_POP exit			*/
	Ssexit_f	readexit;	/* RS_READ exit			*/
	Ssexit_f	summaryexit;	/* RS_SUMMARY exit		*/
	Ssexit_f	writeexit;	/* RS_WRITE exit		*/
	Ssintercept_f	intercept;	/* Ssexit_f intercept		*/
	unsigned char	copy;		/* copy (no sort)		*/
	unsigned char	mark;		/* mark output files with %size	*/
	unsigned char	merge;		/* merge (no sort)		*/
	unsigned char	omit;		/* expr is omit			*/
	unsigned char	stable;		/* stable sort: 'Y'|'N'|default	*/
	unsigned char	type;		/* input record type		*/
	unsigned char	uniq;		/* drop dup records		*/
#ifdef _SS_PRIVATE_
	_SS_PRIVATE_
#endif
};

#define ssinit(d,e)	(memset(d,0,sizeof(Ssdisc_t)),(d)->version=SS_VERSION,(d)->errorf=(Error_f)(e))

extern Ss_t*		ssopen(const char*, Ssdisc_t*);
extern int		ssdd(const char*, const char*, Ssdisc_t*);
extern int		sseval(Ss_t*, Ssexpr_t*, const char*, size_t);
extern char*		sskey(Ss_t*, Ssfield_t*);
extern int		sslist(Ss_t*, Sfio_t*);
extern int		sssum(Ss_t*, Ssfield_t*, const char*, size_t, char*);
extern ssize_t		sscopy(Ss_t*, Ssfile_t*, const char*, size_t, char*, size_t);
extern ssize_t		sswrite(Ss_t*, Ssfile_t*, const char*, size_t);
extern int		ssio(Ss_t*, int);
extern int		ssannounce(Ss_t*, Rs_t*);
extern int		ssclose(Ss_t*);

#endif
