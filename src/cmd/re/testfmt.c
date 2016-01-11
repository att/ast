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
 * fmtre(3) and fmtmatch(3) test harness
 * see help() for details
 */

static const char id[] = "\n@(#)$Id: testfmt (AT&T Research) 2012-06-25 $\0\n";

#include <ast.h>
#include <ctype.h>
#include <setjmp.h>
#include <sig.h>

#define LOOPED		2

#define ERE		1
#define KRE		2

typedef char* (*Call_f)(const char*);

#define H(x)		sfprintf(sfstderr,x)

static void
help(void)
{
H("NAME\n");
H("  testfmt - fmtre(3) and fmtmatch(3) test harness\n");
H("\n");
H("SYNOPSIS\n");
H("  testfmt [ options ] < testfmt.dat\n");
H("\n");
H("DESCRIPTION\n");
H("  testfmt reads test specifications, one per line, from the standard\n");
H("  input and writes one output line for each failed test. A summary\n");
H("  line is written after all tests are done.\n");
H("\n");
H("OPTIONS\n");
H("  -c	catch signals and non-terminating calls\n");
H("  -h	list help\n");
H("  -v	list each test line\n");
H("\n");
H("INPUT FORMAT\n");
H("  Input lines may be blank, a comment beginning with #, or a test\n");
H("  specification. A specification is three fields separated by one\n");
H("  or more tabs. NULL denotes the empty string and NIL denotes the\n");
H("  0 pointer.\n");
H("\n");
H("  Field 1: the pattern type:\n");
H("\n");
H("    E 	ERE		(egrep)\n");
H("    K	KRE		(ksh glob)\n");
H("    i	invert		test the inverse call too\n");
H("\n");
H("    $	                expand C \\c escapes in fields 2 and 3\n");
H("    : comment		comment copied to output\n");
H("\n");
H("  Field 2: the regular expression pattern.\n");
H("\n");
H("  Field 3: the converted pattern to match.\n");
H("\n");
H("  Field 4: optional comment appended to the report.\n");
H("\n");
H("CONTRIBUTORS\n");
H("  Glenn Fowler <gsf@research.att.com>\n");
H("  David Korn <dgk@research.att.com>\n");
}

static struct
{
	int		errors;
	int		lineno;
	int		signals;
	int		warnings;
	char*		file;
	jmp_buf		gotcha;
} state;

static void
quote(char* s, int expand)
{
	unsigned char*	u = (unsigned char*)s;
	int		c;

	if (!u)
		sfprintf(sfstdout, "NIL");
	else if (!*u)
		sfprintf(sfstdout, "NULL");
	else if (expand)
	{
		sfprintf(sfstdout, "\"");
		for (;;)
		{
			switch (c = *u++)
			{
			case 0:
				break;;
			case '\\':
				sfprintf(sfstdout, "\\\\");
				continue;
			case '"':
				sfprintf(sfstdout, "\\\"");
				continue;
			case '\a':
				sfprintf(sfstdout, "\\a");
				continue;
			case '\b':
				sfprintf(sfstdout, "\\b");
				continue;
			case '\f':
				sfprintf(sfstdout, "\\f");
				continue;
			case '\n':
				sfprintf(sfstdout, "\\n");
				continue;
			case '\r':
				sfprintf(sfstdout, "\\r");
				continue;
			case '\t':
				sfprintf(sfstdout, "\\t");
				continue;
			case '\v':
				sfprintf(sfstdout, "\\v");
				continue;
			default:
				if (!iscntrl(c) && isprint(c))
					sfputc(sfstdout, c);
				else
					sfprintf(sfstdout, "\\x%02x", c);
				continue;
			}
			break;
		}
		sfprintf(sfstdout, "\"");
	}
	else
		sfprintf(sfstdout, "%s", s);
}

static void
report(char* comment, char* fun, char* re, char* msg, int expand)
{
	if (state.file)
		sfprintf(sfstdout, "%s:", state.file);
	sfprintf(sfstdout, "%d:", state.lineno);
	if (fun)
		sfprintf(sfstdout, " %s", fun);
	if (re)
	{
		sfprintf(sfstdout, " ");
		quote(re, expand);
	}
	state.errors++;
	sfprintf(sfstdout, " %s", comment);
	if (msg && comment[strlen(comment)-1] != '\n')
		sfprintf(sfstdout, "%s: ", msg);
}

