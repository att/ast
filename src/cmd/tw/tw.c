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
 * tw -- tree walk
 *
 * print [execute cmd on] path names in tree rooted at . [dir]
 * or on path names listed on stdin [-]
 */

#define SNAPSHOT_ID	"snapshot"
#define SNAPSHOT_PATH	"%(url)s"
#define SNAPSHOT_EASY	"%(ctime)..64u,%(perm)..64u,%(size)..64u,%(uid)..64u,%(gid)..64u"
#define SNAPSHOT_HARD	"%(md5sum)s"
#define SNAPSHOT_DELIM	"|"

static const char usage[] =
"[-?\n@(#)$Id: tw (AT&T Research) 2012-04-11 $\n]"
USAGE_LICENSE
"[+NAME?tw - file tree walk]"
"[+DESCRIPTION?\btw\b recursively descends the file tree rooted at the "
    "current directory and lists the pathname of each file found. If \acmd "
    "arg ...\a is specified then the pathnames are collected and appended to "
    "the end of the \aarg\alist and \acmd\a is executed by the equivalent of "
    "\bexecvp\b(2). \acmd\a will be executed 0 or more times, depending the "
    "number of generated pathname arguments.]"
"[+?If the last option is \b-\b and \b--fast\b was not specified then "
    "the pathnames are read, one per line, from the standard input, the "
    "\b--directory\b options are ignored, and the directory tree is not "
    "traversed.]"
"[+?\bgetconf PATH_RESOLVE\b determines how symbolic links are handled. "
    "This can be explicitly overridden by the \b--logical\b, "
    "\b--metaphysical\b, and \b--physical\b options below. \bPATH_RESOLVE\b "
    "can be one of:]"
    "{"
        "[+logical?Follow all symbolic links.]"
        "[+metaphysical?Follow command argument symbolic links, "
            "otherwise don't follow.]"
        "[+physical?Don't follow symbolic links.]"
    "}"
"[a:arg-list?The first \aarg\a named \astring\a is replaced by the "
    "current pathname list before \acmd\a is executed.]:[string]"
"[c:args|arg-count?\acmd\a is executed after \acount\a arguments are "
    "collected.]#[count]"
"[d:directory?The file tree traversal is rooted at \adir\a. Multiple "
    "\b--directory\b directories are traversed in order from left to right. "
    "If the last option was \b-\b then all \b--directory\b are "
    "ignored.]:[dir]"
"[e:expr?\aexpr\a defines expression functions that control tree "
    "traversal. Multiple \b--expr\b expressions are parsed in order from "
    "left to right. See EXPRESSIONS below for details.]:[expr]"
"[f:fast?Searches the \bfind\b(1) or \blocate\b(1) database for paths "
    "matching the \bksh\b(1) \apattern\a. See \bupdatedb\b(1) for details on "
    "this database. Any \b--expr\b expressions are applied to the matching "
    "paths.]:[pattern]"
"[i:ignore-errors?Ignore inaccessible files and directories.]"
"[I:ignore-case?Ignore case in pathname comparisons.]"
"[l:local?Do not descend into non-local filesystem directories.]"
"[m:intermediate?Before visiting a selected file select and visit "
    "intermediate directories leading to the file that have not already been "
    "selected.]"
"[n:notraverse?Evaluate the \bbegin\b, \bselect\b and \bend\b "
    "expressions but eliminate the tree traversal.]"
"[p:post?Visit each directory after its files have been processed. By "
    "default directories are visited pre-order.]"
"[q:query?Emit an interactive query for each visited path. An "
    "affirmative response accepts the path, a negative response rejects the "
    "path, and a quit response exits \atw\a.]"
"[r:recursive?Visit directories listed on the standard input.]"
"[S:separator?The input file list separator is set to the first "
    "character of \astring\a.]:[string]"
"[s:size|max-chars?Use at most \achars\a characters per command. The "
    "default is as large as possible.]#[chars]"
"[t:trace|verbose?Print the command line on the standard error before "
    "executing it.]"
"[x:error-exit?Exit \btw\b with the exit code of the first \acmd\a that "
    "returns an exit code greater than or equal to \acode\a. By default "
    "\acmd\a exit codes are ignored (mostly because of \bgrep\b(1).)]#[code]"
