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
 * partitioned fixed record zip
 */

static const char usage[] =
"[-?\n@(#)$Id: pzip (AT&T Research) 2003-07-17 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - fixed record partition compress/decompress]"
"[+DESCRIPTION?\b\f?\f\b compresses and decompresses data files of fixed"
"	length rows (records) and columns (fields). It performs better than"
"	\bgzip\b(1) in space/time on data that has many (typically > 50%)"
"	columns that change at a low rate (columns with a low rate of change"
"	are low frequency; columns with a high rate of change are high"
"	frequency).]"
"[+?The \apzip\a compress format is itself \agzipped\a; decompressed data"
"	is reorganized according to the user-specified \apartition\a file"
"	(see the \b--partition\b option below) before being passed to"
"	\agzip\a. Low frequency columns are difference encoded and high"
"	frequency column groups are transposed to column-major order."
"	The \agzip\a tables are flushed between each column partition group."
"	This has a positive space/time effect on the \agzip\a string match"
"	and huffman tables.]"
"[+?If a \apartition\a file is specified then \apzip\a compresses the input"
"	\afile\a to the standard output, otherwise \apzip\a decompresses"
"	the input \afile\a to the standard ouput. If \afile\a is omitted"
"	then the standard input is used. If the standard input is a tty"
"	then \b/dev/null\b is silently used.]"
"[+?\afile\a may be \apzip\a compressed, \agzip\a compressed, or raw."
"	\apzip\a files self-identify; the row size and partition can be"
"	determined from the \apzip\a header. For \agzip\a and raw data,"
"	the following are done to determine the row size:]{"
"		[+(1)?The row size is taken from the \b--row\b option"
"			if specified.]"
"		[+(2)?If the \b--partition\b option is specified then the"
"			row size is taken from the \apartition\a file.]"
"		[+(3)?If the data is newline-terminated and if it contains"
"			at least two lines and if the first two data lines"
"			have the same length then that length is taken"
"			to be the row size.]"
"		[+(4)?Otherwise the row size cannot be determined and"
"			\apzip\a exits with a diagnostic.]"
"}"

