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
 * partitioned fixed ops
 */

static const char usage[] =
"[-?\n@(#)$Id: pop (AT&T Research) 2003-04-05 $\n]"
USAGE_LICENSE
"[+NAME?pop - operate on partioned fixed row and column data]"
"[+DESCRIPTION?\bpop\b operates on partitioned fixed row and column data files."
"	It can cut high or low frequency partition columns, list format field"
"	names for partition columns, and list the partition column frequencies."
"	See \bpzip\b(1) for a detailed description of file partitions"
"	and column frequencies.]"

"[c:cut?Copy selected columns from the input rows to the standard output.]"
"[e:endiff?Copy the row-by-row difference to the standard output.]"
"[f:format?Specifies the data format (schema) file. Two input styles"
"	are accepted. The first style lists field names and sizes in"
"	consecutive order: `\bname\b,\asize\a[,\acomment\a...]]'. The second"
"	style lists the field offset range and name:"
"	`\abegin\a[-\aend\a]] \bname\b'. Column offsets start at 0."
"	Names are used to label partition group listings on the standard"
"	output, with partition groups separated by an empty line.]:[file]"
"[h:high?List information on high frequency columns only. This is"
"	the default.]"
"[i:information?List the selected column frequency information on the"
"	standard output.]"
"[l:low?List information on low frequency columns only.]"
"[m:map?List the partition file with the row size equal to the number of"
"	high frequency columns and the high frequency columns renumbered"
"	in order from 0. This partition file can then be used on high"
"	frequency data produced by the \b--cut\b option.]"
"[n:newline?Append a newline to each cut output row.]"
"[o:override?Override the column partition. Currently only fixed value"
"	columns may be specified. The syntax is"
"	\abegin\a[-\aend\a]]='\avalue\a' where \abegin\a is the beginning"
"	column offset (starting at 0), \aend\a is the ending column offset"
"	for an inclusive range, and \avalue\a is the fixed column value."
"	Uncompress time is improved when high frequency columns are given"
"	fixed values (see the \b--partition\b option).]:[name=value]"
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
"[r:row?Specifies the input row size (number of byte columns). Exactly"
"	one of \b--row\b or \b--partition\b must be specified.]#[row-size]"
"[u:undiff?The inverse of the \b--endiff\b difference encoding.]"
"[v:verbose?List header information on the input \apzip\a file or"
"	\apartition-file\a and continue processing.]"
"[x:identify?Identify output information columns with labels from the"
"	\b--format\b file.]"
"[Q:regress?Generate output for regression testing, such that identical"
"	invocations with identical input files will generate the same output.]"
"[T:test?Enable implementation-specific tests and tracing.]#[test-mask]"
"[X:prefix?Uncompressed data contains a prefix that is defined by \acount\a"
"	and an optional \aterminator\a. This data is not \bpzip\b compressed."
"	\aterminator\a may be one of:]:[count[*terminator]]]{"
"		[+\aomitted\a?\acount\a bytes.]"
"		[+L?\acount\a \bnewline\b terminated records.]"
"		[+'\achar\a'?\acount\a \achar\a terminated records.]"

"\n"
"\n[ file ]\n"
"\n"
"[+SEE ALSO?\bgzip\b(1), \bpin\b(1), \bpzip\b(1), \bpzip\b(3)]"
;

#include <ast.h>
#include <ctype.h>
#include <error.h>
#include <pzip.h>
#include <tok.h>

#define OP_CUT		0x0001
#define OP_ENDIFF	0x0002
#define OP_ID		0x0004
#define OP_INFO		0x0008
#define OP_LO		0x0010
#define OP_MAP		0x0020
#define OP_NL		0x0040
#define OP_UNDIFF	0x0100
#define OP_VERBOSE	0x0200

typedef struct
{
	char*		name;
	int		beg;
	int		end;
} Label_t;

typedef struct
{
	unsigned char	hit[UCHAR_MAX+1];	/* values seen		*/
	unsigned long	changes;		/* number of changes	*/
	unsigned int	values;			/* # different values	*/
	int		prev;			/* prev row value	*/
} Info_t;

/*
 * gather stats from sp into ip
 */