"[z:snapshot?Write a snapshot of the selected files to the standard "
    "output. For the first snapshot the standard input must either be empty "
    "or a single line containing delimiter separated output format fields, "
    "with the delimiter appearing as both the first and last character. The "
    "format is in the same style as \bls\b(1) and \bps\b(1) formats: "
    "%(\aidentifier\a)\aprintf-format\a, where \aidentifier\a is one of the "
    "file status identifiers described below, and \aprintf-format\a is a "
    "\bprintf\b(3) format specification. A default delimiter (\"" 
    SNAPSHOT_DELIM "\") and field values are assumed for an empty "
    "snapshot input file. The format fields are:]"
    "{"
        "[+" SNAPSHOT_ID "?The literal string \b" SNAPSHOT_ID "\b.]"
        "[+path-format?The current path format, default \"" SNAPSHOT_PATH "\".]"
        "[+easy-format?The \aeasy\a part of the snapshot file state, default \""
	    SNAPSHOT_EASY "\". This part is recomputed for each file.]"
        "[+hard-format?The \ahard\a part of the snapshot file state, default \""
	    SNAPSHOT_HARD "\". This part is computed only if the easy part "
	    "has changed.]"
        "[+change-status?This field, with both a leading and trailing delimiter, "
	    "is ignored on input and set to one of the following values for "
	    "files that have changed since the last snapshot:]"
	    "{"
	        "[+C?The file changed.]"
	        "[+D?The file was deleted.]"
		"[+N?The file is new.]"
	    "}"
    "}"
"[C:chop?Chop leading \b./\b from printed pathnames. This is implied by "
    "\b--logical\b.]"
"[E:file?Compile \b--expr\b expressions from \afile\a. Multiple "
    "\b--file\b options may be specified. See EXPRESSIONS below for "
    "details.]:[file]"
"[F:codes?Set the \blocate\b(1) fast find codes database "
    "\apath\a.]:[path]"
"[G:generate?Generate a \aformat\a \blocate\b(1) database of the visited "
    "files and directories. Exit status 1 means some files were not "
    "accessible but the database was properly generated; exit status 2 means "
    "that database was not generated. Format may be:]:[format]"
    "{"
        "[+dir?machine independent with directory trailing /.]"
        "[+old?old fast find]"
        "[+gnu?gnu \blocate\b(1)]"
        "[+type?machine independent with directory and mime types]"
    "}"
"[L:logical|follow?Follow symbolic links. The default is determined by "
    "\bgetconf PATH_RESOLVE\b.]"
"[H:metaphysical?Follow command argument symbolic links, otherwise don't "
    "follow. The default is determined by \bgetconf PATH_RESOLVE\b.]"
"[P:physical?Don't follow symbolic links. The default is determined by "
    "\bgetconf PATH_RESOLVE\b.]"
"[X:xdev|mount?Do not descend into directories in different filesystems "
    "than their parents.]"
"[D:debug?Set the debug trace \alevel\a; higher levels produce more "
    "output.]# [level]"
"\n"
"\n[ cmd [ arg ... ] ]\n"
"\n"
"[+EXPRESSIONS?Expressions are C style and operate on elements of the "
    "\bstat\b(2) struct with the leading \bst_\b omitted. A function "
    "expression is defined by one of:]"
    "{"
        "[+?function-name : statement-list]"
        "[+?type function-name() { statement-list }]"
    "}"
"[+?where \afunction-name\a is one of:]"
    "{"
        "[+begin?Evaluated before the traversal starts. The return value "
            "is ignored. The default is a no-op.]"
        "[+select?Evaluated as each file is visited. A 0 return value "
            "skips \baction\b for the file; otherwise \baction\b is "
            "evaluated. All files are selected by default. \bselect\b is "
            "assumed when \afunction-name\a: is omitted.]"
        "[+action?Evaluated for each select file. The return value is "
            "ignored. The default \baction\b list the file path name, with "
            "leading \b./\b stripped, one per line on the standard output.]"
        "[+end?Evaluated after the traversal completes. The return value "
            "is ignored.]"
        "[+sort?A pseudo-function: the statement list is a , separated "
            "list of identifiers used to sort the entries of each directory. "
            "If any identifier is preceded by \b!\b then the sort order is "
            "reversed. If any identifier is preceded by \b~\b then case is "
            "ignored.]"
    "}"
"[+?\astatement-list\a is a C style \bexpr\b(3) expression that "
    "supports: \bint\b \avar\a, ...; and \bfloat\b \avar\a, ...; "
    "declarations, \b(int)\b and \b(float)\b casts, \bif\b-\belse\b "
    "conditionals, \bfor\b and \bwhile\b loops, and \b{...}\b blocks. The "
    "trailing \b;\b in any expression list is optional. The expression value "
    "is the value of the last evaluated expression in \astatement-list\a. "
    "Numbers and comments follow C syntax. String operands must be quoted "
    "with either \b\"...\"\b or \b'...'\b. String comparisons \b==\b and "
    "\b!=\b treat the right hand operand as a \bksh\b(1) file match "
    "pattern.]"
