/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2013 AT&T Intellectual Property          *
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
 * pax common definitions
 */

#ifndef _PAX_H
#define _PAX_H		1

#include <ast.h>
#include <ls.h>
#include <sig.h>
#include <ftwalk.h>
#include <ctype.h>
#include <ccode.h>
#include <hash.h>
#include <proc.h>
#include <regex.h>
#include <error.h>
#include <times.h>
#include <swap.h>
#include <align.h>
#include <debug.h>
#include <stdarg.h>
#include <sum.h>
#include <tv.h>
#include <fnv.h>

#define header	Tarheader_s
#include <tar.h>
#undef	header

typedef struct Tarheader_s Tarheader_t;

#include "FEATURE/local"

#define VENDOR		"ATT"
#define ID		"PAX"
#define VERSION		"1"

#define IMPLEMENTATION	VENDOR ID VERSION

#define PANIC		ERROR_PANIC|ERROR_SOURCE,__FILE__,__LINE__

#define bcount(ap)	((ap)->io->last-(ap)->io->next)
#define bsave(ap)	(state.backup=(*(ap)->io),error(-7, "bsave(%s,@%I*d)", ap->name, sizeof(ap->io->count), ap->io->count))
#define tvmtime(t,s)	(tvgetmtime(t,s),t)

#define BUFFER_FD_MAX	(-2)
#define BUFFER_FD_MIN	(-3)

#undef	getbuffer
#define getbuffer(n)	(((n)>=BUFFER_FD_MIN&&(n)<=BUFFER_FD_MAX)?&state.buffer[BUFFER_FD_MAX-(n)]:(Buffer_t*)0)
#undef	setbuffer
#define setbuffer(n)	(BUFFER_FD_MAX-(n))

#define HOLE_MIN	1024

#define holeinit(fd)	(state.hole=0)
#define holedone(fd)	do if(state.hole){lseek(fd,state.hole-1,SEEK_CUR);state.hole=0;write(fd,"",1);} while(0)

#define ropath(p)	(p[0]=='.'&&(p[1]==0||p[1]=='.'&&(p[2]==0||p[2]=='.'&&p[3]==0))||p[0]=='-'&&p[1]==0)

#define SECTION_CONTROL	1			/* control io		*/
#define SECTION_DATA	2			/* data io		*/

#define SECTION_MAX	3

#define SECTION(p)	(p)->section

#define METER_parts	20
#define METER_width	(METER_parts + 7)

#define NOW		time(NiL)

/*
 * format generic definitions
 */

#define HDR_atime	(1<<0)
#define HDR_charset	(1<<1)
#define HDR_ctime	(1<<2)
#define HDR_gid		(1<<3)
#define HDR_gname	(1<<4)
#define HDR_linkpath	(1<<5)
#define HDR_mtime	(1<<6)
#define HDR_path	(1<<7)
#define HDR_size	(1<<8)
#define HDR_uid		(1<<9)
#define HDR_uname	(1<<10)

#define FMT_COMPRESS	"gzip"		/* default compress format	*/
#define FMT_DEFAULT	"pax"		/* default output format	*/
#define FMT_DELTA	"delta"		/* default delta output format	*/
#define FMT_IGNORE	"ignore"	/* delta ignore output format	*/
#define FMT_PATCH	"patch"		/* delta patch output format	*/

#define IN		PAX_IN		/* copy in			*/
#define OUT		PAX_OUT		/* copy out			*/
#define ARCHIVE		PAX_ARCHIVE	/* archive format		*/
#define COMPRESS	PAX_COMPRESS	/* compress format		*/
#define DELTA		PAX_DELTA	/* delta format			*/
#define DELTAIO		PAX_DELTAIO	/* separate delta io		*/
#define DLL		PAX_DLL		/* format from dll		*/
#define DOS		PAX_DOS		/* may contain dos names	*/
#define PSEUDO		PAX_PSEUDO	/* pseudo delta format		*/
#define COMPRESSED	PAX_COMPRESSED	/* format already compressed	*/
#define CONV		PAX_CONV	/* format handles ccode conv	*/
#define DELTAINFO	PAX_DELTAINFO	/* format handles delta info	*/
#define KEEPSIZE	PAX_KEEPSIZE	/* retain non REG st_size	*/
#define LINKTYPE	PAX_LINKTYPE	/* File_t.linktype supported	*/
#define NOHARDLINKS	PAX_NOHARDLINKS	/* hard links not supported	*/
#define SLASHDIR	PAX_SLASHDIR	/* trailing slash => directory	*/
#define STANDARD	PAX_STANDARD	/* the standard format		*/
#define SUM		PAX_SUM		/* inline member checksum	*/
#define APPEND		PAX_APPEND	/* archive append		*/

