/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2013 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * sum -- list file checksum and size
 */

static const char usage[] =
"[-?\n@(#)$Id: sum (AT&T Research) 2012-04-20 $\n]"
USAGE_LICENSE
"[+NAME?cksum,md5sum,sum - print file checksum and block count]"
"[+DESCRIPTION?\bsum\b lists the checksum, and for most methods the block"
"	count, for each file argument. The standard input is read if there are"
"	no \afile\a arguments. \bgetconf UNIVERSE\b determines the default"
"	\bsum\b method: \batt\b for the \batt\b universe, \bbsd\b otherwise."
"	The default for the other commands is the command name itself. The"
"	\batt\b method is a true sum, all others are order dependent.]"
"[+?Method names consist of a leading identifier and 0 or more options"
"	separated by -.]"
"[+?\bgetconf PATH_RESOLVE\b determines how symbolic links are handled. This"
"	can be explicitly overridden by the \b--logical\b, \b--metaphysical\b,"
"	and \b--physical\b options below. \bPATH_RESOLVE\b can be one of:]{"
"		[+logical?Follow all symbolic links.]"
"		[+metaphysical?Follow command argument symbolic links,"
"			otherwise don't follow.]"
"		[+physical?Don't follow symbolic links.]"
"}"

"[a:all?List the checksum for all files. Use with \b--total\b to list both"
"	individual and total checksums and block counts.]"
"[b:binary?Read files in binary mode. This is the default.]"
"[B:scale?Block count scale (bytes per block) override for methods that"
"	include size in the output.  The default is method specific.]#[scale]"
"[c:check?Each \afile\a is interpreted as the output from a previous \bsum\b."
"	If \b--header\b or \b--permissions\b was specified in the previous"
"	\bsum\b then the checksum method is automatically determined,"
"	otherwise \b--method\b must be specified. The listed checksum is"
"	compared with the current value and a warning is issued for each file"
"	that does not match. If \afile\a was generated by \b--permissions\b"
"	then the file mode, user and group are also checked. Empty lines,"
"	lines starting with \b#<space>\b, or the line \b#\b are ignored. Lines"
"	containing no blanks are interpreted as [no]]\aname\a[=\avalue\a]]"
"	options:]{"
"		[+method=name?Checksum method to apply to subsequent lines.]"
"		[+permissions?Subsequent lines were generated with"
"			\b--permissions\b.]"
"}"
"[h:header?Print the checksum method as the first output line. Used with"
"	\b--check\b and \b--permissions\b.]"
"[l:list?Each \afile\a is interpreted as a list of files, one per line,"
"	that is checksummed.]"
"[p:permissions?If \b--check\b is not specified then list the file"
"	mode, user and group between the checksum and path. User and group"
"	matching the caller are output as \b-\b. If \b--check\b is"
"	specified then the mode, user and group for each path in \afile\a"
"	are updated if necessary to match those in \afile\a. A warning is"
"	printed on the standard error for each changed file.]"
"[R:recursive?Recursively checksum the contents of directories.]"
"[S:silent|status?No output for \b--check\b; 0 exit status means all sums"
"	matched, non-0 means at least one sum failed to match. Ignored for"
"	\b--permissions\b.]"
"[t:text?Read files in text mode (i.e., treat \b\\r\\n\b as \b\\n\b).]"
"[T:total?List only the total checksum and block count of all files."
"	\b--all\b \b--total\b lists each checksum and the total. The"
"	total checksum and block count may be different from the checksum"
"	and block count of the catenation of all files due to partial"
"	blocks that may occur when the files are treated separately.]"
"[w!:warn?Warn about invalid \b--check\b lines.]"
"[x:method|algorithm?Specifies the checksum \amethod\a to"
"	apply. Parenthesized method options are readonly implementation"
"	details.]:[method]{\fmethods\f}"
"[L:logical|follow?Follow symbolic links when traversing directories. The"
"	default is determined by \bgetconf PATH_RESOLVE\b.]"
"[H:metaphysical?Follow command argument symbolic links, otherwise don't"
"	follow symbolic links when traversing directories. The default is"
"	determined by \bgetconf PATH_RESOLVE\b.]"
"[P:physical?Don't follow symbolic links when traversing directories. The"
"	default is determined by \bgetconf PATH_RESOLVE\b.]"
"[r:bsd?Equivalent to \b--method=bsd --scale=512\b for compatibility with"
"	other \bsum\b(1) implementations.]"
"[s:sysv?Equivalent to \b--method=sys5\b for compatibility with other"
"	\bsum\b(1) implementations.]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+SEE ALSO?\bgetconf\b(1), \btw\b(1), \buuencode\b(1)]"
;