"[a:append?Sets the \bPZ_APPEND\b flag that may be used by some disciplines.]"
"[b:bzip?Use \bbzip\b(1) compression instead of the default \bgzip\b(1)."
"	\abzip\a is not fully supported, pending further investigation.]"
"[c:comment?Place \acomment\a in the output \apzip\a file header when"
"	compressing. The comment is listed by the \b--header\b option.]:"
"		[comment]"
"[x:crc?Enable \agzip\a crc32 cyclic redundancy checking for decompress."
"	On some systems this can double the execution wall time."
"	Most data corruption errors are still caught even with \bnocrc\b.]"
"[d:dump?Enable detailed tracing.]"
"[B:bufsize?Set the output buffer size to \asize\a -- for debugging.]#[size]"
"[D:debug?Set the debug trace level to \alevel\a. Higher levels produce"
"	more output.]#[level]"
"[O:dio?Push the \bsfdcdio\b(3) direct io discipline on the input streams."
"	Silently ignored on systems that do not support direct io.]"
"[G!:gzip?\b--nogzip\b disables \agzip\a compression. Most often used for"
"	conversion or debugging.]"
"[h:header?List header information on the input \apzip\a file and exit."
"	This output is compatible with the \b--partition\b file format.]"
"[l:library?Loads the dll \alibrary\a via the \apzlib\a() call."
"	\alibrary\a must contain the function"
"	\bint pz_init(Pz_t* pz, Pzdisc_t* disc)\b"
"	which is called during \apzip\a stream initialization. \bpz_init\b"
"	allows run time modification to \adisc\a: most often it supplies"
"	alternate discipline functions. Runtime libraries may interpret"
"	options specific to the library; library usage and description"
"	will be appended to online help output if the help options"
"	appear after the \b--library\b option. Runtime libraries may"
"	provide additional diagnostics and tracing when \b--summary\b,"
"	\b--verbose\b or \b--dump\b are specified. In general, runtime"
"	libraries are not needed for decompression. The \b--header\b"
"	option lists the runtime libraries used to compress the input"
"	file.]:[library]"
"[z:lzw?Use \bcompress\b(1) lzw compression instead of the default \bgzip\b(1)."
"	\alzw\a is not fully supported, pending further investigation.]"
"[o:override?Override the column partition. Currently only fixed value"
"	columns may be specified. The syntax is"
"	\abegin\a[-\aend\a]]='\avalue\a' where \abegin\a is the beginning"
"	column offset (starting at 0), \aend\a is the ending column offset"
"	for an inclusive range, and \avalue\a is the fixed column value."
"	Uncompress time is improved when high frequency columns are given"
"	fixed values (see the \b--partition\b option).]:[begin[-end]]=value]"
"[p:partition?\afile\a specifies the data row size and the high frequency"
"	column partition groups and permutation. \afile\a may contain URL-like"
"	components: \apath\a\b?name=\b\apart\a or \apath\a\b#\b\apart\a"
"	reads the partition file \apath\a and uses the partition named"
"	\apart\a. Other options may be set by separating each with , or space."
"	The partition file is a sequence of lines. Comments start with # and"
"	continue to the end of the line. The first non-comment line specifies"
"	the optional name string in \"...\". The next non-comment line"
"	specifies the row size. The remaining lines operate on column offset"
"	ranges of the form: \abegin\a[-\aend\a]] where \abegin\a is the"
"	beginning column offset (starting at 0), and \aend\a is the ending"
"	column offset for an inclusive range. The file name \b//\b or"
"	\b/gzip/\b disables \bpzip\b partitioning and applies only"
"	\bgzip\b compression. The operators are:]:[file]{"
"		[+range [...]]?places all columns in the specified \arange\a"
"			list in the same high frequency partition group."
"			Each high frequency partition group is processed as"
"			a separate block by the underlying compressor"
"			(\bgzip\b(1) by default).]"
"		[+range='value'?specifies that each column in \arange\a"
"			has the fixed character value \avalue\a. C-style"
"			character escapes are valid for \avalue\a.]"
"}"
"[Z:push?Push the \bsfdcpzip\b(3) io discipline rather than direct library"
"	calls. Used for debugging and performance testing.]"
"[P!:pzip?\b--nopzip\b disables \apzip\a compression. Most often used for"
"	conversion or debugging.]"
"[Q:regress?Generate output for regression testing, such that identical"
"	invocations with identical input files will generate the same output.]"
"[r:row?Specifies the input row size (number of byte columns) for data"
"	that does not self-identify.]#[row-size]"
"[S:split?Instead of compressing, the input split discipline, which must be"
"	specified by a subsequent \b--library\b option, splits the input"
"	data into files named \aid\a. \aid\a is determined by the split"
"	discipline \bnamef\b function. The optional \apattern\a is a \bksh\b(1)"
"	file match pattern that limits the split to \aid\a's matching"
"	\apattern\a (e.g., \b--split='1234|98765'\b.) If \b--append\b is also"
"	specified then the data is appened to any pre-existing \aid\a files;"
"	otherwise each file is truncated when the first record containing"
"	\aid\a data is read. If there are no records with \aid\a data then"
"	the \aid\a file is not modified. \b--split\b should be used in a"
"	separate directory, and the directory should be cleared when"
"	\b--append\b is not specified to avoid mixing inconsistent"
"	data. No records will be written to a split file with size"
"	>= \b--window\b bytes.]:?[pattern]"
"[s:summary?Enable summary tracing to the standard error. Runtime libraries"
"	may add addtional information to the default \bpzip\b(3) library"
"	summary output. Compression summary includes the compression rate,"
"	bytes per record, and compression wall time. This option also"
"	enables split discipline warnings about \aid\a partitions that"
"	should be generated by \bpin\b(1) and added to the partition"
"	file to improve compression. The \bpin\b(1) output, with an additional"
"	\"\aid\a\" line manually prepended, can then be appended to an existing"
"	partition file.]"
"[T:test?Enable \bpzip\b(3) implementation-specific tests and tracing.]#"
"		[mask]"
"[v:verbose?Enable intermediate tracing.]"
"[w:window?Each chunk of \awindow\a bytes is compressed separately. The"
"	window size may be silently decreased to accomodate an integral"
"	number of complete rows.]#[window-size:=4M]"
"[W:write-test?Loop on \asfread\a()/\bpzwrite\b(3) in chunks of \agroup\a"
"	records rather than a single \apzdeflate\a() call for compression."
"	Used for debugging and performance testing.]#[group]"
"[X:prefix?Uncompressed data contains a prefix that is defined by \acount\a"
"	and an optional \aterminator\a. This data is preserved but is not"
"	\bpzip\b compressed. If \acount\a is \b0\b on uncompress then the"
"	header is not copied to the output. \aterminator\a may be one"
"	of:]:[count[*terminator]]]{"
"		[+\aomitted\a?\acount\a bytes.]"
"		[+L?\acount\a \bnewline\b terminated records.]"
"		[+'\achar\a'?\acount\a \achar\a terminated records.]"
"}"

"\n"
"\nfile\n"
"\n"
"[+SEE ALSO?\bbzip\b(1), \bgzip\b(1), \bpin\b(1), \bpop\b(1), \bpzip\b(3)]"
"[+BUGS?\bpzip\b decompress currently fails if the standard input is a pipe."
"	This will be addressed in a future release.]"
;

#include <ast.h>
#include <error.h>
#include <pzip.h>
#include <sfdcbzip.h>

typedef int (*Method_f)(Sfio_t*, int);

