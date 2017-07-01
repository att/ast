/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*              Doug McIlroy <doug@research.bell-labs.com>              *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * sort main
 *
 * algorithm and interface
 *
 *	Glenn Fowler
 *	Phong Vo
 *	AT&T Research
 *
 * key coders
 *
 *	Doug McIlroy	
 *	Bell Laboratories
 */

static const char usage[] =
"[-n?\n@(#)$Id: sort (AT&T Research) 2013-09-19 $\n]"
USAGE_LICENSE
"[+NAME?sort - sort and/or merge files]"
"[+DESCRIPTION?\bsort\b sorts lines of all the \afiles\a together and "
    "writes the result on the standard output. The file name \b-\b means the "
    "standard input. If no files are named, the standard input is sorted.]"
"[+?The default sort key is an entire line. Default ordering is "
    "lexicographic by bytes in machine collating sequence. The ordering is "
    "affected globally by the locale and/or the following options, one or "
    "more of which may appear. See \brecsort\b(3) for details.]"
"[+?For backwards compatibility the \b-o\b option is allowed in any file "
    "operand position when neither the \b-c\b nor the \b--\b options are "
    "specified.]"
"[k:key?Restrict the sort key to a string beginning at \apos1\a and "
    "ending at \apos2\a. \apos1\a and \apos2\a each have the form \am.n\a, "
    "counting from 1, optionally followed by one or more of the flags "
    "\bCMbdfginprZ\b; \bm\b counts fields from the beginning of the line and "
    "\bn\b counts characters from the beginning of the field. If any flags "
    "are present they override all the global ordering options for this key. "
    "If \a.n\a is missing from \apos1\a, it is taken to be 1; if missing "
    "from \apos2\a, it is taken to be the end of the field. If \apos2\a is "
    "missing, it is taken to be end of line. The second form specifies a "
    "fixed record length \areclen\a, and the last form specifies a fixed "
    "field at byte position \aposition\a (counting from 1) of \alength\a "
    "bytes. The obsolescent \breclen:fieldlen:offset\b (byte offset from 0) "
    "is also accepted.]:[pos1[,pos2]]|.reclen|.position.length]]]]]"
"[K:oldkey?Specified in pairs: \b-K\b \apos1\a \b-K\b \apos2\a, where "
    "positions count from 0.]# [pos]"
"[R:record|recfmt?Sets the record format to \aformat\a; newlines will be "
    "treated as normal characters. The formats are:]:[format]"
    "{"
        "[+d[\aterminator\a]]?Variable length with record \aterminator\a "
            "character, \b\\n\b by default.]"
        "[+[f]]\areclen\a?Fixed record length \areclen\a.]"
        "[+v[op...]]?Variable length. \bh4o0z2bi\b (4 byte IBM V format "
            "descriptor) if \aop\a are omitted. \aop\a may be a combination "
            "of:]"
            "{"
                "[+h\an\a?Header size is \an\a bytes (default 4).]"
                "[+o\an\a?Size offset in header is \an\a bytes (default "
                    "0).]"
                "[+z\an\a?Size length is \an\a bytes (default "
                    "min(\bh\b-\bo\b,2)).]"
                "[+b?Size is big-endian (default).]"
                "[+l?Size is little-endian (default \bb\b).]"
                "[+i?Record length includes header (default).]"
                "[+n?Record length does not include header (default "
                    "\bi\b).]"
            "}"
        "[+%?If the record format is not otherwise specified, and the "
            "any input file name, from left to right, ends with "
            "\b%\b\aformat\a or \b%\b\aformat\a\b.\b* then the record format "
            "is set to \aformat\a. In addition, the \b-o\b path, if "
            "specified and if it does not contain \b%\b and if it names a "
            "regular file, is renamed to contain the input \b%\b\aformat\a.]"
        "[+-?The first block of the first input file is sampled to check "
            "for \bv\b variable length and \bf\b fixed length format "
            "records. Not all formats are detected. \bsort\b exits with an "
            "error diagnostic if the record format cannot be determined from "
            "the sample.]"
    "}"
"[b:ignorespace|ignore-leading-blanks?Ignore leading white space (spaces "
    "and tabs) in field comparisons.]"
"[d:dictionary?`Phone directory' order: only letters, digits and white "
    "space are significant in string comparisons.]"
"[E:codeset|convert?The field data codeset is \acodeset\a or the field "
    "data must be converted from the \afrom\a codeset to the \ato\a codeset. "
    "The codesets are:]:[codeset|from::to]"
    "{\fcodesets\f}"
"[f:fold|ignorecase?Fold lower case letters onto upper case.]"
"[h:scaled|human-readable?Compare numbers scaled with IEEE 1541-2002 "
    "suffixes.]"
"[i:ignorecontrol?Ignore characters outside the ASCII range 040-0176 in "
    "string comparisons.]"
"[J:shuffle|jumble?Do a random shuffle of the sort keys. \aseed\a "
    "specifies a pseudo random number generator seed. A \aseed\a of 0 "
    "generates a seed based on time and pid.]#[seed]"
"[n:numeric?An initial numeric string, consisting of optional white "
    "space, optional sign, and a nonempty string of digits with optional "
    "decimal point, is sorted by value.]"
"[g:floating?Numeric, like \b-n\b, with \be\b-style exponents allowed.]"
"[p:bcd|packed-decimal?Compare packed decimal (bcd) numbers with "
    "trailing sign.]"
"[M:months?Compare as month names. The first three characters after "
    "optional white space are folded to lower case and compared. Invalid "
    "fields compare low to \bjan\b.]"
"[r:reverse|invert?Reverse the sense of comparisons.]"
"[t:tabs?`Tab character' separating fields is \achar\a.]:[tab-char]"
"[c:check?Check that the single input file is sorted according to the "
    "ordering rules; give no output on the standard output. If the input "
    "is out of sort then write one diagnostic line on the standard error "
    "and exit with code \b1\b.]"
"[C:silent-check?Like \b--check\b except no diagnostic is written.]"
"[j:processes|nproc|jobs?Use up to \ajobs\a separate processes to sort "
    "the input. The current implementation still uses one process for the "
    "final merge phase; improvements are planned.]#[processes]"
"[m:merge?Merge; the input files are already sorted.]"
"[u:unique?Unique. Keep only the first of multiple records that compare "
    "equal on all keys. Implies \b-s\b.]"
"[s:stable?Stable sort. When all keys compare equal, preserve input "
    "order. The default is \b--nostable\b (\aunstable\a sort): when all "
    "keys compare equal, break the tie by using the entire record, ignoring "
    "all but the \b-r\b option.]"
"[o:output?Place output in the designated \afile\a instead of on the "
    "standard output. This file may be the same as one of the inputs. The "
    "\afile\a \b-\b names the standard output. The option may appear among "
    "the file arguments, except after \b--\b.]:[output]"
"[l:library?Load the external sort discipline \alibrary\a with optional "
    "comma separated \aname=value\a arguments. Libraries are loaded, in left "
    "to right order, after the sort method has been "
    "initialized.]:[library[,name=value...]]]"
"[T:tempdir?Put temporary files in \atempdir\a.]:[tempdir:=/usr/tmp]"
"[L:list?List the available sort methods. See the \b-x\b option.]"
"[x:method?Specify the sort method to apply:]:[method:=rasp]"
    "{\fmethods\f} [v:verbose?Trace the sort progress on the standard "
    "error.]"