"[+?The expressions operate on the current pathname file status that is "
    "provided by the following field identifiers, most of which are "
    "described under \bst_\b\afield\a in \bstat\b(2). In general, if a "
    "status identifier appears on the left hand side of a binary operator "
    "then the right hand side may be a string that is converted to an "
    "integral constant according to the identifier semantics.]"
    "{"
        "[+atime?access time; time/date strings are interpreted as "
            "\bdate\b(1) expressions]"
        "[+blocks?number of 1k blocks]"
        "[+checksum?equivalent to \bsum(\"tw\")]"
        "[+ctime?status change time]"
        "[+dev?file system device]"
        "[+fstype?file system type name; \bufs\b if it can't be "
            "determined]"
        "[+gid?owner group id; \agid\a strings are interpreted as group "
            "names]"
        "[+gidok?1 if \agid\a is a valid group id in the system "
            "database, 0 otherwise.]"
        "[+ino?inode/serial number]"
        "[+level?the depth of the file relative to the traversal root]"
        "[+local?an integer valued field associated with each active "
            "object in the traversal; This field may be assigned. The "
            "initial value is 0. Multiple \alocal\a elements may be declared "
            "by \bint local.\b\aelement1\a...;. In this case the \blocal\b "
            "field itself is not accessible.]"
        "[+md5sum?equivalent to \bsum(\"md5\")]"
        "[+mime?the file contents \afile\a(1) \b--mime\b type]"
        "[+mode?type and permission bits; the \bFMT\b constant may be "
            "used to mask mask the file type and permission bits; \bmode\b "
            "strings are interpreted as \bchmod\b(1) expressions]"
        "[+mtime?modify time]"
        "[+name?file name with directory prefix stripped]"
        "[+nlink?hard link count]"
        "[+path?full path name relative to the current active "
            "\b--directory\b]"
        "[+perm?the permission bits of \bmode\b]"
        "[+rdev?the major.minor device number if the file is a device]"
        "[+size?size in bytes]"
        "[+status?the \bfts\b(3) \bFTS_\b* or \bftwalk\b(3) \bFTW_\b* "
            "status. This field may be assigned:]"
            "{"
                "[+AGAIN?visit the file again]"
                "[+FOLLOW?if the file is a symbolic link then follow it]"
                "[+NOPOST?cancel any post order visit to this file]"
                "[+SKIP?do not consider this file or any subdirectories "
                    "if it is a directory]"
            "}"
        "[+sum(\"\amethod\a\")?file contents checksum using \amethod\a; "
            "see \bsum\b(1) \b--method\b for details.]"
        "[+symlink?the symbolic link text if the file is a symbolic "
            "link]"
        "[+type?the type bits of \bmode\b:]"
            "{"
                "[+BLK?block special]"
                "[+CHR?block special]"
                "[+DIR?directory]"
                "[+DOOR?door]"
                "[+FIFO?fifo]"
                "[+LNK?symbolic link]"
                "[+REG?regular]"
                "[+SOCK?unix domain socket]"
            "}"
        "[+uid?owner user id; \auid\a strings are interpreted as user "
            "names]"
        "[+uidok?1 if \auid\a is a valid user id in the system database, "
            "0 otherwise.]"
        "[+url?unprintable chars n path converted to %XX hex]"
        "[+visit?an integer variable associated with each unique object "
            "visited; Objects are identified using the \bdev\b and \bino\b "
            "status identifiers. This field may be assigned. The initial "
            "value is 0. Multiple \bvisit\b elements may be declared by "
            "\bint visit.\b \aelement\a...;. In this case the \bvisit\b "
            "field itself is not accessible.]"
    "}"
"[+?Status identifiers may be prefixed by 1 or more \bparent.\b "
    "references, to access ancestor directory information. The parent status "
    "information of a top level object is the same as the object except that "
    "\bname\b and \bpath\b are undefined. If a status identifier is "
    "immediately preceded by \b\"string\"\b. then string is a file pathname "
    "from which the status is taken.]"
"[+?The following \bexpr\b(3) functions are supported:]"
    "{"
        "[+exit(expr)?causes \atw\a to exit with the exit code \aexpr\a "
            "which defaults to 0 if omitted]"
        "[+printf(format[,arg...]])?print the arguments on the standard "
            "output using the \bprintf\b(3) specification \aformat\a.]"
        "[+eprintf(format[,arg...]])?print the arguments on the standard "
            "error using the \bprintf\b(3) specification \aformat\a.]"
        "[+query(format[,arg...]])?prompt with the \bprintf\b(3) message "
            "on the standard error an read an interactive response. An "
            "affirmative response returns 1, \bq\b or \bEOF\b causes \atw\a "
            "to to exit immediately, and any other input returns 0.]"
    "}"
"[+EXAMPLES]"
    "{"
        "[+tw?Lists the current directory tree.]"
        "[+tw chmod go-w?Turns off the group and other write permissions "
            "for all files in the current directory tree using a minimal "
            "amount of \bchmod\b(1) command execs.]"
        "[+tw -e \"uid != 'bozo' || (mode & 'go=w')\"?Lists all files in "
            "the current directory that don't belong to the user \bbozo\b or "
            "have group or other write permission.]"
        "[+tw -m -d / -e \"fstype == '/'.fstype && mtime > '/etc/backup.time'.mtime\"?"
            "Lists all files and intermediate directories on the same file "
            "system type as \b/\b that are newer than the file "
            "\b/etc/backup.time\b.]"
        "[+tw - chmod +x < commands?Executes \bchmod +x\b on the "
            "pathnames listed in the file \bcommands\b.]"
        "[+tw -e \"int count;?\baction: count++; printf('name=%s "
            "inode=%08ld\\\\n', name, ino); end: printf('%d file%s\\\\n', "
            "count, count==1 ? '' : 's');\"\b Lists the name and inode "
            "number of each file and also the total number of files.]"
        "[+tw -pP -e \"?\baction: if (visit++ == 0) { parent.local += "
            "local + blocks; if (type == DIR) printf('%d\\\\t%s\\\\n', local "
            "+ blocks, path); }\"\b Exercise for the reader.]"
    "}"