#define SETIDS		(1<<0)		/* set explicit uid and gid	*/

#define INFO_MATCH	"*([A-Z0-9!])!!!"/* info header file name match	*/
#define INFO_SEP	'!'		/* info header field separator	*/
#define INFO_ORDERED	'O'		/* delta on ordered base	*/

#define IDLEN		(sizeof(ID)-1)	/* strlen(ID)			*/

#define DELTA_94	1
#define DELTA_88	2
#define DELTA_IGNORE	3
#define DELTA_PATCH	4

#define TYPE_COMPRESS	'C'		/* compress encoding type	*/
#define TYPE_DELTA	'D'		/* delta encoding type		*/

#define DELTA_SRC	0		/* source info			*/
#define DELTA_TAR	1		/* target info			*/
#define DELTA_DEL	2		/* delta info			*/
#define DELTA_DATA	3		/* data info mask		*/
#define DELTA_BIO	(1<<2)		/* bio (no arg)			*/
#define DELTA_BUFFER	(1<<3)		/* buffer arg			*/
#define DELTA_FD	(1<<4)		/* fd arg			*/
#define DELTA_HOLE	(1<<5)		/* holewrite() fd arg		*/
#define DELTA_OFFSET	(1<<6)		/* offset arg			*/
#define DELTA_SIZE	(1<<7)		/* size arg			*/
#define DELTA_OUTPUT	(1<<8)		/* output data			*/
#define DELTA_TEMP	(1<<9)		/* temp output fd		*/
#define DELTA_FREE	(1<<10)		/* free fd or buffer		*/
#define DELTA_COUNT	(1<<11)		/* set in|out count		*/
#define DELTA_LIST	(1<<12)		/* listentry() if ok		*/

#define DELTA_create	'c'		/* delta create data op		*/
#define DELTA_delete	'd'		/* delta delete data op		*/
#define DELTA_nop	'x'		/* no delta operation pseudo op	*/
#define DELTA_pass	'p'		/* delta pass pseudo op		*/
#define DELTA_update	'u'		/* delta update data op		*/
#define DELTA_verify	'v'		/* delta verify data op		*/

#define DELTA_TRAILER	10		/* delta trailer output size	*/

#define DELTA_checksum	'c'		/* delta member checksum	*/
#define DELTA_index	'i'		/* delta header index		*/
#define DELTA_trailer	't'		/* delta trailer size		*/

#define DELTA_WINDOW	(128*1024L)	/* delta window buffer size	*/

#define DELTA_LO(c)	((c)&0xffff)	/* lo order checksum bits	*/
#define DELTA_HI(c)	DELTA_LO(c>>16)	/* hi order checksum bits	*/

#define HEADER_EXTENDED		"@PaxHeaders/%(sequence)s"
#define HEADER_EXTENDED_STD	"%(dir)s/PaxHeaders/%(file)s"
#define HEADER_GLOBAL		"@PaxGlobals/%(sequence)s"
#define HEADER_GLOBAL_STD	"%(tmp)s/GlobalHead/%(entry)s"

#define INVALID_binary		0	/* binary for unencodable data	*/
#define INVALID_ignore		1	/* silently ignore		*/
#define INVALID_prompt		2	/* prompt for new name		*/
#define INVALID_translate	3	/* translate and/or truncate	*/
#define INVALID_UTF8		4	/* convert to UTF8		*/

#define NOLINK		PAX_NOLINK	/* not a link			*/
#define HARDLINK	PAX_HARDLINK	/* hard link to previous entry	*/
#define SOFTLINK	PAX_SOFTLINK	/* soft link to previous entry	*/

