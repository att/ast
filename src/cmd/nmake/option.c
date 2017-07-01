/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2013 AT&T Intellectual Property          *
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
 * make options support
 *
 * option name mappings are here
 * option flag mappings are in options.h
 */

#include "make.h"
#include "options.h"

#define OPT_OFFSET	10
#define OPT_NON		'-'
#define OPT_SEP		';'

static const char usage1[] =
"+"
"[-?%s\n]"
USAGE_LICENSE
"[+NAME?nmake - configure, manage and update file hierarchies]"
"[+DESCRIPTION?\bnmake\b reads input \amakefiles\a and triggers shell"
"	actions to build target files that are out of date with prerequisite"
"	files. Most information used to build targets is contained in"
"	the global \abase rules\a that are augmented by user \amakefiles\a."
"	Each operand may be an option, script, or target. An option operand"
"	is preceded by \b-\b or \b+\b. A script operand contains at least"
"	one of \bspace\b, \btab\b, \bnewline\b, \b:\b, \b=\b, \b\"\b, or"
"	\b\\\b and is parsed as a separate, complete makefile. Otherwise"
"	the operand is a \atarget\a that is generated according to the"
"	\amakefile\a and \aglobal\a rules. \atarget\a operands are made"
"	in order from left to right and override the default targets.]"
"[+?Command line options, scripts and targets may appear in any order,"
"	with the exception that no option operand may appear after a"
"	\b--\b operand.]"
"[+?Options are qualified by the base name of the makefile that defined"
"	them. Unqualified options are defined by \bnmake\b itself.]"
;

static Option_t		options[] =	/* option table			*/
{

{ "accept",	OPT_accept,	(char*)&state.accept,		0,
	"Accept filesystem timestamps of existing targets." },
{ "alias",	OPT_alias,	(char*)&state.alias,		0,
	"Enable directory aliasing." },
{ "base",		OPT_base,	(char*)&state.base,	0,
	"Compile base or global rules." },
{ "believe",	OPT_believe,	(char*)&state.believe,		0,
	"Believe the state file time of files lower than view level"
	" \alevel-1\a. The file system time will be checked for files with"
	" no state or files in views equal to or higher than \alevel\a."
	" \alevel=0\a causes the file system time to be checked for"
	" files on all view levels. The top view is level 0.", "level:=0" },
{ "compatibility",OPT_compatibility,(char*)&state.compatibility,0,
	"Disable compatibility messages." },
{ "compile",	OPT_compile,	(char*)&state.compileonly,	0,
	"Compile the input makefile and exit." },
{ "corrupt",OPT_corrupt,	(char*)&state.corrupt,		0,
	"\aaction\a determines the action to take for corrupt or invalid"
	" top view state files. The top view default is \berror\b and the"
	" lower view default is \baccept\b. \aaction\a may be one of:",
	"[action:!accept]"
	"{"
	"	[+accept?print a warning and set \b--accept\b]"
	"	[+error?print a diagnostic and exit]"
	"	[+ignore?print a warning and set \b--noreadstate\b]"
	"}" },
{ "cross",	OPT_cross,	(char*)&state.cross,		0,
	"Don't run generated executables." },
{ "debug",	OPT_debug,	0,				0,
	"Set the debug trace level to \alevel\a. Higher levels produce"
	" more output.", "level" },
{ "errorid",	OPT_errorid,	(char*)&state.errorid,		0,
	"Add \aid\a to the error message command identifier.", "id:=make" },
{ "exec",		OPT_exec,	(char*)&state.exec,	0,
	"Enable shell action execution. \b--noexec\b"
	" disables all but \b.ALWAYS\b shell actions and also disables"
	" make object and state file generation/updates." },
{ "expandview",	OPT_expandview,	(char*)&state.expandview,	0,
	"Expand \a3d\a filesystem paths." },
{ "explain",	OPT_explain,	(char*)&state.explain,		0,
	"Explain each action." },
{ "file",		OPT_file,	(char*)&internal.makefiles,	0,
	"Read the makefile \afile\a. If \b--file\b is not specified then"
	" the makefile names specified by \b$(MAKEFILES)\b are attempted in"
	" order from left to right. The file \b-\b is equivalent"
	" to \b/dev/null\b.", "file" },
{ "force",	OPT_force,	(char*)&state.force,		0,
	"Force all targets to be out of date." },
{ "global",	OPT_global,	(char*)&internal.globalfiles,	0,
	"Read the global makefile \afile\a. The \b--file\b search is not"
	" affected.", "file" },
{ "ignore",	OPT_ignore,	(char*)&state.ignore,		0,
	"Ignore shell action errors." },
{ "ignorelock",	OPT_ignorelock,	(char*)&state.ignorelock,	0,
	"Ignore state file locks." },
{ "include",	OPT_include,	0,				0,
	"Add \adirectory\a to the makefile search list.", "directory" },
{ "intermediate",	OPT_intermediate,(char*)&state.intermediate,	0,
	"Force intermediate target generation." },
{ "jobs",		OPT_jobs,	(char*)&state.jobs,		0,
	"Set the shell action concurrency level to \alevel\a."
	" Level \b1\b allows dependency checking while an action is"
	" executing; level \b0\b stops all activity while an action"
	" is executing.", "level:=${" CO_ENV_PROC "::-1}" },
{ "keepgoing",	OPT_keepgoing,	(char*)&state.keepgoing,	0,
	"Continue after error with sibling prerequisites." },
{ "list",		OPT_list,	(char*)&state.list,		0,
	"List the current rules and variables on the standard output in"
	" makefile form." },
{ "mam",		OPT_mam,	(char*)&state.mam.options,	0,
	"Write \amake abstract machine\a output to \afile\a if specified or"
	" to the standard output otherwise. See \bmam\b(5) for details on"
	" the \amake abstract machine\a language. If \aparent\a !=0 then it is"
	" the process id of a parent \amam\a process. \adirectory\a is"
	" the working directory of the current \amam\a process relative"
	" to the root \amam\a process, \b.\b if not specified. \atype\a"
	" must be one of:",
	"[type[,subtype]][::file[::parent[::directory]]]]]]]"
	"{"
		"[+dynamic?\amam\a trace of an actual build]"
		"[+regress?\amam\a for regression testing; labels, path"
		" names and time stamps are canonicalized for easy comparison]"
		"[+static?\amam\a representation of the makefile assertions;"
		" used for makefile conversion]"
		"[+----?0 or more comma separated subtypes ----]"
		"[+port?used by the base rules to generate portable"
		" makefiles; some paths are parameterized; on by default]"
		"[+----?mam options ----]"
		"[+[no]]hold?\bhold\b \amam\a output until nested \bnohold\b]"
	"}" },
{ "never",	OPT_never,	(char*)&state.never,		0,
	"Don't execute any shell actions. \b--noexec\b executes \b.ALWAYS\b"
	" shell actions." },
{ "option",	OPT_option,	0,				0,
	"Define a new option. The definition is a delimiter separated field"
	" list. Any non-alpha-numeric delimiter other than \b-\b may be used."
	" \b;\b is used in this description. Makefile \bset option\b"
	" definitions must be '...' quoted. Two adjacent delimiters specifies"
	" the literal delimiter character and a \b-\b field value specifies an"
	" empty field. \achar\a is the single character option name, \aname\a"
	" is the long option name, \aset\a is an optional \b.FUNCTION\b that"
	" is called when the option value is changed by \bset\b, \avalues\a is"
	" an \boptget\b(3) value list, and \aflags\a are a combination of:",
	"['char;name;flags;set;description;values']"
	"{"
	"	[+a?multiple values appended]"
	"	[+b?boolean value]"
	"	[+i?internal value inverted]"
	"	[+n?numeric value]"
	"	[+o?\a-char\a means \b--no\b\aname\a]"
	"	[+p?.mo probe prerequisite]"
	"	[+s?string value]"
	"	[+v?optional option argument]"
	"	[+x?not expanded in \b$(-)\b]"
	"}" },
{ "override",	OPT_override,	(char*)&state.override,		0,
	"Implicit rules or metarules override explicit rules." },
{ "questionable",	OPT_questionable,(char*)&state.questionable,	0,
	"Enable questionable features defined by \amask\a. Questionable"
	" features are artifacts of previous implementations (\bnmake\b has"
	" been around since 1984-11-01) that will eventually be dropped."
	" The questionable \amask\a registry is in the \bmain.c\b \bnmake\b"
	" source file.", "mask" },
{ "readonly",	OPT_readonly,	(char*)&state.readonly,		0,
	"Current assignments and assertions will be marked \breadonly\b." },
{ "readstate",	OPT_readstate,	(char*)&state.readstate,	0,
	"Ignore state files lower than view level \alevel\a. \alevel=0\a"
	" ignores state files on all view levels. The top view is level 0.",
	"level:=0" },
{ "regress",	OPT_regress,	(char*)&state.regress,		0,
	"Massage output for regression testing. \aaction\a may be one of:",
	"[action:!message]"
	"{"
	"	[+message?alter messages only]"
	"	[+sync?sync 1-second clocks if necessary and alter messages]"
	"}" },
{ "reread",	OPT_reread,	(char*)&state.reread,		0,
	"Ignore any previously generated \b.mo\b files and re-read all"
	" input makefiles." },
{ "ruledump",	OPT_ruledump,	(char*)&state.ruledump,		0,
	"Dump rule information in tabular form on the standard"
	" error when \bnmake\b exits." },
{ "scan",		OPT_scan,	(char*)&state.scan,	0,
	"Scan for and/or check implicit file prerequisites. On by default." },
{ "serialize",	OPT_serialize,	(char*)&state.serialize,	0,
	"Serialize concurrent output by caching job stdout and stderr"
	" output until job completion." },
{ "silent",	OPT_silent,	(char*)&state.silent,		0,
	"Do not trace shell actions as they are executed." },
{ "strictview",	OPT_strictview,	(char*)&state.strictview,	0,
	"Set \bVPATH\b \b.SOURCE\b rule interpretation to follow strict"
	" \a3d\a filesystem semantics, where directories in the top views"
	" take precedence. On by default when running in \a2d\a with"
	" \bVPATH\b defined, off by default otherwise." },
{ "target-context",OPT_targetcontext,(char*)&state.targetcontext,	0,
	"Expand and execute shell actions in the target directory context."
	" This allows a single makefile to control a directory tree while"
	" generating target files at the source file directory level. By"
	" default target files are generated in the current directory." },
{ "target-prefix",	OPT_targetprefix,(char*)&state.targetprefix,	0,
	"Allow metarules to match \aseparator\a in the target to \b/\b"
	" in the source. Used to disambiguate source file base name clashes"
	" when target files are generated in the current directory."
	" \aseparator\a must not contain metarule or shell pattern characters.",
	"separator" },
{ "test",		OPT_test,	(char*)&state.test,	0,
	"Enable test code defined by \amask\a. Test code is implementation"
	" specific. The test \amask\a registry is in the \bmain.c\b \bnmake\b"
	" source file.", "mask" },
{ "tolerance",	OPT_tolerance,	(char*)&state.tolerance,	0,
	"Set the time comparison tolerance to \aseconds\a. Times within"
	" the tolerance range compare equal. Useful on systems that can't"
	" quite get the file system and local clocks in sync. A tolerance"
	" of more that 5 seconds soon becomes intolerable.", "seconds" },
{ "touch",	OPT_touch,	(char*)&state.touch,		0,
	"Touch the time stamps of out of date targets rather than execute"
	" the shell action." },
{ "vardump",	OPT_vardump,	(char*)&state.vardump,		0,
	"Dump variable information in tabular form on the standard"
	" error when \bnmake\b exits." },
{ "warn",		OPT_warn,	(char*)&state.warn,	0,
	"Enable verbose warning messages." },
{ "writeobject",	OPT_writeobject,(char*)&state.writeobject,	0,
	"Generate a \b.mo\b make object file in \afile\a that can be read"
	" instead of the input makefiles on the next \bnmake\b invocation."
	" On by default. \b--nowriteobject\b prevents the generation."
	" The default name is used if \afile\a is omitted or \b-\b."
	" If \afile\a is a directory then the default is placed in that"
	" directory.",
	"file:=$(MAKEFILE::B::S=.mo)" },
{ "writestate",	OPT_writestate,	(char*)&state.writestate,	0,
	"Generate a \b.ms\b make state file in \afile\a when \bnmake\b exits."
	"The state contains the time stamps of all prerequisites and targets"
	" that have been accessed since the state file was first generated."
	" On by default. \b--nowritestate\b prevents the generation."
	" The default name is used if \afile\a is omitted or \b-\b."
	" If \afile\a is a directory then the default is placed in that"
	" directory.",
	"file:=$(MAKEFILE::B::S=.ms)" },

{ "byname",	OPT_byname,	0,				0,
	"(obsolete) Set options by name.", "name[=value]]" },
{ "define",	OPT_define,	0,				0,
	"(obsolete) Pass macro definition to the makefile preprocessor.",
	"name[=value]]" },
{ "preprocess",	OPT_preprocess,	0,				0,
	"(obsolete) Preprocess all makefiles." },
{ "undef",	OPT_undef,	0,				0,
	"(obsolete) Pass macro deletion to the makefile preprocessor.",
	"name" },

};

