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
 * AT&T Research
 *
 * df -- free disk block report
 */

static const char usage[] =
"[-?\n@(#)$Id: df (AT&T Research) 2010-08-11 $\n]"
USAGE_LICENSE
"[+NAME?df - summarize disk free space]"
"[+DESCRIPTION?\bdf\b displays the available disk space for the filesystem"
"	of each file argument. If no \afile\a arguments are given then all"
"	mounted filesystems are displayed.]"

"[b:blockbytes?Measure disk usage in 512 byte blocks. This is the default"
"	if \bgetconf CONFORMANCE\b is \bstandard\b.]"
"[D:define?Define \akey\a with optional \avalue\a. \avalue\a will be expanded"
"	when \b%(\b\akey\a\b)\b is specified in \b--format\b. \akey\a may"
"	override internal \b--format\b identifiers.]:[key[=value]]]"
"[e:decimal-scale|thousands?Scale disk usage to powers of 1000.]"
"[f:format?Append to the listing format string. The \bls\b(1), \bpax\b(1) and"
"	\bps\b(1) commands also have \b--format\b options in this same style."
"	\aformat\a follows \bprintf\b(3) conventions, except that \bsfio\b(3)"
"	inline ids are used instead of arguments:"
"	%[#-+]][\awidth\a[.\aprecis\a[.\abase\a]]]]]](\aid\a[:\asubformat\a]])\achar\a."
"	If \b#\b is specified then the internal width and precision are used."
"	If \abase\a is non-zero and \b--posix\b is not on then the field values"
"	are wrapped when they exceed the field width. If \achar\a is \bs\b then"
"	the string form of the item is listed, otherwise the corresponding"
"	numeric form is listed. If \achar\a is \bq\b then the string form of"
"	the item is $'...' quoted if it contains space or non-printing"
"	characters. If \awidth\a is omitted then the default width"
"	is assumed. \asubformat\a overrides the default formatting for \aid\a."
"	Supported \aid\as and \asubformat\as are:]:[format]{\fformats\f}"
"[g:gigabytes?Measure disk usage in 1024M byte blocks.]"
"[h!:header|heading?Display a heading line.]"
"[i:inodes?Display inode usage instead of block usage. There is at least one"
"	inode for each active file and directory on a filesystem.]"
"[k:kilobytes?Measure disk usage in 1024 byte blocks.]"
"[K:scale|binary-scale|human-readable?Scale disk usage to powers of 1024."
"	This is the default if \bgetconf CONFORMANCE\b is not \bstandard\b.]"
"[l:local?List information on local filesystems only, i.e., network"
"	mounts are omitted.]"
"[m:megabytes?Measure disk usage in 1024K byte blocks.]"
"[n:native-block?Measure disk usage in the native filesystem block size."
"	This size may vary between filesystems; it is displayed by the"
"	\bsize\b format identifier.]"
"[O:options?Display the \bmount\b(1) options.]"
"[P:portable?Display each filesystem on one line. By default output is"
"	folded for readability. Also implies \b--blockbytes\b.]"
"[s:sync?Call \bsync\b(2) before querying the filesystems.]"
"[F|t:type?Display all filesystems of type \atype\a. Unknown types are"
"	listed as \blocal\b. Typical (but not supported on all systems) values"
"	are:]:[type]{"
"		[+ufs?default UNIX file system]"
"		[+ext2?default linux file system]"
"		[+xfs?sgi XFS]"
"		[+nfs?network file system version 2]"
"		[+nfs3?network file system version 3]"
"		[+afs3?Andrew file system]"
"		[+proc?process file system]"
"		[+fat?DOS FAT file system]"
"		[+ntfs?nt file system]"
"		[+reg?windows/nt registry file system]"
"		[+lofs?loopback file system for submounts]"
"}"
"[v:verbose?Report all filesystem query errors.]"
"[q|T?Ignored by this implementation.]"

