/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2013 AT&T Intellectual Property          *
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
 * ls -- list file status
 */

#define TIME_ISO	"%Q/%m-%d+%H:%M/%Y-%m-%d /"
#define TIME_LONG_ISO	"%_K"
#define TIME_FULL_ISO	"%_EK"
#define TIME_LOCALE	"%c"

static const char usage[] =
"[-?\n@(#)$Id: ls (AT&T Research) 2013-12-06 $\n]"
USAGE_LICENSE
"[+NAME?ls - list files and/or directories]"
"[+DESCRIPTION?For each directory argument \bls\b lists the contents; for each"
"	file argument the name and requested information are listed."
"	The directory \b.\b is assumed if no file arguments appear."
"	The listing is sorted by file name by default, except that file"
"	arguments are listed before directories.]"
"[+?Multi-column terminal output display width is determined by \bioctl\b(2)"
"	and/or the \bCOLUMNS\b environment variable.]"
"[+?\bgetconf PATH_RESOLVE\b determines how symbolic links are handled. This"
"	can be explicitly overridden by the \b--logical\b, \b--metaphysical\b,"
"	and \b--physical\b options below. \bPATH_RESOLVE\b can be one of:]{"
"		[+logical?Follow all symbolic links.]"
"		[+metaphysical?Follow command argument symbolic links,"
"			otherwise don't follow.]"
"		[+physical?Don't follow symbolic links.]"
"}"

"[a:all?List entries starting with \b.\b; turns off \b--almost-all\b.]"
"[A:almost-all?List all entries but \b.\b and \b..\b; turns off \b--all\b.]"
"[b:escape?Print escapes for nongraphic characters.]"
"[B:ignore-backups?Do not list entries ending with ~.]"
"[c:ctime?Sort by change time; list ctime with \b--long\b.]"
"[C:multi-column?List entries by columns.]"
"[d:directory?List directory entries instead of contents.]"
"[D:define?Define \akey\a with optional \avalue\a. \avalue\a will be expanded"
"	when \b%(\b\akey\a\b)\b is specified in \b--format\b. \akey\a may"
"	override internal \b--format\b identifiers.]:[key[=value]]]"
"[e:long-iso|long-time?Equivalent to \b--long --time-style=long-iso\b.]"
"[E:full-iso|full-time?Equivalent to \b--long --time-style=full-iso\b.]"
"[f:force?Force each argument to be interpreted as a directory and list"
"	the name found in each slot in the physical directory order. Turns"
"	on \b-aU\b and turns off \b-lrst\b. The results are undefined for"
"	non-directory arguments.]"
"[Z:format?Append to the listing format string. \aformat\a follows"
"	\bprintf\b(3) conventions, except that \bsfio\b(3) inline ids"
"	are used instead of arguments:"
"	%[-+]][\awidth\a[.\aprecis\a[.\abase\a]]]]]](\aid\a[:\asubformat\a]])\achar\a."
"	If \achar\a is \bs\b then the string form of the item is listed,"
"	otherwise the corresponding numeric form is listed. \asubformat\a"
"	overrides the default formatting for \aid\a. Supported \aid\as"
"	and \asubformat\as are:]:[format]{"
"		[+atime?access time]"
"		[+blocks?size in blocks]"
"		[+ctime?change time]"
"		[+dev?major/minor device numbers]"
"		[+device?major/minor device numbers if block or character special device]"
"		[+devmajor?major device number]"
"		[+devminor?minor device number]"
"		[+dir.blocks?directory blocks]"
"		[+dir.bytes?directory size in bytes]"
"		[+dir.count?directory entry count]"
"		[+dir.files?directory file count]"
"		[+flags?command line flags in effect]"
"		[+gid?group id]"
"		[+header?listing header]"
"		[+ino?serial number]"
"		[+linkop?link operation: -> for symbolic, empty otherwise]"
"		[+linkname?symbolic link text]"
"		[+linkpath?symbolic link text]"
"		[+mark?file or directory mark character]"
"		[+markdir?directory mark character]"
"		[+mode?access mode]"
"		[+mtime?modification time]"
"		[+name?entry name]"
"		[+nlink?hard link count]"
"		[+path?file path from original root dir]"
"		[+perm?access permissions]"
"		[+size?file size in bytes]"
"		[+summary?listing summary info]"
"		[+total.blocks?running total block count]"
"		[+total.bytes?running total size in bytes]"
"		[+total.files?running total file count]"
"		[+trailer?listing trailer]"
"		[+uid?owner id]"
"		[+view?3d fs view level, 0 for the top or 2d]"
"		[+----?subformats ----]"
"		[+case\b::\bp\b\a1\a::\bs\b\a1\a::...::\bp\b\an\a::\bs\b\an\a?Expands"
"			to \bs\b\ai\a if the value of \aid\a matches the shell"
"			pattern \bp\b\ai\a, or the empty string if there is no"
"			match.]"
"		[+mode?The integral value as a \bfmtmode\b(3) string.]"
"		[+perm?The integral value as a \bfmtperm\b(3) string.]"
"		[+time[=\aformat\a]]?The integral value as a \bstrftime\b(3)"
"			string. For example,"
"			\b--format=\"%8(mtime)u %(ctime:time=%H:%M:%S)s\"\b"
"			lists the mtime in seconds since the epoch and the"
"			ctime as hours:minutes:seconds.]"
"	}"
"[F:classify?Append a character for typing each entry. Turns on \b--physical\b.]"
"[g:group?\b--long\b with no owner info.]"
"[G?\b--long\b with no group info.]"
"[h:scale|binary-scale|human-readable?Scale sizes to powers of 1024 { Ki Mi Gi Ti Pi Xi }.]"
"[i:inode?List the file serial number.]"
"[I:ignore?Do not list implied entries matching shell \apattern\a.]:[pattern]"
"[k:kilobytes?Use 1024 blocks instead of 512.]"
"[K:shell-quote?Enclose entry names in shell $'...' if necessary.]"
"[l:long|verbose?Use a long listing format.]"
"[m:commas|comma-list?List names as comma separated list.]"
"[n:numeric-uid-gid?List numeric user and group ids instead of names.]"
"[N:literal|show-controls-chars?Print raw entry names (don't treat e.g. control characters specially).]"
"[o:owner?\b--long\b with no group info.]"
"[O?\b--long\b with no owner info.]"
"[p:markdir?Append / to each directory name.]"
"[q:hide-control-chars?Print ? instead of non graphic characters.]"
"[Q:quote-name?Enclose all entry names in \"...\".]"
"[J:quote-style|quoting-style?Quote entry names according to \astyle\a:]:[style:=question]{"
"	[c:C?C \"...\" quote.]"
"	[e:escape?\b\\\b escape if necessary.]"
"	[l:literal?No quoting.]"
"	[q:question?Replace unprintable characters with \b?\b.]"
"	[s:shell?Shell $'...' quote if necessary.]"
"	[S:shell-always?Shell $'...' every name.]"
"}"
"[r:reverse?Reverse order while sorting.]"
"[R:recursive?List subdirectories recursively.]"
"[s:size?Print size of each file, in blocks.]"
"[S:bysize?Sort by file size.]"
"[t:?Sort by modification time; list mtime with \b--long\b.]"
"[T:tabsize?Ignored by this implementation.]#[columns]"
"[u:access?Sort by last access time; list atime with \b--long\b.]"
"[U?Equivalent to \b--sort=none\b.]"
"[V:colors|colours?\akey\a determines when color is used to distinguish"
"	types:]:?[key:=never]{"
"		[n:never?Never use color.]"
"		[a:always?Always use color.]"
"		[t:tty|auto?Use color when output is a tty.]"
"}"
"[w:width?Set the screen width to \ascreen-width\a and the screen height"
"	to \ascreen-height\a if specified.]:[[screen-heightX]]screen-width]"
"[W:time?Display \akey\a time instead of the modification time:]:[key]{"
"	[a:atime|access|use?access time]"
"	[c:ctime|status?status change time]"
"	[m:mtime|time?modify time]"
"}"
"[x:across?List entries by lines instead of by columns.]"
"[X:extension?Sort alphabetically by entry extension.]"
"[y:sort?Sort by \akey\a:]:?[key]{"
"	[a:atime|access|use?Access time.]"
"	[c:ctime|status?Status change time.]"
"	[x:extension?File name extension.]"
"	[m:mtime|time?Modify time.]"
"	[f:name?File name.]"
"	[n:none?Don't sort.]"
"	[s:size|blocks?File size.]"
"	[v:version?File name version.]"
"}"
"[Y:layout?Listing layout \akey\a:]:[key]{"
"	[a:across|horizontal?Multi-column across the page.]"
"	[c:comma?Comma separated names across the page.]"
"	[l:long|verbose?Long listing.]"
"	[v:multi-column|vertical?Multi-column by column.]"
"	[1:single-column?One column down the page.]"
"}"
"[z:time-style?List the time according to \astyle\a:]:[style]{"
"	[i:iso?Equivalent to \b+" TIME_ISO "\b.]"
"	[10:posix-iso?No change for the C or posix locales, \biso\b otherwise.]"
"	[f:full-iso?Equivalent to \b+" TIME_FULL_ISO "\b.]"
"	[l:long-iso?Equivalent to \b+" TIME_LONG_ISO "\b.]"
"	[11:posix-full-iso?No change for the C or posix locales, \bfull-iso\b"
"		otherwise.]"
"	[L:locale?Equivalent to \b+" TIME_LOCALE "\b.]"
"	[12:+\aformat\a?A \bdate\b(1) +\aformat\a.]"
"}"
"[1:one-column?List one file per line.]"
"[L:logical|follow?Follow symbolic links. The default is determined by"
"	\bgetconf PATH_RESOLVE\b.]"
"[H:metaphysical?Follow command argument symbolic links, otherwise don't"
"	follow. The default is determined by \bgetconf PATH_RESOLVE\b.]"
"[P:physical?Don't follow symbolic links. The default is determined by"
"	\bgetconf PATH_RESOLVE\b.]"
"[101:block-size?Use \ablocksize\a blocks.]#[blocksize]"
"[102:decimal-scale|thousands?Scale sizes to powers of 1000 { K M G T P X }.]"
"[103:dump?Print the generated \b--format\b string on the standard output"
"	and exit.]"
"[104:testdate?\b--format\b time values newer than \adate\a will be printed"
"	as \adate\a. Used for regression testing.]:[date]"
"[105:testsize?Shift file sizes left \ashift\a bits and set file block counts"
"	to the file size divided by 512. Used for regression testing.]#[shift]"