#define BLOCKSIZE	PAX_BLOCK	/* block size			*/
#define IOALIGN		ALIGN_BOUND1	/* io buffer alignment		*/
#define MINBLOCK	1		/* smallest block size		*/
#define DEFBLOCKS	PAX_DEFBLOCKS	/* default blocking		*/
#define DEFBUFFER	PAX_DEFBUFFER	/* default io buffer blocking	*/
#define FILBLOCKS	1024		/* file to file blocking	*/
#define MAXUNREAD	(8*BLOCKSIZE)	/* max bunread() count		*/
#define MINID		80		/* min ident buffer size	*/
#define MAXID		BLOCKSIZE	/* max ident buffer size	*/
#define RESETABLE	(-1)		/* default option can be reset	*/

#define MAP_INDEX	0x01		/* append .%d index on match	*/

typedef int (*Link_f)(const char*, const char*);
typedef int (*Stat_f)(const char*, struct stat*);

struct Archive_s; typedef struct Archive_s Archive_t;
struct File_s; typedef struct File_s File_t;
struct Filter_s; typedef struct Filter_s Filter_t;
struct Format_s; typedef struct Format_s Format_t;
struct List_s; typedef struct List_s List_t;
struct Map_s; typedef struct Map_s Map_t;
struct Member_s; typedef struct Member_s Member_t;

#define _PAX_IO_PRIVATE_ \
	char*		next;		/* next char pointer		*/ \
	char*		last;		/* last char+1 pointer		*/ \
	char*		buffer;		/* io buffer			*/ \
	off_t		count;		/* char transfer count		*/ \
	off_t		expand;		/* add to count at end		*/ \
	off_t		offset;		/* volume offset		*/ \
	off_t		size;		/* total size if seekable	*/ \
	unsigned int	fill;		/* bsave fill serial		*/ \
	int		skip;		/* volume skip			*/ \
	int		keep;		/* volume keep after skip	*/ \
	int		mode;		/* open() O_* mode		*/ \
	int		unread;		/* max unread size		*/ \
	unsigned int	all:1;		/* read all volumes		*/ \
	unsigned int	blocked:1;	/* blocked device io		*/ \
	unsigned int	blok:1;		/* BLOK io file			*/ \
	unsigned int	blokeof:1;	/* BLOK io eof			*/ \
	unsigned int	blokflag:1;	/* io file BLOK flag		*/ \
	unsigned int	empty:1;	/* last read was empty		*/ \
	unsigned int	seekable:1;	/* seekable			*/ \
	unsigned int	unblocked:1;	/* set unblocked device io	*/

typedef struct Buffer_s			/* pseudo fd buffer		*/
{
	char*		base;		/* buffer base			*/
	char*		next;		/* current position		*/
	char*		past;		/* overflow position		*/
} Buffer_t;

typedef struct Fileid_s			/* unique file identifier	*/
{
	int		dev;		/* device			*/
	int		ino;		/* inode			*/
} Fileid_t;

typedef struct Link_s			/* link info			*/
{
	char*		name;		/* name				*/
	char*		checksum;	/* hard link checksum		*/
	int		namesize;	/* name size with null byte	*/
	Fileid_t	id;		/* generated link file id	*/
} Link_t;

#define _PAX_FILE_PRIVATE_ \
	Archive_t*	ap;		/* !=0 if from buffer		*/ \
	unsigned long	checksum;	/* data checksum		*/ \
	off_t		datasize;	/* non-reg has data anyway	*/ \
	struct \
	{ \
	int		op;		/* op				*/ \
	int		same;		/* SRC and TAR same		*/ \
	unsigned long	checksum;	/* data checksum		*/ \
	unsigned long	index;		/* index			*/ \
	Member_t*	base;		/* base file pointer		*/ \
	}		delta;		/* delta info			*/ \
	int		extended;	/* extended header file		*/ \
	int		fd;		/* >=0 read fd			*/ \
	char*		id;		/* archive file id		*/ \
	char*		intermediate;	/* intermediate output name	*/ \
	int		linkpathsize;	/* linkname size with null byte	*/ \
	int		magic;		/* header magic number		*/ \
	int		namesize;	/* name size with null byte	*/ \
	int		perm;		/* original st_mode perm	*/ \
	int		type;		/* st_mode type			*/ \
	Link_t*		link;		/* hard link state		*/ \
	char*		uidname;	/* user id name			*/ \
	char*		gidname;	/* group id name		*/ \
	struct \
	{ \
	char*		partial;	/* partial record buffer	*/ \
	int		blocks;		/* io block count		*/ \
	int		format;		/* format			*/ \
	int		section;	/* file section number		*/ \
	}		record;		/* record format info		*/ \
	unsigned int	longlink:1;	/* f->linkpath too long		*/ \
	unsigned int	longname:1;	/* f->name too long		*/ \
	unsigned int	ordered:1;	/* ordered fileout() override	*/ \
	unsigned int	restoremode:1;	/* must restore mode		*/ \
	unsigned int	ro:1;		/* readonly { . .. ... - }	*/ \
	unsigned int	skip:1;		/* skip this entry		*/

