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
 * Glenn Fowler
 * Adam Buchsbaum
 * AT&T Research
 *
 * induce a column partition on fixed row data
 */

static const char usage[] =
"[-?\n@(#)$Id: pin (AT&T Research) 2003-07-17 $\n]"
USAGE_LICENSE
"[+NAME?pin - induce a pzip partition on fixed record data]"
"[+DESCRIPTION?\bpin\b induces a \bpzip\b(1) column partition on data files"
"	of fixed length rows (records) and columns (fields). If a partition"
"	file is specified then that partition is refined. A partition file,"
"	suitable for use by \bpin\b, \bpzip\b(1) or \bpop\b(1) is listed"
"	on the standard output. The input \afile\a is referred to as"
"	\atraining data\a. \b--size\b allows more than one \afile\a"
"	argument, otherwise exactly one file must be specified.]"
"[+?Partitions are induced in a three step process:]{"
"	[+(1)?If \b--partition\b is not specified then a"
"	subset of columns are filtered from the training data for"
"	partitioning. This filtering usually gathers high frequency columns"
"	from the data. A high frequency column has data with a high rate of"
"	change between rows. High frequency cutoff rates, specified by"
"	\b--high\b, typically range"
"	from 10% to 50%, depending on the data. \bpzip\b(1) compresses"
"	low frequency columns much more efficiently than \bgzip\b(1).]"
"	[+(2)?A heuristic search determines an initial ordering of high"
"	frequency columns to present to step (3). An optimal solution for"
"	both ordering and partitioning is NP-complete.]"
"	[+(3)?The optimal partition for the ordering from step (2) is"
"	determined by a dynamic program that computes the compressed size"
"	for all partitions that preserve order."
"	Alternative greedy methods are also available that work more"
"	quickly but do not guarantee optimality.]"
"}"
"[+?See \bpzip\b(1) for a detailed description of file partitions and"
"	column frequencies. \bpin\b can run for a long time on some data"
"	(e.g., 10 minutes on a 400Mhz Pentium with \b--high 80 --window 4M\b)."
"	Use \b--verbose\b possibly with \b--test=010\b to monitor progress.]"

"[b:bzip?Use \bbzip\b(1) compression instead of the default \bgzip\b(1)."
"	\abzip\a is not fully supported by \apzip\a, pending further"
"	investigation.]"
"[c:cache?Generate some information on \afile\a that can be reused"
"	on another invocation; this information is saved in \bpin-\b\abase\a,"
"	where \abase\a is the base name (no directory) of \afile\a. Saved"
"	information includes column frequencies and singleton and pairwise"
"	column \agzip\a rates.]"
"[g:group?Sets the maximum number of columns in any partition group. Lower"
"	values speed up the the dynamic program but also may produce"
"	sub-optimal solutions.]#[columns:=row-size]"
"[h:high?Select this number of columns with the highest frequencies for the"
"	columns in the partition. If \acolumns\a is followed by `%' then"
"	columns with frequencies larger than this percentage are"
"	selected.]:[columns:=10%]"
"[l:level?Sets the \agzip\a compression level to \alevel\a. Levels range"
"	from 1 (fastest, worst compression) to 9 (slowest, best compression).]#"
"		[level:=6]"
"[m:maxhigh?Exit with exit code 3 if the number of high frequency columns"
"	exceeds \bmaxhigh\b. If \bmaxhigh\b is followed by `%' then the limit"
"	is \bmaxhigh\b percent of the total number of columns.]:[maxhigh:=40%]"
"[o:sort?Sort the window data by row before inducing the partition.]"
"[p:partition?Specifies the data row size and the high frequency column"
"	partition groups and permutation. The partition file is a sequence"
"	of lines. Comments start with # and continue to the end of the line."
"	The first non-comment line specifies the optional name string"
"	in \"...\". The next non-comment line specifies the row size."
"	The remaining lines operate on column offset ranges of the form:"
"	\abegin\a[-\aend\a]] where \abegin\a is the beginning column offset"
"	(starting at 0), and \aend\a is the ending column offset for an"
"	inclusive range. The operators are:]:[file]{"
"		[+range [...]]?places all columns in the specified \arange\a"
"			list in the same high frequency partition group."
"			Each high frequency partition group is processed as"
"			a separate block by the underlying compressor"
"			(\bgzip\b(1) by default).]"
"		[+range='value'?specifies that each column in \arange\a"
"			has the fixed character value \avalue\a. C-style"
"			character escapes are valid for \avalue\a.]"
"}"
"[r:row?Specifies the input row size (number of byte columns). The row size"
"	is determined by sampling the input if not specified.]#[row-size]"
"[v:verbose?List partition search details on the standard error.]"
"[w:window?Limit the number of training data rows used to induce the"
"	partition. The window size may be decreased to accomodate an"
"	integral number of complete rows.]#[window-size:=4M]"
"[O:optimize?Choose the optimization (partitioning) method for step (3)"
"	above. The methods"
"	are:]:[method:=\foptimize_default\f]{\foptimize_methods\f}"
"[Q:regress?Generate output for regression testing, such that identical"
"	invocations with identical input files will generate the same output.]"
"[R:reorder?Choose the reordering method for step (2) above. The methods"
"	are:]:[method:=\freorder_default\f]{\freorder_methods\f}"
"[S:size?Ignore \b--row\b, determine the fixed record size based on a window"
"	of sampled data, print it on the standard output, and exit. If more"
"	than one \afile\a is specified then the record size and name are"
"	printed for each file. If the sample is insufficient, or if"
"	\b--verify\b is specified, then all of the data read to determine"
"	the row size. A \b0\b size means the record size could not be"
"	determined.]"
"[T:test?Enable implementation-specific tests and tracing.]#[test-mask]{"
"	[+0x0010?Enable reorder keep trace.]"
"	[+0x0020?Enable reorder skip/cost trace.]"
"	[+0x0040?Enable reorder permutation trace.]"
"	[+0x0080?Enable reorder level 2 merge prune.]"
"	[+0x0100?Disable reorder merge prune.]"
"	[+0x0200?Partition using initial tsp cycles.]"
"}"
"[V:verify?Verify \b--size\b by reading all data instead of the window sample.]"
"[X:prefix?Uncompressed data contains a prefix that is defined by \acount\a"
"	and an optional \aterminator\a. This data is not \bpzip\b compressed."
"	\aterminator\a may be one of:]:[count[*terminator]]]{"
"		[+\aomitted\a?\acount\a bytes.]"
"		[+L?\acount\a \bnewline\b terminated records.]"
"		[+'\achar\a'?\acount\a \achar\a terminated records.]"
"}"