static const char usage2[] =
"\n"
"[ script ... ] [ target ... ]\n"
"\n"
"[+DIAGNOSTICS?Diagnostic messages are printed on the standard error and are"
"	classified by levels. The level determines if the diagnostic"
"	is printed, if it causes \bnmake\b to exit, and if it affects"
"	the \bnmake\b exit status. The levels are:]{"
"		[+<0?Debug message, enabled when the absolute value of"
"			\alevel\a is greater than or equal to the"
"			\b--debug\b level. Debug diagnostics are prefixed"
"			by \bdebug\b-\alevel\a\b:\b.]"
"		[+1?Warning message, disabled by \b--silent\b. Warning"
"			diagnostics are prefixed by \bwarning\a\b:\b]"
"		[+2?Non-fatal error message. Processing continues after"
"			the diagnostic, but the eventual \bnmake\b exit"
"			status will be non-zero.]"
"		[+>2?Fatal error message. \bnmake\b exits after the"
"			diagnostic (and internal cleanup) with exit"
"			status \alevel\a-2.]"
"}"
"[+SEE ALSO?\b3d\b(1), \bar\b(1), \bcc\b(1), \bcoshell\b(1), \bcpp\b(1),"
"	\bprobe\b(1), \bsh\b(1)]"
;

struct Oplist_s; typedef struct Oplist_s Oplist_t;

struct Oplist_s				/* linked option list		*/
{
	char*		option;		/* option value for set()	*/
	Oplist_t*	next;		/* next in list			*/
};

typedef struct Optstate_s		/* option state			*/
{
	Oplist_t*	hidden;		/* options hidden by cmd line	*/
	Oplist_t*	lasthidden;	/* tail of hidden		*/
	Oplist_t*	delayed;	/* delayed unknown options	*/
	Oplist_t*	lastdelayed;	/* tail of delayed		*/

	Option_t*	head;		/* head of external option list	*/
	Option_t*	tail;		/* tail of external option list	*/

	Hash_table_t*	table;

	Sfio_t*		usage;		/* generated optget() usage	*/
	int		usageindex;	/* next user index		*/
} Optstate_t;

static Optstate_t	opt;