"[P:plugins?List plugin information for each \afile\a operand in "
    "--\astyle\a on the standard error. If no \afile\a operands are "
    "specified then the first instance of each \bsort\b plugin installed on "
    "\b$PATH\b or a sibling dir on \b$PATH\b is listed. The special "
    "\astyle\a \blist\b lists a line on the standard output for each plugin "
    "with the name, a tab character, and plugin specific command line "
    "options parameterized by \b${style}\b (suitable for \beval\b'ing in "
    "\bsh\b(1).)]:[style:=list|man|html|nroff|usage]"
"[Z:zd|zoned-decimal?Compare zoned decimal (ZD) numbers with embedded "
    "trailing sign.]"
"[z:size|zip?Suggest using the specified number of bytes of internal "
    "store to tune performance. Power of 2 and power of 10 size suffixes are "
    "accepted. Type is a single character and may be one of:]:[type[size]]]"
    "{"
        "[+a?Buffer alignment.]"
        "[+b?Input reserve buffer size.]"
        "[+c?Input chunk size; sort chunks of this size and disable "
            "merge.]"
        "[+i?Input buffer size.]"
        "[+m?Maximum number of intermediate merge files.]"
        "[+p?Input sort size; sort chunks of this size before merge.]"
        "[+o?Output buffer size.]"
        "[+r?Maximum record size.]"
        "[+I?Decompress the input if it is compressed.]"
        "[+O?\bgzip\b(1) compress the output.]"
    "}"
"[X:test?Enables implementation defined test code. Some or all of these "
    "may be disabled.]:[test]"
    "{"
        "[+dump?List detailed information on the option settings.]"
        "[+io?List io file paths.]"
        "[+keys?List the canonical key for each record.]"
        "[+read?Force input file read by disabling memory mapping.]"
        "[+show?Show setup information and exit before sorting.]"
        "[+test?Immediatly exit with status 0; used to verify this "
            "implementation]"
    "}"
"[D:debug?Sets the debug trace level. Higher levels produce more "
    "output.]# [level]"
"[S|y?Equivalent to \b-zp\b\asize\a; if \asize\a has no suffix then \bki\b "
    "is assumed.]:[size]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+?+\apos1\a -\apos2\a is the classical alternative to \b-k\b, with "
    "counting from 0 instead of 1, and pos2 designating next-after-last "
    "instead of last character of the key. A missing character count in "
    "\apos2\a means 0, which in turn excludes any \b-t\b tab character from "
    "the end of the key. Thus +1 -1.3 is the same as \b-k\b 2,2.3 and +1r -3 "
    "is the same as \b-k\b 2r,3.]"
"[+?Under option \b-t\b\ax\a fields are strings separated by \ax\a; "
    "otherwise fields are non-empty strings separated by white space. White "
    "space before a field is part of the field, except under option \b-b\b. "
    "A \bb\b flag may be attached independently to \apos1\a and \apos2\a.]"
"[+?When there are multiple sort keys, later keys are compared only "
    "after all earlier keys compare equal. Except under option \b-s\b, lines "
    "with all keys equal are ordered with all bytes significant. \b-S\b "
    "turns off \b-s\b, the last occurrence, left-to-right, takes affect.]"
"[+?Sorting is done by a method determined by the \b-x\b option. \b-L\b "
    "lists the available methods. rasp (radix+splay-tree) is the default and "
    "current all-around best.]"
"[+?Single-letter options may be combined into a single string, such as "
    "\b-cnrt:\b. The option combination \b-di\b and the combination of "
    "\b-n\b with any of \b-diM\b are improper. Posix argument conventions "
    "are supported.]"
"[+?Options \b-b\b, \b-c\b, \b-d\b, \b-f\b, \b-i\b, \b-k\b, \b-m\b, "
    "\b-n\b, \b-o\b, \b-r\b, \b-t\b, and \b-u\b are in the Posix and/or "
    "X/Open standards.]"

"[+DIAGNOSTICS?\asort\a comments and exits with non-zero status for "
    "various trouble conditions and for disorder discovered under option "
    "\b-c\b.]"
"[+SEE ALSO?\bcomm\b(1), \bjoin\b(1), \buniq\b(1), \brecsort\b(3)]"
"[+CAVEATS?The never-documented default \apos1\a=0 for cases such as "
    "\bsort -1\b has been abolished. An input file overwritten by \b-o\b is "
    "not replaced until the entire output file is generated in the same "
    "directory as the input, at which point the input is renamed.]"
;

#include <sfio_t.h>
#include <ast.h>
#include <error.h>
#include <debug.h>
#include <ctype.h>
#include <fs3d.h>
#include <ls.h>
#include <option.h>
#include <recsort.h>
#include <recfmt.h>
#include <sfdcgzip.h>
#include <vmalloc.h>
#include <wait.h>
#include <iconv.h>
#include <dlldefs.h>

#define INMIN		(1024)		/* min input buffer size	*/
#define INBRK		(64*INMIN)	/* default heap increment	*/
#define INMAX		(1024*INBRK)	/* max input buffer size	*/
#define INREC		(16*INMIN)	/* record begin chunk size	*/

#define TEST_dump	0x80000000	/* dump the state before sort	*/
#define TEST_io		0x40000000	/* dump io files		*/
#define TEST_keys	0x20000000	/* dump keys			*/
#define TEST_read	0x10000000	/* force sfread()		*/
#define TEST_show	0x08000000	/* show but don't do		*/
#define TEST_reserve	0x04000000	/* force sfreserve()		*/

#define pathstdin(s)	(!(s)||streq(s,"-")||streq(s,"/dev/stdin")||streq(s,"/dev/fd/0"))
#define pathstdout(s)	(!(s)||streq(s,"-")||streq(s,"/dev/stdout")||streq(s,"/dev/fd/1"))

typedef struct Part_s
{
	Sfdisc_t	disc;		/* sfio discipline		*/
	off_t		offset;		/* file offset			*/
	off_t		size;		/* total size at offset		*/
	off_t		remain;		/* read size remaining		*/
} Part_t;

typedef struct Job_s
{
	off_t		offset;		/* file part offset		*/
	off_t		size;		/* file part size		*/
	size_t		chunk;		/* file part chunk		*/
	int		intermediates;	/* number of intermediate files	*/
} Job_t;

