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
 * <mnt.h> based mount/umount
 */

#include "FEATURE/unmount"

#if _lib_mount && ( _lib_umount || _lib_unmount )

static const char mount_usage[] =
"[-?\n@(#)$Id: mount (AT&T Research) 2011-02-11 $\n]"
USAGE_LICENSE
"[+NAME?mount - mount and display filesystems]"
"[+DESCRIPTION?\bmount\b attaches a named filesystem \afs\a to the"
"	directory \adir\a, which  must already exist. The contents of \adir\a"
"	are hidden until the filesystem is unmounted. The filesystem mount"
"	table is consulted if either of \afs\a or \adir\a are omitted."
"	Information on all mounted filesystems is displayed if both"
"	\afs\a and \adir\a are omitted.]"

"[a:all?Operate on all filesystems in the filesystem table. The \b--host\b"
"	and \b--type\b options can be used to match parts of the table.]"
"[f:show|fake?Display but do not execute the underlying \bmount\b(2)"
"	system calls.]"
"[h:hosts?Limit the filesystem table scan to entries matching the \ahost\a"
"	names. A leading \b!\b inverts the match sense.]:[[!]]host,...]"
"[M:mtab|fstab?Use \afile\a instead of the default filesystem table.]:[file]"
"[b:omit?Omit filesystem table entries matching any of the \atype\a"
"	names.]:[[!]]type,...]"
"[o:options?Specify filesystem specific mount options. Options are a comma"
"	separated list of words preceded by an optional \bno\b to turn the"
"	option off, or \aname=value\a pairs. Multiple \b--options\b are"
"	concatenated with a \bcomma\b separator. See \bfstab\b(4) for"
"	a detailed description of mount options.]:[[no]]name[=value]][,...]]]"
"[p:portable?Print information in \bfstab\b(4) format.]"
"[r:readonly?Mount the filesystems read only. Equivalent to \b--option=ro\b.]"
"[t|T:types?Limit the filesystem table scan to entries matching the \atype\a"
"	names. A leading \b!\b inverts the match sense.]:[[!]]type,...]"
"[u:unmount|umount?Unmount the matched filesystems.]"

"[c:check?Ignored by this implementation.]"
"[m:multiplex|nproc?Distribute multiple mounts across \anproc\a processes."
"	Ignored by this implementation.]"
"[P:prefix?Ignored by this implementation.]:[string]"
"[n!:tab?Ignored by this implementation.]"
"[v:verbose?Ignored by this implementation.]"

"\n"
"\n[ fs [ dir ] ]\n"
"\n"

"[+SEE ALSO?\bdf\b(1), \bumount\b(1), \bmount\b(2), \bfstab\b(4)]"
;

static const char unmount_usage[] =
"[-?\n@(#)$Id: umount (AT&T Research) 1999-11-19 $\n]"
USAGE_LICENSE
"[+NAME?umount - unmount filesystems]"
"[+DESCRIPTION?\bumount\b unmounts one or more currently mounted filesystems,"
"	which can be specified either as mounted-on directories or"
"	filesystems.]"

"[a:all?Operate on all filesystems in the filesystem table. The \b--host\b"
"	and \b--type\b options can be used to match parts of the table.]"
"[f:show|fake?Display but do not execute the underlying \bumount\b(2)"
"	system calls.]"
"[h:hosts?Limit the filesystem table scan to entries matching the \ahost\a"
"	names. A leading \b!\b inverts the match sense.]:[[!]]host,...]"
"[M:mtab|fstab?Use \afile\a instead of the default filesystem table.]:[file]"
"[b:omit?Omit filesystem table entries matching any of the \atype\a"
"	names.]:[[!]]type,...]"
"[t|T:types?Limit the filesystem table scan to entries matching the \atype\a"
"	names. A leading \b!\b inverts the match sense.]:[[!]]type,...]"

"[m:multiplex|nproc?Distribute multiple mounts across \anproc\a processes."
"	Ignored by this implementation.]"
"[P:prefix?Ignored by this implementation.]:[string]"
"[v:verbose?Ignored by this implementation.]"

"\n"
"\n[ fs | dir ]\n"
"\n"

"[+SEE ALSO?\bdf\b(1), \bmount\b(1), \bumount\b(2), \bfstab\b(4)]"
;

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide mount umount unmount
#else
#define mount		______mount
#define umount		______umount
#define unmount		______unmount
#endif

#include <ast.h>
#include <error.h>
#include <cdt.h>
#include <ctype.h>
#include <ls.h>
#include <mnt.h>
#include <ast_fs.h>

#if _sys_mount
#if __bsdi__ || __bsdi || bsdi
#include <sys/param.h>
#endif
#ifndef NGROUPS
#define NGROUPS	NGROUPS_MAX
#endif
#include <sys/mount.h>
#endif

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide mount umount unmount
#else
#undef	mount
#undef	umount
#undef	unmount
#endif