static Option_t*
getoption(const char* name)
{
	register Option_t*	op;
	register int		c;

	if (!(op = (Option_t*)hashget(opt.table, name)) && (strchr(name, '-') || strchr(name, '_')))
	{
		while (c = *name++)
			if (c != '-' && c != '_')
				sfputc(internal.tmp, c);
		op = (Option_t*)hashget(opt.table, sfstruse(internal.tmp));
	}
	return op;
}

static void
putoption(register Option_t* op, int index)
{
	register char*	s;
	register int	c;
	char		buf[16];

	hashput(opt.table, op->name, (char*)op);
	if (strchr(op->name, '-') || strchr(op->name, '_'))
	{
		s = op->name;
		while (c = *s++)
			if (c != '-' && c != '_')
				sfputc(internal.tmp, c);
		hashput(opt.table, strdup(sfstruse(internal.tmp)), (char*)op);
	}
	if (op->flags & Of)
		sfsprintf(buf, sizeof(buf), "+%d", OPT(op->flags));
	else
	{
		buf[0] = '-';
		buf[1] = OPT(op->flags);
		buf[2] = 0;
	}
	hashput(opt.table, strdup(buf), (char*)op);
	if (index >= elementsof(options))
	{
		sfsprintf(buf, sizeof(buf), "-%d", index);
		hashput(opt.table, strdup(buf), (char*)op);
	}
}

/*
 * initialize the option hash table
 */

void
optinit(void)
{
	register int	i;

	opt.table = hashalloc(NiL, HASH_name, "options", 0);
	for (i = 0; i < elementsof(options); i++)
	{
		options[i].flags |= Om;
		switch (OPT(options[i].flags))
		{
		case OPT(OPT_debug):
			options[i].value = (char*)&error_info.trace;
			break;
		}
		putoption(&options[i], i);
		if (opt.tail)
			opt.tail->next = &options[i];
		else
			opt.head = &options[i];
		opt.tail = &options[i];
	}
}

/*
 * return option table entry given OPT_[a-z]+ flag
 * type==0 panics if not in table
 */

Option_t*
optflag(register int flag)
{
	register Option_t*	op;
	char			buf[8];

	if (flag & Of)
		sfsprintf(buf, sizeof(buf), "+%d", OPT(flag));
	else
	{
		buf[0] = '-';
		buf[1] = OPT(flag);
		buf[2] = 0;
	}
	if (!(op = getoption(buf)) && (flag & ~((1<<8)-1)))
		error(ERROR_PANIC, "%s: unknown option flag", buf);
	return op;
}

/*
 * call op->set with new value
 */

static void
setcall(register Option_t* op, int readonly)
{
	Rule_t*		r;
	char*		oset;
	int		oreadonly;
	char		buf[16];

	if (op->set && (r = getrule(op->set)))
	{
		oset = op->set;
		op->set = 0;
		oreadonly = state.readonly;
		state.readonly = readonly;
		switch (op->flags & (Ob|On|Os))
		{
		case Ob:
			call(r, *((unsigned char*)op->value) ? "1" : null);
			break;
		case On:
			sfsprintf(buf, sizeof(buf), "%d", *((int*)op->value));
			call(r, buf);
			break;
		case Os:
			call(r, *((char**)op->value));
			break;
		}
		state.readonly = oreadonly;
		op->set = oset;
	}
}

/*
 * copy a declare() string entry s to sp
 */

static void
declarestr(register Sfio_t* sp, register const char* s)
{
	register int	c;

	if (s && *s)
		while (c = *s++)
		{
			if (c == OPT_SEP)
				sfputc(sp, c);
			sfputc(sp, c);
		}
	else
		sfputc(sp, OPT_NON);
	sfputc(sp, OPT_SEP);
}

/*
 * generate external option declaration
 */

static void
declare(Sfio_t* sp, register Option_t* op)
{
	if (!(op->flags & Of))
		sfputc(internal.tmp, OPT(op->flags));
	sfputc(internal.tmp, OPT_SEP);
	declarestr(internal.tmp, op->name);
	if (op->flags & Oa)
		sfputc(internal.tmp, 'a');
	if (op->flags & Ob)
		sfputc(internal.tmp, 'b');
	if (op->flags & On)
		sfputc(internal.tmp, 'n');
	if (op->flags & Oo)
		sfputc(internal.tmp, 'o');
	if (op->flags & Op)
		sfputc(internal.tmp, 'p');
	if (op->flags & Os)
		sfputc(internal.tmp, 's');
	if (op->flags & Ov)
		sfputc(internal.tmp, 'v');
	if (op->flags & Ox)
		sfputc(internal.tmp, 'x');
	sfputc(internal.tmp, OPT_SEP);
	declarestr(internal.tmp, op->set);
	declarestr(internal.tmp, op->description);
	declarestr(internal.tmp, op->arg);
	shquote(sp, fmtesc(sfstruse(internal.tmp)));
}

/*
 * generate optget() usage for op
 */

static void
genusage(register Option_t* op, int index, int last)
{
	long	pos;

	if (op)
	{
		sfputc(opt.usage, '[');
		if (!(op->flags & Of))
		{
			sfputc(opt.usage, OPT(op->flags));
			if (op->flags & Oo)
				sfputc(opt.usage, '!');
			sfputc(opt.usage, '=');
		}
		sfprintf(opt.usage, "%d:%s?%s]", index + OPT_OFFSET, op->name, op->description);
		if (op->arg)
		{
			if (op->flags & On)
				sfputc(opt.usage, '#');
			else
				sfputc(opt.usage, ':');
			if (op->flags & Ov)
				sfputc(opt.usage, '?');
			if (*op->arg != '[')
				sfputc(opt.usage, '[');
			sfputr(opt.usage, op->arg, -1);
			if (*op->arg != '[')
				sfputc(opt.usage, ']');
		}
		sfputc(opt.usage, '\n');
	}
	pos = sfstrtell(opt.usage);
	if (last)
		sfprintf(opt.usage, usage2, version);
	else
		sfputc(opt.usage, 0);
	sfstrseek(opt.usage, pos, SEEK_SET);
}

/*
 * mam output discipline to parameterize local paths
 */

static ssize_t
mamwrite(Sfio_t* fp, const void* buf, size_t n, Sfdisc_t* dp)
{
	char*		s;
	size_t		z;

	static char*	tmp;
	static int	siz;

	z = n;
	if (n > 1 && ((char*)buf)[n-1] == '\n')
	{
		if (n >= siz)
		{
			siz = roundof(n + 1, 1024);
			tmp = newof(tmp, char, siz, 0);
		}
		memcpy(tmp, buf, n);
		tmp[n-1] = 0;
		if (s = call(makerule(external.mamaction), tmp))
		{
			z = strlen(s);
			if (z >= siz)
			{
				siz = roundof(z + 1, 1024);
				tmp = newof(tmp, char, siz, 0);
			}
			memcpy(tmp, s, z);
			tmp[z++] = '\n';
			buf = (const char*)tmp;
		}
	}
	return sfwr(fp, buf, z, dp) == z ? n : -1;
}

/*
 * make sure the current time is > the start time
 * to differentiate strtime() "recent" vs. "current"
 * if the clock or filesystem doesn't support subsecond
 * granularity then we sleep until the next integral
 * second ticks off
 *
 * the filesystem checks assume that file time stamp
 * subseconds==0 rarely happens by chance -- the penalty
 * for a wrong guess is slightly slower but still correct
 * regression tests
 *
 * this also assumes that regression tests run faster than
 * the disk inode flush frequency on systems where the
 * cache time resolution is higher than the disk
 */

