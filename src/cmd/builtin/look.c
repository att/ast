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
 * look for lines beginning with <prefix> in sorted a sorted file
 *
 *   David Korn
 */

#ifndef DICT_FILE
#   define DICT_FILE	"/usr/dict/words"
#endif

static const char usage[] =
"[-?@(#)$Id: look (AT&T Research) 2008-02-14 $\n]"
USAGE_LICENSE
"[+NAME?look - displays lines beginning with a given prefix]"
"[+DESCRIPTION?\blook\b displays all lines in the sorted \afile\a arguments"
"	that begin with the given prefix \aprefix\a onto the standard output."
"	The results are unspecified if any \afile\a is not sorted. If"
"	\amax-prefix\a is specified then then records matching prefixes"
"	in the inclusive range \aprefix\a..\amax-prefix\a are displayed.]"
"	[+?If \afile\a is not specified, \blook\b uses the file \b"DICT_FILE"\b"
"	and enables \b--dictionary\b and \b--ignorecase\b.]"
"[d:dictionary?`Phone dictionary order': only letters, digits, and"
"	white space characters are significant in string comparisons.]"
"[f:fold|ignorecase?The search is case insensitive.]"
"[h:header?Skip flat file header (all lines up to first blank line.)]"
"\n"
"\nprefix [- max-prefix] [file ...]\n"
"\n"
"[+EXIT STATUS?]{"
"	[+0?The specified \aprefix\a was found in the file.]"
"	[+1?The specified \aprefix\a was not found in the file.]"
"	[+>1?An error occurred.]"
"}"
"[+SEE ALSO?\bgrep\b(1), \bsort\b(1)]"
;


#include <cmd.h>
#include <ctype.h>

#define D_FLAG		0x01
#define F_FLAG		0x02
#define H_FLAG		0x04

#define CLOSE		256

#define EXTRACT(f,p,b,n)	(((b)&&((f)&D_FLAG))?extract(p,b,n):(p))

static char*
extract(register const char* cp, char* buff, int len)
{
	register char*	bp = buff;
	register char*	ep = bp + len;
	register int	n;

	while (n = *cp++)
	{
		if (n == '\n')
		{
			*bp = 0;
			break;
		}
		if (isalnum(n) || isspace(n))
		{
			*bp++ = n;
			if (bp >= ep)
				break;
		}
	}
	return buff;
}