"\n"
"\n[ file ... ]\n"
"\n"
"[+EXAMPLES?The default \b--format\b is"
" \"%#..1(filesystem)s %#(type)s %#(blocks)s %#(used)s %#(available)s  %#(capacity)s  %(mounted)s\"]"
"[+SEE ALSO?\bgetconf\b(1), \bmount\b(1), \bls\b(1), \bpax\b(1), \bps\b(1),"
"	\bmount\b(2)]"
;

#include <ast.h>
#include <error.h>
#include <cdt.h>
#include <ls.h>
#include <mnt.h>
#include <sig.h>
#include <sfdisc.h>

typedef struct				/* df entry			*/
{
	Mnt_t*		mnt;		/* mnt info			*/
	struct statvfs	vfs;		/* statvfs() info		*/
	unsigned long	avail;
	unsigned long	tavail;
	unsigned long	total;
	unsigned long	ttotal;
	unsigned long	used;
	unsigned long	tused;
	unsigned long	itotal;
	unsigned long	iavail;
	unsigned long	iused;
	int		percent;
	int		fraction;
	int		ipercent;
} Df_t;

typedef struct				/* sfkeyprintf() keys		*/
{
	char*		name;		/* key name			*/
	char*		heading;	/* key heading			*/
	char*		description;	/* key description		*/
	short		index;		/* key index			*/
	short		width;		/* default width		*/
	short		abbrev;		/* abbreviation width		*/
	short		disable;	/* macro being expanded		*/
	char*		macro;		/* macro definition		*/
	Dtlink_t	hashed;		/* hash link			*/
} Key_t;

#define KEY_environ		(-1)

#define KEY_available		1
#define KEY_blocks		2
#define KEY_capacity		3
#define KEY_filesystem		4
#define KEY_iavailable		5
#define KEY_icapacity		6
#define KEY_inodes		7
#define KEY_iused		8
#define KEY_mounted		9
#define KEY_native		10
#define KEY_options		11
#define KEY_type		12
#define KEY_used		13

static Key_t	keys[] =
{

	{
		0
	},
	{
		"available",
		"Available",
		"Unused block count.",
		KEY_available,
		0,
		5
	},
	{
		"blocks",
		0,
		"Total block count.",
		KEY_blocks,
		0,
		0
	},
	{
		"capacity",
		"Capacity",
		"Percent of total blocks used.",
		KEY_capacity,
		4,
		3
	},
	{
		"filesystem",
		"Filesystem",
		"Filesystem special device name.",
		KEY_filesystem,
		-19,
		0
	},
	{
		"iavailable",
		"Iavailable",
		"Unused inode count.",
		KEY_iavailable,
		7,
		6
	},
	{
		"icapacity",
		"Icapacity",
		"Percent of total inodes used.",
		KEY_icapacity,
		4,
		4
	},
	{
		"inodes",
		"Inodes",
		"Total inode count.",
		KEY_inodes,
		7,
		3
	},
	{
		"iused",
		"Iused",
		"Used inode count.",
		KEY_iused,
		7,
		3
	},
	{
		"mounted",
		"Mounted on",
		"Mounted on path.",
		KEY_mounted,
		-19,
		5
	},
	{
		"size",
		"Size",
		"Native block size.",
		KEY_native,
		4,
		3
	},
	{
		"options",
		"Options",
		"\bmount\b(1) options.",
		KEY_options,
		-29,
		3
	},
	{
		"type",
		"Type",
		"Filesystem type.",
		KEY_type,
		10,
		3
	},
	{
		"used",
		"Used",
		"Used block count.",
		KEY_used,
		0,
		3
	},

};

static const char	fmt_def[] = "%#..1(filesystem)s %#(type)s %#(blocks)s %#(used)s %#(available)s  %#(capacity)s  %(mounted)s";
static const char	fmt_ino[] = "%#..1(filesystem)s %#(type)s %#(blocks)s %#(available)s %#(capacity)s %#(inodes)s %#(iavailable)s %#(icapacity)s %(mounted)s";
static const char	fmt_opt[] = "%#..1(filesystem)s %#(type)s  %#(options)s %(mounted)s";
static const char	fmt_std[] = "%#..1(filesystem)s %#(blocks)s %#(used)s %#(available)s %8(capacity)s %(mounted)s";

