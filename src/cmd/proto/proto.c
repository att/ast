/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2012 AT&T Intellectual Property          *
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
 * proto - make prototyped C compatible with K&R, ANSI, C++
 *
 * output to stdout unless -r
 *
 * NOTE: coded for minimal library support
 */

#if !PROTO_STANDALONE

static const char usage[] =
"[-?\n@(#)$Id: proto (AT&T Research) 2012-02-20 $\n]"
USAGE_LICENSE
"[+NAME?proto - make prototyped C source compatible with K&R, ANSI and C++]"
"[+DESCRIPTION?\bproto\b converts ANSI C prototype constructs in \afile\a"
"	to constructs compatible with K&R C, ANSI C, and C++. Only files"
"	with the line \b#pragma prototyped\b appearing in one of the first"
"	64 lines are processed; other files are silently ignored. Output is"
"	written to the standard output unless the \b--replace\b option is"
"	specified, in which case \afile\a is modified in-place.]"

"[c:comment?\ab[m[e]]]]\a are the beginning, middle, and end comment"
"	characters. If \ae\a is omitted then it defaults to \ab\a. If \am\a is"
"	omitted then it defaults to \ab\a. Use \"\b/*\b\" for C comments,"
"	\"\b#\b\" for shell, and \"\b(*)\b\" for pascal. If \abme\a is \"\""
"	(the empty string) then the comment style is determined from the"
"	input \afile\a suffix; no notice is prepended if the comment"
"	style cannot be determined.]:[bme:=\"/*\"]"
"[d:disable?Disable prototype conversion but still emit the identification"
"	comment.]"
"[e:externs?All \bextern\b references are for \apackage\a. Some systems"
"	require special attributes for imported and exported dll data symbols."
"	If \b_BLD_\b\apackage\a is not defined then \bextern\b data references"
"	will be assigned the dll import attribute when supported by the local"
"	compiler.]:[package]"
"[f:force?Force conversion for files that do not contain"
"	\b#pragma prototyped\b.]"
"[h!:header?Emit the \bproto\b header definition preamble.]"
"[i:inverse?Specifies inverse proto: classic function definitions are"
"	converted to ANSI prototype form and non-ANSI directives are"
"	commented out. In this case files with the line"
"	\b#pragma noprototyped\b within the first 64 lines are silently"
"	ignored. If \b--header\b is also specified then only \bextern\b"
"	prototypes are emitted for each non-static function. This option"
"	requires that all classic function formal arguments be declared,"
"	i.e., omitted declarations for \bint\b formals will not generate"
"	the correct prototype. Typical usage would be to first run"
"	\bproto --inverse --header *.c\b to generate the external prototypes"
"	that should be placed in a \b.h\b file shared by \b*.c\b and then"
"	\bproto --inverse --replace *.c\b to convert the function prototypes"
"	in place. Note that prototype code conditioned on \b__STDC__\b will"
"	most likely confuse \b--inverse\b.]"
"[l:license?Generate a license notice comment at the top of the output"
"	controlled by one or more \aname\a=\avalue\a pairs in \afile\a."
"	If a \avalue\a contains space characters then it must be enclosed in"
"	either single or double quotes. \aname\a may"
"	be:]:[file]{"
"		[+type?The license type:]{"
"			[+bsd?The BSD open source license.]"
"			[+cpl?The Common Public License.]"
"			[+epl?The Eclipse Public License.]"
"			[+gpl?The GNU Public License.]"
"			[+mit?The MIT open source license.]"
"			[+inline?License text already in source.]"
"			[+none?No license.]"
"			[+noncommercial?Non-commercial use.]"
"			[+nonexclusive?Single person non-commercial use.]"
"			[+open?Open source.]"
"			[+proprietary?Only by individual agreement.]"
"			[+special?License text in \bnotice\b.]"
"			[+usage?License specific \boptget\b(3) usage strings.]"
"			[+verbose?Include the disclaimer notice if any.]"
"			[+zlib?The ZLIB open source license.]"
"		}"
"		[+author?A \b,\b separated list of \aname\a <\aemail\a> pairs.]"
"		[+class?The license class. The default values are:]{"
"			[+gpl?The GNU Public License.]"
"			[+open?\bwww.opensource.org\b sanctioned open source.]"
"			[+proprietary?Internal or non-disclosure use only.]"
"			[+special?Nonstandard license text and notices.]"
"		}"
"		[+corporation?Within the parent, e.g., \bAT&T\b.]"
"		[+company?Within the corporation, e.g., \bResearch\b.]"
"		[+location?Company location.]"
"		[+organization?Within the company, e.g., \bNetwork Services"
"			Research Department\b.]"
"		[+notice?\btype=special\b notice text with embedded newlines"
"			or additional notice text listed by \btype=verbose\b.]"
"		[+package?The generic software package name, e.g., \bast\b.]"
"		[+parent?They own it all, e.g., \bAT&T\b.]"
"		[+since?The year the software was first released.]"
"		[+source?The year the input files was modified.]"
"		[+start?The year the license was first applied to the software.]"
"		[+url?The URL of the detailed license text, e.g.,"
"		\bhttp://www.research.att.com/sw/license/open-ast.html\b.]"
"		[+urlmd5?The \bmd5sum\b(1) of the downloaded URL data. Note that"
"			the downloaded data may have embedded \b\\r\b not present"
"			in the original source.]"
"		[+query=\atext\a?If \atext\a is one of the \aname\as above"
"			then its value is expanded, otherwise"
"			\b${\b\aname\a\b}\b in \atext\a is expanded. The"
"			result is printed as a line on the standard output"
"			and \bproto\b exits. This should only be used via"
"			\b--options\b.]"
"}"
"[n:sync?Output C style line syncs to retain the original line structure.]"
"[o:options?Additional space or \b,\b separated \aname=value\a \b--license\b"
"	options.]:[name=value,...]"
"[p:pass?Pass input to output, even if not converted.]"
"[r:replace?Process the input files in place; original information is"
"	replaced by \bproto\b output.]"
"[s:include?Output \b#include <prototyped.h>\b rather than expanding the"
"	equivalent inline.]"
"[t:test?Enable test code. Use at your own risk.]"
"[v:verbose?List each file as it is processed.]"
"[x:externalize?Convert \bstatic\b function attributes to \bextern\b.  Used"
"	for analysis programs that only instrument \bextern\b functions.]"
"[z:zap?Disable conversion and remove \b#pragma prototyped\b.]"
"[C:copy?Convert each input \afile\a to \adirectory\a/\afile\a,"
"	making intermediate directories as necessary.]:[directory]"
"[L:list?Input file names are read one per line from \afile\a.]:[file]"
"[P|+:plusplus?Convert \bextern()\b to \bextern(...)\b.]"
"[S:shell?Equivalent to \b--comment=\"#\".]"

