/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
 * html2db - extract flat file database from html tables
 */

static const char usage[] =
"[-?\n@(#)$Id: html2db (AT&T Research) 1998-11-10 $\n]"
USAGE_LICENSE
"[+NAME?html2db - extract flat file database from html tables]"
"[+DESCRIPTION?\bhtml2db\b extracts a flat file database from tables in the"
"	input \bhtml\b \afile\as. If \afile\a is not specified then the"
"	standard input is read. The \bhtml\b parse is rudimentary; don't use"
"	\bhtml2db\b to detect valid \bhtml\b files.]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+SEE ALSO?\bhtml2rtf\b(1)]"
;
#include <ast.h>
#include <ctype.h>
#include <error.h>

static void
flatten(const char* path, Sfio_t* ip, Sfio_t* op)
{
	register int	c;
	register int	q;
	register int	p;
	register int	b;
	register char*	s;

	char		tag[256];

	b = p = 0;
	for (;;)
	{
		switch (c = sfgetc(ip))
		{
		case EOF:
			break;
		case '<':
			q = 0;
			s = tag;
			for (;;)
			{
				switch (c = sfgetc(ip))
				{
				case EOF:
					return;
				case '>':
					sfungetc(ip, c);
					break;
				default:
					if (isspace(c))
						break;
					if (s < &tag[sizeof(tag)-1])
						*s++ = islower(c) ? toupper(c) : c;
					continue;
				}
				break;
			}
			*s = 0;
			q = 0;
			for (;;)
			{
				switch (c = sfgetc(ip))
				{
				case EOF:
					return;
				case '\'':
				case '"':
					if (q == c)
						q = 0;
					else if (q == 0)
						q = c;
					continue;
				case '>':
					if (q == 0)
						break;
					continue;
				default:
					continue;
				}
				break;
			}
			s = tag;
			if (s[0] == 'T' && s[1] == 'D' && s[2] == 0)
				p = 1;
			else if (s[0] == '/' && s[1] == 'T')
			{
				if (s[2] == 'D' && s[3] == 0)
				{
					b = p = 0;
					sfputc(op, ';');
				}
				else if (s[2] == 'R' && s[3] == 0)
					sfputc(op, '\n');
			}
			continue;
		default:
			if (p)
			{
				if (isspace(c))
				{
					if (b)
						continue;
					b = 1;
					c = ' ';
				}
				else
					b = 0;
				sfputc(op, c);
			}
			continue;
		}
		break;
	}
}

int
main(int argc, char** argv)
{
	register char*		s;
	register Sfio_t*	ip;

	NoP(argc);
	error_info.id = "html2db";
	for (;;)
	{
		switch (optget(argv, usage))
		{
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
	do
	{
		if (!(s = *argv) || streq(s, "-") || streq(s, "/dev/stdin") || streq(s, "/dev/fd/0"))
		{
			s = "/dev/stdin";
			ip = sfstdin;
		}
		else if (!(ip = sfopen(NiL, s, "r")))
		{
			error(ERROR_SYSTEM|2, "%s: cannot read", s);
			continue;
		}
		flatten(s, ip, sfstdout);
		if (ip != sfstdin)
			sfclose(ip);
	} while (*argv++);
	return error_info.errors != 0;
}