static void
regressinit(const char* type)
{
	Stat_t	st;
	Time_t	t;
	int	i;

	error_info.version = 0;
	if (*type == 's')
	{
		t = CURTIME;
		if (i = tmxnsec(t) && !stat(".", &st) && !tmxnsec(tmxgetatime(&st)) && !tmxnsec(tmxgetmtime(&st)))
			state.start = tmxsns(tmxsec(t)+1, 0);
		while (state.start >= (t = CURTIME))
			tmsleep(0L, 100000000L);
		if (!i)
			state.start = t;
	}
}

/*
 * return option name and details
 */

static char*
showop(register Option_t* op)
{
	sfprintf(internal.tmp, "%s,", op->name);
	sfprintf(internal.tmp, (op->flags & Of) ? "%03o," : "'%c',", op->flags & ((1<<8) - 1));
	if (op->flags & Oa)
		sfprintf(internal.tmp, "|a");
	if (op->flags & Ob)
		sfprintf(internal.tmp, "|b");
	if (op->flags & Of)
		sfprintf(internal.tmp, "|f");
	if (op->flags & Oi)
		sfprintf(internal.tmp, "|i");
	if (op->flags & On)
		sfprintf(internal.tmp, "|n");
	if (op->flags & Oo)
		sfprintf(internal.tmp, "|o");
	if (op->flags & Op)
		sfprintf(internal.tmp, "|p");
	if (op->flags & Os)
		sfprintf(internal.tmp, "|s");
	if (op->flags & Ov)
		sfprintf(internal.tmp, "|v");
	if (op->flags & Ox)
		sfprintf(internal.tmp, "|x");
	if (op->flags & OPT_COMPILE)
		sfprintf(internal.tmp, "|COMPILE");
	if (op->flags & OPT_DECLARE)
		sfprintf(internal.tmp, "|DECLARE");
	if (op->flags & OPT_DEFAULT)
		sfprintf(internal.tmp, "|DEFAULT");
	if (op->flags & OPT_EXTERNAL)
		sfprintf(internal.tmp, "|EXTERNAL");
	if (op->flags & OPT_READONLY)
		sfprintf(internal.tmp, "|READONLY");
	if (op->flags & OPT_SET)
		sfprintf(internal.tmp, "|SET");
	sfputc(internal.tmp, '|');
	return sfstruse(internal.tmp);
}

/*
 * return next option definition field
 */

static char*
field(char** p, int sep, int app, int lenient)
{
	register char*	s;
	register char*	t;
	char*		v;
	register int	c;

	if (!(c = *(s = *p)) || c == app)
		return 0;
	if (c == sep)
	{
		*p = s + 1;
		return 0;
	}
	v = t = s;
	for (;;)
	{
		if (!(c = *t++ = *s++))
		{
			s--;
			t--;
			break;
		}
		else if (c == sep)
		{
			if (lenient || !*s)
			{
				t--;
				s--;
				break;
			}
			if (*s == OPT_NON && (!*(s + 1) || *(s + 1) == sep))
			{
				t--;
				s++;
				break;
			}
			if (*s != c)
			{
				t--;
				s--;
				break;
			}
			s++;
		}
	}
	if (*s)
		s++;
	*p = s;
	if (*t)
		*t = 0;
	if (!*v || lenient && *v == OPT_NON && !*(v + 1))
		return 0;
	return v;
}

/*
 * set an option given its pointer
 */