typedef struct Sort_s
{
	Rskeydisc_t	disc;		/* rskey discipline		*/
	Rs_t*		rec;		/* rsopen() context		*/
	Rskey_t*	key;		/* rskeyopen() context		*/
	Rsdefkey_f	defkeyf;	/* real defkeyf if TEST_keys	*/
	Sfio_t*		tp;		/* TEST_keys tmp stream		*/
	Sfio_t*		op;		/* output stream		*/
	Job_t*		jobs;		/* multi-proc job table		*/
	char*		overwrite;	/* -o input overwrite tmp file	*/
	char*		buf;		/* input buffer			*/
	Sfio_t*		opened;		/* fileopen() peek stream	*/
	size_t		cur;		/* input buffer index		*/
	size_t		hit;		/* input buffer index overflow	*/
	size_t		end;		/* max input buffer index	*/
	size_t		bufsize;	/* input reserve buffer size	*/
	off_t		total;		/* total size of single file	*/
	unsigned long	test;		/* test bit mask		*/
	int		child;		/* in child process		*/
	int		chunk;		/* chunk the input (no merge)	*/
	int		hadstdin;	/* already has - on input	*/
	int		map;		/* sfreserve() input		*/
	int		mfiles;		/* multi-stage files[] count	*/
	int		nfiles;		/* files[] count		*/
	int		xfiles;		/* max files[] count		*/
	int		preserve;	/* rename() tmp output to input	*/
	int		single;		/* one input file		*/
	int		verbose;	/* trace main actions		*/
	int		zip;		/* sfdcgzip SF_* flags		*/
	Sfio_t*		files[OPEN_MAX > 68 ? 64 : (OPEN_MAX-4)];
} Sort_t;

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register iconv_list_t*	ic;
	register int		n;

	if (streq(s, "codesets"))
	{
		n = 0;
		for (ic = iconv_list(NiL); ic; ic = iconv_list(ic))
			if (ic->ccode >= 0)
				n += sfprintf(sp, "[%c:%s?%s]", ic->match[ic->match[0] == '('], ic->name, ic->desc);
		return n;
	}
	else if (streq(s, "methods"))
		return rskeylist(NiL, sp, 1);
	return 0;
}

/*
 * handle RS_VERIFY event
 */

static int
verify(Rs_t* rs, int op, Void_t* data, Void_t* arg, Rsdisc_t* disc)
{
	if (op == RS_VERIFY)
		error(3, "disorder at record %lld", (Sflong_t)((Rsobj_t*)data)->order);
	return 0;
}

static int
verify_silent(Rs_t* rs, int op, Void_t* data, Void_t* arg, Rsdisc_t* disc)
{
	if (op == RS_VERIFY)
		exit(1);
	return 0;
}

/*
 * return read stream for path
 */

static Sfio_t*
fileopen(register Sort_t* sp, const char* path)
{
	Sfio_t*		fp;

	if (fp = sp->opened)
		sp->opened = 0;
	else
	{
		if (pathstdin(path))
		{
			if (sp->hadstdin)
				error(3, "%s: can only read once", path);
			sp->hadstdin = 1;
			fp = sfstdin;
		}
		else if (!(fp = sfopen(NiL, path, "r")))
			error(ERROR_SYSTEM|3, "%s: cannot open", path);
		if (rsfileread(sp->rec, fp, path))
		{
			if (fp != sfstdin)
				sfclose(fp);
			return 0;
		}
		sfset(fp, SF_SHARE, 0);
		if (sp->zip & SF_READ)
			sfdcgzip(fp, 0);
	}
	return fp;
}

/*
 * prevent ERROR_USAGE|4 messages from exiting
 */

static void
noexit(int code)
{
}

/*
 * list info for one plugin on the standard error
 */

static void
showlib(Sort_t* sp, Rskey_t* kp, const char* name, const char* style)
{
	char*		args;
	char		buf[128];

	if (style)
		sfsprintf(args = buf, sizeof(buf), "%s,%s", name, style);
	else
		args = (char*)name;
	if (!rslib(sp->rec, kp, args, RS_IGNORE) && !style)
		sfprintf(sfstdout, "%s\t--library=%s,${style}\n", name, name);
}

/*
 * list info for all [selected] plugins on the standard error
 */

static int
showplugins(Sort_t* sp, Rskey_t* kp, const char* style)
{
	Dllscan_t*	dls;
	Dllent_t*	dle;
	char*		name;
	void		(*oexit)(int);

	if (streq(style, "list"))
		style = 0;
	else
	{
		oexit = error_info.exit;
		error_info.exit = noexit;
	}
	if (*kp->input)
		while (name = *kp->input++)
			showlib(sp, kp, name, style);
	else if (dls = dllsopen("sort", NiL, NiL))
	{
		while (dle = dllsread(dls))
			showlib(sp, kp, dle->name, style);
		dllsclose(dls);
	}
	if (style)
		error_info.exit = oexit;
	return 0;
}

struct Lib_s; typedef struct Lib_s Lib_t;

struct Lib_s
{
	Lib_t*		next;
	char*		name;
};

/*
 * process argv as in sort(1)
 */