static ssize_t
gather(register Pz_t* pz, register Pzpart_t* pp, Sfio_t* sp, register Info_t* ip, size_t* map, size_t m)
{
	register int		i;
	register int		j;
	register unsigned char*	buf;
	register size_t		n;
	register ssize_t	r;
	register size_t		rows;

	for (i = 0; i < m; i++)
		ip[i].prev = -1;
	rows = 0;
	for (;;)
	{
		buf = pz->buf;
		if ((r = sfread(sp, buf, pz->win)) < (ssize_t)pp->row)
		{
			if (r < 0)
			{
				error(ERROR_SYSTEM|2, "read error");
				return -1;
			}
			if (r > 0)
				error(1, "last record incomplete");
			break;
		}
		for (rows += (n = r / pp->row); n--; buf += pp->row)
			for (i = 0; i < m; i++)
				if (ip[i].prev != buf[j = map[i]])
				{
					ip[i].hit[ip[i].prev = buf[j]] = 1;
					ip[i].changes++;
				}
	}
	for (i = 0; i < m; i++)
		for (j = 0; j < elementsof(ip[i].hit); j++)
			if (ip[i].hit[j])
				ip[i].values++;
	return rows;
}

/*
 * cut hi (default) or lo cols from stdin to stdout
 */

static int
cut(register Pz_t* pz, register Pzpart_t* pp, int op, register size_t* map, size_t m)
{
	register int		i;
	register int		j;
	register size_t		n;
	register ssize_t	r;
	register unsigned char*	ib;
	register unsigned char*	ob;

	if (op & OP_VERBOSE)
		for (n = 0; n < m; n++)
			error(0, "map %3d => %3d", map[n], n);
	if (!(pz->wrk = vmnewof(pz->vm, 0, unsigned char, pz->win, 0)))
		error(ERROR_SYSTEM|3, "out of space");
	for (;;)
	{
		ib = pz->buf;
		ob = pz->wrk;
		if ((r = sfread(pz->io, ib, pz->win)) < (ssize_t)pp->row)
		{
			if (r > 0)
				error(1, "last record incomplete");
			break;
		}
		n = r / pp->row;
		for (i = 0; i < n; i++)
		{
			if (op & OP_ID)
			{
				*ob++ = i >> 8;
				*ob++ = i;
			}
			for (j = 0; j < m; j++)
				*ob++ = ib[map[j]];
			if (op & OP_NL)
				*ob++ = '\n';
			ib += pp->row;
		}
		n = ob - pz->wrk;
		if (sfwrite(sfstdout, pz->wrk, n) != (ssize_t)n)
			error(ERROR_SYSTEM|3, "write error");
	}
	return 0;
}

/*
 * label the mapped format fields
 */

static int
label(register Pz_t* pz, Pzpart_t* pp, int op, register size_t* map, size_t m, char* format)
{
	register char*	s;
	register int	i;
	register int	g;
	ssize_t		rows;
	Sfio_t*		sp;
	Label_t*	lv;
	Label_t*	lp;
	Label_t**	xv;
	Info_t*		ip;

	if (!(sp = pzfind(pz, format, "fmt", "r")))
		error(ERROR_SYSTEM|3, "%s: cannot read format file", format);
	if (!(lv = vmnewof(pz->vm, 0, Label_t, pp->row + 1, 0)))
		error(ERROR_SYSTEM|3, "out of space");
	if (!(xv = vmnewof(pz->vm, 0, Label_t*, pp->row, 0)))
		error(ERROR_SYSTEM|3, "out of space");
	error_info.file = format;
	lv->end = -1;
	lp = ++lv;
	while (s = sfgetr(sp, '\n', 1))
	{
		error_info.line++;
		for (; isspace(*s); s++);
		if (!*s || *s == '#' || *s == '"')
			continue;
		if (!isdigit(*s))
		{
			if (tokscan(s, NiL, "%s, %d,", &lp->name, &lp->end) != 2)
				continue;
			lp->beg = (lp-1)->end + 1;
			lp->end += lp->beg - 1;
		}
		else if (tokscan(s, NiL, "%d-%d %s", &lp->beg, &lp->end, &lp->name) != 3)
			continue;
		if (streq(lp->name, "variable_ascii"))
			continue;
		if (streq(lp->name, "Newline"))
			break;
		if (lp->end >= pp->row)
			error(3, "format entry extends beyond %I*d row size", sizeof(pp->row), pp->row);
		if (!(lp->name = vmstrdup(pz->vm, lp->name)))
			error(ERROR_SYSTEM|3, "out of space");
		for (i = lp->beg; i <= lp->end; i++)
			xv[i] = lp;
		if (pz->test & 0x0010)
			error(2, "%d-%d\t%s", lp->beg, lp->end, lp->name);
		lp++;
	}
	sfclose(sp);
	lp->name = "Newline";
	lp->beg = lp->end = (lp-1)->end + 1;
	if (lp->end != (pp->row - 1))
		error(3, "format file row size %d does not match expected %I*d", lp->end + 1, sizeof(pp->row), pp->row);
	xv[lp->beg] = lp;
	error_info.file = 0;
	error_info.line = 0;
	if (op & OP_INFO)
	{
		if (!(ip = vmnewof(pz->vm, 0, Info_t, m, 0)))
			error(ERROR_SYSTEM|3, "out of space");
		if ((rows = gather(pz, pp, pz->io, ip, map, m)) < 0)
			return 1;
		sfprintf(sfstdout, "%s frequency info over %I*d rows\n\n", (op & OP_LO) ? "low" : "high", sizeof(rows), rows);
		sfprintf(sfstdout, "%33s %3s %6s %3s\n\n", "FIELD", "COL", "FREQ", "VAL");
		if (op & OP_LO)
			g = map[0];
		else
			g = 0;
		for (i = g = 0; i < m; i++)
		{
			if (op & OP_LO)
			{
				if (g != map[i])
					sfprintf(sfstdout, "\n");
				g = map[i] + 1;
			}
			else if (g != pp->lab[i])
			{
				g = pp->lab[i];
				sfprintf(sfstdout, "\n");
			}
			sfprintf(sfstdout, "%33s %3d %6lu %3d\n", xv[map[i]]->name, map[i], ip[i].changes, ip[i].values);
		}
	}
	else
		for (i = 0; i < m;)
		{
			lp = xv[map[i]];
			if (op & OP_LO)
				g = map[i] + 1;
			else
				g = pp->lab[i];
			sfprintf(sfstdout, "%33s %3d", lp->name, map[i]);
			while (++i < m)
			{
				if (op & OP_LO)
				{
					if (g != map[i])
					{
						sfprintf(sfstdout, "\n");
						break;
					}
					g = map[i] + 1;
				}
				else if (g != pp->lab[i])
				{
					sfprintf(sfstdout, "\n");
					break;
				}
				if (xv[map[i]] != lp)
					break;
				sfprintf(sfstdout, " %3d", map[i]);
			}
			sfprintf(sfstdout, "\n");
		}
	return 0;
}

