/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2013 AT&T Intellectual Property          *
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
 * split.c
 * David Korn
 * AT&T Research
 */

static const char split_usage[] =
"[-?\n@(#)$Id: split (AT&T Research) 2006-09-19 $\n]"
USAGE_LICENSE
"[+NAME?split - split files into pieces]"
"[+DESCRIPTION?\bsplit\b reads an input file and writes one or more "
    "output files so that \bcat\b(1) on these files will produce the input "
    "file. The default size for each piece is 1000 lines. The suffix "
    "consists of \asuffix_len\a lower case characters from the POSIX "
    "locale.]"
"[+?If \aname\a is specified then it will be used as a prefix for each "
    "of the resulting files from the split operation; otherwise the prefix "
    "\bx\b will be used.]"
"[+?If no \afile\a is given, or if the \afile\a is \b-\b, \bsplit\b "
    "copies from standard input starting at the current location.]"
"[+?The option arguments for \b-b\b and \b-C\b can optionally be "
    "followed by one of the following characters to specify a different unit "
    "other than a single byte:]"
    "{"
        "[+b?512 bytes.]"
        "[+k?1-killobytes.]"
        "[+m?1-megabyte.]"
        "[+g?1-gigabyte.]"
        "[+t?1-terabyte.]"
    "}"
"[+?For backwards compatibility, \b-\b\aline_count\a is equivalent to "
    "\b-l\b \aline_count\a.]"
"[l:lines]#[line_count:=1000?\aline_count\a specified the number of "
    "lines for each piece except the last. If the input does not end in a "
    "newline, the partial line is included in the last piece.]"
"[a|n:suffix-length]#[suffix_len:=2?\asuffix_len\a defines the number of "
    "letters that form the suffix portion of the file names for each of the "
    "pieces that the file is split into.]"
"[b:bytes]#[n?Splits the file into byte size pieces defined by \an\a "
    "rather than lines.]"
"[C:line-bytes]#[n?Splits the file into lines totaling at most \an\a "
    "bytes.]"
"\n"
"\n[ file [ name ] ]\n"
"\n"
"[+EXIT STATUS]"
    "{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
    "}"
"[+SEE ALSO? \bcsplit\b(1), \bcat\b(1)]"
;

static const char csplit_usage[] =
"[-?\n@(#)$Id: csplit (AT&T Research) 2003-08-21 $\n]"
USAGE_LICENSE
"[+NAME?csplit - split a file into sections determined by context lines]"
"[+DESCRIPTION?\bcsplit\b creates zero or more output files containing"
"	sections of the given input \afile\a, or the standard input if the"
"	name \b-\b is given. By default, \bcsplit\b prints the number of"
"	bytes written to each output file after it has been created.]"
"[+?The contents of the output files are determined by the \apattern\a"
"	arguments. An error occurs if a pattern argument refers to a"
"	nonexistent line of the input file, such as if no remaining line"
"	matches a given regular expression.  After all the given patterns have"
"	been matched, any remaining output is copied into one last output"
"	file. The types of pattern arguments are:]{"
"		[+line?Create an output file containing the current line up"
"			to (but not including) line \aline\a (a positive"
"			integer) of the input file. If followed by a repeat"
"			count, also create an output file containing the"
"			next \aline\a lines of the input file once for each"
"			repeat.]"
"		[+/regexp/[offset]]?Create an output file containing the"
"			current line up to (but not including) the next line"
"			of the input file that contains a match for"
"			\aregexp\a. The optional \aoffset\a is a \b+\b or"
"			\b-\b followed by a positive integer. If it is given,"
"			the input up to the matching line plus or minus"
"			\aoffset\a is put into the output file, and the line"
"			after that begins the next section of input.]"
"		[+%regexp%[offset]]?Like the previous type, except that it"
"			does not create an output file, so that section of"
"			the input file is effectively ignored.]"
"		[+{repeat-count}?Repeat the previous pattern \arepeat-count\a"
"			(a positive integer) additional times. An asterisk"
"			may be given in place of the (integer) repeat count,"
"			in which case the preceeding pattern is repeated as"
"			many times as necessary until the input is exausted.]"
"	}"
"[+?The output file names consist of a prefix followed by a suffix. By"
"	default, the suffix is merely an ascending linear sequence of two-digit"
"	decimal numbers starting with 00 and ranging up to 99, however this"
"	default may be overridden by either the \b--digits\b option or by the"
"	\b--suffix-format\b option (see below.) In any case, concatenating"
"	the output files in sorted order by file name produces the original"
"	input file, in order. The default output file name prefix is \bxx\b.]"
"[+?By default, if \bcsplit\b encounters an error or receives a hangup,"
"	interrupt, quit, or terminate signal, it removes any output files"
"	that it has created so far before it exits.]"
"[b:suffix-format?Use the \bprintf\b(3) \aformat\a to generate the file"
"	name suffix.]:[format:=\b%02d\b]"
"[f:prefix?Use \aprefix\a to generate the file name prefix.]:[prefix:=\bxx\b]"
"[k:keep-files?Do not remove output files on errors.]"
"[a|n:digits?Use \adigits\a in the generated file name suffixes.]#[digits:=2]"
"[s:silent|quiet?Do not print output file counts and sizes.]"
"[z:elide-empty-files?Remove empty output files.]"
"\n"
"\nfile arg ...\n"
"\n"
"[+EXIT STATUS?]{"
"	[+0?Successful completion.]"
"	[+>0?An error occurred.]"
"}"
"[+SEE ALSO? \bsplit\b(1), \bcat\b(1)]"
;

