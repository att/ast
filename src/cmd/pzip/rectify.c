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

/*
 * induce fixed length record groups from data
 */

static const char usage[] =
"[-?\n@(#)$Id: rectify (AT&T Research) 1999-03-22 $\n]"
USAGE_LICENSE
"[+NAME?rectify - induce fixed length record groups from data]"
"[+DESCRIPTION?\brectify\b induces fixed length record groups from input data"
"	by sampling and comparing character frequencies. The standard input is"
"	read if \a-\a or no files are specified.]"

"[c:context?List \acontext\a records at the beginning and end of"
"	record groups larger that 3*\acontext\a.]#[context]"
"[d:description?Specify a structured dump description file. Each line of"
"	this file describes the size and content of a contiguous portion"
"	of the input file. The description is applied separately to each"
"	input file. Comments and optional labels in the following"
"	descriptions are listed with the \b--verbose\b option. Supported"
"	descriptions are:]:[file]{"
"		[+c comment?comment]"
"		[+d size [label]]?\asize\a bytes of data with optional label]"
"		[+i size [label]]?ignore \asize\a bytes of data]"
"		[+r size count [label]]?\acount\a records of length \asize\a]"
"		[+t count?Match \acount\a records against the \bT\b record"
"			table. \acount\a=0 continues until no record type"
"			match is found.]"
"		[+z size [label]]?a string with length determined by a"
"			\asize\a byte binary integer]"
"		[+T idlen id size unit [offset]]?Defines a sized record"
"			table entry.]{"
"			[+idlen?type identifier length, must be"
"				<= 4 bytes]"
"			[+id?type identifier, starting at record offset 0]"
"			[+size?default record size]"
"			[+unit?if > 0 then the record is variable length and"
"				the size is the byte at \aoffset\a]"
"			[+offset?if \aunit\a > 0 then this byte multiplied by"
"				\aunit\a is the size of variable length data"
"				appended to the record]"
"		}"
"}"
"[f:format?Byte output \bprintf\b(3) format.]:[format:=02x]"
"[g!:group?Group output in 4's.]"
"[m:min?Minimum record length to consider.]#[min:=8]"
"[n:count?List the top \acount\a candidate record lengths.]#[count:=16]"
"[o:offset?Start description listing at \aoffset\a.]#[offset:=0]"
"[r:run?List runs at least as long as \arun\a.]#[run]"
"[v:verbose?Dump description labels with data.]"

"\n"
"\n[ file ... ]\n"
"\n"
"[+SEE ALSO?\bpin\b(1), \bpop\b(1)]"
;

#include <ast.h>
#include <error.h>
#include <tok.h>

typedef struct Item_s
{
	unsigned long	index;
	unsigned long	offset;
	unsigned long	start;
	unsigned long	count;
	unsigned long	run;
} Item_t;

typedef struct
{
	int		len;
	unsigned long	id;
	int		size;
	int		unit;
	int		offset;
} Type_t;

typedef struct
{
	Sfoff_t		offset;
	unsigned long	count;
} Loop_t;

static struct
{
	Type_t		type[4 * 1024];
	Item_t		mod[4 * 1024];
	unsigned long	hit[UCHAR_MAX + 1];
	Sfoff_t		offset;
	char*		format1;
	char*		format4;
	unsigned long	context;
	unsigned long	count;
	unsigned long	min;
	unsigned long	run;
	int		group;
	int		types;
	int		typelen;
	int		typelast;
} state;

/*
 * order items by count hi to lo
 */

static int
bycount(const void* va, const void* vb)
{
	register Item_t*	a = (Item_t*)va;
	register Item_t*	b = (Item_t*)vb;

	if (a->count < b->count)
		return 1;
	if (a->count > b->count)
		return -1;
	if (a < b)
		return 1;
	if (a > b)
		return -1;
	return 0;
}

/*
 * rectify fp open for read on file
 */