"[+EXIT STATUS]"
    "{"
        "[+0?All invocations of \acmd\a returned exit status 0.]"
        "[+1-125?A command line meeting the specified requirements could "
            "not be assembled, one or more of the invocations of \acmd\a "
            "returned non-0 exit status, or some other error occurred.]"
        "[+126?\acmd\a was found but could not be executed.]"
        "[+127?\acmd\a was not found.]"
    "}"
"[+ENVIRONMENT]"
    "{"
        "[+FINDCODES?Path name of the \blocate\b(1) database.]"
        "[+LOCATE_PATH?Alternate path name of \blocate\b(1) database.]"
    "}"
"[+FILES]"
    "{"
        "[+lib/find/find.codes?Default \blocate\b(1) database.]"
    "}"
"[+NOTES?In order to access the \bslocate\b(1) database the \btw\b "
    "executable must be setgid to the \bslocate\b group.]"
"[+SEE ALSO?\bfind\b(1), \bgetconf\b(1), \blocate\b(1), \bslocate\b(1), "
    "\bsum\b(1), \bupdatedb\b(1), \bxargs\b(1)]"
;

#include "tw.h"

#include <ctype.h>
#include <proc.h>
#include <wait.h>

#define ALL		((Exnode_t*)0)
#define LIST		((Exnode_t*)1)

#define FTW_LIST	(FTW_USER<<0)	/* files listed on stdin	*/
#define FTW_RECURSIVE	(FTW_USER<<1)	/* walk files listed on stdin	*/

typedef struct Dir			/* directory list		*/
{
	struct Dir*	next;		/* next in list			*/
	char*		name;		/* dir name			*/
} Dir_t;

State_t			state;

static void		intermediate(Ftw_t*, char*);

static int
urlcmp(register const char* p, register const char* s, int d)
{
	int	pc;
	int	sc;

	for (;;)
	{
		if ((pc = *p++) == d)
		{
			if (*s != d)
				return -1;
			break;
		}
		if ((sc = *s++) == d)
			return 1;
		if (pc == '%')
		{
			pc = (int)strntol(p, 2, NiL, 16);
			p += 2;
		}
		if (sc == '%')
		{
			sc = (int)strntol(s, 2, NiL, 16);
			s += 2;
		}
		if (pc < sc)
			return -1;
		if (pc > sc)
			return 1;
	}
	return 0;
}

#define SNAPSHOT_changed	'C'
#define SNAPSHOT_deleted	'D'
#define SNAPSHOT_new		'N'

/*
 * do the action
 */