#include <cmd.h>
#include <sum.h>
#include <ls.h>
#include <modex.h>
#include <fts_fix.h>
#include <error.h>

typedef struct State_s			/* program state		*/
{
	int		all;		/* list all items		*/
	Sfio_t*		check;		/* check previous output	*/
	int		flags;		/* sumprint() SUM_* flags	*/
	gid_t		gid;		/* caller gid			*/
	int		header;		/* list method on output	*/
	int		list;		/* list file name too		*/
	Sum_t*		oldsum;		/* previous sum method		*/
	int		permissions;	/* include mode,uer,group	*/
	int		haveperm;	/* permissions in the input	*/
	int		recursive;	/* recursively descend dirs	*/
	size_t		scale;		/* scale override		*/
	unsigned long	size;		/* combined size of all files	*/
	int		silent;		/* silent check, 0 exit if ok	*/
	int		(*sort)(FTSENT* const*, FTSENT* const*);
	Sum_t*		sum;		/* sum method			*/
	bool		text;		/* \r\n == \n			*/
	int		total;		/* list totals only		*/
	uid_t		uid;		/* caller uid			*/
	int		warn;		/* invalid check line warnings	*/
} State_t;

static void	verify(State_t*, char*, char*, Sfio_t*);

/*
 * open path for read mode
 */

static Sfio_t*
openfile(const char* path, const char* mode)
{
	Sfio_t*		sp;

	if (!path || streq(path, "-") || streq(path, "/dev/stdin") || streq(path, "/dev/fd/0"))
	{
		sp = sfstdin;
		sfopen(sp, NiL, mode);
	}
	else if (!(sp = sfopen(NiL, path, mode)))
		error(ERROR_SYSTEM|2, "%s: cannot read", path);
	return sp;
}

/*
 * close an openfile() stream
 */

static int
closefile(Sfio_t* sp)
{
	return sp == sfstdin ? 0 : sfclose(sp);
}

/*
 * compute and print sum on an open file
 */