"\n"
"\nfile ...\n"
"\n"
"[+SEE ALSO?\bbzip\b(1), \bgzip\b(1), \bpop\b(1), \bpzip\b(1), \bpzip\b(3)]"
;

#include <ast.h>
#include <ctype.h>
#include <error.h>
#include <ls.h>
#include <pzip.h>
#include <zlib.h>
#include <bzlib.h>

#include "FEATURE/local"

#if _hdr_tsp && _lib_tspopen
#include <tsp.h>
#endif

#define OP_optimize	0x01
#define OP_reorder	0x02
#define OP_size		0x04
#define OP_verify	0x10

#define	minof(x,y)	((x)<(y)?(x):(y))

typedef struct
{
	size_t		elements;
	size_t		size;
	size_t		rate;
	size_t		hit;
	size_t		skip;
	size_t*		member;
} Part_t;

typedef struct
{
	unsigned long	frequency;
	int		prev;
	int		column;
} Stats_t;

struct Optimize_method_s;

typedef void (*Optimize_f)(struct Optimize_method_s*, unsigned char*, unsigned char*, size_t**, size_t*, size_t*, size_t, size_t, size_t);

typedef struct Optimize_method_s
{
	const char*	name;
	const char*	description;
	Optimize_f	fun;
} Optimize_method_t;

struct Reorder_method_s;

typedef void (*Reorder_f)(struct Reorder_method_s*, unsigned char*, unsigned char*, int*, size_t, size_t);

typedef struct Reorder_method_s
{
	const char*	name;
	const char*	description;
	Reorder_f	fun;
} Reorder_method_t;

static struct
{
	Stats_t*	stats;
	size_t*		pam;
	size_t*		map;
	char*		input;
	char*		cachefile;
	int		bzip;
	int		cache;
	int		level;
	int		pairs;
	int		sort;
	int		test;
	int		window;
	int		verbose;
} state;

/*
 * allocate an i-length vector of size_t
 */

static size_t*
vector(int i)
{
	register size_t*	p;

	if (!(p = newof(0, size_t, i, 0)))
		error(ERROR_SYSTEM|3, "out of space [%d vector]", i);
	return p;
}

/*
 * allocate an i X i matrix of size_t
 */

static size_t**
matrix(int i)
{
	register size_t**	p;
	register size_t*	v;
	register int		k;
	size_t			n;

	n = i * i;
	if (!(p = newof(0, size_t*, i, n * sizeof(size_t))))
		error(ERROR_SYSTEM|3, "out of space [%d X %d matrix]", i, i);
	v = (size_t*)(p + i);
	for (k = 0; k < i; k++)
	{
		p[k] = v;
		v += i;
	}
	return p;
}

/*
 * allocate an i X i partition workspace
 */

static Part_t*
partition(int i)
{
	register Part_t*	p;
	register size_t*	v;
	register int		k;
	size_t			n;

	n = i * i;
	if (!(p = newof(0, Part_t, n, n * i * sizeof(size_t))))
		error(ERROR_SYSTEM|3, "out of space [%d partition]", i);
	v = (size_t*)(p + n);
	for (k = 0; k < n; k++)
	{
		p[k].member = v;
		v += i;
	}
	return p;
}

/*
 * order frequencies hi to lo
 */

static int
byfrequency(const void* va, const void* vb)
{
	register Stats_t*	a = (Stats_t*)va;
	register Stats_t*	b = (Stats_t*)vb;

	if (a->frequency < b->frequency)
		return 1;
	if (a->frequency > b->frequency)
		return -1;
	if (a->column < b->column)
		return -1;
	if (a->column > b->column)
		return 1;
	return 0;
}

/*
 * order column lo to hi
 */

static int
bycolumn(const void* va, const void* vb)
{
	register Stats_t*	a = (Stats_t*)va;
	register Stats_t*	b = (Stats_t*)vb;

	if (a->column < b->column)
		return -1;
	if (a->column > b->column)
		return 1;
	return 0;
}

/*
 * order partition rate lo to hi
 */

static int
byrate(const void* va, const void* vb)
{
	register Part_t*	a = (Part_t*)va;
	register Part_t*	b = (Part_t*)vb;

	if (a->rate < b->rate)
		return -1;
	if (a->rate > b->rate)
		return 1;
	if (a->member < b->member)
		return -1;
	if (a->member > b->member)
		return 1;
	return 0;
}

/*
 * order row lo to hi
 */

static int
byrow(const void* va, const void* vb)
{
	return memcmp(va, vb, state.sort);
}

/*
 * dump one partition line with label to sp
 */

static void
dumppart(Sfio_t* sp, const char* label, register Part_t* pp)
{
	register int	i;

	sfprintf(sp, "reorder %s %5u %5u :", label, pp->rate, pp->size);
	for (i = 0; i < pp->elements; i++)
		sfprintf(sp, " %3u", state.map[pp->member[i]]);
	sfprintf(sp, "\n");
}

/*
 * return the compressed size of buffer b of size bytes
 */

static size_t
zip(unsigned char* b, size_t size)
{
	unsigned long		used;
	unsigned int		dest;
	size_t			safe;

	static unsigned char*	buf;
	static int		bufsize;

	safe = 4 * roundof(size, 8 * 1024);
	if (bufsize < safe)
	{
		bufsize = safe;
		if (!(buf = newof(buf, unsigned char, bufsize, 0)))
			error(ERROR_SYSTEM|3, "out of space [buf]");
	}
	if (state.bzip)
	{
		dest = bufsize;
		if (bzBuffToBuffCompress((char*)buf, &dest, (char*)b, size, state.level, 0, 30) != BZ_OK)
			error(3, "bzip compress failed");
		used = dest;
	}
	else
	{
		used = bufsize;
		if (compress2(buf, &used, b, size, state.level) != Z_OK)
			error(3, "gzip compress failed");
	}
	return used;
}