static void
act(register Ftw_t* ftw, int op)
{
	char*	s;
	Sfio_t*	fp;
	int	i;
	int	j;
	int	k;
	int	n;
	int	r;

	switch (op)
	{
	case ACT_CMDARG:
		if ((i = cmdarg(state.cmd, ftw->path, ftw->pathlen)) >= state.errexit)
			exit(i);
		break;
	case ACT_CODE:
		if (findwrite(state.find, ftw->path, ftw->pathlen, (ftw->info & FTW_D) ? "system/dir" : (char*)0))
			state.finderror = 1;
		break;
	case ACT_CODETYPE:
		fp = sfopen(NiL, PATH(ftw), "r");
		if (findwrite(state.find, ftw->path, ftw->pathlen, magictype(state.magic, fp, PATH(ftw), &ftw->statb)))
			state.finderror = 1;
		if (fp)
			sfclose(fp);
		break;
	case ACT_EVAL:
		eval(state.action, ftw);
		break;
	case ACT_INTERMEDIATE:
		intermediate(ftw, ftw->path);
		break;
	case ACT_LIST:
		sfputr(sfstdout, ftw->path, '\n');
		break;
	case ACT_SNAPSHOT:
		print(state.snapshot.tmp, ftw, state.snapshot.format.path);
		sfputc(state.snapshot.tmp, state.snapshot.format.delim);
		i = sfstrtell(state.snapshot.tmp);
		print(state.snapshot.tmp, ftw, state.snapshot.format.easy);
		j = sfstrtell(state.snapshot.tmp);
		s = sfstrbase(state.snapshot.tmp);
		r = SNAPSHOT_new;
		if (!state.snapshot.prev)
			k = 1;
		else
		{
			do
			{
				if (!(k = urlcmp(state.snapshot.prev, s, state.snapshot.format.delim)))
				{
					r = SNAPSHOT_changed;
					if (!(k = memcmp(state.snapshot.prev + i, s + i, j - i) || state.snapshot.prev[j] != state.snapshot.format.delim))
					{
						if ((n = (int)sfvalue(sfstdin)) > 4 && state.snapshot.prev[n-2] == state.snapshot.format.delim)
						{
							sfwrite(sfstdout, state.snapshot.prev, n - 4);
							sfputc(sfstdout, '\n');
						}
						else
							sfwrite(sfstdout, state.snapshot.prev, n);
					}
				}
				else if (k > 0)
					break;
				else if (k < 0 && (n = (int)sfvalue(sfstdin)) > 4 && (state.snapshot.prev[n-2] != state.snapshot.format.delim || state.snapshot.prev[n-3] != SNAPSHOT_deleted))
				{
					sfwrite(sfstdout, state.snapshot.prev, n - (state.snapshot.prev[n-2] == state.snapshot.format.delim ? 4 : 1));
					sfputc(sfstdout, state.snapshot.format.delim);
					sfputc(sfstdout, SNAPSHOT_deleted);
					sfputc(sfstdout, state.snapshot.format.delim);
					sfputc(sfstdout, '\n');
					if (state.cmdflags & CMD_TRACE)
						error(1, "%s deleted", ftw->path);
				}
				if (!(state.snapshot.prev = sfgetr(sfstdin, '\n', 0)))
					break;
			} while (k < 0);
		}
		if (k)
		{
			if (state.snapshot.format.hard && (ftw->info & FTW_F))
			{
				sfputc(state.snapshot.tmp, state.snapshot.format.delim);
				print(state.snapshot.tmp, ftw, state.snapshot.format.hard);
			}
			sfputc(state.snapshot.tmp, state.snapshot.format.delim);
			sfputc(state.snapshot.tmp, r);
			sfputc(state.snapshot.tmp, state.snapshot.format.delim);
			sfputr(sfstdout, sfstruse(state.snapshot.tmp), '\n');
			if (state.cmdflags & CMD_TRACE)
				error(1, "%s %s", ftw->path, r == SNAPSHOT_new ? "new" : "changed");
		}
		else
			sfstrseek(state.snapshot.tmp, SEEK_SET, 0);
		break;
	}
}

/*
 * generate intermediate (missing) directories for terminal elements
 */

static void
intermediate(register Ftw_t* ftw, register char* path)
{
	register char*	s;
	register char*	t;
	register int	c;

	if (!(ftw->info & FTW_D) || ftw->statb.st_nlink)
	{
		ftw->statb.st_nlink = 0;
		if (ftw->level > 1)
			intermediate(ftw->parent, path);
		s = path + ftw->pathlen;
		c = *s;
		*s = 0;
		t = ftw->path;
		ftw->path = path;
		act(ftw, state.actII);
		ftw->path = t;
		*s = c;
	}
}

/*
 * tw a single file
 */

static int
tw(register Ftw_t* ftw)
{
	Local_t*	lp;

	switch (ftw->info)
	{
	case FTW_NS:
		if (!state.info)
		{
			if (!state.pattern && !state.ignore)
				error(2, "%s: not found", ftw->path);
			return 0;
		}
		break;
	case FTW_DC:
		if (!state.info)
		{
			if (!state.ignore)
				error(2, "%s: directory causes cycle", ftw->path);
			return 0;
		}
		break;
	case FTW_DNR:
		if (!state.info)
		{
			if (!state.ignore)
				error(2, "%s: cannot read directory", ftw->path);
		}
		break;
	case FTW_DNX:
		if (!state.info)
		{
			if (!state.ignore)
				error(2, "%s: cannot search directory", ftw->path);
			ftw->status = FTW_SKIP;
		}
		break;
	case FTW_DP:
		if (!(state.ftwflags & FTW_TWICE) || (state.ftwflags & FTW_DOT) && stat(PATH(ftw), &ftw->statb))
			goto pop;
		break;
	case FTW_D:
		ftw->ignorecase = (state.icase || (!ftw->level || !ftw->parent->ignorecase) && strchr(astconf("PATH_ATTRIBUTES", ftw->name, NiL), 'c')) ? STR_ICASE : 0;
		break;
	default:
		ftw->ignorecase = ftw->level ? ftw->parent->ignorecase : (state.icase || strchr(astconf("PATH_ATTRIBUTES", ftw->name, NiL), 'c')) ? STR_ICASE : 0;
		break;
	}
	if (state.localfs && (ftw->info & FTW_D) && !ftwlocal(ftw))
		ftw->status = FTW_SKIP;
	else
	{
		if (state.select == ALL || eval(state.select, ftw) && ftw->status != FTW_SKIP)
			act(ftw, state.act);
 pop:
		if (state.localmem && (lp = (Local_t*)ftw->local.pointer))
		{
			lp->next = state.local;
			state.local = lp;
		}
		if ((state.ftwflags & (FTW_LIST|FTW_RECURSIVE)) == FTW_LIST)
			ftw->status = FTW_SKIP;
	}
	return 0;
}