/*
 * man page and header comments notwithstanding
 * some systems insist on reporting block counts in 512 units
 * let me know how you probe for this
 */

#if _SCO_COFF || _SCO_ELF
#define F_FRSIZE(v)	(512)
#else
#if _mem_f_frsize_statvfs
#define F_FRSIZE(v)	((v)->f_frsize?(v)->f_frsize:(v)->f_bsize?(v)->f_bsize:1024)
#else
#define F_FRSIZE(v)	((v)->f_bsize?(v)->f_bsize:1024)
#endif
#endif

#if _mem_f_basetype_statvfs
#define F_BASETYPE(v,p)	((v)->f_basetype)
#else
static char*
basetype(const char* path)
{
	struct stat	st;

	return stat(path, &st) ? "ufs" : fmtfs(&st);
}
#define F_BASETYPE(v,p)	(basetype(p))
#endif

#define REALITY(n)	((((long)(n))<0)?0:(n))
#define UNKNOWN		"local"

#if _lib_sync
extern void	sync(void);
#endif

static struct
{
	int		block;		/* block unit			*/
	int		local;		/* local mounts only		*/
	int		posix;		/* posix format			*/
	int		scale;		/* metric scale power		*/
	int		sync;		/* sync() first			*/
	int		timeout;	/* status() timed out		*/
	int		verbose;	/* verbose message level {-1,1}	*/
	char*		type;		/* type pattern			*/
	Sfio_t*		mac;		/* temporary macro stream	*/
	Sfio_t*		tmp;		/* really temporary stream	*/
	Dt_t*		keys;		/* format key table		*/
	char		buf[1024];	/* format item buffer		*/
} state;

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register int	i;

	if (streq(s, "formats"))
		for (i = 1; i < elementsof(keys); i++)
		{
			sfprintf(sp, "[+%s?%s The title string ", keys[i].name, keys[i].description);
			if (keys[i].heading)
				sfprintf(sp, "is \b%s\b ", keys[i].heading, keys[i].width);
			sfprintf(sp, "and the default width ");
			if (keys[i].width)
				sfprintf(sp, "is %d.]", keys[i].width);
			else
				sfprintf(sp, "%s determined by the \b-b\b, \b-g\b, \b-k\b, \b-m\b and \b-n\b options.]", keys[i].heading ? "is" : "are");
		}
	return 0;
}

/*
 * append format string to fmt
 */

static void
append(Sfio_t* fmt, const char* format)
{
	if (sfstrtell(fmt))
		sfputc(fmt, ' ');
	sfputr(fmt, format, -1);
}

/*
 * scale <m,w,p> into op
 */

static char*
scale(int m, unsigned long w, unsigned long p)
{
	Sfulong_t	n;

	if (state.scale)
	{
		n = w;
		n *= state.block;
		return fmtscale(n, state.scale);
	}
	if (state.block < 1024 * 1024)
		sfsprintf(state.buf, sizeof(state.buf), "%lu", w);
	else if (!m || !w && !p || w > 9)
		sfsprintf(state.buf, sizeof(state.buf), "%lu", w);
	else
		sfsprintf(state.buf, sizeof(state.buf), "%lu.%lu", w, p);
	return state.buf;
}

/*
 * sfkeyprintf() lookup
 * handle==0 for heading
 */

