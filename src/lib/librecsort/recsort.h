/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#ifndef _RECSORT_H
#define	_RECSORT_H		1

#if _PACKAGE_ast
#include <ast.h>
#else
#include <sfio.h>
#endif

#define RS_PLUGIN_VERSION	AST_PLUGIN_VERSION(20100528L)
#define RS_VERSION		20030811L
#define RSKEY_VERSION		20111011L

#define SORTLIB(m)		unsigned long plugin_version(void) { return RS_PLUGIN_VERSION; }

typedef struct _rsobj_s		Rsobj_t;
typedef struct _rs_s		Rs_t;
typedef struct _rsmethod_s	Rsmethod_t;
typedef struct _rsdisc_s	Rsdisc_t;
typedef ssize_t 		(*Rsdefkey_f)_ARG_((Rs_t*, unsigned char*, size_t, unsigned char*, size_t, Rsdisc_t*));
typedef int			(*Rsevent_f)_ARG_((Rs_t*, int, Void_t*, Void_t*, Rsdisc_t*));

typedef struct _rskey_s		Rskey_t;
typedef struct _rskeydisc_s	Rskeydisc_t;
typedef struct _rskeyfield_s	Rskeyfield_t;
typedef int			(*Rskeyerror_f)_ARG_((Void_t*, Void_t*, int, ...));

typedef ssize_t (*Rskeycode_f)_ARG_((Rskey_t*, Rskeyfield_t*, unsigned char*, size_t, unsigned char*, unsigned char*));

struct _rsmethod_s
{	int		(*insertf)_ARG_((Rs_t*, Rsobj_t*));
	Rsobj_t*	(*listf)_ARG_((Rs_t*));
	ssize_t		size;	/* size of private data			*/
	int		type;	/* method type				*/
	char*		name;	/* method name				*/
	char*		desc;	/* method description			*/
};

struct _rsdisc_s
{	unsigned long	version;/* interface version			*/
	int		type;	/* types of key&data			*/
	ssize_t		data;	/* length or separator			*/
	ssize_t		key;	/* key offset or expansion factor	*/
	ssize_t		keylen;	/* >0 for key length else end-offset	*/
	Rsdefkey_f	defkeyf;/* to define key from data		*/
	Rsevent_f	eventf;	/* to announce various events		*/
	unsigned long	events;	/* events to announce			*/
	Rsdisc_t*	disc;	/* next in stack			*/
};

struct _rsobj_s
{	unsigned long	order;	/* for fast compare or ordinal 		*/
	Rsobj_t*	left;	/* left/last link or out of order	*/
	Rsobj_t*	right;	/* next record in sorted list		*/
	Rsobj_t*	equal;	/* equivalence class			*/
	unsigned char*	key;	/* object key				*/
	ssize_t		keylen;	/* key length				*/
	unsigned char*	data;	/* object data				*/
	ssize_t		datalen;/* data length				*/
};

struct _rskeydisc_s
{	unsigned long	version;	/* interface version		*/
	unsigned long	flags;		/* RSKEY_* flags		*/
	Rskeyerror_f	errorf;		/* error function		*/
};

struct _rskeyfield_s
{	Rskeyfield_t*	next;		/* next in list			*/
	Rskeycode_f	coder;		/* encode data into key		*/
	void*		user;		/* user specific data		*/

	unsigned	flag;		/* code flag			*/
	unsigned char	rflag;		/* reverse order		*/

#ifdef _RSKEYFIELD_PRIVATE_
	_RSKEYFIELD_PRIVATE_
#endif
};

struct _rskey_s
{	const char*	id;		/* library id			*/
	Rskeydisc_t*	keydisc;	/* rskey discipline		*/
	Rsdisc_t*	disc;		/* rsopen() discipline		*/
	Rsmethod_t*	meth;		/* rsopen() method		*/
	int		type;		/* rsopen() type		*/

	char**		input;		/* input files			*/

	char*		output;		/* output file name		*/

	size_t		alignsize;	/* buffer alignment size	*/
	size_t		fixed;		/* fixed record size		*/
	size_t		insize;		/* input buffer size		*/
	size_t		outsize;	/* output buffer size		*/
	size_t		procsize;	/* process buffer size		*/
	size_t		recsize;	/* max record size		*/
	unsigned long	test;		/* test mask			*/

	int		merge;		/* merge sorted input files	*/
	int		nproc;		/* max number of processes	*/
	int		verbose;	/* trace execution		*/
	int		code;		/* global ccode translation	*/

	unsigned char	tab[32];	/* global tab char/string	*/

	Rskeyfield_t*	head;		/* key field list head		*/
	Rskeyfield_t*	tail;		/* key field list tail		*/

#ifdef _RSKEY_PRIVATE_
	_RSKEY_PRIVATE_
#endif
};

struct _rs_s
{	Rsmethod_t*	meth;	/* method to sort			*/
	Rsdisc_t*	disc;	/* discipline describing data		*/
	Sfulong_t	count;	/* number of accumulated objects	*/
	int		type;
#ifdef _RS_PRIVATE_
	_RS_PRIVATE_
#endif
};

