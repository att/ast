/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2011 AT&T Intellectual Property          *
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
 * nocom [file ...]
 *
 * nocom -- strip comments from C source files
 */

static const char usage[] =
"[-?\n@(#)$Id: nocom (AT&T Research) 1994-01-11 $\n]"
USAGE_LICENSE
"[+NAME?nocom - strip comments from C source files]"
"[+DESCRIPTION?\bnocom\b strips \b// ...\b and \b/* ... */\b comments from"
"	each C source \afile\a and writes the result on the standard output."
"	Comments that span multiple lines are replaced by \bnewline\b"
"	characters to retain the original source line numbering. If \afile\a"
"	is omitted then the standard input is read.]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+SEE ALSO?\bcc\b(1), \bwc\b(1)]"
;

#include <ast.h>
#include <error.h>

#include "nocomment.c"

int
main(int argc, char** argv)
{
	register char*		s;
	register Sfio_t*	sp;

	NoP(argc);
	error_info.id = "nocom";
	for (;;)
	{
		switch (optget(argv, usage))
		{
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
	if (!*argv)
	{
		if (nocomment(sfstdin, sfstdout) < 0)
			error(ERROR_SYSTEM|2, "write error");
	}
	else while (s = *argv++)
	{
		if (!(sp = sfopen(NiL, s, "r")))
			error(ERROR_SYSTEM|2, "%s: cannot read", s);
		else
		{
			if (nocomment(sp, sfstdout) < 0)
				error(ERROR_SYSTEM|2, "%s: write error", s);
			sfclose(sp);
		}
	}
	return error_info.errors != 0;
}