/*
 * list info on the mapped fields
 */

static int
info(register Pz_t* pz, register Pzpart_t* pp, int op, register size_t* map, size_t m)
{
	register int	i;
	register int	g;
	ssize_t		rows;
	Info_t*		ip;

	if (!(ip = vmnewof(pz->vm, 0, Info_t, m, 0)))
		error(ERROR_SYSTEM|3, "out of space");
	if ((rows = gather(pz, pp, pz->io, ip, map, m)) < 0)
		return 1;
	sfprintf(sfstdout, "%s frequency info over %I*d rows\n\n", (op & OP_LO) ? "low" : "high", sizeof(rows), rows);
	sfprintf(sfstdout, "%3s %6s %3s\n\n", "COL", "FREQ", "VAL");
	if (op & OP_LO)
		g = map[0];
	else
		g = 0;
	for (i = g = 0; i < m; i++)
	{
		if (op & OP_LO)
		{
			if (g != map[i])
				sfprintf(sfstdout, "\n");
			g = map[i] + 1;
		}
		else if (g != pp->lab[i])
		{
			g = pp->lab[i];
			sfprintf(sfstdout, "\n");
		}
		sfprintf(sfstdout, "%3d %6lu %3d\n", map[i], ip[i].changes, ip[i].values);
	}
	return 0;
}

/*
 * copy the row by row diff of path to sfstdout
 */

static int
diff(int op, const char* path, size_t row)
{
	register int	i;
	register int	j;
	register int	k;
	ssize_t		r;
	unsigned char*	buf[2];
	unsigned char*	dif;
	Sfio_t*		sp;

	if (!(buf[0] = newof(0, unsigned char, row, 0)) || !(buf[1] = newof(0, unsigned char, row, 0)) || !(dif = newof(0, unsigned char, row, 0)))
	{
		error(ERROR_SYSTEM|2, "out of space");
		return 1;
	}
	if (!(sp = sfopen(NiL, path, "r")))
	{
		error(ERROR_SYSTEM|2, "%s: cannot read", path);
		return 1;
	}
	if (op & OP_ENDIFF)
	{
		for (i = 0; (r = sfread(sp, buf[i], row)) == row; i = k)
		{
			k = !i;
			for (j = 0; j < row; j++)
				dif[j] = buf[i][j] - buf[k][j];
			if (sfwrite(sfstdout, dif, row) != row)
				break;
		}
	}
	else
	{
		for (i = 0; (r = sfread(sp, dif, row)) == row; i = k)
		{
			k = !i;
			for (j = 0; j < row; j++)
				buf[i][j] = dif[j] + buf[k][j];
			if (sfwrite(sfstdout, buf[i], row) != row)
				break;
		}
	}
	sfclose(sp);
	if (sfsync(sfstdout))
	{
		error(ERROR_SYSTEM|2, "write error");
		return 1;
	}
	if (r < 0)
	{
		error(ERROR_SYSTEM|2, "%s: read error", path);
		return 1;
	}
	if (r)
		error(1, "%s: last record incomplete", path);
	return 0;
}

