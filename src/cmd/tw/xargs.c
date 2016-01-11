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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * xargs -- construct arg list and exec -- use tw instead
 */

static const char usage[] =
"[-?\n@(#)$Id: xargs (AT&T Research) 2012-04-11 $\n]"
USAGE_LICENSE
"[+NAME?xargs - construct arg list and execute command]"
"[+DESCRIPTION?\bxargs\b constructs a command line consisting of the"
"	\acommand\a and \aargument\a operands specified followed by as"
"	many arguments read in sequence from standard input as will fit"
"	in length and number constraints specified by the options and the"
"	local system. \axargs\a executes the constructed command and waits"
"	for its completion. This sequence is repeated until an end-of-file"
"	condition is detected on standard input or an invocation of a"
"	constructed command line returns an exit status of 255. If"
"	\acommand\a is omitted then the equivalent of \b/bin/echo\b is used.]"

"[+?Arguments in the standard input must be separated by unquoted blank"
"	characters, or unescaped blank characters or newline characters."
"	A string of zero or more non-double-quote and non-newline characters"
"	can be quoted by enclosing them in double-quotes. A string of zero or"
"	more non-apostrophe and non-newline characters can be quoted by"
"	enclosing them in apostrophes. Any unquoted character can be escaped"
"	by preceding it with a backslash. The utility will be executed one"
"	or more times until the end-of-file is reached. The results are"
"	unspecified if \acommand\a attempts to read from its standard input.]"

"[e:eof?Set the end of file string. The first input line matching this string"
"	terminates the input list. There is no eof string if \astring\a is"
"	omitted. The default eof string is \b_\b if neither \b--eof\b nor"
"	\b-E\b are specified. For backwards compatibility \astring\a"
"	must immediately follow the \b-e\b option flag; \b-E\b follows"
"	standard option syntax.]:?[string]"
"[i:insert|replace?Replace occurences of \astring\a in the command"
"	arguments with names read from the standard input. Implies"
"	\b--exit\b and \b--lines=1\b. For backwards compatibility \astring\a"
"	must immediately follow the \b-i\b option flag; \b-I\b follows"
"	standard option syntax.]:?[string:={}]"
"[l:lines|max-lines?Use at most \alines\a lines from the standard input."
"	Lines with trailing blanks logically continue onto the"
"	next line. For backwards compatibility \alines\a"
"	must immediately follow the \b-l\b option flag; \b-L\b follows"
"	standard option syntax.]#?[lines:=1]"
"[n:args|max-args?Use at most \aargs\a arguments per command line."
"	Fewer than \aargs\a will be used if \b--size\b is exceeded.]#[args]"
"[p:interactive|prompt?Prompt to determine if each command should execute."
"	A \by\b or \bY\b recsponse executes, otherwise the command is skipped."
"	Implies \b--verbose\b.]"
"[N|0:null?The file name list is NUL terminated; there is no other special"
"	treatment of the list.]"
"[s:size|max-chars?Use at most \achars\a characters per command. The default"
"	is as large as possible.]#[chars]"
"[t:trace|verbose?Print the command line on the standard error"
"	before executing it.]"
"[x:exit?Exit if \b--size\b is exceeded.]"
"[X:exact?If \b--args=\b\aargs\a was specified then terminate before the last"
"	command if it would run with less than \aargs\a arguments.]"
"[z:nonempty|no-run-if-empty?If no file names are found then do not execute"
"	the command. By default the command is executed at least once.]"
"[E?Equivalent to \b--eof=string\b.]:[string]"
"[I?Equivalent to \b--insert=string\b.]:[string]"
"[L?Equivalent to \b--lines=number\b.]#[number]"

"\n"
"\n[ command [ argument ... ] ]\n"
"\n"

"[+EXIT STATUS]{"
"	[+0?All invocations of \acommand\a returned exit status 0.]"
"	[+1-125?A command line meeting the specified requirements could not"
"		be assembled, one or more of the invocations of \acommand\a"
"		returned  non-0 exit status, or some other error occurred.]"
"	[+126?\acommand\a was found but could not be executed.]"
"	[+127?\acommand\a was not found.]"
"}"
"[+SEE ALSO?\bfind\b(1), \btw\b(1)]"
;

#include <ast.h>
#include <ctype.h>
#include <error.h>

#include "cmdarg.h"