#include <cmd.h>
#include <regex.h>

#define	S_FLAG		001
#define	K_FLAG		002
#define	C_FLAG		004
#define	B_FLAG		010
#define	Z_FLAG		020
#define	M_FLAG		040

#define OP_LINES	0
#define OP_SEARCH	1
#define OP_SKIP		2
#define OP_ABSOLUTE	3

#define BLK_SIZE	2048

struct fname
{
	char*		fname;
	char*		format;
	char*		suffix;
	char*		last;
	char		low;
	char		high;
	int		count;
};

struct op
{
	struct op*	next;
	Sfoff_t		size;
	size_t		repeat;
	int		flags;
	regex_t*	re;
};

/*
 * create an operation structure
 */
static struct op*
getop(struct op** prev, Sfoff_t size, size_t repeat, int flags, int re)
{
	struct op*	op;

	if (op = newof(0, struct op, 1, re ? sizeof(regex_t) : 0))
	{
		op->repeat = repeat;
		op->flags = flags;
		op->size = size;
		op->next = 0;
		if (re)
			op->re = (regex_t*)(op + 1);
		*prev = op;
	}
	return op;
}

/*
 * process /expr/offset arguments
 * returns new operation structure which is added to linked list
 */

static struct op*
getexpr(struct op** prev, const char* arg)
{
	char*		cp = (char*)arg;
	char*		ep;
	int		n;
	struct op*	op;

	if (op = getop(prev, 0, 1, *cp == '/' ? OP_SEARCH : OP_SKIP, 1))
	{
		if (n = regcomp(op->re, cp, REG_DELIMITED|REG_NOSUB))
		{
			regfatal(op->re, 2, n);
			return 0;
		}
		cp += op->re->re_npat;
		if (*cp)
		{
			op->size = strtoll(cp, &ep, 10);
			if (*ep)
				error(ERROR_exit(1), "%s: invalid offset", cp);
		}
	}
	return op;
}

/*
 * set up file name generator whose form is <prefix>... where ... is
 * suflen characters from low..high
 * returns a pointer to a structure that can be used to create
 * file names
 */

static struct fname*
setfname(const char* prefix, char* format, int suflen, int low, int high)
{
	struct fname*	fp;
	int		flen;
	int		slen;
	int		len;
	char*		cp;

	flen = strlen(prefix);
	len = flen + suflen + 1;
	if (format)
	{
		slen = strlen(format);
		len += flen + slen + 1;
	}
	else
		slen = 0;
	if (fp = newof(0, struct fname, 1, len))
	{
		cp = (char*)(fp + 1);
		if (format)
		{
			strcpy(fp->format = cp, prefix);
			cp += flen;
			strcpy(cp, format);
			cp += slen + 1;
		}
		fp->low = low;
		fp->high = high;
		fp->count = 0;
		strcpy(fp->fname = cp, prefix);
		cp += flen;
		fp->suffix = cp;
		while (suflen-- > 0)
			*cp++ = low;
		*cp-- = 0;
		fp->last = cp;
		(*cp)--;
		flen = _POSIX_NAME_MAX;
		if (cp = strrchr(fp->fname, '/'))
		{
			cp++;
			len = strlen(cp);
			if (len > flen)
			{
				*(cp - 1) = 0;
				flen = (int)strtol(astconf("NAME_MAX", fp->fname, NiL), NiL, 0);
				*(cp - 1) = '/';
			}
		}
		else
		{
			cp = fp->fname;
			if (len > flen)
				flen = (int)strtol(astconf("NAME_MAX", ".", NiL), NiL, 0);
		}
		if (len > flen)
			error(ERROR_exit(1), "%s: filename too long", prefix);
	}
	return fp;
}

