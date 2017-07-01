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
 * David Korn
 * AT&T Research
 *
 * strings
 */

static const char usage[] =
"[-?\n@(#)$Id: strings (AT&T Research) 2000-04-01 $\n]"
USAGE_LICENSE
"[+NAME?strings - find and display printable strings in files]"
"[+DESCRIPTION?\bstrings\b searches for printable strings in regular files"
"	and writes those strings to the standard output. A printable string"
"	is any sequence of four (by default) or more printable characters"
"	terminated by a newline or NUL character.]"

"[a:all?Scan the entire file. Always enabled in this implementation.]"
"[l:long-strings?Ignore \anewline\a characters as string terminators and"
"	display strings using C character escape sequences. These strings"
"	are suitably escaped for placement inside C \"...\" and"
"	\bksh\b(1) $'...' string literals.]"
"[m:multi-byte?Scan for multibyte strings.]"
"[n:length|bytes?Set the minimum matched string length to \alength\a. For"
"	compatibility -\anumber\a is equivalent to"
"	\b--length\b=\anumber\a.]#[number:=4]"
"[t:radix|format?Write each string preceded by its byte offset from the"
"	start of the file. The offset radix is determined by:]:[format]{"
"		[+d?decimal]"
"		[+o?octal]"
"		[+x?hexadecimal]"
"}"
"[o:octal?Equivalent to \b--radix=o\b.]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+SEE ALSO?\bgrep\b(1), \bnm\b(1), \bwhat\b(1)]"
;

#include <cmd.h>
#include <ctype.h>

#define MULTIBYTE	0x01		/* must be 1			*/
#define MULTILINE	0x02

#define WIDTH		4

#define	special(c)	(isspace(c) || (c) == '\a' || (c) == '\b')

static int
mapchar(register int c)
{
	switch (c)
	{
	case '\a':
		return('a');
	case '\b':
		return('b');
	case '\f':
		return('f');
	case '\n':
		return('n');
	case '\r':
		return('r');
	case '\t':
		return('t');
	case '\v':
		return('v');
	case '"':
		return('"');
	case '\'':
		return('\'');
	case '\\':
		return('\\');
	}
	return 0;
}

static int
outstring(Sfio_t* out, char* cp, int nbytes, register int flags)
{
	register int	c;
	register int	d;
	register int	n = nbytes;

	while (n-- > 0)
	{
		c = *cp;
		if (flags & MULTIBYTE)
			cp += 2;
		else
			cp++;
		if ((flags & MULTILINE) && (d = mapchar(c)))
		{
			sfputc(out, '\\');
			nbytes++;
			c = d;
		}
		sfputc(out, c);
	}
	sfputc(out, '\n');
	return nbytes + 1;
}

static int
strings(Sfio_t* in, Sfio_t* out, register int width, char* format, register int flags)
{
	register int		n = 0;
	register int		c;
	register unsigned char*	inp;
	register unsigned char*	inend;
	register int		state = 0;
	int			sep;
	off_t			offset;

	char			fmt[64];

	if (format)
	{
		c = strlen(format) - 1;
		if (flags & MULTIBYTE)
			sfsprintf(fmt, sizeof(fmt), "%%%.*sI*%c", c, format, format[c]);
		else
			sfsprintf(fmt, sizeof(fmt), "%%%.*sI*%c %%.*s\n", c, format, format[c]);
		if ((offset = sfseek(in, (off_t)0, SEEK_CUR)) < 0)
			offset = 0;
		offset--;
	}
	sep = (flags & MULTILINE) ? 0 : '\n';
	while ((inp = (unsigned char*)sfgetr(in, sep, 0)) || (inp = (unsigned char*)sfgetr(in, sep, -1)))
	{
		c = sfvalue(in);
		inend = inp+c;
		offset += c;
		for (;;)
		{
			if (inp >= inend || !(c = *inp++) || !isprint(c) && (!(flags & MULTILINE) || !special(c)))
			{
				if (n >= width && !state)
				{
					if (format)
					{
						if (flags & (MULTIBYTE|MULTILINE))
						{
							if (sfprintf(out, fmt, sizeof(offset), offset - (inend - inp) - n) < 0)
								return 0;
							n = outstring(out, (char*)inp - ((flags & MULTIBYTE) + 1) * n - 1, n, flags);
						}
						else
							n = sfprintf(out, fmt, sizeof(offset), offset - (inend - inp) - n, n, inp - n - 1);
					}
					else if (flags & (MULTIBYTE|MULTILINE))
						n = outstring(out, (char*)inp - ((flags & MULTIBYTE) + 1) * n - 1, n, flags);
					else
						n = sfprintf(out, "%.*s\n", n, inp - n - 1);
					if (n < 0)
						return 0;
				}
				if (c || !state)
					n = 0;
				else
					state = 0;
				if (inp >= inend)
					break;
			}
			else if (state)
				n = 0;
			else
			{
				if (flags & MULTIBYTE)
					state = 1;
				n++;
			}
		}
	}
	return 1;
}

int
b_strings(int argc, char** argv, Shbltin_t* context)
{
	register int		n;
	register int		width = WIDTH;
	register int		flags = 0;
	register Sfio_t*	fp;
	register char*		cp;
	register char*		format = 0;

	cmdinit(argc, argv, context, ERROR_CATALOG, 0);
	if (argv[1] && streq(argv[1], "-"))
		argv++;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			/* ignore this */
			continue;
		case 'l':
			flags |= MULTILINE;
			continue;
		case 'm':
			flags |= MULTIBYTE;
			continue;
		case 'n':
			if ((width =  opt_info.num) <= 0)
				error(2, "%d: width must be positive", opt_info.num);
			continue;
		case 'o':
			format = "07d";
			continue;
		case 't':
			for (cp = opt_info.arg; (n = *cp) == '+' || n == '-' || n == '.' || isdigit(n); cp++);
			if (!*(cp + 1) && ((n = *cp) == 'd' || n == 'o' || n == 'x'))
				format = opt_info.arg;
			else
				error(2, "%d: format must be d, o, or x", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (cp = *argv)
		argv++;
	do
	{
		if (!cp || streq(cp, "-"))
			fp = sfstdin;
		else if (!(fp = sfopen(NiL, cp, "r")))
		{
			error(ERROR_system(0), "%s: cannot open", cp);
			error_info.errors = 1;
			continue;
		}
		if (!strings(fp, sfstdout, width, format, flags))
		{
			error(ERROR_system(0), "%s: failed", cp);
			error_info.errors = 1;
		}
		if (fp != sfstdin)
			sfclose(fp);
	} while (cp = *argv++);
	return error_info.errors;
}