static void
pr(State_t* state, Sfio_t* op, Sfio_t* ip, char* file, int perm, struct stat* st, Sfio_t* check)
{
	char*		p;
	char*		r;
	char*		e;
	int		peek;
	struct stat		ss;

	static const char*	indicator[] = { "*", " " };

	if (check)
	{
		state->oldsum = state->sum;
		while (p = sfgetr(ip, '\n', 1))
			verify(state, p, file, check);
		state->sum = state->oldsum;
		if (state->warn && !sfeof(ip))
			error(2, "%s: last line incomplete", file);
		return;
	}
	suminit(state->sum);
	if (state->text)
	{
		peek = 0;
		while (p = sfreserve(ip, SF_UNBOUND, 0))
		{
			e = p + sfvalue(ip);
			if (peek)
			{
				peek = 0;
				if (*p != '\n')
					sumblock(state->sum, "\r", 1);
			}
			while (r = memchr(p, '\r', e - p))
			{
				if (++r >= e)
				{
					e--;
					peek = 1;
					break;
				}
				sumblock(state->sum, p, r - p - (*r == '\n'));
				p = r;
			}
			sumblock(state->sum, p, e - p);
		}
		if (peek)
			sumblock(state->sum, "\r", 1);
	}
	else
		while (p = sfreserve(ip, SF_UNBOUND, 0))
			sumblock(state->sum, p, sfvalue(ip));
	if (sfvalue(ip))
		error(ERROR_SYSTEM|2, "%s: read error", file);
	sumdone(state->sum);
	if (!state->total || state->all)
	{
		sumprint(state->sum, op, state->flags|SUM_SCALE, state->scale);
		if (perm >= 0)
		{
			if (perm)
			{
				if (!st && fstat(sffileno(ip), st = &ss))
					error(ERROR_SYSTEM|2, "%s: cannot stat", file);
				else
					sfprintf(sfstdout, " %04o %s %s",
						modex(st->st_mode & S_IPERM),
						(st->st_uid != state->uid && ((st->st_mode & S_ISUID) || (st->st_mode & S_IRUSR) && !(st->st_mode & (S_IRGRP|S_IROTH)) || (st->st_mode & S_IXUSR) && !(st->st_mode & (S_IXGRP|S_IXOTH)))) ? fmtuid(st->st_uid) : "-",
						(st->st_gid != state->gid && ((st->st_mode & S_ISGID) || (st->st_mode & S_IRGRP) && !(st->st_mode & S_IROTH) || (st->st_mode & S_IXGRP) && !(st->st_mode & S_IXOTH))) ? fmtgid(st->st_gid) : "-");
			}
			if (ip != sfstdin)
				sfprintf(op, " %s%s", (state->sum->flags & SUM_INDICATOR) ? indicator[state->text] : "",  file);
			sfputc(op, '\n');
		}
	}
}

/*
 * verify previous sum output
 */

static void
verify(State_t* state, char* s, char* check, Sfio_t* rp)
{
	char*	t;
	char*		e;
	char*		file;
	int		attr;
	int		mode;
	int		uid;
	int		gid;
	bool		text;
	Sfio_t*		sp;
	struct stat	st;

	if (!*s || *s == '#' && (!*(s + 1) || *(s + 1) == ' ' || *(s + 1) == '\t'))
		return;
	if (t = strchr(s, ' '))
	{
		if ((t - s) > 10 || !(file = strchr(t + 1, ' ')))
			file = t;
		*file++ = 0;
		if (!(state->sum->flags & SUM_INDICATOR))
			text = state->text;
		else if (*file == '*')
		{
			file++;
			text = 0;
		}
		else if (*file == ' ')
		{
			file++;
			text = 1;
		}
		else
			text = state->text;
		attr = 0;
		if ((mode = strtol(file, &e, 8)) && *e == ' ' && (e - file) == 4)
		{
			mode = modei(mode);
			if (t = strchr(++e, ' '))
			{
				if (*e == '-' && (t - e) == 1)
					uid = -1;
				else
				{
					*t = 0;
					uid = struid(e);
					*t = ' ';
				}
				if (e = strchr(++t, ' '))
				{
					if (*t == '-' && (e - t) == 1)
						gid = -1;
					else
					{
						*e = 0;
						gid = struid(t);
						*e = ' ';
					}
					file = e + 1;
					attr = 1;
				}
			}
		}
		if (sp = openfile(file, text ? "rt" : "rb"))
		{
			pr(state, rp, sp, file, -1, NiL, NiL);
			if (!(t = sfstruse(rp)))
				error(ERROR_SYSTEM|3, "out of space");
			if (!streq(s, t))
			{
				if (state->silent)
					error_info.errors++;
				else
					error(2, "%s: checksum changed", file);
			}
			else if (attr)
			{
				if (fstat(sffileno(sp), &st))
				{
					if (state->silent)
						error_info.errors++;
					else
						error(ERROR_SYSTEM|2, "%s: cannot stat", file);
				}
				else
				{
					if (uid < 0 || uid == st.st_uid)
						uid = -1;
					else if (!state->permissions)
					{
						if (state->silent)
							error_info.errors++;
						else
							error(2, "%s: uid should be %s", file, fmtuid(uid));
					}
					if (gid < 0 || gid == st.st_gid)
						gid = -1;
					else if (!state->permissions)
					{
						if (state->silent)
							error_info.errors++;
						else
							error(2, "%s: gid should be %s", file, fmtgid(gid));
					}
					if (state->permissions && (uid >= 0 || gid >= 0))
					{
						if (chown(file, uid, gid) < 0)
						{
							if (uid < 0)
								error(ERROR_SYSTEM|2, "%s: cannot change group to %s", file, fmtgid(gid));
							else if (gid < 0)
								error(ERROR_SYSTEM|2, "%s: cannot change user to %s", file, fmtuid(uid));
							else
								error(ERROR_SYSTEM|2, "%s: cannot change user to %s and group to %s", file, fmtuid(uid), fmtgid(gid));
						}
						else
						{
							if (uid < 0)
								error(1, "%s: changed group to %s", file, fmtgid(gid));
							else if (gid < 0)
								error(1, "%s: changed user to %s", file, fmtuid(uid));
							else
								error(1, "%s: changed user to %s and group to %s", file, fmtuid(uid), fmtgid(gid));
						}
					}
					if ((st.st_mode & S_IPERM) ^ mode)
					{
						if (state->permissions)
						{
							if (chmod(file, mode) < 0)
								error(ERROR_SYSTEM|2, "%s: cannot change mode to %s", file, fmtmode(mode, 0));
							else
								error(ERROR_SYSTEM|1, "%s: changed mode to %s", file, fmtmode(mode, 0));
						}
						else if (state->silent)
							error_info.errors++;
						else
							error(2, "%s: mode should be %s", file, fmtmode(mode, 0));
					}
				}
			}
			closefile(sp);
		}
	}
	else if (strneq(s, "method=", 7))
	{
		s += 7;
		if (state->sum != state->oldsum)
			sumclose(state->sum);
		if (!(state->sum = sumopen(s)))
			error(3, "%s: %s: unknown checksum method", check, s);
	}
	else if (streq(s, "permissions"))
		state->haveperm = 1;
	else if (streq(s, "text"))
		state->text = 1;
	else if (streq(s, "binary"))
		state->text = 0;
	else
		error(1, "%s: %s: unknown option", check, s);
}

