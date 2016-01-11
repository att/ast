/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2012 AT&T Intellectual Property          *
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
 * file -- determine file type
 *
 * the sum of the hacks {s5,v10,planix} is _____ than the parts
 */

static const char usage[] =
"[-?\n@(#)$Id: file (AT&T Research) 2011-08-01 $\n]"
USAGE_LICENSE
"[+NAME?file - determine file type]"
"[+DESCRIPTION?\bfile\b tests and attempts to classify each \afile\a argument."
"	Non-regular files are classified by their \bstat\b(2) types. Empty and"
"	non-readable regular files are classified as such. Otherwise a data"
"	block is read from \afile\a and this is used to match against the"
"	\amagic\a file(s) (see \bMAGIC FILE\b below). Files with less than 1024"
"	bytes of data are labelled \bsmall\b to note that the sample may"
"	be too small for an accurate classification. Failing a content match,"
"	the file name extension may be used to classify. As a last resort"
"	statistical sampling is done for a small range of languages and"
"	applications. Failed matches usually result in the less informative"
"	\bascii text\b or \bbinary data\b.]"

"[a:all?List all magic table matches.]"
"[b:brief|no-filename?Suppress the output line file name prefix.]"
"[c:mime?List the \bmime\b(1) classification for each \afile\a. Although the"
"	default descriptions are fairly consistent, use \b--mime\b for"
"	precise classification matching.]"
"[d:default-magic?Equivalent to \b--magic=-\b.]"
"[f:files|file-list?\afile\a contains list of file names, one per line, that"
"	are classified.]:[file]"
"[i:ignore-magic?Equivalent to \b--magic=/dev/null\b.]"
"[l:list?The loaded \amagic\a files are listed and then \bfile\b exits.]"
"[M:magic?\afile\a is loaded as a \amagic\a file. More than one \b--magic\b"
"	option may be specified; the precedence is from left to right. The"
"	first \b--magic\b option causes the default system \amagic\a file to"
"	be ignored; the file \b-\b may then be specified to explicitly"
"	load the default system \amagic\a file. To ignore all magic files"
"	specify the file \b/dev/null\b and no others.]:[file]"
"[m:append-magic?\afile\a is loaded as a \amagic\a file. Equivalent to the"
"	\b--magic\b option, except that the default system \amagic\a file is"
"	still loaded last. If \b--magic\b is also specified then the default"
"	system \amagic\a is only loaded if explicity specified.]:[file]"
"[p:pattern|match?Only files with descriptions matching the \bsh\b(1)"
"	match \apattern\a are listed. \bfile\b exits with status 0 if any"
"	files match, 0 otherwise.]:[pattern]"
"[q:quiet|silent?Do not list matching \b--pattern\b files.]"
"[L:logical|dereference?Follow symbolic links.]"
"[P|h:physical?Don't follow symbolic links.]"
"[w:warn?Enable magic file parse warning messages.]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+MAGIC FILE?A \amagic\a file specifies file content and name match"
"	expressions, descriptions, and \bmime\b(1) classifications. Each line"
"	in the file consists of five \btab\b separated fields:]{"
"	[+[op]]offset?\aoffset\a determines tha data location for the content"
"		test. \b(@\b\aexpression\a\b)\b specifies an indirect offset,"
"		i.e., the offset is the numeric contents of the data"
"		location at \aexpression\a. The default indirect numeric size"
"		is 4 bytes; a \bB\b suffix denotes 1 byte, \bH\b denotes 2"
"		bytes, and \bQ\b denotes 8 bytes. \aoffset\a may also be one"
"		of { \batime\b \bblocks\b \bctime\b \bfstype\b \bgid\b"
"		\bmode\b \bmtime\b \bname\b \bnlink\b \bsize\b \buid\b } to"
"		access \bstat\b(2) information for the current file. The"
"		optional \aop\a specifies relationships with surrounding"
"		\amagic\a lines:]{"
"		[++?previous fields in block match, current optional]"
"		[+&?previous and current fields in block match]"
"		[+|?previous fields in block do not match, subsequent skipped]"
"		[+{?start nesting block]"
"		[+}?end nesting block]"
"		[+c{?function declaration and call (1 char names)]"
"		[+}?function return]"
"		[+c()?function call]"
"	}"
"	[+type?The content data type:]{"
"		[+byte?1 byte integer]"
"		[+short?2 byte integer]"
"		[+long?4 byte integer]"
"		[+quad?8 byte integer]"
"		[+date?4 byte time_t]"
"		[+version?4 byte unsigned integer of the form \aYYYYMMDD\a"
"			for \aYYYY-MM-DD\a, 0x\aYYZZ\a for \aYY.ZZ\a, or"
"			0x\aWWXXYYZZ\a for \aWW.XX.YY.ZZ\a]"
"		[+edit?substitute operator for string data:"
"			%\aold\a%\anew\a%[glu]], where \b%\b is any delimiter]"
"		[+match?case insensitive \bsh\b(1) match pattern operator"
"			for string data]"
"	}"
"	[+[mask]]operator?\amask\a is an optional \b&\b\anumber\a that is"
"		masked (bit \band\b) with the content data before"
"		comparison. \aoperator\a is one of { \b< <= > >= != ==\b }."
"		Numeric values may be decimal, octal or hex.]"
"	[+description?The description text. Care was taken to maintain"
"		consistency between all descriptions, i.e., character case,"
"		grammatical parts placement, and punctuation, making"
"		description pattern matches feasible. \adescription\a may"
"		contain one \bprintf\b(3) format specification for the"
"		current data value at \aoffset\a.]"
"	[+mime?The \bmime\b(1) type/subtype. This provides a standard"
"		and consistent matching key space.]"
"}"