extern int	mount(const char*, const char*, int, const char*, const char*, int);
extern int	umount(const char*, int);
extern int	unmount(const char*);

#if _lib_unmount && !_lib_umount
#define umount(a,b)	unmount(a)
#endif

#ifndef FSTAB
#define FSTAB		"/etc/fstab"
#endif

#ifndef MS_DATA
#define MS_DATA		(1L<<28)
#endif

#define MATCH_HOST	001
#define MATCH_NOHOST	002
#define MATCH_TYPE	004
#define MATCH_NOTYPE	010

typedef int (*Cmp_f)(const char*, const char*);

typedef struct
{
	Dtlink_t	link;
	int		flags;
	char		name[1];
} Match_t;

typedef struct
{
	int		all;
	int		check;
	int		fake;
	int		fstabable;
	int		notab;
	int		nproc;
	int		unmount;
	int		verbose;

	char*		host;
	char*		mtab;
	char*		options;
	char*		prefix;
	char*		type;

	Dt_t*		match;
	Dtdisc_t	matchdisc;
	int		matchflags;

	Sfio_t*		tmp;
} State_t;

static State_t		state;

static int
matchcmp(Dt_t* dt, void* k1, void* k2, Dtdisc_t* disc)
{
	NoP(dt);
	NoP(disc);
	return strcasecmp((char*)k1, (char*)k2);
}

static void
matchset(register char* s, int flags)
{
	register int		c;
	register char*		b;
	register Match_t*	p;

	if (!state.match)
	{
		state.matchdisc.key = offsetof(Match_t, name);
		state.matchdisc.comparf = matchcmp;
		if (!(state.match = dtopen(&state.matchdisc, Dtoset)))
			error(ERROR_SYSTEM|3, "out of space [match table]");
	}
	if (*s == '!')
	{
		s++;
		flags <<= 1;
	}
	state.matchflags |= flags;
	for (;;)
	{
		while (isspace(*s))
			s++;
		for (b = s; (c = *s) && c != ',' && !isspace(c); s++);
		if (s == b)
			break;
		if (c)
			*s = 0;
		if (!(p = (Match_t*)dtmatch(state.match, b)))
		{
			if (!(p = newof(0, Match_t, 1, s - b)))
				error(ERROR_SYSTEM|3, "out of space [match entry]");
			memcpy(p->name, b, s - b);
			dtinsert(state.match, p);
		}
		p->flags |= flags;
		if (!c)
			break;
		*s++ = c;
	}
}

static int
matchhost(char* s)
{
	char*		t;
	char*		u;
	char*		v;
	Match_t*	p;

	if (!state.match || !(state.matchflags & (MATCH_HOST|MATCH_NOHOST)))
		return 1;
	if (!(t = strchr(s, ':')))
		return 0;
	*t = 0;
	if (!(p = (Match_t*)dtmatch(state.match, s)))
		for (u = s; v = strchr(u, '.'); u = v)
		{
			*v = 0;
			p = (Match_t*)dtmatch(state.match, s);
			*v++ = '.';
			if (p)
				break;
			u = v;
		}
	*t = ':';
	if (state.matchflags & MATCH_HOST)
		return p && (p->flags & MATCH_HOST);
	return !p || !(p->flags & MATCH_NOHOST);
}

static int
matchtype(char* s)
{
	Match_t*	p;

	if (!state.match || !(state.matchflags & (MATCH_TYPE|MATCH_NOTYPE)))
		return 1;
	p = (Match_t*)dtmatch(state.match, s);
	if (state.matchflags & MATCH_TYPE)
		return p && (p->flags & MATCH_TYPE);
	return !p || !(p->flags & MATCH_NOTYPE);
}

static void
mountop(register Mnt_t* mnt, char* options)
{
	char*	s;
	int	n;

	if (state.unmount)
	{
		if (state.fake)
			sfprintf(sfstderr, "umount(%s)\n", mnt->fs);
		else if (umount(mnt->fs, 0))
			error(ERROR_SYSTEM|2, "%s: cannot unmount", mnt->fs);
	}
	else
	{
		n = (s = options ? options : mnt->options) ? strlen(s) : 0;
		if (state.fake)
			sfprintf(sfstderr, "mount(\"%s\",\"%s\",0x%08x,\"%s\",\"%s\",%d)\n", mnt->fs, mnt->dir, mnt->flags|MS_DATA, mnt->type, s, n);
		else if (mount(mnt->fs, mnt->dir, mnt->flags|MS_DATA, mnt->type, s, n))
			error(ERROR_SYSTEM|2, "%s: cannot mount", mnt->fs);
	}
}

