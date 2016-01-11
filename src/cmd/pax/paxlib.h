/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2011 AT&T Intellectual Property          *
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
 * pax_lib() definitions
 */

#ifndef _PAXLIB_H
#define _PAXLIB_H		1

#include <ast.h>
#include <error.h>
#include <ls.h>
#include <modex.h>

#if _BLD_DEBUG
#include <vmalloc.h>
#endif

#define PAX_PLUGIN_VERSION	AST_PLUGIN_VERSION(20100528L)

#define PAX_IN		(1<<0)		/* copy in			*/
#define PAX_OUT		(1<<1)		/* copy out			*/
#define PAX_ARCHIVE	(1<<2)		/* archive format		*/
#define PAX_COMPRESS	(1<<3)		/* compress format		*/
#define PAX_DELTA	(1<<4)		/* delta format			*/
#define PAX_DLL		(1<<5)		/* format from external dll	*/
#define PAX_DOS		(1<<6)		/* may contain dos names	*/
#define PAX_PSEUDO	(1<<7)		/* pseudo delta format		*/
#define PAX_COMPRESSED	(1<<8)		/* format already compressed	*/
#define PAX_CONV	(1<<9)		/* format handles ccode conv	*/
#define PAX_DELTAINFO	(1<<10)		/* format handles delta info	*/
#define PAX_KEEPSIZE	(1<<11)		/* retain non REG st_size	*/
#define PAX_LINKTYPE	(1<<12)		/* Paxfile_t.linktype supported	*/
#define PAX_NOHARDLINKS	(1<<13)		/* hard links not supported	*/
#define PAX_SLASHDIR	(1<<14)		/* trailing slash => directory	*/
#define PAX_STANDARD	(1L<<15)	/* the standard format		*/
#define PAX_SUM		(1L<<16)	/* inline member checksum	*/
#define PAX_DELTAIO	(1L<<17)	/* separate delta io		*/
#define PAX_APPEND	(1L<<18)	/* archive append		*/

#define PAX_FORMAT	(1L<<24)	/* first format specific flag	*/

#define PAX_BLOCK	512		/* io block size		*/
#define PAX_DEFBLOCKS	20		/* default blocking		*/
#define PAX_DEFBUFFER	16		/* default io buffer blocking	*/

#define PAX_NOLINK	0		/* not a link			*/
#define PAX_HARDLINK	'1'		/* hard link to previous entry	*/
#define PAX_SOFTLINK	'2'		/* soft link to previous entry	*/

#define PAX_EVENT_BUG_19951031	0x00000001 /* old write bug workaround	*/
#define PAX_EVENT_DELTA_EXTEND	0x00000002 /* add delta entension info	*/
#define PAX_EVENT_SKIP_JUNK	0x00000004 /* junk header test		*/

struct Pax_s; typedef struct Pax_s Pax_t;
#ifndef Paxarchive_t
struct Paxarchive_s; typedef struct Paxarchive_s Paxarchive_t;
#endif
#ifndef Paxfile_t
struct Paxfile_s; typedef struct Paxfile_s Paxfile_t;
#endif
#ifndef Paxformat_t
struct Paxformat_s; typedef struct Paxformat_s Paxformat_t;
#endif
struct Paxio_s; typedef struct Paxio_s Paxio_t;
struct Paxvalue_s; typedef struct Paxvalue_s Paxvalue_t;

typedef Paxformat_t* (*Paxlib_f)(Pax_t*);

struct Paxvalue_s			/* string and/or number value	*/
{
	char*		string;		/* string value			*/
	int32_t		number;		/* numeric value		*/
	int32_t		fraction;	/* fractional part		*/
	size_t		size;		/* max string size		*/
};

struct Paxio_s				/* io info			*/
{
	int		fd;		/* file descriptor		*/
	int		eof;		/* hit EOF			*/
	off_t		buffersize;	/* buffer size			*/

#ifdef _PAX_IO_PRIVATE_
	_PAX_IO_PRIVATE_
#endif

};