"\n"
"\n[ file ... ]\n"
"\n"
"[+SEE ALSO?\bchmod\b(1), \bfind\b(1), \bgetconf\b(1), \btw\b(1)]"
"[+BUGS?Can we add options to something else now?]"
;

#include <cmd.h>
#include <ls.h>
#include <ctype.h>
#include <error.h>
#include <fts.h>
#include <sfdisc.h>
#include <tmx.h>
#include <fs3d.h>
#include <dt.h>
#include <vmalloc.h>

#define LS_ACROSS	(LS_USER<<0)	/* multi-column row order	*/
#define LS_ALL		(LS_USER<<1)	/* list all			*/
#define LS_ALWAYS	(LS_USER<<2)	/* always quote			*/
#define LS_COLUMNS	(LS_USER<<3)	/* multi-column column order	*/
#define LS_COMMAS	(LS_USER<<4)	/* comma separated name list	*/
#define LS_DIRECTORY	(LS_USER<<5)	/* list directories as files	*/
#define LS_ESCAPE	(LS_USER<<6)	/* C escape unprintable chars	*/
#define LS_EXTENSION	(LS_USER<<7)	/* sort by name extension	*/
#define LS_LABEL	(LS_USER<<8)	/* label for all dirs		*/
#define LS_MARKDIR	(LS_USER<<9)	/* marks dirs with /		*/
#define LS_MOST		(LS_USER<<10)	/* list all but . and ..	*/
#define LS_NOBACKUP	(LS_USER<<11)	/* omit *~ names		*/
#define LS_NOSTAT	(LS_USER<<13)	/* leaf FTS_NS ok		*/
#define LS_PRINTABLE	(LS_USER<<14)	/* ? for non-printable chars	*/
#define LS_QUOTE	(LS_USER<<15)	/* "..." file names		*/
#define LS_RECURSIVE	(LS_USER<<16)	/* recursive directory descent	*/
#define LS_SEPARATE	(LS_USER<<17)	/* dir header needs separator	*/
#define LS_SHELL	(LS_USER<<18)	/* $'...' file names		*/
#define LS_TIME		(LS_USER<<19)	/* sort by time			*/

#define LS_STAT		LS_NOSTAT

#define VISIBLE(s,f)	((f)->fts_level<=0||(!(s)->ignore||!strmatch((f)->fts_name,(s)->ignore))&&(!((s)->lsflags&LS_NOBACKUP)||(f)->fts_name[(f)->fts_namelen-1]!='~')&&(((s)->lsflags&LS_ALL)||(f)->fts_name[0]!='.'||((s)->lsflags&LS_MOST)&&((f)->fts_name[1]&&(f)->fts_name[1]!='.'||(f)->fts_name[2])))

#define BETWEEN		2		/* space between columns	*/
#define AFTER		1		/* space after last column	*/

#define INVISIBLE	(-1)
#define LISTED		(-2)

#define KEY_environ		(-1)

#define KEY_atime		1
#define KEY_blocks		2
#define KEY_ctime		3
#define KEY_dev			4
#define KEY_device		5
#define KEY_devmajor		6
#define KEY_devminor		7
#define KEY_dir_blocks		8
#define KEY_dir_bytes		9
#define KEY_dir_count		10
#define KEY_dir_files		11
#define KEY_flags		12
#define KEY_gid			13
#define KEY_header		14
#define KEY_ino			15
#define KEY_linkop		16
#define KEY_linkpath		17
#define KEY_mark		18
#define KEY_markdir		19
#define KEY_mode		20
#define KEY_mtime		21
#define KEY_name		22
#define KEY_nlink		23
#define KEY_path		24
#define KEY_perm		25
#define KEY_size		26
#define KEY_summary		27
#define KEY_total_blocks	28
#define KEY_total_bytes		29
#define KEY_total_files		30
#define KEY_trailer		31
#define KEY_uid			32
#define KEY_view		33

#define BLOCKS(s,st)	(((s)->blocksize==LS_BLOCKSIZE)?iblocks(st):(iblocks(st)*LS_BLOCKSIZE+(s)->blocksize-1)/(s)->blocksize)
#define PRINTABLE(s,t)	(((s)->lsflags&LS_PRINTABLE)?printable(s,t):(t))

typedef int (*Order_f)(FTSENT*, FTSENT*);

typedef struct Count_s			/* dir/total counts		*/
{
	Sfulong_t	blocks;		/* number of blocks		*/
	Sfulong_t	bytes;		/* number of bytes		*/
	Sfulong_t	files;		/* number of files		*/
} Count_t;

typedef struct Key_s			/* sfkeyprintf() keys		*/
{
	char*		name;		/* key name			*/
	short		index;		/* index			*/
	short		disable;	/* macro being expanded		*/
	char*		macro;		/* macro definition		*/
	Dtlink_t	link;		/* dict link			*/
} Key_t;

typedef struct List_s			/* list state			*/
{
	Count_t		count;		/* directory counts		*/
	FTSENT*		ent;		/* ent info			*/
	char*		dirnam;		/* pr() dirnam			*/
	int		dirlen;		/* pr() dirlen			*/
} List_t;