static void
setop(register Option_t* op, register int n, char* s, int type)
{
	char*		t;
	Rule_t*		r;
	int		readonly;

	readonly = state.readonly;
	if (OPT(op->flags) != OPT(OPT_option))
	{
		if (type == ':' && (op->flags & OPT_SET))
			return;
		if (readonly)
			op->flags |= OPT_READONLY;
		else if (!state.user && (op->flags & OPT_READONLY))
		{
			Oplist_t*	x;

			/*
			 * save for listops(*,'@')
			 */

			sfprintf(internal.tmp, "%s", op->name);
			if (type == ':' && !(op->flags & OPT_SET))
				sfputc(internal.tmp, ':');
			if (!s)
				sfprintf(internal.tmp, "=%d", n);
			else if (!strchr(s, ' ') && !strchr(s, '\t'))
				sfprintf(internal.tmp, "=%s", s);
			else if (!strchr(s, '\''))
				sfprintf(internal.tmp, "='%s'", s);
			else
				sfprintf(internal.tmp, "=\"%s\"", s);
			n = sfstrtell(internal.tmp) + 1;
			s = sfstruse(internal.tmp);
			x = newof(0, Oplist_t, 1, n);
			x->option = strcpy((char*)(x + 1), s);
			if (opt.lasthidden)
				opt.lasthidden = opt.lasthidden->next = x;
			else
				opt.hidden = opt.lasthidden = x;
			return;
		}
		if (error_info.trace <= -3)
			error(-3, "option(%s,%d,\"%s\")", showop(op), n,  s ? s : null);
	}
	if (type != ':')
	{
		op->flags |= OPT_SET;
		op->flags &= ~OPT_DEFAULT;
	}
	else if (!(op->flags & OPT_SET))
	{
		op->flags |= OPT_DEFAULT;
		if (!state.loading)
			op->flags |= OPT_COMPILE;
	}
	if (state.reading)
		op->flags |= OPT_COMPILE;
	if (op->flags & Oi)
	{
		if (op->flags & On)
			n = -n;
		else
			n = !n;
	}
	else if (op->flags & Ob)
		n = n != 0;
	else if ((op->flags & (Os|Ov)) == Os && n && !s)
		error(3, "-%c: option argument expected", OPT(op->flags));
	if (!n)
		s = 0;
	switch (OPT(op->flags))
	{
	case OPT(OPT_believe):
		if (state.compile < COMPILED)
			state.believe = n;
		else
			error(2, "%s: option must be set before %s", op->name, external.makeinit);
		return;
	case OPT(OPT_byname):
		if (s)
			set(s, 1, NiL);
		return;
	case OPT(OPT_corrupt):
		if (!s)
			s = "-";
		if ((*s == *(state.corrupt = "accept") || *s == '-') && (!*(s + 1) || !strcmp(s, state.corrupt)))
			;
		else if (*s == *(state.corrupt = "error") && (!*(s + 1) || !strcmp(s, state.corrupt)))
			state.corrupt = 0;
		else if (*s == *(state.corrupt = "ignore") && (!*(s + 1) || !strcmp(s, state.corrupt)))
			;
		else
		{
			state.corrupt = 0;
			error(2, "%s: invalid corrupt action", s);
		}
		return;
	case OPT(OPT_global):
		if (!s)
		{
			if (n)
			{
				state.pushed = 1;
				state.push_global = state.global;
				state.global = 1;
				state.push_user = state.user;
				state.user = 0;
			}
			else if (state.pushed)
			{
				state.pushed = 0;
				state.global = state.push_global;
				state.user = state.push_user;
			}
			return;
		}
		break;
	case OPT(OPT_include):
		addprereq(catrule(internal.source->name, external.source, NiL, 1), makerule(s), PREREQ_APPEND);
		if (!(op->flags & OPT_READONLY))
			break;
		/*FALLTHROUGH*/
	case OPT(OPT_define):
	case OPT(OPT_undef):
		if (s)
		{
			sfprintf(internal.tmp, "-%c%s", OPT(op->flags), s);
			if (!(r = getrule(sfstruse(internal.tmp))))
				r = makerule(NiL);
			addprereq(internal.preprocess, r, PREREQ_APPEND);
		}
		return;
	case OPT(OPT_preprocess):
		if (!state.preprocess)
		{
			state.preprocess = 1;
			if (!state.compatibility)
				error(1, "makefile preprocessing is obsolete -- use make statements");
		}
		return;
	case OPT(OPT_errorid):
		if (s && *s)
		{
			if (state.errorid)
				sfprintf(internal.tmp, "%s/", state.errorid);
			sfprintf(internal.tmp, "%s", s);
			state.errorid = strdup(sfstruse(internal.tmp));
			sfprintf(internal.tmp, "%s [%s]", idname, state.errorid);
			error_info.id = strdup(sfstruse(internal.tmp));
		}
		else
		{
			op->flags &= ~OPT_SET;
			error_info.id = idname;
		}
		return;
	case OPT(OPT_jobs):
		if (n >= MAXJOBS)
			n = MAXJOBS - 1;
		if (n < 1)
			n = 0;
		state.jobs = n;
		return;
	case OPT(OPT_mam):
		if (t = s)
		{
			if (t[0] == 'n' && t[1] == 'o')
				t += 2;
			if (streq(t, "hold"))
			{
				if (t == s)
					state.mam.hold++;
				else
					state.mam.hold--;
				return;
			}
		}
		if (state.mam.label != null)
		{
			if (state.mam.label)
				free(state.mam.label);
			state.mam.label = null;
		}
		if (state.mam.options)
		{
			free(state.mam.options);
			state.mam.options = 0;
		}
		if (state.mam.root)
		{
			free(state.mam.root);
			state.mam.root = 0;
		}
		if (state.mam.out)
		{
			if (state.mam.out != sfstdout && state.mam.out != sfstderr)
				sfclose(state.mam.out);
			state.mam.out = 0;
		}
		state.mam.dontcare = state.mam.dynamic = state.mam.regress = state.mam.statix = state.mam.parent = 0;
		state.mam.port = 1;
		if (s)
		{
			char*	o;
			char*	u;
			Sfio_t*	tmp;

			tmp = sfstropen();
			sfputr(tmp, s, 0);
			s = sfstruse(tmp);
			if (t = strchr(s, ':'))
				*t++ = 0;
			if (o = strchr(s, ','))
				*o++ = 0;
			if (*s == *(state.mam.type = "dynamic") && (!*(s + 1) || !strcmp(s, state.mam.type)))
				state.mam.dynamic = 1;
			else if (*s == *(state.mam.type = "regress") && (!*(s + 1) || !strcmp(s, state.mam.type)))
			{
				state.regress = "sync";
				state.mam.regress = 1;
				state.silent = 1;
				if (!table.regress)
					table.regress = hashalloc(table.rule, HASH_name, "regress-paths", 0);
				regressinit(state.regress);
			}
			else if (*s == *(state.mam.type = "static") && (!*(s + 1) || !strcmp(s, state.mam.type)))
				state.mam.statix = 1;
			else
				error(3, "%s: invalid mam type: {dynamic,regress,static} expected", s);
			while (s = o)
			{
				if (o = strchr(s, ','))
					*o++ = 0;
				if (*s == 'n' && *(s + 1) == 'o')
				{
					s += 2;
					n = 0;
				}
				else
					n = 1;
				if (*s == *(u = "dontcare") && (!*(s + 1) || !strcmp(s, u)))
					state.mam.dontcare = n;
				else if (*s == *(u = "port") && (!*(s + 1) || !strcmp(s, u)))
					state.mam.port = n;
				else
					error(3, "%s: invalid mam option: [no]{dontcare,port} expected", s);
			}
			if (t)
			{
				s = t;
				if (t = strchr(s, ':'))
				{
					*t++ = 0;
					if (isdigit(*t))
					{
						while (isdigit(*t))
							state.mam.parent = state.mam.parent * 10 + *t++ - '0';
						if (!state.mam.regress)
						{
							sfprintf(internal.tmp, "%05d ", state.pid);
							state.mam.label = strdup(sfstruse(internal.tmp));
						}
					}
					if (*t)
					{
						if (*t != ':')
							error(3, "%s: mam label expected", t);
						t++;
					}
					if (!*t)
						t = 0;
				}
			}
			else s = null;
			if (!*s || streq(s, "-") || streq(s, "/dev/fd/1") || streq(s, "/dev/stdout"))
			{
				s = "/dev/stdout";
				state.mam.out = sfstdout;
			}
			else if (streq(s, "/dev/fd/2") || streq(s, "/dev/stderr"))
				state.mam.out = sfstderr;
			else if (!(state.mam.out = sfopen(NiL, s, state.mam.parent ? "ae" : "we")))
				error(ERROR_SYSTEM|3, "%s: cannot write mam output file", s);
			sfset(state.mam.out, SF_LINE, 1);
			state.mam.disc.writef = mamwrite;
			if (sfdisc(state.mam.out, &state.mam.disc) != &state.mam.disc)
				error(3, "%s: cannot push mam output discipline", s);
			sfprintf(internal.tmp, "%s", state.mam.type);
			if (state.mam.dontcare)
				sfprintf(internal.tmp, ",dontcare");
			sfprintf(internal.tmp, ",%sport", state.mam.port ? null : "no");
			sfprintf(internal.tmp, ":");
			if (*s != '/')
				sfprintf(internal.tmp, "%s/", internal.pwd);
			sfprintf(internal.tmp, "%s:%d", s, state.pid);
			if (t)
			{
				sfprintf(internal.nam, "$(\"%s\":P=A)", t);
				expand(tmp, sfstruse(internal.nam));
				state.mam.root = strdup(sfstruse(tmp));
				state.mam.rootlen = strlen(state.mam.root);
				sfprintf(internal.tmp, ":%s", state.mam.root);
			}
			state.mam.options = strdup(sfstruse(internal.tmp));
			sfstrclose(tmp);
		}
		return;
	case OPT(OPT_option):
		s = strdup(s);
		while (*s)
		{
			char*		name;
			char*		func;
			char*		desc;
			char*		args;
			Option_t*	nop;
			int		app;
			int		sep;
			char		buf[16];

			sep = *s;
			if (isalpha(sep))
			{
				n = sep|Ob;
				sep = *++s;
			}
			else
			{
				n = '?'|Of|Ob;
				if (sep == OPT_NON)
					sep = *++s;
			}
			app = (sep == ':') ? ';' : ':';
			s++;
			if (!(name = field(&s, sep, app, 1)))
			{
				if (n & Of)
					error(2, "option flag and name omitted");
				else
					error(2, "-%c: option name omitted", OPT(n));
				break;
			}
			if (t = field(&s, sep, app, 1))
			{
				for (;;)
				{
					switch (*t++)
					{
					case 0:
						break;
					case 'a':
						n |= Oa;
						continue;
					case 'b':
						n &= ~(On|Os);
						n |= Ob;
						continue;
					case 'n':
						n &= ~(Ob|Os);
						n |= On;
						continue;
					case 'o':
						n |= Oo;
						continue;
					case 'p':
						n |= Op;
						continue;
					case 's':
						n &= ~(Ob|On);
						n |= Os;
						continue;
					case 'v':
						n |= Ov;
						continue;
					case 'x':
						n |= Ox;
						continue;
					/* no complaints here for future extensions */
					}
					break;
				}
			}
			func = field(&s, sep, app, 1);
			desc = field(&s, sep, app, 0);
			args = field(&s, sep, app, 0);
			if (!(n & Of))
			{
				for (nop = &options[0]; nop < &options[elementsof(options)]; nop++)
					if (OPT(nop->flags) == OPT(n))
						break;
				if (nop < &options[elementsof(options)])
				{
					error(2, "--%s: -%c conflicts with --%s", s, OPT(n), nop->name);
					return;
				}
				buf[0] = '-';
				buf[1] = OPT(n);
				buf[2] = 0;
				nop = getoption(buf);
			}
			else
				nop = 0;
			if (nop)
			{
				if (n & On)
					*((int*)nop->value) = *((unsigned char*)nop->value);
				else if (n & Os)
					*((char**)nop->value) = 0;
				nop->flags = n;
			set_insert:
				nop->name = name;
				nop->set = func;
				if ((n & (Os|Oa)) == (Os|Oa) && !op->value)
					*((Rule_t**)nop->value) = catrule(".OPTION.", nop->name, ".LIST.", 1);
				if (!desc)
					desc = "option.";
				if (*desc != '(')
				{
					sfputc(internal.tmp, '(');
					if ((t = parsefile()) && *t)
						edit(internal.tmp, t, DELETE, KEEP, DELETE);
					else
						sfputr(internal.tmp, state.base ? "rules" : state.global ?  "global" : "user", -1);
					sfprintf(internal.tmp, ") %s", desc);
					desc = strdup(sfstruse(internal.tmp));
				}
				nop->description = desc;
				if (!args && (nop->flags & (On|Os)))
					args = (nop->flags & On) ? "number" : "string";
				if (args && (nop->flags & Ob))
				{
					nop->flags &= ~Ob;
					nop->flags |= Os;
				}
				nop->arg = args;
				putoption(nop, opt.usageindex);
				genusage(nop, opt.usageindex++, 1);
				if (nop->set && (nop->flags & OPT_SET))
					readonly = 1;
			}
			else if (!(nop = getoption(name)))
			{
				nop = newof(0, Option_t, 1, sizeof(char*));
				nop->value = (char*)(nop + 1);
				nop->flags = n|OPT_EXTERNAL;
				if (!state.loading)
					nop->flags |= OPT_DECLARE;
				if (opt.tail)
					opt.tail->next = nop;
				else
					opt.head = nop;
				opt.tail = nop;
				goto set_insert;
			}
			else if (!(nop->flags & OPT_EXTERNAL))
				error(1, "--%s is an internal option", nop->name);
			else
			{
				if (nop->flags & Of)
				{
					if ((n & (Ob|On|Os)) != (nop->flags & (Ob|On|Os)))
					{
						if (n & Os)
							*((char**)nop->value) = 0;
						else if ((n & On) && (nop->flags & Ob))
							*((int*)nop->value) = *((unsigned char*)nop->value);
						else if ((n & Ob) && (nop->flags & On))
							*((unsigned char*)nop->value) = *((int*)nop->value) != 0;
					}
					nop->flags = n;
				}
				else if (OPT(nop->flags) != OPT(n))
					error(1, "--%s: option flag -%c conflicts with -%c", nop->name, OPT(n), OPT(nop->flags));
				if ((nop->flags & (Ob|On|Os)) != (n & (Ob|On|Os)))
					error(1, "--%s: option is %s", nop->name, (nop->flags & Ob) ? "boolean" : (nop->flags & On) ? "numeric" : "string valued");
			}

			/*
			 * skip the remaining fields for future extensions
			 * and consume the append char if specified
			 */

			while (field(&s, sep, app, 0));
			if (*s == app)
				s++;
		}
		optcheck(0);
		return;
	case OPT(OPT_never):
		if (state.never = n)
			state.exec = 0;
		return;
	case OPT(OPT_regress):
		if (n)
		{
			if (!s)
				s = "-";
			if ((*s == *(state.regress = "message") || *s == '-') && (!*(s + 1) || !strcmp(s, state.regress)))
				;
			else if (*s == *(state.regress = "sync") && (!*(s + 1) || !strcmp(s, state.regress)))
				;
			else
			{
				state.regress = 0;
				error(2, "%s: invalid regress action", s);
			}
			if (state.regress)
				regressinit(state.regress);
		}
		else
			state.regress = 0;
		return;
	case OPT(OPT_reread):
		if (state.reread = n)
			state.forceread = 1;
		return;
	case OPT(OPT_silent):
		if (state.silent = n)
		{
			if (!error_info.trace)
				error_info.trace = 2;
		}
		else
			if (error_info.trace > 0)
				error_info.trace = 0;
		return;
	case OPT(OPT_targetprefix):
		if (!n)
			s = 0;
		else if (s)
			s = strdup(s);
		if (state.targetprefix = s)
			for (;;)
			{
				switch (*s++)
				{
				case 0:
					break;
				case '%':
				case '*':
				case '?':
				case '&':
				case '"':
				case '\'':
				case '\\':
					error(3, "--%s: %s: value must not contain metarule or shell pattern characters", op->name, state.targetprefix);
					break;
				default:
					continue;
				}
				break;
			}
		return;
	case OPT(OPT_tolerance):
		if ((state.tolerance = n) > 60)
			error(1, "the time comparison tolerance should probably be less than a minute");
		return;
	case OPT(OPT_writeobject):
		if (!n)
			s = 0;
		else if (state.makefile)
		{
			error(1, "%s: object file name cannot change after %s read", op->name, state.makefile);
			return;
		}
		else if (!s)
			s = "-";
		else
			s = strdup(s);
		state.writeobject = s;
		return;
	case OPT(OPT_writestate):
		if (!n)
			s = 0;
		else if (!s)
			s = "-";
		else
			s = strdup(s);
		state.writestate = s;
		if (state.statefile)
		{
			free(state.statefile);
			state.statefile = 0;
		}
		return;
	}
	if (op->value)
	{
		if (op->flags & Ob)
		{
			switch (type)
			{
			case '^':
				*((unsigned char*)op->value) ^= n;
				break;
			default:
				*((unsigned char*)op->value) = n != 0;
				break;
			}
		}
		else if (op->flags & On)
		{
			switch (type)
			{
			case '+':
				*((int*)op->value) += n;
				break;
			case '-':
				*((int*)op->value) -= n;
				break;
			case '|':
				*((int*)op->value) |= n;
				break;
			case '&':
				*((int*)op->value) &= n;
				break;
			case '^':
				*((int*)op->value) ^= n;
				break;
			default:
				*((int*)op->value) = n;
				break;
			}
		}
		else if (op->flags & Os)
		{
			if (op->flags & Oa)
			{
				if (s)
				{
					/*
					 * s is a ':' list
					 */

					for (;;)
					{
						if (t = strchr(s, ':'))
							*t = 0;
						addprereq((*(Rule_t**)op->value), makerule(s), PREREQ_APPEND);
						if (!t)
							break;
						*t++ = ':';
						s = t;
					}
				}
				else
				{
					freelist((*(Rule_t**)op->value)->prereqs);
					(*(Rule_t**)op->value)->prereqs = 0;
				}
			}
			else
				*((char**)op->value) = s ? strdup(s) : 0;
		}
	}
	if (op->set)
		setcall(op, readonly);
	if ((op->flags & Op) && !state.reading && !state.user)
	{
		if ((op->flags & Os) && s)
		{
			sfprintf(internal.tmp, "--%s=%s", op->name, s);
			if (!(r = getrule(sfstruse(internal.tmp))))
				r = makerule(NiL);
			addprereq(internal.preprocess, r, PREREQ_APPEND);
		}
		else if ((op->flags & (Ob|On)) && n)
		{
			if (!n)
				sfprintf(internal.tmp, "--no%s", op->name);
			else if (n != 1)
				sfprintf(internal.tmp, "--%s=%d", op->name, n);
			else
				sfprintf(internal.tmp, "--%s", op->name);
			if (!(r = getrule(sfstruse(internal.tmp))))
				r = makerule(NiL);
			addprereq(internal.preprocess, r, PREREQ_APPEND);
		}
	}
}

