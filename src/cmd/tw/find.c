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
 * David Korn
 * Glenn Fowler
 * AT&T Research
 *
 * rewrite of find program to use fts*() and optget()
 * this implementation should have all your favorite
 * find options plus these extensions:
 * 
 *	-fast pattern
 *		traverses the fast find database (updatedb(1))
 *		under the dirs specified on the command line
 *		for paths that contain pattern; all other
 *		expression operators apply to matching paths
 *	-magic pattern
 *		matches pattern agains the file(1) magic description
 *	-mime type/subtype
 *		matches type/subtype against the file mime type
 *	-ignorecase
 *		case ingnored in path pattern matching
 *	-xargs command ... \;
 *		like -exec except that command will be invoked
 *		with as many file arguments as the system
 *		allows per command
 *	-test now
 *		set the current time to now for testing
 */

static const char usage1[] =
"[-1p1?@(#)$Id: find (AT&T Research) 2013-08-02 $\n]"
USAGE_LICENSE
"[+NAME?find - find files]"
"[+DESCRIPTION?\bfind\b recursively descends the directory hierarchy for each"
"	\apath\a and applies an \aoption\a expression to each file in the"
"	hierarchy. \b-print\b is implied if there is no action that"
"	generates output. The expression starts with the first argument"
"	that matches [(-!]]. Option expressions may occur before and/or"
"	after the \apath\a arguments. For numeric arguments \an\a, \a+n\a"
"	means \a>n\a, \a-n\a means \a<n\a, and \an\a means exactly \an\a.]"
;

static const char usage2[] =
"\n"
"[ path ... ] [ options ]\n"
"\n"
"[+EXIT STATUS?If no commands were executed (\b-exec\b, \b-xargs\b) the exit"
"	status is 1 if errors were detected in the directory traversal and"
"	0 if no errors were ecountered. Otherwise the status is:]{"
"	[+0?All \acommand\a executions returned exit status 0.]"
"	[+1-125?A command line meeting the specified requirements could not"
"		be assembled, one or more of the invocations of \acommand\a"
"		returned  non-0 exit status, or some other error occurred.]"
"	[+126?\acommand\a was found but could not be executed.]"
"	[+127?\acommand\a was not found.]"
"}"
"[+ENVIRONMENT]{"
"	[+FINDCODES?Path name of the \blocate\b(1) database.]"
"	[+LOCATE_PATH?Alternate path name of \blocate\b(1) database.]"
"}"
"[+FILES]{"
"	[+lib/find/codes?Default \blocate\b(1) database.]"
"}"
"[+NOTES?In order to access the \bslocate\b(1) database the \bfind\b executable"
"	must be setgid to the \bslocate\b group.]"
"[+SEE ALSO?\bcpio\b(1), \bfile\b(1), \blocate\b(1), \bls\b(1), \bsh\b(1),"
"	\bslocate\b(1), \btest\b(1), \btw\b(1), \bupdatedb\b(1),"
"	\bxargs\b(1), \bstat\b(2)]"
;

#include <ast.h>
#include <ls.h>
#include <modex.h>
#include <find.h>
#include <fts.h>
#include <dirent.h>
#include <error.h>
#include <proc.h>
#include <tm.h>
#include <ctype.h>
#include <magic.h>
#include <mime.h>
#include <regex.h>
#include <vmalloc.h>

#include "cmdarg.h"

#define ignorecase	fts_number

#define PATH(f)		((f)->fts_path)

#define DAY		(unsigned long)(24*60*60)

/*
 * this is the list of operations
 * retain the order
 */

#undef	CMIN
#undef	CTIME
#undef	NOGROUP
#undef	NOUSER

enum Command
{
	CFALSE, CTRUE,

	PRINT, PRINT0, PRINTF, PRINTX, FPRINT, FPRINT0, FPRINTF, FPRINTX,
	LS, FLS,
	ATIME, AMIN, ANEWER, CTIME, CMIN, CNEWER, MTIME, MMIN, NEWER,
	POST, LOCAL, XDEV, PHYS,
	NAME, USER, GROUP, INUM, SIZE, LINKS, PERM, EXEC, OK, CPIO, NCPIO,
	TYPE, AND, OR, NOT, COMMA, LPAREN, RPAREN, LOGIC, PRUNE,
	CHECK, SILENT, IGNORE, SORT, REVERSE, FSTYPE, META,
	NOGROUP, NOUSER, FAST, ICASE, MAGIC, MIME, XARGS,
	DAYSTART, MAXDEPTH, MINDEPTH, NOLEAF, EMPTY,
	ILNAME, INAME, IPATH,
	IREGEX, LNAME, PATH, REGEX, USED, XTYPE, CHOP,
	LEVEL, TEST, CODES, DELETE, SHOW
};

#define Unary		(1<<0)
#define Num		(1<<1)
#define Str		(1<<2)
#define Exec		(1<<3)
#define Op		(1<<4)
#define File		(1<<5)
#define Re		(1<<6)
#define Stat		(1<<7)
#define Unit		(1<<8)

typedef int (*Sort_f)(FTSENT* const*, FTSENT* const*);

struct Node_s;
typedef struct Node_s Node_t;

struct State_s;
typedef struct State_s State_t;

typedef struct Args_s
{
	const char*	name;
	enum Command	action;
	int		type;
	int		primary;
	const char*	arg;
	const char*	values;
	const char*	help;
} Args_t;

typedef union Value_u
{
	char**		p;
	char*		s;
	unsigned long	u;
	long		l;
	int		i;
	short		h;
	char		c;
} Value_t;

typedef struct Fmt_s
{
	Sffmt_t		fmt;
	State_t*	state;
	FTSENT*		ent;
	char		tmp[PATH_MAX];
} Fmt_t;

typedef union Item_u
{
	Node_t*		np;
	char*		cp;
	Cmdarg_t*	xp;
	regex_t*	re;
	Sfio_t*		fp;
	unsigned long	u;
	long		l;
	int		i;
} Item_t;

struct Node_s
{
	Node_t*		next;
	const char*	name;
	enum Command	action;
	const Args_t*	op;
	Item_t		first;
	Item_t		second;
	Item_t		third;
};

struct State_s
{
	unsigned int	minlevel;
	unsigned int	maxlevel;
	int		walkflags;
	char		buf[LS_W_LONG+LS_W_INUMBER+LS_W_BLOCKS+2*PATH_MAX+1];
	char		txt[PATH_MAX+1];
	char*		usage;
	Node_t*		cmd;
	Proc_t*		proc;
	Node_t*		topnode;
	Node_t*		lastnode;
	Node_t*		nextnode;
	unsigned long	now;
	unsigned long	day;
	Sfio_t*		output;
	char*		codes;
	char*		fast;
	int		icase;
	int		primary;
	int		reverse;
	int		show;
	int		silent;
	enum Command	sortkey;
	Magic_t*	magic;
	Magicdisc_t	magicdisc;
	regdisc_t	redisc;
	Fmt_t		fmt;
	Sfio_t*		str;
	Sfio_t*		tmp;
	Vmalloc_t*	vm;
};

static const char* const	cpio[] = { "cpio", "-o", 0 };
static const char* const	ncpio[] = { "cpio", "-ocB", 0 };

/*
 * Except for pathnames, these are the only legal arguments
 */