int
main(int argc, register char** argv)
{
	register int		c;
	register int		q;
	register char*		s;
	register Sfio_t*	sp;
	register Cmdarg_t*	cmd;

	int			argmax = 0;
	char*			eof = "_";
	char*			insert = 0;
	int			lines = 0;
	size_t			size = 0;
	int			term = -1;

	Cmddisc_t		disc;

	NoP(argc);
	CMDDISC(&disc, CMD_EMPTY|CMD_EXIT, errorf);
	error_info.id = "xargs";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'e':
			/*
			 * backwards compatibility requires no space between
			 * option and value
			 */

			if (opt_info.arg == argv[opt_info.index - 1])
			{
				opt_info.arg = 0;
				opt_info.index--;
			}
			/*FALLTHROUGH*/
		case 'E':
			eof = opt_info.arg;
			continue;
		case 'i':
			/*
			 * backwards compatibility requires no space between
			 * option and value
			 */

			if (opt_info.arg == argv[opt_info.index - 1])
			{
				opt_info.arg = 0;
				opt_info.index--;
			}
			/*FALLTHROUGH*/
		case 'I':
			insert = opt_info.arg ? opt_info.arg : "{}";
			disc.flags |= CMD_INSERT;
			term = '\n';
			continue;
		case 'l':
			/*
			 * backwards compatibility requires no space between
			 * option and value
			 */

			if (opt_info.arg == argv[opt_info.index - 1])
			{
				opt_info.arg = 0;
				opt_info.index--;
			}
			/*FALLTHROUGH*/
		case 'L':
			argmax = opt_info.num ? opt_info.num : 1;
			lines = 1;
			continue;
		case 'n':
			argmax = opt_info.num;
			continue;
		case 'p':
			disc.flags |= CMD_QUERY;
			continue;
		case 's':
			size = opt_info.num;
			continue;
		case 't':
			disc.flags |= CMD_TRACE;
			continue;
		case 'x':
			disc.flags |= CMD_MINIMUM;
			continue;
		case 'z':
			disc.flags &= ~CMD_EMPTY;
			continue;
		case 'D':
			error_info.trace = -opt_info.num;
			continue;
		case 'N':
			term = 0;
			continue;
		case 'X':
			disc.flags |= CMD_EXACT;
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
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!(cmd = cmdopen(argv, argmax, size, insert, &disc)))
		error(ERROR_SYSTEM|3, "out of space [cmd]");
	if (!(sp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [arg]");
	sfopen(sfstdin, NiL, "rt");
	error_info.line = 1;
	if (term >= 0)
	{
		while (s = sfgetr(sfstdin, term, 0))
		{
			error_info.line++;
			if ((c = sfvalue(sfstdin) - 1) && (s[c-1] != '\r' || --c))
				cmdarg(cmd, s, c);
		}
		if (sfvalue(sfstdin) > 0)
			error(2, "last argument incomplete");
	}
	else
		for (;;)
		{
			switch (c = sfgetc(sfstdin))
			{
			case '"':
			case '\'':
				q = c;
				while ((c = sfgetc(sfstdin)) != q)
				{
					if (c == EOF)
						goto arg;
					if (c == '\n')
					{
						error(1, "missing %c quote", q);
						error_info.line++;
						goto arg;
					}
					sfputc(sp, c);
				}
				continue;
			case '\\':
				if ((c = sfgetc(sfstdin)) == EOF)
				{
					if (sfstrtell(sp))
						goto arg;
					break;
				}
				if (c == '\n')
					error_info.line++;
				sfputc(sp, c);
				continue;
			case EOF:
				if (sfstrtell(sp))
					goto arg;
				break;
			case '\n':
				error_info.line++;
			arg:
				c = sfstrtell(sp);
				if (!(s = sfstruse(sp)))
					error(ERROR_SYSTEM|3, "out of space");
				if (eof && streq(s, eof))
					break;
				if (c || insert)
				{
					if (lines && c > 1 && isspace(s[c - 2]))
						cmdarg(cmd, 0, -1);
					cmdarg(cmd, s, c);
				}
				continue;
			default:
				if (isspace(c))
					goto arg;
				sfputc(sp, c);
				continue;
			}
			break;
		}
	if (sferror(sfstdin))
		error(ERROR_SYSTEM|2, "input read error");
	cmdclose(cmd);
	exit(error_info.errors != 0);
}
