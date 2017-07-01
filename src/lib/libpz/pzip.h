/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2011 AT&T Intellectual Property          *
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
 * partitioned fixed record zip library interface
 */

#ifndef _PZIP_H
#define _PZIP_H		1

#include <ast.h>
#include <cdt.h>
#include <option.h>
#include <sfdcgzip.h>
#include <vmalloc.h>

#define PZ_PLUGIN_VERSION	AST_PLUGIN_VERSION(PZ_VERSION)

#define PZ_VERSION	19990811L	/* interface version		*/

#if __STDC__
#define PZLIB(m)	unsigned long	plugin_version(void) { return PZ_PLUGIN_VERSION; }
#else
#define PZLIB(m)	unsigned long	plugin_version() { return PZ_PLUGIN_VERSION; }
#endif

#define PZ_DATA_SUF	"pz"		/* data file suffix		*/
#define PZ_PART_SUF	"prt"		/* partition file suffix	*/

#define PZ_WINDOW	(4*1024*1024)	/* default window size		*/

#define PZ_MAGIC_1	0x1f		/* first magic char		*/
#define PZ_MAGIC_2	0x10		/* second magic char		*/

#define PZ_OPEN		0		/* open event			*/
#define PZ_REOPEN	1		/* reopen event			*/
#define PZ_CLOSE	2		/* close event			*/
#define PZ_OPTION	3		/* run time option event	*/
#define PZ_PARTITION	4		/* new partition event		*/
#define PZ_TAILREAD	5		/* data tail read event		*/
#define PZ_TAILWRITE	6		/* data tail write event	*/

#define PZ_READ		0x00000001	/* open for read		*/
#define PZ_WRITE	0x00000002	/* open for write		*/
#define PZ_FORCE	0x00000004	/* open even if not pzip format	*/
#define PZ_STAT		0x00000008	/* open to stat header only	*/
#define PZ_STREAM	0x00000010	/* path arg is Sfio_t* stream	*/
#define PZ_CRC		0x00000020	/* (redundant) inflate crc	*/
#define PZ_DUMP		0x00000040	/* detailed trace		*/
#define PZ_VERBOSE	0x00000080	/* intermediate trace		*/
#define PZ_REGRESS	0x00000100	/* regression test output	*/
#define PZ_DIO		0x00000200	/* push sfdcdio()		*/
#define PZ_NOGZIP	0x00000400	/* no gzip			*/
#define PZ_SECTION	0x00000800	/* process one pzip section	*/
#define PZ_UPDATE	0x00001000	/* header update needed		*/
#define PZ_OVERSIZE	0x00002000	/* row elts for Pzpart_t vectors*/
#define PZ_HEAD		0x00004000	/* header already processed	*/
#define PZ_NOPZIP	0x00008000	/* no pzip			*/
#define PZ_ACCEPT	0x00010000	/* accept input (manual id)	*/
#define PZ_DISC		0x00020000	/* discipline push -- any ok	*/
#define PZ_APPEND	0x00040000	/* discipline append op		*/
#define PZ_SUMMARY	0x00080000	/* summary trace		*/
#define PZ_PUSHED	0x00100000	/* internal sfdcpzip() stream	*/
#define PZ_SPLIT	0x00200000	/* open for split		*/
#define PZ_SORT		0x00400000	/* sort window before deflate	*/
#define PZ_VARIABLE	0x00800000	/* variable row size		*/

#define PZ_VAR_OFF(r)	((r)&0x3fff)	/* row => offset		*/
#define PZ_VAR_LEN(r)	(1<<(((r)>>14)&0x3))	/* row => size len	*/

#define PZ_VERIFY	PZ_STAT		/* verify but don't push	*/

#define SFPZ_HANDLE	SFDCEVENT('P','Z',1)

struct Pz_s; typedef struct Pz_s Pz_t;
struct Pzconvert_s; typedef struct Pzconvert_s Pzconvert_t;
struct Pzcount_s; typedef struct Pzcount_s Pzcount_t;
struct Pzdisc_s; typedef struct Pzdisc_s Pzdisc_t;
struct Pzformat_s; typedef struct Pzformat_s Pzformat_t;
struct Pzindex_s; typedef struct Pzindex_s Pzindex_t;
struct Pzpart_s; typedef struct Pzpart_s Pzpart_t;
struct Pzsplit_s; typedef struct Pzsplit_s Pzsplit_t;