static void
rectify(register Sfio_t* fp, char* file, int verbose)
{
	register unsigned char*	s;
	register Item_t*	p;
	register unsigned long*	q;
	register unsigned long	offset;
	register unsigned long	i;
	unsigned long		n;
	unsigned long		cur;
	unsigned long		dif;
	unsigned long		max;

	memset(state.hit, 0, sizeof(state.hit));
	memset(state.mod, 0, sizeof(state.mod));
	for (i = 0; i < elementsof(state.mod); i++)
		state.mod[i].index = i;
	max = 0;
	offset = 0;
	while (s = sfreserve(fp, SF_UNBOUND, 0))
	{
		n = sfvalue(fp);
		for (i = 0; i < n; i++)
		{
			cur = offset + i;
			q = state.hit + s[i];
			dif = cur - *q;
			*q = cur;
			if (dif < elementsof(state.mod))
			{
				p = state.mod + dif;
				if (dif > max)
					max = dif;
				p->count++;
				if ((cur - p->offset) <= dif)
				{
					if (!p->run++)
						p->start = cur;
				}
				else if (p->run)
				{
					if (state.run && p->run >= state.run && p->index >= state.min)
						sfprintf(sfstdout, "run %7lu %7lu %7lu\n", p->index, p->run, p->start);
					p->run = 0;
				}
				p->offset = cur;
			}
		}
		offset += n;
	}
	qsort(state.mod, elementsof(state.mod), sizeof(state.mod[0]), bycount);
	n = 0;
	for (i = 0; i < elementsof(state.mod) && n < state.count; i++)
		if (state.mod[i].index >= state.min)
		{
			n++;
			sfprintf(sfstdout, "rec %7lu %7lu %7lu\n", state.mod[i].index, state.mod[i].count, state.mod[i].offset);
		}
}

/*
 * dump size n buffer b to op in 4 hex byte chunks
 */

static void
dump(Sfio_t* op, register unsigned char* b, size_t n)
{
	register unsigned char*	e = b + n / 4 * 4;
	register unsigned char*	x;

	x = state.group ? (b + n) : b;
	while (b < e)
	{
		sfprintf(op, state.format4, b[0], b[1], b[2], b[3]);
		if ((b += 4) < x)
			sfputc(op, ' ');
	}
	while (b < x)
		sfprintf(op, state.format1, *b++);
	sfputc(op, '\n');
}

/*
 * return a number from b and advance b
 */

static unsigned long
number(char** b)
{
	register char*	s;
	unsigned long	r;

	for (s = *b; *s == ' ' || *s == '\t'; s++);
	r = strtoul(s, b, 0);
	if (*b == s)
		error(3, "numeric argument expected");
	for (s = *b; *s == ' ' || *s == '\t'; s++);
	*b = s;
	return r;
}

/*
 * dump fp according to dp
 */