/*
 * return next sequential file name
 */

static char*
getfname(struct fname* fp)
{
	register char*	cp = fp->last;

	if (fp->format)
		return sfprints(fp->format, fp->count++);
	while (++(*cp) > fp->high)
	{
		*cp-- = fp->low;
		if (cp < fp->suffix)
		{
			error(0, "file limit reached");
			return 0;
		}
	}
	fp->count++;
	return fp->fname;
}

/*
 * remove all generated files
 */

static void
removeall(struct fname* fp)
{
	register char*	cp = fp->suffix;

	while (*cp)
		*cp++ = fp->low;
	*(cp - 1) -= 1;
	while (fp->count-- > 0)
	{
		remove(getfname(fp));
		fp->count--;
	}
	fp->count = 0;
}

static int
msize(Sfio_t* in, long len)
{
	Sfoff_t		off = sftell(in);
	register char*	cp;
	register char*	dp;
	register long	m;
	register long	n = len;
	register long	nlen = 0;

	if (sfsize(in) - off <= len)
		return len;
	while (nlen == 0 && n > 0)
	{
		n -= BLK_SIZE;
		if (n < 0)
			n = 0;
		sfseek(in, off + n, SEEK_SET);
		if (!(dp = cp = sfreserve(in, BLK_SIZE, 0)))
			return len;
		m = BLK_SIZE;
		while (m-- > 0)
		{
			if (*cp++ == '\n')
				nlen = n + (cp - dp);
		}
	}
	if (n > 0)
		sfseek(in, off, SEEK_SET);
	return nlen ? nlen : len;
}

static int
split(Sfio_t* in, struct fname* fp, struct op* op, int flags)
{
	register char*		cp;
	register char*		s;
	Sfoff_t			len;
	Sfoff_t			z;
	Sfoff_t			size;
	size_t			repeat;
	int			c;

	register Sfio_t*	out = 0;
	register char*		peek = 0;
	register long		n = 0;
	int			delim = (flags & B_FLAG) ? -1 : '\n';
	size_t			lineno = 1;

	while (op)
	{
		if (op->flags == OP_LINES)
			len = op->size;
		repeat = op->repeat;
		do
		{
			if (op->flags != OP_SKIP)
			{
				if (!(cp = getfname(fp)))
					goto err;
				if (!(out = sfopen(NiL, cp, "w")))
				{
					fp->count--;
					error(ERROR_SYSTEM|2, "%s: cannot create", cp);
					goto err;
				}
			}
			if (op->flags == OP_ABSOLUTE || op->flags == OP_LINES)
			{
				if (op->flags == OP_ABSOLUTE)
					len = op->size - lineno;
				if (peek)
				{
					if ((n = sfputr(out, peek, delim)) <= 0)
						goto done;
					peek = 0;
					if (len > 0)
						len--;
					lineno++;
				}
				if (len)
				{
					z = (flags & M_FLAG) ? msize(in, len) : len;
					if ((n = sfmove(in, out, z, delim)) < z || n < 0)
						goto done;
					lineno += n;
				}
			}
			else
			{
				if (peek)
				{
					if (out && (n = sfputr(out, peek, delim)) <= 0)
						goto done;
					lineno++;
					peek = 0;
				}
				while (s = sfgetr(in, delim, 1))
				{
					if (!(c = regexec(op->re, s, 0, NiL, 0)))
						break;
					lineno++;
					if (c != REG_NOMATCH)
					{
						regfatal(op->re, 2, c);
						goto err;
					}
					if (out && (n = sfputr(out, s, delim)) <= 0)
						goto done;
				}
				if (!(peek = s))
				{
					while (op->next)
						op = op->next;
					repeat = 1;
				}
			}
			if (out)
			{
				size = sfseek(out, (Sfoff_t)0, SEEK_END);
				if (!(flags & S_FLAG))
					sfprintf(sfstdout, "%I*d\n", sizeof(size), size);
				sfclose(out);
				out = 0;
				if ((flags & Z_FLAG) && size <= 0)
					remove(cp);
			}
		} while (!repeat || --repeat);
		op = op->next;
	}
 done:
	if (out)
	{
		sfclose(out);
		if (n <= 0)
			remove(cp);
	}
	if (n >= 0)
		return 0;
 err:
	if (!(flags & K_FLAG))
		removeall(fp);
	return 1;
}