int
main(int argc, char** argv)
{
	register Pz_t*	pz;
	Pzdisc_t	disc;
	Sfio_t*		dp;
	char*		s;

	Method_f	method = 0;
	ssize_t		bufsize = 0;
	int		push = 0;
	int		testwrite = 0;
	unsigned long	flags = PZ_READ|PZ_FORCE;

	if (s = strrchr(*argv, '/'))
		s++;
	else
		s = *argv;
	error_info.id = s;
	memset(&disc, 0, sizeof(disc));
	disc.version = PZ_VERSION;
	disc.errorf = errorf;
	if (!(dp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space");
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			flags |= PZ_APPEND;
			continue;
		case 'b':
			method = sfdcbzip;
			continue;
		case 'c':
			disc.comment = opt_info.arg;
			continue;
		case 'd':
			flags |= PZ_DUMP;
			continue;
		case 'h':
			flags &= ~(PZ_READ|PZ_WRITE|PZ_FORCE);
			flags |= PZ_STAT|PZ_DUMP;
			continue;
		case 'l':
			sfprintf(dp, "library=%s\n", opt_info.arg);
			continue;
		case 'o':
			sfputr(dp, opt_info.arg, '\n');
			continue;
		case 'p':
			disc.partition = opt_info.arg;
			flags &= ~(PZ_READ|PZ_FORCE);
			flags |= PZ_WRITE;
			continue;
		case 'r':
			sfprintf(dp, "row=%d\n", opt_info.num);
			continue;
		case 's':
			flags |= PZ_SUMMARY;
			continue;
		case 'S':
			flags |= PZ_SPLIT;
			sfprintf(dp, "split%s%s\n", opt_info.arg ? "=" : "", opt_info.arg ? opt_info.arg : "");
			continue;
		case 'v':
			flags |= PZ_VERBOSE;
			continue;
		case 'w':
			disc.window = opt_info.num;
			continue;
		case 'x':
			flags |= PZ_CRC;
			continue;
		case 'z':
			method = sfdclzw;
			continue;
		case 'B':
			bufsize = opt_info.num;
			continue;
		case 'D':
			error_info.trace = -opt_info.num;
			continue;
		case 'O':
			flags |= PZ_DIO;
			continue;
		case 'G':
			flags |= PZ_NOGZIP;
			continue;
		case 'P':
			flags |= PZ_NOPZIP;
			continue;
		case 'Q':
			sfprintf(dp, "regress\n");
			continue;
		case 'T':
			sfprintf(dp, "test=%s\n", opt_info.arg);
			continue;
		case 'W':
			testwrite = opt_info.num;
			continue;
		case 'X':
			sfprintf(dp, "prefix=%s\n", opt_info.arg);
			continue;
		case 'Z':
			push = 1;
			continue;
		case '?':
			if (opt_info.name[0] == '-' && opt_info.name[1] == '-')
				sfputr(dp, opt_info.name + 2, '\n');
			else
				sfputr(dp, "??short", '\n');
			continue;
		case ':':
			if (!opt_info.option[0] && opt_info.name[0] == opt_info.name[1] || opt_info.option[0] == '-' && opt_info.option[1] == '?')
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
	if (sftell(dp) && !(disc.options = strdup(sfstruse(dp))))
		error(ERROR_SYSTEM|3, "out of space");
	sfclose(dp);
	if (flags & PZ_SPLIT)
	{
		flags &= ~PZ_WRITE;
		flags |= PZ_READ;
	}
	if (push)
	{
		if (*argv)
			error(3, "%s: file argument not expected for sfdcpzip discipline test", *argv);
		if (sfdcpzip(sfstdin, NiL, flags, &disc) < 0)
			error(3, "sfdcpzip discipline push error");
		if (sfmove(sfstdin, sfstdout, SF_UNBOUND, -1) < 0 || sfclose(sfstdout))
			error(ERROR_SYSTEM|3, "sfdcpzip io error");
		return 0;
	}
	if (method)
	{
		flags |= PZ_NOGZIP;
		if ((*method)((flags & PZ_WRITE) ? sfstdout : sfstdin, 0) < 0)
			error(3, "compression method discipline push error");
	}
	if (bufsize)
	{
		sfset(sfstdout, SF_SHARE|SF_LINE, 0);
		sfsetbuf(sfstdout, NiL, bufsize);
	}
	if (pz = pzopen(&disc, *argv, flags))
	{
		if (testwrite && (flags & PZ_WRITE))
		{
			unsigned char*	buf;
			size_t		n;
			ssize_t		r;

			n = pz->part->row * testwrite;
			error(1, "pzwrite test %d bytes per chunk", n);
			if (!(buf = newof(0, unsigned char, n, 0)))
				error(ERROR_SYSTEM|3, "out of space [buf]");
			while ((r = sfread(pz->io, buf, n)) > 0)
				if (pzwrite(pz, sfstdout, buf, r) != r)
					return 1;
			if (r < 0)
				error(ERROR_SYSTEM|3, "%s: read error", pz->path);
		}
		return ((flags & PZ_WRITE) && pzdeflate(pz, sfstdout) || (flags & PZ_READ) && pzinflate(pz, sfstdout)) || pzclose(pz) || error_info.errors;
	}
	return 1;
}