static int
parse(register Sort_t* sp, char** argv)
{
	register Rskey_t*	key = sp->key;
	register int		n;
	register char*		s;
	char*			e;
	char*			p;
	char**			a;
	char**			v;
	size_t			z;
	int			i;
	int			map;
	char*			plugins = 0;
	Lib_t*			firstlib = 0;
	Lib_t*			lastlib = 0;
	Lib_t*			lib;
	Recfmt_t		r;
	Mbstate_t		q;
	int			obsolescent = 1;
	char			opt[64];
	Optdisc_t		optdisc;
	struct stat		st;

	optinit(&optdisc, optinfo);
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 0:
			break;
		case 'c':
		case 'C':
			obsolescent = 0;
			key->meth = Rsverify;
			key->disc->events = RS_VERIFY;
			key->disc->eventf = opt_info.option[1] == 'C' ? verify_silent : verify;
			continue;
		case 'E':
		case 'J':
			sfsprintf(opt, sizeof(opt), "%c%s", opt_info.option[1], opt_info.arg);
			if (rskeyopt(key, opt, 1))
				return 0;
			continue;
		case 'j':
			key->nproc = opt_info.num;
			continue;
		case 'k':
			if (rskey(key, opt_info.arg, 0))
				return -1;
			continue;
		case 'l':
			if (!(lib = newof(0, Lib_t, 1, 0)))
				error(ERROR_SYSTEM|3, "out of space");
			lib->name = opt_info.arg;
			if (lastlib)
				lastlib = lastlib->next = lib;
			else
				firstlib = lastlib = lib;
			continue;
		case 'm':
			key->merge = !!opt_info.num;
			continue;
		case 'o':
			key->output = opt_info.arg;
			continue;
		case 's':
			if (opt_info.num)
				key->type &= ~RS_DATA;
			else
				key->type |= RS_DATA;
			continue;
		case 't':
			if (key->tab[0])
				error(1, "%s: %s conflicts with %s", opt_info.option, *opt_info.arg, key->tab);
			mbtinit(&q);
			if ((n = mbtsize(opt_info.arg, MB_LEN_MAX, &q)) < 1)
			{
				error(1, "%s: %s: invalid tab character", opt_info.option, opt_info.arg);
				n = 0;
			}
			if (*(opt_info.arg + n) || n >= sizeof(key->tab))
				error(1, "%s: %s: single character expected", opt_info.option, opt_info.arg);
			memcpy(key->tab, opt_info.arg, n);
			key->tab[n] = 0;
			continue;
		case 'u':
			key->type &= ~RS_DATA;
			key->type |= RS_UNIQ;
			continue;
		case 'v':
			key->verbose = !!opt_info.num;
			sp->verbose = key->verbose || (key->test & TEST_show);
			continue;
		case 'x':
			if (!(key->meth = rskeymeth(key, opt_info.arg)))
				error(2, "%s: unknown method", opt_info.arg);
			continue;
		case 'z':
			if (isalpha(n = *(s = opt_info.arg)))
				s++;
			else
				n = 'r';
		size:
			z = strton(s, &e, NiL, 1);
			if (*e == '%')
			{
				error(2, "%s %c%s: %% not supported -- do you really want that much memory?", opt_info.option, n, s);
				return -1;
			}
			if (*e || z < ((n == 'm' || n == 'o' || n == 'r' || isupper(n)) ? 0 : 512))
			{
				error(2, "%s %c%s: invalid size", opt_info.option, n, s);
				return -1;
			}
			switch (n)
			{
			case 'a':
				key->alignsize = z;
				break;
			case 'b':
				sp->bufsize = z;
				break;
			case 'c':
				sp->chunk = 1;
				key->alignsize = key->insize = z;
				break;
			case 'i':
				key->insize = z;
				break;
			case 'm':
				if (z <= 0 || z > elementsof(sp->files))
					z = elementsof(sp->files);
				sp->xfiles = z;
				break;
			case 'p':
				key->procsize = z;
				break;
			case 'o':
				key->outsize = z;
				break;
			case 'r':
				key->recsize = z;
				break;
			case 'I':
				sp->zip |= SF_READ;
				break;
			case 'O':
				sp->zip |= SF_WRITE;
				break;
			}
			continue;
		case 'D':
			error_info.trace = -opt_info.num;
			continue;
		case 'K':
			if (opt_info.offset)
			{
				opt_info.offset = 0;
				opt_info.index++;
			}
			if (rskey(key, opt_info.arg, *opt_info.option))
				return -1;
			continue;
		case 'L':
			rskeylist(key, sfstdout, 0);
			exit(0);
		case 'P':
			plugins = opt_info.arg;
			continue;
		case 'R':
			key->disc->data = recstr(opt_info.arg, &e);
			if (*e)
			{
				error(2, "%s: invalid record format", opt_info.arg);
				return -1;
			}
			continue;
		case 'S':
		case 'y':
			n = 'p';
			s = opt_info.arg;
			if (*s && *(e = s + strlen(s) - 1) != '%' && !isalpha(*e))
			{
				sfsprintf(opt, sizeof(opt), "%ski", s);
				s = opt;
			}
			goto size;
		case 'T':
			pathtemp(NiL, 0, opt_info.arg, "/TMPPATH", NiL);
			continue;
		case 'X':
			s = opt_info.arg;
			opt_info.num = strton(s, &e, NiL, 1);
			if (*e)
			{
				if (streq(s, "dump"))
					opt_info.num = TEST_dump;
				else if (streq(s, "io"))
					opt_info.num = TEST_io;
				else if (streq(s, "keys"))
					opt_info.num = TEST_keys;
				else if (streq(s, "read"))
					opt_info.num = TEST_read;
				else if (streq(s, "reserve"))
					opt_info.num = TEST_reserve;
				else if (streq(s, "show"))
					opt_info.num = TEST_show;
				else if (streq(s, "test"))
				{
					sfprintf(sfstdout, "ok\n");
					exit(0);
				}
				else
					error(1, "%s: unknown test", s);
			}
			if (*opt_info.option == '+')
				key->test &= ~opt_info.num;
			else
				key->test |= opt_info.num;
			sp->test = key->test;
			sp->verbose = key->verbose || (key->test & TEST_show);
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			return 1;
		case ':':
			error(2, "%s", opt_info.arg);
			return -1;
		default:
			opt[0] = opt_info.option[1];
			opt[1] = 0;
			if (rskeyopt(key, opt, 1))
				return 0;
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (obsolescent && (opt_info.index <= 1 || !streq(*(argv - 1), "--")))
	{
		/*
		 * check for obsolescent `-o output' after first file operand
		 */

		a = v = argv;
		while (s = *a++)
		{
			if (*s == '-' && *(s + 1) == 'o')
			{
				if (!*(s += 2) && !(s = *a++))
				{
					error(2, "-o: output argument expected");
					break;
				}
				key->output = s;
			}
			else
				*v++ = s;
		}
		*v = 0;
	}
	key->input = argv;

	/*
	 * disciplines have the opportunity to modify key info
	 */

	while (lib = firstlib)
	{
		if (rslib(sp->rec, key, lib->name, 0))
			return 1;
		firstlib = firstlib->next;
		free(lib);
	}

	/*
	 * plugins list bails early
	 */

	if (plugins)
		exit(showplugins(sp, key, plugins));

	/*
	 * record format chicanery
	 */

	if (map = RECTYPE(key->disc->data) == REC_method && REC_M_INDEX(key->disc->data) == REC_M_path)
		for (n = 0, i = -1; p = key->input[n]; n++)
			if (s = strrchr(p, '%'))
			{
				r = recstr(s + 1, &e);
				if (!*e || *e == '.' && e > (s + 1))
				{
					if (r != key->disc->data && i >= 0 && (RECTYPE(r) != REC_variable || RECTYPE(key->disc->data) != REC_variable || REC_V_ATTRIBUTES(r) != REC_V_ATTRIBUTES(key->disc->data)))
					{
						error(2, "%s: format %s incompatible with %s format %s", p, fmtrec(r, 0), key->input[i], fmtrec(key->disc->data, 0));
						return 1;
					}
					if (RECTYPE(r) != REC_variable || RECTYPE(key->disc->data) != REC_variable || REC_V_SIZE(key->disc->data) < REC_V_SIZE(r))
						key->disc->data = r;
					i = n;
				}
			}
	if (RECTYPE(key->disc->data) == REC_method && ((n = REC_M_INDEX(key->disc->data)) == REC_M_path || n == REC_M_data))
	{
		if ((sp->opened = fileopen(sp, key->input[0])) && (s = sfreserve(sp->opened, SF_UNBOUND, SF_LOCKR)))
		{
			struct stat	st;

			z = sfvalue(sp->opened);
			if (fstat(sffileno(sp->opened), &st) || st.st_size < z)
				st.st_size = 0;
			key->disc->data = recfmt(s, z, st.st_size);
			sfread(sp->opened, s, 0);
		}
		else
		{
			z = sp->opened ? sfvalue(sp->opened) : -1;
			key->disc->data = REC_N_TYPE();
		}
		if (z && key->disc->data == REC_N_TYPE())
			error(3, "%s: record format cannot be determined from data sample", key->input[0]);
	}
	if (RECTYPE(key->disc->data) == REC_fixed)
		key->fixed = REC_F_SIZE(key->disc->data);
	if (map && key->output && key->disc->data != REC_N_TYPE() && (stat(key->output, &st) || S_ISREG(st.st_mode)))
	{
		if (p = strrchr(key->output, '/'))
			s = p + 1;
		else
			s = key->output;
		if (!strchr(s, '%'))
		{
			p = key->output;
			if (!(e = strrchr(s, '.')))
				e = s + strlen(s);
			if (RECTYPE(key->disc->data) == REC_variable && !REC_V_SIZE(key->disc->data))
				key->disc->data |= ((1<<15)-1);
			if (!(s = strdup(sfprints("%-*.*s%%%s%s", e - p, e - p, p, fmtrec(key->disc->data, 1), e))))
				error(ERROR_SYSTEM|3, "out of space");
			key->output = s;
			if (sp->verbose)
				error(0, "%s rename output %s => %s", error_info.id, p, s);
		}
	}
	if (sp->verbose)
		error(0, "%s %s record format", error_info.id, fmtrec(key->disc->data, 0));
	return error_info.errors != 0;
}