/*
 * generate a single option setting in sp given the option pointer
 * setting:
 *	 0  only the value is generated, "" if option not set
 *	'+' only the value is generated, "" if Os option not set, 0 for Ob|Oi not set
 *	'-' option name and value suitable for set()
 *	'?' 1 if OPT_SET, "" otherwise
 *	'#' table attributes with the current value
 * flag!=0 if relative to option flag rather than option name
 */

static void
genop(register Sfio_t* sp, register Option_t* op, int setting, int flag)
{
	register long		n;
	char*			v;
	List_t*			p;

	switch (setting)
	{
	case '?':
		if (op->flags & OPT_SET)
			sfprintf(sp, "1");
		return;
	case '#':
		sfprintf(sp, "%s=", showop(op));
		break;
	case 0:
		break;
	default:
		flag &= ~Oi;
		break;
	}
	switch (op->flags & (Ob|On|Os))
	{
	case Ob:
		if (op->value)
			n = *((unsigned char*)op->value);
		else
			n = 0;
		if ((op->flags & Oi) ^ (flag & Oi))
			n = !n;
		if ((flag & Of) && (op->flags & Oo))
			n = !n;
		switch (setting)
		{
		case 0:
			if (!n)
				return;
			/*FALLTHROUGH*/
		case '+':
		case '#':
			break;
		default:
			if (op->flags & OPT_DEFAULT)
				sfprintf(sp, "--%s:=", op->name);
			else if (n)
			{
				sfprintf(sp, "--%s", op->name);
				return;
			}
			else
			{
				sfprintf(sp, "--no%s", op->name);
				return;
			}
			break;
		}
		sfputc(sp, n ? '1' : '0');
		break;
	case On:
		if (op->value)
			n = *((int*)op->value);
		else
			n = 0;
		if (op->flags & Oi)
			n = -n;
		if (flag & Oi)
			n = !n;
		switch (setting)
		{
		case 0:
			if (!n)
				return;
			/*FALLTHROUGH*/
		case '+':
		case '#':
			break;
		default:
			if (op->flags & OPT_DEFAULT)
				sfprintf(sp, "--%s:=", op->name);
			else if (n)
				sfprintf(sp, "--%s=", op->name);
			else
			{
				sfprintf(sp, "--no%s", op->name);
				return;
			}
			break;
		}
		sfprintf(sp, (op->flags & Oa) ? "0x%08lx" : "%ld", n);
		break;
	case Os:
		if ((op->flags & Oa) && op->value && !(flag & Oi))
		{
			p = (*(Rule_t**)op->value)->prereqs;
			switch (setting)
			{
			case 0:
				if (!p)
					return;
				/*FALLTHROUGH*/
			case '+':
			case '#':
				break;
			default:
				if (op->flags & OPT_DEFAULT)
					sfprintf(sp, "--%s:=", op->name);
				else if (p)
					sfprintf(sp, "--%s=", op->name);
				else
				{
					sfprintf(sp, "--no%s", op->name);
					return;
				}
				break;
			}
			if (p)
				for (;;)
				{
					shquote(sp, fmtesc(p->rule->name));
					if (!(p = p->next))
						break;
					sfputc(sp, ':');
				}
		}
		else
		{
			if (op->value && !(flag & Oi))
				v = *((char**)op->value);
			else
				v = 0;
			switch (setting)
			{
			case 0:
			case '#':
				if (!v)
					return;
				setting = 0;
				break;
			case '+':
				break;
			default:
				if (op->flags & OPT_DEFAULT)
					sfprintf(sp, "--%s:=", op->name);
				else if (v)
					sfprintf(sp, "--%s=", op->name);
				else
				{
					sfprintf(sp, "--no%s", op->name);
					return;
				}
				break;
			}
			if (v)
			{
				if (setting)
					shquote(sp, fmtesc(v));
				else
					sfputr(sp, v, -1);
			}
		}
		break;
#if DEBUG
	default:
		error(PANIC, "%s: option has%s%s%s", op->name, (op->flags & Ob) ? " Ob" : null, (op->flags & On) ? " On" : null, (op->flags & Os) ? " Os" : null);
		break;
#endif
	}
}