static int
key(void* handle, register Sffmt_t* fp, const char* arg, char** ps, Sflong_t* pn)
{
	register Df_t*		df = (Df_t*)handle;
	register char*		s = 0;
	register Sflong_t	n = 0;
	register Key_t*		kp;
	char*			t;

	if (!fp->t_str)
		return 0;
	if (!(kp = (Key_t*)dtmatch(state.keys, fp->t_str)))
	{
		if (*fp->t_str != '$')
		{
			error(3, "%s: unknown format key", fp->t_str);
			return 0;
		}
		if (!(kp = newof(0, Key_t, 1, strlen(fp->t_str) + 1)))
			error(3, "out of space [key]");
		kp->name = strcpy((char*)(kp + 1), fp->t_str);
		kp->macro = getenv(fp->t_str + 1);
		kp->index = KEY_environ;
		kp->disable = 1;
		dtinsert(state.keys, kp);
	}
	if (kp->macro && !kp->disable)
	{
		kp->disable = 1;
		sfkeyprintf(state.mac, handle, kp->macro, key, NiL);
		if (!(*ps = sfstruse(state.mac)))
			error(ERROR_SYSTEM|3, "out of space");
		kp->disable = 0;
	}
	else if (!df)
	{
		if (fp->base >= 0)
			fp->base = -1;
		s = kp->heading;
		if (fp->flags & SFFMT_ALTER)
		{
			if (!kp->width)
				kp->width = keys[KEY_blocks].width;
			if ((fp->width = kp->width) < 0)
			{
				fp->width = -fp->width;
				fp->flags |= SFFMT_LEFT;
			}
			fp->precis = fp->width;
		}
		if (fp->width > 0 && fp->width < strlen(kp->heading))
		{
			if (fp->width < kp->abbrev)
			{
				fp->width = kp->abbrev;
				if (fp->precis >= 0 && fp->precis < fp->width)
					fp->precis = fp->width;
			}
			if (t = newof(0, char, kp->abbrev, 1))
				s = kp->heading = (char*)memcpy(t, kp->heading, kp->abbrev);
		}
		kp->width = fp->width;
		if (fp->flags & SFFMT_LEFT)
			kp->width = -kp->width;
		fp->fmt = 's';
		*ps = s;
	}
	else
	{
		if ((fp->flags & SFFMT_ALTER) && (fp->width = kp->width) < 0)
		{
			fp->width = -fp->width;
			fp->flags |= SFFMT_LEFT;
		}
		switch (kp->index)
		{
		case KEY_available:
			s = (df->total || df->ttotal) ? scale(df->fraction, df->avail, df->tavail) : "-";
			break;
		case KEY_blocks:
			s = (df->total || df->ttotal) ? scale(df->fraction, df->total, df->ttotal) : "-";
			break;
		case KEY_capacity:
			s = (df->total || df->ttotal) ? (sfsprintf(state.buf, sizeof(state.buf), "%3d%%", df->percent), state.buf) : "-";
			break;
		case KEY_environ:
			if (!(s = kp->macro))
				return 0;
			break;
		case KEY_filesystem:
			if (!(s = df->mnt->fs))
				s = "";
			break;
		case KEY_iavailable:
			if (df->total || df->ttotal)
				n = df->iavail;
			else
				s = "-";
			break;
		case KEY_icapacity:
			s = (df->total || df->ttotal) ? (sfsprintf(state.buf, sizeof(state.buf), "%3d%%", df->ipercent), state.buf) : "-";
			break;
		case KEY_inodes:
			if (df->total || df->ttotal)
				n = df->itotal;
			else
				s = "-";
			break;
		case KEY_iused:
			if (df->total || df->ttotal)
				n = df->iused;
			else
				s = "-";
			break;
		case KEY_mounted:
			if (!(s = df->mnt->dir))
				s = "";
			break;
		case KEY_native:
			if ((n = F_FRSIZE(&df->vfs)) >= 1024)
				sfsprintf(state.buf, sizeof(state.buf), "%I*3dk", sizeof(n), n / 1024);
			else
				sfsprintf(state.buf, sizeof(state.buf), "%I*4d", sizeof(n), n);
			s = state.buf;
			break;
		case KEY_options:
			if (!(s = df->mnt->options))
				s = "";
			break;
		case KEY_type:
			if (!(s = df->mnt->type))
				s = "";
			break;
		case KEY_used:
			s = (df->total || df->ttotal) ? scale(df->fraction, df->used, df->tused) : "-";
			break;
		default:
			return 0;
		}
		if (s)
		{
			if (fp->base >= 0)
			{
				fp->base = -1;
				if (!state.posix && strlen(s) >= fp->width)
				{
					sfprintf(state.tmp, "%s%-*.*s", s, fp->width + 1, fp->width + 1, "\n");
					if (!(s = sfstruse(state.tmp)))
						error(ERROR_SYSTEM|3, "out of space");
				}
			}
			*ps = s;
		}
		else
			*pn = n;
	}
	return 1;
}

