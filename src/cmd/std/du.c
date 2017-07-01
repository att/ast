/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2012 AT&T Intellectual Property          *
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
 * du -- report number of blocks used by . | file ...
 */

static const char usage[] =
"[-?\n@(#)$Id: du (AT&T Research) 2012-01-26 $\n]"
USAGE_LICENSE
"[+NAME?du - summarize disk usage]"
"[+DESCRIPTION?\bdu\b reports the number of blocks contained in all files"
"	and recursively all directories named by the \apath\a arguments."
"	The current directory is used if no \apath\a is given. Usage for"
"	all files and directories is counted, even when the listing is"
"	omitted. Directories and files are only counted once, even if"
"	they appear more than once as a hard link, a target of a"
"	symbolic link, or an operand.]"
"[+?The default block size is 512. The block count includes only the actual"
"	data blocks used by each file and directory, and may not include"
"	other filesystem data required to represent the file. Blocks are"
"	counted only for the first link to a file; subsequent links are"
"	ignored. Partial blocks are rounded up for each file.]"
"[+?If more than one of \b-b\b, \b-h\b, \b-k\b, \b-K,\b or \b-m\b are"
"	specified only the rightmost takes affect.]"

"[a:all?List usage for each file. If neither \b--all\b nor \b--summary\b"
"	is specified then only usage for the \apath\a arguments and"
"	directories is listed.]"
"[b:blocksize?Set the block size to \asize\a. If omitted, \asize\a defaults"
"	to 512.]#?[size:=512]"
"[f:silent?Do not report file and directory access errors.]"
"[h:binary-scale|human-readable?Scale disk usage to powers of 1024.]"
"[K:decimal-scale?Scale disk usage to powers of 1000.]"
"[k:kilobytes?List usage in units of 1024 bytes.]"
"[m:megabytes?List usage in units of 1024K bytes.]"
"[s:summary|summarize?Only display the total for each \apath\a argument.]"
"[t|c:total?Display a grand total for all files and directories.]"
"[v|r:verbose?Report all file and directory access errors. This is the"
"	default.]"
"[x|X|l:xdev|local|mount|one-file-system?Do not descend into directories in"
"	different filesystems than their parents.]"
"[L:logical|follow?Follow symbolic links. The default is \b--physical\b.]"
"[H:metaphysical?Follow command argument symbolic links, otherwise don't"
"	follow. The default is \b--physical\b.]"
"[P:physical?Don't follow symbolic links. The default is \b--physical\b.]"

"\n"
"\n[ path ... ]\n"
"\n"

"[+SEE ALSO?\bfind\b(1), \bls\b(1), \btw\b(1)]"
;

#include <ast.h>
#include <ls.h>
#include <cdt.h>
#include <fts.h>
#include <error.h>

#define BLOCKS(n)	(Count_t)((blocksize==LS_BLOCKSIZE)?(n):(((n)*LS_BLOCKSIZE+blocksize-1)/blocksize))

typedef Sfulong_t Count_t;

typedef struct Fileid_s			/* unique file id		*/
{
	ino_t		ino;
	dev_t		dev;
} Fileid_t;

typedef struct Hit_s			/* file already seen		*/
{
	Dtlink_t	link;		/* dictionary link		*/
	Fileid_t	id;		/* unique file id		*/
} Hit_t;

static void
mark(Dt_t* dict, Hit_t* key, FTSENT* ent)
{
	Hit_t*		hit;

	static int	warned;

	if (hit = newof(0, Hit_t, 1, 0))
	{
		*hit = *key;
		dtinsert(dict, hit);
	}
	else if (!warned)
	{
		warned = 1;
		error(1, "%s: file id dictionary out of space", ent->fts_path);
	}
}