static int
look(Sfio_t* fp, char* prefix, char* maxprefix, int flags)
{
	Sfoff_t		low;
	Sfoff_t		mid;
	Sfoff_t		high;
	int		n;
	int		len;
	int		found;
	register char*	cp;
	register char*	dp;
	register char*	buff = 0;
	int		(*compare)(const char*, const char*, size_t);

	compare = (flags & F_FLAG) ? strncasecmp : strncmp;
	if (flags & D_FLAG)
	{
		cp = dp = prefix;
		while (n = *cp++)
			if (isalnum(n) || isspace(n))
				*dp++ = n;
		*dp = 0;
		len = strlen(prefix);
		if (maxprefix)
		{
			cp = dp = maxprefix;
			while (n = *cp++)
				if (isalnum(n) || isspace(n))
					*dp++ = n;
			*dp = 0;
			if ((n = strlen(maxprefix)) < len)
				n = len;
		}
		else
			n = len;
		buff = (void*)malloc(n + 1);
	}
	else
		n = len = strlen(prefix);
	if (maxprefix && (*compare)(prefix, maxprefix, n) > 0)
		return 1;
	if (flags & H_FLAG)
		while (sfgetr(fp, '\n', 0) && sfvalue(fp) > 1);
	if ((low = sfseek(fp, (Sfoff_t)0, SEEK_CUR)) < 0 || (high = sfseek(fp, (Sfoff_t)0, SEEK_END)) <= 0)
	{
		found = 0;
		n = 0;
		while (cp = sfgetr(fp, '\n', 0))
		{
			n = (*compare)(prefix, EXTRACT(flags, cp, buff, len), len);
			if (n <= 0)
				break;
		}
		if (!cp)
			return 1;
		if (maxprefix)
		{
			prefix = maxprefix;
			len = strlen(prefix);
			if (n && (*compare)(prefix, EXTRACT(flags, cp, buff, len), len) >= 0)
				n = 0;
		}
		found = !n;
		while (!n)
		{
			sfprintf(sfstdout, "%.*s", sfvalue(fp), cp);
			if (!(cp = sfgetr(fp, '\n', 0)))
				break;
			n = (*compare)(prefix, EXTRACT(flags, cp, buff, len), len);
			if (maxprefix && n > 0)
				n = 0;
		}
	}
	else
	{
		while ((high - low) > (len + CLOSE))
		{
			mid = (low + high) / 2;
			sfseek(fp, mid, SEEK_SET);
			sfgetr(fp, '\n', 0);
			mid = sftell(fp);
			if (mid > high)
				break;
			if (!(cp = sfgetr(fp, '\n', 0)))
				low = mid;
			else
			{
				n = (*compare)(prefix, EXTRACT(flags, cp, buff, len), len);
				if (n < 0)
					high = mid - len;
				else if (n > 0)
					low = mid;
				else
				{
					if((mid+=sfvalue(fp)) >= high)
						break;
					high = mid;
				}
			}
		}
		sfseek(fp, low, SEEK_SET);
		while (low <= high)
		{
			if (!(cp = sfgetr(fp, '\n', 0)))
				return 1;
			n = (*compare)(prefix, EXTRACT(flags, cp, buff, len), len);
			if (n <= 0)
				break;
			low += sfvalue(fp);
		}
		if (maxprefix)
		{
			prefix = maxprefix;
			len = strlen(prefix);
			if (n && (*compare)(prefix, EXTRACT(flags, cp, buff, len), len) >= 0)
				n = 0;
		}
		found = !n;
		while (!n)
		{
			sfprintf(sfstdout, "%.*s", sfvalue(fp), cp);
			if (!(cp = sfgetr(fp, '\n', 0)))
				break;
			n = (*compare)(prefix, EXTRACT(flags, cp, buff, len), len);
			if (maxprefix && n > 0)
				n = 0;
		}
		if (buff)
			free((void*)buff);
	}
	return !found;
}

int
b_look(int argc, char** argv, Shbltin_t* context)
{
	register Sfio_t*	fp;
	register int		n;
	register int		flags = 0;
	char*			ep = 0;
	char*			bp;
	char*			file;

	static const char*	dict[] = { DICT_FILE, "/usr/share/dict/words", "/usr/lib/dict/words" };

	cmdinit(argc, argv, context, ERROR_CATALOG, 0);
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'd':
			flags |= D_FLAG;
			continue;
		case 'f':
			flags |= F_FLAG;
			continue;
		case 'h':
			flags |= H_FLAG;
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || !(bp = *argv++))
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (file = *argv)
	{
		argv++;
		if (streq(file, "-") && (ep = *argv))
		{
			argv++;
			if (streq(ep, "-"))
			{
				file = ep;
				ep = 0;
			}
			else if (file = *argv)
				argv++;
		}
	}
	if (!file)
	{
		for (n = 0; n < elementsof(dict); n++)
			if (!eaccess(dict[n], R_OK))
			{
				file = (char*)dict[n];
				break;
			}
		if (!file)
			error(ERROR_system(1), "%s: not found", dict[0]);
		flags |= (D_FLAG|F_FLAG);
	}
	n = 0;
	do
	{
		if (streq(file, "-") || streq(file, "/dev/stdin") || streq(file, "/dev/fd/0"))
			fp = sfstdin;
		else if (!(fp = sfopen(NiL, file, "r")))
		{
			error(ERROR_system(0), "%s: cannot open", file);
			continue;
		}
		if (look(fp, bp, ep, flags))
			n = 1;
		if (fp != sfstdin)
			sfclose(fp);
	} while (file = *argv++);
	return n || error_info.errors;
}