int
main(int argc, register char** argv)
{
	register char*	s;
	register Mnt_t*	mnt;
	const char*	usage;
	void*		mp;
	char*		p;
	int		trydefault;
	Cmp_f		cmp;
	Mnt_t		ent;

	NoP(argc);
	if (s = strrchr(*argv, '/'))
		s++;
	else
		s = *argv;
	error_info.id = s;
	usage = (state.unmount = strmatch(s, "*u?(n)mount")) ? unmount_usage : mount_usage;
	state.matchflags = 0;
	if (!(state.tmp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [tmp string stream]");
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			state.all = 1;
			continue;
		case 'b':
			matchset(opt_info.arg, MATCH_NOTYPE);
			continue;
		case 'c':
			state.check = 1;
			continue;
		case 'f':
			state.fake = 1;
			continue;
		case 'h':
			matchset(opt_info.arg, MATCH_HOST);
			continue;
		case 'm':
			state.nproc = opt_info.num;
			continue;
		case 'n':
			state.notab = 1;
			continue;
		case 'o':
			if (sfstrtell(state.tmp))
				sfputc(state.tmp, ',');
			sfputr(state.tmp, opt_info.arg, -1);
			continue;
		case 'p':
			state.fstabable = 1;
			continue;
		case 'r':
			if (sfstrtell(state.tmp))
				sfputc(state.tmp, ',');
			sfputr(state.tmp, "ro", -1);
			continue;
		case 't':
		case 'T':
			state.type = opt_info.arg;
			matchset(state.type, MATCH_TYPE);
			continue;
		case 'u':
			state.unmount = 1;
			continue;
		case 'v':
			state.verbose = 1;
			continue;
		case 'M':
			state.mtab = opt_info.arg;
			continue;
		case 'P':
			state.prefix = opt_info.arg;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		case 0:
			break;
		default:
			error(2, "%s: option not implemented", opt_info.option);
			break;
		}
		break;
	}
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	trydefault = !state.mtab;
	argv += opt_info.index;
	if ((s = *argv) && !*++argv && !state.mtab || state.mtab && (!*state.mtab || streq(state.mtab, "-")) || !state.mtab && (state.all || state.match))
		state.mtab = FSTAB;
	if (state.unmount && (!s || *argv))
		error(ERROR_SYSTEM|3, "one argument expected");
	if (!(mp = mntopen(state.mtab, "r")) && (!trydefault || !(mp = mntopen(state.mtab = 0, "r"))))
	{
		if (state.mtab && !trydefault)
			error(ERROR_SYSTEM|3, "%s: cannot open fs table", state.mtab);
		else
			error(ERROR_SYSTEM|3, "cannot open default mount table");
	}
	if (sfstrtell(state.tmp) && !(state.options = strdup(sfstruse(state.tmp))))
		error(ERROR_SYSTEM|3, "out of space [option string]");
	if (!s)
	{
		if (state.all || state.match)
		{
			while (mnt = mntread(mp))
				if (matchhost(mnt->fs) && matchtype(mnt->type))
					mountop(mnt, state.options);
		}
		else if (state.fstabable)
		{
			while (mnt = mntread(mp))
				sfprintf(sfstdout, "%s %s %s %s %d %d\n",
					mnt->fs,
					mnt->dir,
					mnt->type,
					mnt->options,
					mnt->freq,
					mnt->npass);
		}
		else
		{
			while (mnt = mntread(mp))
				sfprintf(sfstdout, "%s on %s type %s (%s)\n",
					mnt->fs,
					mnt->dir,
					mnt->type,
					mnt->options);
		}
	}
	else if (!*argv)
	{
		for (;;)
		{
			while (mnt = mntread(mp))
			{
				cmp = (Cmp_f)strcmp;
				if (p = mnt->options)
					while (*p)
					{
						if (*p == 'i' && (!memcmp(p, "ic", 2) || !memcmp(p, "icase", 5) || !memcmp(p, "ignorecase", 10)))
						{
							cmp = (Cmp_f)strcasecmp;
							break;
						}
						while (*p && *p++ != ',');
					}
				if (!(*cmp)(mnt->fs, s) || !(*cmp)(mnt->dir, s))
				{
					mountop(mnt, state.options);
					break;
				}
			}
			if (!mnt)
			{
				if (trydefault && state.mtab)
				{
					trydefault = 0;
					mntclose(mp);
					if (mp = mntopen(NiL, "r"))
						continue;
				}
				error(3, "%s: file system or mount point not found", s);
			}
			break;
		}
	}
	else if (!*(argv + 1))
	{
		mnt = &ent;
		memset(mnt, 0, sizeof(*mnt));
		mnt->fs = s;
		mnt->dir = *argv;
		mnt->type = state.type;
		mountop(mnt, state.options);
	}
	else
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	mntclose(mp);
	exit(0);
}

#else

int
main(int argc, register char** argv)
{
	register char*	s;

	if (s = strrchr(*argv, '/'))
		s++;
	else
		s = *argv;
	execv(*s == 'u' ? "/etc/umount" : "/etc/mount", argv);
	exit(EXIT_NOTFOUND);
}

#endif