struct Member_s				/* cached member info		*/
{
	File_t*		info;		/* deltapass() file info	*/
	Tv_t		atime;		/* access time			*/
	Tv_t		mtime;		/* modify time			*/
	off_t		offset;		/* data offset			*/
	off_t		size;		/* data size			*/
	off_t		uncompressed;	/* uncompressed size		*/
	short		dev;		/* dev				*/
	short		ino;		/* ino				*/
	short		mode;		/* mode				*/
	unsigned int	mark:1;		/* visit mark			*/
};

typedef uint32_t Magic_t;

typedef struct Compress_format_s	/* compress format data		*/
{
	char*		variant;	/* variant qualifier/flag	*/
	char*		undo[2];	/* compress undo name and arg	*/
	char*		undotoo[2];	/* alternate undo		*/
} Compress_format_t;

typedef struct Delta_format_s		/* delta format data		*/
{
	char*		variant;	/* variant qualifier/flag	*/
} Delta_format_t;

struct List_s				/* generic list			*/
{
	List_t*		next;		/* next in list			*/
	void*		item;		/* list item pointer		*/
};

struct Map_s				/* file name map list		*/
{
	Map_t*		next;		/* next in list			*/
	regex_t		re;		/* compiled match re		*/
	int		flags;		/* MAP_* flags			*/
};

typedef struct Post_s			/* post processing restoration	*/
{
	Tv_t		atime;		/* access time			*/
	Tv_t		mtime;		/* modify time			*/
	int		mode;		/* permissions			*/
	int		uid;		/* user id			*/
	int		gid;		/* group id			*/
	unsigned int	restoremode:1;	/* must restore mode		*/
} Post_t;

typedef union Integral_u		/* byte|half swap probe		*/
{
	uint32_t	l;
	uint16_t	s[2];
	uint8_t		c[4];
} Integral_t;

typedef struct Convert_s		/* char code conversion		*/
{
	int		on;		/* do the conversion		*/
	unsigned char*	f2t;		/* from=>to map			*/
	unsigned char*	t2f;		/* to=>from map			*/
	unsigned char*	f2a;		/* from=>CC_ASCII map		*/
	unsigned char*	t2a;		/* to=>CC_ASCII map		*/
} Convert_t;

typedef struct Delta_s			/* delta archive info		*/
{
	Archive_t*	base;		/* base archive			*/
	unsigned long	checksum;	/* expected base checksum	*/
	int		compress;	/* delta source is /dev/null	*/
	int		epilogue;	/* epilogue hit			*/
	int		index;		/* member index			*/
	int		initialized;	/* delta base initialized	*/
	Format_t*	format;		/* delta format			*/
	char*		hdr;		/* header pointer		*/
	char		hdrbuf[64];	/* header buffer		*/
	int		ordered;	/* members ordered by file name	*/
	off_t		size;		/* expected base size		*/
	Hash_table_t*	tab;		/* entry table			*/
	int		trailer;	/* optional trailer size	*/
	char*		version;	/* optional delta version	*/
} Delta_t;

typedef struct Part_s
{
	Sfdisc_t	disc;
	off_t		n;
	struct State_s*	pax;
	Archive_t*	ap;
	Sfio_t*		sp;
} Part_t;

struct Filter_s
{
	Filter_t*	next;		/* next in list			*/
	regex_t*	re;		/* path pattern or default if 0	*/
	char*		command;	/* original command line	*/
	char**		argv;		/* command argv			*/
	char**		patharg;	/* file arg in argv		*/
};