/*
 * dump keys to stderr
 */

static ssize_t
dumpkey(Rs_t* rs, unsigned char* dat, size_t datlen, unsigned char* key, size_t keylen, Rsdisc_t* disc)
{
	Sort_t*	sp = (Sort_t*)RSKEYDISC(disc);
	ssize_t	n;
	int	i;
	char	buf[2];

	if ((n = (*sp->defkeyf)(rs, dat, datlen, key, keylen, disc)) > 0)
	{
		buf[1] = 0;
		for (i = 0; i < n; i++)
		{
			buf[0] = key[i];
			sfputr(sp->tp, fmtesc(buf), -1);
		}
		sfprintf(sfstderr, "key: %s\n", sfstruse(sp->tp));
	}
	return n;
}

/*
 * initialize sp from argv
 */

static int
init(register Sort_t* sp, Rskeydisc_t* dp, char** argv)
{
	register Rskey_t*	key;
	register char*		s;
	register char**		p;
	char*			t;
	int			n;
	unsigned long		x;
	unsigned long		z;
	size_t			fixed;
	struct stat		is;
	struct stat		os;

	memset(sp, 0, sizeof(*sp));
	sp->xfiles = elementsof(sp->files);
	sfset(sfstdout, SF_SHARE, 0);
	sfset(sfstderr, SF_SHARE, 0);
	Vmdcsbrk->round = INBRK;
	dp->version = RSKEY_VERSION;
	dp->flags = 0;
	dp->errorf = errorf;
	if (!(sp->key = key = rskeyopen(dp, NiL)) || !(sp->rec = rsnew(key->disc)))
		return -1;
	z = key->insize = 2 * INMAX;
#if 0
	if (conformance(0, 0))
#endif
	key->type |= RS_DATA;
	if ((n = strtol(astconf("PAGESIZE", NiL, NiL), &t, 0)) > 0 && !*t)
		key->alignsize = n;
	if ((n = parse(sp, argv)) || rskeyinit(key))
	{
		rskeyclose(key);
		if (n < 0)
			error(ERROR_USAGE|4, "%s", optusage(NiL));
		return -1;
	}

	/*
	 * finalize the buffer dimensions
	 */

	if ((x = key->insize) != z)
		z = 0;
	if (x > INMAX)
		x = INMAX;
	else if (x < INMIN && !sp->chunk)
		x = INMIN;
	if (sp->single = !key->input[1])
	{
		if (!(sp->opened = fileopen(sp, key->input[0])))
			error(ERROR_SYSTEM|3, "%s: cannot open", key->input[0]);
		if (fstat(sffileno(sp->opened), &is))
			error(ERROR_SYSTEM|3, "%s: cannot stat", key->input[0]);
		if (!S_ISREG(is.st_mode) || sp->opened->disc) /* XXX: need sfio call to test if any disc pushed */
		{
			sp->total = 0;
			sp->test |= TEST_read;
		}
		else if (x > (sp->total = is.st_size))
			x = sp->total;
	}
	else
		sp->test |= TEST_read;
	if (sp->zip & SF_READ)
		sp->test |= TEST_read;
	fixed = key->fixed;
	if ((sp->test & TEST_reserve) || !(sp->test & TEST_read))
	{
		sp->map = 1;
		if (z)
			x = key->procsize;
		if (fixed)
			x += fixed - x % fixed;
	}
	else
	{
		if (z)
			x = key->procsize;
		for (;;)
		{
			if (fixed)
				x += fixed - x % fixed;
			if (sp->buf = (char*)vmalign(Vmheap, x, key->alignsize))
				break;
			if ((x >>= 1) < INMIN)
				error(ERROR_SYSTEM|3, "out of space");
		}
		sp->hit = x - key->alignsize;
	}
	if (sp->test & TEST_keys)
	{
		if (!key->disc->defkeyf)
			error(2, "no key function to intercept");
		else if (!(sp->tp = sfstropen()))
			error(ERROR_SYSTEM|3, "out of space");
		else
		{
			sp->defkeyf = key->disc->defkeyf;
			key->disc->defkeyf = dumpkey;
		}
	}
	if (key->nproc > 1)
	{
		off_t		offset;
		off_t		total;
		off_t		size;
		size_t		chunk;
		int		i;
		Job_t*		jp;

		if (!sp->map || pathstdin(key->input[0]))
		{
	uno:
			key->nproc = 1;
		}
		else if ((n = (sp->total + key->procsize - 1) / (key->procsize)) <= 1)
			goto uno;
		else
		{
			if (n < key->nproc)
				key->nproc = n;
			else
				n = key->nproc;
			if (!(sp->jobs = vmnewof(Vmheap, 0, Job_t, n, 0)))
				goto uno;
			size = (sp->total + n - 1) / n;
			if (fixed)
			{
				if (size % fixed)
					size += fixed - size % fixed;
				i = (size + x - 1) / x;
				if (i * n > sp->xfiles)
				{
					error(1, "multi-process multi-stage not implemented; falling back to one processor");
					goto uno;
				}
				chunk = size / i;
				if (chunk % fixed)
					chunk += fixed - chunk % fixed;
				size = chunk * i;
				offset = 0;
				total = sp->total;
				for (jp = sp->jobs; jp < sp->jobs + n; jp++)
				{
					jp->offset = offset;
					if (size > total)
						size = total;
					total -= (jp->size = size);
					jp->chunk = chunk;
					jp->intermediates = i;
					offset += size;
				}
				if (key->procsize > chunk)
					key->procsize = chunk;
				else
				{
					size = key->procsize;
					i = (chunk + size - 1) / size;
					size = chunk / i;
					if (size % fixed)
						size += fixed - size % fixed;
					key->procsize = size;
				}
			}
			else
			{
				register char*	s;
				register char*	t;
				register char*	b;
				off_t		ideal;
				off_t		scan;
				char*		file;
				Sfio_t*		ip;

				i = (size + x - 1) / x;
				if (i * n > sp->xfiles)
				{
					error(1, "multi-process multi-stage not implemented; falling back to one processor");
					goto uno;
				}
				chunk = (size + i - 1) / i;
				size = ideal = chunk * i;
				if ((scan = INREC) >= ideal)
					scan = (ideal / 32) * 4;
				offset = 0;
				total = sp->total;
				file = key->input[0];
				if (!(ip = fileopen(sp, file)))
					error(ERROR_SYSTEM|3, "%s: cannot read", file);
				for (jp = sp->jobs; jp < sp->jobs + n; jp++)
				{
					jp->offset = offset;
					if (((size = ideal) + scan) >= total)
						size = total;
					else
					{
						/*UNDENT...*/

	/*
	 * snoop around for the closest record boundary
	 */

	size -= scan / 2;
	if (sfseek(ip, offset + size, SEEK_SET) != (offset + size))
		error(ERROR_SYSTEM|3, "%s: record boundary seek error at offset %lld", file, (Sflong_t)offset + size);
	if (!(b = (char*)sfreserve(ip, scan, 0)))
		error(ERROR_SYSTEM|3, "%s: record boundary read error at offset %lld", file, (Sflong_t)offset + size);
	s = t = b + scan / 2 - 1;
	while (*s++ != '\n')
	{
		if (t < b)
		{
		bigger:
			if (((size += scan) + offset) >= (total - scan))
				error(3, "%s: monster record at offset %lld", (Sflong_t)offset);
			if (sfseek(ip, offset + size, SEEK_SET) != (offset + size))
				error(ERROR_SYSTEM|3, "%s: record boundary input seek error at %lld", file, (Sflong_t)offset + size);
			if (!(b = (char*)sfreserve(ip, scan, 0)))
				error(ERROR_SYSTEM|3, "%s: record boundary read error at %lld", file, (Sflong_t)offset + size);
			t = (s = b) + scan;
			do
			{
				if (s >= t)
					goto bigger;
			} while (*s++ != '\n');
			break;
		}
		if (*t-- == '\n')
		{
			s = t + 2;
			break;
		}
	}
	size += (s - b);

						/*...INDENT*/
					}
					total -= (jp->size = size);
					jp->chunk = (size + i - 1) / i;
					if (jp->chunk > chunk)
						chunk = jp->chunk;
					jp->intermediates = i;
					offset += size;
				}
				if (rsfileclose(sp->rec, ip))
					return -1;
				sfclose(ip);
				key->procsize = (key->procsize > chunk) ? chunk : chunk / ((chunk + key->procsize - 1) / key->procsize);
			}
		}
	}
	key->insize = sp->end = x;

	/*
	 * check the output file for clash with the input files
	 */

	n = stat("/dev/null", &is);
	if (pathstdout(key->output))
	{
		key->output = "/dev/stdout";
		sp->op = sfstdout;
		if (!n && !fstat(sffileno(sp->op), &os) && os.st_dev == is.st_dev && os.st_ino == is.st_ino)
			key->type |= RS_IGNORE;
	}
	else if (key->input)
	{
		if (!stat(key->output, &os))
		{
			if (!n && os.st_dev == is.st_dev && os.st_ino == is.st_ino)
				key->type |= RS_IGNORE;
			else if (eaccess(key->output, W_OK))
				error(ERROR_SYSTEM|3, "%s: cannot write", key->output);
			else if (!fs3d(FS3D_TEST) || !iview(&os))
			{
				p = key->input;
				while (s = *p++)
					if (!pathstdin(s))
					{
						if (stat(s, &is))
							error(ERROR_SYSTEM|2, "%s: not found", s);
						else if (os.st_dev == is.st_dev && os.st_ino == is.st_ino)
						{
							if (t = strrchr(key->output, '/'))
							{
								s = key->output;
								*t = 0;
							}
							else
								s = ".";
							if (sp->overwrite = pathtemp(NiL, 0, s, error_info.id, &n))
								sp->op = sfnew(NiL, NiL, SF_UNBOUND, n, SF_WRITE);
							if (t)
								*t = '/';
							if (!sp->op || fstat(n, &is))
								error(ERROR_SYSTEM|3, "%s: cannot create overwrite file %s", key->output, sp->overwrite);
							if (os.st_uid != is.st_uid || os.st_gid != is.st_gid)
								sp->preserve = 1;
							break;
						}
					}
			}
		}
		if (!sp->overwrite && !(sp->op = sfopen(NiL, key->output, "w")))
			error(ERROR_SYSTEM|3, "%s: cannot write", key->output);
	}
	if (rsfilewrite(sp->rec, sp->op, key->output))
		return -1;
	if (key->outsize > 0)
		sfsetbuf(sp->op, NiL, key->outsize);
	if (sp->zip & SF_WRITE)
		sfdcgzip(sp->op, 0);

	/*
	 * finally ready for recsort now
	 */

	if (rsinit(sp->rec, key->meth, key->procsize, key->type, key))
	{
		error(ERROR_SYSTEM|2, "sort library initialization error");
		rskeyclose(key);
		return -1;
	}
	if (sp->rec->meth->type == RS_MTCOPY)
		sp->chunk = 1;
	if (sp->test & TEST_io)
	{
		for (n = 0; s = key->input[n]; n++)
			error(0, "%s input[%d]\t\"%s\"", error_info.id, n, s);
		if (s = key->output)
			error(0, "%s output\t\"%s\"", error_info.id, s);
	}
	return 0;
}