typedef ssize_t (*Pzconvert_f)(Pz_t*, Pzconvert_t*, const unsigned char*, unsigned char*, Pzdisc_t*);
typedef int (*Pzevent_f)(Pz_t*, int, void*, size_t, Pzdisc_t*);
typedef int (*Pzindex_f)(Pz_t*, Pzindex_t*, void*, Pzdisc_t*);
typedef const char* (*Pzinit_f)(Pz_t*, Pzdisc_t*);
typedef char* (*Pzname_f)(Pz_t*, unsigned long, Pzdisc_t*);
typedef Pzsplit_t* (*Pzsplit_f)(Pz_t*, Sfio_t*, Pzdisc_t*);
typedef ssize_t (*Pzread_f)(Pz_t*, Sfio_t*, void*, Pzdisc_t*);
typedef ssize_t (*Pzwrite_f)(Pz_t*, Sfio_t*, const void*, Pzdisc_t*);

struct Pzcount_s			/* caller/library counters	*/
{
	Sfulong_t	uncompressed;	/* uncompressed size		*/
	Sfulong_t	compressed;	/* compressed size		*/
	Sfulong_t	windows;	/* # windows			*/
	Sfulong_t	sections;	/* # split sections		*/
	Sfulong_t	records;	/* # records			*/
	Sfulong_t	modules;	/* # split records		*/
	Sfulong_t	converted;	/* # converted records		*/
	Sfulong_t	repaired;	/* # repaired records		*/
	Sfulong_t	truncated;	/* # truncated records		*/
	Sfulong_t	dropped;	/* # dropped records		*/
};

struct Pzformat_s			/* pzdcconvert() format info	*/
{
	const char*	name;		/* format (partition) name	*/
	size_t		row;		/* PZ_FORCE row size match	*/
	const char*	description;	/* format description		*/
};

struct Pzconvert_s			/* pzdcconvert() table element	*/
{
	Pzformat_t*	from;		/* from format			*/
	Pzformat_t*	to;		/* to format			*/
	Pzconvert_f	convertf;	/* row conversion function	*/
	Pzevent_f	eventf;		/* conversion event function	*/
	void*		userdata;	/* caller defined, 0 by default	*/
};

struct Pzdisc_s				/* user discipline		*/
{
	unsigned long	version;	/* PZ_VERSION			*/
	const char*	comment;	/* zipped data comment		*/
	const char*	options;	/* run time options		*/
	const char*	partition;	/* data partition file		*/
	const char*	lib;		/* pathfind() lib		*/
	size_t		window;		/* max window size		*/
	Error_f		errorf;		/* error function		*/
	Pzevent_f	eventf;		/* event function		*/
	Pzinit_f	initf;		/* library init function	*/
	Pzread_f	readf;		/* partition row read function	*/
	Pzwrite_f	writef;		/* partition row write function	*/
	Pzindex_f	indexf;		/* index generation function	*/
	Pzsplit_f	splitf;		/* record split function	*/
	Pzname_f	namef;		/* record id name function	*/
	void*		local;		/* discipline local data	*/
};

struct Pzindex_s			/* record index			*/
{
	Sfulong_t	block;		/* compressed block		*/
	Sfulong_t	offset;		/* offset in block		*/
};

struct Pzpart_s				/* partition context		*/
{
	char*		name;		/* partition name		*/
	unsigned long	flags;		/* PZ_* flags			*/
	size_t		row;		/* # chars per row		*/
	size_t		col;		/* # chars per col per win	*/
	size_t		loq;		/* lo frequency window size	*/

	/* hi frequency info						*/

	size_t*		map;		/* external to internal map	*/
	size_t*		grp;		/* column groupings		*/
	unsigned char**	mix;		/* column mix			*/
	size_t*		lab;		/* column mix group label	*/
	size_t*		inc;		/* column mix increment		*/
	size_t		nmap;		/* # elements in map		*/
	size_t		ngrp;		/* # elements in grp		*/