static const Args_t	commands[] =
{
"begin",	LPAREN,		Unary,		0,	0,	0,
	"Equivalent to \\(. Begin nested expression.",
"end",		RPAREN,		Unary,		0,	0,	0,
	"Equivalent to \\). End nested expression.",
"a|and",	AND,		Op,		0,	0,	0,
	"Equivalent to `\\&'. \aexpr1\a \b-and\b \aexpr2\a:"
	" Both \aexpr1\a and \aexpr2\a must evaluate \btrue\b."
	" This is the default operator for two expression in sequence.",
"amin",		AMIN,		Num|Stat,	0,	"minutes",	0,
	"File was last accessed \aminutes\a minutes ago.",
"anewer",	ANEWER,		Str|Stat,	0,	"file",	0,
	"File was last accessed more recently than \afile\a was modified.",
"atime",	ATIME,		Num|Stat,	0,	"days",	0,
	"File was last accessed \adays\a days ago.",
"check",	CHECK,		Unary,		0,	0,	0,
	"Turns off \b-silent\b; enables inaccessible file and directory"
	" warnings. This is the default.",
"chop",		CHOP,		Unary,		0,	0,	0,
	"Chop leading \b./\b from printed pathnames.",
"cmin",		CMIN,		Num|Stat,	0,	"minutes",	0,
	"File status changed \aminutes\a minutes ago.",
"cnewer",	CNEWER,		Str|Stat,	0,	"file",	0,
	"File status changed more recently than \afile\a was modified.",
"codes",	CODES,		Str,		0,	"path",	0,
	"Sets the \bfind\b or \blocate\b(1) database \apath\a."
	" See \bupdatedb\b(1) for a description of this database.",
"comma",	COMMA,		Op,		0,	0,	0,
	"Equivalent to `,'. Joins two expressions; the status of the first"
	" is ignored.",
"cpio",		CPIO,		File|Stat,	1,	"archive",	0,
	"File is written as a binary format \bcpio\b(1) file entry.",
"ctime",	CTIME,		Num|Stat,	0,	"days",	0,
	"File status changed \adays\a days ago.",
"daystart",	AMIN,		Unary|Stat,	0,	0,	0,
	"Measure times (-amin -atime -cmin -ctime -mmin -mtime) from the"
	" beginning of today. The default is 24 hours ago.",
"delete",	DELETE,		Unary|Stat,	1,	0,	0,
	"Delete the file. If deletion fails a message is written to the"
	" standard error, \bfind\b continues, but its exit staus will be"
	" non-zero. Implies \b-depth\b. Warning: '\bfind -delete\b' is"
	" equivalent to '\brm -r .\b'; use \b-show\b to debug.",
"depth",	POST,		Unary,		0,	0,	0,
	"Process directory contents before the directory itself.",
"empty",	EMPTY,		Unary|Stat,	0,	0,	0,
	"A directory with size 0 or with no entries other than . or .., or a"
	" regular file with size 0.",
"exec",		EXEC,		Exec,		1,	"command ... ; | command ... {} +", 0,
	"Execute \acommand ...\a; true if 0 exit status is returned."
	" Arguments up to \b;\b are taken as arguments to \acommand\a."
	" The string `{}' in \acommand ...\a is globally replaced by "
	" the current filename. The command is executed in the directory"
	" from which \bfind\b was executed. The second form gathers"
	" pathnames until \bARG_MAX\b is reached, replaces {} preceding"
	" \b+\b with the list of pathnames, one pathname per argument,"
	" and executes \acommand\a ... \apathname\a ..., possibly multiple"
	" times, until all pathnames are exhausted.",
"false",	CFALSE,		Unary,		0,	0,	0,
	"Always false.",
"fast",		FAST,		Str,		0,	"pattern",	0,
	"Searches the \bfind\b or \blocate\b(1) database for paths"
	" matching the \bksh\b(1) \apattern\a. See \bupdatedb\b(1) for"
	" details on this database. The command line arguments limit"
	" the search and the expression, but all depth options are ignored."
	" The remainder of the expression is applied to the matching paths.",
"fls",		FLS,		File|Stat,	1,	"file",	0,
	"Like -ls except the output is written to \afile\a.",
"fprint",	FPRINT,		File|Stat,	1,	"file",	0,
	"Like -print except the output is written to \afile\a.",
"fprint0",	FPRINT0,	File|Stat,	1,	"file",	0,
	"Like -print0 except the output is written to \afile\a.",
"fprintf",	FPRINTF,	File|Stat,	1,	"file format",	0,
	"Like -printf except the output is written to \afile\a.",
"fprintx",	FPRINTX,	File|Stat,	1,	"file",	0,
	"Like -printx except the output is written to \afile\a.",
"fstype",	FSTYPE,		Str|Stat,	0,	"type",	0,
	"File is on a filesystem \atype\a. See \bdf\b(1) or"
	" \b-printf %F\b for local filesystem type names.",
"group|gid",	GROUP,		Str|Stat,	0,	"id",	0,
	"File group id name or number matches \aid\a.",
"ignorecase",	ICASE,		Unary,		0,	0,	0,
	"Ignore case in all pattern match expressions.",
"ilname",	ILNAME,		Str,		0,	"pattern",	0,
	"A case-insensitive version of \b-lname\b \apattern\a.",
"iname",	INAME,		Str,		0,	"pattern",	0,
	"A case-insensitive version of \b-name\b \apattern\a.",
"inum",		INUM,		Num|Stat,	0,	"number",	0,
	"File has inode number \anumber\a.",
"ipath",	IPATH,		Str,		0,	"pattern",	0,
	"A case-insensitive version of \b-path\b \apattern\a.",
"iregex",	IREGEX,		Re,		0,	"pattern",	0,
	"A case-insensitive version of \b-regex\b \apattern\a.",
"level",	LEVEL,		Num,		0,	"level",	0,
	"Current level (depth) is \alevel\a.",
"links",	LINKS,		Num|Stat,	0,	"number",	0,
	"File has \anumber\a links.",
"lname",	LNAME,		Str,		0,	"pattern",	0,
	"File is a symbolic link with text that matches \apattern\a.",
"local",	LOCAL,		Unary|Stat,	0,	0,	0,
	"File is on a local filesystem.",
"logical|follow|L",LOGIC,	Unary,		0,	0,	0,
	"Follow symbolic links.",
"ls",		LS,		Unary|Stat,	1,	0,	0,
	"List the current file in `ls -dils' format to the standard output.",
"magic",	MAGIC,		Str,		0,	"pattern",	0,
	"File magic number matches the \bfile\b(1) and \bmagic\b(3)"
	" description \apattern\a.",
"maxdepth",	MAXDEPTH,	Num,		0,	"level",	0,
	"Descend at most \alevel\a directory levels below the command"
	" line arguments. \b-maxdepth 0\b limits the search to the command"
	" line arguments.",
"metaphysical|H",META,		Unary,		0,	0,	0,
	"\b-logical\b for command line arguments, \b-physical\b otherwise.",
"mime",		MIME,		Str,		0,	"type/subtype",	0,
	"File mime type matches the pattern \atype/subtype\a.",
"mindepth",	MINDEPTH,	Num,		0,	"level",	0,
	"Do not apply tests or actions a levels less than \alevel\a."
	" \b-mindepth 1\b processes all but the command line arguments.",
"mmin",		MMIN,		Num|Stat,	0,	"minutes",	0,
	"File was modified \aminutes\a minutes ago.",
"mount|x|xdev|X",XDEV,		Unary|Stat,	0,	0,	0,
	"Do not descend into directories in different filesystems than"
	" their parents.",
"mtime",	MTIME,		Num|Stat,	0,	"days",	0,
	"File was modified \adays\a days ago.",
"name",		NAME,		Str,		0,	"pattern",	0,
	"File base name (no directory components) matches \apattern\a.",
"ncpio",	NCPIO,		File|Stat,	1,	"archive",	0,
	"File is written as a character format \bcpio\b(1) file entry.",
"newer",	NEWER,		Str|Stat,	0,	"file",	0,
	"File was modified more recently than \afile\a.",
"nogroup",	NOGROUP,	Unary|Stat,	0,	0,	0,
	"There is no group name matching the file group id.",
"noleaf",	NOLEAF,		Unary|Stat,	0,	0,	0,
	"Disable \b-physical\b leaf file \bstat\b(2) optimizations."
	" Only required on filesystems with . and .. as the first entries"
	" and link count not equal to 2 plus the number of subdirectories.",
"not",		NOT,		Op,		0,	0,	0,
	"\b-not\b \aexpr\a: inverts the truth value of \aexpr\a.",
"nouser",	NOUSER,		Unary|Stat,	0,	0,	0,
	"There is no user name matching the file user id.",
"ok",		OK,		Exec,		1,	"command ... \\;", 0,
	"Like \b-exec\b except a prompt is written to the terminal."
	" If the response does not match `[yY]].*' then the command"
	" is not run and false is returned.",
"o|or",		OR,		Op,		0,	0,	0,
	"Equivalent to `\\|'. \aexpr1\a \b-or\b \aexpr2\a:"
	" \aexpr2\a is not"
	" evaluated if \aexpr1\a is true.",
"path",		PATH,		Str,		0,	"pattern",	0,
	"File path name (with directory components) matches \apattern\a.",
"perm",		PERM,		Num|Stat,	0,	"mode",	0,
	"File permission bits tests; \amode\a may be octal or symbolic as"
	" in \bchmod\b(1). \amode\a: exactly \amode\a; \a-mode\a: all"
	" \amode\a bits are set; \a+mode\a: at least one of \amode\a"
	" bits are set.",
"physical|phys|P",PHYS,		Unary,		0,	0,	0,
	"Do not follow symbolic links. This is the default.",
"post",		POST,		Unary,		0,	0,	0,
	"Process directories before and and after the contents are processed.",
"print",	PRINT,		Unary,		1,	0,	0,
	"Print the path name (including directory components) to the"
	" standard output, followed by a newline.",
"print0",	PRINT0,		Unary,		1,	0,	0,
	"Like \b-print\b, except that the path is followed by a NUL character.",
"printf",	PRINTF,		Str|Stat,	1,	"format",
	"[+----- escape sequences -----?]"
		"[+\\a?alert]"
		"[+\\b?backspace]"
		"[+\\f?form feed]"
		"[+\\n?newline]"
		"[+\\t?horizontal tab]"
		"[+\\v?vertical tab]"
		"[+\\xnn?hexadecimal character \ann\a]"
		"[+\\nnn?octal character \annn\a]"
	"[+----- format directives -----?]"
		"[+%%?literal %]"
		"[+%a?access time in \bctime\b(3) format]"
		"[+%Ac?access time is \bstrftime\b(3) %\ac\a format]"
		"[+%b?file size in 512 byte blocks]"
		"[+%c?status change time in \bctime\b(3) format]"
		"[+%Cc?status change time is \bstrftime\b(3) %\ac\a format]"
		"[+%d?directory tree depth; 0 means command line argument]"
		"[+%f?file base name (no directory components)]"
		"[+%F?filesystem type name; use this for \b-fstype\b]"
		"[+%g?group name, or numeric group id if no name found]"
		"[+%G?numeric group id]"
		"[+%h?file directory name (no base component)]"
		"[+%H?command line argument under which file is found]"
		"[+%i?file inode number]"
		"[+%k?file size in kilobytes]"
		"[+%l?symbolic link text, empty if not symbolic link]"
		"[+%m?permission bits in octal]"
		"[+%n?number of hard links]"
		"[+%p?full path name]"
		"[+%P?file path with command line argument prefix deleted]"
		"[+%s?file size in bytes]"
		"[+%t?modify time in \bctime\b(3) format]"
		"[+%Tc?modify time is \bstrftime\b(3) %\ac\a format]"
		"[+%u?user name, or numeric user id if no name found]"
		"[+%U?numeric user id]"
		"[+%x?%p quoted for \bxargs\b(1)]"
		"[+%X?%P quoted for \bxargs\b(1)]",
	"Print format \aformat\a on the standard output, interpreting"
	" `\\' escapes and `%' directives. \bprintf\b(3) field width"
	" and precision are interpreted as usual, but the directive"
	" characters have special interpretation.",
"printx",	PRINTX,		Unary,		1,	0,	0,
	"Print the path name (including directory components) to the"
	" standard output, with \bxargs\b(1) special characters preceded"
	" by \b\\\b, followed by a newline.",
"prune",	PRUNE,		Unary,		0,	0,	0,
	"Ignored if \b-depth\b is given, otherwise do not descend the"
	" current directory.",
"regex",	REGEX,		Re,		0,	"pattern",	0,
	"Path name matches the anchored regular expression \apattern\a,"
	" i.e., leading ^ and traling $ are implied.",
"reverse",	REVERSE,	Unary,		0,	0,	0,
	"Reverse the \b-sort\b sense.",
"show",		SHOW,		Unary,		0,	0,	0,
	"Show actions on the standard output but do not execute. For example,"
	" use \b-show\b to test \b-delete\b.",
"silent",	SILENT,		Unary,		0,	0,	0,
	"Do not warn about inaccessible directories or symbolic link loops.",
"size",		SIZE,		Num|Stat|Unit,	0,	"number[bcgkm]]", 0,
	"File size is \anumber\a units (b: 512 byte blocks, c: characters"
	" g: 1024*1024*1024 blocks, k: 1024 blocks, m: 1024*1024 blocks.)"
	" Sizes are rounded to the next unit.",
"sort",		SORT,		Str,		0,	"option",	0,
	"Search each directory in \a-option\a sort order, e.g., \b-name\b"
	" sorts by name, \b-size\b sorts by size.",
"test",		TEST,		Num,		0,	"seconds",	0,
	"Set the current time to \aseconds\a since the epoch. Other"
	" implementation defined test modes may also be enabled.",
"true",		CTRUE,		Unary,		0,	0,	0,
	"Always true.",
"type",		TYPE,		Str|Stat,	0,	"type",
		"[+b?block special]"
		"[+c?character special]"
		"[+d?directory]"
		"[+f?regular file]"
		"[+l?symbolic link]"
		"[+p?named pipe (FIFO)]"
		"[+s?socket]"
		"[+C?contiguous]"
		"[+D?door]",
	"File type matches \atype\a:",
"used",		USED,		Num|Stat,	0,	"days",	0,
	"File was accessed \adays\a days after its status changed.",
"user|uid",	USER,		Str|Stat,0,	"id",	0,
	"File user id matches the name or number \aid\a.",
"xargs",	XARGS,		Exec,		1,	"command ... \\;", 0,
	"Like \b-exec\b except as many file args as permitted are"
	" appended to \acommand ...\a which may be executed"
	" 0 or more times depending on the number of files found and"
	" local system \bexec\b(2) argument limits.",
"xtype",	XTYPE,		Str|Stat,	0,	"type",	0,
	"Like \b-type\b, except if symbolic links are followed, the test"
	" is applied to the symbolic link itself, otherwise the test is applied"
	" to the pointed to file. Equivalent to \b-type\b if no symbolic"
	" links are involved.",
	0,
};

