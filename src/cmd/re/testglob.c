/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1995-2012 AT&T Intellectual Property          *
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
 * glob(3) test harness
 * see help() for details
 */

static const char id[] = "\n@(#)$Id: testglob (AT&T Research) 2012-06-23 $\0\n";

#if _PACKAGE_ast

#include <ast.h>

#define quniq(v,n)	struniq(v,n)

#else

#define fmtident(s)	((char*)(s)+10)

static int
quniq(char** argv, int n)
{
	register char**	ao;
	register char**	an;
	register char**	ae;

	ao = an = argv;
	ae = ao + n;
	while (++an < ae) {
		while (streq(*ao, *an))
			if (++an >= ae)
				return ao - argv + 1;
		*++ao = *an;
	}
	return ao - argv + 1;
}

#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <glob.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

#ifdef	__STDC__
#include <stdlib.h>
#include <locale.h>
#endif

#ifndef NiL
#ifdef	__STDC__
#define NiL		0
#else
#define NiL		(char*)0
#endif
#endif

#define H(x)		fprintf(stderr,x)

static void
help(void)
{
H("NAME\n");
H("  testglob - glob(3) test harness\n");
H("\n");
H("SYNOPSIS\n");
H("  testglob [ options ] < testglob.dat\n");
H("\n");
H("DESCRIPTION\n");
H("  testglob reads glob(3) test specifications, one per line, from the\n");
H("  standard input and writes one output line for each failed test. A\n");
H("  summary line is written after all tests are done. Unsupported\n");
H("  features are noted before the first test, and tests requiring these\n");
H("  features are silently ignored. Each test is repeated with GLOB_LIST,\n");
H("  GLOB_STACK, and GLOB_LIST|GLOB_STACK set.\n");
H("\n");
H("OPTIONS\n");
H("  -c	catch signals and non-terminating calls\n");
H("  -e	ignore error code mismatches\n");
H("  -n	do not repeat tests with GLOB_LIST, GLOB_STACK, or GLOB_LIST|GLOB_STACK\n");
H("  -v	list each test line\n");
H("\n");
H("INPUT FORMAT\n");
H("  Input lines may be blank, a comment beginning with #, or a test\n");
H("  specification. A specification is five fields separated by one\n");
H("  or more tabs. NULL denotes the empty string and NIL denotes the\n");
H("  0 pointer.\n");
H("\n");
H("  Field 1: the glob(3) flags to apply, one character per GLOB_feature\n");
H("  flag. The test is skipped if GLOB_feature is not supported by the\n");
H("  implementation. If the first character is not [SK] then the\n");
H("  specification is a global control line. One or more of [SK] may be\n");
H("  specified; the test will be repeated for each mode.\n");
H("\n");
H("    K	GLOB_AUGMENTED		augmented (ksh) patterns\n");
H("    S	0			basic shell patterns\n");
H("\n");
H("    a	GLOB_APPEND		append to previous result\n");
H("    b	GLOB_BRACE		enable {...} expansion\n");
H("    c	GLOB_COMPLETE		shell file completion\n");
H("    e	GLOB_NOESCAPE		\\ not special\n");
H("    E	GLOB_ERR		abort on error\n");
H("    i	GLOB_ICASE		ignore case\n");
H("    m	GLOB_MARK		append / to directories\n");
H("    n	GLOB_NOCHECK		no match returns original pattern\n");
H("    r	GLOB_STARSTAR		enable /**/ expansion\n");
H("    s	GLOB_NOSORT		don't sort\n");
H("    u	standard unspecified behavior -- errors not counted\n");
H("\n");
H("  Field 1 control lines:\n");
H("\n");
H("    C	set LC_COLLATE and LC_CTYPE to the locale in field 2\n");
H("    I	set gl_fignore to the pattern in field 2\n");
H("    W	workspace file/dir; tab indentation denotes directory level\n");
H("\n");
H("    {				silent skip if failed until }\n");
H("    }				end of skip\n");
H("\n");
H("    : comment			comment copied to output\n");
H("\n");
H("  Field 2: a shell filename expansion pattern\n");
H("\n");
H("  Field 3: the expected glob() return value, OK for success, glob\n");
H("    return codes otherwise, with the GLOB_ prefix omitted\n");
H("\n");
H("  Field 4: optional space-separated matched file list\n");
H("\n");
H("  Field 5: optional comment appended to the report.\n");
H("\n");
H("CONTRIBUTORS\n");
H("  Glenn Fowler <gsf@research.att.com> (ksh strmatch, regex extensions)\n");
H("  David Korn <dgk@research.att.com> (ksh glob matcher)\n");
}

