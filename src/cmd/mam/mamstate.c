/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
 * AT&T Bell Laboratories
 * mamexec state support
 *
 * mamstate reference [ file ... | <files ]
 *
 * stdout is list of <file,delta> pairs where delta
 * is diff between reference and file times
 * non-existent files are not listed
 */

static const char id[] = "\n@(#)$Id: mamstate (AT&T Bell Laboratories) 1989-06-26 $\0\n";

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int
main(int argc, register char** argv)
{
	register char*	s;
	char*		id;
	unsigned long	ref;
	struct stat	st;
	char		buf[1024];

	id = "mamstate";
	if (!(s = *++argv))
	{
		fprintf(stderr, "%s: reference file argument expected\n", id);
		return 1;
	}
	if (stat(s, &st))
	{
		fprintf(stderr, "%s: %s: cannot stat\n", id, s);
		return 1;
	}
	ref = (unsigned long)st.st_mtime;
	if (s = *++argv)
		do
		{
			if (!stat(s, &st))
				printf("%s %ld\n", s, (unsigned long)st.st_mtime - ref);
		} while (s = *++argv);
	else
		while (s = fgets(buf, sizeof(buf), stdin))
			if (!stat(s, &st))
				printf("%s %ld\n", s, (unsigned long)st.st_mtime - ref);
	return 0;
}