	/* lo frequency info						*/

	unsigned char*	low;		/* 1 if lo frequency, 0 if hi	*/
	int*		value;		/* fixed value if >= 0		*/
	size_t*		fix;		/* fixed columns		*/
	size_t		nfix;		/* # fixed values		*/

	/* caller defined info						*/

	void*		userdata;	/* caller defined, 0 by default	*/

#ifdef	_PZ_PART_PRIVATE_
	_PZ_PART_PRIVATE_
#endif

};

struct Pzsplit_s			/* splitf record info		*/
{
	unsigned long	id;		/* id: 0 == sized		*/
	void*		data;		/* data				*/
	size_t		size;		/* module size			*/
	size_t		record;		/* record size (guess) if header*/
};

struct Pz_s				/* pzip context			*/
{
	const char*	id;		/* library id			*/
	Pzdisc_t*	disc;		/* user discipline		*/
	unsigned long	flags;		/* PZ_* flags			*/
	unsigned long	test;		/* implementation test mask	*/
	int		major;		/* data major version number	*/
	int		minor;		/* data minor version number	*/
	size_t		row;		/* default input row size	*/
	size_t		win;		/* window size			*/
	const char*	path;		/* io path			*/
	Sfio_t*		io;		/* io stream			*/
	Vmalloc_t*	vm;		/* local malloc region		*/
	Pzpart_t*	part;		/* current partition		*/
	Pzcount_t	count;		/* caller/library counts	*/

	/* workspace info						*/

	unsigned char*	buf;		/* window buffer		*/
	unsigned char*	wrk;		/* window buffer		*/
	unsigned char*	pat;		/* row buffer			*/
	unsigned char*	nxt;		/* next val pointer		*/
	unsigned char*	val;		/* lo frequency window buffer	*/

	/* caller defined info						*/

	void*		userdata;	/* caller defined, 0 by default	*/

#ifdef	_PZ_PRIVATE_
	_PZ_PRIVATE_
#endif

};

#if defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern const char*	pz_init(Pz_t*, Pzdisc_t*);

#undef	extern

#if _BLD_pz && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Pz_t*		pzopen(Pzdisc_t*, const char*, unsigned long);
extern int		pzclose(Pz_t*);

extern ssize_t		pzread(Pz_t*, void*, size_t);
extern ssize_t		pzwrite(Pz_t*, Sfio_t*, const void*, size_t);
extern int		pzsync(Pz_t*);
extern int		pzfile(Pz_t*);

extern int		pzdeflate(Pz_t*, Sfio_t*);
extern int		pzinflate(Pz_t*, Sfio_t*);

extern Sfio_t*		pzfind(Pz_t*, const char*, const char*, const char*);
extern int		pzlib(Pz_t*, const char*, int);
extern const char*	pzinit(Pz_t*, const char*, Pzinit_f);

extern int		pzoptions(Pz_t*, Pzpart_t*, char*, int);
extern int		pzpartinit(Pz_t*, Pzpart_t*, const char*);
extern int		pzpartmap(Pz_t*, Pzpart_t*);
extern int		pzpartition(Pz_t*, const char*);
extern int		pzpartprint(Pz_t*, Pzpart_t*, Sfio_t*);
extern int		pzpartread(Pz_t*);
extern int		pzpartwrite(Pz_t*, Sfio_t*);
extern Pzpart_t*	pzpartget(Pz_t*, const char*);
extern Pzpart_t*	pzpartnext(Pz_t*, Pzpart_t*);
extern Pzpart_t*	pzpartset(Pz_t*, Pzpart_t*);

extern int		pzheadread(Pz_t*);
extern int		pzheadwrite(Pz_t*, Sfio_t*);
extern int		pzheadprint(Pz_t*, Sfio_t*, int);

extern int		pzdcconvert(Pz_t*, const Pzconvert_t*);

extern ssize_t		pzfixed(Pz_t*, Sfio_t*, void*, size_t);

extern int		sfdcpzip(Sfio_t*, const char*, unsigned long, Pzdisc_t*);
extern int		sfdczip(Sfio_t*, const char*, const char*, Error_f);

#undef	extern

#endif