"\n"
"\nfile ...\n"
"\n"

"[+SEE ALSO?\bcc\b(1), \bcpp\b(1)]"
;

#include <ast.h>
#include <error.h>
#include <times.h>

#define utime(p,t)	touch(p,t[0],t[1],0)

#endif

#define PROTOMAIN	1

/*DELAY_CONTROL*/

#ifndef __STDC__
#ifndef creat
#define creat		_huh_creat
#endif
#if PROTO_STANDALONE
#ifndef access
#define access		_huh_access
#endif
#ifndef ctime
#define ctime		_huh_ctime
#endif
#ifndef mkdir
#define mkdir		_huh_mkdir
#endif
#endif
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if PROTO_STANDALONE
#include <stdio.h>
#else
#include <time.h>
#endif

#ifndef __STDC__
#undef	access
#undef	ctime
#undef	creat
#undef	mkdir
#endif

#ifndef O_RDONLY
#define O_RDONLY	0
#endif

#ifndef S_IRUSR
#define S_IRUSR		0400
#endif
#ifndef S_IWUSR
#define S_IWUSR		0200
#endif
#ifndef S_IXUSR
#define S_IXUSR		0100
#endif
#ifndef S_IRGRP
#define S_IRGRP		0040
#endif
#ifndef S_IWGRP
#define S_IWGRP		0020
#endif
#ifndef S_IXGRP
#define S_IXGRP		0010
#endif
#ifndef S_IROTH
#define S_IROTH		0004
#endif
#ifndef S_IWOTH
#define S_IWOTH		0002
#endif
#ifndef S_IXOTH
#define S_IXOTH		0001
#endif

