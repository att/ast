/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2012 AT&T Intellectual Property          *
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

static const char usage[] =
"[-?\n@(#)$Id: mktemp (AT&T Research) 2012-12-12 $\n]"
USAGE_LICENSE
"[+NAME?mktemp - make temporary file or directory]"
"[+DESCRIPTION?\bmktemp\b creates a temporary file with optional base "
    "name prefix \aprefix\a. If \aprefix\a is omitted then \btmp\b is used "
    "and \b--tmp\b is implied. A consecutive string of trailing X's in "
    "\aprefix\a is replaced by a pseudorandom combination of [0-9a-zA-Z]]"
    "characters, otherwise the first 5 characters of \aprefix\a is catenated "
    "with a pseudorandom string to construct a file name component of 14 "
    "characters. If \adirectory\a is specified or if \aprefix\a contains a "
    "directory prefix then that directory overrides any of the directories "
    "described below. A temporary file will have mode \brw-------\b and a "
    "temporary directory will have mode \brwx------\b, subject to "
    "\bumask\b(1). Generated paths have these attributes:]"
    "{"
        "[+*?Lower case to avoid clashes on case ignorant filesystems.]"
        "[+*?Pseudo-random part to deter denial of service attacks.]"
        "[+*?Default pseudo-random part (no specific \bX...\b template) "
            "formatted to accomodate 8.3 filesystems.]"
    "}"
"[+?A consecutive trailing sequence of \bX\b's in \aprefix\a is replaced "
    "by the pseudo-random part. If there are no \bX\b's then the "
    "pseudo-random part is appended to the prefix.]"
"[d:directory?Create a directory instead of a regular file.]"
"[m:mode?Set the mode of the created temporary to \amode\a. "
    "\amode\a is symbolic or octal mode as in \bchmod\b(1). Relative modes "
    "assume an initial mode of \bu=rwx\b.]:[mode]"
"[p:default?Use \adirectory\a if the \bTMPDIR\b environment variable is "
    "not defined. Implies \b--tmp\b.]:[directory]"
"[q:quiet?Suppress file and directory error diagnostics.]"
"[R:regress?The pseudo random generator is seeded with \aseed\a instead "
    "of process/system specific transient data. Use for testing "
    "only. A seed of \b0\b is silently changed to \b1\b.]#[seed]"
"[t:tmp|temporary-directory?Create a path rooted in a temporary "
    "directory.]"
"[u:unsafe|dry-run?Check for file/directory existence but do not create. "
    "Use this for testing only.]"
"\n"
"\n[ prefix [ directory ] ]\n"
"\n"
"[+SEE ALSO?\bmkdir\b(1), \bpathtemp\b(3), \bmktemp\b(3)]"
;

#include <cmd.h>
#include <ls.h>

int
b_mktemp(int argc, char** argv, Shbltin_t* context)
{
	mode_t		mode = 0;
	mode_t		mask;
	int		fd;
	int		i;
	int		directory = 0;
	int		list = 1;
	int		quiet = 0;
	int		unsafe = 0;
	int*		fdp = &fd;
	char*		dir = "";
	char*		pfx;
	char*		t;
	char		path[PATH_MAX];

	cmdinit(argc, argv, context, ERROR_CATALOG, ERROR_NOTIFY);
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'd':
			directory = 1;
			continue;
		case 'm':
			mode = strperm(pfx = opt_info.arg, &opt_info.arg, S_IRWXU);
			if (*opt_info.arg)
				error(ERROR_exit(0), "%s: invalid mode", pfx);
			continue;
		case 'p':
			if ((t = getenv("TMPDIR")) && *t)
				dir = 0;
			else
				dir = opt_info.arg;
			continue;
		case 'q':
			quiet = 1;
			continue;
		case 't':
			dir = 0;
			continue;
		case 'u':
			unsafe = 1;
			fdp = 0;
			continue;
		case 'R':
			if (!pathtemp(NiL, 0, opt_info.arg, "/seed", NiL))
				error(2, "%s: regression test initializtion failed", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || (pfx = *argv) && *++argv && *(argv + 1))
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (!pfx)
	{
		pfx = "tmp";
		if (dir && !*dir)
			dir = 0;
	}
	if (*argv)
	{
		dir = *argv;
		if (*pfx == '/')
		{
			fdp = 0;
			list = 0;
		}
	}
	else if (t = strrchr(pfx, '/'))
	{
		i = ++t - pfx;
		dir = fmtbuf(i);
		memcpy(dir, pfx, i);
		dir[i] = 0;
		pfx = t;
	}
	if (directory)
	{
		i = strlen(pfx);
		t = pfx;
		pfx = fmtbuf(i + 2);
		memcpy(pfx, t, i);
		pfx[i] = '/';
		pfx[i+1] = 0;
	}
	if (fdp)
	{
		mask = umask(0);
		if (!mode)
			mode = (fdp ? (S_IRUSR|S_IWUSR) : S_IRWXU) & ~mask;
		if (directory)
			mode |= S_IXUSR;
		umask(~mode & (S_IRWXU|S_IRWXG|S_IRWXO));
	}
	if (pathtemp(path, sizeof(path), dir, pfx, fdp))
	{
		if (fdp)
			close(*fdp);
		if (list)
			sfputr(sfstdout, path, '\n');
	}
	else if (quiet)
		error_info.errors++;
	else
		error(ERROR_SYSTEM|2, "cannot create temporary path");
	if (fdp)
		umask(mask);
	return error_info.errors != 0;
}