/*
 * copy col i through col j from s into t
 * and return the compressed size
 */

static size_t
field(unsigned char* t, register unsigned char* s, int i, int j, register int row, size_t tot)
{
	register unsigned char*	b;
	register unsigned char*	e;
	register int		n;

	b = t;
	e = s + tot;
	n = j - i + 1;
	for (s += i; s < e; s += row)
	{
		memcpy(b, s, n);
		b += n;
	}
	return zip(t, b - t);
}

/*
 * copy col i and col j from s into t
 * and return the compressed size
 */

static size_t
pair(unsigned char* t, register unsigned char* s, int i, register int j, size_t row, size_t tot)
{
	register unsigned char*	b;
	register unsigned char*	e;

	b = t;
	e = s + tot;
	j -= i;
	for (s += i; s < e; s += row)
	{
		*b++ = *s;
			*b++ = *(s + j);
	}
	return zip(t, b - t);
}

/*
 * return the compressed size for partition pp
 */

static size_t
part(unsigned char* t, register unsigned char* s, register Part_t* pp, size_t row, size_t tot)
{
	register unsigned char*	b;
	register unsigned char*	e;
	register int		i;

	b = t;
	e = s + tot;
	for (; s < e; s += row)
		for (i = 0; i < pp->elements; i++)
			*b++ = s[pp->member[i]];
	return zip(t, b - t);
}

/*
 * merge col i with part pp and keep the best compression in np
 * (np+1) is used as tmp workspace
 * return 1 if the best merge is better than i and pp separately
 */

static int
merge(unsigned char* t, unsigned char* s, int i, Part_t* pp, register Part_t* np, size_t** siz, size_t row, size_t tot)
{
	register int		j;
	register int		k;
	register Part_t*	bp;
	size_t			x;
	size_t			z;

	k = 0;
	np->member[k++] = i;
	for (j = 0; j < pp->elements; j++)
		np->member[k++] = pp->member[j];
	np->elements = k;
	bp = 0;
	x = siz[i][i] + pp->size;
	for (i = 0;;)
	{
		z = part(t, s, np, row, tot);
		if (z < x)
		{
			bp = np + 1;
			bp->size = x = z;
			for (j = 0; j < k; j++)
				bp->member[j] = np->member[j];
		}
		if (++i >= k)
			break;
		j = np->member[i];
		np->member[i] = np->member[i - 1];
		np->member[i - 1] = j;
	}
	if (bp)
	{
		np->size = bp->size;
		np->rate = np->size / k;
		for (j = 0; j < k; j++)
			np->member[j] = bp->member[j];
		if (state.test & 0x0040)
			dumppart(sfstderr, "merg", np);
		return 1;
	}
	return 0;
}

/*
 * filter out the high frequency columns from dat
 */

static size_t
filter(Sfio_t* ip, unsigned char** bufp, unsigned char** datp, Pz_t* pz, int high, int maxhigh, size_t row, size_t tot)
{
	register int	i;
	register int	j;
	char*		q;
	unsigned char*	s;
	unsigned char*	t;
	unsigned char*	dat;
	unsigned char*	buf;
	int		c;
	size_t		rows;
	size_t		n;
	ssize_t		r;
	unsigned long	freq;
	Pzpart_t*	pp;
	Sfio_t*		sp;

	if (pz || high)
	{
		buf = *bufp;
		dat = *datp;
		rows = tot / row;
		if (pz)
		{
			pp = pz->part;
			high = pp->nmap;
			if (!(state.map = newof(state.map, size_t, high, 0)))
				error(ERROR_SYSTEM|3, "out of space [map]");
			memcpy(state.map, pp->map, high * sizeof(state.map[0]));
			pzclose(pz);
		}
		else
		{
			if (!(state.map = oldof(state.map, size_t, row, 0)))
				error(ERROR_SYSTEM|3, "out of space [map]");
			if (!(state.stats = oldof(state.stats, Stats_t, row, 0)))
				error(ERROR_SYSTEM|3, "out of space [stats]");
			memset(state.stats, 0, row * sizeof(state.stats[0]));
			if (state.cache && (sp = sfopen(NiL, state.cachefile, "r")))
			{
				int		column;
				unsigned long	frequency;

				if (state.verbose)
					sfprintf(sfstderr, "filter using %s for frequencies\n", state.cachefile);
				error_info.file = state.cachefile;
				error_info.line = 0;
				while (q = sfgetr(sp, '\n', 1))
				{
					error_info.line++;
					if (*q == 'f')
					{
						if (sfsscanf(q + 1, "%d %d %lu", &c, &column, &frequency) != 3 || c < 0 || c >= row || column < 0 || column >= row)
							error(3, "%s: `%s' is invalid", buf, q);
						state.stats[c].prev = 1;
						state.stats[c].column = column;
						state.stats[c].frequency = frequency;
					}
				}
				for (j = 0; j < row; j++)
					if (!state.stats[j].prev)
						error(3, "invalid cache -- column %d frequency omitted", j);
				sfclose(sp);
				error_info.file = 0;
				error_info.line = 0;
			}
			else
			{
				if (state.verbose)
				{
					if (high < 0)
						sfprintf(sfstderr, "filter top %d%% high frequency columns\n", -high);
					else if (high > 0)
						sfprintf(sfstderr, "filter top %d high frequency columns\n", high);
				}
				s = dat;
				for (i = 0; i < rows; i++)
					for (j = 0; j < row; j++)
						if (state.stats[j].prev != (c = *s++))
						{
							state.stats[j].prev = c;
							state.stats[j].frequency++;
						}
				n = rows;
				while ((r = sfread(ip, s = buf, state.window)) > 0)
				{
					r /= row;
					if (state.sort)
						qsort(s, r, row, byrow);
					for (i = 0; i < r; i++)
						for (j = 0; j < row; j++)
							if (state.stats[j].prev != (c = *s++))
							{
								state.stats[j].prev = c;
								state.stats[j].frequency++;
							}
					if ((n += r) > 10000000L)
					{
						/*
						 * that'll do pig
						 */

						r = 0;
						break;
					}
				}
				if (r < 0)
					error(ERROR_SYSTEM|3, "data read error");
				else if (r)
					error(1, "last record incomplete");
				for (j = 0; j < row; j++)
					state.stats[j].column = j;
				qsort(state.stats, row, sizeof(Stats_t), byfrequency);
				if (state.cache)
				{
					error_info.file = state.cachefile;
					error_info.line = 0;
					if (!(sp = sfopen(NiL, state.cachefile, "w")))
						error(ERROR_SYSTEM|1, "cannot write cache");
					else
					{
						sfprintf(sp, "# %s cache for %s\n", error_info.id, state.input);
						for (j = 0; j < row; j++)
						{
							sfprintf(sp, "f %d %d %lu\n", j, state.stats[j].column, state.stats[j].frequency);
							error_info.line++;
						}
						if (sfclose(sp))
							error(ERROR_SYSTEM|3, "cache write error");
					}
					error_info.file = 0;
					error_info.line = 0;
				}
				if (state.verbose)
					sfprintf(sfstderr, "filter done -- %u rows\n", n);
			}
			if (high < 0)
			{
				high = -high;
				freq = state.stats[0].frequency * high / 100;
				for (j = 0; j < row; j++)
					if (state.stats[j].frequency <= freq)
						break;
				high = j;
				if (state.verbose)
					sfprintf(sfstderr, "%d high frequency column%s out of %d\n", high, high == 1 ? "" : "s", row);
			}
			if (maxhigh < 0)
			{
				maxhigh = -maxhigh;
				if (high > (row * maxhigh / 100))
					error(6, "high frequency count %d exceeds %d%% of %d", high, maxhigh, row);
			}
			else if (maxhigh > 0)
			{
				if (high > maxhigh)
					error(6, "high frequency count %d exceeds %d", high, maxhigh);
			}

			/*
			 * cut the original data layout some slack
			 * by using the original column order
			 */

			qsort(state.stats, high, sizeof(Stats_t), bycolumn);
			for (j = 0; j < high; j++)
				state.map[j] = state.stats[j].column;
			state.pairs = row;
			if (!(state.pam = newof(state.pam, size_t, row, 0)))
				error(ERROR_SYSTEM|3, "out of space [pam]");
			for (j = 0; j < row; j++)
				state.pam[j] = row;
			for (j = 0; j < high; j++)
				state.pam[state.map[j]] = j;
		}
		s = dat;
		t = buf;
		for (i = 0; i < rows; i++)
		{
			for (j = 0; j < high; j++)
				*t++ = s[state.map[j]];
			s += row;
		}
		row = high;
		*bufp = dat;
		*datp = buf;
	}
	else
	{
		if (!(state.map = newof(0, size_t, row, 0)))
			error(ERROR_SYSTEM|3, "out of space [map]");
		for (i = 0; i < row; i++)
			state.map[i] = i;
	}
	return row;
}