/*
 * sum the list of files in lp
 */

static void
list(State_t* state, Sfio_t* lp)
{
	char*		file;
	Sfio_t*	sp;

	while (file = sfgetr(lp, '\n', 1))
		if (sp = openfile(file, state->check ? "rt" : "rb"))
		{
			pr(state, sfstdout, sp, file, state->permissions, NiL, state->check);
			closefile(sp);
		}
}

/*
 * order child entries
 */

static int
order(FTSENT* const* f1, FTSENT* const* f2)
{
	return strcoll((*f1)->fts_name, (*f2)->fts_name);
}

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	if (streq(s, "methods"))
		return sumusage(sp);
	return 0;
}

int
b_cksum(int argc, char** argv, Shbltin_t* context)
{
	int	flags;
	char*		file;
	char*		method;
	Sfio_t*		sp;
	FTS*		fts;
	FTSENT*		ent;
	int		logical;
	Optdisc_t	optdisc;
	State_t		state;

	cmdinit(argc, argv, context, ERROR_CATALOG, ERROR_NOTIFY);
	memset(&state, 0, sizeof(state));
	flags = fts_flags() | FTS_META | FTS_TOP | FTS_NOPOSTORDER;
	state.flags = SUM_SIZE;
	state.warn = 1;
	logical = 1;
	method = 0;
	optinit(&optdisc, optinfo);
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			state.all = 1;
			continue;
		case 'b':
			state.text = 0;
			continue;
		case 'B':
			state.scale = opt_info.num;
			continue;
		case 'c':
			if (!(state.check = sfstropen()))
				error(3, "out of space [check]");
			continue;
		case 'h':
			state.header = 1;
			continue;
		case 'l':
			state.list = 1;
			continue;
		case 'p':
			state.permissions = 1;
			continue;
		case 'r':
			method = "bsd";
			state.scale = 512;
			state.flags |= SUM_LEGACY;
			continue;
		case 'R':
			flags &= ~FTS_TOP;
			state.recursive = 1;
			state.sort = order;
			logical = 0;
			continue;
		case 's':
			method = "sys5";
			continue;
		case 'S':
			state.silent = opt_info.num;
			continue;
		case 't':
			state.text = 1;
			continue;
		case 'T':
			state.total = 1;
			continue;
		case 'w':
			state.warn = opt_info.num;
			continue;
		case 'x':
			method = opt_info.arg;
			continue;
		case 'H':
			flags |= FTS_META|FTS_PHYSICAL;
			logical = 0;
			continue;
		case 'L':
			flags &= ~(FTS_META|FTS_PHYSICAL);
			logical = 0;
			continue;
		case 'P':
			flags &= ~FTS_META;
			flags |= FTS_PHYSICAL;
			logical = 0;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));

	/*
	 * check the method
	 */

	if (method && !(state.sum = sumopen(method)))
		error(3, "%s: unknown checksum method", method);
	if (!state.sum && !(state.sum = sumopen(error_info.id)) && !(state.sum = sumopen(astconf("UNIVERSE", NiL, NiL))))
		state.sum = sumopen(NiL);

	/*
	 * do it
	 */

	if (logical)
	{
		flags &= ~(FTS_META|FTS_PHYSICAL);
		flags |= FTS_SEEDOTDIR;
	}
	if (state.permissions)
	{
		state.uid = geteuid();
		state.gid = getegid();
		state.silent = 0;
	}
	if (!state.check && (state.header || state.permissions))
	{
		sfprintf(sfstdout, "method=%s\n", state.sum->name);
		if (state.permissions)
			sfprintf(sfstdout, "permissions\n");
		if (state.text)
			sfprintf(sfstdout, "text\n");
	}
	if (state.list)
	{
		if (*argv)
		{
			while (file = *argv++)
				if (sp = openfile(file, "rt"))
				{
					list(&state, sp);
					closefile(sp);
				}
		}
		else if (sp = openfile(NiL, "rt"))
		{
			list(&state, sp);
			closefile(sp);
		}
	}
	else if (!*argv && !state.recursive)
		pr(&state, sfstdout, sfstdin, "/dev/stdin", state.permissions, NiL, state.check);
	else if (!(fts = fts_open(argv, flags, state.sort)))
		error(ERROR_system(1), "%s: not found", *argv);
	else
	{
		while (!sh_checksig(context) && (ent = fts_read(fts)))
			switch (ent->fts_info)
			{
			case FTS_SL:
				if (!(flags & FTS_PHYSICAL) || (flags & FTS_META) && ent->fts_level == 1)
					fts_set(NiL, ent, FTS_FOLLOW);
				break;
			case FTS_F:
				if (sp = openfile(ent->fts_accpath, "rb"))
				{
					pr(&state, sfstdout, sp, ent->fts_path, state.permissions, ent->fts_statp, state.check);
					closefile(sp);
				}
				break;
			case FTS_DC:
				error(ERROR_warn(0), "%s: directory causes cycle", ent->fts_path);
				break;
			case FTS_DNR:
				error(ERROR_system(0), "%s: cannot read directory", ent->fts_path);
				break;
			case FTS_DNX:
				error(ERROR_system(0), "%s: cannot search directory", ent->fts_path);
				break;
			case FTS_NS:
				error(ERROR_system(0), "%s: not found", ent->fts_path);
				break;
			}
		fts_close(fts);
	}
	if (state.total)
	{
		sumprint(state.sum, sfstdout, state.flags|SUM_TOTAL|SUM_SCALE, state.scale);
		sfputc(sfstdout, '\n');
	}
	sumclose(state.sum);
	if (state.check)
		sfclose(state.check);
	return error_info.errors != 0;
}