typedef struct Pattern_s
{
	char*		pattern;
	unsigned char	directory;
	unsigned char	matched;
} Pattern_t;

#define _PAX_ARCHIVE_PRIVATE_ \
	unsigned long	checksum;	/* running checksum		*/ \
	int		checkdelta;	/* getprologue delta check	*/ \
	Format_t*	compress;	/* compression format		*/ \
	Convert_t	convert[SECTION_MAX];/* data/header conversion	*/ \
	Delta_t*	delta;		/* delta info			*/ \
	int		errors;		/* intermediate error count	*/ \
	Format_t*	expected;	/* expected format		*/ \
	File_t		file;		/* current member file info	*/ \
	File_t*		info;		/* last deltabase() member	*/ \
	int		locked;		/* newio() recursion lock	*/ \
	Bio_t		mio;		/* main buffered io		*/ \
	unsigned long	memsum;		/* member checksum		*/ \
	off_t		offset;		/* relative byte offset		*/ \
	int		ordered;	/* archive members ordered	*/ \
	struct \
	{ \
	unsigned long	checksum;	/* old running checksum		*/ \
	unsigned long	memsum;		/* old checksum			*/ \
	int		warned;		/* old checksum warning		*/ \
	}		old; \
	char*		package;	/* package id			*/ \
	Archive_t*	parent;		/* parent (delta) for base	*/ \
	int		part;		/* media change count		*/ \
	Part_t*		partio;		/* archive part sfio info	*/ \
	struct				/* stash() values		*/ \
	{ \
	Paxvalue_t	copy;		/* copy() path buffer		*/ \
	Paxvalue_t	name;		/* real path name		*/ \
	Paxvalue_t	path;		/* access path			*/ \
	Paxvalue_t	peek;		/* peek file			*/ \
	Paxvalue_t	prev;		/* previous entry order check	*/ \
	char		temp[PATH_MAX];	/* temp intermediate name	*/ \
	}		path; \
	int		peek;		/* already peeked at file entry */ \
	File_t*		record;		/* record output file		*/ \
	int		raw;		/* don't convert sections	*/ \
	int		section;	/* current archive section	*/ \
	size_t		selected;	/* number of selected members	*/ \
	off_t		size;		/* size				*/ \
	off_t		skip;		/* base archive skip offset	*/ \
	struct stat	st;		/* memver stat			*/ \
	int		sum;		/* collect running checksum	*/ \
	int		swap;		/* swap operation		*/ \
	int		swapio;		/* io swap operation		*/ \
	Hash_table_t*	tab;		/* verification table		*/ \
	Bio_t		tio;		/* temporary buffered io	*/ \
	struct \
	{ \
	Sfio_t*		extended;	/* extend() temporary		*/ \
	Sfio_t*		global;		/* extend() temporary		*/ \
	Sfio_t*		hdr;		/* headname() temporary		*/ \
	Sfio_t*		key;		/* extend() temporary		*/ \
	}		tmp;		/* temporary stuff		*/ \
	off_t		total;		/* newio() total io check	*/ \
	char*		type;		/* archive type			*/ \
	off_t		uncompressed;	/* uncompressed size estimate	*/ \
	Hash_table_t*	update;		/* update info			*/ \
	size_t		updated;	/* number of updated members	*/ \
	int		warnlinkhead;	/* invalid hard link header	*/