typedef struct State_s			/* program state		*/
{
	Shbltin_t*	context;	/* builtin context		*/
	Dtdisc_t	keydisc;	/* key dict discipline		*/
	Dt_t*		keys;		/* key dict			*/
	char		flags[64];	/* command line option flags	*/
	int		ftsflags;	/* FTS_* flags			*/
	long		lsflags;	/* LS_* flags			*/
	long		timeflags;	/* time LS_* flags		*/
	long		blocksize;	/* file block size		*/
	unsigned long	directories;	/* directory count		*/
	unsigned long	testdate;	/* --format test date		*/
	Count_t		total;		/* total counts			*/
	int		adjust;		/* key() print with adjustment	*/
	int		comma;		/* LS_COMMAS ent.level crossing	*/
	int		height;		/* output height in lines	*/
	int		reverse;	/* reverse the sort		*/
	int		scale;		/* metric scale power		*/
	int		testsize;	/* st_size left shift		*/
	int		width;		/* output width in chars	*/
	char*		endflags;	/* trailing 0 in flags		*/
	char*		format;		/* sfkeyprintf() format		*/
	char*		ignore;		/* ignore files matching this	*/
	char*		timefmt;	/* time list format		*/
	char*		prdata;		/* pr buffer			*/
	size_t		prsize;		/* pr buffer size		*/
	char*		txtdata;	/* txt buffer			*/
	size_t		txtsize;	/* txt buffer size		*/
	unsigned short*	siz;		/* siz buffer			*/
	size_t		sizsiz;		/* siz buffer size		*/
	FTSENT**	vec;		/* vec buffer			*/
	size_t		vecsiz;		/* vec buffer size		*/
	Sfio_t*		tmp;		/* tmp string stream		*/
	FTSENT*		top;		/* top directory -- no label	*/
	Order_f		order;		/* sort comparison function	*/
	List_t		list;		/* list state			*/
	Vmalloc_t*	vm;		/* allocation region		*/
} State_t;

static char	DEF_header[] =
"%(dir.count:case;0;;1;%(path)s:\n;*;\n%(path)s:\n)s"
"%(flags:case;*d*;;*[ls]*;total %(dir.blocks)u\n)s"
;

static Key_t	keys[] =
{
	{ 0 },
	{ "atime",		KEY_atime		},
	{ "blocks",		KEY_blocks		},
	{ "ctime",		KEY_ctime		},
	{ "dev",		KEY_dev			},
	{ "device",		KEY_device		},
	{ "devmajor",		KEY_devmajor		},
	{ "devminor",		KEY_devminor		},
	{ "dir.blocks",		KEY_dir_blocks		},
	{ "dir.bytes",		KEY_dir_bytes		},
	{ "dir.count",		KEY_dir_count		},
	{ "dir.files",		KEY_dir_files		},
	{ "flags",		KEY_flags		},
	{ "gid",		KEY_gid			},
	{ "header",		KEY_header, 0, DEF_header },
	{ "ino",		KEY_ino			},
	{ "linkop",		KEY_linkop		},
	{ "linkpath",		KEY_linkpath		},
	{ "mark",		KEY_mark		},
	{ "markdir",		KEY_markdir		},
	{ "mode",		KEY_mode		},
	{ "mtime",		KEY_mtime		},
	{ "name",		KEY_name		},
	{ "nlink",		KEY_nlink		},
	{ "path",		KEY_path		},
	{ "perm",		KEY_perm		},
	{ "size",		KEY_size		},
	{ "summary",		KEY_summary		},
	{ "total.blocks",	KEY_total_blocks	},
	{ "total.bytes",	KEY_total_bytes		},
	{ "total.files",	KEY_total_files		},
	{ "trailer",		KEY_trailer		},
	{ "uid",		KEY_uid			},
	{ "view",		KEY_view		},

	/* aliases */

	{ "linkname",		KEY_linkpath		},
};

/*
 * return a copy of s with unprintable chars replaced by ?
 */

static char*
printable(State_t* state, register char* s)
{
	register char*	t;
	register char*	p;
	register int	c;
	wchar_t		w;
	Mbstate_t	q;

	if (state->lsflags & LS_ESCAPE)
	{
		if (!(state->lsflags & LS_QUOTE))
			return fmtesc(s);
		if (state->lsflags & LS_SHELL)
			return fmtquote(s, "$'", "'", strlen(s), (state->lsflags & LS_ALWAYS) ? FMT_ALWAYS : 0);
		return fmtquote(s, "\"", "\"", strlen(s), FMT_ALWAYS);
	}
	c = strlen(s) + 4;
	if (c > state->prsize)
	{
		state->prsize = roundof(c, 512);
		if (!(state->prdata = vmnewof(state->vm, state->prdata, char, state->prsize, 0)))
		{
			error(ERROR_SYSTEM|2, "out of space");
			return 0;
		}
	}
	t = state->prdata;
	if (state->lsflags & LS_QUOTE)
		*t++ = '"';
	if (!mbwide())
		while (c = *s++)
			*t++ = (iscntrl(c) || !isprint(c)) ? '?' : c;
	else
	{
		mbinit(&q);
		for (p = s; c = mbchar(&w, s, MB_LEN_MAX, &q);)
			if (mberrno(&q))
			{
				s++;
				*t++ = '?';
			}
			else if (mbwidth(c) <= 0)
				*t++ = '?';
			else
				while (p < s)
					*t++ = *p++;
	}
	if (state->lsflags & LS_QUOTE)
		*t++ = '"';
	*t = 0;
	return state->prdata;
}

/*
 * sfkeyprintf() lookup
 */