/*
 * close sp->files and push fp if not 0
 */

static void
clear(register Sort_t* sp, Sfio_t* fp)
{
	register int	i;

	for (i = fp ? sp->mfiles : 0; i < sp->nfiles; i++)
	{
		rstempclose(sp->rec, sp->files[i]);
		sp->files[i] = 0;
	}
	if (fp)
	{
		sp->files[sp->mfiles++] = fp;
		sp->nfiles = sp->mfiles;
		if (sp->mfiles >= (sp->xfiles - 1))
			sp->mfiles = 0;
	}
	else
		sp->nfiles = sp->mfiles = 0;
}

/*
 * flush the intermediate data
 * r is the partial record offset
 * updated r is returned
 */

static ssize_t
flush(register Sort_t* sp, register size_t r)
{
	register Sfio_t*	fp;
	register size_t		n;
	register size_t		m;
	register size_t		b;

	if (sp->chunk)
	{
		/*
		 * skip merge and output sorted chunk
		 */

		if (rswrite(sp->rec, sp->op, RS_OTEXT))
		{
			if (!error_info.errors)
				error(ERROR_SYSTEM|2, "%s: write error", sp->key->output);
			return -1;
		}
	}
	else if (sp->rec->meth->type != RS_MTVERIFY)
	{
		/*
		 * write to an intermediate file and rewind for rsmerge
		 */

		if (!(fp = sp->files[sp->nfiles]))
		{
			if (sp->child || !(fp = rstempwrite(sp->rec, (Sfio_t*)0)))
				error(ERROR_SYSTEM|3, "cannot create intermediate sort file %d", sp->nfiles);
			sp->files[sp->nfiles] = fp;
		}
		sp->nfiles++;
		if (sp->verbose)
			error(0, "%s write intermediate", error_info.id);
		if (rswrite(sp->rec, fp, 0))
		{
			error(ERROR_SYSTEM|2, "intermediate sort file write error");
			return -1;
		}
		if (rstempread(sp->rec, fp))
		{
			error(ERROR_SYSTEM|2, "intermediate sort file rewind error");
			return -1;
		}

		/*
		 * multi-stage merge when open file limit exceeded
		 */

		if (sp->nfiles >= sp->xfiles)
		{
			if (sp->child || !(fp = rstempwrite(sp->rec, (Sfio_t*)0)))
				error(ERROR_SYSTEM|3, "cannot create intermediate merge file");
			if (sp->verbose)
				error(0, "%s merge multi-stage intermediate", error_info.id);
			if (rsmerge(sp->rec, fp, sp->files + sp->mfiles, sp->nfiles - sp->mfiles, 0))
			{
				error(ERROR_SYSTEM|2, "intermediate merge file write error");
				return -1;
			}
			if (rstempread(sp->rec, fp))
			{
				error(ERROR_SYSTEM|2, "intermediate merge file rewind error");
				return -1;
			}
			clear(sp, fp);
		}
	}

	/*
	 * slide over partial record data so the next read is aligned
	 */

	if (!sp->map && (m = sp->cur - r))
	{
		n = roundof(m, sp->key->alignsize) - m;
		if (n < r)
		{
			m = n;
			while (r < sp->cur)
				sp->buf[n++] = sp->buf[r++];
			sp->cur = n;
		}
		else
		{
			b = r;
			r += m;
			n += m;
			sp->cur = n;
			while (r > b)
				sp->buf[--n] = sp->buf[--r];
			m = n;
		}
	}
	else
		m = sp->cur = 0;
	return m;
}