/*
 * catch statvfs() timeout
 */

static void
timeout(int sig)
{
	state.timeout = 1;
}

/*
 * statvfs() with timeout
 */

static int
status(const char* path, struct statvfs* vfs)
{
	int		r;

	state.timeout = 0;
	signal(SIGALRM, timeout);
	alarm(4);
	r = statvfs(path, vfs);
	alarm(0);
	signal(SIGALRM, SIG_DFL);
	if (r)
	{
		if (state.timeout)
			error(ERROR_SYSTEM|2, "%s: filesystem stat timed out", path);
		else
			error(ERROR_SYSTEM|2, "%s: cannot stat filesystem", path);
	}
	return r;
}

/*
 * list one entry
 */

static void
entry(Df_t* df, const char* format)
{
	unsigned long	b;
	int		s;

	if ((!state.type || strmatch(df->mnt->type, state.type)) && (!state.local || !(df->mnt->flags & MNT_REMOTE)))
	{
		if (REALITY(df->vfs.f_blocks) == 0)
		{
			df->total = df->avail = df->used = 0;
			df->percent = 0;
		}
		else
		{
			/*
			 * NOTE: on some systems vfs.f_* are unsigned,
			 *	 and on others signed negative values
			 *	 denote error
			 */

			df->total = df->vfs.f_blocks;
			df->used = ((long)df->vfs.f_blocks <= (long)df->vfs.f_bfree) ? 0 : (df->vfs.f_blocks - df->vfs.f_bfree);
			df->avail = ((long)df->vfs.f_bavail < 0) ? 0 : df->vfs.f_bavail;
			df->percent = (df->ttotal = df->avail + df->used) ? (unsigned long)(((double)df->used / (double)df->ttotal + 0.005) * 100.0) : 0;
		}
		df->fraction = 0;
		df->ttotal = df->tavail = df->tused = 0;
		if (state.scale)
			state.block = F_FRSIZE(&df->vfs);
		else if (state.block)
		{
			b = F_FRSIZE(&df->vfs);
			if (b > state.block)
			{
				s = b / state.block;
				df->total *= s;
				df->avail *= s;
				df->used *= s;
			}
			else if (b < state.block)
			{
				s = state.block / b;
				if (df->fraction = s / 10)
				{
					df->ttotal = (df->total / df->fraction) % 10;
					df->tavail = (df->avail / df->fraction) % 10;
					df->tused = (df->used / df->fraction) % 10;
				}
				df->total /= s;
				df->avail /= s;
				df->used /= s;
			}
		}
		df->itotal = REALITY(df->vfs.f_files);
		df->iavail = REALITY(df->vfs.f_ffree);
		if (df->itotal < df->iavail)
			df->iused = 0;
		else
			df->iused = df->itotal - df->iavail;
		df->iavail = REALITY(df->vfs.f_favail);
		df->ipercent = (s = df->iused + df->iavail) ? (unsigned long)(((double)df->iused / (double)s + 0.005) * 100.0) : 0;
		sfkeyprintf(sfstdout, df, format, key, NiL);
	}
}

