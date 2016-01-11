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
/*
 * Glenn Fowler
 * AT&T Research
 */

static const char usage[] =
"[-?\n@(#)$Id: iconv (AT&T Research) 2012-10-15 $\n]"
USAGE_LICENSE
"[+NAME?iconv - codeset conversion]"
"[+DESCRIPTION?\biconv\b converts the encoding of characters in the \afile\a"
"	operands from one codeset to another and writes the results to"
"	the standard output. If \afile\a is \b-\b or omitted then the"
"	standard input is read.]"
"[+?Character encodings in either codeset may include single-byte values"
"	(for example, for the ISO 8859-1:1987 standard characters) or"
"	multi-byte values (for example, for certain characters in the"
"	ISO 6937:1983 standard). Invalid characters in the input stream"
"	(either those that are not valid members of the input codeset or"
"	those that have no corresponding value in the output codeset) are"
"	output as the underscore character (\b_\b) in the output codeset.]"
"[+?The \bnative\b codeset is determined by the \bLANG\b, \bLC_ALL\b and"
"	\bLC_CTYPE\b environment variables. The supported codesets"
"	are matched by these left-anchored case-insensitive \bksh\b(1)"
"	patterns:]{\fcodesets\f}"
"[+?Conversion between certain codesets may not be supported. Also, since the"
"	standard(s) provide no support for listing the known codesets, the"
"	above list may be incomplete.]"

"[a:all?List all conversion errors. By default (and \b--omit\b is not "
    "specified) \biconv\b stops after the first error.]"
"[c:omit?Omit invalid input characters from the output. Invalid input "
    "characters still affect the exit status.]"
"[e:errors?Do not ignore conversion errors.]"
"[f:from?The input codeset is set to \acodeset\a.]:[codeset:=native]"
"[i:ignore?Ignore conversion errors.]"
"[l:list?List all known codesets on the standard output.]"
"[s:silent?Suppress invalid character diagnostics. Invalid input "
    "characters still affect the exit status. If \b--all\b is also specified "
    "then non-zero invalid character counts are listed.]"
"[t:to?The output codeset is set to \acodeset\a.]:[codeset:=native]"

"\n"
"\n[ pid ... ]\n"
"\n"

"[+SEE ALSO?\bdd\b(1), \biconv\b(3), \bsetlocale\b(3)]"
;

#include <cmd.h>
#include <iconv.h>

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register iconv_list_t*	ic;
	register const char*	p;
	register int		c;

	if (streq(s, "codesets"))
		for (ic = iconv_list(NiL); ic; ic = iconv_list(ic))
		{
			sfputc(sp, '[');
			sfputc(sp, '+');
			sfputc(sp, '\b');
			p = ic->match;
			if (*p == '(')
				p++;
			while (c = *p++)
			{
				if (c == ')' && !*p)
					break;
				if (c == '?' || c == ']')
					sfputc(sp, c);
				sfputc(sp, c);
			}
			sfputc(sp, '?');
			p = ic->desc;
			while (c = *p++)
			{
				if (c == ']')
					sfputc(sp, c);
				sfputc(sp, c);
			}
			sfputc(sp, ']');
		}
	return 0;
}

static int
listall(void* context)
{
	
	register iconv_list_t*	ic;
	register const char*	p;

	sfprintf(sfstdout, "Patterns:\n\n");
	for (ic = iconv_list(NiL); ic; ic = iconv_list(ic))
		sfprintf(sfstdout, "  %s -- %s\n", ic->match, ic->desc);
	p = "/usr/bin/iconv";
	if (!access(p, X_OK) || !access(p += 4, X_OK))
	{
		char*	argv[3];

		sfprintf(sfstdout, "\n");
		argv[0] = (char*)p;
		argv[1] = "-l";
		argv[2] = 0;
		return sh_run(context, 2, argv);
	}
	return 0;
}

static int
checksig(void* context)
{
	return sh_checksig(context);
}

int
b_iconv(int argc, register char** argv, Shbltin_t* context)
{
	char*		file;
	char*		from;
	char*		to;
	iconv_t		cvt;
	int		all;
	int		fail;
	int		ignore;
	int		list;
	Sfio_t*		ip;
	Optdisc_t	od;
	Iconv_disc_t	id;

	cmdinit(argc, argv, context, ERROR_CATALOG, ERROR_NOTIFY);
	error_info.id = "iconv";
	from = to = "native";
	all = ignore = list = 0;

	/*
	 * set up the disciplines
	 */

	optinit(&od, optinfo);
	iconv_init(&id, errorf);
	id.flags |= ICONV_FATAL;
	id.handle = context;
	id.checksig = checksig;

	/*
	 * grab the options
	 */

	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			all = 1;
			id.flags &= ~ICONV_FATAL;
			continue;
		case 'c':
			id.flags |= ICONV_OMIT;
			id.flags &= ~ICONV_FATAL;
			continue;
		case 'e':
			ignore = 0;
			continue;
		case 'f':
			from = opt_info.arg;
			continue;
		case 'i':
			ignore = 1;
			continue;
		case 'l':
			list = 1;
			continue;
		case 's':
			id.errorf = 0;
			continue;
		case 't':
			to = opt_info.arg;
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
	if (list)
		return listall(context);
	if ((cvt = iconv_open(to, from)) == (iconv_t)(-1))
	{
		if ((cvt = iconv_open(to, "utf-8")) == (iconv_t)(-1))
			error(3, "%s: unknown destination codeset", to);
		iconv_close(cvt);
		if ((cvt = iconv_open("utf-8", from)) == (iconv_t)(-1))
			error(3, "%s: unknown source codeset", from);
		iconv_close(cvt);
		error(3, "cannot convert from %s to %s", from, to);
	}
	fail = 0;
	if (file = *argv)
		argv++;
	do
	{
		if (!file || streq(file, "-"))
		{
			file = "/dev/stdin";
			ip = sfstdin;
		}
		else if (!(ip = sfopen(NiL, file, "r")))
		{
			error(ERROR_SYSTEM|2, "%s: cannot open", file);
			continue;
		}
		id.errors = 0;
		iconv_move(cvt, ip, sfstdout, SF_UNBOUND, &id);
		if (!id.errors && (!sfeof(ip) || sferror(ip)))
			error(ERROR_SYSTEM|2, "%s: conversion read error", file);
		if (id.errors)
		{
			if (ignore || !id.errors)
				fail = 1;
			else if (!id.errorf && all)
			{
				if (id.errors == 1)
					error(2, "%s: %d character conversion error", file, id.errors);
				else if (id.errors)
					error(2, "%s: %d character conversion errors", file, id.errors);
			}
		}
		if (ip != sfstdin)
			sfclose(ip);
	} while (file = *argv++);
	if (sfsync(sfstdout))
		error(ERROR_SYSTEM|3, "conversion write error");
	return error_info.errors != 0 || fail;
}