/*
 * order child entries
 */

static int
order(register Ftw_t* f1, register Ftw_t* f2)
{
	register Exnode_t*	x;
	register Exnode_t*	y;
	register int		v;
	long			n1;
	long			n2;
	int			icase;
	int			reverse;

	v = 0;
	icase = state.icase;
	reverse = state.reverse;
	x = state.sortkey;
	y = 0;
	for (;;)
	{
		switch (x->op)
		{
		case '~':
			icase = !icase;
			x = x->data.operand.left;
			continue;
		case '!':
			reverse = !reverse;
			x = x->data.operand.left;
			continue;
		case ',':
			y = x->data.operand.right;
			x = x->data.operand.left;
			continue;
		case S2B:
		case X2I:
			x = x->data.operand.left;
			continue;
		case ID:
			x->data.variable.symbol;
			if (x->data.variable.symbol->index == F_name || x->data.variable.symbol->index == F_path)
				v = icase ? strcasecmp(f1->name, f2->name) : strcoll(f1->name, f2->name);
			else
			{
				n1 = getnum(x->data.variable.symbol, f1);
				n2 = getnum(x->data.variable.symbol, f2);
				if (n1 < n2)
					v = -1;
				else if (n1 > n2)
					v = 1;
				else
					v = 0;
			}
			if (v)
			{
				if (reverse)
					v = -v;
				break;
			}
			if (!(x = y))
				break;
			y = 0;
			icase = state.icase;
			reverse = state.reverse;
			continue;
		}
		break;
	}
	message((-2, "order(%s,%s) = %d [n1=%ld n2=%ld]", f1->name, f2->name, v, n1, n2));
	return v;
}