int
main(int argc, register char** argv)
{
	register int	n;
	int		rem;
	int		head;
	int		i;
	int*		match;
	dev_t		dirdev;
	dev_t		mntdev;
	dev_t*		dev;
	void*		mp;
	Sfio_t*		fmt;
	Key_t*		kp;
	char*		s;
	char*		format;
	struct stat	st;
	struct statvfs	vfs;
	Dtdisc_t	keydisc;
	Optdisc_t	optdisc;
	Mnt_t		mnt;
	Df_t		df;

	error_info.id = "df";
	state.block = -1;
	state.posix = -1;
	state.verbose = -1;
	dev = 0;
	head = 1;

	/*
	 * set up the disciplines
	 */

	optinit(&optdisc, optinfo);
	memset(&keydisc, 0, sizeof(keydisc));
	keydisc.key = offsetof(Key_t, name);
	keydisc.size = -1;
	keydisc.link = offsetof(Key_t, hashed);

	/*
	 * initialize the tables and string streams
	 */

	if (!(fmt = sfstropen()) || !(state.mac = sfstropen()) || !(state.tmp = sfstropen()))
		error(3, "out of space [fmt]");
	if (!(state.keys = dtopen(&keydisc, Dtset)))
		error(3, "out of space [dict]");
	for (n = 1; n < elementsof(keys); n++)
		dtinsert(state.keys, keys + n);

	/*
	 * grab the options
	 */

	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'b':
			state.block = 512;
			continue;
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
					continue;
				if (!(kp = newof(0, Key_t, 1, strlen(opt_info.arg) + 1)))
					error(ERROR_SYSTEM|3, "out of space [macro]");
				kp->name = strcpy((char*)(kp + 1), opt_info.arg);
				dtinsert(state.keys, kp);
			}
			if (kp->macro = s)
				stresc(s);
			continue;
		case 'e':
			state.scale = 1000;
			continue;
		case 'f':
			append(fmt, opt_info.arg);
			continue;
		case 'F':
		case 't':
			state.type = opt_info.arg;
			continue;
		case 'g':
			state.block = 1024 * 1024 * 1024;
			continue;
		case 'h':
			head = opt_info.num;
			continue;
		case 'i':
			append(fmt, fmt_ino);
			continue;
		case 'k':
			state.block = 1024;
			continue;
		case 'K':
			state.scale = 1024;
			continue;
		case 'l':
			state.local = 1;
			continue;
		case 'm':
			state.block = 1024 * 1024;
			continue;
		case 'n':
			state.block = 0;
			continue;
		case 'O':
			append(fmt, fmt_opt);
			continue;
		case 'P':
			state.posix = 1;
			continue;
		case 'q':
			continue;
		case 's':
			state.sync = opt_info.num;
			continue;
		case 'T':
			continue;
		case 'v':
			state.verbose = ERROR_SYSTEM|1;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	argc -= opt_info.index;
	argv += opt_info.index;
	if (!sfstrtell(fmt))
		append(fmt, state.posix > 0 ? fmt_std : fmt_def);
	sfputc(fmt, '\n');
	if (!(format = sfstruse(fmt)))
		error(ERROR_SYSTEM|3, "out of space");
	stresc(format);
	if (state.posix < 0)
		state.posix = !!conformance(0, 0);
	if (state.block < 0)
	{
		if (state.posix)
			state.block = 512;
		else
		{
			state.block = 1024 * 1024;
			if (!state.scale)
				state.scale = 1024;
		}
	}
	s = 0;
	if (state.scale)
	{
		n = 5;
		s = "Size";
	}
	else if (state.block <= 512)
	{
		n = 10;
		if (!state.block)
			s = "blocks";
	}
	else if (state.block <= 1024)
	{
		n = 8;
		if (state.block == 1024)
			s = "Kbytes";
	}
	else if (state.block <= 1024 * 1024)
	{
		n = 6;
		if (state.block == 1024 * 1024)
			s = "Mbytes";
	}
	else if (state.block <= 1024 * 1024 * 1024)
	{
		n = 6;
		if (state.block == 1024 * 1024 * 1024)
			s = "Gbytes";
	}
	if (!s)
	{
		sfprintf(state.mac, "%d-blocks", state.block);
		if (!(s = strdup(sfstruse(state.mac))))
			error(ERROR_SYSTEM|3, "out of space [heading]");
		n = strlen(s);
	}
	keys[KEY_blocks].width = n;
	keys[KEY_blocks].heading = s;
#if _lib_sync
	if (state.sync)
		sync();