/*
 * permute state.map and dat according to a new order
 */

static void
permute(unsigned char* buf, unsigned char* dat, size_t* ord, size_t row, size_t tot)
{
	register int	i;
	size_t*		tmap;
	unsigned char*	end;

	tmap = vector(row);
	for (i = 0; i < row; i++)
		tmap[i] = state.map[i];
	for (i = 0; i < row; i++)
		state.map[i] = tmap[ord[i]];
	for (end = dat + tot; dat < end; dat += row)
	{
		memcpy(buf, dat, row);
		for (i = 0; i < row; i++)
			dat[i] = buf[ord[i]];
	}
	free(tmap);
}

/*
 * stuff the partion group labels in lab
 */

static int
solution(int* lab, int label, size_t* sol, int beg, int end)
{
	int	i;

	i = sol[end];
	if (i >= beg)
		label = solution(lab, label, sol, beg, i);
	for (label++, beg = i + 1; beg <= end; beg++)
		lab[beg] = label;
	return label;
}

/*
 * list the final partition on sp
 */

static void
list(Sfio_t* sp, int* lab, size_t row)
{
	register int	i;
	register int	j;
	register int	g;

	g = -1;
	for (i = 0; i < row; i++)
	{
		if (g != lab[i])
		{
			g = lab[i];
			sfprintf(sp, "\n");
		}
		else
			sfprintf(sp, " ");
		for (j = i + 1; j < row && lab[j] == g && state.map[j] == (state.map[j - 1] + 1); j++);
		sfprintf(sp, "%d", state.map[i]);
		if (j > (i + 2))
		{
			i = j - 1;
			sfprintf(sp, "-%d", state.map[i]);
		}
	}
	sfprintf(sp, "\n");
}

/*
 * search for a good initial ordering
 */