static int
key(void* handle, register Sffmt_t* fp, const char* arg, char** ps, Sflong_t* pn)
{
	State_t*		state= (State_t*)handle;
	register FTSENT*		ent;
	register struct stat*	st;
	register char*		s = 0;
	register Sflong_t	n = 0;
	register Key_t*		kp;
	Time_t			t;

	static Sfio_t*		mp;
	static const char	fmt_mode[] = "mode";
	static const char	fmt_perm[] = "perm";
	static const char	fmt_time[] = "time";

	if (!fp->t_str)
		return 0;
	if (ent = state->list.ent)
		st = ent->fts_statp;
	else
		st = 0;
	t = TMX_NOTIME;
	if (!(kp = (Key_t*)dtmatch(state->keys, fp->t_str)))
	{
		if (*fp->t_str != '$')
		{
			error(2, "%s: unknown format key", fp->t_str);
			return 0;
		}
		if (!(kp = vmnewof(state->vm, 0, Key_t, 1, strlen(fp->t_str) + 1)))
			goto nospace;
		kp->name = (char*)(kp + 1);
		strcpy(kp->name, fp->t_str);
		kp->macro = getenv(fp->t_str + 1);
		kp->index = KEY_environ;
		kp->disable = 1;
	}
	if (kp->macro && !kp->disable)
	{
		kp->disable = 1;
		if (!mp && !(mp = sfstropen()))
			goto nospace;
		sfkeyprintf(mp, handle, kp->macro, key, NiL);
		if (!(s = sfstruse(mp)))
			goto nospace;
		kp->disable = 0;
	}
	else switch (kp->index)
	{
	case KEY_atime:
		if (st)
		{
			n = st->st_atime;
			t = tmxgetatime(st);
		}
		if (!arg)
			arg = state->timefmt;
		break;
	case KEY_blocks:
		if (st)
			n = BLOCKS(state, st);
		break;
	case KEY_ctime:
		if (st)
		{
			n = st->st_ctime;
			t = tmxgetctime(st);
		}
		if (!arg)
			arg = state->timefmt;
		break;
	case KEY_dev:
		if (st)
			s = fmtdev(st);
		break;
	case KEY_device:
		if (st && (S_ISBLK(st->st_mode) || S_ISCHR(st->st_mode)))
			s = fmtdev(st);
		else
			return 0;
		break;
	case KEY_devmajor:
		if (st)
			n = (S_ISBLK(st->st_mode) || S_ISCHR(st->st_mode)) ? major(idevice(st)) : major(st->st_dev);
		break;
	case KEY_devminor:
		if (st)
			n = (S_ISBLK(st->st_mode) || S_ISCHR(st->st_mode)) ? minor(idevice(st)) : minor(st->st_dev);
		break;
	case KEY_dir_blocks:
		if (!state->scale)
		{
			n = state->list.count.blocks;
			break;
		}
		/*FALLTHROUGH*/
	case KEY_dir_bytes:
		n = state->list.count.bytes;
		if (state->scale)
		{
			s = fmtscale(n, state->scale);
			fp->fmt = 's';
		}
		break;
	case KEY_dir_count:
		if (ent != state->top)
		{
			if (state->lsflags & LS_SEPARATE)
				n = state->directories;
			else if (state->lsflags & LS_LABEL)
				n = 1;
		}
		break;
	case KEY_dir_files:
		n = state->list.count.files;
		break;
	case KEY_environ:
		if (!(s = kp->macro))
			return 0;
		break;
	case KEY_flags:
		s = state->flags;
		break;
	case KEY_gid:
		if (st)
		{
			if (fp->fmt == 's')
				s = fmtgid(st->st_gid);
			else
				n = st->st_gid;
		}
		break;
	case KEY_ino:
		if (st)
			n = st->st_ino;
		break;
	case KEY_linkpath:
		if (ent && (ent->fts_info & FTS_SL))
		{
			char*		dirnam;
			int		c;

			if ((st->st_size + 1) > state->txtsize)
			{
				state->txtsize = roundof(st->st_size + 1, 512);
				if (!(state->txtdata = vmnewof(state->vm, state->txtdata, char, state->txtsize, 0)))
					goto nospace;
			}
			if (*ent->fts_name == '/' || !state->list.dirnam)
				dirnam = ent->fts_name;
			else
			{
				sfprintf(state->tmp, "%s/%s", state->list.dirnam + streq(state->list.dirnam, "/"), ent->fts_name);
				if (!(dirnam = sfstruse(state->tmp)))
					goto nospace;
			}
			c = pathgetlink(dirnam, state->txtdata, state->txtsize);
			s = (c > 0) ? PRINTABLE(state, state->txtdata) : "";
		}
		else
			return 0;
		break;
	case KEY_linkop:
		if (ent && (ent->fts_info & FTS_SL))
			s = "->";
		else
			return 0;
		break;
	case KEY_mark:
		if (!st)
			return 0;
		else if (S_ISLNK(st->st_mode))
			s = "@";
		else if (S_ISDIR(st->st_mode))
			s = "/";
#ifdef S_ISDOOR
		else if (S_ISDOOR(st->st_mode))
			s = ">";
#endif
		else if (S_ISFIFO(st->st_mode))
			s = "|";
#ifdef S_ISSOCK
		else if (S_ISSOCK(st->st_mode))
			s = "=";
#endif
		else if (S_ISBLK(st->st_mode) || S_ISCHR(st->st_mode))
			s = "$";
		else if (st->st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))
			s = "*";
		else
			return 0;
		break;
	case KEY_markdir:
		if (!st || !S_ISDIR(st->st_mode))
			return 0;
		s = "/";
		break;
	case KEY_mode:
		if (st)
			n = st->st_mode;
		if (!arg)
			arg = fmt_mode;
		break;
	case KEY_mtime:
		if (st)
		{
			n = st->st_mtime;
			t = tmxgetmtime(st);
		}
		if (!arg)
			arg = state->timefmt;
		break;
	case KEY_name:
		if (ent)
			s = PRINTABLE(state, ent->fts_name);
		break;
	case KEY_nlink:
		if (st)
			n = st->st_nlink;
		break;
	case KEY_path:
		if (ent)
			s = ent->fts_path ? PRINTABLE(state, ent->fts_path) : PRINTABLE(state, ent->fts_name);
		break;
	case KEY_perm:
		if (st)
			n = st->st_mode & S_IPERM;
		if (!arg)
			arg = fmt_perm;
		break;
	case KEY_size:
		if (st)
		{
			n = st->st_size;
			if (state->scale)
			{
				s = fmtscale(n, state->scale);
				fp->fmt = 's';
			}
		}
		break;
	case KEY_total_blocks:
		if (!state->scale)
		{
			n = state->total.blocks;
			break;
		}
		/*FALLTHROUGH*/
	case KEY_total_bytes:
		n = state->total.bytes;
		if (state->scale)
		{
			s = fmtscale(n, state->scale);
			fp->fmt = 's';
		}
		break;
	case KEY_total_files:
		n = state->total.files;
		break;
	case KEY_uid:
		if (st)
		{
			if (fp->fmt == 's')
				s = fmtuid(st->st_uid);
			else
				n = st->st_uid;
		}
		break;
	case KEY_view:
		if (st)
			n = iview(st);
		break;
	default:
		return 0;
	}
	if (s)
	{
		*ps = s;
		if (mbwide())
		{
			register char*	p;
			wchar_t		w;
			Mbstate_t	q;
			int		i;

			mbinit(&q);
			for (p = s; w = mbchar(&w, s, MB_LEN_MAX, &q); p = s)
				if (mberrno(&q))
					s++;
				else if ((i = mbwidth(w)) >= 0)
					state->adjust -= (s - p) + i - 2;
		}
	}
	else if (fp->fmt == 's' && arg)
	{
		if (strneq(arg, fmt_mode, sizeof(fmt_mode) - 1))
			*ps = fmtmode(n, 0);
		else if (strneq(arg, fmt_perm, sizeof(fmt_perm) - 1))
			*ps = fmtperm(n & S_IPERM);
		else
		{
			if (strneq(arg, fmt_time, sizeof(fmt_time) - 1))
			{
				arg += sizeof(fmt_time) - 1;
				if (*arg == '=')
					arg++;
			}
			if (!*arg)
				arg = state->timefmt;
			if ((unsigned long)n >= state->testdate)
			{
				n = state->testdate;
				t = TMX_NOTIME;
			}
			*ps = t == TMX_NOTIME ? fmttime(arg, (time_t)n) : fmttmx(arg, t);
		}
	}
	else
		*pn = n;
	return 1;
 nospace:
	error(ERROR_SYSTEM|2, "out of space");
	return 0;
}

/*
 * print info on a single file
 * parent directory name is dirnam of dirlen chars
 */

static void
pr(State_t* state, FTSENT* ent, register int fill)
{
#ifdef S_ISLNK
	/*
	 * -H == --hairbrained
	 * no way around it - this is bud tugley
	 * symlinks should be no more visible than mount points
	 * but I wear my user hat more than my administrator hat
	 */

	if (ent->fts_level == 0 && (state->ftsflags & (FTS_META|FTS_PHYSICAL)) == (FTS_META|FTS_PHYSICAL) && !(ent->fts_info & FTS_D) && !lstat(ent->fts_path ? ent->fts_path : ent->fts_name, ent->fts_statp) && S_ISLNK(ent->fts_statp->st_mode))
		ent->fts_info = FTS_SL;
#endif
	if (state->testsize && (ent->fts_info & FTS_F))
	{
		ent->fts_statp->st_size <<= state->testsize;
		ent->fts_statp->st_blocks = ent->fts_statp->st_size / LS_BLOCKSIZE;
	}
	state->list.ent = ent;
	state->adjust = 0;
	fill -= sfkeyprintf(sfstdout, state, state->format, key, NiL) + state->adjust;
	if (!(state->lsflags & LS_COMMAS))
	{
		if (fill > 0)
			while (fill-- > 0)
				sfputc(sfstdout, ' ');
		else
			sfputc(sfstdout, '\n');
	}
}

/*
 * pr() ent directory child list in column order
 * directory name is dirnam of dirlen chars
 * count is the number of VISIBLE children
 * length is the length of the longest VISIBLE child
 */