int
main(int argc, register char** argv)
{
	register FTS*		fts;
	register FTSENT*	ent;
	char*			s;
	char*			d;
	Dt_t*			dict;
	Count_t			n;
	Count_t			b;
	int			dirs;
	int			flags;
	int			list;
	int			logical;
	int			multiple;
	struct stat		st;
	Hit_t			hit;
	Dtdisc_t		disc;

	int			all = 0;
	int			silent = 0;
	int			summary = 0;
	int			scale = 0;
	int			total = 0;
	unsigned long		blocksize = 0;
	Count_t			count = 0;

	NoP(argc);
	error_info.id = "du";
	blocksize = 0;
	flags = FTS_PHYSICAL|FTS_NOSEEDOTDIR;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			all = 1;
			continue;
		case 'b':
			blocksize = (opt_info.num <= 0) ? 512 : opt_info.num;
			continue;
		case 'f':
			silent = 1;
			continue;
		case 'h':
			scale = 1024;
			blocksize = 0;
			continue;
		case 'K':
			scale = 1000;
			blocksize = 0;
			continue;
		case 'k':
			blocksize = 1024;
			continue;
		case 'm':
			blocksize = 1024 * 1024;
			continue;
		case 's':
			summary = 1;
			continue;
		case 't':
			total = 1;
			continue;
		case 'x':
			flags |= FTS_XDEV;
			continue;
		case 'v':
			silent = 0;
			continue;
		case 'H':
			flags |= FTS_META|FTS_PHYSICAL;
			continue;
		case 'L':
			flags &= ~(FTS_META|FTS_PHYSICAL);
			continue;
		case 'P':
			flags &= ~FTS_META;
			flags |= FTS_PHYSICAL;
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
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	memset(&hit, 0, sizeof(hit));
	memset(&disc, 0, sizeof(disc));
	disc.key = offsetof(Hit_t, id);
	disc.size = sizeof(Fileid_t);
	if (!(dict = dtopen(&disc, Dtset)))
		error(3, "not enough space for file id dictionary");
	if (blocksize)
		scale = 0;
	else
		blocksize = LS_BLOCKSIZE;
	if (logical = !(flags & (FTS_META|FTS_PHYSICAL)))
		flags |= FTS_PHYSICAL;
	multiple = argv[0] && argv[1];
	dirs = logical || multiple;
	if (!(fts = fts_open(argv, flags, NiL)))
		error(ERROR_system(1), "%s: not found", argv[1]);
	while (ent = fts_read(fts))
	{
		if (ent->fts_info != FTS_DP)
		{
			if (multiple && !ent->fts_level)
			{
				if (s = strrchr(ent->fts_path, '/'))
				{
					*s = 0;
					d = ent->fts_path;
				}
				else
					d = "..";
				if (stat(d, &st))
				{
					error(ERROR_SYSTEM|2, "%s: cannot stat", d);
					continue;
				}
				hit.id.dev = st.st_dev;
				hit.id.ino = st.st_ino;
				if (dtsearch(dict, &hit))
				{
					fts_set(NiL, ent, FTS_SKIP);
					continue;
				}
				if (s)
					*s = '/';
			}
			hit.id.dev = ent->fts_statp->st_dev;
			hit.id.ino = ent->fts_statp->st_ino;
			if (dirs && dtsearch(dict, &hit))
			{
				fts_set(NiL, ent, FTS_SKIP);
				continue;
			}
		}
		list = !summary;
		n = 0;
		switch (ent->fts_info)
		{
		case FTS_NS:
			if (!silent)
				error(ERROR_SYSTEM|2, "%s: not found", ent->fts_path);
			continue;
		case FTS_D:
			if (!(ent->fts_pointer = newof(0, Count_t, 1, 0)))
				error(ERROR_SYSTEM|3, "out of space");
			if (dirs)
				mark(dict, &hit, ent);
			continue;
		case FTS_DC:
			if (!silent)
				error(2, "%s: directory causes cycle", ent->fts_path);
			continue;
		case FTS_DNR:
			if (!silent)
				error(ERROR_SYSTEM|2, "%s: cannot read directory", ent->fts_path);
			break;
		case FTS_DNX:
			if (!silent)
				error(ERROR_SYSTEM|2, "%s: cannot search directory", ent->fts_path);
			fts_set(NiL, ent, FTS_SKIP);
			break;
		case FTS_DP:
			if (ent->fts_pointer)
			{
				n = *(Count_t*)ent->fts_pointer;
				free(ent->fts_pointer);
			}
			break;
		case FTS_SL:
			if (logical)
			{
				fts_set(NiL, ent, FTS_FOLLOW);
				continue;
			}
			/*FALLTHROUGH*/
		default:
			if (ent->fts_statp->st_nlink > 1 || dirs && !ent->fts_level)
				mark(dict, &hit, ent);
			if (!all)
				list = 0;
			break;
		}
		b = iblocks(ent->fts_statp);
		count += b;
		n += b;
		if (ent->fts_parent->fts_pointer)
			*(Count_t*)ent->fts_parent->fts_pointer += n;
		if (!total && (list || ent->fts_level <= 0))
		{
			if (scale)
				sfprintf(sfstdout, "%s\t%s\n", fmtscale((Sfulong_t)n * blocksize, scale), ent->fts_path);
			else
				sfprintf(sfstdout, "%I*u\t%s\n", sizeof(Count_t), BLOCKS(n), ent->fts_path);
		}
	}
	if (total)
	{
		if (scale)
			sfprintf(sfstdout, "%s\n", fmtscale(count * blocksize, scale));
		else
			sfprintf(sfstdout, "%I*u\n", sizeof(Count_t), BLOCKS(count));
	}
	return error_info.errors != 0;
}
