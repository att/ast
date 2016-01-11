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

static const char usage[] =
"[-?\n@(#)$Id: realpath (AT&T Research) 2013-12-06 $\n]"
USAGE_LICENSE
"[+NAME?realpath - print canonicalized path]"
"[+DESCRIPTION?The canoncalized path of each \apath\a operand is printed "
    "as a line on the standard output. Canonicalization: eliminates all "
    "\b.\b and \b..\b path components; reduces multiple adjacent \b/\b "
    "separators to a single separator, with exception that multiple leading "
    "\b/\b separators are reduced to \b//\b. By default a relative \apath\a "
    "(no leading \b/\b) is converted to an absolute path by prepending the "
    "current working directory path before canonicalization. If "
    "\b--logical\b is specified then symbolic links are followed and not "
    "expanded in the output; otherwise all symbolic link components are "
    "recursively expanded (resolved) in the output. If no \apath\a operand "
    "is specified then \b.\b is assumed.]"
"[+?When invoked as \breadlink\b: \b--readlink\b and \b--quiet\b are "
    "set and exactly one \apath\a operand must be specified.]"

"[d:dirfd?Resolve relative paths relative to the open directory "
    "file descriptor \afd\a.]#[fd]"
"[e:canonicalize-existing?Canonicalize: all path components must exist.]"
"[f:canonicalize?Canonicalize: all path components but the last must "
    "exist. This is the default when \b--readlink\b is not specified.]"
"[m:canonicalize-missing?Canonicalize: non-existing path components "
    "silently ignored.]"
"[L:logical?Follow symbolc links and do not expand in the output.]"
"[P:physical?Recursively expand all symbolic link components. This is the "
    "default.]"
"[n!:newline?Print a newline character after the processed path on the "
    "standard output.]"
"[q:quiet?Disable diagnostics on the standard error.]"
"[R:readlink?Each \apath\a operand must be a symbolic link. If any "
    "\b--canonicalize\b* options are specified then the symbolic link text "
    "is canonicalized and printed on the standard output. Otherwise the "
    "unprocessed symbolic link text is printed. This option also sets "
    "\b--quiet\b; to enable diagnostics use \b--readlink --verbose\b.]"
"[r:relative?Canonicalize relative paths; this may result in leading "
    "\b..\b components.]"
#if 0
"[t:relative-to?Print canonicalized \apath\a operands relative to "
    "\arelative-path\a.]:[to-path]"
"[b:relative-base?Print canonicalized \apath\a operands relative to "
    "\ato-path\a.]:[base-path]"
#endif
"[v:verbose?Enable diagnostics on the standard error. This is the default.]"
"[z:zero?Instead of a newline print a \anul\a character (0 byte) after "
    "the processed path on the standard output.]"

"\n"
"\n[ path ... ]\n"
"\n"

"[+SEE ALSO?\breadlink\b(2), \bpathdev\b(3)]"
;

#include <cmd.h>

#define READLINK_COMMAND	1
#define READLINK_OPTION		2

static int
read_real_link_path_canon(int argc, char** argv, Shbltin_t* context, int readlink_mode)
{
	char*			file;
	char*			path;
	ssize_t			n;
	int			r = 0;
	int			dirfd = context ? context->pwdfd : AT_FDCWD;
	int			flags = PATH_ABSOLUTE|PATH_DOTDOT|PATH_PHYSICAL;
	int			separator = '\n';
	bool			oneoperand = !!(readlink_mode & READLINK_COMMAND);
	bool			verbose = !readlink_mode;
	char			linkbuf[4 * PATH_MAX];
	char			canonbuf[4 * PATH_MAX];

	static const char	fail[] = "";

	cmdinit(argc, argv, context, ERROR_CATALOG, 0);
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'd':
			dirfd = (int)opt_info.num;
			continue;
		case 'e':
			flags |= PATH_CANON|PATH_EXISTS;
			flags &= ~PATH_EXCEPT_LAST;
			readlink_mode &= READLINK_OPTION;
			continue;
		case 'f':
			flags |= PATH_CANON|PATH_EXISTS|PATH_EXCEPT_LAST;
			readlink_mode &= READLINK_OPTION;
			continue;
		case 'm':
			flags |= PATH_CANON;
			flags &= ~(PATH_EXISTS|PATH_EXCEPT_LAST);
			readlink_mode &= READLINK_OPTION;
			continue;
		case 'L':
			flags &= ~PATH_PHYSICAL;
			continue;
		case 'P':
			flags |= PATH_PHYSICAL;
			continue;
		case 'n':
			separator = -1;
			continue;
		case 'q':
			verbose = !opt_info.num;
			continue;
		case 'r':
			flags &= ~PATH_ABSOLUTE;
			continue;
		case 'R':
			readlink_mode |= READLINK_OPTION;
			continue;
		case 'v':
			verbose = !!opt_info.num;
			continue;
		case 'z':
			separator = 0;
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
	argc -= opt_info.index;
	argv += opt_info.index;
	if (error_info.errors || oneoperand && (!argv[0] || argv[1]))
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (!(flags & PATH_CANON) && !readlink_mode)
		flags |= PATH_CANON|PATH_EXISTS|PATH_EXCEPT_LAST;
	if (file = *argv)
		argv++;
	else
		file = ".";
	do
	{
		if (readlink_mode)
		{
			if ((n = readlinkat(dirfd, file, linkbuf, sizeof(linkbuf))) >= 0 && n < (sizeof(linkbuf) - 1))
			{
				linkbuf[n] = 0;
				path = linkbuf;
			}
			else
			{
				if (verbose)
					error(ERROR_SYSTEM|2, "%s: cannot read link text", file);
				r = 1;
				path = (char*)fail;
			}
		}
		else
			path = file;
		if ((flags & PATH_CANON) && path != fail)
		{
			if (pathdev(dirfd, path, canonbuf, sizeof(canonbuf), flags, NiL))
				path = canonbuf;
			else
			{
				if (verbose)
					error(ERROR_SYSTEM|2, "%s: canonicalization error%s%s", file, *canonbuf ? " at " : "", canonbuf);
				r = 1;
				path = (char*)fail;
			}
		}
		if (argc > 1 || path != (char*)fail)
			sfputr(sfstdout, path, separator);
	} while (file = *argv++);
	if (sfsync(sfstdout))
	{
		if (verbose)
			error(ERROR_SYSTEM|2, "write error");
		r = 2;
	}
	return r;
}

int
b_readlink(int argc, char** argv, Shbltin_t* context)
{
	return read_real_link_path_canon(argc, argv, context, READLINK_COMMAND);
}

int
b_realpath(int argc, char** argv, Shbltin_t* context)
{
	return read_real_link_path_canon(argc, argv, context, 0);
}