"[+FILES]{"
"	[+lib/file/magic?Default magic file on \b$PATH\b.]"
"}"

"[+SEE ALSO?\bfind\b(1), \bls\b(1), \bmime\b(1), \btw\b(1)]"
;

#include <ast.h>
#include <magic.h>
#include <ctype.h>
#include <error.h>

#define MAGIC_BRIEF	(MAGIC_USER<<0)
#define MAGIC_LIST	(MAGIC_USER<<1)
#define MAGIC_LOAD	(MAGIC_USER<<2)
#define MAGIC_PHYSICAL	(MAGIC_USER<<3)
#define MAGIC_SILENT	(MAGIC_USER<<4)

static int
type(Magic_t* mp, char* file, const char* pattern, register Magicdisc_t* disc)
{
	char*		s;
	char*		e;
	Sfio_t*		fp;
	struct stat*	sp;
	struct stat	st;

	sp = ((disc->flags & MAGIC_PHYSICAL) ? lstat(file, &st) : stat(file, &st)) ? (struct stat*)0 : &st;
	fp = (sp && S_ISREG(sp->st_mode)) ? sfopen(NiL, file, "r") : (Sfio_t*)0;
	s = magictype(mp, fp, file, sp);
	if (fp)
		sfclose(fp);
	e = pathcanon(file, 0, 0);
	if (!pattern)
	{
		if (!(disc->flags & MAGIC_BRIEF))
			sfprintf(sfstdout, "%s:\t%s", file, e - file > 6 ? "" : "\t");
		sfprintf(sfstdout, "%s\n", s);
		return 1;
	}
	else if (strmatch(s, pattern))
	{
		if (!(disc->flags & MAGIC_SILENT))
			sfprintf(sfstdout, "%s\n", file);
		return 1;
	}
	return 0;
}

int
main(int argc, register char** argv)
{
	register Magic_t*	mp;
	register char*		p;
	char*			pattern = 0;
	Sfio_t*			list = 0;
	int			hit;
	Magicdisc_t		disc;

	NoP(argc);
	error_info.id = "file";
	disc.version = MAGIC_VERSION;
	disc.flags = 0;
	disc.errorf = errorf;
	if (!(mp = magicopen(&disc)))
		error(3, "out of space");
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			disc.flags |= MAGIC_ALL;
			continue;
		case 'b':
			disc.flags |= MAGIC_BRIEF;
			continue;
		case 'c':
			disc.flags |= MAGIC_MIME;
			continue;
		case 'd':
			if (magicload(mp, NiL, 0))
				error(3, "cannot load default magic file");
			disc.flags |= MAGIC_LOAD;
			continue;
		case 'f':
			if (streq(opt_info.arg, "-") || streq(opt_info.arg, "/dev/stdin") || streq(opt_info.arg, "/dev/fd/0"))
				list = sfstdin;
			else if (!(list = sfopen(NiL, opt_info.arg, "r")))
				error(3, "cannot open %s", opt_info.arg);
			continue;
		case 'i':
			disc.flags |= MAGIC_LOAD;
			continue;
		case 'l':
			disc.flags |= MAGIC_LIST|MAGIC_VERBOSE;
			continue;
		case 'L':
			disc.flags &= ~MAGIC_PHYSICAL;
			continue;
		case 'm':
			if (magicload(mp, opt_info.arg, 0))
				error(3, "%s: cannot load magic file", opt_info.arg);
			continue;
		case 'M':
			if (magicload(mp, opt_info.arg, 0))
				error(3, "%s: cannot load magic file", opt_info.arg);
			disc.flags |= MAGIC_LOAD;
			continue;
		case 'p':
			pattern = opt_info.arg;
			continue;
		case 'P':
		case 'h':
			disc.flags |= MAGIC_PHYSICAL;
			continue;
		case 'q':
			disc.flags |= MAGIC_SILENT;
			continue;
		case 'w':
			disc.flags |= MAGIC_VERBOSE;
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
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	argv += opt_info.index;
	if (!(disc.flags & MAGIC_LOAD) && magicload(mp, NiL, 0))
		error(3, "$%s,%s: cannot load default magic file", MAGIC_FILE_ENV, MAGIC_FILE);
	if (disc.flags & MAGIC_LIST)
	{
		magiclist(mp, sfstdout);
		hit = 1;
	}
	else
	{
		hit = 0;
		if (!list && !*argv)
			list = sfstdin;
		if (list)
			while (p = sfgetr(list, '\n', 1))
				if (*p)
					hit |= type(mp, p, pattern, &disc);
		while (p = *argv++)
			if (*p)
				hit |= type(mp, p, pattern, &disc);
	}
	return !hit;
}