static void
describe(register Sfio_t* dp, char* desc, register Sfio_t* fp, char* file, int verbose)
{
	register unsigned char*	p;
	unsigned char*		e;
	long			size;
	long			count;
	unsigned long		context;
	Sfoff_t			offset;
	Sfoff_t			skip;
	int			nest;
	int			op;
	char*			s;
	char*			t;
	Loop_t			loop[64];
	unsigned long		id[5];

	error_info.file = desc;
	error_info.line = 0;
	offset = 0;
	nest = -1;
	while (s = sfgetr(dp, '\n', 0))
	{
		error_info.line++;
		for (t = s + sfvalue(dp) - 1; *s == ' ' || *s == '\t'; s++);
		for (op = *s; *s != ' ' && *s != '\t' && *s != '\n'; s++);
		for (; *s == ' ' || *s == '\t'; s++);
		switch (op)
		{
		case '#':
		case '\n':
			break;
		case '{':
			if (++nest >= elementsof(loop))
				error(3, "%c: nesting too deep -- %d max", op, elementsof(loop));
			count = number(&s);
			loop[nest].offset = sfseek(dp, (Sfoff_t)0, SEEK_CUR);
			loop[nest].count = count;
			if (verbose && offset >= state.offset)
				sfprintf(sfstdout, "=== %I*d === loop %d %lu %I*d === %-.*s\n", sizeof(offset), offset, nest, loop[nest].count, sizeof(loop[nest].offset), loop[nest].offset, t - s, s);
			break;
		case '}':
			if (nest < 0)
				error(3, "%c: no matching {", op); /*balance}*/
			if (loop[nest].count-- <= 1)
				nest--;
			else if (sfseek(dp, loop[nest].offset, SEEK_SET) < 0)
				error(ERROR_SYSTEM|3, "loop seek error to %I*d", sizeof(loop[nest].offset), loop[nest].offset);
			else if (verbose && offset >= state.offset)
				sfprintf(sfstdout, "=== %I*d === loop %d %lu %I*d === %-.*s\n", sizeof(offset), offset, nest, loop[nest].count, sizeof(loop[nest].offset), loop[nest].offset, t - s, s);
			break;
		case 'c':
			if (verbose && offset >= state.offset)
				sfprintf(sfstdout, "=== %I*d === %-.*s\n", sizeof(offset), offset, t - s, s);
			break;
		case 'd':
			size = number(&s);
			if (offset >= state.offset)
			{
				if (verbose)
					sfprintf(sfstdout, "=== %I*d === %ld === %-.*s\n", sizeof(offset), offset, size, t - s, s);
				if (!(p = sfreserve(fp, size, 0)))
					error(ERROR_SYSTEM|3, "%s: cannot read %ld bytes at %I*d", file, size, sizeof(offset), offset);
				dump(sfstdout, p, size);
			}
			else if (sfseek(fp, (Sfoff_t)size, SEEK_CUR) < 0)
				error(ERROR_SYSTEM|3, "%s: cannot seek %ld bytes at %I*d", file, size, sizeof(offset), offset);
			offset += size;
			break;
		case 'i':
			size = number(&s);
			if (verbose && offset >= state.offset)
				sfprintf(sfstdout, "=== %I*d === %ld === %-.*s\n", sizeof(offset), offset, size, t - s, s);
			if (sfseek(fp, (Sfoff_t)size, SEEK_CUR) < 0)
				error(ERROR_SYSTEM|3, "%s: cannot seek %ld bytes at %I*d", file, size, sizeof(offset), offset);
			offset += size;
			break;
		case 'r':
			size = number(&s);
			count = number(&s);
			if (offset < state.offset)
			{
				skip = count * size;
				if ((offset + skip) > state.offset)
				{
					skip = (state.offset - offset) / size;
					count -= skip;
					skip *= size;
					if (sfseek(fp, skip, SEEK_CUR) < 0)
						error(ERROR_SYSTEM|3, "%s: cannot seek %I*d bytes at %I*d", file, sizeof(skip), skip, sizeof(offset), offset);
					offset += skip;
				}
			}
			if (offset >= state.offset)
			{
				if (verbose)
					sfprintf(sfstdout, "=== %I*d === %ld * %ld === %-.*s\n", sizeof(offset), offset, size, count, t - s, s);
				if (state.context && count > (3 * state.context))
				{
					skip = (count - 2 * state.context) * size;
					count = state.context;
					while (count-- > 0)
					{
						if (!(p = sfreserve(fp, size, 0)))
							error(ERROR_SYSTEM|3, "cannot read %ld bytes at %I*d", size, sizeof(offset), offset);
						offset += size;
						dump(sfstdout, p, size);
					}
					sfprintf(sfstdout, " . . .\n");
					if (sfseek(fp, skip, SEEK_CUR) < 0)
						error(ERROR_SYSTEM|3, "%s: cannot seek %I*d bytes at %I*d", file, sizeof(skip), skip, sizeof(offset), offset);
					offset += skip;
					count = state.context;
				}
				while (count-- > 0)
				{
					if (!(p = sfreserve(fp, size, 0)))
						error(ERROR_SYSTEM|3, "cannot read %ld bytes at %I*d", size, sizeof(offset), offset);
					offset += size;
					dump(sfstdout, p, size);
				}
			}
			else
			{
				skip = count * size;
				if (sfseek(fp, skip, SEEK_CUR) < 0)
					error(ERROR_SYSTEM|3, "%s: cannot seek %I*d bytes at %I*d", file, sizeof(skip), skip, sizeof(offset), offset);
				offset += skip;
			}
			break;
		case 't':
			if (!state.typelen)
				error(3, "no sized record types defined");
			context = 0;
			count = number(&s);
			do
			{
				if (!(p = sfreserve(fp, state.typelen, SF_LOCKR)))
					break;
				switch (state.typelen)
				{
				case 4: id[4] = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
				case 3: id[3] = (p[0] << 16) | (p[1] << 8) | p[2];
				case 2: id[2] = (p[0] << 8) | p[1];
				case 1: id[1] = p[0];
				}
				sfread(fp, p, 0);
				if (state.type[state.typelast].id != id[state.type[state.typelast].len])
				{
					for (state.typelast = 0; state.typelast < state.types && state.type[state.typelast].id != id[state.type[state.typelast].len]; state.typelast++);
					if (state.typelast >= state.types)
					{
						if (verbose)
							sfprintf(sfstdout, "=== %I*d === %0*x === type not found\n", sizeof(offset), offset, 2 * state.typelen, id[state.typelen]);
						break;
					}
					if (verbose && offset >= state.offset)
						sfprintf(sfstdout, "=== %I*d === %0*x === type\n", sizeof(offset), offset, 2 * state.type[state.typelast].len, id[state.type[state.typelast].len]);
					context = 0;
				}
				size = state.type[state.typelast].size;
				if (!(p = sfreserve(fp, size, state.type[state.typelast].unit ? SF_LOCKR : 0)))
					error(ERROR_SYSTEM|3, "cannot read %ld bytes at %I*d", size, sizeof(offset), offset);
				if (state.type[state.typelast].unit)
				{
					size += p[state.type[state.typelast].offset] * state.type[state.typelast].unit;
					sfread(fp, p, 0);
					if (!(p = sfreserve(fp, size, 0)))
						error(ERROR_SYSTEM|3, "cannot read %ld bytes at %I*d", size, sizeof(offset), offset);
				}
				if (offset >= state.offset)
				{
					if (!state.context)
						dump(sfstdout, p, size);
					else if (context++ < state.context)
						dump(sfstdout, p, size);
					else if (context == state.context + 1)
						sfprintf(sfstdout, " . . .\n");
				}
				offset += size;
			} while (!count || --count);
			break;
		case 'z':
			size = number(&s);
			if (!(p = sfreserve(fp, size, 0)))
				error(ERROR_SYSTEM|3, "cannot read %ld bytes at %I*d", size, sizeof(offset), offset);
			count = 0;
			e = p + size;
			while (p < e)
				count = (count << 8) | *p++;
			if (offset >= state.offset)
			{
				if (verbose)
					sfprintf(sfstdout, "=== %I*d === %ld === %-.*s\n", sizeof(offset), offset, size, t - s, s);
				offset += size;
				if (!(p = sfreserve(fp, count, 0)))
					error(ERROR_SYSTEM|3, "cannot read %ld bytes at %I*d", count, sizeof(offset), offset);
				sfprintf(sfstdout, "\"%s\"\n", fmtnesq((char*)p, "\"", count));
			}
			else
			{
				offset += 2;
				if (sfseek(fp, (Sfoff_t)count, SEEK_CUR) < 0)
					error(ERROR_SYSTEM|3, "%s: cannot seek %ld bytes at %I*d", file, count, sizeof(offset), offset);
			}
			offset += count;
			break;
		case 'T':
			if (state.types >= elementsof(state.type))
				error(3, "too many types -- %d max", elementsof(state.type));
			if ((state.type[state.types].len = number(&s)) > state.typelen)
				state.typelen = state.type[state.types].len;
			if (state.type[state.types].len >= elementsof(id))
				error(3, "type id length must be <= %d", elementsof(id) - 1);
			state.type[state.types].id = number(&s);
			state.type[state.types].size = number(&s);
			if (state.type[state.types].unit = number(&s))
				state.type[state.types].offset = number(&s);
			state.types++;
			break;
		default:
			error(2, "%c: unknown description op", op);
			break;
		}
	}
	if (verbose && offset >= state.offset)
		sfprintf(sfstdout, "=== %I*d === EOF\n", sizeof(offset), offset);
	error_info.file = 0;
	error_info.line = 0;
	if (skip = sfseek(fp, (Sfoff_t)0, SEEK_END) - offset)
		error(1, "%s: %I*d bytes ignored at %I*d", file, sizeof(skip), skip, sizeof(offset), offset);
}