/*
 * input the records for file ip
 */

static int
input(register Sort_t* sp, Sfio_t* ip, const char* name, int last)
{
	register ssize_t	n;
	register ssize_t	p;
	register ssize_t	m;
	register ssize_t	r;
	size_t			z;
	Sfoff_t			w;
	char*			b;
	int			c;
	char			del[2];

	/*
	 * align the read buffer and
	 * loop on insize chunks
	 */

	error_info.file = ip == sfstdin ? (char*)0 : (char*)name;
	m = -1;
	z = 0;
	if (sp->bufsize)
		sfsetbuf(ip, NiL, sp->bufsize);
	else if (sfsize(ip) > SF_BUFSIZE)
	{
		if (sp->map)
			sfsetbuf(ip, NiL, z = sp->end);
		else
			sfsetbuf(ip, NiL, 0);
	}
	if (sp->map)
		w = sfsize(ip);
	r = sp->cur = roundof(sp->cur, sp->key->alignsize);
	p = 0;
	for (;;)
	{
		if (sp->cur > sp->hit)
		{
			if (sp->single && !sp->nfiles && sp->total == (sp->map ? 0 : p))
				break;
			if ((r = flush(sp, r)) < 0)
				return -1;
		}
		if (!sp->map)
		{
			n = sfeof(ip) ? 0 : sfread(ip, sp->buf + sp->cur, sp->end - sp->cur);
			if (last)
			{
				if ((c = sfgetc(ip)) == EOF)
					sp->rec->type |= RS_LAST;
				else
					sfungetc(ip, c);
			}
		}
		else
		{
			sp->buf = (char*)sfreserve(ip, m, SF_LOCKR);
			n = sfvalue(ip);
			if (!sp->buf)
			{
				if (m < 0 && n < -m && z == sp->end)
				{
					sfsetbuf(ip, NiL, z = 2 * sp->end);
					sp->buf = (char*)sfreserve(ip, m, SF_LOCKR);
					n = sfvalue(ip);
					if (sp->verbose && n)
						error(0, "%s buffer boundary expand to %I*d", error_info.id, sizeof(n), n);
				}
				if (!sp->buf && n > 0 && !(sp->buf = sfreserve(ip, n, SF_LOCKR)))
					n = -1;
			}
			if (last && w == (sftell(ip) + n))
				sp->rec->type |= RS_LAST;
			if (sp->verbose)
				error(0, "%s reserve %*d => %*d", error_info.id, sizeof(m), m, sizeof(n), n);
		}
		if (n <= 0)
		{
			if (n < 0)
				error(ERROR_SYSTEM|3, "read error");
			if (sp->cur <= r)
				break;
			if (sp->key->fixed)
			{
				error(1, "incomplete record length=%lld", (Sflong_t)(sp->cur - r));
				break;
			}
			if (RECTYPE(sp->key->disc->data) == REC_delimited && sp->buf[sp->cur - 1] != (c = REC_D_DELIMITER(sp->key->disc->data)))
			{
				sp->buf[sp->cur++] = c;
				if (c == '\n')
					error(1, "newline appended");
				else
				{
					del[0] = c;
					del[1] = 0;
					error(1, "%s appended", fmtquote(del, "'", NiL, 1, 0));
				}
			}
		}
		sp->cur += n;
	process:
		if (sp->verbose && !sp->child)
			error(ERROR_PROMPT, "%s process %lld ->", error_info.id, (Sflong_t)(sp->cur - r));
		if ((p = rsprocess(sp->rec, sp->buf + r, sp->cur - r)) < 0)
			error(ERROR_SYSTEM|3, "sort error");
		if (sp->verbose)
		{
			if (sp->child)
				error(0, "%s process %lld -> %lld", error_info.id, (Sflong_t)(sp->cur - r), (Sflong_t)p);
			else
				error(0, " %lld", (Sflong_t)p);
		}
		if (sp->map)
		{
			if (sp->map > 2)
				break;
			sfread(ip, sp->buf, p);
			if (p)
			{
				m = -(n - p + 1);
				if (((sp->total -= p) / 3) < (sp->end / 2) && sp->total > sp->end)
				{
					if ((r = flush(sp, r)) < 0)
						return -1;
					sfsetbuf(ip, NiL, sp->total);
				}
			}
			else if (sp->map == 1)
			{
				sp->map++;
				m = -(n + 1);
			}
			else if (n > sp->end)
			{
				error(2, "monster record", n, sp->end, sp->cur, p);
				break;
			}
			else if (sp->key->fixed)
			{
				error(1, "incomplete record length=%ld", n - p);
				break;
			}
			else
			{
				sp->cur = n - p;
				if (!(b = vmnewof(Vmheap, 0, char, sp->cur, 1)))
					error(ERROR_SYSTEM|3, "out of space");
				memcpy(b, sp->buf + p, sp->cur);
				if (RECTYPE(sp->key->disc->data) == REC_delimited && b[sp->cur - 1] != (c = REC_D_DELIMITER(sp->key->disc->data)))
				{
					b[sp->cur++] = '\n';
					if (c == '\n')
						error(1, "newline appended");
					else
					{
						del[0] = c;
						del[1] = 0;
						error(1, "%s appended", fmtquote(del, "'", "'", 1, 0));
					}
				}
				sp->buf = b;
				sp->map++;
				goto process;
			}
		}
		else
			r += p;
	}
	error_info.file = 0;
	return 0;
}

/*
 * sfio part discipline read
 */

static ssize_t
partread(Sfio_t* fp, Void_t* buf, size_t size, Sfdisc_t* dp)
{
	register Part_t*	pp = (Part_t*)dp;

	if (pp->remain <= 0)
		return 0;
	if (size > pp->remain)
		size = pp->remain;
	pp->remain -= size;
	return sfrd(fp, buf, size, dp);
}

/*
 * sfio part discipline seek
 */

static Sfoff_t
partseek(Sfio_t* fp, Sfoff_t lloffset, int op, Sfdisc_t* dp)
{
	register Part_t*	pp = (Part_t*)dp;
	off_t			offset = lloffset;

	switch (op)
	{
	case SEEK_SET:
		offset += pp->offset;
		break;
	case SEEK_CUR:
		offset += pp->offset;
		break;
	case SEEK_END:
		offset = pp->offset + pp->size - offset;
		op = SEEK_SET;
		break;
	}
	if ((offset = sfsk(fp, offset, op, dp)) >= 0)
	{
		offset -= pp->offset;
		pp->remain = pp->size - offset;
	}
	return offset;
}

/*
 * job control
 * requires single named input file
 * no multi-stage merge
 */