#endif
	if (!(mp = mntopen(NiL, "r")))
	{
		error(ERROR_SYSTEM|(argc > 0 ? 1 : 3), "cannot access mount table");
		sfkeyprintf(head ? sfstdout : state.tmp, NiL, format, key, NiL);
		sfstrseek(state.tmp, 0, SEEK_SET);
		df.mnt = &mnt;
		mnt.dir = UNKNOWN;
		mnt.type = 0;
		mnt.flags = 0;
		while (mnt.fs = *argv++)
			if (!status(mnt.fs, &df.vfs))
				entry(&df, format);
	}
	else
	{
		sfkeyprintf(head ? sfstdout : state.tmp, NiL, format, key, NiL);
		sfstrseek(state.tmp, 0, SEEK_SET);
		if (argc)
		{
			rem = argc;
			if (!(dev = newof(0, dev_t, argc, 0)))
				error(ERROR_SYSTEM|3, "out of space [dev_t]");
			for (n = 0; n < argc; n++)
				if (stat(argv[n], &st))
				{
					error(ERROR_SYSTEM|2, "%s: cannot stat", argv[n]);
					argv[n] = 0;
					rem--;
				}
				else
					dev[n] =
#if _mem_st_rdev_stat
					S_ISBLK(st.st_mode) ? st.st_rdev :
#endif
					st.st_dev;
		}
		else
			rem = 0;
		if (rem && (match = newof(0, int, n, 0)))
		{
			/*
			 * scan the mount table (up to 3x) for prefix matches
			 * to avoid the more expensive stat() and statvfs()
			 * calls in the final loop below
			 */

			while (df.mnt = mntread(mp))

				for (n = 0; n < argc; n++)
					if (argv[n] && (i = strlen(df.mnt->dir)) > 1 && i > match[n] && strneq(argv[n], df.mnt->dir, i) && (argv[n][i] == '/' || !argv[n][i]))
						match[n] = i;
			mntclose(mp);
			if (!(mp = mntopen(NiL, "r")))
				error(ERROR_SYSTEM|3, "cannot reopen mount table");
			while (rem && (df.mnt = mntread(mp)))
				for (n = 0; n < argc; n++)
					if (match[n] && (i = strlen(df.mnt->dir)) == match[n] && strneq(argv[n], df.mnt->dir, i) && (argv[n][i] == '/' || !argv[n][i]))
					{
						argv[n][match[n]] = 0;
						if (!status(argv[n], &df.vfs))
						{
							entry(&df, format);
							argv[n] = 0;
							match[n] = 0;
							if (!--rem)
								break;
						}
					}
			free(match);
			if (rem)
			{
				mntclose(mp);
				if (!(mp = mntopen(NiL, "r")))
					error(ERROR_SYSTEM|3, "cannot reopen mount table");
			}
		}
		while ((!argc || rem) && (df.mnt = mntread(mp)))
		{
			if (stat(df.mnt->dir, &st))
			{
				if (errno != ENOENT && errno != ENOTDIR)
					error(state.verbose, "%s: cannot stat", df.mnt->dir);
				continue;
			}
			dirdev = st.st_dev;
			mntdev = (!rem || *df.mnt->fs != '/' || stat(df.mnt->fs, &st)) ? dirdev : st.st_dev;
			if (rem)
			{
				for (n = 0; n < argc; n++)
					if (argv[n] && (dev[n] == dirdev || dev[n] == mntdev))
					{
						argv[n] = 0;
						rem--;
						break;
					}
				if (n >= argc)
					continue;
			}
			if (!status(df.mnt->dir, &df.vfs) || !status(df.mnt->fs, &df.vfs))
				entry(&df, format);
			if (rem)
			{
				while (++n < argc)
					if (dev[n] == dirdev || dev[n] == mntdev)
					{
						argv[n] = 0;
						rem--;
					}
				if (rem <= 0)
					break;
			}
		}
		mntclose(mp);
		if (argc > 0)
		{
			df.mnt = &mnt;
			memset(&mnt, 0, sizeof(mnt));
			for (n = 0; n < argc; n++)
				if ((mnt.dir = argv[n]) && !status(mnt.dir, &vfs))
					entry(&df, format);
		}
	}
	return error_info.errors != 0;
}