int
main(int argc, char** argv)
{
	struct fname*	fp;
	struct op*	top;
	char*		cp;
	char*		prefix;
	const char*	usage;
	Sfio_t*		in;
	int		flags;
	ssize_t		n;

	char*		format = 0;
	Sfoff_t		size = 10000;
	int		suflen = 2;

	if (cp = strrchr(*argv, '/'))
		cp++;
	else
		cp = *argv;
	error_info.id = cp;
	if (streq(cp, "split"))
	{
		usage = split_usage;
		flags = S_FLAG|K_FLAG;
		prefix = "x";
	}
	else
	{
		usage = csplit_usage;
		flags = C_FLAG;
		prefix = "xx";
	}
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 0:
			break;
		case 'l':
			flags &= ~(B_FLAG|M_FLAG);
			if ((size = opt_info.number) <= 0)
				error(1, "%s: invalid size", opt_info.arg);
			continue;
		case 'k':
			flags |= K_FLAG;
			continue;
		case 's':
			flags |= S_FLAG;
			continue;
		case 'z':
			flags |= Z_FLAG;
			continue;
		case 'f':
			prefix = opt_info.arg;
			continue;
		case 'a':
		case 'n':
			suflen = opt_info.num;
			continue;
		case 'C':
			flags |= M_FLAG;
		case 'b':
			if (flags & S_FLAG)
			{
				if ((size = opt_info.number) <= 0)
					error(1, "%s: invalid size", opt_info.arg);
				flags |= B_FLAG;
			}
			else
				format = opt_info.arg;
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
	argc -= opt_info.index;
	if (error_info.errors || !(flags & C_FLAG) && argc > 2 || (flags & C_FLAG) && argc < 2)
		error(ERROR_usage(2), "%s", optusage(NiL));
	cp = *argv++;
	if (flags & C_FLAG)
	{
		struct op*	op = 0;
		char*		sp;

		while (sp = *argv++)
		{
			switch (*sp)
			{
			case '/':
			case '?':
			case '%':
				op = getexpr(op ? &op->next : &top, sp);
				break;
			case '{':
				if (!op)
					error(ERROR_exit(1), "%s: pattern expected for repeat count", *(argv - 1));
				if (*++sp == '*' && *(sp + 1) == '}' && !*(sp + 2))
					op->repeat = 0;
				else
				{
					if ((n = strtol(sp, &sp, 10)) <= 0 || *sp != '}' || *(sp + 1))
						error(ERROR_exit(1), "%s: invalid repeat count", *(argv - 1));
					op->repeat = n + 1;
				}
				if (op->flags == OP_ABSOLUTE)
					op->flags = OP_LINES;
				break;
			default:
				if ((size = strtoll(sp, &sp, 10)) <= 0 || *sp)
					error(ERROR_exit(1), "%s: invalid line number", *(argv - 1));
				op = getop(op ? &op->next : &top, size, 1, OP_ABSOLUTE, 0);
				break;
			}
		}
		op = getop(op ? &op->next : &top, SF_UNBOUND, 1, OP_LINES, 0);
		fp = setfname(prefix, format, suflen, '0', '9');
	}
	else
	{
		if (cp && *argv)
			prefix = *argv;
		getop(&top, size, SF_UNBOUND, OP_LINES, 0);
		fp = setfname(prefix, format, suflen, 'a', 'z');
	}
	if (!cp || streq(cp, "-"))
		in = sfstdin;
	else if (!(in = sfopen(NiL, cp, "r")))
		error(ERROR_system(1), "%s: cannot open", cp);
	n = split(in, fp, top, flags);
	if (in != sfstdin)
		sfclose(in);
	return n;
}