/*
 * check and set delayed options
 * this gives base, global and local makefiles a chance to
 * define the options via --option=definition
 */

void
optcheck(int must)
{
	Oplist_t*	x;
	int		errors;

	if (must)
	{
		errors = error_info.errors;
		while (x = opt.delayed)
		{
			opt.delayed = x->next;
			if (*x->option)
				set(x->option, 1, NiL);
			free(x);
		}
		if (error_info.errors != errors)
			finish(2);
	}
	else
		for (x = opt.delayed; x; x = x->next)
			if (!set(x->option, 0, NiL))
				*x->option = 0;
}

/*
 * generate option setting list in sp suitable for set()
 * setting:
 *       0  non-Ox OPT_SET options
 *      '+' non-Om non-Ox OPT_SET options
 *	'-' all
 *      '?' non-Ox OPT_DEFAULT
 *	'#' table attributes with the current value
 *	'@' internal object file dump
 */

void
listops(register Sfio_t* sp, int setting)
{
	register Option_t*	op;
	register Oplist_t*	x;
	int			sc;
	int			sep;
	long			mask;
	long			test;
	long			clear;

	sep = 0;
	sc = ' ';
	clear = 0;
	switch (setting)
	{
	case '-':
		mask = 0;
		test = 0;
		break;
	case '+':
		setting = '-';
		mask = Om|Ox|OPT_SET;
		test = Om|OPT_SET;
		break;
	case '?':
		setting = '-';
		mask = Ox|OPT_DEFAULT;
		test = OPT_DEFAULT;
		break;
	case '#':
		sc = '\n';
		mask = 0;
		test = 0;
		break;
	case '@':
		setting = '-';
		mask = Ox|OPT_COMPILE;
		test = OPT_COMPILE;
		clear = ~OPT_COMPILE;
		for (op = opt.head; op; op = op->next)
			if (op->flags & OPT_DECLARE)
			{
				op->flags &= ~OPT_DECLARE;
				if (sep)
					sfputc(sp, ':');
				else
				{
					sep = 1;
					sfprintf(sp, "--%s=", optflag(OPT_option)->name);
				}
				declare(sp, op);
			}
		while (x = opt.hidden)
		{
			opt.hidden = x->next;
			if (sep)
				sfputc(sp, sc);
			else
				sep = 1;
			sfputr(sp, x->option, -1);
			free(x);
		}
		break;
	default:
		setting = '-';
		mask = Ox|OPT_SET;
		test = OPT_SET;
		break;
	}
	for (op = opt.head; op; op = op->next)
		if ((op->flags & mask) == test)
		{
			if (sep)
				sfputc(sp, sc);
			else
				sep = 1;
			genop(sp, op, setting, 0);
			if (clear)
				op->flags &= clear;
		}
}

/*
 * generate a single option setting in sp given the option name
 * setting passed to genop()
 * end of s is returned
 */

void
getop(register Sfio_t* sp, char* name, int setting)
{
	register Option_t*	op;
	int			flag;

	if ((op = getoption(name)) && !(flag = 0) || name[0] == 'n' && name[1] == 'o' && (op = getoption(&name[2])) && (flag = Oi) || name[0] && !name[1] && (op = optflag(name[0])) && (flag = Of))
		genop(sp, op, setting, flag);
}

/*
 * set an option by its optget()/optstr() index
 */

static void
optset(int i, char* v, Sfio_t* scope)
{
	register char*	s;
	int		n;
	Option_t*	op;
	Oplist_t*	x;
	char		buf[16];

	if (i > 0)
	{
		if (state.readonly && !state.interpreter)
		{
			if (strchr(v, ' ') || strchr(v, '\t'))
			{
				while (*v && *v != '=')
					sfputc(internal.tmp, *v++);
				if (*v)
				{
					sfputc(internal.tmp, *v++);
					if (!strchr(v, '\"'))
						sfprintf(internal.tmp, "\"%s\"", v);
					else
						sfprintf(internal.tmp, "'%s'", v);
				}
				n = sfstrtell(internal.tmp);
				v = sfstruse(internal.tmp);
			}
			else
				n = strlen(v);
			x = newof(0, Oplist_t, 1, n + 1);
			x->option = strcpy((char*)(x + 1), v);
			if (opt.lastdelayed)
				opt.lastdelayed = opt.lastdelayed->next = x;
			else
				opt.delayed = opt.lastdelayed = x;
		}
		else
			error((i == '?' && opt_info.option[0] == '-' && opt_info.option[1] != '?') ? (ERROR_USAGE|(state.interpreter ? 2 : 4)) : 2, "%s", opt_info.arg);
	}
	else
	{
		if ((i = -OPT_OFFSET - i) < elementsof(options))
			op = &options[i];
		else
		{
			sfsprintf(buf, sizeof(buf), "-%d", i);
			op = getoption(buf);
		}
		n = (op->flags & On) ? (opt_info.arg ? opt_info.num : 0) : (op->flags & Ob) ? (opt_info.num != 0) : (opt_info.num != 0) == !(op->flags & Oi);
		if (*opt_info.option == '+')
			n = !n;
		if ((s = opt_info.arg) && (!*s || !(op->flags & Os)))
			s = 0;
		if (scope)
			genop(scope, op, '-', 0);
		setop(op, n, s, opt_info.assignment);
	}
}