static void
bad(char* comment, char* re, char* s, int expand)
{
	sfprintf(sfstdout, "bad test case ");
	report(comment, NiL, re, s, expand);
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
				if (*t = *++s)
					*t &= 037;
				else
					s--;
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
				q = *s == 'u' ? (s + 4) : (char*)0;
				c = 0;
				e = s;
				while (!e || !q || s < q)
				{
					switch (*s)
					{
					case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
						c = (c << 4) + *s++ - 'a' + 10;
						continue;
					case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
						c = (c << 4) + *s++ - 'A' + 10;
						continue;
					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						c = (c << 4) + *s++ - '0';
						continue;
					case '{':
					case '[':
						if (s != e)
							break;
						e = 0;
						s++;
						if (q && *s == 'U' && *(s + 1) == '+')
							s += 2;
						continue;
					case '}':
					case ']':
						if (!e)
							s++;
						break;
					default:
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
gotcha(int sig)
{
	signal(sig, gotcha);
	alarm(0);
	state.signals++;
	sigunblock(sig);
	longjmp(state.gotcha, sig);
}

int
main(int argc, char** argv)
{
	Call_f		call;
	int		expand;
	int		type;
	int		invert;
	int		i;
	int		subunitlen;
	int		testno;
	Sfio_t*		fp;
	char*		p;
	char*		spec;
	char*		re;
	char*		s;
	char*		ans;
	char*		msg;
	char*		fun;
	char*		subunit;
	char*		version;
	char*		field[5];
	char		unit[64];

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
			case 'v':
				verbose = 1;
				continue;
			default:
				sfprintf(sfstderr, "%s: -%c: invalid option", unit, *p);
				return 1;
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
			fp = sfstdin;
		}
		else if (!(fp = sfopen(NiL, state.file, "r")))
		{
			sfprintf(sfstderr, "%s: %s: cannot read\n", unit, state.file);
			return 2;
		}
		sfprintf(sfstdout, "TEST\t%s ", unit);
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
				sfprintf(sfstdout, "%-.*s ", subunitlen, subunit);
		}
		else
			subunit = 0;
		sfprintf(sfstdout, "%s", version);
		if (catch)
			sfprintf(sfstdout, ", catch");
		if (verbose)
			sfprintf(sfstdout, ", verbose");
		sfprintf(sfstdout, "\n");
		testno = state.errors = state.lineno = state.signals = state.warnings = 0;
		while (p = sfgetr(fp, '\n', 1))
		{
			state.lineno++;

		/* parse: */

			if (*p == 0 || *p == '#')
				continue;
			if (*p == ':')
			{
				while (*++p == ' ');
				sfprintf(sfstdout, "NOTE	%s\n", p);
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

			expand = invert = type = 0;
			for (p = spec; *p; p++)
			{
				switch (*p)
				{
				case 'E':
					type = ERE;
					continue;
				case 'K':
					type = KRE;
					continue;

				case 'i':
					invert = 1;
					continue;

				case '$':
					expand = 1;
					continue;

				default:
					bad("bad spec\n", spec, NiL, 0);
					break;

				}
				break;
			}
			if (i < 3)
				bad("too few fields\n", NiL, NiL, 0);
			while (i < elementsof(field))
				field[i++] = 0;
			if ((re = field[1]) && expand)
				escape(re);
			if ((ans = field[2]) && expand)
				escape(s);
			msg = field[3];
			sfsync(sfstdout);

			for (;;)
			{
				if (type == ERE)
				{
					fun = "fmtmatch";
					call = fmtmatch;
				}
				else if (type == KRE)
				{
					fun = "fmtre";
					call = fmtre;
				}
				else
					break;
				testno++;
				if (verbose)
					sfprintf(sfstdout, "test %-3d %s \"%s\" \"%s\"\n", state.lineno, fun, re, ans ? ans : "NIL");
				if (!catch)
					s = (*call)(re);
				else if (setjmp(state.gotcha))
					s = "SIGNAL";
				else
				{
					alarm(LOOPED);
					s = (*call)(re);
					alarm(0);
				}
				if (!s && ans || s && !ans || s && ans && !streq(s, ans))
				{
					report("failed: ", fun, re, msg, expand);
					quote(ans, expand);
					sfprintf(sfstdout, " expected, ");
					quote(s, expand);
					sfprintf(sfstdout, " returned\n");
				}
				if (!invert)
					break;
				invert = 0;
				s = ans;
				ans = re;
				re = s;
				type = (type == ERE) ? KRE : ERE;
			}
		}
		sfprintf(sfstdout, "TEST\t%s", unit);
		if (subunit)
			sfprintf(sfstdout, " %-.*s", subunitlen, subunit);
		sfprintf(sfstdout, ", %d test%s", testno, testno == 1 ? "" : "s");
		if (state.warnings)
			sfprintf(sfstdout, ", %d warning%s", state.warnings, state.warnings == 1 ? "" : "s");
		if (state.signals)
			sfprintf(sfstdout, ", %d signal%s", state.signals, state.signals == 1 ? "" : "s");
		sfprintf(sfstdout, ", %d error%s\n", state.errors, state.errors == 1 ? "" : "s");
		if (fp != sfstdin)
			sfclose(fp);
	}
	return 0;
}