#define _PAX_PRIVATE_ \
	int		acctime;	/* preserve member access times	*/ \
	int		append;		/* append -- must be 0 or 1 !!!	*/ \
	Bio_t		backup;		/* backup() position		*/ \
	long		blocksize;	/* explicit buffer size		*/ \
	Buffer_t	buffer[BUFFER_FD_MAX-BUFFER_FD_MIN+1];/* fd buf	*/ \
	struct \
	{ \
	char*		name;		/* archive member name		*/ \
	char*		path;		/* local file path		*/ \
	Sfio_t*		sp;		/* tmp stream			*/ \
	Sum_t*		sum;		/* method handle		*/ \
	}		checksum;	/* --checksum state		*/ \
	int		clobber;	/* overwrite output files	*/ \
	int		complete;	/* files completely in volume	*/ \
	int		current;	/* current file[] index		*/ \
	struct \
	{ \
	int		bufferindex;	/* delta buffer index		*/ \
	int		buffersize;	/* delta buffer size		*/ \
	int		update;		/* update only delta members	*/ \
	}		delta;		/* delta info			*/ \
	int		delta2delta;	/* -rz- -wz- : retain delta info*/ \
	int		descend;	/* dir names self+descendents	*/ \
	char*		destination;	/* pass mode destination dir	*/ \
	dev_t		dev;		/* . device number		*/ \
	unsigned short	devcnt;		/* dev assignment count		*/ \
	int		drop;		/* drop a `.' for each file	*/ \
	int		dropcount;	/* current line drop count	*/ \
	int		exact;		/* exact archive read		*/ \
	char**		files;		/* alternate file name list	*/ \
	struct \
	{ \
	Filter_t*	list;		/* filter list			*/ \
	Filter_t*	last;		/* filter list tail		*/ \
	Filter_t*	all;		/* match all filter		*/ \
	char*		options;	/* line mode options		*/ \
	char*		command;	/* line mode command		*/ \
	char*		path;		/* line mode physical path	*/ \
	char*		name;		/* line mode logical path	*/ \
	int		line;		/* line mode			*/ \
	}		filter;		/* file output filter state	*/ \
	int		forceconvert;	/* force binary conversion	*/ \
	Format_t*	format;		/* default output format	*/ \
	int		ftwflags;	/* ftwalk() flags		*/ \
	struct \
	{ \
	char*		comment;	/* comment text			*/ \
	int		invalid;	/* invalid path INVALID_ action	*/ \
	int		linkdata;	/* data with each hard link	*/ \
	char*		listformat;	/* verbose listing format	*/ \
	char*		global;		/* global header name format	*/ \
	char*		extended;	/* extended name format		*/ \
	}		header;		/* header specific options	*/ \
	off_t		hole;		/* one past last hole		*/ \
	Archive_t*	in;		/* input archive info		*/ \
	unsigned short	inocnt;		/* ino assignment count		*/ \
	struct \
	{ \
	char*		name;		/* archive member name		*/ \
	char*		path;		/* local file path		*/ \
	Sfio_t*		sp;		/* tmp stream			*/ \
	}		install;	/* --install state		*/ \
	int		intermediate;	/* intermediate ouput - rename	*/ \
	int		interrupt;	/* this signal caused exit	*/ \
	Link_f		linkf;		/* -L=pathsetlink() -P=link()	*/ \
	Hash_table_t*	linktab;	/* hard link table		*/ \
	char*		listformat;	/* verbose listing format	*/ \
	int		local;		/* reject files/links outside .	*/ \
	struct \
	{ \
	unsigned char*	a2n;		/* CC_ASCII=>CC_NATIVE		*/ \
	unsigned char*	e2n;		/* CC_EBCDIC=>CC_NATIVE		*/ \
	unsigned char*	n2e;		/* CC_NATIVE=>CC_EBCDIC		*/ \
	}		map;		/* ccode maps			*/ \
	Map_t*		maps;		/* file name maps		*/ \
	int		matchsense;	/* pattern match sense		*/ \
	off_t		maxout;		/* max volume/part output count	*/ \
	struct \
	{ \
	Sfio_t*		tmp;		/* tmp buffer			*/ \
	off_t		size;		/* total size			*/ \
	int		fancy;		/* fancy display		*/ \
	int		last;		/* length of last display path	*/ \
	int		on;		/* meter progress		*/ \
	int		width;		/* display line width		*/ \
	}		meter;		/* meter state			*/ \
	int		mkdir;		/* make intermediate dirs	*/ \
	struct \
	{ \
	char*		magic;		/* separator magic		*/ \
	size_t		length;		/* separator magic length	*/ \
	size_t		fill;		/* last member filler size	*/ \
	}		mime; \
	char*		mode;		/* output chmod(1) mode		*/ \
	int		modekeep;	/* must preserve mode		*/ \
	int		modtime;	/* retain mtime			*/ \
	char*		mtime;		/* output date(1) mtime		*/ \
	int		newer;		/* append only if newer		*/ \
	int		operation;	/* IN|OUT operation mode	*/ \
	Hash_table_t*	options;	/* option table			*/ \
	int		ordered;	/* sorted base and file list	*/ \
	int		owner;		/* set owner info		*/ \
	Archive_t*	out;		/* output archive info		*/ \
	int		pass;		/* archive to archive		*/ \
	Pattern_t*	pattern;	/* current name match pattern	*/ \
	Pattern_t*	patterns;	/* name match patterns		*/ \
	char*		peekfile;	/* stdin file list peek		*/ \
	int		peeklen;	/* peekfile length		*/ \
	char		pwd[PATH_MAX];	/* full path of .		*/ \
	int		pwdlen;		/* pwd length sans null		*/ \
	List_t*		proc;		/* procopen() list for finish	*/ \
	struct \
	{ \
	int		charset;	/* convert record charset	*/ \
	int		delimiter;	/* directory delimiter		*/ \
	File_t*		file;		/* current output file		*/ \
	int		format;		/* record format		*/ \
	char*		header;		/* file header			*/ \
	int		headerlen;	/* file header length		*/ \
	int		line;		/* convert records<->lines	*/ \
	int		pad;		/* pad output record blocks	*/ \
	char*		pattern;	/* format match pattern		*/ \
	int		offset;		/* data buffer offset		*/ \
	long		size;		/* io size			*/ \
	char*		trailer;	/* file trailer			*/ \
	int		trailerlen;	/* file trailer length		*/ \
	}		record;		/* record info			*/ \
	int		resetacctime;	/* reset input file access times*/ \
	Hash_table_t*	restore;	/* post proc restoration table	*/ \
	Sfio_t*		rtty;		/* tty file read pointer	*/ \
	int		scanned;	/* scanned for format libs	*/ \
	size_t		selected;	/* total of selected members	*/ \
	int		setgid;		/* set file gid to this value	*/ \
	int		setuid;		/* set file uid to this value	*/ \
	Stat_f		statf;		/* -L=pathstat() -P=lstat()	*/ \
	int		sync;		/* fsync() each file after copy	*/ \
	unsigned long	testdate;	/* listformat test date		*/ \
	struct \
	{ \
	char*		buffer;		/* temporary buffer		*/ \
	long		buffersize;	/* temporary buffer size	*/ \
	char*		delta;		/* tmp delta file name		*/ \
	char*		file;		/* tmp file name		*/ \
	Sfio_t*		fmt;		/* temporary format stream	*/ \
	Sfio_t*		lst;		/* temporary list stream	*/ \
	Sfio_t*		mac;		/* temporary print macro stream	*/ \
	Sfio_t*		str;		/* temporary string stream	*/ \
	}		tmp;		/* temporary stuff		*/ \
	int		update;		/* copy file only if newer	*/ \
	size_t		updated;	/* total of updated members	*/ \
	char*		usage;		/* optget() usage string	*/ \
	char		volume[64];	/* volume id			*/ \
	int		warnmkdir;	/* --mkdir hint issued		*/ \
	int		warnlinknum;	/* too many hard links		*/ \
	Sfio_t*		wtty;		/* tty file write pointer	*/ \
	int		yesno;		/* interactive answer is yes/no	*/