static void
reorder_heuristic(Reorder_method_t* method, unsigned char* buf, unsigned char* dat, int* lab, size_t row, size_t tot)
{
	register int		i;
	register int		j;
	register int		k;
	register Part_t*	cp;
	register Part_t*	np;
	register Part_t*	tp;
	register Part_t*	bp;
	Part_t*			fp;
	size_t*			hit;
	size_t*			ord;
	ssize_t*		cst;
	char*			s;
	int			ii;
	int			jj;
	size_t			y;
	size_t			z;
	ssize_t			cy;
	Part_t*			cur;
	Part_t*			nxt;
	Part_t*			fin;
	size_t**		siz;
	Sfio_t*			sp;

	siz = matrix(row);
	cur = partition(row);
	nxt = partition(row);
	fin = partition(row);
	hit = vector(row);
	ord = vector(row);
	cst = (ssize_t*)vector(row);

	/*
	 * fill in the pair compression size matrix
	 * and the initial partition candidate list
	 */

	k = 2;
	cp = cur;

	/*
	 * check the cache
	 */

	if (state.cache && (sp = sfopen(NiL, state.cachefile, "r")))
	{
		if (state.verbose)
			sfprintf(sfstderr, "reorder using %s for pair compress sizes\n", state.cachefile);
		error_info.file = state.cachefile;
		error_info.line = 0;
		while (s = sfgetr(sp, '\n', 1))
		{
			error_info.line++;
			if (*s == 'p')
			{
				if (sfsscanf(s + 1, "%d %d %I*u", &ii, &jj, sizeof(z), &z) != 3 || ii < 0 || jj < 0 || state.pairs && (ii >= state.pairs || jj >= state.pairs))
					error(3, "%s: `%s' is invalid", buf, s);
				if (state.pairs)
				{
					ii = state.pam[ii];
					jj = state.pam[jj];
				}
				if (ii >= row || jj >= row)
					continue;
				siz[ii][jj] = z;
				if (ii == jj)
					hit[ii] = 1;
				else
				{
					hit[ii] = hit[jj] = k;
					cp->size = siz[ii][jj];
					cp->rate = cp->size / 2;
					cp->elements = 2;
					cp->member[0] = ii;
					cp->member[1] = jj;
					cp++;
				}
			}
		}
		sfclose(sp);
		error_info.file = 0;
		error_info.line = 0;
	}
	sp = 0;
	for (i = 0; i < row; i++)
		if (!hit[i])
		{
			hit[i] = 1;
			if (state.cache && !error_info.file)
			{
				error_info.file = state.cachefile;
				error_info.line = 0;
				if (!(sp = sfopen(NiL, error_info.file, "a")))
					error(ERROR_SYSTEM|1, "cannot update cache");
				else if (!sfseek(sp, (Sfoff_t)0, SEEK_END))
				{
					sfprintf(sp, "# %s cache for %s\n", error_info.id, state.input);
					error_info.line++;
				}
			}
			siz[i][i] = field(buf, dat, i, i, row, tot);
			if (sp)
			{
				sfprintf(sp, "p %d %d %I*u\n", state.map[i], state.map[i], sizeof(siz[i][i]), siz[i][i]);
				error_info.line++;
			}
			if (state.verbose)
				sfprintf(sfstderr, "reorder pairs for %d [%d]\n", i, state.map[i]);
			for (j = 0; j < i; j++)
			{
				z = pair(buf, dat, i, j, row, tot);
				y = pair(buf, dat, j, i, row, tot);
				if (z <= y)
				{
					ii = i;
					jj = j;
				}
				else
				{
					z = y;
					ii = j;
					jj = i;
				}
				if (z < (siz[i][i] + siz[j][j]))
				{
					hit[i] = hit[j] = k;
					cp->size = siz[ii][jj] = z;
					cp->rate = cp->size / 2;
					cp->elements = 2;
					cp->member[0] = ii;
					cp->member[1] = jj;
					cp++;
					if (sp)
					{
						sfprintf(sp, "p %d %d %I*u\n", state.map[ii], state.map[jj], sizeof(z), z);
						error_info.line++;
					}
				}
			}
		}
	if (sp && sfclose(sp))
		error(ERROR_SYSTEM|1, "cache write error");
	error_info.file = 0;
	error_info.line = 0;

	/*
	 * coalesce the partitions
	 */

	fp = fin;
	for (;;)
	{
		if (state.verbose)
			sfprintf(sfstderr, "reorder part %d\n", k + 1);
		qsort(cur, cp - cur, sizeof(Part_t), byrate);
		np = nxt;
		if (k == 2)
		{
			for (i = 0; i < row; i++)
				cst[i] = state.window;
			for (tp = cur; tp < cp; tp++)
			{
				i = tp->member[0];
				j = tp->member[1];
				if (!(z = siz[i][j]))
					z = siz[j][i];
				if (z)
				{
					cy = z - siz[i][i];
					if (cy < cst[j])
						cst[j] = cy;
					cy = z - siz[j][j];
					if (cy < cst[i])
						cst[i] = cy;
				}
			}
		}
		else
			for (tp = cur; tp < cp; tp++)
				for (j = 0; j < tp->elements; j++)
				{
					ii = 0;
					for (i = 0; i < tp->elements; i++)
						if (i != j)
							np->member[ii++] = tp->member[i];
					np->elements = ii;
					cst[tp->member[j]] = tp->size - part(buf, dat, np, row, tot);
					if (state.test & 0x0020)
						sfprintf(sfstderr, "reorder cost %6u %u\n", state.map[tp->member[j]], cst[tp->member[j]]);
				}
		for (tp = cur; tp < cp; tp++)
		{
			bp = 0;
			for (i = 0; i < row; i++)
			{
				if (hit[i] > k)
					continue;
				z = 0;
				jj = 0;
				for (j = 0; j < tp->elements; j++)
					if (hit[tp->member[j]] > k)
						z = 2;
					else
					{
						tp->member[jj++] = tp->member[j];
						if (i == tp->member[j])
							z = 2;
						else if (!z && (siz[i][tp->member[j]] || siz[tp->member[j]][i]))
							z = 1;
					}
				tp->elements = jj;
				if (z != 1)
					continue;
				if (!merge(buf, dat, i, tp, np, siz, row, tot))
				{
					if (!(state.test & 0x0100) && (k > 2 || (state.test & 0x0080)))
						for (j = 0; j < tp->elements; j++)
							siz[i][tp->member[j]] = siz[tp->member[j]][i] = 0;
				}
				else if ((ssize_t)(np->size - tp->size) < cst[i] && (!bp || np->size < bp->size))
				{
					bp = np + 2;
					bp->size = np->size;
					bp->elements = np->elements;
					for (j = 0; j < np->elements; j++)
						bp->member[j] = np->member[j];
				}
			}
			if (bp)
			{
				np->size = bp->size;
				np->elements = bp->elements;
				for (j = 0; j < bp->elements; j++)
					hit[np->member[j] = bp->member[j]] = k + 1;
				if (state.test & 0x0010)
					dumppart(sfstderr, "keep", np);
				np++;
			}
			else
				tp->skip = 1;
		}
		for (tp = cur; tp < cp; tp++)
			if (tp->skip)
			{
				if (state.test & 0x0020)
					dumppart(sfstderr, "skip", tp);
				tp->skip = 0;
				if (k > 2 && tp->elements > 1)
				{
					fp->hit = k;
					fp->size = tp->size;
					fp->elements = tp->elements;
					for (j = 0; j < tp->elements; j++)
						fp->member[j] = tp->member[j];
					fp++;
				}
			}
		if (np == nxt)
			break;
		tp = nxt;
		nxt = cur;
		cur = tp;
		cp = np;
		k++;
	}

	/*
	 * collect the order in ord and the partition labels in lab
	 */

	if (state.verbose)
		sfprintf(sfstderr, "reorder done\n");
	j = 0;
	jj = 0;
	k = 0;
	while (fp-- > fin)
		for (i = 0; i < fp->elements; i++)
			if (hit[fp->member[i]] == fp->hit)
			{
				if (!jj)
				{
					jj = 1;
					j++;
				}
				hit[fp->member[i]] = 0;
				ord[k++] = fp->member[i];
				lab[fp->member[i]] = j;
			}
	for (i = 0; i < row; i++)
		if (hit[i])
		{
			ord[k++] = i;
			lab[i] = ++j;
		}

	/*
	 * permute state.map and dat according to the new order
	 */

	permute(buf, dat, ord, row, tot);

	/*
	 * clean up
	 */

	free(siz);
	free(cur);
	free(nxt);
	free(fin);
	free(hit);
	free(cst);
	free(ord);
}