int
main(int argc, char** argv)
{
	register Pz_t*		pz;
	register Pzpart_t*	pp;
	register int		i;
	int			m;
	size_t*			map;
	Pzdisc_t		disc;
	Sfio_t*			dp;

	int			flags = 0;
	char*			format = 0;
	int			op = 0;
	size_t			row = 0;

	error_info.id = "pop";
	memset(&disc, 0, sizeof(disc));
	disc.version = PZ_VERSION;
	disc.errorf = errorf;
	if (!(dp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [options]");
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'c':
			op |= OP_CUT;
			continue;
		case 'e':
			op |= OP_ENDIFF;
			continue;
		case 'f':
			format = opt_info.arg;
			continue;
		case 'h':
			op &= ~OP_LO;
			continue;
		case 'i':
			op |= OP_INFO;
			continue;
		case 'l':
			op |= OP_LO;
			continue;
		case 'm':
			op |= OP_MAP;
			continue;
		case 'n':
			op |= OP_NL;
			continue;
		case 'o':
			sfputr(dp, opt_info.arg, '\n');
			continue;
		case 'p':
			disc.partition = opt_info.arg;
			continue;
		case 'r':
			row = opt_info.num;
			continue;
		case 'u':
			op |= OP_UNDIFF;
			continue;
		case 'v':
			op |= OP_VERBOSE;
			flags |= PZ_VERBOSE;
			continue;
		case 'x':
			op |= OP_ID;
			continue;
		case 'Q':
			sfprintf(dp, "regress\n");
			continue;
		case 'T':
			sfprintf(dp, "test=%s\n", opt_info.arg);
			continue;
		case 'X':
			sfprintf(dp, "prefix=%s\n", opt_info.arg);
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			if (!opt_info.option[0])
				sfputr(dp, &argv[opt_info.index - 1][2], '\n');
			else
				error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || *argv && *(argv + 1))
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (sfstrtell(dp) && !(disc.options = strdup(sfstruse(dp))))
		error(ERROR_SYSTEM|3, "out of space [options]");
	sfstrclose(dp);
	if (op & (OP_ENDIFF|OP_UNDIFF))
	{
		if (!row)
			error(3, "-r row-size required for -e");
		return diff(op, *argv, row);
	}
	if (row)
	{
		if (disc.partition)
			error(3, "only one of -r and -p may be specified");
		if (!(disc.partition = strdup(sfprints("/%I*u/", sizeof(row), row))))
			error(ERROR_SYSTEM|3, "out of space");
	}
	if (!disc.partition)
		flags |= PZ_READ;
	else if (op & OP_INFO)
		flags |= PZ_WRITE;
	if (!(pz = pzopen(&disc, *argv, flags)))
		return 1;
	pp = pz->part;
	if (!disc.partition && (op & OP_INFO))
	{
		sfprintf(sfstdout, "row size %d\n", pp->row);
		op |= OP_LO;
	}
	pz->win = (pz->win / pp->row) * pp->row;
	if (op & OP_LO)
	{
		if (!(map = vmnewof(pz->vm, 0, size_t, pp->row - pp->nmap, 0)))
			error(ERROR_SYSTEM|3, "out of space");
		m = 0;
		for (i = 0; i < pp->row; i++)
			if (pp->low[i])
				map[m++] = i;
	}
	else
	{
		map = pp->map;
		m = pp->nmap;
	}
	if (op & OP_CUT)
		i = cut(pz, pp, op, map, m);
	else if (format)
		i = label(pz, pp, op, map, m, format);
	else if (op & OP_INFO)
		i = info(pz, pp, op, map, m);
	else if (op & OP_MAP)
	{
		pp->row = pp->nmap;
		for (i = 0; i < pp->nmap; i++)
			pp->map[i] = i;
		pzpartprint(pz, pp, sfstdout);
	}
	pzclose(pz);
	return i;
}