struct Paxfile_s			/* common internal file info	*/
{
	char*		name;		/* archive file name		*/
	char*		path;		/* local file name for reading	*/
	char*		linkpath;	/* link path			*/
	struct stat*	st;		/* stat() info from ftwalk()	*/
	off_t		uncompressed;	/* uncompressed size if != 0	*/
	int		linktype;	/* link type			*/

#ifdef _PAX_FILE_PRIVATE_
	_PAX_FILE_PRIVATE_
#endif

};

struct Paxformat_s			/* format info			*/
{
	char*		name;		/* name				*/
	char*		match;		/* name strgrpmatch pattern	*/
	char*		desc;		/* description			*/
	int		variant;	/* variant index		*/
	int32_t		flags;		/* flags			*/
	unsigned long	regular;	/* default regular blocking	*/
	unsigned long	special;	/* default special blocking	*/
	int		align;		/* trailer alignment		*/
	struct Paxformat_s*next;	/* next in list of all formats	*/
	void*		data;		/* format specific data		*/
	int		(*done)(Pax_t*, Paxarchive_t*);
	int		(*getprologue)(Pax_t*, Paxformat_t*, Paxarchive_t*, Paxfile_t*, unsigned char*, size_t);
	int		(*getheader)(Pax_t*, Paxarchive_t*, Paxfile_t*);
	int		(*getdata)(Pax_t*, Paxarchive_t*, Paxfile_t*, int);
	int		(*gettrailer)(Pax_t*, Paxarchive_t*, Paxfile_t*);
	int		(*getepilogue)(Pax_t*, Paxarchive_t*);
	int		(*putprologue)(Pax_t*, Paxarchive_t*, int);
	int		(*putheader)(Pax_t*, Paxarchive_t*, Paxfile_t*);
	int		(*putdata)(Pax_t*, Paxarchive_t*, Paxfile_t*, int);
	int		(*puttrailer)(Pax_t*, Paxarchive_t*, Paxfile_t*);
	off_t		(*putepilogue)(Pax_t*, Paxarchive_t*);
	int		(*lookup)(Pax_t*, Paxarchive_t*, Paxfile_t*, int, char**, Sflong_t*);
	int		(*backup)(Pax_t*, Paxarchive_t*);
	unsigned long	(*checksum)(Pax_t*, Paxarchive_t*, Paxfile_t*, void*, size_t, unsigned long);
	int		(*validate)(Pax_t*, Paxarchive_t*, Paxfile_t*);
	int		(*event)(Pax_t*, Paxarchive_t*, Paxfile_t*, void*, unsigned long);
	unsigned long	events;
	char*		details;	/* instance details		*/
};

struct Paxarchive_s			/* archive info			*/
{
	char*		name;		/* archive name			*/
	void*		data;		/* format specific data		*/
	Paxformat_t*	format;		/* format			*/
	int32_t		flags;		/* format flags			*/
	int		incomplete;	/* file requires new volume	*/
	int		volume;		/* volume number		*/
	size_t		entries;	/* total number of entries	*/
	size_t		entry;		/* current entry index		*/
	Paxio_t*	io;		/* current io			*/
	struct				/* paxstash() values		*/
	{
	Paxvalue_t	comment;	/* header comment		*/
	Paxvalue_t	head;		/* header path name		*/
	Paxvalue_t	link;		/* link text			*/
	Paxvalue_t	zip;		/* zip header name		*/
	}		stash;

#ifdef _PAX_ARCHIVE_PRIVATE_
	_PAX_ARCHIVE_PRIVATE_
#endif

};