/*
 * tsp ordering
 */

static void
reorder_tsp(Reorder_method_t* method, unsigned char* buf, unsigned char* dat, int* lab, size_t row, size_t tot)
{
#if TSP_VERSION
	size_t		i;
	size_t		j;
	size_t		end;
	size_t		breakat;
	size_t*		self;
	size_t**	apart;
	size_t**	together;
	Tsp_t*		tsp;
	Tsp_index_t*	tour;
	Tsp_index_t*	cycle;
	Tsp_cost_t	breakval;
	Tsp_cost_t**	cost;
	Tsp_cost_t*	v;
	Tsp_disc_t	disc;

	if (!(cost = newof(0, Tsp_cost_t*, row, row * row * sizeof(Tsp_cost_t))))
		error(ERROR_SYSTEM|3, "out of space [%d X %d cost matrix]", row, row);
	v = (Tsp_cost_t*)(cost + row);
	for (i = 0; i < row; i++)
	{
		cost[i] = v;
		v += row;
	}
	self = vector(row);
	apart = matrix(row);
	together = matrix(row);

	/*
	 * compute the Tsp_cost_t matrix
	 */

	if (state.verbose)
		sfprintf(sfstderr, "compute the tsp cost matrix\n");
	for (i = 0; i < row; i++)
		self[i] = field(buf, dat, i, i, row, tot);
	for (i = 0; i < row; i++)
		for (j = 0; j < row; j++)
			if (i != j)
			{
				together[i][j] = pair(buf, dat, i, j, row, tot);
				apart[i][j] = self[i] + self[j];
				cost[i][j] = minof(together[i][j], apart[i][j]);
			}

	/*
	 * generate a tour
	 */

	if (state.verbose)
		sfprintf(sfstderr, "generate a tour\n");
	memset(&disc, 0, sizeof(disc));
	disc.version = TSP_VERSION;
	disc.errorf = errorf;
	if (!(tsp = tspopen(&disc, cost, row, TSP_DFS|(state.verbose ? TSP_VERBOSE : 0))))
		error(3, "tspopen error");
	if (!(tour = tsptour(tsp)))
		error(3, "tsptour error");

	/*
	 * break tour at most expensive link; put order into self
	 */

	breakat = end = row-1;
	breakval = cost[tour[end]][tour[0]];
	for (i = 0; i < end; i++)
		if (cost[tour[i]][tour[i+1]] > breakval)
		{
			breakat = i;
			breakval = cost[tour[i]][tour[i+1]];
		}
	j = 0;
	for (i = breakat + 1; i < row; i++)
		self[j++] = tour[i];
	for (i = 0; i <= breakat; i++)
		self[j++] = tour[i];

	/*
	 * permute state.map and dat according to the new order
	 */

	permute(buf, dat, self, row, tot);

	/*
	 * partition
	 */

	if (state.test & 0x0200)
	{
		/*
		 * partition according to the initial tsp cycles
		 */

		if (!(cycle = tspcycle(tsp)))
			error(3, "tspcycle error");
		for (i = 0; i < row; i++)
			lab[i] = cycle[self[i]];
	}
	else
	{
		/*
		 * partition according to dependence along tour
		 */

		for (i = j = 0; i < end; i++)
		{
			lab[i] = j;
			if (together[self[i]][self[i+1]] > apart[self[i]][self[i+1]])
				j++;
		}
		lab[i] = j;
	}

	/*
	 * clean up
	 */

	free(self);
	free(apart);
	free(together);
	free(cost);
	tspclose(tsp);
#else
	error(3, "%s ordering requires -l%s", method->name, method->name);
#endif
}

/*
 * dynamic program to find optimal partition based on zip sizes in val
 */

static void
optimize_dynamic(Optimize_method_t* method, unsigned char* buf, unsigned char* dat, size_t** val, size_t* cst, size_t* sol, size_t row, size_t tot, size_t grp)
{
	int	i;
	int	j;
	int	k;
	size_t	new;

	/*
	 * fill in the field compress value matrix
	 */

	for (i = 0; i < row; i++)
	{
		if (grp <= 0)
			k = row;
		else if ((k = i + grp) > row)
			k = row;
		if (state.verbose)
			sfprintf(sfstderr, "%s %d..%d\n", method->name, i, k - 1);
		for (j = i; j < k; j++)
			val[i][j] = field(buf, dat, i, j, row, tot);
		for (; j < row; j++)
			val[i][j] = ~0;
	}

	/*
	 * now run the dynamic program
	 */

	for (i = 0; i < row; i++)
	{
		cst[i] = val[0][i];
		sol[i] = -1;
		for (j = 0; j < i; j++)
		{
			new = cst[j] + val[j+1][i];
			if (new < cst[i])
			{
				cst[i] = new;
				sol[i] = j;
			}
		}
	}
}

/*
 * greedy algorithm to approximate optimal partition based on zip sizes in val
 */

static void
optimize_greedy(Optimize_method_t* method, unsigned char* buf, unsigned char* dat, size_t** val, size_t* cst, size_t* sol, size_t row, size_t tot, size_t grp)
{
	int	i;
	size_t	expand;
	size_t	new;

	cst[0] = val[0][0] = field(buf, dat, 0, 0, row, tot);
	sol[0] = -1;
	for (i = 1; i < row; i++)
	{
		val[sol[i-1] + 1][i] = field(buf, dat, sol[i-1]+1, i, row, tot);
		val[i][i] = field(buf, dat, i, i, row, tot);
		expand = val[sol[i-1] + 1][i];
		new = cst[i-1] + val[i][i];
		if (new < expand) {
			cst[i] = val[i][i];
			sol[i] = i-1;
		}
		else {
			cst[i] = expand;
			sol[i] = sol[i-1];
		}
	}
}