static void
col(State_t* state, register FTSENT* ent, int length)
{
	register FTSENT*	p;
	register int	i;
	register int	n;
	register int	files;
	register char*	s;
	int		w;
	int		a;

	state->list.ent = ent;
	if (keys[KEY_header].macro && ent->fts_level >= 0)
		sfkeyprintf(sfstdout, state, keys[KEY_header].macro, key, NiL);
	if ((files = state->list.count.files) > 0)
	{
		if (!(state->lsflags & LS_COLUMNS) || length <= 0)
		{
			n = w = 1;
			a = 0;
		}
		else
		{
			i = ent->fts_name[1];
			ent->fts_name[1] = 0;
			state->adjust = 2;
			a = sfkeyprintf(state->tmp, state, state->format, key, NiL) - 1;
			w = a + state->adjust + 1;
			length += w;
			sfstrseek(state->tmp, 0, SEEK_SET);
			ent->fts_name[1] = i;
			n = ((state->width - (length + BETWEEN + 2)) < 0) ? 1 : 2;
		}
		if (state->lsflags & LS_COMMAS)
		{
			length = w - 1;
			i = 0;
			n = state->width;
			for (p = ent->fts_link; p; p = p->fts_link)
				if (p->fts_number != INVISIBLE)
				{
					if (!mbwide())
						w = p->fts_namelen;
					else
					{
						wchar_t		x;
						Mbstate_t	q;

						mbinit(&q);
						for (s = p->fts_name; w = mbchar(&x, s, MB_LEN_MAX, &q);)
							if (mberrno(&q))
							{
								s++;
								w++;
							}
							else if ((n = mbwidth(i)) > 0)
								w += n;
					}
					w += a;
					if ((n -= length + w) < 0)
					{
						n = state->width - (length + w);
						if (i)
							sfputr(sfstdout, ",\n", -1);
					}
					else if (i)
						sfputr(sfstdout, ", ", -1);
					pr(state, p, 0);
					i = 1;
				}
			if (i)
				sfputc(sfstdout, '\n');
		}
		else if (n <= 1)
		{
			for (p = ent->fts_link; p; p = p->fts_link)
				if (p->fts_number != INVISIBLE)
					pr(state, p, 0);
		}
		else
		{
			register FTSENT**	x;
			int			c;
			int			j;
			int			k;
			int			l;
			int			m;
			int			o;
			int			q;
			int			r;
			int			w;
			int			z;

			if (files > state->sizsiz)
			{
				state->sizsiz = roundof(files, 64);
				if (!(state->siz = vmnewof(state->vm, state->siz, unsigned short, state->sizsiz, 0)))
					error(ERROR_SYSTEM|2, "out of space");
			}
			if ((files + 1) > state->vecsiz)
			{
				state->vecsiz = roundof(files + 1, 64);
				if (!(state->vec = vmnewof(state->vm, state->vec, FTSENT*, state->vecsiz, 0)))
					error(ERROR_SYSTEM|2, "out of space");
			}
			x = state->vec;
			i = 0;
			for (p = ent->fts_link; p; p = p->fts_link)
				if (p->fts_number != INVISIBLE)
					x[i++] = p;
			n = i / (state->width / (length + BETWEEN)) + 1;
			o = 0;
			if ((state->lsflags & LS_ACROSS) && n > 1)
			{
				c = (i - 1) / n + 1;
				do
				{
					w = -AFTER;
					for (j = 0; j < c; j++)
					{
						z = 0;
						for (l = 0, r = j; l < n && r < i; r += c, l++)
							if (z < (x[r]->fts_namelen + a))
								z = x[r]->fts_namelen + a;
						w += z + BETWEEN;
					}
					if (w <= state->width)
						o = n;
				} while (c < state->width / 2 && (n = (i + c) / (c + 1)) && ++c);
				n = o ? o : 1;
				c = (i - 1) / n + 1;
				k = 0;
				for (j = 0; j < c; j++)
				{
					state->siz[k] = 0;
					for (l = 0, r = j; l < n && r < i; r += c, l++)
						if (state->siz[k] < x[r]->fts_namelen)
							state->siz[k] = x[r]->fts_namelen;
					state->siz[k] += a + BETWEEN;
					k++;
				}
				for (j = 0; j <= i; j += c)
					for (l = 0, w = j; l < k && w < i; l++, w++)
						pr(state, x[w], l < (k - 1) && w < (i - 1) ? state->siz[l] : 0);
			}
			else
			{
				o = 0;
				if (n > 1)
				{
					if (!(q = i / n))
						q = 1;
					for (c = q; (c - q) < 2 && c <= state->width / (BETWEEN + 1); ++c)
					{
						n = m = (i + c - 1) / c;
						if ((r = i - m * c) > state->height)
							n -= (r + c - 1) / c;
						for (; n <= m; n++)
						{
							w = -AFTER;
							j = 0;
							while (j < i)
							{
								z = 0;
								for (l = 0; l < n && j < i; j++, l++)
									if (z < x[j]->fts_namelen)
										z = x[j]->fts_namelen;
								w += z + a + BETWEEN;
							}
							if (w <= state->width)
							{
								q = c;
								o = n;
								break;
							}
						}
					}
				}
				n = o ? o : 1;
				j = k = 0;
				while (j < i)
				{
					state->siz[k] = 0;
					for (l = 0; l < n && j < i; j++, l++)
						if (state->siz[k] < x[j]->fts_namelen)
							state->siz[k] = x[j]->fts_namelen;
					state->siz[k] += a + BETWEEN;
					k++;
				}
				for (j = 0; j < n; j++)
					for (l = 0, w = j; l < k && w < i; l++, w += n)
						pr(state, x[w], l < (k - 1) && w < (i - n) ? state->siz[l] : 0);
			}
		}
	}
	if (keys[KEY_trailer].macro && ent->fts_level >= 0)
		sfkeyprintf(sfstdout, state, keys[KEY_trailer].macro, key, NiL);
}

/*
 * order() helpers
 */

static int
order_none(register FTSENT* f1, register FTSENT* f2)
{
	return 0;
}

static int
order_blocks(register FTSENT* f1, register FTSENT* f2)
{
	if (f1->fts_statp->st_size < f2->fts_statp->st_size)
		return 1;
	if (f1->fts_statp->st_size > f2->fts_statp->st_size)
		return -1;
	return 0;
}

static int
order_atime(register FTSENT* f1, register FTSENT* f2)
{
	Time_t		t1;
	Time_t		t2;

	t1 = tmxgetatime(f1->fts_statp);
	t2 = tmxgetatime(f2->fts_statp);
	if (t1 < t2)
		return 1;
	if (t1 > t2)
		return -1;
	return 0;
}

static int
order_ctime(register FTSENT* f1, register FTSENT* f2)
{
	Time_t		t1;
	Time_t		t2;

	t1 = tmxgetctime(f1->fts_statp);
	t2 = tmxgetctime(f2->fts_statp);
	if (t1 < t2)
		return 1;
	if (t1 > t2)
		return -1;
	return 0;
}

static int
order_mtime(register FTSENT* f1, register FTSENT* f2)
{
	Time_t		t1;
	Time_t		t2;

	t1 = tmxgetmtime(f1->fts_statp);
	t2 = tmxgetmtime(f2->fts_statp);
	if (t1 < t2)
		return 1;
	if (t1 > t2)
		return -1;
	return 0;
}

static int
order_extension(register FTSENT* f1, register FTSENT* f2)
{
	register int	n;
	char*		x1;
	char*		x2;

	x1 = strrchr(f1->fts_name, '.');
	x2 = strrchr(f2->fts_name, '.');
	if (x1)
	{
		if (x2)
			n = strcoll(x1, x2);
		else
			n = 1;
	}
	else if (x2)
		n = -1;
	else
		n = 0;
	if (!n)
		n = strcoll(f1->fts_name, f2->fts_name);
	return n;
}

static int
order_version(FTSENT* f1, FTSENT* f2)
{
	return strvcmp(f1->fts_name, f2->fts_name);
}

static int
order_name(FTSENT* f1, FTSENT* f2)
{
	return strcoll(f1->fts_name, f2->fts_name);
}

/*
 * order child entries
 */