#ifndef elementsof
#define elementsof(x)	(sizeof(x)/sizeof(x[0]))
#endif

#ifndef streq
#define streq(a,b)	(*(a)==*(b)&&!strcmp(a,b))
#endif

#define LOOPED		2
#define NOTEST		(~0)

static const char* unsupported[] = {
#ifndef GLOB_AUGMENTED
	"AUGMENTED",
#endif
#ifndef GLOB_BRACE
	"BRACE",
#endif
#ifndef GLOB_COMPLETE
	"COMPLETE",
#endif
#ifndef GLOB_DISC
	"DISC",
#endif
#ifndef GLOB_ICASE
	"ICASE",
#endif
#ifndef GLOB_LIST
	"LIST",
#endif
#ifndef GLOB_STACK
	"STACK",
#endif
#ifndef GLOB_STARSTAR
	"STARSTAR",
#endif
	0
};

#ifndef GLOB_BRACE
#define GLOB_BRACE	NOTEST
#endif
#ifndef GLOB_COMPLETE
#define GLOB_COMPLETE	NOTEST
#endif
#ifndef GLOB_DISC
#define GLOB_DISC	0
#endif
#ifndef GLOB_ICASE
#define GLOB_ICASE	NOTEST
#endif
#ifndef GLOB_LIST
#define GLOB_LIST	0
#endif
#ifndef GLOB_STACK
#define GLOB_STACK	0
#endif
#ifndef GLOB_STARSTAR
#define GLOB_STARSTAR	0
#endif

#define GLOB_UNKNOWN	(-1)

#ifndef GLOB_ABORTED
#define GLOB_ABORTED	(GLOB_UNKNOWN-1)
#endif
#ifndef GLOB_NOMATCH
#define GLOB_NOMATCH	(GLOB_UNKNOWN-2)
#endif
#ifndef GLOB_NOSPACE
#define GLOB_NOSPACE	(GLOB_UNKNOWN-3)
#endif
#ifndef GLOB_INTR
#define GLOB_INTR	(GLOB_UNKNOWN-4)
#endif
#ifndef GLOB_APPERR
#define GLOB_APPERR	(GLOB_UNKNOWN-5)
#endif
#ifndef GLOB_NOSYS
#define GLOB_NOSYS	(GLOB_UNKNOWN-6)
#endif
#ifndef GLOB_ELOOP
#define GLOB_ELOOP	(GLOB_UNKNOWN-7)
#endif
#ifndef GLOB_EBUS
#define GLOB_EBUS	(GLOB_UNKNOWN-8)
#endif
#ifndef GLOB_EFAULT
#define GLOB_EFAULT	(GLOB_UNKNOWN-9)
#endif

static const struct { int code; char* name; char* desc;} codes[] = {
	GLOB_UNKNOWN,	"UNKNOWN",	"unknown",
	0,		"OK",		"ok",
	GLOB_ABORTED,	"ABORTED",	"glob aborted",
	GLOB_NOMATCH,	"NOMATCH",	"no match",
	GLOB_NOSPACE,	"NOSPACE",	"no space left",
	GLOB_INTR,	"INTR",		"an interrupt occurred",
	GLOB_APPERR,	"APPERR",	"aplication error",
	GLOB_NOSYS,	"NOSYS",	"not a system call",
	GLOB_ELOOP,	"ELOOP",	"recursin loop",
	GLOB_EBUS,	"EBUS",		"bus error",
	GLOB_EFAULT,	"EFAULT",	"memory fault",
};