#define Pax_t		State_t
#define Pax_s		State_s
#define Paxarchive_t	Archive_t
#define Paxarchive_s	Archive_s
#define Paxfile_t	File_t
#define Paxfile_s	File_s
#define Paxformat_t	Format_t
#define Paxformat_s	Format_s
#define Paxio_t		Bio_t
#define Paxio_s		Bio_s
#define Paxvalue_t	Value_t
#define Paxvalue_s	Value_s

#include "paxlib.h"
#include "options.h"

extern char*		definput;
extern char*		defoutput;
extern char*		eomprompt;
extern Format_t*	formats;
extern State_t		state;

extern int		addlink(Archive_t*, File_t*);
extern int		apply(Archive_t*, File_t*, Filter_t*);
extern long		asc_checksum(char*, int, unsigned long);
extern void		backup(Archive_t*);
extern long		bblock(int);
extern void		bflushin(Archive_t*, int);
extern void		bflushout(Archive_t*);
extern char*		bget(Archive_t*, off_t, off_t*);
extern void		binit(Archive_t*);
extern void		bput(Archive_t*, off_t);
extern off_t		bread(Archive_t*, void*, off_t, off_t, int);
extern off_t		bseek(Archive_t*, off_t, int, int);
extern int		bskip(Archive_t*);
extern void		bunread(Archive_t*, void*, int);
extern void		bwrite(Archive_t*, void*, off_t);
extern int		closein(Archive_t*, File_t*, int);
extern int		closeout(Archive_t*, File_t*, int);
extern int		cmpftw(Ftw_t*, Ftw_t*);
extern void		complete(Archive_t*, File_t*, size_t);
extern void		convert(Archive_t*, int, int, int);
extern void		copy(Archive_t*, int(*)(Ftw_t*));
extern void		copyin(Archive_t*);
extern int		copyinout(Ftw_t*);
extern int		copyout(Ftw_t*);
extern void		deltabase(Archive_t*);
extern int		deltacheck(Archive_t*, File_t*);
extern void		deltadelete(Archive_t*);
extern void		deltaout(Archive_t*, Archive_t*, File_t*);
extern void		deltapass(Archive_t*, Archive_t*);
extern void		deltaset(Archive_t*, char*);
extern void		deltaverify(Archive_t*);
extern int		dirprefix(char*, char*, int);
extern void		filein(Archive_t*, File_t*);
extern void		fileout(Archive_t*, File_t*);
extern void		fileskip(Archive_t*, File_t*);
extern Filter_t*	filter(Archive_t*, File_t*);
extern void		finish(int);
extern Archive_t*	getarchive(int);
extern void		getdeltaheader(Archive_t*, File_t*);
extern void		getdeltatrailer(Archive_t*, File_t*);
extern int		getepilogue(Archive_t*);
extern int		getfile(Archive_t*, File_t*, Ftw_t*);
extern Format_t*	getformat(char*, int);
extern int		getheader(Archive_t*, File_t*);
extern void		getidnames(File_t*);
extern int		getprologue(Archive_t*);
extern void		gettrailer(Archive_t*, File_t*);
extern ssize_t		holewrite(int, void*, size_t);
extern Archive_t*	initarchive(const char*, int);
extern void		initdelta(Archive_t*, Format_t*);
extern void		initfile(Archive_t*, File_t*, struct stat*, char*, int);
extern void		initmatch(char**);
extern void		interactive(void);
extern void		listentry(File_t*);
extern int		listprintf(Sfio_t*, Archive_t*, File_t*, const char*);
extern char*		map(Archive_t*, char*);
extern int		match(char*);
extern void		newio(Archive_t*, int, int);
extern Format_t*	nextformat(Format_t*);
extern void		nospace(void);
extern unsigned long	omemsum(const void*, int, unsigned long);
extern int		openin(Archive_t*, File_t*);
extern int		openout(Archive_t*, File_t*);
extern int		paxdelta(Archive_t*, Archive_t*, File_t*, int, ...);
extern void		paxinit(Pax_t*, const char*);
extern int		portable(Archive_t*, const char*);
extern int		prune(Archive_t*, File_t*, struct stat*);
extern void		putdeltaheader(Archive_t*, File_t*);
extern void		putdeltatrailer(Archive_t*, File_t*);
extern void		putepilogue(Archive_t*);
extern int		putheader(Archive_t*, File_t*);
extern void		putinfo(Archive_t*, char*, unsigned long, unsigned long);
extern void		putkey(Archive_t*, Sfio_t*, Option_t*, const char*, Sfulong_t);
extern void		putprologue(Archive_t*, int);
extern void		puttrailer(Archive_t*, File_t*);
extern char*		release(void);
extern int		restore(const char*, char*, void*);
extern void		seekable(Archive_t*);
extern int		selectfile(Archive_t*, File_t*);
extern void		setdeltaheader(Archive_t*, File_t*);
extern void		setfile(Archive_t*, File_t*);
extern void		setidnames(File_t*);
extern void		setinfo(Archive_t*, File_t*);
extern void		setoptions(char*, size_t, char**, char*, Archive_t*, int);
extern void		settime(const char*, Tv_t*, Tv_t*, Tv_t*);
extern char*		stash(Value_t*, const char*, size_t);
extern char*		strlower(char*);
extern char*		strupper(char*);
extern void		undoable(Archive_t*, Format_t*);
extern void		undos(File_t*);
extern int		validout(Archive_t*, File_t*);
extern int		verify(Archive_t*, File_t*, char*);

#endif