/*
 * transitive greedy algorithm to approximate optimal partition
 * based on zip sizes in val
 */

static void
optimize_transitive(Optimize_method_t* method, unsigned char* buf, unsigned char* dat, size_t** val, size_t* cst, size_t* sol, size_t row, size_t tot, size_t grp)
{
	int	i;
	size_t	expand;

	cst[0] = val[0][0] = field(buf, dat, 0, 0, row, tot);
	sol[0] = -1;
	for (i = 1; i < row; i++)
	{
		val[i][i] = field(buf, dat, i, i, row, tot);
		expand = field(buf, dat, i-1, i, row, tot);
		if (expand <= val[i-1][i-1] + val[i][i])
		{
			cst[i] = expand;
			sol[i] = sol[i-1];
		}
		else
		{
			cst[i] = val[i][i];
			sol[i] = i-1;
		}
	}
}

/*
 * optimization (partitioning) method table
 * the first entry is the default
 */

static Optimize_method_t	optimize_methods[] =
{
	{
		"dynamic",
		"dynamic program optimal partition",
		optimize_dynamic
	},
	{
		"greedy",
		"greedy approximation partition",
		optimize_greedy
	},
	{
		"none",
		"no partition",
		0
	},
	{
		"transitive",
		"transitive greedy approximation partition",
		optimize_transitive
	},
};

/*
 * reorder method table
 * the first entry is the default
 */

static Reorder_method_t		reorder_methods[] =
{
	{
		"heuristic",
		"heuristic reorder",
		reorder_heuristic
	},
	{
		"none",
		"no reordering",
		0
	},
	{
		"tsp",
		"tsp reordering",
		reorder_tsp
	},
};

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register int	i;
	register int	n;

	n = 0;
	if (streq(s, "optimize_default"))
		n += sfprintf(sp, "%s", optimize_methods[0].name);
	else if (streq(s, "optimize_methods"))
		for (i = 0; i < elementsof(optimize_methods); i++)
			n += sfprintf(sp, "[+%s?%s]", optimize_methods[i].name, optimize_methods[i].description);
	else if (streq(s, "reorder_default"))
		n += sfprintf(sp, "%s", reorder_methods[0].name);
	else if (streq(s, "reorder_methods"))
		for (i = 0; i < elementsof(reorder_methods); i++)
			n += sfprintf(sp, "[+%s?%s]", reorder_methods[i].name, reorder_methods[i].description);
	return n;
}