static struct {
	int		errors;
	struct {
	int		count;
	int		error;
	int		position;
	}		ignore;
	int		lineno;
	int		ret;
	int		signals;
	int		unspecified;
	int		warnings;
	char*		stack;
	char*		which;
	jmp_buf		gotcha;
} state;

static void
quote(char* s)
{
	unsigned char*	u = (unsigned char*)s;
	int		c;

	if (!u)
		printf("NIL");
	else if (!*u)
		printf("NULL");
	else {
		printf("\"");
		for (;;) {
			switch (c = *u++) {
			case 0:
				break;;
			case '\\':
				printf("\\\\");
				continue;
			case '"':
				printf("\\\"");
				continue;
			case '\n':
				printf("\\n");
				continue;
			case '\r':
				printf("\\r");
				continue;
			case '\t':
				printf("\\t");
				continue;
			default:
				if (!iscntrl(c) && isprint(c))
					putchar(c);
				else
					printf("\\x%02x", c);
				continue;
			}
			break;
		}
		printf("\"");
	}
}

static void
report(char* comment, char* pat, char* msg, int flags, int unspecified)
{
	printf("%d: ", state.lineno);
#if GLOB_LIST
	if (flags & GLOB_LIST)
		printf("LIST: ");
#endif
#if GLOB_STACK
	if (flags & GLOB_STACK)
		printf("STACK: ");
#endif
	if (unspecified) {
		state.unspecified++;
		printf(" unspecified behavior");
	}
	else
		state.errors++;
	if (pat)
		quote(pat);
	printf(" %s %s", state.which, comment);
	if (msg && comment[strlen(comment)-1] != '\n')
		printf(": %s: ", msg);
}

static int
note(int level, int skip, char* msg)
{
	if (!skip) {
		printf("NOTE\t");
		if (msg)
			printf("%s: ", msg);
		printf("skipping lines %d", state.lineno);
	}
	return skip | level;
}

static void
bad(char* comment, char* pat)
{
	printf("bad test case ");
	report(comment, pat, NiL, 0, 0);
	exit(1);
}

static int
hex(int c)
{
	return isdigit(c) ? (c - '0') : (c - (isupper(c) ? 'A' : 'a') + 10);
}

static void
escape(char* s)
{
	char*	t;

	for (t = s; *t = *s; s++, t++) {
		if (*s != '\\')
			continue;
		switch (*++s) {

		case 0:
			*++t = 0;
			break;
		case '\\':
			*t = '\\';
			break;
		case 'n':
			*t = '\n';
			break;
		case 'r':
			*t = '\r';
			break;
		case 't':
			*t = '\t';
			break;
		case 'x':
			if (!isxdigit(s[1]) || !isxdigit(s[2]))
				bad("bad \\x\n", NiL);
			*t = hex(*++s) << 4;
			*t |= hex(*++s);
			break;
		default:
			s--;
			break;
		}
	}
}

static void
sigunblock(int s)
{
#ifdef SIG_SETMASK
	int		op;
	sigset_t	mask;

	sigemptyset(&mask);
	if (s) {
		sigaddset(&mask, s);
		op = SIG_UNBLOCK;
	}
	else
		op = SIG_SETMASK;
	sigprocmask(op, &mask, NiL);
#else
#ifdef sigmask
	sigsetmask(s ? (sigsetmask(0L) & ~sigmask(s)) : 0L);
#endif
#endif
}

static void
gotcha(int sig)
{
	signal(sig, gotcha);
	alarm(0);
	state.signals++;
	switch (sig) {

	case SIGALRM:
		state.ret = GLOB_ELOOP;
		break;
	case SIGBUS:
		state.ret = GLOB_EBUS;
		break;
	case SIGSEGV:
		state.ret = GLOB_EFAULT;
		break;
	}
	sigunblock(sig);
	longjmp(state.gotcha, 1);
}

