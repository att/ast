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
 * strmatch(3) test harness
 * see testmatch --help for a description of the input format
 */

#if OLD
#define LEGACY		"old"
#else
#define LEGACY		""
#endif

static const char id[] = "\n@(#)$Id: test" LEGACY "match (AT&T Research) 2012-06-25 $\0\n";

#if _PACKAGE_ast
#include <ast.h>
#else
#define fmtident(s)	((char*)(s)+10)
#endif

#include <stdio.h>
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
H("  testmatch - strgrpmatch(3) test harness\n");
H("\n");
H("SYNOPSIS\n");
H("  testmatch [ options ] testmatch.dat\n");
H("\n");
H("DESCRIPTION\n");
H("  testmatch reads strgrpmatch(3) test specifications, one per line, from\n");
H("  the standard input and writes one output line for each failed test. A\n");
H("  summary line is written after all tests are done. Each successful\n");
H("  test is run again with no subexpression pointer array. Unsupported\n");
H("  features are noted before the first test, and tests requiring these\n");
H("  features are silently ignored.\n");
H("\n");
#if OLD
H("  This version tests the legacy ad-hoc implementation with roots in the\n");
H("  Bourne sh implementation.\n");
#else
H("  This version tests the AST <regex.h> implementation.\n");
#endif
H("\n");
H("OPTIONS\n");
H("  -c	catch signals and non-terminating calls\n");
H("  -h	list help\n");
H("  -v	list each test line\n");
H("\n");
H("INPUT FORMAT\n");
H("  Input lines may be blank, a comment beginning with #, or a test\n");
H("  specification. A specification is five fields separated by one\n");
H("  or more tabs. NULL denotes the empty string and NIL denotes the\n");
H("  0 pointer.\n");
H("\n");
H("  Field 1: the strgrpmatch(3) flags to apply, one character per\n");
H("  STR_feature flag. The test is skipped if STR_feature is not supported\n");
H("  by the implementation. If the first character is not [SK] then the\n");
H("  specification is a global control line. Note that no distinction\n");
H("  is made between SRE and KRE tests. STR_MAXIMAL is set by default.\n");
H("  Specifications containing testre(1T) flags are silently ignored.\n");
H("\n");
H("    S	0			SRE	(sh glob)\n");
H("    K	0			KRE	(ksh glob)\n");
H("\n");
H("    a	STR_LEFT|STR_RIGHT	implicit ^...$\n");
H("    i	STR_ICASE		ignore case\n");
H("    l	STR_LEFT		implicit ^...\n");
H("    m	~STR_MAXIMAL		minimal match (default is STR_MAXIMAL)\n");
H("    r	STR_RIGHT		implicit ...$\n");
H("    u	standard unspecified behavior -- errors not counted\n");
H("    $	                        expand C \\c escapes in fields 2 and 3\n");
H("\n");
H("  Field 1 control lines:\n");
H("\n");
H("    C	set LC_COLLATE and LC_CTYPE to locale in field 2\n");
H("\n");
H("    {				silent skip if failed until }\n");
H("    }				end of skip\n");
H("\n");
H("    : comment			comment copied to output\n");
H("\n");
H("    number			use number for nmatch (20 by default)\n");
H("\n");
H("  Field 2: the strgrpmatch expression pattern; SAME uses the pattern\n");
H("    from the previous specification.\n");
H("\n");
H("  Field 3: the string to match.\n");
H("\n");
H("  Field 4: the test outcome. This is either OK, NOMATCH, BADPAT, or\n");
H("    the match array, a list of (m,n) entries with m and n being first\n");
H("    and last+1 positions in the field 3 string, or NULL if subexpression\n");
H("    array is specified and success is expected. The match[]\n");
H("    subexpression pointer array is initialized to (-2,-2).\n");
H("    All array elements not equal to (-2,-2) must be specified\n");
H("    in the outcome. Unspecified endpoints are denoted by ?.\n");
H("\n");
H("  Field 5: optional comment appended to the report.\n");
H("\n");
H("CONTRIBUTORS\n");
H("  Glenn Fowler <gsf@research.att.com> (ksh strgrpmatch)\n");
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

#if !defined(STR_ICASE) && !defined(STR_LEFT)  && !defined(STR_MAXIMAL) && !defined(STR_RIGHT)
#define NOT_SUPPORTED	1
#endif