static int
order(register FTSENT* const* f1, register FTSENT* const* f2)
{
	State_t*	state = (State_t*)(*f1)->fts->fts_handle;
	int		n;

	if ((state->lsflags & (LS_DIRECTORY|LS_LABEL)) == LS_LABEL && (*f1)->fts_level == 0)
	{
		if ((*f1)->fts_info == FTS_D)
		{
			if ((*f2)->fts_info != FTS_D)
				return 1;
		}
		else if ((*f2)->fts_info == FTS_D)
			return -1;
	}
	n = (*state->order)(*f1, *f2);
	return state->reverse ? -n : n;
}

/*
 * list a directory and its children
 */

static void
dir(State_t* state, register FTSENT* ent)
{
	register FTSENT*	p;
	register int		length;
	int			top = 0;

	state->list.dirlen = ent->fts_namelen;
	state->list.dirnam = ent->fts_name;
	if (ent->fts_level >= 0)
		state->directories++;
	else
		state->top = ent;
	length = 0;
	state->list.count.blocks = 0;
	state->list.count.bytes = 0;
	state->list.count.files = 0;
	for (p = ent->fts_link; p; p = p->fts_link)
	{
		if (p->fts_level == 0 && p->fts_info == FTS_D && !(state->lsflags & LS_DIRECTORY))
		{
			p->fts_number = INVISIBLE;
			top++;
		}
		else if (VISIBLE(state, p))
		{
			if (p->fts_info == FTS_NS)
			{
				if (ent->fts_level < 0 || !(state->lsflags & LS_NOSTAT))
				{
					if (ent->fts_path[0] == '.' && !ent->fts_path[1])
						error(2, "%s: not found", p->fts_name);
					else
						error(2, "%s/%s: not found", ent->fts_path, p->fts_name);
					goto invisible;
				}
			}
			else
			{
				state->list.count.blocks += BLOCKS(state, p->fts_statp);
				state->list.count.bytes += p->fts_statp->st_size;
			}
			state->list.count.files++;
			if (p->fts_namelen > length)
				length = p->fts_namelen;
			if (!(state->lsflags & LS_RECURSIVE))
				fts_set(NiL, p, FTS_SKIP);
		}
		else
		{
		invisible:
			p->fts_number = INVISIBLE;
			fts_set(NiL, p, FTS_SKIP);
		}
	}
	state->total.blocks += state->list.count.blocks;
	state->total.bytes += state->list.count.bytes;
	state->total.files += state->list.count.files;
	col(state, ent, length);
	state->lsflags |= LS_SEPARATE;
	if (top)
	{
		if (state->list.count.files)
		{
			state->directories++;
			state->top = 0;
		}
		else if (top > 1)
			state->top = 0;
		else
			state->top = ent->fts_link;
		for (p = ent->fts_link; p; p = p->fts_link)
			if (p->fts_level == 0 && p->fts_info == FTS_D)
				p->fts_number = 0;
	}
}

/*
 * list info on a single file
 */

static int
ls(State_t* state, register FTSENT* ent)
{
	if (!VISIBLE(state, ent))
	{
		fts_set(NiL, ent, FTS_SKIP);
		return;
	}
	switch (ent->fts_info)
	{
	case FTS_NS:
		if (ent->fts_parent->fts_info == FTS_DNX)
			break;
		error(2, "%s: not found", ent->fts_path);
		return;
	case FTS_DC:
		if (state->lsflags & LS_DIRECTORY)
			break;
		error(2, "%s: directory causes cycle", ent->fts_path);
		return;
	case FTS_DNR:
		if (state->lsflags & LS_DIRECTORY)
			break;
		error(2, "%s: cannot read directory", ent->fts_path);
		return 0;
	case FTS_DOT:
		#if 0
		fts_set(NiL, ent, FTS_SKIP);
		/*FALLTHROUGH*/
		#endif
	case FTS_D:
	case FTS_DNX:
		if ((state->lsflags & LS_DIRECTORY) && ent->fts_level >= 0)
			break;
		if (!(state->lsflags & LS_RECURSIVE))
			fts_set(NiL, ent, FTS_SKIP);
		else if (ent->fts_info == FTS_DNX)
		{
			error(2, "%s: cannot search directory", ent->fts_path, ent->fts_level);
			fts_set(NiL, ent, FTS_SKIP);
			if (ent->fts_level > 0 && !(state->lsflags & LS_NOSTAT))
				return 0;
		}
		if (ent->fts_level >= 0)
			fts_children(ent->fts, 0);
		dir(state, ent);
		return 0;
	}
	fts_set(NiL, ent, FTS_SKIP);
	if (ent->fts_level == 1)
	{
		memset(&state->list, 0, sizeof(state->list));
		state->list.ent = ent;
		pr(state, ent, 0);
	}
	return 0;
}

/*
 * snarfed from libast/misc/ftwalk.c
 * need a rainy day to clean this up ...
 */

static int
walk(const char* path, int (*userf)(State_t*, FTSENT*), int flags, int (*comparf)(FTSENT*const*, FTSENT*const*), State_t* state)
{
	register FTS*		f;
	register FTSENT*	e;
	register int		rv;
	int			oi;
	int			ns;
	int			os;
	int			nd;
	FTSENT*			x;
	FTSENT*			dd[2];

	flags |= FTS_NOPOSTORDER;
	if (!(f = fts_open((char* const*)path, flags, comparf)))
	{
		if (!path || !(path = (const char*)(*((char**)path))))
			return -1;
		ns = strlen(path) + 1;
		if (!(e = vmnewof(state->vm, 0, FTSENT, 1, ns)))
			return -1;
		e->fts_accpath = e->fts_name = e->fts_path = strcpy((char*)(e + 1), path);
		e->fts_namelen = e->fts_pathlen = ns;
		e->fts_info = FTS_NS;
		e->fts_parent = e;
		e->fts_parent->fts_link = e;
		rv = (*userf)(state, e);
		return rv;
	}
	f->fts_handle = state;
	rv = 0;
	if (e = fts_children(f, 0))
	{
		nd = 0;
		for (x = e; x; x = x->fts_link)
			if (x->fts_info & FTS_DD)
			{
				x->fts_info &= ~FTS_DD;
				dd[nd++] = x;
				if (nd >= elementsof(dd))
					break;
			}
		e->fts_parent->fts_link = e;
		rv = (*userf)(state, e->fts_parent);
		e->fts_parent->fts_link = 0;
		while (nd > 0)
			dd[--nd]->fts_info |= FTS_DD;
		for (x = e; x; x = x->fts_link)
			if (!(x->fts_info & FTS_D))
				x->_fts_status = FTS_SKIP;
	}
	while (!rv && !sh_checksig(state->context) && (e = fts_read(f)))
	{
		oi = e->fts_info;
		os = e->_fts_status;
		nd = 0;
		switch (e->fts_info)
		{
		case FTS_D:
		case FTS_DNX:
			for (x = fts_children(f, 0); x; x = x->fts_link)
				if (x->fts_info & FTS_DD)
				{
					x->fts_info &= ~FTS_DD;
					dd[nd++] = x;
					if (nd >= elementsof(dd))
						break;
				}
			break;
		case FTS_DOT:
			continue;
		case FTS_ERR:
			e->fts_info = FTS_NS;
			break;
		case FTS_NSOK:
			e->fts_info = FTS_NSOK;
			break;
		case FTS_SLNONE:
			e->fts_info = FTS_SL;
			break;
		}
		rv = (*userf)(state, e);
		e->fts_info = oi;
		if (e->_fts_status == ns)
			e->_fts_status = os;
		while (nd > 0)
			dd[--nd]->fts_info |= FTS_DD;
	}
	fts_close(f);
	return rv;
}

#define set(s,f)	(opt_info.num?((s)->lsflags|=(f)):(((s)->lsflags&=~(f)),0))
#define clr(s,f)	(opt_info.num?((s)->lsflags&=~(f)):((s)->lsflags|=(f)))