static int
qstrcmp(const void* a, const void* b)
{
	return strcmp(*(char**)a, *(char**)b);
}

static char*
getline(void)
{
	static char	buf[32 * 1024];

	register char*	s = buf;
	register char*	e = &buf[sizeof(buf)];
	register char*	b;

	for (;;) {
		if (!(b = fgets(s, e - s, stdin)))
			return 0;
		state.lineno++;
		s += strlen(s) - 1;
		if (*s != '\n')
			break;
		if (s == b || *(s - 1) != '\\') {
			*s = 0;
			break;
		}
		s--;
	}
	return buf;
}

int
main(int argc, char** argv)
{
	int		flags;
	int		query;
	int		unspecified;
	int		kre;
	int		sre;
	int		okre;
	int		osre;
	int		ret;
	int		i;
	int		m;
	int		n;
	int		expected;
	int		got;
	int		ok;
	int		nmodes;
	int		extra;
	char*		spec;
	char*		pat;
	char*		p;
	char*		bp;
	char*		k;
	char*		s;
	char*		bs;
	char*		ans;
	char*		err;
	char*		msg;
	char**		v;
	char*		field[5];
	char		unit[64];
	char		pathbuf[1024];
	char		linkbuf[1024];
	char*		buf;
	char*		path;
	char*		pathmax;
	char*		work[16];
	char*		av[256];
	glob_t		gl;
	struct stat	st;
#if GLOB_DISC
	char		fignore[128];
#endif
#if GLOB_LIST
	globlist_t*	gi;
#endif

	int		catch = 0;
	int		cwd = 0;
	int		level = 1;
	int		locale = 0;
	int		skip = 0;
	int		testno = 0;
	int		verbose = 0;
	int		working = 0;

	static int	modes[] = {
				0,
#if GLOB_LIST
				GLOB_LIST,
#endif
#if GLOB_STACK
				GLOB_STACK,
#endif
#if GLOB_LIST && GLOB_STACK
				GLOB_LIST|GLOB_STACK,
#endif
	};

	nmodes = elementsof(modes);
	printf("TEST\t%s", s = fmtident(id));
	p = unit;
	while (p < &unit[sizeof(unit)-1] && (*p = *s++) && !isspace(*p))
		p++;
	*p = 0;
	while ((p = *++argv) && *p == '-')
		for (;;) {
			switch (*++p) {

			case 0:
				break;
			case 'c':
				catch = 1;
				printf(", catch");
				continue;
			case 'e':
				state.ignore.error = 1;
				printf(", ignore error code mismatches");
				continue;
			case 'h':
			case '?':
			case '-':
				printf(", help\n\n");
				help();
				return 2;
			case 'n':
				nmodes = 1;
				continue;
			case 'v':
				verbose = 1;
				printf(", verbose");
				continue;
			default:
				printf(", invalid option %c", *p);
				continue;
			}
			break;
		}
	if (p)
		printf(", argument(s) ignored");
	printf("\n");
	if (elementsof(unsupported) > 1) {
		printf("NOTE\tunsupported:");
		got = ' ';
		for (i = 0; i < elementsof(unsupported) - 1; i++) {
			printf("%c%s", got, unsupported[i]);
			got = ',';
		}
		printf("\n");
	}
	if (catch) {
		signal(SIGALRM, gotcha);
		signal(SIGBUS, gotcha);
		signal(SIGSEGV, gotcha);
	}
	path = pathbuf + 1;
	pathmax = &path[elementsof(pathbuf)-1];
	path[0] = 0;
	work[cwd] = path;
	work[cwd + 1] = 0;
	work[cwd + 2] = 0;
	ok = 0;
	extra = 0;
	while (p = buf = getline()) {

	/* parse: */

		if (*p == 0 || *p == '#')
			continue;
		if (*p == ':') {
			while (*++p == ' ');
			printf("NOTE	%s\n", p);
			continue;
		}
		if (*p == 'W') {
			while (*++p == '\t');
			if (*p) {
				i = p - buf;
				if (i > (cwd + 1) || i >= elementsof(work))
					bad("invalid workspace depth\n", NiL);
				if (working) {
					working = 0;
					cwd = 1;
					if (chdir("../.."))
						bad("cannot chdir\n", "../..");
					else if (verbose)
						printf("test %-3d chdir ../..\n", state.lineno);
				}
				if (i > cwd) {
					if (path[0] && access(path, F_OK)) {
						if (verbose) {
							printf("test %-3d mkdir ", state.lineno);
							quote(path);
							printf("\n");
						}
						if (mkdir(path, 0755))
							bad("cannot create work directory\n", path);
					}
				}
				else {
					if (access(path, F_OK)) {
						if (k) {
							if (verbose) {
								printf("test %-3d  link ", state.lineno);
								quote(path);
								printf(" ");
								quote(k);
								printf("\n");
							}
							if (symlink(k, path))
								bad("cannot create work link\n", path);
							k = 0;
						}
						else if (!streq(work[cwd - 1], ".")) {
							if (verbose) {
								printf("test %-3d  file ", state.lineno);
								quote(path);
								printf("\n");
							}
							if (close(creat(path, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)))
								bad("cannot create work file\n", path);
						}
					}
					cwd = i - 1;
				}
				s = work[cwd];
				*(s - 1) = '/';
				k = 0;
				while (s < pathmax && *p) {
					if (*p == '\t') {
						k = linkbuf;
						while (k < &linkbuf[sizeof(linkbuf) - 1] && (*k = *++p))
							k++;
						*k = 0;
						k = linkbuf;
						break;
					}
					*s++ = *p++;
				}
				*s++ = 0;
				work[cwd = i] = s;
			}
			continue;
		}
		if (work[2]) {
			if (!streq(work[cwd-1], ".") && access(path, F_OK))
				if (k) {
					if (verbose) {
						printf("test %-3d  link ", state.lineno);
						quote(path);
						printf(" ");
						quote(k);
						printf("\n");
					}
					if (symlink(k, path))
						bad("cannot create work link\n", path);
				}
				else {
					if (verbose) {
						printf("test %-3d  file ", state.lineno);
						quote(path);
						printf("\n");
					}
					if (close(creat(path, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)))
						bad("cannot create work file\n", path);
				}
			*(work[2] - 1) = 0;
			if (!working)
				working = 1;
			else if (chdir("../.."))
				bad("cannot chdir\n", "../..");
			else if (verbose)
				printf("test %-3d chdir ..\n", state.lineno);
			if (chdir(path))
				bad("cannot chdir\n", path);
			else if (verbose) {
				printf("test %-3d chdir ", state.lineno);
				quote(path);
				printf("\n");
			}
			work[2] = 0;
		}
#if GLOB_DISC
		fignore[0] = 0;
#endif
		i = 0;
		field[i++] = p;
		for (;;) {
			switch (*p++) {
			case 0:
				p--;
				goto checkfield;
			case '\t':
				*(p - 1) = 0;
			checkfield:
				s = field[i - 1];
				if (streq(s, "NIL"))
					field[i - 1] = 0;
				else if (streq(s, "NULL"))
					*s = 0;
				while (*p == '\t')
					p++;
				if (!*p)
					break;
				if (i >= elementsof(field))
					bad("too many fields\n", NiL);
				field[i++] = p;
				/*FALLTHROUGH*/
			default:
				continue;
			}
			break;
		}
		if (!(spec = field[0]))
			bad("NIL spec\n", NiL);

	/* interpret: */

		flags = 0;
		query = unspecified = kre = sre = 0;
		for (p = spec; *p; p++) {
			switch (*p) {
			case 'C':
				if (!query && !(skip & level))
					bad("locale query expected\n", NiL);
				query = 0;
				if (locale)
					bad("locale nesting not supported\n", NiL);
				if (i != 2)
					bad("locale field expected\n", NiL);
				if (!(skip & level)) {
#if defined(LC_COLLATE) && defined(LC_CTYPE)
					s = field[1];
					if (!s || streq(s, "POSIX"))
						s = "C";
					if (!(ans = setlocale(LC_COLLATE, s)) || streq(ans, "C") || streq(ans, "POSIX") || !(ans = setlocale(LC_CTYPE, s)) || streq(ans, "C") || streq(ans, "POSIX"))
						skip = note(level, skip, s);
					else {
						printf("NOTE	\"%s\" locale\n", s);
						locale = level;
					}
#else
					skip = note(level, skip, "locales not supported");
#endif
				}
				flags |= NOTEST;
				continue;
			case 'E':
				flags |= GLOB_ERR;
				continue;
			case 'I':
#if GLOB_DISC
				if (field[1])
					strncpy(fignore, field[1], sizeof(fignore) - 1);
				else
					fignore[0] = 0;
#endif
				flags |= NOTEST;
				continue;
			case 'K':
				kre = 1;
				continue;
			case 'S':
				sre = 1;
				continue;

			case 'a':
				flags |= GLOB_APPEND;
				continue;
			case 'b':
				flags |= GLOB_BRACE;
				continue;
			case 'c':
				flags |= GLOB_COMPLETE;
				continue;
			case 'e':
				flags |= GLOB_NOESCAPE;
				continue;
			case 'i':
				flags |= GLOB_ICASE;
				continue;
			case 'm':
				flags |= GLOB_MARK;
				continue;
			case 'n':
				flags |= GLOB_NOCHECK;
				continue;
			case 'r':
				flags |= GLOB_STARSTAR;
				continue;
			case 's':
				flags |= GLOB_NOSORT;
				continue;
			case 'u':
				unspecified = 1;
				continue;

			case 'x':
#if GLOB_VERSION >= 20060717L
				extra = 15;
#endif
				continue;

			case '{':
				level <<= 1;
				if (skip & (level >> 1)) {
					skip |= level;
					flags |= NOTEST;
				}
				else {
					skip &= ~level;
					query = 1;
				}
				continue;
			case '}':
				if (level == 1)
					bad("invalid {...} nesting\n", NiL);
				else {
					if ((skip & level) && !(skip & (level>>1)))
						printf("-%d\n", state.lineno);
#if defined(LC_COLLATE) && defined(LC_CTYPE)
					else if (locale & level) {
						locale = 0;
						if (!(skip & level)) {
							s = "C";
							setlocale(LC_COLLATE, s);
							setlocale(LC_CTYPE, s);
							printf("NOTE	\"%s\" locale\n", s);
						}
					}
#endif
					level >>= 1;
				}
				flags |= NOTEST;
				continue;
			default:
				bad("bad spec\n", spec);
				break;

			}
			break;
		}
		if (flags == NOTEST || !sre && !kre)
			continue;
		if (i < 3)
			bad("too few fields\n", NiL);
		if (skip & level)
			continue;
		while (i < elementsof(field))
			field[i++] = 0;
		if (pat = field[1])
			escape(pat);
		if (ans = field[3])
			escape(ans);
		okre = kre;
		osre = sre;
		for (m = 0; m < nmodes; m++) {
			if (modes[m]) {
				if ((flags & modes[m]) == modes[m])
					continue;
				flags |= modes[m];
			}
			err = field[2];
			msg = field[4];
			kre = okre;
			sre = osre;
			fflush(stdout);

	/* execute: */

			if (sre) {
				state.which = "SRE";
				sre = 0;
#ifdef GLOB_AUGMENTED
				flags &= ~GLOB_AUGMENTED;
#endif
			}
#ifdef GLOB_AUGMENTED
			else if (kre) {
				state.which = "KRE";
				kre = 0;
				flags |= GLOB_AUGMENTED;
			}
#endif
			else
				continue;
			if (!m && !query && verbose) {
				printf("test %-3d ", state.lineno);
				quote(pat);
				printf("\n");
			}
			if (!ok || !(flags & GLOB_APPEND)) {
				if (ok) {
					ok = 0;
					globfree(&gl);
				}
				memset(&gl, 0, sizeof(gl));
			}
#if GLOB_DISC
			if (fignore[0]) {
				gl.gl_fignore = (const char*)fignore;
				flags |= GLOB_DISC;
			}
			else
				gl.gl_fignore = 0;
#if GLOB_VERSION >= 20060717L
			if (gl.gl_extra = extra)
				flags |= GLOB_DISC;
#endif
#endif
			if (!query)
				testno++;
			if (catch) {
				if (setjmp(state.gotcha))
					ret = state.ret;
				else {
					alarm(LOOPED);
					ret = glob(pat, flags, 0, &gl);
					alarm(0);
				}
			}
			else
				ret = glob(pat, flags, 0, &gl);
			if (ret == 0) {
				ok = 1;
				if (!gl.gl_pathc)
					ret = GLOB_NOMATCH;
			}
			expected = got = 0;
			for (i = 1; i < elementsof(codes); i++) {
				if (streq(err, codes[i].name))
					expected = i;
				if (ret == codes[i].code)
					got = i;
			}
			if (expected != got) {
				if (query)
					skip = note(level, skip, msg);
				else if (state.ignore.error)
					state.ignore.count++;
				else {
					report("return failed: ", pat, msg, flags, unspecified);
					printf("%s expected, %s returned", codes[expected].name, codes[got].name);
					if (!ret)
						printf(" with %d match%s", gl.gl_pathc, gl.gl_pathc == 1 ? "" : "es");
					printf("\n");
				}
			}
			else if (ret != GLOB_NOMATCH) {
#if GLOB_LIST
				if (flags & GLOB_LIST) {
					n = 0;
					for (gi = gl.gl_list; gi; gi = gi->gl_next) {
						if (n >= (elementsof(av) - 1))
							break;
						av[n++] = gi->gl_path + extra;
					}
					av[n] = 0;
					v = av;
				}
				else
#endif
				{
					n = gl.gl_pathc;
					v = gl.gl_pathv;
				}
				if (verbose) {
					printf("    ");
					for (i = 0; i < n; i++)
						printf(" %s", v[i]);
					printf("\n");
				}
				if (flags & (GLOB_LIST|GLOB_NOSORT)) {
					qsort(v, n, sizeof(*v), qstrcmp);
					if ((flags & GLOB_STARSTAR) && !(gl.gl_flags & GLOB_STARSTAR))
						v[n = quniq(v, n)] = 0;
				}
				if (!ans)
					ans = "";
				bs = s = ans;
				bp = p = "";
				for (i = 0; i < n; i++) {
					bp = p = v[i];
					bs = s;
					while (*p == *s && *s && *s != ' ') {
						p++;
						s++;
					}
					if (*p || *s && *s != ' ')
						break;
					while (*s == ' ')
						s++;
				}
				if (*p || *s) {
					if (!*p && i >= n)
						bp = 0;
					if (!*s)
						bs = s = 0;
					else {
						while (*s && *s != ' ')
							s++;
						if (*s == ' ')
							*s = 0;
						else
							s = 0;
					}
					report("match failed: ", pat, msg, flags, unspecified);
					quote(bs);
					printf(" expected, ");
					quote(bp);
					printf(" returned\n");
					if (s)
						*s = 0;
				}
			}
			if (flags & GLOB_APPEND)
				break;
			flags &= ~modes[m];
		}
	}
	printf("TEST\t%s, %d test%s", unit, testno, testno == 1 ? "" : "s");
	if (state.ignore.count)
		printf(", %d ignored mismatche%s", state.ignore.count, state.ignore.count == 1 ? "" : "s");
	if (state.warnings)
		printf(", %d warning%s", state.warnings, state.warnings == 1 ? "" : "s");
	if (state.unspecified)
		printf(", %d unspecified difference%s", state.unspecified, state.unspecified == 1 ? "" : "s");
	if (state.signals)
		printf(", %d signal%s", state.signals, state.signals == 1 ? "" : "s");
	printf(", %d error%s\n", state.errors, state.errors == 1 ? "" : "s");
	return 0;
}