int
main(int argc, char** argv)
{
	register char*	file;
	int		n;
	Sfio_t*		fp;
	Sfio_t*		dp;

	char*		desc = 0;
	char*		format = "02x";
	int		verbose = 0;

	error_info.id = "rectify";
	state.count = 16;
	state.group = 1;
	state.min = 8;
	state.run = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'c':
			state.context = opt_info.num;
			continue;
		case 'd':
			if (desc)
				error(2, "%s: only one description file allowed", opt_info.arg);
			else
				desc = opt_info.arg;
			continue;
		case 'f':
			format = opt_info.arg;
			continue;
		case 'g':
			state.group = opt_info.num;
			continue;
		case 'm':
			state.min = opt_info.num;
			continue;
		case 'n':
			state.count = opt_info.num;
			continue;
		case 'o':
			state.offset = opt_info.num;
			continue;
		case 'r':
			state.run = opt_info.num;
			continue;
		case 'v':
			verbose = opt_info.num;
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
	n = (strlen(format) + 1) * 4 + 1;
	if (!(state.format4 = newof(0, char, n, 0)))
		error(ERROR_SYSTEM|3, "out of space [format]");
	sfsprintf(state.format4, n, "%%%s%%%s%%%s%%%s", format, format, format, format);
	state.format1 = state.format4 + 3 * (strlen(format) + 1);
	if (desc && !(dp = sfopen(NiL, desc, "r")))
		error(ERROR_SYSTEM|3, "%s: cannot open description file", desc);
	if (file = *argv)
		argv++;
	do
	{
		if (!file || streq(file, "-"))
			fp = sfstdin;
		else if (!(fp = sfopen(NiL, file, "r")))
			error(ERROR_SYSTEM|3, "%s: cannot read", file);
		if (desc)
			describe(dp, desc, fp, file, verbose);
		else
			rectify(fp, file, verbose);
	} while (file = *argv++);
	return error_info.errors != 0;;
}