int
main(int argc, register char** argv)
{
	register int	n;
	register char*	s;
	char*		args;
	char*		codes;
	char**		av;
	char**		ap;
	int		i;
	int		count;
	int		len;
	int		traverse;
	int		size;
	Dir_t*		firstdir;
	Dir_t*		lastdir;
	Exnode_t*	x;
	Exnode_t*	y;
	Ftw_t		ftw;
	Finddisc_t	disc;

	setlocale(LC_ALL, "");
	error_info.id = "tw";
	av = argv + 1;
	args = 0;
	codes = 0;
	count = 0;
	size = 0;
	traverse = 1;
	firstdir = lastdir = newof(0, Dir_t, 1, 0);
	firstdir->name = ".";
	state.action = LIST;
	state.cmdflags = CMD_EXIT|CMD_IGNORE|CMD_IMPLICIT|CMD_NEWLINE;
	state.errexit = EXIT_QUIT;
	state.ftwflags = ftwflags()|FTW_DELAY;
	state.select = ALL;
	state.separator = '\n';
	memset(&disc, 0, sizeof(disc));
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			args = opt_info.arg;
			state.cmdflags |= CMD_POST;
			continue;
		case 'c':
			if ((count = opt_info.num) < 0)
				error(3, "argument count must be >= 0");
			continue;
		case 'd':
			lastdir = lastdir->next = newof(0, Dir_t, 1, 0);
			lastdir->name = opt_info.arg;
			continue;
		case 'e':
			compile(opt_info.arg, 0);
			continue;
		case 'f':
			state.pattern = opt_info.arg;
			continue;
		case 'i':
			state.ignore = 1;
			continue;
		case 'l':
			state.localfs = 1;
			continue;
		case 'm':
			state.intermediate = 1;
			continue;
		case 'n':
			traverse = 0;
			continue;
		case 'p':
			state.ftwflags |= FTW_TWICE;
			continue;
		case 'q':
			state.cmdflags |= CMD_QUERY;
			continue;
		case 'r':
			state.ftwflags |= FTW_RECURSIVE;
			continue;
		case 's':
			if ((size = opt_info.num) < 0)
				error(3, "command size must be >= 0");
			continue;
		case 't':
			state.cmdflags |= CMD_TRACE;
			continue;
		case 'x':
			state.errexit = opt_info.arg ? opt_info.num : EXIT_QUIT;
			continue;
		case 'z':
			if (s = sfgetr(sfstdin, '\n', 1))
			{
				if (!(s = strdup(s)))
					error(ERROR_SYSTEM|3, "out of space");
				n = state.snapshot.format.delim = *s++;
				state.snapshot.format.path = s;
				if (!(s = strchr(s, n)))
				{
				osnap:
					error(3, "invalid snapshot on standard input");
				}
				*s++ = 0;
				if (!streq(state.snapshot.format.path, SNAPSHOT_ID))
					goto osnap;
				state.snapshot.format.path = s;
				if (!(s = strchr(s, n)))
					goto osnap;
				*s++ = 0;
				state.snapshot.format.easy = s;
				if (!(s = strchr(s, n)))
					goto osnap;
				*s++ = 0;
				if (*(state.snapshot.format.hard = s))
				{
					if (!(s = strchr(s, n)))
						goto osnap;
					*s = 0;
				}
				else
					state.snapshot.format.hard = 0;
				state.snapshot.sp = sfstdin;
				state.snapshot.prev = sfgetr(sfstdin, '\n', 0);
			}
			else
			{
				state.snapshot.format.path = SNAPSHOT_PATH;
				state.snapshot.format.easy = SNAPSHOT_EASY;
				state.snapshot.format.hard = SNAPSHOT_HARD;
				state.snapshot.format.delim = SNAPSHOT_DELIM[0];
			}
			if (!(state.snapshot.tmp = sfstropen()))
				error(ERROR_SYSTEM|3, "out of space");
			compile("sort:name;", 0);
			continue;
		case 'C':
			state.ftwflags |= FTW_NOSEEDOTDIR;
			continue;
		case 'D':
			error_info.trace = -opt_info.num;
			continue;
		case 'E':
			compile(opt_info.arg, 1);
			continue;
		case 'F':
			codes = opt_info.arg;
			continue;
		case 'G':
			disc.flags |= FIND_GENERATE;
			if (streq(opt_info.arg, "old"))
				disc.flags |= FIND_OLD;
			else if (streq(opt_info.arg, "gnu") || streq(opt_info.arg, "locate"))
				disc.flags |= FIND_GNU;
			else if (streq(opt_info.arg, "type"))
				disc.flags |= FIND_TYPE;
			else if (streq(opt_info.arg, "?"))
			{
				error(2, "formats are { default|dir type old gnu|locate }");
				return 0;
			}
			else if (!streq(opt_info.arg, "-") && !streq(opt_info.arg, "default") && !streq(opt_info.arg, "dir"))
				error(3, "%s: invalid find codes format -- { default|dir type old gnu|locate } expected", opt_info.arg);
			continue;
		case 'H':
			state.ftwflags |= FTW_META|FTW_PHYSICAL;
			continue;
		case 'I':
			state.icase = 1;
			continue;
		case 'L':
			state.ftwflags &= ~(FTW_META|FTW_PHYSICAL|FTW_SEEDOTDIR);
			continue;
		case 'P':
			state.ftwflags &= ~FTW_META;
			state.ftwflags |= FTW_PHYSICAL;
			continue;
		case 'S':
			state.separator = *opt_info.arg;
			continue;
		case 'X':
			state.ftwflags |= FTW_MOUNT;
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
	argc -= opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));

	/*
	 * do it
	 */

	if (state.snapshot.tmp)
		sfprintf(sfstdout, "%c%s%c%s%c%s%c%s%c\n",
			state.snapshot.format.delim, SNAPSHOT_ID,
			state.snapshot.format.delim, state.snapshot.format.path,
			state.snapshot.format.delim, state.snapshot.format.easy,
			state.snapshot.format.delim, state.snapshot.format.hard ? state.snapshot.format.hard : "",
			state.snapshot.format.delim);
	if (x = exexpr(state.program, "begin", NiL, 0))
		eval(x, NiL);
	if ((x = exexpr(state.program, "select", NiL, INTEGER)) || (x = exexpr(state.program, NiL, NiL, INTEGER)))
		state.select = x;
	if (!(state.ftwflags & FTW_PHYSICAL))
		state.ftwflags &= ~FTW_DELAY;
	memset(&ftw, 0, sizeof(ftw));
	ftw.path = ftw.name = "";
	if (traverse)
	{
		if (x = exexpr(state.program, "action", NiL, 0))
			state.action = x;
		if (x = exexpr(state.program, "sort", NiL, 0))
		{
			state.sortkey = x;
			y = 0;
			for (;;)
			{
				switch (x->op)
				{
				case ',':
					y = x->data.operand.right;
					/*FALLTHROUGH*/
				case '!':
				case '~':
				case S2B:
				case X2I:
					x = x->data.operand.left;
					continue;
				case ID:
					if (!(x = y))
						break;
					y = 0;
					continue;
				default:
					error(3, "invalid sort identifier (op 0x%02x)", x->op);
					break;
				}
				break;
			}
			state.sort = order;
		}
		if (*argv && (*argv)[0] == '-' && (*argv)[1] == 0)
		{
			state.ftwflags |= FTW_LIST;
			argv++;
			argc--;
		}
		if (*argv || args || count || !(state.cmdflags & CMD_IMPLICIT))
		{
			Cmddisc_t	disc;

			CMDDISC(&disc, state.cmdflags, errorf);
			state.cmd = cmdopen(argv, count, size, args, &disc);
			state.ftwflags |= FTW_DOT;
		}
		else
			state.cmdflags &= ~CMD_IMPLICIT;
		if (codes && (disc.flags & FIND_GENERATE))
		{
			char*	p;
			Dir_t*	dp;
			char	pwd[PATH_MAX];
			char	tmp[PATH_MAX];

			disc.version = FIND_VERSION;
			if (state.cmdflags & CMD_TRACE)
				disc.flags |= FIND_TYPE;
			if (state.cmdflags & CMD_QUERY)
				disc.flags |= FIND_OLD;
			disc.errorf = errorf;
			if (!(state.find = findopen(codes, NiL, NiL, &disc)))
				exit(2);
			if (disc.flags & FIND_TYPE)
			{
				state.act = ACT_CODETYPE;
				compile("_tw_init:mime;", 0);
				state.magicdisc.flags |= MAGIC_MIME;
			}
			else
				state.act = ACT_CODE;
			state.icase = 1;
			state.pattern = 0;
			state.sort = order;
			if (!state.program)
				compile("1", 0);
			if (!(state.sortkey = newof(0, Exnode_t, 1, 0)) || !(state.sortkey->data.variable.symbol = (Exid_t*)dtmatch(state.program->symbols, "name")))
				error(ERROR_SYSTEM|3, "out of space");
			state.sortkey->op = ID;
			s = p = 0;
			for (dp = (firstdir == lastdir) ? firstdir : firstdir->next; dp; dp = dp->next)
			{
				if (*(s = dp->name) == '/')
					sfsprintf(tmp, sizeof(tmp), "%s", s);
				else if (!p && !(p = getcwd(pwd, sizeof(pwd))))
					error(ERROR_SYSTEM|3, "cannot determine pwd path");
				else
					sfsprintf(tmp, sizeof(tmp), "%s/%s", p, s);
				pathcanon(tmp, sizeof(tmp), PATH_PHYSICAL);
				if (!(dp->name = strdup(tmp)))
					error(ERROR_SYSTEM|3, "out of space [PATH_PHYSICAL]");
			}
		}
		else if (state.snapshot.tmp)
			state.act = ACT_SNAPSHOT;
		else if (state.cmdflags & CMD_IMPLICIT)
			state.act = ACT_CMDARG;
		else if (state.action == LIST)
			state.act = ACT_LIST;
		else if (state.action)
			state.act = ACT_EVAL;
		if (state.intermediate)
		{
			state.actII = state.act;
			state.act = ACT_INTERMEDIATE;
		}
		if (state.pattern)
		{
			disc.version = FIND_VERSION;
			if (state.icase)
				disc.flags |= FIND_ICASE;
			disc.errorf = errorf;
			disc.dirs = ap = av;
			if (firstdir != lastdir)
				firstdir = firstdir->next;
			do {*ap++ = firstdir->name;} while (firstdir = firstdir->next);
			*ap = 0;
			if (!(state.find = findopen(codes, state.pattern, NiL, &disc)))
				exit(1);
			state.ftwflags |= FTW_TOP;
			n = state.select == ALL ? state.act : ACT_EVAL;
			while (s = findread(state.find))
			{
				switch (n)
				{
				case ACT_CMDARG:
					if ((i = cmdarg(state.cmd, s, strlen(s))) >= state.errexit)
						exit(i);
					break;
				case ACT_LIST:
					sfputr(sfstdout, s, '\n');
					break;
				default:
					ftwalk(s, tw, state.ftwflags, NiL);
					break;
				}
			}
		}
		else if (state.ftwflags & FTW_LIST)
		{
			sfopen(sfstdin, NiL, "rt");
			n = state.select == ALL && state.act == ACT_CMDARG;
			for (;;)
			{
				if (s = sfgetr(sfstdin, state.separator, 1))
					len = sfvalue(sfstdin) - 1;
				else if (state.separator != '\n')
				{
					state.separator = '\n';
					continue;
				}
				else if (s = sfgetr(sfstdin, state.separator, -1))
					len = sfvalue(sfstdin);
				else
					break;
				if (!n)
					ftwalk(s, tw, state.ftwflags, NiL);
				else if ((i = cmdarg(state.cmd, s, len)) >= state.errexit)
					exit(i);
			}
			if (sferror(sfstdin))
				error(ERROR_SYSTEM|2, "input read error");
		}
		else if (firstdir == lastdir)
			ftwalk(firstdir->name, tw, state.ftwflags, state.sort);
		else
		{
			ap = av;
			while (firstdir = firstdir->next)
				*ap++ = firstdir->name;
			*ap = 0;
			ftwalk((char*)av, tw, state.ftwflags|FTW_MULTIPLE, state.sort);
		}
		if (state.cmd && (i = cmdflush(state.cmd)) >= state.errexit)
			exit(i);
		if (state.find && (findclose(state.find) || state.finderror))
			exit(2);
	}
	else if (state.select)
		error_info.errors = eval(state.select, &ftw) == 0;
	if (x = exexpr(state.program, "end", NiL, 0))
		eval(x, &ftw);
	if (sfsync(sfstdout))
		error(ERROR_SYSTEM|2, "write error");
	exit(error_info.errors != 0);
}
