/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2011 AT&T Intellectual Property          *
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

#include <ast.h>
#include <error.h>
#include <sfdcgzip.h>

static const char usage[] =
"[-?\n@(#)$Id: funzip (AT&T Research) 1998-08-11 $\n]"
USAGE_LICENSE
"[+NAME?funzip - fast gunzip]"
"[+DESCRIPTION?\bfunzip\b decompresses \bgzip\b(1) compressed files."
"	By default \bgzip\b crc32 cyclic redundancy checking is disabled."
"	This may speed up decompression by 25% or more over \bgunzip\b(1)."
"	Most data corruption errors are still caught even with crc disabled.]"

"[x:crc?Enable \agzip\a crc32 cyclic redundancy checking for decompress."
"	On some systems this can double the execution wall time."
"	Most data corruption errors are still caught even with \bnocrc\b.]"

"\n"
"\n[ file ]\n"
"\n"

"[+SEE ALSO?\bgzip\b(1), \bgunzip\b(1), \blibz\b(3)]"
;

int
main(int argc, char** argv)
{
	Sfio_t*	sp;
	char*	file;
	int	r;

	int	flags = SFGZ_NOCRC;

	error_info.id = "funzip";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'x':
			flags &= ~SFGZ_NOCRC;
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
	if (error_info.errors || *argv && *(argv + 1))
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!(file = *argv))
	{
		file = "/dev/stdin";
		sp = sfstdin;
	}
	else if (!(sp = sfopen(NiL, file, "r")))
		error(ERROR_SYSTEM|3, "%s: cannot read", file);
	if ((r = sfdcgzip(sp, flags)) < 0)
		error(3, "sfdcgzip discipline push error");
	else if (!r)
		error(3, "input not a gzip file");
	if (sfmove(sp, sfstdout, SF_UNBOUND, -1) < 0 || sfsync(sfstdout))
		error(ERROR_SYSTEM|3, "sfdcgzip io error");
	return 0;
}