static const char* unsupported[] =
{
	0,
#if NOT_SUPPORTED
	"strmatch",
	"strgrpmatch",
#endif
#ifndef STR_ICASE
	"ICASE",
#endif
#ifndef STR_LEFT
	"LEFT",
#endif
#ifndef STR_MAXIMAL
	"MAXIMAL",
#endif
#ifndef STR_RIGHT
	"RIGHT",
#endif
};

#ifndef STR_ICASE
#define STR_ICASE	NOTEST
#endif
#ifndef STR_LEFT
#define STR_LEFT	NOTEST
#endif
#ifndef STR_MAXIMAL
#define STR_MAXIMAL	NOTEST
#endif
#ifndef STR_RIGHT
#define STR_RIGHT	NOTEST
#endif

#if STR_ICASE==NOTEST && STR_LEFT==NOTEST  && STR_MAXIMAL==NOTEST && STR_RIGHT==NOTEST
#define NOT_SUPPORTED	1
#endif

static struct
{
	int		errors;
	struct
	{
	int		count;
	int		error;
	int		position;
	}		ignore;
	int		lineno;
	int		ret;
	int		signals;
	int		unspecified;
	int		warnings;
	char*		file;
	char*		which;
	jmp_buf		gotcha;
} state;

static void
quote(char* s, int expand)
{
	unsigned char*	u = (unsigned char*)s;
	int		c;

	if (!u)
		printf("NIL");
	else if (!*u)
		printf("NULL");
	else if (expand)
	{
		printf("\"");
		for (;;)
		{
			switch (c = *u++)
			{
			case 0:
				break;;
			case '\\':
				printf("\\\\");
				continue;
			case '"':
				printf("\\\"");
				continue;
			case '\a':
				printf("\\a");
				continue;
			case '\b':
				printf("\\b");
				continue;
			case '\f':
				printf("\\f");
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
			case '\v':
				printf("\\v");
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
	else
		printf("%s", s);
}

static void
report(char* comment, char* fun, char* re, char* s, char* msg, int flags, int unspecified, int expand)
{
	if (state.file)
		printf("%s:", state.file);
	printf("%d:", state.lineno);
	if (re)
	{
		printf(" ");
		quote(re, expand);
		if (s)
		{
			printf(" versus ");
			quote(s, expand);
		}
	}
	if (unspecified)
	{
		state.unspecified++;
		printf(" unspecified behavior");
	}
	else
		state.errors++;
	printf(" %s", state.which);
	if (fun)
		printf(" %s", fun);
	if (comment[strlen(comment)-1] == '\n')
		printf(" %s", comment);
	else
	{
		printf(" %s: ", comment);
		if (msg)
			printf("%s: ", msg);
	}
}

static int
note(int level, int skip, char* msg)
{
	if (!skip)
	{
		printf("NOTE\t");
		if (msg)
			printf("%s: ", msg);
		printf("skipping lines %d", state.lineno);
	}
	return skip | level;
}

static void
bad(char* comment, char* re, char* s, int expand)
{
	printf("bad test case ");
	report(comment, NiL, re, s, NiL, 0, 0, expand);
	exit(1);
}

static void
escape(char* s)
{
	char*	e;
	char*	t;
	char*	q;
	int	c;

	for (t = s; *t = *s; s++, t++)
		if (*s == '\\')
			switch (*++s)
			{
			case '\\':
				break;
			case 'a':
				*t = '\a';
				break;
			case 'b':
				*t = '\b';
				break;
			case 'c':
				if (*t = *s)
					s++;
				*t &= 037;
				break;
			case 'e':
			case 'E':
				*t = 033;
				break;
			case 'f':
				*t = '\f';
				break;
			case 'n':
				*t = '\n';
				break;
			case 'r':
				*t = '\r';
				break;
			case 's':
				*t = ' ';
				break;
			case 't':
				*t = '\t';
				break;
			case 'v':
				*t = '\v';
				break;
			case 'u':
			case 'x':
				q = *s == 'u' ? (s + 5) : (char*)0;
				c = 0;
				e = s + 1;
				while (!e || !q || s < q)
				{
					switch (*++s)
					{
					case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
						c = (c << 4) + *s - 'a' + 10;
						continue;
					case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
						c = (c << 4) + *s - 'A' + 10;
						continue;
					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						c = (c << 4) + *s - '0';
						continue;
					case '{':
					case '[':
						if (s != e)
						{
							s--;
							break;
						}
						e = 0;
						if (q && *(s + 1) == 'U' && *(s + 2) == '+')
							s += 2;
						continue;
					case '}':
					case ']':
						if (e)
							s--;
						break;
					default:
						s--;
						break;
					}
					break;
				}
				*t = c;
				break;
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				c = *s - '0';
				q = s + 2;
				while (s < q)
					switch (*++s)
					{
					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
						c = (c << 3) + *s - '0';
						break;
					default:
						q = --s;
						break;
					}
				*t = c;
				break;
			default:
				bad("invalid C \\ escape\n", NiL, NiL, 0);
			}
}

static void
matchprint(ssize_t* match, int nmatch, char* ans)
{
	int	i;

	for (; nmatch > 0; nmatch -= 2)
		if (match[nmatch-2] != -2 && (!state.ignore.position || match[nmatch-2] >= 0 && match[nmatch-2] >= 0))
			break;
	for (i = 0; i < nmatch; i += 2)
	{
		printf("(");
		if (match[i] == -1)
			printf("?");
		else
			printf("%zd", match[i]);
		printf(",");
		if (match[i+1] == -1)
			printf("?");
		else
			printf("%zd", match[i+1]);
		printf(")");
	}
	if (ans)
		printf(" expected: %s", ans);
	printf("\n");
}

static int
matchcheck(int nmatch, ssize_t* match, char* ans, char* re, char* s, int flags, int query, int unspecified, int expand)
{
	char*	p;
	int	i;
	ssize_t	m;
	ssize_t	n;

	for (i = 0, p = ans; i < nmatch && *p; i += 2)
	{
		if (*p++ != '(')
			bad("improper answer\n", re, s, expand);
		if (*p == '?')
		{
			m = -1;
			p++;
		}
		else
			m = strtol(p, &p, 10);
		if (*p++ != ',')
			bad("improper answer\n", re, s, expand);
		if (*p == '?')
		{
			n = -1;
			p++;
		}
		else
			n = strtol(p, &p, 10);
		if (*p++ != ')')
			bad("improper answer\n", re, s, expand);
		if (m!=match[i] || n!=match[i+1])
		{
			if (!query)
			{
				report("failed: match was", NiL, re, s, NiL, flags, unspecified, expand);
				matchprint(match, nmatch, ans);
			}
			return 0;
		}
	}
	for (; i < nmatch; i += 2)
	{
		if (match[i]!=-2 || match[i+1]!=-2)
		{
			if (!query)
			{
				if (state.ignore.position && (match[i]<0 || match[i+1]<0))
				{
					state.ignore.count++;
					return 0;
				}
				report("failed: match was", NiL, re, s, NiL, flags, unspecified, expand);
				matchprint(match, nmatch, ans);
			}
			return 0;
		}
	}
	if (match[nmatch] != -2)
	{
		report("failed: overran match array", NiL, re, s, NiL, flags, unspecified, expand);
		matchprint(match, nmatch + 1, NiL);
	}
	return 1;
}

static void
sigunblock(int s)
{
#ifdef SIG_SETMASK
	int		op;
	sigset_t	mask;

	sigemptyset(&mask);
	if (s)
	{
		sigaddset(&mask, s);
		op = SIG_UNBLOCK;
	}
	else op = SIG_SETMASK;
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
	state.ret = 0;
	sigunblock(sig);
	longjmp(state.gotcha, 1);
}

static char*
getline(FILE* fp)
{
	static char	buf[32 * 1024];

	register char*	s = buf;
	register char*	e = &buf[sizeof(buf)];
	register char*	b;

	for (;;)
	{
		if (!(b = fgets(s, e - s, fp)))
			return 0;
		state.lineno++;
		s += strlen(s) - 1;
		if (*s != '\n')
			break;
		if (s == b || *(s - 1) != '\\')
		{
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
	int		cflags;
	int		eflags;
	int		query;
	int		expand;
	int		unspecified;
	int		kre;
	int		sre;
	int		nmatch;
	int		rmatch;
	int		eret;
	int		i;
	int		sub;
	int		subunitlen;
	int		level = 1;
	int		locale = 0;
	int		skip = 0;
	int		testno = 0;
	FILE*		fp;
	char*		p;
	char*		spec;
	char*		re;
	char*		s;
	char*		ans;
	char*		msg;
	char*		fun;
	char*		subunit;
	char*		version;
	char*		field[6];
	char		unit[64];
	ssize_t		match[200];

	int		catch = 0;
	int		verbose = 0;

	static char*	filter[] = { "-", 0 };

	version = fmtident(id);
	p = unit;
	while (p < &unit[sizeof(unit)-1] && (*p = *version++) && !isspace(*p))
		p++;
	*p = 0;
	while ((p = *++argv) && *p == '-')
		for (;;)
		{
			switch (*++p)
			{
			case 0:
				break;
			case 'c':
				catch = 1;
				continue;
			case 'h':
			case '?':
			case '-':
				help();
				return 2;
			case 'p':
				state.ignore.position = 1;
				continue;
			case 'v':
				verbose = 1;
				continue;
			default:
				fprintf(stderr, "%s: -%c: invalid option", unit, *p);
				continue;
			}
			break;
		}
	if (catch)
	{
		signal(SIGALRM, gotcha);
		signal(SIGBUS, gotcha);
		signal(SIGSEGV, gotcha);
	}
	if (!*argv)
		argv = filter;
	while (state.file = *argv++)
	{
		if (streq(state.file, "-") || streq(state.file, "/dev/stdin") || streq(state.file, "/dev/fd/0"))
		{
			state.file = 0;
			fp = stdin;
		}
		else if (!(fp = fopen(state.file, "r")))
		{
			fprintf(stderr, "%s: %s: cannot read\n", unit, state.file);
			return 2;
		}
		printf("TEST\t%s ", unit);
		if (s = state.file)
		{
			subunit = p = 0;
			for (;;)
			{
				switch (*s++)
				{
				case 0:
					break;
				case '/':
					subunit = s;
					continue;
				case '.':
					p = s - 1;
					continue;
				default:
					continue;
				}
				break;
			}
			if (!subunit)
				subunit = state.file;
			if (p < subunit)
				p = s - 1;
			subunitlen = p - subunit;
			if (subunitlen == strlen(unit) && !memcmp(subunit, unit, subunitlen))
				subunit = 0;
			else
				printf("%-.*s ", subunitlen, subunit);
		}
		else
			subunit = 0;
		printf("%s", version);
		if (catch)
			printf(", catch");
		if (verbose)
			printf(", verbose");
		for (i = 1; i < elementsof(unsupported); i++)
			printf(", no%s", unsupported[i]);
		printf("\n");
		level = 1;
		locale = skip = testno = 0;
		state.ignore.count = state.lineno = state.signals = state.unspecified = state.warnings = 0;
		while (p = getline(fp))
		{

		/* parse: */

			if (*p == 0 || *p == '#')
				continue;
			if (*p == ':')
			{
				while (*++p == ' ');
				printf("NOTE	%s\n", p);
				continue;
			}
			i = 0;
			field[i++] = p;
			for (;;)
			{
				switch (*p++)
				{
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
						bad("too many fields\n", NiL, NiL, 0);
					field[i++] = p;
					/*FALLTHROUGH*/
				default:
					continue;
				}
				break;
			}
			if (!(spec = field[0]))
				bad("NIL spec\n", NiL, NiL, 0);

		/* interpret: */

			cflags = 0;
#if STR_MAXIMAL != NOTEST
			eflags = STR_MAXIMAL;
#else
			eflags = 0;
#endif
			expand = query = unspecified = kre = sre = 0;
			nmatch = 20;
			for (p = spec; *p; p++)
			{
				if (isdigit(*p))
				{
					if ((nmatch = 2 * strtol(p, &p, 10)) >= elementsof(match))
						bad("nmatch too large\n", spec, NiL, 0);
					p--;
					continue;
				}
				switch (*p)
				{
				case 'A':
					continue;
				case 'B':
					continue;
				case 'C':
					if (!query && !(skip & level))
						bad("locale query expected\n", NiL, NiL, 0);
					query = 0;
#if OLD
					if (!(skip & level))
						skip = note(level, skip, "locales not supported by old strmatch()");
#else
					if (locale)
						bad("locale nesting not supported\n", NiL, NiL, 0);
					if (i != 2)
						bad("locale field expected\n", NiL, NiL, 0);
					if (!(skip & level))
					{
#if defined(LC_COLLATE) && defined(LC_CTYPE)
						s = field[1];
						if (!s || streq(s, "POSIX"))
							s = "C";
						if (!(ans = setlocale(LC_COLLATE, s)) || streq(ans, "C") || streq(ans, "POSIX") || !(ans = setlocale(LC_CTYPE, s)) || streq(ans, "C") || streq(ans, "POSIX"))
							skip = note(level, skip, s);
						else
						{
							printf("NOTE	\"%s\" locale\n", s);
							locale = level;
						}
#else
						skip = note(level, skip, "locales not supported");
#endif
					}
#endif
					cflags = NOTEST;
					continue;
				case 'E':
					continue;
				case 'K':
					kre = 1;
					continue;
				case 'L':
					continue;
				case 'S':
					sre = 1;
					continue;

				case 'a':
					eflags |= STR_LEFT|STR_RIGHT;
					continue;
				case 'b':
					cflags = NOTEST;
					continue;
				case 'c':
					cflags = NOTEST;
					continue;
				case 'd':
					cflags = NOTEST;
					continue;
				case 'e':
					cflags = NOTEST;
					continue;
				case 'f':
					continue;
				case 'g':
					cflags = NOTEST;
					continue;
				case 'h':
					cflags = NOTEST;
					continue;
				case 'i':
					eflags |= STR_ICASE;
					continue;
				case 'j':
					cflags = NOTEST;
					continue;
				case 'k':
					cflags = NOTEST;
					continue;
				case 'l':
					eflags |= STR_LEFT;
					continue;
				case 'm':
#if STR_MAXIMAL != NOTEST
					eflags &= ~STR_MAXIMAL;
#else
					eflags = NOTEST;
#endif
					continue;
				case 'n':
					cflags = NOTEST;
					continue;
				case 'o':
					cflags = NOTEST;
					continue;
				case 'p':
					cflags = NOTEST;
					continue;
				case 'r':
					eflags |= STR_RIGHT;
					continue;
				case 's':
					cflags = NOTEST;
					continue;
				case 'u':
					unspecified = 1;
					continue;
				case 'v':
					cflags = NOTEST;
					continue;
				case 'x':
					cflags = NOTEST;
					continue;
				case 'y':
					eflags = NOTEST;
					continue;
				case 'z':
					cflags = NOTEST;
					continue;

				case '$':
					expand = 1;
					continue;
				case '/':
					cflags = NOTEST;
					continue;

				case '{':
					level <<= 1;
					if (skip & (level >> 1))
					{
						skip |= level;
						cflags = NOTEST;
					}
					else
					{
						skip &= ~level;
						query = 1;
					}
					continue;
				case '}':
					if (level == 1)
						bad("invalid {...} nesting\n", NiL, NiL, 0);
					else
					{
						if ((skip & level) && !(skip & (level>>1)))
							printf("-%d\n", state.lineno);
#if defined(LC_COLLATE) && defined(LC_CTYPE)
						else if (locale & level)
						{
							locale = 0;
							if (!(skip & level))
							{
								s = "C";
								setlocale(LC_COLLATE, s);
								setlocale(LC_CTYPE, s);
								printf("NOTE	\"%s\" locale\n", s);
							}
						}
#endif
						skip &= ~level;
						level >>= 1;
					}
					cflags = NOTEST;
					continue;

				default:
					bad("bad spec\n", spec, NiL, 0);
					break;

				}
				break;
			}
			if ((cflags|eflags) == NOTEST || !sre && !kre)
				continue;
			if (i < 4)
				bad("too few fields\n", NiL, NiL, 0);
			while (i < elementsof(field))
				field[i++] = 0;
			if ((re = field[1]) && expand)
				escape(re);
			if ((s = field[2]) && expand)
				escape(s);
			if (!(ans = field[3]))
				bad("NIL answer\n", NiL, NiL, 0);
			msg = field[4];
			fflush(stdout);

		compile:

			if (skip)
				continue;
#if NOT_SUPPORTED == 0
			if (sre)
			{
				state.which = "SRE";
				sre = 0;
			}
			else if (kre)
			{
				state.which = "KRE";
				kre = 0;
			}
			else
				continue;
			if (!query && verbose)
				printf("test %-3d %s \"%s\" \"%s\"\n", state.lineno, state.which, re, s);
			sub = 1;

		nosub:

			if (skip)
				continue;
			if (!query)
				testno++;
			for (i = 0; i < elementsof(match); i++)
				match[i] = -2;
			if (catch)
			{
				if (setjmp(state.gotcha))
					eret = state.ret;
				else
				{
					alarm(LOOPED);
					if (sub)
					{
						fun = "strgrpmatch";
						eret = (rmatch = strgrpmatch(s, re, match, nmatch / 2, eflags)) == 0;
						if (verbose)
							printf("[%s]", fun);
					}
					else
					{
						fun = "strmatch";
						eret = (rmatch = strmatch(s, re)) == 0;
					}
					alarm(0);
				}
			}
			else if (sub)
			{
				fun = "strgrpmatch";
				eret = (rmatch = strgrpmatch(s, re, match, nmatch / 2, eflags)) == 0;
				if (verbose)
					printf("[%s]", fun);
			}
			else
			{
				fun = "strmatch";
				eret = (rmatch = strmatch(s, re)) == 0;
			}
#if OLD
			if (eret && streq(s, re))
			{
				note(level, skip, "old strmatch() does not fall back to literal match on error");
				printf("-EOF\n");
				goto skip;
			}
#endif
			if (!sub)
			{
				if (eret)
				{
					if (!streq(ans, "NOMATCH") && *ans != 'E')
					{
						if (query)
							skip = note(level, skip, msg);
						else
							report("failed", fun, re, s, msg, nmatch, unspecified, expand);
							if (eret == 1)
								printf("OK expected, NOMATCH returned");
							else
								printf("OK expected, error %d returned", eret);
							printf("\n");
					}
				}
				else if (streq(ans, "NOMATCH") || *ans == 'E')
				{
					if (query)
						skip = note(level, skip, msg);
					else
					{
						report("failed", fun, re, s, msg, nmatch, unspecified, expand);
						printf("%s expected, OK returned", ans);
						printf("\n");
					}
				}
			}
			else if (eret)
			{
				if (!streq(ans, "NOMATCH") && *ans != 'E')
				{
					if (query)
						skip = note(level, skip, msg);
					else
					{
						report("failed", fun, re, s, msg, nmatch, unspecified, expand);
						if (eret == 1)
							printf("OK expected, NOMATCH returned");
						else
							printf("OK expected, error %d returned", eret);
						printf("\n");
					}
				}
			}
			else if (streq(ans, "NOMATCH") || *ans == 'E')
			{
				if (query)
					skip = note(level, skip, msg);
				else
				{
					report("should fail and didn't", fun, re, s, msg, nmatch, unspecified, expand);
					matchprint(match, nmatch, NiL);
				}
			}
			else if (!*ans)
			{
				if (match[0] != -2)
				{
					if (query)
						skip = note(level, skip, msg);
					else
					{
						report("failed: no match but match array assigned", NiL, re, s, msg, nmatch, unspecified, expand);
						matchprint(match, nmatch, NiL);
					}
				}
			}
			else if (!matchcheck(2 * rmatch, match, ans, re, s, nmatch, query, unspecified, expand))
			{
				if (eflags ^ (STR_LEFT|STR_RIGHT))
					continue;
				sub = 0;
				goto nosub;
			}
			else if (query)
				skip = note(level, skip, msg);
			goto compile;
#endif
		}
#if OLD

	skip:

#endif
		printf("TEST\t%s", unit);
		if (subunit)
			printf(" %-.*s", subunitlen, subunit);
		printf(", %d test%s", testno, testno == 1 ? "" : "s");
		if (state.ignore.count)
			printf(", %d ignored mismatche%s", state.ignore.count, state.ignore.count == 1 ? "" : "s");
		if (state.warnings)
			printf(", %d warning%s", state.warnings, state.warnings == 1 ? "" : "s");
		if (state.unspecified)
			printf(", %d unspecified difference%s", state.unspecified, state.unspecified == 1 ? "" : "s");
		if (state.signals)
			printf(", %d signal%s", state.signals, state.signals == 1 ? "" : "s");
		printf(", %d error%s\n", state.errors, state.errors == 1 ? "" : "s");
		if (fp != stdin)
			fclose(fp);
	}
	return 0;
}