int
b_ls(int argc, register char** argv, Shbltin_t* context)
{
	register int	n;
	register char*	s;
	char*		e;
	FTS*		fts;
	FTSENT*		ent;
	FTSENT*		x;
	Key_t*		kp;
	Sfio_t*		fmt;
	long		lsflags;
	int		dump = 0;
	char*		av[2];
	int		oi;
	int		ns;
	int		os;
	int		nd;
	FTSENT*		dd[2];
	State_t		state;

	static char	fmt_color[] = "%(mode:case:d*:\\E[01;34m%(name)s\\E[0m:l*:\\E[01;36m%(name)s\\E[0m:*x*:\\E[01;32m%(name)s\\E[0m:*:%(name)s)s";

	cmdinit(argc, argv, context, ERROR_CATALOG, ERROR_NOTIFY);
	memset(&state, 0, sizeof(state));
	state.keydisc.key = offsetof(Key_t, name);
	state.keydisc.size = -1;
	state.keydisc.link = offsetof(Key_t, link);
	if (!(fmt = sfstropen()) || !(state.tmp = sfstropen()) || !(state.vm = vmopen(Vmdcheap, Vmbest, 0)) || !(state.keys = dtnew(state.vm, &state.keydisc, Dtset)))
	{
		error(ERROR_SYSTEM|2, "out of space");
		goto done;
	}
	if (s = strrchr(argv[0], '/'))
		s++;
	else
		s = argv[0];
	error_info.id = s;
	state.ftsflags = fts_flags() | FTS_META | FTS_SEEDOT;
	for (n = 1; n < elementsof(keys); n++)
		dtinsert(state.keys, keys + n);
	if (streq(s, "lc"))
		state.lsflags |= LS_COLUMNS;
	else if (streq(s, "lf") || streq(s, "lsf"))
		state.lsflags |= LS_MARK;
	else if (streq(s, "ll"))
		state.lsflags |= LS_LONG;
	else if (streq(s, "lsr"))
		state.lsflags |= LS_RECURSIVE;
	else if (streq(s, "lsx"))
		state.lsflags |= LS_ACROSS|LS_COLUMNS;
	else if (isatty(1))
	{
		state.lsflags |= LS_COLUMNS;
		if (!strmatch(setlocale(LC_ALL, NiL), "*[Uu][Tt][Ff]?(-)8"))
			state.lsflags |= LS_PRINTABLE;
	}
	state.endflags = state.flags;
	state.blocksize = 512;
	state.testdate = ~0;
	state.timefmt = "%?%l";
	lsflags = state.lsflags;
	while (n = optget(argv, usage))
	{
		switch (n)
		{
		case 'a':
			set(&state, LS_ALL);
			break;
		case 'b':
			set(&state, LS_PRINTABLE|LS_ESCAPE);
			break;
		case 'c':
			state.lsflags &= ~LS_ATIME;
			state.lsflags |= LS_CTIME;
			if (!state.order)
				state.order = order_ctime;
			break;
		case 'd':
			set(&state, LS_DIRECTORY);
			break;
		case 'e':
			state.lsflags |= LS_LONG;
			state.timefmt = TIME_LONG_ISO;
			break;
		case 'f':
			state.lsflags |= LS_ALL;
			state.lsflags &= ~(LS_BLOCKS|LS_LONG|LS_TIME);
			state.reverse = 0;
			state.order = order_none;
			break;
		case 'g':
		case 'O':
			if (opt_info.num)
				state.lsflags |= LS_LONG|LS_NOUSER;
			else
				state.lsflags |= LS_LONG|LS_NOGROUP;
			break;
		case 'h':
			state.scale = 1024;
			break;
		case 'i':
			set(&state, LS_INUMBER);
			break;
		case 'k':
			state.blocksize = 1024;
			break;
		case 'l':
			set(&state, LS_LONG);
			break;
		case 'm':
			set(&state, LS_COMMAS);
			break;
		case 'n':
			set(&state, LS_NUMBER);
			break;
		case 'o':
		case 'G':
			if (opt_info.num)
				state.lsflags |= LS_LONG|LS_NOGROUP;
			else
				state.lsflags |= LS_LONG|LS_NOUSER;
			break;
		case 'p':
			set(&state, LS_MARKDIR);
			break;
		case 'q':
			set(&state, LS_PRINTABLE);
			break;
		case 'r':
			state.reverse = !!opt_info.num;
			break;
		case 's':
			set(&state, LS_BLOCKS);
			break;
		case 't':
			if (set(&state, LS_TIME) && !state.order)
				state.order = order_mtime;
			break;
		case 'u':
			state.lsflags &= ~LS_CTIME;
			state.lsflags |= LS_ATIME;
			if (!state.order)
				state.order = order_atime;
			break;
		case 'w':
			state.width = strtol(opt_info.arg, &e, 0); 
			if (*e == 'x' || *e == 'X' || *e == '.' || *e == '+')
			{
				state.height = state.width;
				state.width = strtol(e + 1, &e, 0); 
			}
			if (*e)
				error(2, "%s: invalid screen width specification at `%s'", opt_info.arg, e);
			break;
		case 'x':
			set(&state, LS_ACROSS|LS_COLUMNS);
			break;
		case 'y':
			if (!opt_info.arg)
				state.order = order_none;
			else
				switch (opt_info.num)
				{
				case 'a':
					state.order = order_atime;
					break;
				case 'c':
					state.order = order_ctime;
					break;
				case 'f':
					state.order = 0;
					break;
				case 'm':
					state.order = order_mtime;
					break;
				case 'n':
					state.order = order_none;
					break;
				case 's':
					state.order = order_blocks;
					break;
				case 't':
					state.order = order_mtime;
					break;
				case 'v':
					state.order = order_version;
					break;
				case 'x':
					state.order = order_extension;
					break;
				}
			break;
		case 'z':
			switch (opt_info.num)
			{
			case -10:
				if (!strcmp(setlocale(LC_TIME, NiL), "C"))
					break;
				/*FALLTHROUGH*/
			case 'i':
				state.timefmt = TIME_ISO;
				break;
			case -11:
				if (!strcmp(setlocale(LC_TIME, NiL), "C"))
					break;
				/*FALLTHROUGH*/
			case 'f':
				state.timefmt = TIME_FULL_ISO;
				break;
			case 'l':
				state.timefmt = TIME_LONG_ISO;
				break;
			case 'L':
				state.timefmt = TIME_LOCALE;
				break;
			case -12:
				s = opt_info.arg + 1;
				if (strchr(s, '\n'))
				{
					/*
					 * gnu compatibility
					 */

					s = sfprints("%%Q\n%s\n", s);
					if (!s || !(s = vmstrdup(state.vm, s)))
					{
						error(ERROR_SYSTEM|2, "out of space");
						goto done;
					}
				}
				state.timefmt = s;
				break;
			}
			break;
		case 'A':
			state.lsflags |= LS_MOST;
			state.lsflags &= ~LS_ALL;
			break;
		case 'B':
			set(&state, LS_NOBACKUP);
			break;
		case 'C':
			set(&state, LS_COLUMNS);
			break;
		case 'D':
			if (s = strchr(opt_info.arg, '='))
				*s++ = 0;
			if (*opt_info.arg == 'n' && *(opt_info.arg + 1) == 'o')
			{
				opt_info.arg += 2;
				s = 0;
			}
			if (!(kp = (Key_t*)dtmatch(state.keys, opt_info.arg)))
			{
				if (!s)
					break;
				if (!(kp = vmnewof(state.vm, 0, Key_t, 1, 0)))
				{
					error(ERROR_SYSTEM|2, "out of space");
					goto done;
				}
				kp->name = opt_info.arg;
				dtinsert(state.keys, kp);
			}
			if (kp->macro = s)
			{
				stresc(s);
				if (strmatch(s, "*:case:*"))
					state.lsflags |= LS_STAT;
			}
			break;
		case 'E':
			state.lsflags |= LS_LONG;
			state.timefmt = TIME_FULL_ISO;
			break;
		case 'F':
			set(&state, LS_MARK);
			break;
		case 'H':
			state.ftsflags |= FTS_META|FTS_PHYSICAL;
			break;
		case 'I':
			state.ignore = opt_info.arg;
			break;
		case 'J':
			state.lsflags &= ~(LS_ALWAYS|LS_ESCAPE|LS_PRINTABLE|LS_QUOTE|LS_SHELL);
			switch (opt_info.num)
			{
			case 'c':
				state.lsflags |= LS_ESCAPE|LS_PRINTABLE|LS_QUOTE;
				break;
			case 'e':
				state.lsflags |= LS_ESCAPE|LS_PRINTABLE;
				break;
			case 'l':
				break;
			case 'q':
				state.lsflags |= LS_PRINTABLE;
				break;
			case 's':
				state.lsflags |= LS_ESCAPE|LS_PRINTABLE|LS_QUOTE|LS_SHELL;
				break;
			case 'S':
				state.lsflags |= LS_ALWAYS|LS_ESCAPE|LS_PRINTABLE|LS_QUOTE|LS_SHELL;
				break;
			}
			break;
		case 'K':
			set(&state, LS_PRINTABLE|LS_SHELL|LS_QUOTE|LS_ESCAPE);
			break;
		case 'L':
			state.ftsflags &= ~(FTS_META|FTS_PHYSICAL|FTS_SEEDOTDIR);
			break;
		case 'N':
			clr(&state, LS_PRINTABLE);
			break;
		case 'P':
			state.ftsflags &= ~FTS_META;
			state.ftsflags |= FTS_PHYSICAL;
			break;
		case 'Q':
			set(&state, LS_PRINTABLE|LS_QUOTE);
			break;
		case 'R':
			set(&state, LS_RECURSIVE);
			break;
		case 'S':
			state.order = order_blocks;
			break;
		case 'T':
			/* ignored */
			break;
		case 'U':
			state.order = order_none;
			break;
		case 'V':
			switch (opt_info.num)
			{
			case 't':
				if (!isatty(1))
					break;
				/*FALLTHROUGH*/
			case 'a':
				if (kp = (Key_t*)dtmatch(state.keys, "name"))
				{
					stresc(kp->macro = fmt_color);
					state.lsflags |= LS_STAT;
				}
				break;
			}
			break;
		case 'W':
			state.timeflags = 0;
			switch (opt_info.num)
			{
			case 'a':
				state.timeflags = LS_ATIME;
				break;
			case 'c':
				state.timeflags = LS_CTIME;
				break;
			}
			break;
		case 'X':
			set(&state, LS_EXTENSION);
			break;
		case 'Y':
			switch (opt_info.num)
			{
			case 'a':
				state.lsflags |= LS_ACROSS|LS_COLUMNS;
				break;
			case 'c':
				state.lsflags |= LS_COMMAS;
				break;
			case 'l':
				state.lsflags |= LS_LONG;
				break;
			case 'v':
				state.lsflags &= ~LS_ACROSS;
				state.lsflags |= LS_COLUMNS;
				break;
			case '1':
				state.lsflags &= ~(LS_ACROSS|LS_COLUMNS);
				break;
			}
			break;
		case 'Z':
			if (!sfstrtell(fmt))
				state.lsflags &= ~LS_COLUMNS;
			sfputr(fmt, opt_info.arg, ' ');
			break;
		case '1':
			clr(&state, LS_COLUMNS|LS_PRINTABLE);
			break;
		case -101:
			if (opt_info.num <= 0)
				error(3, "%ld: invalid block size", opt_info.num);
			state.blocksize = opt_info.num;
			break;
		case -102:
			state.scale = 1000;
			break;
		case -103:
			dump = 1;
			break;
		case -104:
			state.testdate = tmdate(opt_info.arg, &e, NiL);
			if (*e)
				error(3, "%s: invalid date string", opt_info.arg, opt_info.option);
			break;
		case -105:
			state.testsize = opt_info.num;
			break;
		case '?':
			vmclose(state.vm);
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		default:
			error(1, "%s: option not implemented", opt_info.name);
			continue;
		}
		if (!strchr(state.flags, n))
			*state.endflags++ = n;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (state.lsflags == (lsflags|LS_TIME))
		state.ftsflags |= FTS_SEEDOTDIR; /* keep configure happy */
	if (state.lsflags & LS_DIRECTORY)
		state.lsflags &= ~LS_RECURSIVE;
	if (!state.order)
		state.order = order_name;
	if (!state.timeflags)
		state.timeflags = state.lsflags;
	if (state.lsflags & (LS_COLUMNS|LS_COMMAS))
	{
		if (state.lsflags & LS_LONG)
			state.lsflags &= ~(LS_COLUMNS|LS_COMMAS);
		else
		{
			if (!state.width)
			{
				astwinsize(1, &state.height, &state.width);
				if (state.width <= 20)
					state.width = 80;
			}
			if (state.height <= 4)
				state.height = 24;
		}
	}
	if (state.lsflags & LS_STAT)
		state.lsflags &= ~LS_NOSTAT;
	else if (!(state.lsflags & (LS_DIRECTORY|LS_BLOCKS|LS_LONG|LS_MARK|LS_MARKDIR|LS_TIME
#if !_mem_d_fileno_dirent && !_mem_d_ino_dirent
		|LS_INUMBER
#endif
		)) && !sfstrtell(fmt))
	{
		state.lsflags |= LS_NOSTAT;
		state.ftsflags |= FTS_NOSTAT|FTS_NOCHDIR;
	}
	if (!sfstrtell(fmt))
	{
		if (state.lsflags & LS_INUMBER)
			sfputr(fmt, "%6(ino)u ", -1);
		if (state.lsflags & LS_BLOCKS)
			sfputr(fmt, "%5(blocks)u ", -1);
		if (state.lsflags & LS_LONG)
		{
			sfputr(fmt, "%(mode)s %3(nlink)u", -1);
			if (!(state.lsflags & LS_NOUSER))
				sfprintf(fmt, " %%-8(uid)%c", (state.lsflags & LS_NUMBER) ? 'd' : 's');
			if (!(state.lsflags & LS_NOGROUP))
				sfprintf(fmt, " %%-8(gid)%c", (state.lsflags & LS_NUMBER) ? 'd' : 's');
			sfputr(fmt, " %8(device:case::%(size)u:*:%(device)s)s", -1);
			sfprintf(fmt, " %%(%s)s ", (state.timeflags & LS_ATIME) ? "atime" : (state.timeflags & LS_CTIME) ? "ctime" : "mtime");
		}
		sfputr(fmt, "%(name)s", -1);
		if (state.lsflags & LS_MARK)
			sfputr(fmt, "%(mark)s", -1);
		else if (state.lsflags & LS_MARKDIR)
			sfputr(fmt, "%(markdir)s", -1);
		if (state.lsflags & LS_LONG)
			sfputr(fmt, "%(linkop:case:?*: %(linkop)s %(linkpath)s)s", -1);
	}
	else
		sfstrseek(fmt, -1, SEEK_CUR);
	if (!(state.format = sfstruse(fmt)))
	{
		error(ERROR_SYSTEM|2, "out of space");
		goto done;
	}
	if (dump)
		sfprintf(sfstdout, "%s\n", state.format);
	else
	{
		stresc(state.format);

		/*
		 * do it
		 */

		if (!argv[0])
		{
			argv = av;
			argv[0] = ".";
			argv[1] = 0;
		}
		if (argv[1])
			state.lsflags |= LS_LABEL;
		walk((char*)argv, ls, state.ftsflags, order, &state);
		if (keys[KEY_summary].macro)
			sfkeyprintf(sfstdout, NiL, keys[KEY_summary].macro, key, NiL);
	}
 done:
	if (fmt)
		sfstrclose(fmt);
	if (state.tmp)
		sfstrclose(state.tmp);
	if (state.vm)
		vmclose(state.vm);
	return error_info.errors != 0;
}