#ifndef __STDC__
#if !_WIN32 && !_WINIX
#define remove(x)	unlink(x)
#define rename(x,y)	((link(x,y)||remove(x))?-1:0)
#endif

#if PROTO_STANDALONE
extern int	access(const char*, int);
extern int	mkdir(const char*, int);
#endif

#endif

#if PROTO_STANDALONE
extern int	utime(const char*, time_t*);
#endif

/*
 * rename newfile to oldfile
 * preserve!=0 preserves atime,mtime
 */

int
replace(const char* newfile, const char* oldfile, int preserve)
{
	struct stat	st;
	time_t		ut[2];

	if (stat(oldfile, &st))
	{
		if (preserve)
			return -1;
		st.st_mode = 0;
	}
	if (remove(oldfile) || rename(newfile, oldfile))
		return -1;
	if (st.st_mode &= (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH))
		chmod(oldfile, st.st_mode);
	if (preserve)
	{
		ut[0] = st.st_atime;
		ut[1] = st.st_mtime;
		preserve = utime(oldfile, ut);
	}
	return preserve;
}

#undef	utime
#define utime		______utime

/*NODELAY_CONTROL*/

#include "ppproto.c"

#define PROTO_ERROR	(PROTO_USER<<0)
#define PROTO_REPLACE	(PROTO_USER<<1)
#define PROTO_VERBOSE	(PROTO_USER<<2)