static void
jobs(register Sort_t* sp)
{
	register Job_t*	jp;
	register Job_t*	xp;
	register int	i;
	register int	j;
	register int	f;
	int		status;
	char*		file;
	Sfio_t*		ip;
	Part_t		part;
	char		id[32];

	sp->single = 0;
	if (sp->verbose)
		error(0, "%s %d processes %lld total", error_info.id, sp->key->nproc, (Sflong_t)sp->total);
	xp = sp->jobs + sp->key->nproc;
	if (sp->test & TEST_show)
	{
		for (jp = sp->jobs; jp < xp; jp++)
			error(0, "%s#%d pos %12lld : len %10lld : buf %10lld : num %2d", error_info.id, jp - sp->jobs + 1, (Sflong_t)jp->offset, (Sflong_t)jp->size, (Sflong_t)jp->chunk, jp->intermediates);
		exit(0);
	}
	f = 0;
	for (jp = sp->jobs; jp < xp; jp++)
		for (i = 0; i < jp->intermediates; i++)
			if (!(sp->files[f++] = rstempwrite(sp->rec, (Sfio_t*)0)))
				error(ERROR_SYSTEM|3, "cannot create intermediate file %d", i);
	part.disc.readf = partread;
	part.disc.writef = 0;
	part.disc.seekf = partseek;
	part.disc.exceptf = 0;
	part.disc.disc = 0;
	file = sp->key->input[0];
	j = 0;
	for (jp = sp->jobs; jp < xp; jp++)
	{
		ip = fileopen(sp, file);
		switch (fork())
		{
		case -1:
			error(ERROR_SYSTEM|3, "not enough child processes");
		case 0:
			sp->child = 1;
			sfsprintf(id, sizeof(id), "%s#%d", error_info.id, jp - sp->jobs + 1);
			error_info.id = id;
			sp->end = jp->chunk;
			part.offset = jp->offset;
			sp->total = part.size = part.remain = jp->size;
			sfdisc(ip, &part.disc);
			for (i = 0; i < jp->intermediates; i++)
				sp->files[i] = sp->files[j++];
			while (i < f)
				sp->files[i++] = 0;
			if (sp->verbose)
				error(0, "%s pos %12lld : len %10lld : buf %10lld : num %2d", error_info.id, (Sflong_t)jp->offset, (Sflong_t)jp->size, (Sflong_t)jp->chunk, jp->intermediates);
			exit(input(sp, ip, file, 0) < 0);
		}
		if (rsfileclose(sp->rec, ip))
			exit(1);
		sfclose(ip);
		j += jp->intermediates;
	}
	sp->nfiles = f;
	i = 0;
	j = sp->key->nproc;
	while (j > 0)
	{
		if (wait(&status) != -1)
		{
			if (status)
				i++;
			j--;
		}
		else if (errno != EINTR)
		{
			error(ERROR_SYSTEM|3, "%d process%s did not complete", j, j == 1 ? "" : "es");
			break;
		}
	}
	if (i)
		error(3, "%d child process%s failed", i, i == 1 ? "" : "es");
}

/*
 * all done
 */

static void
done(register Sort_t* sp)
{
	while (rsdisc(sp->rec, NiL, RS_POP));
	if ((sfsync(sp->op) || sp->op != sfstdout && rsfileclose(sp->rec, sp->op)) && !error_info.errors)
		error(ERROR_SYSTEM|2, "%s: write error", sp->key->output);
	if (rsclose(sp->rec))
		error(2, "sort error");
	if (sp->map > 2)
		vmfree(Vmheap, sp->buf);

	/*
	 * if the output would have overwritten an input
	 * file now is the time to commit to it
	 */

	if (sp->overwrite)
	{
		if (error_info.errors)
			remove(sp->overwrite);
		else if (sp->preserve)
		{
			Sfio_t*	ip;
			Sfio_t*	op;

			if (ip = sfopen(NiL, sp->overwrite, "r"))
			{
				if (op = sfopen(NiL, sp->key->output, "w"))
				{
					if ((sfmove(ip, op, SF_UNBOUND, -1) < 0 || sfclose(op) || !sfeof(ip)) && !error_info.errors)
						error(ERROR_SYSTEM|2, "%s: write error", sp->key->output);
					sfclose(op);
				}
				else
					error(ERROR_SYSTEM|2, "%s: cannot write", sp->key->output);
				sfclose(ip);
				remove(sp->overwrite);
			}
			else
				error(ERROR_SYSTEM|2, "%s: cannot read", sp->overwrite);
			sp->preserve = 0;
		}
		else if (remove(sp->key->output) || rename(sp->overwrite, sp->key->output))
			error(ERROR_SYSTEM|2, "%s: cannot overwrite", sp->key->output);
		free(sp->overwrite);
		sp->overwrite = 0;
	}
	if (rskeyclose(sp->key))
		error(2, "sort key error");
}

int
main(int argc, char** argv)
{
	register char*		s;
	register Sfio_t*	fp;
	char*			last;
	char**			merge;
	int			i;
	struct stat		st;
	Sort_t			sort;

	error_info.id = "sort";
	if (init(&sort, &sort.disc, argv))
		exit(1);
	if (sort.test & TEST_dump)
	{
		sfprintf(sfstderr, "main\n\tintermediates=%d\n", sort.xfiles);
		rskeydump(sort.key, sfstderr);
	}
	if (sort.key->type & RS_CAT)
	{
		while (s = *sort.key->input++)
		{
			fp = fileopen(&sort, s);
			if (sfmove(fp, sfstdout, SF_UNBOUND, -1) < 0 || !sfeof(fp))
				error(ERROR_SYSTEM|2, "%s: read error", s);
			if (sferror(sfstdout) || rsfileclose(sort.rec, fp))
				break;
		}
	}
	else
	{
		merge = sort.key->merge && sort.key->input[0] && sort.key->input[1] ? sort.key->input : (char**)0;
		fp = 0;
		if (sort.jobs)
			jobs(&sort);
		else if (sort.test & TEST_show)
			exit(0);
		else
		{
			last = 0;
			if (!merge && (sort.rec->type & RS_MORE))
				for (i = 0; s = sort.key->input[i]; i++)
					if ((pathstdin(s) && !fstat(0, &st) || !stat(s, &st)) && st.st_size)
						last = s;
			while (s = *sort.key->input++)
			{
				fp = fileopen(&sort, s);
				if (merge)
				{
					if (sort.nfiles >= sort.xfiles)
					{
						clear(&sort, NiL);
						sort.key->input = merge;
						merge = 0;
						if (fp != sfstdin)
							sfclose(fp);
						fp = 0;
						continue;
					}
					sort.files[sort.nfiles++] = fp;
				}
				else if (input(&sort, fp, s, s == last) < 0)
					break;
				else if (fp != sfstdin && !sort.map)
				{
					sfclose(fp);
					fp = 0;
				}
			}
		}
		if (sort.nfiles)
		{
			if (sort.cur && flush(&sort, sort.cur) < 0)
				return 1;
			if (sort.verbose)
				error(0, "%s merge text", error_info.id);
			if (rsmerge(sort.rec, sort.op, sort.files, sort.nfiles, merge ? RS_TEXT : RS_OTEXT))
				error(ERROR_SYSTEM|3, "merge error");
			clear(&sort, NiL);
		}
		else
		{
			if (sort.verbose)
				error(0, "%s write text", error_info.id);
			if (rswrite(sort.rec, sort.op, RS_OTEXT) && !error_info.errors)
				error(ERROR_SYSTEM|3, "%s: write error", sort.key->output);
			if (fp && fp != sfstdin)
				sfclose(fp);
		}
	}
	done(&sort);
	exit(error_info.errors != 0);
}