/* events */
#define RS_CLOSE	0000001		/* sort context is being closed	*/
#define RS_DISC		0000002		/* discipline is being changed	*/
#define RS_METHOD	0000004		/* method is being changed	*/
#define RS_OPEN		0001000		/* rsopen() is being called	*/
#define RS_POP		0000010		/* discipline is being popped	*/
#define RS_PUSH		0000020		/* discipline is being pushed	*/
#define RS_READ		0000200		/* called for each read record	*/
#define RS_SUMMARY	0000040		/* RS_UNIQ summary		*/
#define RS_VERIFY	0000100		/* objects out of order		*/
#define RS_WRITE	0000400		/* called for each write record	*/
#define RS_FILE_WRITE	0002000		/* rsfilewrite() callout	*/
#define RS_FILE_READ	0004000		/* rsfileread() callout		*/
#define RS_FILE_CLOSE	0010000		/* rsfileclose() callout	*/
#define RS_TEMP_WRITE	0020000		/* rstempwrite() callout	*/
#define RS_TEMP_READ	0040000		/* rstempread() callout		*/
#define RS_TEMP_CLOSE	0100000		/* rstempclose() callout	*/

/* { RS_READ RS_SUMMARY RS_WRITE } event returns */
#define RS_TERMINATE	(-1)		/* terminate the sort		*/
#define RS_ACCEPT	0		/* accept possibly modified rec	*/
#define RS_DELETE	1		/* delete/ignore record		*/
#define RS_INSERT	2		/* insert new record		*/
#define RS_DONE		3		/* user defined			*/

#define RS_NEXT		0		/* rsdisc() next		*/

/* sort controls */
#define RS_UNIQ		0000001		/* remove duplicates		*/
#define RS_REVERSE	0000002		/* reverse sort order		*/
#define RS_DATA		0000004		/* sort by key, then by data	*/
#define RS_IGNORE	0000040		/* rswrite() will be ignored	*/
#define RS_CAT		0040000		/* just catenate input files	*/
#define RS_MORE		0010000		/* RS_LAST on last rsprocess()	*/
#define RS_LAST		0100000		/* the last rsprocess()		*/

/* discipline data */
#define RS_KSAMELEN	0000010		/* key has fixed length		*/
#define RS_DSAMELEN	0000020		/* data has fixed length	*/
#define RS_TYPES	0040377

/* input/output control */
#define RS_ITEXT	0000100		/* input is plain text		*/
#define RS_OTEXT	0000200		/* output is plain text		*/
#define RS_TEXT		0000300

/* method type */
#define RS_MTVERIFY	0000400
#define RS_MTRASP	0001000
#define RS_MTRADIX	0002000
#define RS_MTSPLAY	0004000
#define RS_MTCOPY	0020000

#define RSKEY_ERROR	000001		/* unrecoverable error		*/
#define RSKEY_KEYS	000002		/* keys specified		*/

#define RSKEYDISC(p)	(((Rskey_t*)((char*)(p)-offsetof(Rskey_t,disc)))->keydisc)

#define rscount(rs)	((rs)->count)	/* count # of rsprocess() objs	*/

_BEGIN_EXTERNS_	/* public data */

#if defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Rsdisc_t*	rs_disc _ARG_((Rskey_t*, const char*));

#undef	extern

#if _BLD_recsort && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif
#if !_BLD_recsort && defined(__IMPORT__)
#define extern		extern __IMPORT__
#endif

extern Rsmethod_t* Rscopy;	/* copy original order	*/
extern Rsmethod_t* Rsrasp;	/* radix + splay trees	*/
extern Rsmethod_t* Rsradix;	/* radix only		*/
extern Rsmethod_t* Rssplay;	/* splay insertion	*/
extern Rsmethod_t* Rsverify;	/* verify order		*/

#undef extern
_END_EXTERNS_

_BEGIN_EXTERNS_	/* public functions */
#if _BLD_recsort && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern Rs_t*		rsnew _ARG_((Rsdisc_t*));
extern int		rsinit _ARG_((Rs_t*, Rsmethod_t*, ssize_t, int, Rskey_t*));

extern Rs_t*		rsopen _ARG_((Rsdisc_t*, Rsmethod_t*, ssize_t, int));
extern int		rsclear _ARG_((Rs_t*));
extern int		rsclose _ARG_((Rs_t*));
extern ssize_t		rsprocess _ARG_((Rs_t*, Void_t*, ssize_t));
extern int		rslib _ARG_((Rs_t*, Rskey_t*, const char*, int));
extern Rsobj_t*		rslist _ARG_((Rs_t*));
extern int		rswrite _ARG_((Rs_t*, Sfio_t*, int));
extern int		rsmerge _ARG_((Rs_t*, Sfio_t*, Sfio_t**, int, int));
extern Rsdisc_t*	rsdisc _ARG_((Rs_t*, Rsdisc_t*, int));
extern Rsmethod_t*	rsmethod _ARG_((Rs_t*, Rsmethod_t*));

extern Rskey_t*		rskeyopen _ARG_((Rskeydisc_t*, Rsdisc_t*));
extern int		rskey _ARG_((Rskey_t*, const char*, int));
extern int		rskeyopt _ARG_((Rskey_t*, const char*, int));
extern Rsmethod_t*	rskeymeth _ARG_((Rskey_t*, const char*));
extern int		rskeylist _ARG_((Rskey_t*, Sfio_t*, int));
extern void		rskeydump _ARG_((Rskey_t*, Sfio_t*));
extern int		rskeyinit _ARG_((Rskey_t*));
extern int		rskeyclose _ARG_((Rskey_t*));

extern int		rsfilewrite _ARG_((Rs_t*, Sfio_t*, const char*));
extern int		rsfileread _ARG_((Rs_t*, Sfio_t*, const char*));
extern int		rsfileclose _ARG_((Rs_t*, Sfio_t*));

extern Sfio_t*		rstempwrite _ARG_((Rs_t*, Sfio_t*));
extern int		rstempread _ARG_((Rs_t*, Sfio_t*));
extern int		rstempclose _ARG_((Rs_t*, Sfio_t*));

#undef extern
_END_EXTERNS_

#endif /*_RECSORT_H*/