static int
proto(char* file, char* license, char* options, char* package, char* copy, char* comment, int flags)
{
	char*		b;
	char*		e;
	char*		p;
	int		n;
	int		m;
	int		x;
	int		fd;
	char		buf[1024];

	if (file && access(file, 4))
		proto_error(NiL, 2, file, "not found");
	else if (b = pppopen(file, 0, license, options, package, comment, flags))
	{
		if (!file)
			fd = 1;
		else if (flags & PROTO_REPLACE)
		{
			e = file + strlen(file) - 1;
			x = *e;
			*e = '_';
			if ((fd = creat(file, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0)
			{
				proto_error(b, 2, file, "cannot create temporary file");
				pppclose(b);
				return flags | PROTO_ERROR;
			}
			*e = x;
		}
		else if (copy)
		{
			if (((n = strlen(copy)) + strlen(file) + 2) > sizeof(buf))
			{
				proto_error(b, 2, copy, "copy path too long");
				pppclose(b);
				return flags | PROTO_ERROR;
			}
			strcpy(buf, copy);
			e = buf + n;
			if (*file != '/')
				*e++ = '/';
			strcpy(e, file);
			if ((fd = creat(buf, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0)
			{
				for (e = buf; *e == '/'; e++);
				do
				{
					if (*e == '/')
					{
						*e = 0;
						if (access(buf, 0) && mkdir(buf, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH))
						{
							proto_error(b, 2, buf, "cannot create copy directory");
							pppclose(b);
							return flags | PROTO_ERROR;
						}
						*e = '/';
					}
				} while (*e++);
				if ((fd = creat(buf, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0)
				{
					proto_error(b, 2, buf, "cannot create copy file");
					pppclose(b);
					return flags | PROTO_ERROR;
				}
			}
			file = buf;
		}
		else
			fd = 1;
		if (file && (flags & PROTO_VERBOSE))
			proto_error(b, 0, "convert to", file);
		while ((n = pppread(b)) > 0)
		{
			p = b;
			for (;;)
			{
				if ((m = write(fd, p, n)) <= 0)
				{
					proto_error(b, 2, "write error", NiL);
					flags |= PROTO_ERROR;
					break;
				}
				if ((n -= m) <= 0)
					break;
				p += m;
			}
			if (m < 0)
				break;
		}
		if (fd > 1)
			close(fd);
		if (file && (flags & PROTO_REPLACE))
		{
			*e = '_';
			strcpy(b, file);
			*e = x;
			if (replace(b, file, !(flags & PROTO_CLASSIC)))
				proto_error(b, 2, "cannot rename to", file);
		}
		pppclose(b);
	}
	return flags;
}

#if !PROTO_STANDALONE
#undef	error
#endif

typedef struct Sufcom_s
{
	char		suffix[4];
	char		comment[4];
} Sufcom_t;

static const Sufcom_t	sufcom[] =
{
	"c",		"/*",
	"cpp",		"/*",
	"cxx",		"/*",
	"c++",		"/*",
	"C",		"/*",
	"CPP",		"/*",
	"CXX",		"/*",
	"C++",		"/*",
	"f",		"C",
	"F",		"C",
	"h",		"/*",
	"hpp",		"/*",
	"hxx",		"/*",
	"H",		"/*",
	"HPP",		"/*",
	"HXX",		"/*",
	"ksh",		"#",
	"KSH",		"#",
	"l",		"/*",
	"L",		"/*",
	"p",		"(*)",
	"pas",		"(*)",
	"P",		"(*)",
	"PAS",		"(*)",
	"pl",		"#",
	"PL",		"#",
	"pl1",		"/*",
	"pli",		"/*",
	"PL1",		"/*",
	"PLI",		"/*",
	"sh",		"#",
	"SH",		"#",
	"sml",		"(*)",
	"SML",		"(*)",
	"y",		"/*",
	"Y",		"/*",
};

/*
 * if !comment || !*comment then return a value compatible with file
 */

static char*
type(register char* file, char* comment)
{
	register char*	suffix;
	register int	i;

	if (file && (!comment || !*comment))
	{
		suffix = 0;
		while (*file)
			if (*file++ == '.')
				suffix = file;
		if (suffix && strlen(suffix) <= 3)
			for (i = 0; i < sizeof(sufcom) / sizeof(sufcom[0]); i++)
				if (!strcmp(suffix, sufcom[i].suffix))
					return (char*)sufcom[i].comment;
	}
	return comment;
}

int
main(int argc, char** argv)
{
	char*		b;
	char*		file;
	int		fd;
	int		n;
	char*		op;
	char*		oe;
	char*		comment = 0;
	char*		copy = 0;
	char*		list = 0;
	char*		license = 0;
	char*		options = 0;
	char*		package = 0;
	int		flags = PROTO_HEADER;
	char		buf[1024];
	char		opt[4 * 1024];

	NoP(argc);
#if PROTO_STANDALONE
	while ((file = *++argv) && *file == '-' && *(file + 1))
	{
		for (;;)
		{
			switch (*++file)
			{
			case 0:
				break;
			case 'c':
				if (!*(comment = ++file))
					comment = *++argv;
				break;
			case 'd':
				flags |= PROTO_DISABLE;
				continue;
			case 'e':
				if (!*(package = ++file) && !(package = *++argv))
				{
					file = "??";
					continue;
				}
				break;
			case 'f':
				flags |= PROTO_FORCE;
				continue;
			case 'h':
				flags &= ~PROTO_HEADER;
				continue;
			case 'i':
				flags |= PROTO_CLASSIC;
				continue;
			case 'l':
				if (!*(license = ++file) && !(license = *++argv))
				{
					file = "??";
					continue;
				}
				break;
			case 'n':
				flags |= PROTO_LINESYNC;
				continue;
			case 'o':
				if (!*(b = ++file) && !(b = *++argv))
				{
					file = "??";
					continue;
				}
				if (!options)
				{
					options = op = opt;
					oe = op + sizeof(opt) - 1;
				}
				n = strlen(b);
				if ((n + 1) >= (oe - op))
					proto_error(NiL, 3, b, "too many options");
				else
				{
					*op++ = '\n';
					memcpy(op, b, n + 1);
					op += n;
				}
				break;
			case 'p':
				flags |= PROTO_PASS;
				continue;
			case 'r':
				flags |= PROTO_REPLACE;
				continue;
			case 's':
				flags |= PROTO_INCLUDE;
				continue;
			case 't':
				flags |= PROTO_TEST;
				continue;
			case 'v':
				flags |= PROTO_VERBOSE;
				continue;
			case 'x':
				flags |= PROTO_EXTERNALIZE;
				continue;
			case 'z':
				flags |= PROTO_DISABLE|PROTO_NOPRAGMA;
				continue;
			case 'C':
				if (!*(copy = ++file) && !(copy = *++argv))
				{
					file = "??";
					continue;
				}
				break;
			case 'L':
				if (!*(list = ++file) && !(list = *++argv))
				{
					file = "??";
					continue;
				}
				break;
			case 'P':
			case '+':
				flags |= PROTO_PLUSPLUS;
				continue;
			case 'S':
				comment = "#";
				continue;
			default:
				proto_error(NiL, 2, file, "unknown option");
				/*FALLTHROUGH*/
			case '?':
				b = "Usage: proto [-dfhinprstvzP+S] [-C directory] [-e package] [-l file]\n             [-o \"name='value' ...\"] [-L file] file ...\n";
				write(2, b, strlen(b));
				return 2;
			}
			break;
		}
	}
#else
	error_info.id = "proto";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'c':
			comment = opt_info.arg;
			continue;
		case 'd':
			flags |= PROTO_DISABLE;
			continue;
		case 'e':
			package = opt_info.arg;
			continue;
		case 'f':
			flags |= PROTO_FORCE;
			continue;
		case 'h':
			flags &= ~PROTO_HEADER;
			continue;
		case 'i':
			flags |= PROTO_CLASSIC;
			continue;
		case 'l':
			license = opt_info.arg;
			continue;
		case 'n':
			flags |= PROTO_LINESYNC;
			continue;
		case 'o':
			if (!options)
			{
				options = op = opt;
				oe = op + sizeof(opt) - 1;
			}
			n = strlen(opt_info.arg);
			if ((n + 1) >= (oe - op))
				error(3, "%s: too many options", opt_info.arg);
			else
			{
				*op++ = '\n';
				memcpy(op, opt_info.arg, n + 1);
				op += n;
			}
			continue;
		case 'p':
			flags |= PROTO_PASS;
			continue;
		case 'r':
			flags |= PROTO_REPLACE;
			continue;
		case 's':
			flags |= PROTO_INCLUDE;
			continue;
		case 't':
			flags |= PROTO_TEST;
			continue;
		case 'v':
			flags |= PROTO_VERBOSE;
			continue;
		case 'x':
			flags |= PROTO_EXTERNALIZE;
			continue;
		case 'z':
			flags |= PROTO_DISABLE|PROTO_NOPRAGMA;
			continue;
		case 'C':
			copy = opt_info.arg;
			continue;
		case 'L':
			list = opt_info.arg;
			continue;
		case 'P':
			flags |= PROTO_PLUSPLUS;
			continue;
		case 'S':
			comment = "#";
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	argv += opt_info.index;
	file = *argv;
#endif
	if (list)
	{
		if (*list == '-' && !*(list + 1))
			fd = 0;
		else if ((fd = open(list, O_RDONLY)) < 0)
			proto_error(NiL, 3, list, "not found");
		do
		{
			for (b = buf; (n = read(fd, b, 1)) > 0 && *b != '\n' && b < &buf[sizeof(buf) - 1]; b++);
			if (b > buf)
			{
				*b = 0;
				flags = proto(buf, license, options, package, copy, type(buf, comment), flags);
			}
		} while (n > 0);
		if (fd > 0)
			close(fd);
	}
	if (file)
		do flags = proto(file, license, options, package, copy, type(file, comment), flags); while (file = *++argv);
	else if (!list)
		flags = proto(file, license, options, package, copy, type(file, comment), flags);
	return errors ? 1 : (flags & PROTO_ERROR) ? 2 : 0;
}