/*
 *  Table lookup routine
 */

static Args_t*
lookup(register char* word)
{
	register Args_t*	argp;
	register int		second;

	while (*word == '-')
		word++;
	if (*word)
	{
		second = word[1];
		for (argp = (Args_t*)commands; argp->name; argp++)
			if (second == argp->name[1] && streq(word, argp->name))
				return argp;
	}
	return 0;
}

/*
 * quote path component to sp for xargs(1)
 */

static void
quotex(register Sfio_t* sp, register const char* s, int term)
{
	register int	c;

	while (c = *s++)
	{
		if (isspace(c) || c == '\\' || c == '\'' || c == '"')
			sfputc(sp, '\\');
		sfputc(sp, c);
	}
	if (term >= 0)
		sfputc(sp, term);
}

/*
 * printf %! extension function
 */

static int
print(Sfio_t* sp, void* vp, Sffmt_t* dp)
{
	register Fmt_t*		fp = (Fmt_t*)dp;
	register FTSENT*	ent = fp->ent;
	register State_t*	state = fp->state;
	Value_t*		value = (Value_t*)vp;

	char*			s;

	if (dp->n_str > 0)
		sfsprintf(s = fp->tmp, sizeof(fp->tmp), "%.*s", dp->n_str, dp->t_str);
	else
		s = 0;
	switch (dp->fmt)
	{
	case 'A':
		dp->fmt = 's';
		dp->size = -1;
		value->s = fmttime(s, ent->fts_statp->st_atime);
		break;
	case 'b':
		dp->fmt = 'u';
		value->u = iblocks(ent->fts_statp);
		break;
	case 'C':
		dp->fmt = 's';
		dp->size = -1;
		value->s = fmttime(s, ent->fts_statp->st_ctime);
		break;
	case 'd':
		dp->fmt = 'u';
		value->u = ent->fts_level;
		break;
	case 'H':
		while (ent->fts_level > 0)
			ent = ent->fts_parent;
		/*FALLTHROUGH*/
	case 'f':
		dp->fmt = 's';
		dp->size = ent->fts_namelen;
		value->s = ent->fts_name;
		break;
	case 'F':
		dp->fmt = 's';
		dp->size = -1;
		value->s = fmtfs(ent->fts_statp);
		break;
	case 'g':
		dp->fmt = 's';
		dp->size = -1;
		value->s = fmtgid(ent->fts_statp->st_gid);
		break;
	case 'G':
		dp->fmt = 'd';
		value->i = ent->fts_statp->st_gid;
		break;
	case 'i':
		dp->fmt = 'u';
		value->u = ent->fts_statp->st_ino;
		break;
	case 'k':
		dp->fmt = 'u';
		value->u = iblocks(ent->fts_statp);
		break;
	case 'm':
		dp->fmt = 'o';
		value->i = ent->fts_statp->st_mode;
		break;
	case 'n':
		dp->fmt = 'u';
		value->u = ent->fts_statp->st_nlink;
		break;
	case 'p':
		dp->fmt = 's';
		dp->size = ent->fts_pathlen;
		value->s = ent->fts_path;
		break;
	case 'P':
		dp->fmt = 's';
		dp->size = -1;
		s = ent->fts_path;
		while (ent->fts_level > 0)
			ent = ent->fts_parent;
		s += ent->fts_pathlen;
		if (*s == '/')
			s++;
		value->s = s;
		break;
	case 's':
		dp->fmt = 'u';
		value->u = ent->fts_statp->st_size;
		break;
	case 'T':
		dp->fmt = 's';
		dp->size = -1;
		value->s = fmttime(s, ent->fts_statp->st_mtime);
		break;
	case 'u':
		dp->fmt = 's';
		dp->size = -1;
		value->s = fmtuid(ent->fts_statp->st_uid);
		break;
	case 'U':
		dp->fmt = 'd';
		value->i = ent->fts_statp->st_uid;
		break;
	case 'x':
		dp->fmt = 's';
		quotex(state->tmp, ent->fts_path, -1);
		dp->size = sfstrtell(state->tmp);
		if (!(value->s = sfstruse(state->tmp)))
		{
			error(ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		break;
	case 'X':
		dp->fmt = 's';
		s = ent->fts_path;
		while (ent->fts_level > 0)
			ent = ent->fts_parent;
		s += ent->fts_pathlen;
		if (*s == '/')
			s++;
		quotex(state->tmp, s, -1);
		dp->size = sfstrtell(state->tmp);
		if (!(value->s = sfstruse(state->tmp)))
		{
			error(ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		break;
	case 'Y':
		if (s)
		{
			switch (*s)
			{
			case 'H':
				dp->fmt = 's';
				dp->size = -1;
				value->s = "ERROR";
				break;
			case 'h':
				dp->fmt = 's';
				if (s = strrchr(ent->fts_path, '/'))
				{
					value->s = ent->fts_path;
					dp->size = s - ent->fts_path;
				}
				else
				{
					value->s = ".";
					dp->size = 1;
				}
				break;
			case 'l':
				dp->fmt = 's';
				dp->size = -1;
				value->s = S_ISLNK(ent->fts_statp->st_mode) && pathgetlink(PATH(ent), fp->tmp, sizeof(fp->tmp)) > 0 ? fp->tmp : "";
				break;
			default:
				error(2, "%%(%s)Y: invalid %%Y argument", s);
				return -1;
			}
			break;
		}
		/*FALLTHROUGH*/
	default:
		error(2, "internal error: %%%c: unknown format", dp->fmt);
		return -1;
	case 'Z':
		dp->fmt = 'c';
		value->i = 0;
		break;
	}
	dp->flags |= SFFMT_VALUE;
	return 0;
}

/*
 * convert the gnu-style-find printf format string for sfio extension
 */

static char*
format(State_t* state, register char* s)
{
	register char*	t;
	register int	c;
	char*		b;

	stresc(s);
	c = strlen(s);
	if (!(t = vmnewof(state->vm, 0, char, c * 2, 0)))
	{
		error(ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	b = t;
	while (c = *s++)
	{
		if (c == '%')
		{
			if (*s == '%')
			{
				*t++ = c;
				*t++ = *s++;
			}
			else
			{
				do
				{
					*t++ = c;
				} while ((c = *s++) && !isalpha(c));
				if (!c)
					break;
				switch (c)
				{
				case 'A':
				case 'C':
				case 'T':
					*t++ = '(';
					*t++ = '%';
					switch (*t++ = *s++)
					{
					case '@':
						*(t - 1) = '#';
						break;
					default:
						if (isalpha(*(t - 1)))
							break;
						*(t - 1) = 'K';
						s--;
						break;
					}
					*t++ = ')';
					break;
				case 'a':
				case 'c':
				case 't':
					c = toupper(c);
					break;
				case 'H':
				case 'h':
				case 'l':
					*t++ = '(';
					*t++ = c;
					*t++ = ')';
					c = 'Y';
					break;
				case 'b':
				case 'd':
				case 'f':
				case 'F':
				case 'g':
				case 'G':
				case 'i':
				case 'k':
				case 'm':
				case 'n':
				case 'p':
				case 'P':
				case 's':
				case 'u':
				case 'U':
				case 'x':
				case 'X':
				case 'Z':
					break;
				default:
					error(2, "%%%c: unknown format", c);
					return 0;
				}
			}
		}
		*t++ = c;
	}
	*t = 0;
	return b;
}

/*
 * compile the arguments
 */

static int
compile(State_t* state, char** argv, register Node_t* np, int nested)
{
	register char*		b;
	register Node_t*	oldnp = 0;
	register const Args_t*	argp;
	Node_t*			tp;
	char*			e;
	char**			com;
	regdisc_t*		redisc;
	int			index = opt_info.index;
	int			i;
	int			k;
	Cmddisc_t		disc;
	enum Command		oldop = PRINT;

	for (;;)
	{
		if ((i = optget(argv, state->usage)) > 0)
		{
			k = argv[opt_info.index-1][0];
			if (i == '?')
				error(ERROR_USAGE|4, "%s", opt_info.arg);
			if (i == ':')
				error(2, "%s", opt_info.arg);
			continue;
		}
		else if (i == 0)
		{
			if (e = argv[opt_info.index])
			{
				k = e[0];
				if (!e[1] || e[1] == k && !e[2])
					switch (k)
					{
					case '(':
						argv[opt_info.index] = "-begin";
						continue;
					case ')':
						argv[opt_info.index] = "-end";
						continue;
					case '!':
						argv[opt_info.index] = "-not";
						continue;
					case '&':
						argv[opt_info.index] = "-and";
						continue;
					case '|':
						argv[opt_info.index] = "-or";
						continue;
					}
			}
			oldop = PRINT;
			break;
		}
		argp = commands - (i + 10);
		state->primary |= argp->primary;
		np->next = 0;
		np->name = argp->name;
		np->action = argp->action;
		np->op = argp;
		np->second.i = 0; 
		np->third.u = 0; 
		if (argp->type & Stat)
			state->walkflags &= ~FTS_NOSTAT;
		if (argp->type & Op)
		{
			if (oldop == NOT || np->action != NOT && (oldop != PRINT || !oldnp))
			{
				error(2, "%s: operator syntax error", np->name);
				return -1;
			}
			oldop = argp->action;
		}
		else
		{
			oldop = PRINT;
			if (!(argp->type & Unary))
			{
				b = opt_info.arg;
				switch (argp->type & ~(Stat|Unit))
				{
				case File:
					if (state->show || streq(b, "/dev/stdout") || streq(b, "/dev/fd/1"))
						np->first.fp = state->output;
					else if (!(np->first.fp = sfopen(NiL, b, "w")))
					{
						error(ERROR_SYSTEM|2, "%s: cannot write", b);
						return -1;
					}
					break;
				case Num:
					if (*b == '+' || *b == '-')
					{
						np->second.i = *b; 
						b++;
					}
					np->first.u = strtoul(b, &e, 0);
					if (argp->type & Unit)
						switch (*e++)
						{
						default:
							e--;
							/*FALLTHROUGH*/
						case 'b':
							np->third.u = 512;
							break;
						case 'c':
							break;
						case 'g':
							np->third.u = 1024 * 1024 * 1024;
							break;
						case 'k':
							np->third.u = 1024;
							break;
						case 'm':
							np->third.u = 1024 * 1024;
							break;
						case 'w':
							np->third.u = 2;
							break;
						}
					if (*e)
						error(1, "%s: invalid character%s after number", e, *(e + 1) ? "s" : "");
					break;
				default:
					np->first.cp = b;
					break;
				}
			}
		}
		switch (argp->action)
		{
		case AND:
			continue;
		case OR:
		case COMMA:
			np->first.np = state->topnode;
			state->topnode = np;
			oldnp->next = 0;
			break;
		case LPAREN:
			tp = state->topnode;
			state->topnode = np + 1;
			if ((i = compile(state, argv, state->topnode, 1)) < 0)
				return i;
			if (!streq(argv[opt_info.index-1], "-end"))
			{
				error(2, "(...) imbalance -- closing ) expected", np->name);
				return -1;
			}
			np->first.np = state->topnode;
			state->topnode = tp;
			oldnp = np;
			np->next = np + i;
			np += i;
			continue;
		case RPAREN:
			if (!oldnp || !nested)
			{
				error(2, "(...) imbalance -- opening ( omitted", np->name);
				return -1;
			}
			oldnp->next = 0;
			return opt_info.index - index;
		case LOGIC:
			state->walkflags &= ~(FTS_META|FTS_PHYSICAL);
		ignore:
			np->action = IGNORE;
			continue;
		case META:
			state->walkflags |= FTS_META|FTS_PHYSICAL;
			goto ignore;
		case PHYS:
			state->walkflags &= ~FTS_META;
			state->walkflags |= FTS_PHYSICAL;
			goto ignore;
		case XDEV:
			state->walkflags |= FTS_XDEV;
			goto ignore;
		case POST:
			state->walkflags &= ~FTS_NOPOSTORDER;
			goto ignore;
		case CHECK:
			state->silent = 0;
			goto ignore;
		case NOLEAF:
			goto ignore;
		case REVERSE:
			state->reverse = 1;
			goto ignore;
		case SHOW:
			state->show = 1;
			goto ignore;
		case SILENT:
			state->silent = 1;
			goto ignore;
		case CODES:
			state->codes = b;
			goto ignore;
		case FAST:
			state->fast = b;
			goto ignore;
		case ICASE:
			state->icase = 1;
			goto ignore;
		case LOCAL:
			np->first.l = 0;
			np->second.i = '-';
			break;
		case ATIME:
		case CTIME:
		case MTIME:
			switch (np->second.i)
			{
			case '+':
				np->second.u = state->day - (np->first.u + 1) * DAY - 1;
				np->first.u = 0;
				break;
			case '-':
				np->second.u = ~0;
				np->first.u = state->day - np->first.u * DAY + 1;
				break;
			default:
				np->second.u = state->day - np->first.u * DAY - 1;
				np->first.u = state->day - (np->first.u + 1) * DAY;
				break;
			}
			break;
		case AMIN:
		case CMIN:
		case MMIN:
			np->action--;
			switch (np->second.i)
			{
			case '+':
				np->second.u = state->now - np->first.u * 60;
				np->first.u = 0;
				break;
			case '-':
				np->first.u = state->now - np->first.u * 60;
				np->second.u = ~0;
				break;
			default:
				np->second.u = state->now - np->first.u * 60;
				np->first.u = np->second.u - 60;
				break;
			}
			break;
		case USER:
			if ((np->first.l = struid(b)) < 0)
			{
				error(2, "%s: invalid user name", np->name);
				return -1;
			}
			break;
		case GROUP:
			if ((np->first.l = strgid(b)) < 0)
			{
				error(2, "%s: invalid group name", np->name);
				return -1;
			}
			break;
		case EXEC:
		case OK:
		case XARGS:
			state->walkflags |= FTS_NOCHDIR;
			com = argv + opt_info.index - 1;
			i = np->action == XARGS ? 0 : 1;
			k = np->action == OK ? CMD_QUERY : 0;
			for (;;)
			{
				if (!(b = argv[opt_info.index++]))
				{
					error(2, "incomplete statement");
					return -1;
				}
				if (streq(b, ";"))
					break;
				if (strmatch(b, "*{}*"))
				{
					if (!(k & CMD_INSERT) && streq(b, "{}") && (b = argv[opt_info.index]) && (streq(b, ";") || streq(b, "+") && !(i = 0)))
					{
						argv[opt_info.index - 1] = 0;
						opt_info.index++;
						break;
					}
					k |= CMD_INSERT;
				}
			}
			argv[opt_info.index - 1] = 0;
			if (k & CMD_INSERT)
				i = 1;
			CMDDISC(&disc, k|CMD_EXIT|CMD_IGNORE, errorf);
			if (!(np->first.xp = cmdopen(com, i, 0, "{}", &disc)))
			{
				error(ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			np->second.np = state->cmd;
			state->cmd = np;
			break;
		case MAGIC:
		case MIME:
			if (!state->magic)
			{
				state->magicdisc.version = MAGIC_VERSION;
				state->magicdisc.flags = 0;
				state->magicdisc.errorf = errorf;
				if (!(state->magic = magicopen(&state->magicdisc)) || magicload(state->magic, NiL, 0))
				{
					error(2, "%s: cannot load magic file", MAGIC_FILE);
					return -1;
				}
			}
			break;
		case IREGEX:
		case REGEX:
			if (!(np->second.re = vmnewof(state->vm, 0, regex_t, 1, sizeof(regdisc_t))))
			{
				error(ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			redisc = (regdisc_t*)(np->second.re + 1);
			redisc->re_version = REG_VERSION;
			redisc->re_flags = REG_NOFREE;
			redisc->re_errorf = (regerror_t)errorf;
			redisc->re_resizef = (regresize_t)vmgetmem;
			redisc->re_resizehandle = (void*)state->vm;
			np->second.re->re_disc = redisc;
			i = REG_EXTENDED|REG_LENIENT|REG_NOSUB|REG_NULL|REG_LEFT|REG_RIGHT|REG_DISCIPLINE;
			if (argp->action == IREGEX)
			{
				i |= REG_ICASE;
				np->action = REGEX;
			}
			if (i = regcomp(np->second.re, b, i))
			{
				regfatal(np->second.re, 2, i);
				return -1;
			}
			break;
		case PERM:
			if (*b == '-' || *b == '+')
				np->second.l = *b++;
			np->first.l = strperm(b, &e, -1);
			if (*e)
			{
				error(2, "%s: invalid permission expression", e);
				return -1;
			}
			break;
		case SORT:
			if (!(argp = lookup(b)))
			{
				error(2, "%s: invalid sort key", b);
				return -1;
			}
			state->sortkey = argp->action;
			goto ignore;
		case TYPE:
		case XTYPE:
			np->first.l = *b;
			break;
		case CPIO:
			com = (char**)cpio;
			goto common;
		case NCPIO:
			{
				long	ops[2];
				int	fd;

				com = (char**)ncpio;
			common:
				/*
				 * set up cpio
				 */

				state->output = np->first.fp;
				if (!state->show)
				{
					ops[0] = PROC_FD_DUP(sffileno(state->output), 1, PROC_FD_PARENT|PROC_FD_CHILD);
					ops[1] = 0;
					if (!(state->proc = procopen("cpio", com, NiL, ops, PROC_WRITE)))
					{
						error(ERROR_SYSTEM|2, "cpio: cannot exec");
						return -1;
					}
				}
				state->walkflags &= ~FTS_NOPOSTORDER;
				np->action = PRINT;
			}
			/*FALLTHROUGH*/
		case PRINT:
			np->first.fp = state->output;
			np->second.i = '\n';
			break;
		case PRINT0:
			np->first.fp = state->output;
			np->second.i = 0;
			np->action = PRINT;
			break;
		case PRINTF:
			np->second.cp = format(state, np->first.cp);
			np->first.fp = state->output;
			break;
		case PRINTX:
			np->first.fp = state->output;
			np->second.i = '\n';
			break;
		case FPRINT:
			np->second.i = '\n';
			np->action = PRINT;
			break;
		case FPRINT0:
			np->second.i = 0;
			np->action = PRINT;
			break;
		case FPRINTF:
			if (!(b = argv[opt_info.index++]))
			{
				error(2, "incomplete statement");
				return -1;
			}
			np->second.cp = format(state, b);
			break;
		case FPRINTX:
			np->second.i = '\n';
			np->action = PRINTX;
			break;
		case LS:
			np->first.fp = state->output;
			if (state->sortkey == IGNORE)
				state->sortkey = NAME;
			break;
		case FLS:
			if (state->sortkey == IGNORE)
				state->sortkey = NAME;
			np->action = LS;
			break;
		case DELETE:
			state->walkflags &= ~FTS_NOPOSTORDER;
			break;
		case NEWER:
		case ANEWER:
		case CNEWER:
			{
				struct stat	st;

				if (stat(b, &st))
				{
					error(2, "%s: not found", b);
					return -1;
				}
				np->first.l = st.st_mtime;
				np->second.i = '+';
			}
			break;
		case CHOP:
			state->walkflags |= FTS_NOSEEDOTDIR;
			goto ignore;
		case DAYSTART:
			{
				Tm_t*	tm;
				time_t	t;

				t = state->now;
				tm = tmmake(&t);
				tm->tm_hour = 0;
				tm->tm_min = 0;
				tm->tm_sec = 0;
				state->day = tmtime(tm, TM_LOCALZONE);
			}
			goto ignore;
		case MINDEPTH:
			state->minlevel = np->first.l;
			goto ignore;
		case MAXDEPTH:
			state->maxlevel = np->first.l;
			goto ignore;
		case TEST:
			state->day = np->first.u;
			goto ignore;
		}
		oldnp = np;
		oldnp->next = ++np;
	}
	if (oldop != PRINT)
	{
		error(2, "%s: invalid argument", argv[opt_info.index - 1]);
		return -1;
	}
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	state->nextnode = np;
	if (state->lastnode = oldnp)
		oldnp->next = 0;
	return opt_info.index - index;
}

/*
 * This is the function that gets executed at each node
 */

static int
execute(State_t* state, FTSENT* ent)
{
	register Node_t*	np = state->topnode;
	register int		val = 0;
	register unsigned long	u;
	unsigned long		m;
	int			not = 0;
	char*			bp;
	Sfio_t*			fp;
	Node_t*			tp;
	struct stat		st;
	DIR*			dir;
	struct dirent*		dnt;

	if (ent->fts_level > state->maxlevel)
	{
		fts_set(NiL, ent, FTS_SKIP);
		return 0;
	}
	switch (ent->fts_info)
	{
	case FTS_DP:
		if ((state->walkflags & FTS_NOCHDIR) && stat(PATH(ent), ent->fts_statp))
			return 0;
		break;
	case FTS_NS:
		if (!state->silent)
			error(2, "%s: not found", ent->fts_path);
		return 0;
	case FTS_DC:
		if (!state->silent)
			error(2, "%s: directory causes cycle", ent->fts_path);
		return 0;
	case FTS_DNR:
		if (!state->silent)
			error(2, "%s: cannot read directory", ent->fts_path);
		break;
	case FTS_DNX:
		if (!state->silent)
			error(2, "%s: cannot search directory", ent->fts_path);
		fts_set(NiL, ent, FTS_SKIP);
		break;
	case FTS_D:
		if (!(state->walkflags & FTS_NOPOSTORDER))
			return 0;
		ent->ignorecase = (state->icase || (!ent->fts_level || !ent->fts_parent->ignorecase) && strchr(astconf("PATH_ATTRIBUTES", ent->fts_name, NiL), 'c')) ? STR_ICASE : 0;
		break;
	default:
		ent->ignorecase = ent->fts_level ? ent->fts_parent->ignorecase : (state->icase || strchr(astconf("PATH_ATTRIBUTES", ent->fts_name, NiL), 'c')) ? STR_ICASE : 0;
		break;
	}
	if (ent->fts_level < state->minlevel)
		return 0;
	while (np)
	{
		switch (np->action)
		{
		case NOT:
			not = !not;
			np = np->next;
			continue;
		case COMMA:
		case LPAREN:
		case OR:
			tp = state->topnode;
			state->topnode = np->first.np;
			if ((val = execute(state, ent)) < 0)
				return val;
			state->topnode = tp;
			switch (np->action)
			{
			case COMMA:
				val = 1;
				break;
			case OR:
				if (val)
					return 1;
				val = 1;
				break;
			}
			break;
		case LOCAL:
			u = fts_local(ent);
			goto num;
		case XTYPE:
			val = ((state->walkflags & FTS_PHYSICAL) ? stat(PATH(ent), &st) : lstat(PATH(ent), &st)) ? 0 : st.st_mode;
			goto type;
		case TYPE:
			val = ent->fts_statp->st_mode;
		type:
			switch (np->first.l)
			{
			case 'b':
				val = S_ISBLK(val);
				break;
			case 'c':
				val = S_ISCHR(val);
				break;
			case 'd':
				val = S_ISDIR(val);
				break;
			case 'f':
				val = S_ISREG(val);
				break;
			case 'l':
				val = S_ISLNK(val);
				break;
			case 'p':
				val = S_ISFIFO(val);
				break;
#ifdef S_ISSOCK
			case 's':
				val = S_ISSOCK(val);
				break;
#endif
#ifdef S_ISCTG
			case 'C':
				val = S_ISCTG(val);
				break;
#endif
#ifdef S_ISDOOR
			case 'D':
				val = S_ISDOOR(val);
				break;
#endif
			default:
				val = 0;
				break;
			}
			break;
		case PERM:
			u = modex(ent->fts_statp->st_mode) & 07777;
			switch (np->second.i)
			{
			case '-':
				val = (u & np->first.u) == np->first.u;
				break;
			case '+':
				val = (u & np->first.u) != 0;
				break;
			default:
				val = u == np->first.u;
				break;
			}
			break;
		case INUM:
			u = ent->fts_statp->st_ino;
			goto num;
		case ATIME:
			u = ent->fts_statp->st_atime;
			goto tim;
		case CTIME:
			u = ent->fts_statp->st_ctime;
			goto tim;
		case MTIME:
			u = ent->fts_statp->st_mtime;
		tim:
			val = u >= np->first.u && u <= np->second.u;
			break;
		case NEWER:
			val = (unsigned long)ent->fts_statp->st_mtime > (unsigned long)np->first.u;
			break;
		case ANEWER:
			val = (unsigned long)ent->fts_statp->st_atime > (unsigned long)np->first.u;
			break;
		case CNEWER:
			val = (unsigned long)ent->fts_statp->st_ctime > (unsigned long)np->first.u;
			break;
		case SIZE:
			u = ent->fts_statp->st_size;
			goto num;
		case USER:
			u = ent->fts_statp->st_uid;
			goto num;
		case NOUSER:
			val = *fmtuid(ent->fts_statp->st_uid);
			val = isdigit(val);
			break;
		case GROUP:
			u = ent->fts_statp->st_gid;
			goto num;
		case NOGROUP:
			val = *fmtgid(ent->fts_statp->st_gid);
			val = isdigit(val);
			break;
		case LINKS:
			u = ent->fts_statp->st_nlink;
		num:
			if (m = np->third.u)
				u = (u + m - 1) / m;
			switch (np->second.i)
			{
			case '+':
				val = (u > np->first.u);
				break;
			case '-':
				val = (u < np->first.u);
				break;
			default:
				val = (u == np->first.u);
				break;
			}
			break;
		case EXEC:
		case OK:
		case XARGS:
			val = !cmdarg(np->first.xp, ent->fts_path, ent->fts_pathlen);
			break;
		case NAME:
		case INAME:
			if (bp = ent->fts_level ? (char*)0 : strchr(ent->fts_name, '/'))
				*bp = 0;
			val = strgrpmatch(ent->fts_name, np->first.cp, NiL, 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|(np->action == INAME ? STR_ICASE : ent->ignorecase)) != 0;
			if (bp)
				*bp = '/';
			break;
		case LNAME:
			val = S_ISLNK(ent->fts_statp->st_mode) && pathgetlink(PATH(ent), state->txt, sizeof(state->txt)) > 0 && strgrpmatch(state->txt, np->first.cp, NiL, 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|ent->ignorecase);
			break;
		case ILNAME:
			val = S_ISLNK(ent->fts_statp->st_mode) && pathgetlink(PATH(ent), state->txt, sizeof(state->txt)) > 0 && strgrpmatch(state->txt, np->first.cp, NiL, 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|STR_ICASE);
			break;
		case PATH:
			val = strgrpmatch(ent->fts_path, np->first.cp, NiL, 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|ent->ignorecase) != 0;
			break;
		case IPATH:
			val = strgrpmatch(ent->fts_path, np->first.cp, NiL, 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|STR_ICASE) != 0;
			break;
		case MAGIC:
			fp = sfopen(NiL, PATH(ent), "r");
			val = strmatch(magictype(state->magic, fp, PATH(ent), ent->fts_statp), np->first.cp) != 0;
			if (fp)
				sfclose(fp);
			break;
		case MIME:
			fp = sfopen(NiL, PATH(ent), "r");
			state->magicdisc.flags |= MAGIC_MIME;
			val = strmatch(magictype(state->magic, fp, PATH(ent), ent->fts_statp), np->first.cp) != 0;
			state->magicdisc.flags &= ~MAGIC_MIME;
			if (fp)
				sfclose(fp);
			break;
		case REGEX:
			if (!(val = regnexec(np->second.re, ent->fts_path, ent->fts_pathlen, NiL, 0, 0)))
				val = 1;
			else if (val == REG_NOMATCH)
				val = 0;
			else
			{
				regfatal(np->second.re, 4, val);
				return -1;
			}
			break;
		case PRINT:
			sfputr(np->first.fp, ent->fts_path, np->second.i);
			val = 1;
			break;
		case PRINTF:
			state->fmt.fmt.version = SFIO_VERSION;
			state->fmt.fmt.extf = print;
			state->fmt.fmt.form = np->second.cp;
			state->fmt.ent = ent;
			sfprintf(np->first.fp, "%!", &state->fmt);
			val = 1;
			break;
		case PRINTX:
			quotex(np->first.fp, ent->fts_path, np->second.i);
			val = 1;
			break;
		case PRUNE:
			fts_set(NiL, ent, FTS_SKIP);
			val = 1;
			break;
		case FSTYPE:
			val = strcmp(fmtfs(ent->fts_statp), np->first.cp) == 0;
			break;
		case LS:
			fmtls(state->buf, ent->fts_path, ent->fts_statp, NiL, S_ISLNK(ent->fts_statp->st_mode) && pathgetlink(PATH(ent), state->txt, sizeof(state->txt)) > 0 ? state->txt : NiL, LS_LONG|LS_INUMBER|LS_BLOCKS);
			sfputr(np->first.fp, state->buf, '\n');
			val = 1;
			break;
		case EMPTY:
			if (S_ISREG(ent->fts_statp->st_mode))
				val = !ent->fts_statp->st_size;
			else if (!S_ISDIR(ent->fts_statp->st_mode))
				val = 0;
			else if (!ent->fts_statp->st_size)
				val = 1;
			else if (!(dir = opendir(ent->fts_path)))
			{
				if (!state->silent)
					error(2, "%s: cannot read directory", ent->fts_path);
				val = 0;
			}
			else
			{
				while ((dnt = readdir(dir)) && (dnt->d_name[0] == '.' && (!dnt->d_name[1] || dnt->d_name[1] == '.' && !dnt->d_name[2])));
				val = !dnt;
				closedir(dir);
			}
			break;
		case CFALSE:
			val = 0;
			break;
		case CTRUE:
			val = 1;
			break;
		case LEVEL:
			u = ent->fts_level;
			goto num;
		case DELETE:
			if (S_ISDIR(ent->fts_statp->st_mode))
			{
				if (!streq(ent->fts_path, ".") && rmdir(ent->fts_accpath))
					error(ERROR_SYSTEM|2, "%s: cannot delete directory", ent->fts_path);
			}
			else if (remove(ent->fts_accpath))
				error(ERROR_SYSTEM|2, "%s: cannot delete file", ent->fts_path);
			val = 1;
			break;
		case SHOW:
			sfprintf(sfstdout, "%s %s\n", np->name, ent->fts_path);
			val = 1;
			break;
		default:
			error(2, "internal error: %s: action not implemented", np->name);
			return -1;
		}
		if (!(val ^= not))
			break;
		not = 0;
		np = np->next;
	}
	return val;
}

/*
 * order child entries
 */

static int
order(FTSENT* const* p1, FTSENT* const* p2)
{
	register const FTSENT*	f1 = *p1;
	register const FTSENT*	f2 = *p2;
	register State_t*	state = f1->fts->fts_handle;
	register long		n1;
	register long		n2;
	int			n;

	switch (state->sortkey)
	{
	case ATIME:
		n2 = f1->fts_statp->st_atime;
		n1 = f2->fts_statp->st_atime;
		break;
	case CTIME:
		n2 = f1->fts_statp->st_ctime;
		n1 = f2->fts_statp->st_ctime;
		break;
	case MTIME:
		n2 = f1->fts_statp->st_mtime;
		n1 = f2->fts_statp->st_mtime;
		break;
	case SIZE:
		n2 = f1->fts_statp->st_size;
		n1 = f2->fts_statp->st_size;
		break;
	default:
		error(1, "invalid sort key -- name assumed");
		state->sortkey = NAME;
		/*FALLTHROUGH*/
	case NAME:
		n = state->icase ? strcasecmp(f1->fts_name, f2->fts_name) : strcoll(f1->fts_name, f2->fts_name);
		goto done;
	}
	if (n1 < n2)
		n = -1;
	else if (n1 > n2)
		n = 1;
	else
		n = 0;
 done:
	if (state->reverse)
		n = -n;
	return n;
}

static int
find(State_t* state, char** paths, int flags, Sort_f sort)
{
	FTS*	fts;
	FTSENT*	ent;
	int	r;

	r = 0;
	if (fts = fts_open(paths, flags, sort))
	{
		fts->fts_handle = state;
		while (ent = fts_read(fts))
			if (execute(state, ent) < 0)
			{
				r = 1;
				break;
			}
		fts_close(fts);
	}
	return r;
}

int
main(int argc, char** argv)
{
	register char*			cp;
	register char**			op;
	register Find_t*		fp;
	register const Args_t*		ap;
	Node_t*				np;
	int				r;
	Sort_f				sort;
	Finddisc_t			disc;
	State_t				state;

	static const char* const	defpath[] = { ".", 0 };

	setlocale(LC_ALL, "");
	error_info.id = "find";
	memset(&state, 0, sizeof(state));
	if (!(state.vm = vmopen(Vmdcheap, Vmbest, 0)) || !(state.str = sfstropen()) || !(state.tmp = sfstropen()))
	{
		error(ERROR_SYSTEM|2, "out of space");
		goto done;
	}
	state.maxlevel = ~0;
	state.walkflags = FTS_PHYSICAL|FTS_NOSTAT|FTS_NOPOSTORDER|FTS_SEEDOTDIR;
	state.sortkey = IGNORE;
	sort = 0;
	fp = 0;
	sfputr(state.str, usage1, -1);
	for (ap = commands; ap->name; ap++)
	{
		sfprintf(state.str, "[%d:%s?%s]", ap - commands + 10, ap->name, ap->help);
		if (ap->arg)
			sfprintf(state.str, "%c[%s]", (ap->type & Num) ? '#' : ':', ap->arg);
		if (ap->values)
			sfprintf(state.str, "{%s}", ap->values);
		sfputc(state.str, '\n');
	}
	sfputr(state.str, usage2, -1);
	if (!(state.usage = sfstruse(state.str)))
	{
		error(ERROR_SYSTEM|2, "out of space");
		goto done;
	}
	state.day = state.now = (unsigned long)time(NiL);
	state.output = sfstdout;
	if (!(state.topnode = vmnewof(state.vm, 0, Node_t, argc + 3, 0)))
	{
		error(2, "not enough space for expressions");
		goto done;
	}
	if (compile(&state, argv, state.topnode, 0) < 0)
		goto done;
	op = argv + opt_info.index;
	while (cp = argv[opt_info.index])
	{
		if (*cp == '-' || (*cp == '!' || *cp == '(' || *cp == ')' || *cp == ',') && *(cp + 1) == 0)
		{
			r = opt_info.index;
			if (compile(&state, argv, state.topnode, 0) < 0)
				goto done;
			argv[r] = 0;
			if (cp = argv[opt_info.index])
			{
				error(2, "%s: invalid argument", cp);
				goto done;
			}
			break;
		}
		opt_info.index++;
	}
	if (!*op)
		op = (char**)defpath;
	while (state.topnode && state.topnode->action == IGNORE)
		state.topnode = state.topnode->next;
	if (!(state.walkflags & FTS_PHYSICAL))
		state.walkflags &= ~FTS_NOSTAT;
	if (state.fast)
	{
		if (state.sortkey != IGNORE)
			error(1, "-sort ignored for -fast");
		memset(&disc, 0, sizeof(disc));
		disc.version = FIND_VERSION;
		disc.flags = state.icase ? FIND_ICASE : 0;
		disc.errorf = errorf;
		disc.dirs = op;
		state.walkflags |= FTS_TOP;
		if (fp = findopen(state.codes, state.fast, NiL, &disc))
			while (cp = findread(fp))
			{
				if (!state.topnode)
					sfputr(sfstdout, cp, '\n');
				else if (find(&state, (char**)cp, FTS_ONEPATH|state.walkflags, NiL))
					goto done;
			}
	}
	else
	{
		if (!state.primary)
		{
			if (!state.topnode)
				state.topnode = state.nextnode;
			else if (state.topnode != state.nextnode)
			{
				state.nextnode->action = LPAREN;
				state.nextnode->first.np = state.topnode;
				state.nextnode->next = state.nextnode + 1;
				state.topnode = state.nextnode++;
			}
			state.nextnode->action = PRINT;
			state.nextnode->first.fp = state.output;
			state.nextnode->second.i = '\n';
			state.nextnode->next = 0;
		}
		if (state.show)
			for (np = state.topnode; np; np = np->next)
				if (np->op->primary)
					np->action = SHOW;
		fp = 0;
		if (state.sortkey != IGNORE)
			sort = order;
		find(&state, op, state.walkflags, sort);
	}
 done:
	while (state.cmd)
	{
		cmdflush(state.cmd->first.xp);
		cmdclose(state.cmd->first.xp);
		state.cmd = state.cmd->second.np;
	}
	if (state.vm)
		vmclose(state.vm);
	if (state.str)
		sfstrclose(state.str);
	if (state.tmp)
		sfstrclose(state.tmp);
	if (fp && findclose(fp))
		error(ERROR_SYSTEM|2, "fast find error");
	if (state.proc && (r = procclose(state.proc)))
		error(ERROR_SYSTEM|2, "subprocess exit code %d", r);
	if (sfsync(sfstdout))
		error(ERROR_SYSTEM|2, "write error");
	return error_info.errors != 0;
}