/*
 * set options by name
 */

int
set(char* s, int must, Sfio_t* scope)
{
	register int	i;
	int		r;
	int		oreadonly;
	Opt_t		info;

	r = 0;
	info = opt_info;
	while (i = optstr(s, sfstrbase(opt.usage)))
	{
		if (i > 0 && !must)
		{
			r = -1;
			break;
		}
		s += opt_info.offset;
		if (!must)
		{
			oreadonly = state.readonly;
			state.readonly = 1;
		}
		optset(i, opt_info.argv[1], scope);
		if (!must)
			state.readonly = oreadonly;
	}
	opt_info = info;
	return r;
}

/*
 * set command line options with optget(3)
 * options may appear in any position before --
 * read command line assignments
 * mark the command line scripts and targets
 * index of the first command line script or target is returned
 */

int
scanargs(int argc, char** argv, int* argf)
{
	register int	i;
	register char*	s;
	register int	c;
	int		args;
	int		done;
	char*		e;

	/*
	 * generate the optget() usage string from options[]
	 */

	if (!(opt.usage = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [usage]");
	sfprintf(opt.usage, usage1, version);
	for (i = 0; i < elementsof(options); i++)
		genusage(options + i, i, 0);
	genusage(NiL, 0, 1);
	opt.usageindex = i;
	args = 0;
	done = 0;
 again:
	while (i = optget(argv, sfstrbase(opt.usage)))
		optset(i, argv[opt_info.index - (opt_info.offset == 0)], NiL);
	if (!done && streq(argv[opt_info.index - 1], "--"))
		done = 1;
	for (i = opt_info.index; i < argc; i++)
	{
		s = argv[i];
		while (isspace(*s))
			s++;
		if (!done && (*s == '-' || *s == '+') && *(s + 1))
		{
			opt_info.index = i;
			opt_info.offset = 0;
			goto again;
		}
		if (*s)
		{
			for (e = s; c = *s; s++)
				if (c == ',')
				{
					s = null;
					break;
				}
				else if (istype(c, C_TERMINAL) && c != '+' && c != '&')
				{
					while (isspace(*s))
						s++;
					if (*s == '=' || *(s + 1) == '=')
					{
						argf[i] |= ARG_ASSIGN;
						state.reading = 1;
						parse(NiL, e, "command line assignment", NiL);
						state.reading = 0;
					}
					else
					{
						argf[i] |= ARG_SCRIPT;
						if (!args)
							args = i;
					}
					break;
				}
			if (!*s)
			{
				argf[i] |= ARG_TARGET;
				if (!args)
					args = i;
			}
		}
	}
	return error_info.errors ? -1 : args ? args : argc;
}

/*
 * please reboot your program to finish setup ...
 *
 * old!=0 execs external.old for backwards compatibility
 * otherwise re-exec forcing input files to be read
 */

void
punt(int old)
{
	register char*		s;
	register char**		av;
	int			i;
	List_t*			p;
	Oplist_t*		x;
	Sfio_t*			vec;

	if (state.reread > 1)
		error(PANIC, "makefile prerequisites cause unbounded make exec recursion");
	vec = sfstropen();
	if (old)
	{
		expand(internal.tmp, getval(external.old, VAL_PRIMARY));
		putptr(vec, strdup(sfstruse(internal.tmp)));

		/*
		 * this chunk must track external.old options
		 */

		sfputc(internal.tmp, '-');

		/*
		 * options with same flag and meaning
		 */

		if (error_info.trace < -3)
			sfputc(internal.tmp, 'd');
		if (state.ignore)
			sfputc(internal.tmp, 'i');
		if (state.keepgoing)
			sfputc(internal.tmp, 'k');
		if (!state.mam.options && !state.exec)
			sfputc(internal.tmp, 'n');
		if (state.silent)
			sfputc(internal.tmp, 's');
		if (state.touch)
			sfputc(internal.tmp, 't');

		/*
		 * options with different flag but same meaning
		 */

		if (state.vardump)
			sfputc(internal.tmp, 'p');
		if (!state.mam.options && state.force)
			sfputc(internal.tmp, 'u');

		/*
		 * options with different flag and meaning
		 * the external.old meaning prevails
		 */

		if (state.base)
			sfputc(internal.tmp, 'b');
		if (state.explain)
			sfputc(internal.tmp, 'e');
		if (state.ruledump)
			sfputc(internal.tmp, 'r');
		s = sfstruse(internal.tmp);
		if (s[1])
			putptr(vec, strdup(s));

		/*
		 * mam arguments -- assume oldmake knows mam
		 */

		if (state.mam.options)
		{
			sfputc(internal.tmp, '-');
			if (state.never)
				sfputc(internal.tmp, 'N');
			else if (!state.exec)
				sfputc(internal.tmp, 'n');
			if (state.force)
				sfputc(internal.tmp, 'F');
			sfputc(internal.tmp, 'M');
			sfputr(internal.tmp, state.mam.options, -1);
			putptr(vec, strdup(sfstruse(internal.tmp)));
		}

		/*
		 * makefile arguments
		 */

		if (!(p = internal.makefiles->prereqs))
		{
			putptr(vec, "-f");
			putptr(vec, state.makefile);
		}
		else for (; p; p = p->next)
		{
			putptr(vec, "-f");
			putptr(vec, p->rule->name);
		}

		/*
		 * variable assignment arguments
		 */

		for (i = 1; i < state.argc; i++)
			if (state.argf[i] & (ARG_ASSIGN|ARG_TARGET))
				putptr(vec, state.argv[i]);
		if (!state.silent)
		{
			/*
			 * echo the exec action external.old style
			 */

#if !__sun__ && !sun
			sfprintf(sfstderr, "\t");
#endif
			putptr(vec, 0);
			av = (char**)sfstrbase(vec);
			while (*av)
				sfprintf(sfstderr, "%s ", *av++);
			sfprintf(sfstderr, "\n");
		}
	}
	else
	{
		/*
		 * copy the original argv adding OPT_reread
		 * and possibly OPT_preprocess
		 */

		for (av = state.argv; *av; putptr(vec, *av++));
		sfprintf(internal.tmp, "--%s=%d", optflag(OPT_reread)->name, state.reread + 1);
		if (state.preprocess)
			sfprintf(internal.tmp, "--%s", optflag(OPT_preprocess)->name);
		putptr(vec, sfstruse(internal.tmp));
	}
	putptr(vec, 0);

	/*
	 * tidy up
	 */

	sfsync(sfstdout);
	sfsync(sfstderr);

	/*
	 * start fresh
	 */

	av = (char**)sfstrbase(vec);
	execvp(av[0], av);
	error(3, "cannot exec %s", av[0]);
}

/*
 * return 1 if name is an option
 */

int
isoption(const char* name)
{
	return getoption(name) != 0 || name[0] == 'n' && name[1] == 'o' && getoption(&name[2]) != 0;
}