struct Pax_s				/* global state			*/
{
	const char*	id;		/* interface id			*/
	const char*	passphrase;	/* encryption passphrase	*/
	int32_t		flags;		/* flags			*/
	int		gid;		/* current group id		*/
	int		keepgoing;	/* keep going on error		*/
	int		list;		/* full file trace		*/
	int		modemask;	/* ~umask()			*/
	int		strict;		/* strict standard conformance	*/
	int		summary;	/* output summary info		*/
	int		test;		/* debug test bits		*/
	int		uid;		/* current user id		*/
	int		verbose;	/* trace files when acted upon	*/
	int		verify;		/* verify action on file	*/
	int		warn;		/* archive specific warnings	*/
	long		pid;		/* main pid			*/
	off_t		buffersize;	/* io buffer size		*/

	char		buf[SF_BUFSIZE];/* file io buffer		*/

	Error_f		errorf;

	int		(*dataf)(Pax_t*, Paxarchive_t*, Paxfile_t*, int, void*, off_t);
	void*		(*getf)(Pax_t*, Paxarchive_t*, off_t, off_t*);
	Sfio_t*		(*partf)(Pax_t*, Paxarchive_t*, off_t);
	int		(*putf)(Pax_t*, Paxarchive_t*, off_t);
	off_t		(*readf)(Pax_t*, Paxarchive_t*, void*, off_t, off_t, int);
	off_t		(*seekf)(Pax_t*, Paxarchive_t*, off_t, int, int);
	char*		(*stashf)(Pax_t*, Paxvalue_t*, const char*, size_t);
	int		(*syncf)(Pax_t*, Paxarchive_t*, int);
	int		(*unreadf)(Pax_t*, Paxarchive_t*, void*, off_t);
	int		(*writef)(Pax_t*, Paxarchive_t*, const void*, off_t);

	int		(*corruptf)(Pax_t*, Paxarchive_t*, Paxfile_t*, const char*);
	int		(*checksumf)(Pax_t*, Paxarchive_t*, Paxfile_t*, unsigned long, unsigned long);
	int		(*nospacef)(Pax_t*);

	int		(*nextf)(Pax_t*, Paxarchive_t*, size_t, size_t);

#ifdef _PAX_PRIVATE_
	_PAX_PRIVATE_
#endif

};

#define paxchecksum(p,a,f,x,v)	(*(p)->checksumf)(p,a,f,x,v)
#define paxcorrupt(p,a,f,m)	(*(p)->corruptf)(p,a,f,m)
#define paxdata(p,a,f,d,b,n)	(*(p)->dataf)(p,a,f,d,b,n)
#define paxget(p,a,o,r)		(*(p)->getf)(p,a,o,r)
#define paxnext(p,a,c,n)	(*(p)->nextf)(p,a,c,n)
#define paxnospace(p)		(*(p)->nospacef)(p)
#define paxpart(p,a,n)		(*(p)->partf)(p,a,n)
#define paxput(p,a,b,n)		(*(p)->putf)(p,a,b,n)
#define paxread(p,a,b,n,m,f)	(*(p)->readf)(p,a,b,n,m,f)
#define paxseek(p,a,o,w,f)	(*(p)->seekf)(p,a,o,w,f)
#define paxstash(p,v,s,z)	(*(p)->stashf)(p,v,s,z)
#define paxsync(p,a,f)		(*(p)->syncf)(p,a,f)
#define paxunread(p,a,b,n)	(*(p)->unreadf)(p,a,b,n)
#define paxwrite(p,a,b,n)	(*(p)->writef)(p,a,b,n)

#ifdef _PAX_ARCHIVE_PRIVATE_

#define PAXLIB(m)
#define PAXNEXT(m)	pax_##m##_next

#else

#ifdef __STDC__
#define PAXLIB(f)	extern Paxformat_t* pax_lib(Pax_t* pax) {return &pax_##f##_format;} \
			unsigned long plugin_version(void) {return PAX_PLUGIN_VERSION;}
#else
#define PAXLIB(f)	extern Paxformat_t* pax_lib(pax) Pax_t* pax; {return &pax_##f##_format;} \
			unsigned long plugin_version() {return PAX_PLUGIN_VERSION;}
#endif

#define PAXNEXT(m)	0

#if defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Paxformat_t*	pax_lib(Pax_t*);

#undef	extern

#endif

#endif