int
main(int argc, char** argv)
{
	unsigned char*	dat;
	unsigned char*	buf;
	char*		s;
	int		i;
	ssize_t		n;
	ssize_t		m;
	size_t		rows;
	size_t		tot;
	size_t		rec;
	size_t		win;
	int*		lab;
	Sfio_t*		ip;
	Sfio_t*		dp;
	Pzdisc_t	disc;
	struct stat	is;
	struct stat	cs;
	Optdisc_t	optdisc;

	int		op = 0;
	int		high = -10;
	int		maxhigh = -40;
	int		maxgrp = 0;
	size_t		row = 0;
	Pz_t*		pz = 0;
	char*		partition = 0;

	Optimize_method_t*	optimize_method = &optimize_methods[0];
	Reorder_method_t*	reorder_method = &reorder_methods[0];

	error_info.id = "pin";
	optinit(&optdisc, optinfo);
	state.level = 6;
	state.window = PZ_WINDOW;
	if (!(dp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space");
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'b':
			state.bzip = 1;
			continue;
		case 'c':
			state.cache = opt_info.num;
			continue;
		case 'g':
			maxgrp = opt_info.num;
			continue;
		case 'h':
			high = strtol(opt_info.arg, &s, 0);
			if (*s == '%')
			{
				s++;
				high = -high;
			}
			if (*s)
				error(3, "%s: %s: invalid number", opt_info.name, opt_info.arg);
			continue;
		case 'l':
			state.level = opt_info.num;
			continue;
		case 'm':
			maxhigh = strtol(opt_info.arg, &s, 0);
			if (*s == '%')
			{
				s++;
				maxhigh = -maxhigh;
			}
			if (*s)
				error(3, "%s: %s: invalid number", opt_info.name, opt_info.arg);
			continue;
		case 'o':
			state.sort = 1;
			continue;
		case 'p':
			partition = opt_info.arg;
			continue;
		case 'r':
			row = opt_info.num;
			continue;
		case 'v':
			state.verbose = 1;
			continue;
		case 'w':
			state.window = opt_info.num;
			continue;
		case 'O':
			n = strlen(opt_info.arg);
			for (i = 0;; i++)
			{
				if (i >= elementsof(optimize_methods))
				{
					error(2, "%s: unknown partition method", opt_info.arg);
					break;
				}
				if (strneq(opt_info.arg, optimize_methods[i].name, n))
				{
					optimize_method = &optimize_methods[i];
					break;
				}
			}
			continue;
		case 'Q':
			sfprintf(dp, "regress\n");
			continue;
		case 'R':
			n = strlen(opt_info.arg);
			for (i = 0;; i++)
			{
				if (i >= elementsof(reorder_methods))
				{
					error(2, "%s: unknown reorder method", opt_info.arg);
					break;
				}
				if (strneq(opt_info.arg, reorder_methods[i].name, n))
				{
					reorder_method = &reorder_methods[i];
					break;
				}
			}
			continue;
		case 'S':
			op |= OP_size;
			continue;
		case 'T':
			sfprintf(dp, "test=%s\n", opt_info.arg);
			continue;
		case 'V':
			op |= OP_verify;
			continue;
		case 'X':
			sfprintf(dp, "prefix=%s\n", opt_info.arg);
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			if (!opt_info.option[0] && opt_info.name[0] == opt_info.name[1])
				sfputr(dp, &argv[opt_info.index - 1][2], '\n');
			else
				error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || !(state.input = *argv++) || (n = *argv != 0) && !(op & OP_size))
		error(ERROR_USAGE|4, "%s", optusage(NiL));

	/*
	 * set up the workspace
	 */

	if (optimize_method->fun)
		op |= OP_optimize;
	if (reorder_method->fun)
		op |= OP_reorder;
	memset(&disc, 0, sizeof(disc));
	disc.version = PZ_VERSION;
	disc.errorf = errorf;
	disc.partition = partition;
	disc.window = 1;
	if (sftell(dp) && !(disc.options = strdup(sfstruse(dp))))
		error(ERROR_SYSTEM|3, "out of space");
	sfclose(dp);
	if (op & OP_size)
	{
		row = 0;
		dat = 0;
		buf = 0;
	}
	for (;;)
	{
		ip = 0;
		if (pz = pzopen(&disc, state.input, 0))
		{
			state.test = pz->test;
			ip = pz->io;
			if (partition)
			{
				row = pz->part->row;
				high = pz->part->nmap;
			}
			else
			{
				m = pz->part->row;
				if (!row)
				{
					if (m > 0)
					{
						row = m;
						if (state.verbose && !(op & OP_size))
							sfprintf(sfstderr, "row size %I*u\n", sizeof(row), row);
					}
					else if (!(op & OP_size))
						error(1, "could not determine row size");
				}
				else if (m > 0 && (row < m || row > m && (row % m)))
					error(1, "row size %I*d may be invalid -- try %I*u next time", sizeof(row), row, sizeof(m), m);
				pz->io = 0;
				pzclose(pz);
				pz = 0;
			}
		}
		else if (!(op & OP_size))
			return 1;
		if (!(op & OP_size))
			break;
		if (row && (op & OP_verify))
		{
			win = (state.window / row) * row;
			if (!(dat = newof(dat, unsigned char, win, 0)))
				error(ERROR_SYSTEM|3, "out of space [dat]");
			if (!(buf = newof(buf, unsigned char, win, 0)))
				error(ERROR_SYSTEM|3, "out of space [buf]");
			if ((m = sfread(ip, dat, win)) < 0)
				m = 0;
			rows = m / row;
			if (filter(ip, &buf, &dat, pz, high, maxhigh, row, row * rows) == row)
				row = 0;
		}
		if (!n)
		{
			sfprintf(sfstdout, "%I*u\n", sizeof(row), row);
			return 0;
		}
		sfprintf(sfstdout, "%8I*u %s\n", sizeof(row), row, state.input);
		if (pz)
			pzclose(pz);
		else if (ip)
			sfclose(ip);
		if (!(state.input = *argv++))
			return 0;
		row = 0;
	}
	if (!(rec = row))
		error(3, "-r row-size is required");
	if (high > (int)row)
		error(3, "-h col-count must be <= -r row-size");
	state.window = (state.window / row) * row;
	if (!(dat = newof(0, unsigned char, state.window, 0)))
		error(ERROR_SYSTEM|3, "out of space [dat]");
	if (!(buf = newof(0, unsigned char, state.window, 0)))
		error(ERROR_SYSTEM|3, "out of space [buf]");
	if ((n = sfread(ip, dat, state.window)) <= 0)
		error(3, "input empty");
	rows = n / row;
	state.window = rows * row;
	if (state.sort)
	{
		state.sort = row;
		qsort(dat, rows, row, byrow);
	}

	/*
	 * set up the cache file
	 */

	if (stat(state.input, &is))
		error(ERROR_SYSTEM|3, "%s: cannot stat", state.input);
	if (s = strrchr(state.input, '/'))
		s++;
	else
		s = state.input;
	if (state.cache)
	{
		if (!(state.cachefile = strdup(sfprints("%s-%s", error_info.id, s))))
			error(ERROR_SYSTEM|3, "out of space");
		if (!stat(state.cachefile, &cs) && cs.st_mtime < is.st_mtime && remove(state.cachefile))
			error(ERROR_SYSTEM|3, "%s: cannot update intermediate data cache", state.cachefile);
	}

	/*
	 * filter out the high frequency columns
	 */

	if (row = filter(ip, &buf, &dat, pz, high, maxhigh, row, row * rows))
	{
		tot = row * rows;
		if (!(lab = newof(0, int, row, 0)))
			error(ERROR_SYSTEM|3, "out of space [lab]");
		if (pz && !(op & (OP_reorder|OP_optimize)))
			for (n = 0; n < pz->part->nmap; n++)
				lab[n] = pz->part->lab[n];
	}
	else
		op &= ~(OP_optimize|OP_reorder);

	/*
	 * determine a better ordering
	 */

	if (op & OP_reorder)
		(*reorder_method->fun)(reorder_method, buf, dat, lab, row, tot);

	/*
	 * generate a partition based on the ordering
	 */

	if (op & OP_optimize)
	{
		size_t*		cst;
		size_t*		sol;
		size_t**	val;

		/*
		 * allocate the tmp workspace
		 */

		val = matrix(row);
		cst = vector(row);
		sol = vector(row);

		/*
		 * partition
		 */

		(*optimize_method->fun)(optimize_method, buf, dat, val, cst, sol, row, tot, maxgrp);

		/*
		 * gather the optimal partition group labels in lab
		 */

		solution(lab, 0, sol, 0, row - 1);
		if (state.verbose)
			sfprintf(sfstderr, "%s done\n", optimize_method->name);

		/*
		 * clean up
		 */

		free(val);
		free(cst);
		free(sol);
	}

	/*
	 * finished -- list the partition
	 */

	sfprintf(sfstdout, "# pzip partition\n");
	sfprintf(sfstdout, "# %s\n", fmtident(usage));
	if (!(op & OP_reorder))
		sfprintf(sfstdout, "# group coalescing limited to adjacent columns\n");
	if (maxgrp)
		sfprintf(sfstdout, "# group size limited to %d columns\n", maxgrp);
	sfprintf(sfstdout, "# row %d window %d compression level %d\n", rec, state.window, state.level);
	if (state.sort)
		sfprintf(sfstdout, "\nsort=1\n");
	if (high)
		sfprintf(sfstdout, "\n%d\t# high frequency %d\n", rec, row);
	else
		sfprintf(sfstdout, "\n%d\n", rec);
	list(sfstdout, lab, row);
	return 0;
}
