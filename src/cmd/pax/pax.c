/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2013 AT&T Intellectual Property          *
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
 * pax -- portable archive interchange
 *
 * test registry:
 *
 *	0000010	dump option table
 *	0000020	force DELTA_TEMP to file
 *	0000040	pretend io device is char special
 *	0000100	don't copy in holes
 *	0000200	sleep(1) between meter displays
 */

static const char usage[] =
"[-?\n@(#)$Id: pax (AT&T Research) 2013-06-24 $\n]"
USAGE_LICENSE
"[+NAME?pax - read, write, and list file archives]"
"[+DESCRIPTION?The pax command reads, writes, and lists archive files in "
    "various formats. There are four operation modes controlled by "
    "combinations of the -\br\b and -\bw\b options.]"
"[+?\bpax -w\b writes the files and directories named by the "
    "\apathname\a arguments to the standard output together with pathname "
    "and status information. A directory \apathname\a argument refers to the "
    "files and (recursively) subdirectories of that directory. If no "
    "\apathname\a arguments are given then the standard input is read to get "
    "a list of pathnames to copy, one pathname per line. In this case only "
    "those pathnames appearing on the standard input are copied.]"
"[+?\bpax -r\b reads the standard input that is assumed to be the result "
    "of a previous \bpax -w\b command. Only member files with names that "
    "match any of the \apattern\a arguments are selected. Matching is done "
    "before any \b-i\b or \b-s\b options are applied. A \apattern\a is given "
    "in the name-generating notation of \bsh\b(1), except that the \b/\b "
    "character is also matched. The default if no \apattern\a is given is "
    "\b*\b which selects all files. The selected files are conditionally "
    "created and copied relative to the current directory tree, subject to "
    "the options described below. By default the owner and group of selected "
    "files will be that of the current user, and the permissions and modify "
    "times will be the same as those in the archive.]"
"[+?\bpax -rw\b reads the files and directories named in the "
    "\apathname\a arguments and copies them to the destination "
    "\adirectory\a. A directory \apathname\a argument refers to the files "
    "and (recursively) subdirectories of that directory. If no \apathname\a "
    "arguments are given then the standard input is read to get a list of "
    "pathnames to copy, one pathname per l, lineine. In this case only those "
    "pathnames appearing on the standard input are copied. \adirectory\a "
    "must exist before the copy.]"
"[+?\bpax\b (\b-r\b and \b-w\b omitted) reads the standard input that is "
    "assumed to be the result of a previous \bpax -w\b command and lists "
    "table of contents of the selected member files on the standard output.]"
"[+?The standard archive formats and compression methods are "
    "automatically detected on input. The default output archive format is "
    "\b\fdefault\f\b, but may be overridden by the \b-x\b option described "
    "below. \bpax\b archives may be concatenated to combine multiple volumes "
    "on a single tape or file. This is accomplished by forcing any format "
    "prescribed pad data to be null bytes. Hard links are not maintained "
    "between volumes, and delta and base archives cannot be multi-volume.]"
"[+?A single archive may span many files/devices. The second and "
    "subsequent file names are prompted for on the terminal input. The "
    "response may be:]"
    "{"
        "[+!command?Execute \acommand\a via \bsystem\b(3) and prompt "
            "again for file name.]"
        "[+EOF?Terminate processing and exit.]"
        "[+CR?An empty input line retains the previous file name.]"
        "[+pathname?The file name for the next archive part.]"
    "}"
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
;

/* state.usage is generated at runtime from usage+options+usage2 */

static const char usage2[] =
"\n"
"[ pathname ... ]\n"
"[ pattern ... ]\n"
"[ pathname ... directory ]\n"
"\n"
"[+DIAGNOSTICS?The number of files, blocks, and optionally the number of "
    "volumes and media parts are listed on the standard error. For -\bv\b "
    "the input archive formats are also listed on the standard error.]"
"[+EXAMPLES]"
    "{"
        "[+pax -w -t 1m .?Copies the contents of the current directory "
            "to tape drive 1, medium density.]"
        "[+mkdir newdir; cd olddir; pax -rw . newdir?Copies the "
            "\aolddir\a directory hierarchy to \anewdir\a.]"
    "}"
"[+SEE ALSO?\bar\b(1), \bcpio\b(1), \bfind\b(1), \bgetconf\b(1), "
    "\bgzip\b(1), \bksh\b(1), \bratz\b(1), \bstar\b(1), \btar\b(1), "
    "\btw\b(1), \bfsync\b(2), \blibdelta\b(3), \bcpio\b(5), \btar\b(5)]"
"[+BUGS?Special privileges may be required to copy special files. Some "
    "archive formats have hard upper limits on member string, numeric and "
    "data sizes. Attribute values larger than the standard min-max values "
    "may cause additional header or embedded data records to be output for "
    "some formats; these records are ignored by old versions of \bcpio\b(1) "
    "and \btar\b(1). \b--format=pax\b avoids size and portability "
    "limitations \abut\a requires a working reader on the receiving side.]"
;

#include "pax.h"

#include <ardir.h>
#include <iconv.h>
#include <tm.h>

char*			definput = "/dev/stdin";
char*			defoutput = "/dev/stdout";
char*			eomprompt = "Change to part %d and hit RETURN: ";

State_t			state;

static int		signals[] =	/* signals caught by interrupt() */
{
	SIGHUP,
	SIGINT,
#if !DEBUG
	SIGQUIT,
#endif
	SIGALRM,
	SIGTERM,
};

static struct
{
	char*		arg0;
	Sfio_t*		ignore_all;
	Sfio_t*		ignore_ext;
	Map_t*		lastmap;
	Sfio_t*		listformat;
	char*		owner;
} opt;

/*
 * clean up dir info before exit
 */

static void
interrupt(int sig)
{
	signal(sig, SIG_IGN);
	switch (sig)
	{
	case SIGINT:
	case SIGQUIT:
		sfprintf(sfstderr, "\n");
		break;
	}
	state.interrupt = sig;
	finish(1);
}

/*
 * enter new substitute expression(s)
 */

static void
substitute(Map_t** lastmap, register char* s)
{
	register Map_t*	mp;
	int		c;

	for (;;)
	{
		while (isspace(*s))
			s++;
		if (!*s)
			break;
		if (!(mp = newof(0, Map_t, 1, 0)))
			error(3, "no space [substitution]");
		if (!(c = regcomp(&mp->re, s, REG_DELIMITED|REG_LENIENT|REG_NULL)))
		{
			s += mp->re.re_npat;
			if (!(c = regsubcomp(&mp->re, s, NiL, 0, 0)))
				s += mp->re.re_npat;
		}
		if (c)
			regfatal(&mp->re, 4, c);
		for (;;)
		{
			switch (*s++)
			{
			case 'i':
				mp->flags |= MAP_INDEX;
				continue;
			}
			s--;
			break;
		}
		if (*s && !isspace(*s))
			error(1, "invalid character after substitution: %s", s);
		if (*lastmap)
			*lastmap = (*lastmap)->next = mp;
		else
			state.maps = *lastmap = mp;
	}
}

/*
 * clear meter line for each error message
 */

static ssize_t
meterror(int fd, const void* buf, size_t n)
{
	if (state.meter.last)
	{
		sfprintf(sfstderr, "%*s\r", state.meter.last, "");
		state.meter.last = 0;
	}
	return write(fd, buf, n);
}

/*
 * construct an action from command
 */

static Filter_t*
action(const char* command, int pattern)
{
	register Filter_t*	fp;
	register char*		s;
	register char*		t;
	register int		c;
	register int		q;
	register int		n;
	regex_t*		re;

	s = (char*)command;
	if (pattern && *s && *s == *(s + 1))
	{
		s += 2;
		pattern = 0;
	}
	if (pattern)
	{
		if (!(re = newof(0, regex_t, 1, 0)))
			nospace();
		if (c = regcomp(re, s, REG_SHELL|REG_AUGMENTED|REG_DELIMITED|REG_LENIENT|REG_NULL|REG_LEFT|REG_RIGHT))
			regfatal(re, 4, c);
		s += re->re_npat;
	}
	else
		re = 0;
	command = (const char*)s;
	q = 0;
	n = 3;
	while (c = *s++)
		if (c == '"' || c == '\'')
		{
			if (!q)
				q = c;
			else if (q == c)
				q = 0;
		}
		else if (c == '\\')
		{
			if (q != '\'' && *s)
				s++;
		}
		else if (!q && isspace(c))
		{
			n++;
			while (isspace(*s))
				s++;
		}
	if (!(fp = newof(0, Filter_t, 1, n * sizeof(char**) + 2 * (s - (char*)command))))
		nospace();
	fp->re = re;
	fp->argv = (char**)(fp + 1);
	fp->command = (char*)(fp->argv + n);
	s = stpcpy(fp->command, command) + 1;
	strcpy(s, command);
	fp->argv[0] = s;
	q = 0;
	n = 1;
	t = s;
	while (c = *s++)
		if (c == '"' || c == '\'')
		{
			if (!q)
				q = c;
			else if (q == c)
				q = 0;
			else
				*t++ = c;
		}
		else if (c == '\\')
		{
			if (q == '\'')
				*t++ = c;
			else if (*s)
				*t++ = *s++;
		}
		else if (q || !isspace(c))
			*t++ = c;
		else
		{
			*t++ = 0;
			while (isspace(*s))
				s++;
			if (*(t = s))
				fp->argv[n++] = s;
		}
	*t = 0;
	fp->patharg = fp->argv + n;
	return fp;
}

/*
 * return action for f, 0 if no match
 */

Filter_t*
filter(Archive_t* ap, File_t* f)
{
	register Filter_t*	fp;

	if (f->st->st_size && (fp = state.filter.list))
		do
		{
			if (!fp->re || !regexec(fp->re, f->name, NiL, 0, 0))
				return fp;
		} while (fp = fp->next);
	return 0;
}

/*
 * set options from line if != 0 or argv according to usage
 * type: 0:command EXTTYPE:extended GLBTYPE:global
 */

void
setoptions(char* line, size_t hdr, char** argv, char* usage, Archive_t* ap, int type)
{
	intmax_t	n;
	int		c;
	int		y;
	int		assignment;
	int		cvt;
	int		index;
	int		offset;
	int		from;
	int		to;
	char*		e;
	char*		s;
	char*		v;
	char*		o;
	char*		end;
	Filter_t*	xp;
	Format_t*	fp;
	Option_t*	op;
	Sfio_t*		sp;
	Value_t*	vp;
	char		tmp1[PATH_MAX];
	char		tmp2[PATH_MAX];

	cvt = 0;
	index = opt_info.index;
	offset = opt_info.offset;
	if (line && hdr)
		end = line + hdr;
	for (;;)
	{
		if (hdr)
		{
			if (!*line)
				break;
			while (isspace(*line))
				line++;
			s = line;
			y = 0;
			while ((c = *line++) >= '0' && c <= '9')
				y = y * 10 + (c - '0');
			if ((e = (s + y - 1)) > end)
				e = end;
			else
				*e++ = 0;
			while (isspace(*line))
				line++;
			o = line;
			assignment = 0;
			for (;;)
			{
				switch (*line++)
				{
				case 0:
					line--;
					break;
				case '=':
					*(line - 1) = 0;
					break;
				case ':':
					if (*line == '=')
					{
						*(line - 1) = 0;
						line++;
						assignment = 1;
						break;
					}
					continue;
				default:
					continue;
				}
				break;
			}
			v = line;
			line = e;
			y = 1;
			if (!(op = (Option_t*)hashget(state.options, o)))
			{
				s = o;
				if (strneq(o, VENDOR ".", sizeof(VENDOR)))
				{
					o += sizeof(VENDOR);
					op = (Option_t*)hashget(state.options, o);
				}
				if (!op && *o == 'n' && *(o + 1) == 'o')
				{
					o += 2;
					y = 0;
					op = (Option_t*)hashget(state.options, o);
				}
				if (!op)
				{
					if (islower(*s) && !strchr(s, '.'))
						error(2, "%s: unknown option", s);
					continue;
				}
			}
			if (!y)
				n = 0;
			else if (!(op->flags & OPT_NUMBER))
				n = 1;
			else
			{
				n = strtonll(v, &e, NiL, 0);
				if (*e)
					error(2, "%s: %s: invalid numeric option value", op->name, v);
			}
		}
		else if (!(c = line ? optstr(line, usage) : optget(argv, usage)))
			break;
		else if (c > 0)
		{
			if (c == '?')
				error(ERROR_USAGE|4, "%s", opt_info.arg);
			if (c == ':' && (!type || islower(*opt_info.name) && !strchr(opt_info.name, '.')))
				error(2, "%s", opt_info.arg);
			continue;
		}
		else
		{
			assignment = opt_info.assignment == ':';
			y = (n = opt_info.number) != 0;
			if (!(v = opt_info.arg))
				v = "";
			else if (!n)
				y = 1;
			op = options - c;
		}

		/*
		 * option precedence levels
		 *
		 *	8	ignore all
		 *	7	command:=
		 *	6	ignore extended
		 *	5	extended:=
		 *	4	extended=
		 *	3	command=
		 *	2	global:=
		 *	1	global=
		 */

		switch (type)
		{
		case EXTTYPE:
			c = 4;
			vp = &op->temp;
			break;
		case GLBTYPE:
			c = 1;
			vp = &op->perm;
			break;
		default:
			c = 3;
			vp = &op->perm;
			break;
		}
		c += assignment;
		message((-5, "option: %c %s level=%d:%d", type ? type : '-', op->name, op->level, c));
		if (op->level > c)
			continue;
		if (y && (op->flags & (OPT_HEADER|OPT_READONLY)) == OPT_HEADER)
		{
			if (vp == &op->temp)
				op->entry = ap->entry;
			else
				op->level = c;
			if (*v)
			{
				op->flags |= OPT_SET;
				if (op->flags & OPT_NUMBER)
					vp->number = n;
				stash(vp, v, 0);
			}
			else
				vp = 0;
		}
		else
			vp = 0;
		message((-4, "option: %c %s%s%s=%s entry=%d:%d level=%d:%d number=%I*u", type ? type : '-', y ? "" : "no", op->name, assignment ? ":" : "", v, op->entry, ap ? ap->entry : 0, op->level, c, sizeof(n), n));
		switch (op->index)
		{
		case OPT_action:
			if (*v)
			{
				xp = action(v, 1);
				if (!xp->re)
					state.filter.all = xp;
				else
				{
					if (state.filter.last)
						state.filter.last->next = xp;
					else
						state.filter.list = xp;
					state.filter.last = xp;
				}
			}
			break;
		case OPT_append:
			state.append = y;
			break;
		case OPT_atime:
			if (vp)
			{
			settime:
				vp->number = strtoul(vp->string, &e, 10);
				vp->fraction = 0;
				if (*e)
				{
					if (*e != '.')
						vp->number = tmdate(vp->string, &e, NiL);
					if (*e == '.')
						vp->fraction = strtoul(s = e + 1, &e, 10);
					if (*e)
						error(2, "%s: invalid %s date string", vp->string, options[op->index].name);
					else if (vp->fraction)
					{
						y = e - s;
						for (y = e - s; y < 9; y++)
							vp->fraction *= 10;
						for (; y > 9; y--)
							vp->fraction /= 10;
					}
				}
			}
			break;
		case OPT_base:
			ap = getarchive(state.operation);
			if (ap->delta)
				error(3, "base archive already specified");
			if (y)
			{
				initdelta(ap, NiL);
				if (!*v || streq(v, "-"))
				{
					state.delta2delta++;
					if (!(state.operation & OUT))
					{
						ap->delta->format = getformat(FMT_IGNORE, 1);
						break;
					}
					v = "/dev/null";
				}
				ap->delta->base = initarchive(strdup(v), O_RDONLY);
			}
			break;
		case OPT_blocksize:
			if (y)
			{
				state.blocksize = n;
				if (state.blocksize < MINBLOCK)
					error(3, "block size must be at least %d", MINBLOCK);
				if (state.blocksize & (BLOCKSIZE - 1))
					error(1, "block size should probably be a multiple of %d", BLOCKSIZE);
			}
			else
				state.blocksize = DEFBLOCKS * BLOCKSIZE;
			break;
		case OPT_blok:
			if (!*v)
				getarchive(IN)->io->blok = getarchive(OUT)->io->blok = n;
			else
				while (*v) switch (*v++)
				{
				case 'i':
					getarchive(IN)->io->blok = 1;
					break;
				case 'o':
					getarchive(OUT)->io->blok = 1;
					break;
				default:
					error(3, "%s: [io] expected", op->name);
					break;
				}
			break;
		case OPT_checksum:
			if (y)
			{
				if (e = strchr(v, ':'))
					*e++ = 0;
				else
				{
					e = v;
					v = "md5";
				}
				state.checksum.name = strdup(e);
				if (!(state.checksum.sum = sumopen(v)))
					error(3, "%s: %s: unknown checksum algorithm", e, v);
			}
			else
				state.checksum.name = 0;
			break;
		case OPT_chmod:
			if (y && *v)
			{
				strperm(v, &e, 0);
				if (*e)
					error(3, "%s: invalid file mode expression", v);
				state.mode = strdup(v);
			}
			else
				state.mode = 0;
			break;
		case OPT_clobber:
			state.clobber = y;
			break;
		case OPT_comment:
			state.header.comment = y ? strdup(v) : (char*)0;
			break;
		case OPT_complete:
			state.complete = y;
			break;
		case OPT_crossdevice:
			if (!y)
				state.ftwflags |= FTW_MOUNT;
			else
				state.ftwflags &= ~FTW_MOUNT;
			break;
		case OPT_ctime:
			if (vp)
				goto settime;
			break;
		case OPT_debug:
			if (y)
			{
				y = error_info.trace;
				error_info.trace = -(int)n;
				if (!y)
					message((-10, "usage %s", usage));
			}
			else
				error_info.trace = 0;
			break;
		case OPT_delete:
			if (y && *v)
				sfprintf(opt.ignore_all, "%s(%s)", sfstrtell(opt.ignore_all) ? "|" : "", v);
			break;
		case OPT_delta_base_checksum:
			if (ap && ap->delta)
				ap->delta->checksum = n;
			break;
		case OPT_delta_base_size:
			if (ap && ap->delta)
				ap->delta->size = n;
			break;
		case OPT_delta_checksum:
			if (ap)
				ap->file.delta.checksum = n;
			break;
		case OPT_delta_compress:
			if (ap && ap->delta)
				ap->delta->compress = 1;
			break;
		case OPT_delta_index:
			if (ap)
			{
				if (type == GLBTYPE)
				{
					if (ap->delta && (c = n - ap->delta->index - 1))
					{
						if (c > 0)
							error(2, "%s: corrupt archive: %d missing file%s", ap->name, c, c == 1 ? "" : "s");
						else
							error(2, "%s: corrupt archive: %d extra file%s", ap->name, -c, -c == 1 ? "" : "s");
					}
				}
				else
					ap->file.delta.index = n;
			}
			break;
		case OPT_delta_method:
			if (ap)
			{
				if (!(fp = getformat(v, 0)) || !(fp->flags & DELTA))
					error(3, "%s: %s: delta method not supported", ap->name, v);
				initdelta(ap, fp);
			}
			break;
		case OPT_delta_op:
			if (ap && ap->delta)
				ap->file.delta.op = line ? *v : n;
			break;
		case OPT_delta_ordered:
			if (ap && ap->delta)
				ap->delta->ordered = n;
			break;
		case OPT_delta_update:
			state.delta.update = y;
			break;
		case OPT_delta_version:
			break;
		case OPT_descend:
			state.descend = y;
			break;
		case OPT_different:
		case OPT_newer:
		case OPT_update:
			state.update = y ? op->index : 0;
			break;
		case OPT_dots:
			state.drop = y;
			break;
		case OPT_edit:
			substitute(&opt.lastmap, (char*)v);
			break;
		case OPT_eom:
			eomprompt = y ? strdup(v) : (char*)0;
			break;
		case OPT_exact:
			state.exact = y;
			break;
		case OPT_extended_name:
			state.header.extended = y ? strdup(v) : (char*)0;
			break;
		case OPT_file:
			ap = getarchive(state.operation);
			if (ap->name)
				error(3, "%s: %s: archive name already specified", v, ap->name);
			ap->name = strdup(v);
			break;
		case OPT_filter:
			if (y && *v)
			{
				state.filter.command = v;
				state.descend = 0;
			}
			else
				state.filter.command = 0;
			break;
		case OPT_format:
			ap = getarchive(state.operation);
			if (!y)
				ap->format = 0;
			else if (s = strdup(v))
			{
				v = s;
				do
				{
					for (e = s, o = 0;;)
					{
						switch (*e++)
						{
						case 0:
							e = 0;
							break;
						case ' ':
						case '\t':
						case '\n':
						case ':':
						case ',':
						case '.':
							*(e - 1) = 0;
							if (*s)
								break;
							s = e;
							continue;
						case '=':
							if (!o)
							{
								*(e - 1) = 0;
								o = e;
							}
							continue;
						default:
							continue;
						}
						break;
					}
					if (!(fp = getformat(s, 0)) && s == v && s[0] == 't' && !strchr(s, ':') && (fp = getformat(s + 1, 0)))
					{
						if (ap->format = getformat("tar", 0))
							s++;
						else
							fp = 0;
					}
					if (!fp)
					{
						if (!pathpath("lib/pax", opt.arg0, PATH_EXECUTE, tmp1, sizeof(tmp1)) || sfsprintf(tmp2, sizeof(tmp2) - 1, "%s/%s.fmt", tmp1, s) <= 0 || !(sp = sfopen(NiL, tmp2, "r")))
							error(3, "%s: unknown archive format", s);
						while (e = sfgetr(sp, '\n', 1))
							if (*e != '#')
							{
								setoptions(e, sfvalue(sp), NiL, state.usage, ap, type);
								if (line && !hdr)
									line += opt_info.offset;
							}
						sfclose(sp);
					}
					else
					{
						fp->details = o;
						switch (fp->flags & (ARCHIVE|COMPRESS|DELTA))
						{
						case ARCHIVE:
							ap->format = fp;
							break;
						case COMPRESS:
							ap->compress = fp;
							break;
						case DELTA:
							initdelta(ap, fp);
							break;
						}
					}
				} while (s = e);
			}
			ap->expected = ap->format;
			if (!state.operation)
				state.format = ap->format;
			break;
		case OPT_from:
		case OPT_to:
			if (!cvt)
			{
				cvt = 1;
				from = to = CC_NATIVE;
			}
			ap = getarchive(state.operation);
			if ((y = ccmapid(v)) < 0)
				error(3, "%s: unknown character code set", v);
			switch (op->index)
			{
			case OPT_from:
				from = y;
				break;
			case OPT_to:
				to = y;
				break;
			}
			break;
		case OPT_global_name:
			state.header.global = y ? strdup(v) : (char*)0;
			break;
		case OPT_header:
			v = y ? strdup(v) : (char*)0;
			if (assignment)
				state.header.extended = v;
			else
				state.header.global = v;
			break;
		case OPT_ignore:
			if (y && *v)
			{
				if (assignment)
					sfprintf(opt.ignore_ext, "%s(%s)", sfstrtell(opt.ignore_ext) ? "|" : "", v);
				else
					sfprintf(opt.ignore_all, "%s(%s)", sfstrtell(opt.ignore_all) ? "|" : "", v);
			}
			break;
		case OPT_install:
			state.install.name = y ? strdup(v) : (char*)0;
			break;
		case OPT_intermediate:
			state.intermediate = y;
			break;
		case OPT_invalid:
			if (line)
			{
				n = 0;
				y = strlen(v);
				s = op->details;
				while (s = strchr(s, '['))
				{
					c = *++s;
					o = ++s;
					for (;;)
					{
						if (strneq(v, o, y))
						{
							s = "";
							n = c;
							break;
						}
						if (!(o = strchr(o, '|')))
							break;
						o++;
					}
				}
			}
			switch ((int)n)
			{
			case 'b':
				state.header.invalid = INVALID_binary;
				break;
			case 'i':
				state.header.invalid = INVALID_ignore;
				break;
			case 'p':
				state.header.invalid = INVALID_prompt;
				break;
			case 't':
				state.header.invalid = INVALID_translate;
				break;
			case 'u':
				state.header.invalid = INVALID_UTF8;
				break;
			default:
				error(2, "%s: %s: unknown option value", op->name, v);
				break;
			}
			break;
		case OPT_invert:
			state.matchsense = !y;
			break;
		case OPT_keepgoing:
			state.keepgoing = y;
			break;
		case OPT_label:
			if (*state.volume)
			{
				if (assignment)
					sfsprintf(tmp1, sizeof(tmp1), "%s %s", v, state.volume);
				else
					sfsprintf(tmp1, sizeof(tmp1), "%s %s", state.volume, v);
				v = tmp1;
			}
			strncpy(state.volume, v, sizeof(state.volume) - 2);
			break;
		case OPT_link:
			if (y)
				state.linkf = link;
			else
				state.linkf = 0;
			break;
		case OPT_linkdata:
			state.header.linkdata = y;
			break;
		case OPT_listformat:
			if (y && *v)
				sfputr(opt.listformat, v, ' ');
			break;
		case OPT_listmacro:
			if (y && *v)
			{
				if (s = strchr(v, '='))
					*s++ = 0;
				if (!(op = (Option_t*)hashget(state.options, v)))
				{
					if (!s)
						break;
					if (!(op = newof(0, Option_t, 1, 0)))
						nospace();
					op->name = hashput(state.options, 0, op);
				}
				if (s)
				{
					op->macro = strdup(s);
					*(s - 1) = 0;
				}
				else
					op->macro = 0;
			}
			break;
		case OPT_local:
			state.local = 1;
			break;
		case OPT_logical:
			if (y)
				state.ftwflags &= ~FTW_PHYSICAL;
			else
				state.ftwflags |= FTW_PHYSICAL;
			break;
		case OPT_maxout:
			state.maxout = n;
			break;
		case OPT_metaphysical:
			if (y)
				state.ftwflags |= FTW_META|FTW_PHYSICAL;
			else
				state.ftwflags &= ~(FTW_META|FTW_PHYSICAL);
			break;
		case OPT_meter:
			if (state.meter.on = y)
			{
				if (!(state.meter.tmp = sfstropen()))
					nospace();
				if (state.meter.fancy = isatty(sffileno(sfstderr)))
				{
					error_info.write = meterror;
					astwinsize(1, NiL, &state.meter.width);
					if (state.meter.width < 2 * (METER_width + 1))
						state.meter.width = 2 * (METER_width + 1);
				}
			}
			break;
		case OPT_mkdir:
			state.mkdir = y;
			break;
		case OPT_mtime:
			if (vp)
				goto settime;
			break;
		case OPT_options:
			if (v)
			{
				setoptions(v, 0, NiL, usage, ap, type);
				if (line && !hdr)
					line += opt_info.offset;
			}
			break;
		case OPT_ordered:
			state.ordered = y;
			break;
		case OPT_owner:
			if (!(state.owner = y))
				opt.owner = 0;
			else if (*v)
				opt.owner = strdup(v);
			break;
		case OPT_passphrase:
			state.passphrase = y ? strdup(v) : (char*)0;
			break;
		case OPT_physical:
			if (y)
			{
				state.ftwflags &= ~FTW_META;
				state.ftwflags |= FTW_PHYSICAL;
			}
			else
				state.ftwflags &= ~FTW_PHYSICAL;
			break;
		case OPT_preserve:
			for (;;)
			{
				switch (*v++)
				{
				case 0:
					break;
				case 'a':
					state.acctime = 0;
					continue;
				case 'e':
					state.acctime = 1;
					state.modtime = 1;
					state.owner = 1;
					state.modemask = 0;
					state.modekeep = 1;
					continue;
				case 'm':
					state.modtime = 0;
					continue;
				case 'o':
					state.owner = 1;
					continue;
				case 'p':
					state.modemask &= (S_ISUID|S_ISGID);
					state.modekeep = 1;
					continue;
				case 's':
					state.modemask &= ~(S_ISUID|S_ISGID);
					continue;
				default:
					error(1, "%s=%c: unknown flag", op->name, *(v - 1));
					continue;
				}
				break;
			}
			break;
		case OPT_read:
			if (y)
				state.operation |= IN;
			else
				state.operation &= ~IN;
			break;
		case OPT_record_charset:
			state.record.charset = y;
			break;
		case OPT_record_delimiter:
			if (!y)
				state.record.delimiter = 0;
			else
				state.record.delimiter = *v;
			break;
		case OPT_record_format:
			state.record.format = y ? *v : 0;
			break;
		case OPT_record_header:
			if (!y)
			{
				state.record.header = 0;
				state.record.headerlen = 0;
			}
			else if (!(state.record.headerlen = stresc(state.record.header = strdup(v))))
				state.record.headerlen = 1;
			break;
		case OPT_record_line:
			state.record.line = y;
			break;
		case OPT_record_match:
			state.record.pattern = y ? strdup(v) : (char*)0;
			break;
		case OPT_record_pad:
			state.record.pad = y;
			break;
		case OPT_record_size:
			state.record.size = n;
			break;
		case OPT_record_trailer:
			if (!y)
			{
				state.record.trailer = 0;
				state.record.trailerlen = 0;
			}
			else if (!(state.record.trailerlen = stresc(state.record.trailer = strdup(v))))
				state.record.trailerlen = 1;
			break;
		case OPT_reset_atime:
			state.resetacctime = y;
			break;
		case OPT_size:
			break;
		case OPT_strict:
			state.strict = y;
			break;
		case OPT_summary:
			state.summary = y;
			break;
		case OPT_symlink:
			if (y)
				state.linkf = pathsetlink;
			else
				state.linkf = 0;
			break;
		case OPT_sync:
#if _lib_fsync
			state.sync = y;
#else
			error(1, "%s not implemented on this system", op->name);
#endif
			break;
		case OPT_tape:
			ap = getarchive(state.operation);
			if (ap->name)
				error(3, "%s: %s: archive name already specified", v, ap->name);
			s = strtape(v, &e);
			if (*s)
				ap->name = s;
			for (;;)
			{
				switch (*e++)
				{
				case 'k':
					if (!(n = strtonll(e, &e, 0, 1)))
						n = -1;
					ap->io->keep = n;
					ap->io->mode = O_RDWR;
					continue;
				case 's':
					if (!(n = strtonll(e, &e, 0, 1)))
						n = -1;
					ap->io->skip = n;
					ap->io->mode = O_RDWR;
					continue;
				}
				e--;
				break;
			}
			if (*e)
				error(3, "%s: invalid tape unit specification [%s]", v, e);
			break;
		case OPT_test:
			if (y)
				state.test |= (unsigned long)n;
			else
				state.test = 0;
			break;
		case OPT_testdate:
			if (y)
			{
				state.testdate = tmdate(v, &e, NiL);
				if (*e)
					error(3, "%s: invalid %s date string", v, options[op->index].name);
			}
			else
				state.testdate = ~0;
			break;
		case OPT_times:
			if (y)
			{
				setoptions("atime:= ctime:= mtime:=", 0, NiL, usage, ap, type);
				if (line && !hdr)
					line += opt_info.offset;
			}
			break;
		case OPT_unblocked:
			if (!*v)
				getarchive(IN)->io->unblocked = getarchive(OUT)->io->unblocked = y;
			else
				while (*v) switch (*v++)
				{
				case 'i':
					getarchive(IN)->io->unblocked = 1;
					break;
				case 'o':
					getarchive(OUT)->io->unblocked = 1;
					break;
				default:
					error(3, "%s: [io] expected", op->name);
					break;
				}
			break;
		case OPT_uncompressed:
			ap->file.uncompressed = n;
			break;
		case OPT_verbose:
			state.verbose = y;
			break;
		case OPT_verify:
			state.verify = y;
			break;
		case OPT_warn:
			state.warn = y;
			break;
		case OPT_write:
			if (y)
				state.operation |= OUT;
			else
				state.operation &= ~OUT;
			if (!(state.operation & IN) && state.in && !state.out)
			{
				state.out = state.in;
				state.in = 0;
				state.out->io->mode = O_CREAT|O_TRUNC|O_WRONLY;
			}
			break;
		case OPT_yes:
			state.verify = state.yesno = y;
			break;
		default:
			if (!type && !(op->flags & OPT_HEADER) || !(op->flags & (OPT_IGNORE|OPT_SET)))
				error(1, "%s: option ignored [index=%d]", op->name, op->index);
			break;
		}
	}
	if (line && !hdr)
	{
		opt_info.index = index;
		opt_info.offset = offset;
	}
	if (cvt)
	{
		ap->convert[0].on = 1;
		convert(ap, SECTION_DATA, from, to);
	}
}

/*
 * option match with VENDOR check
 */

static int
matchopt(const char* name, const char* pattern, Option_t* op)
{
	return strmatch(name, pattern) || (op->flags & OPT_VENDOR) && strmatch(sfprints("%s.%s", VENDOR, name), pattern);
}

/*
 * mark ignored header keywords
 */

static void
ignore(void)
{
	register Option_t*	op;
	Hash_position_t*	pos;
	char*			all;
	char*			ext;
	int			lev;

	if (!sfstrtell(opt.ignore_all))
		all = 0;
	else if (!(all = sfstruse(opt.ignore_all)))
		nospace();
	if (!sfstrtell(opt.ignore_ext))
		ext = 0;
	else if (!(ext = sfstruse(opt.ignore_ext)))
		nospace();
	if ((all || ext) && (pos = hashscan(state.options, 0)))
	{
		while (hashnext(pos))
		{
			op = (Option_t*)pos->bucket->value;
			if (!(op->flags & OPT_READONLY) && (all && matchopt(pos->bucket->name, all, op) && (lev = 8) || ext && matchopt(pos->bucket->name, ext, op) && (lev = 6)) && op->level < lev)
				op->level = lev;
		}
		hashdone(pos);
	}
	sfstrclose(opt.ignore_all);
	sfstrclose(opt.ignore_ext);
}

/*
 * list fp for optinfo()
 */

static void
listformat(register Sfio_t* sp, register Format_t* fp)
{
	register const char*	p;
	register int		c;

	sfprintf(sp, "[+%s", fp->name);
	if (p = fp->match)
	{
		sfputc(sp, '|');
		if (*p == '(')
			p++;
		while (c = *p++)
		{
			if (c == ')' && !*p)
				break;
			if (c == '?' || c == ']')
				sfputc(sp, c);
			sfputc(sp, c);
		}
	}
	sfputc(sp, '?');
	p = fp->desc;
	while (c = *p++)
	{
		if (c == ']')
			sfputc(sp, c);
		sfputc(sp, c);
	}
	switch (fp->flags & (IN|OUT))
	{
	case 0:
		sfputr(sp, "; for listing only", -1);
		break;
	case IN:
		sfputr(sp, "; for input only", -1);
		break;
	case OUT:
		sfputr(sp, "; for output only", -1);
		break;
	}
	sfputc(sp, ']');
}

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register Format_t*	fp;
	register iconv_list_t*	ic;
	register const char*	p;
	register int		i;
	register int		c;
	Ardirmeth_t*		ar;

	switch (*s)
	{
	case 'c':
		for (ic = iconv_list(NiL); ic; ic = iconv_list(ic))
		{
			sfputc(sp, '[');
			sfputc(sp, '+');
			sfputc(sp, '\b');
			p = ic->match;
			if (*p == '(')
				p++;
			while (c = *p++)
			{
				if (c == ')' && !*p)
					break;
				if (c == '?' || c == ']')
					sfputc(sp, c);
				sfputc(sp, c);
			}
			sfputc(sp, '?');
			p = ic->desc;
			while (c = *p++)
			{
				if (c == ']')
					sfputc(sp, c);
				sfputc(sp, c);
			}
			sfputc(sp, ']');
		}
		break;
	case 'd':
		sfputr(sp, FMT_DEFAULT, -1);
		break;
	case 'D':
		fp = 0;
		while (fp = nextformat(fp))
			if (fp->flags & DELTA)
			{
				sfprintf(sp, "[+%s", fp->name);
				sfputc(sp, '?');
				p = fp->desc;
				while (c = *p++)
				{
					if (c == ']')
						sfputc(sp, c);
					sfputc(sp, c);
				}
				sfputc(sp, ']');
			}
		break;
	case 'f':
		fp = 0;
		while (fp = nextformat(fp))
			if (fp->flags & ARCHIVE)
				listformat(sp, fp);
		ar = 0;
		while (ar = ardirlist(ar))
			sfprintf(sp, "[+%s?%s; for input only]", ar->name, ar->description);
		sfprintf(sp, "[+----?compression methods ----]");
		fp = 0;
		while (fp = nextformat(fp))
			if (fp->flags & COMPRESS)
				listformat(sp, fp);
		sfprintf(sp, "[+----?delta methods ----]");
		fp = 0;
		while (fp = nextformat(fp))
			if (fp->flags & DELTA)
				listformat(sp, fp);
				break;
		break;
	case 'l':
		for (i = 1; options[i].name; i++)
			if ((options[i].flags & (OPT_GLOBAL|OPT_READONLY)) == OPT_READONLY)
			{
				sfprintf(sp, "[+%s?%s]\n", options[i].name, options[i].description);
				if (options[i].details)
					sfprintf(sp, "{\n%s\n}", options[i].details);
			}
		sfprintf(sp, "%s",
"	[+----?subformats ----]"
"	[+case\b::\bp\b\a1\a::\bs\b\a1\a::...::\bp\b\an\a::\bs\b\an\a?Expands"
"		to \bs\b\ai\a if the value of \aid\a matches the shell"
"		pattern \bp\b\ai\a, or the empty string if there is no"
"		match.]"
"	[+mode?The integral value as a \bfmtmode\b(3) string.]"
"	[+perm?The integral value as a \bfmtperm\b(3) string.]"
"	[+time[=\aformat\a]]?The integral value as a \bstrftime\b(3)"
"		string. For example,"
"		\b--format=\"%8(mtime)u %(ctime:time=%H:%M:%S)s\"\b"
"		lists the mtime in seconds since the epoch and the"
"		ctime as hours:minutes:seconds.]");
		break;
	}
	return 0;
}

int
main(int argc, char** argv)
{
	register int		i;
	register char*		s;
	register Archive_t*	ap;
	char*			p;
	Hash_position_t*	pos;
	Option_t*		op;
	int			n;
	int			pass = 0;
	unsigned long		blocksize;
	struct stat		st;
	Optdisc_t		optdisc;

	static Format_t		rw = { "rw", 0, 0, 0, IN|OUT };

	setlocale(LC_ALL, "");
	paxinit(&state, error_info.id = "pax");
	optinit(&optdisc, optinfo);
	state.strict = !!conformance(0, 0);
	state.gid = getegid();
	state.uid = geteuid();
	state.pid = getpid();
	umask(state.modemask = umask(0));
	state.modemask |= S_ISUID|S_ISGID;
	state.ftwflags = ftwflags()|FTW_DOT;
	state.acctime = 1;
	state.buffersize = DEFBUFFER * DEFBLOCKS;
	state.clobber = 1;
	state.delta.buffersize = DELTA_WINDOW >> 1;
	state.descend = RESETABLE;
	state.format = getformat(FMT_DEFAULT, 1);
	state.header.extended = state.strict ? HEADER_EXTENDED_STD : HEADER_EXTENDED;
	state.header.global = state.strict ? HEADER_GLOBAL_STD : HEADER_GLOBAL;
	state.map.a2n = ccmap(CC_ASCII, CC_NATIVE);
	state.map.e2n = ccmap(CC_EBCDIC, CC_NATIVE);
	state.map.n2e = ccmap(CC_NATIVE, CC_EBCDIC);
	if (!(opt.ignore_all = sfstropen()) || !(opt.ignore_ext = sfstropen()))
		nospace();
	if (!(opt.listformat = sfstropen()))
		nospace();
	state.matchsense = 1;
	state.mkdir = 1;
	state.modtime = 1;
	if (!(state.tmp.fmt = sfstropen()) || !(state.tmp.lst = sfstropen()) || !(state.tmp.str = sfstropen()))
		nospace();
	stash(&options[OPT_release].perm, release(), 0);
	options[OPT_release].flags |= OPT_SET;
	if (!(state.options = hashalloc(NiL, HASH_name, "options", 0)))
		nospace();
	for (i = 1; options[i].name; i++)
	{
		p = options[i].name;
		if (strchr(p, '|'))
			p = strdup(p);
		do
		{
			if (s = strchr(p, '|'))
				*s++ = 0;
			hashput(state.options, p, &options[options[i].index]);
		} while (p = s);
	}
	hashset(state.options, HASH_ALLOCATE);
	state.record.charset = 1;
	state.record.line = 1;
	state.summary = 1;
	state.testdate = ~0;
	if (!(state.tmp.file = pathtemp(NiL, 0, NiL, error_info.id, NiL)))
		nospace();
	sfputr(state.tmp.str, usage, -1);
	for (i = 1; options[i].name; i++)
		if ((options[i].flags & (OPT_GLOBAL|OPT_READONLY)) != OPT_READONLY)
		{
			sfputc(state.tmp.str, '[');
			if (options[i].flag)
			{
				sfputc(state.tmp.str, options[i].flag);
				if (options[i].flags & OPT_INVERT)
					sfputc(state.tmp.str, '!');
			}
			sfprintf(state.tmp.str, "=%d:%s", options[i].index, options[i].name);
			if (options[i].flags & OPT_VENDOR)
			{
				for (s = (char*)options[i].name; p = strchr(s, '|'); s = p + 1)
					sfprintf(state.tmp.str, "|%s.%-.*s", VENDOR, p - s, s);
				sfprintf(state.tmp.str, "|%s.%s", VENDOR, s);
			}
			sfprintf(state.tmp.str, "?%s]", options[i].description);
			if (options[i].argument)
			{
				sfputc(state.tmp.str, (options[i].flags & OPT_NUMBER) ? '#' : ':');
				if (options[i].flags & OPT_OPTIONAL)
					sfputc(state.tmp.str, '?');
				sfprintf(state.tmp.str, "[%s]", options[i].argument);
			}
			if (options[i].details)
				sfprintf(state.tmp.str, "\n{%s}", options[i].details);
			sfputc(state.tmp.str, '\n');
		}
	sfputr(state.tmp.str, usage2, -1);
	if (!(state.usage = sfstruse(state.tmp.str)))
		nospace();
	opt.arg0 = argv[0];
	setoptions(NiL, 0, argv, state.usage, NiL, 0);
	argv += opt_info.index;
	argc -= opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!state.operation)
	{
		state.operation = IN;
		state.list = 1;
	}
	if (!sfstrtell(opt.listformat))
		sfputr(opt.listformat, (state.list && state.verbose) ? "%(mode)s %2(nlink)d %-8(uname)s %-8(gname)s%8(device:case::%(size)llu:*:%(device)s)s %(mtime)s %(delta.op:case:?*:%(delta.op)s )s%(path)s%(linkop:case:?*: %(linkop)s %(linkpath)s)s" : "%(delta.op:case:?*:%(delta.op)s )s%(path)s%(linkop:case:?*: %(linkop)s %(linkpath)s)s", ' ');
	sfstrseek(opt.listformat, -1, SEEK_CUR);
	if (!state.meter.on)
		sfputc(opt.listformat, '\n');
	if (!(state.listformat = strdup(sfstruse(opt.listformat))))
		nospace();
	sfstrclose(opt.listformat);
	ignore();
	if (s = state.filter.command)
	{
		if (streq(s, "-"))
		{
			state.filter.line = -1;
			s = "sh -c";
		}
		state.filter.all = action(s, 0);
	}
	if (state.filter.last)
		state.filter.last->next = state.filter.all;
	else
		state.filter.list = state.filter.all;
	state.statf = (state.ftwflags & FTW_PHYSICAL) ? lstat : pathstat;

	/*
	 * determine the buffer sizes
	 */

	switch (state.operation)
	{
	case IN|OUT:
		if (!state.in)
			break;
		/*FALLTHROUGH*/
	case IN:
	case OUT:
		getarchive(state.operation);
		break;
	}
	blocksize = state.blocksize;
	if (ap = state.out)
	{
		if (!ap->format)
			ap->format = state.format;
		else if (state.operation == (IN|OUT))
			pass = 1;
		if (state.operation == OUT)
		{
			if (state.files)
				state.ftwflags |= FTW_POST;
		}
		if (state.append || state.update)
			ap->io->mode = O_RDWR|O_CREAT;
		ap->io->fd = 1;
		if (!ap->name || streq(ap->name, "-"))
			ap->name = defoutput;
		else
		{
			close(1);
			if (open(ap->name, ap->io->mode|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) != 1)
				error(ERROR_SYSTEM|3, "%s: cannot write", ap->name);
		}
		if (fstat(ap->io->fd, &st))
			error(ERROR_SYSTEM|3, "%s: cannot stat", ap->name);
		if (S_ISREG(st.st_mode))
		{
			ap->io->seekable = 1;
			ap->io->size = st.st_size;
		}
		if (!state.blocksize)
		{
			st.st_mode = modex(st.st_mode);
			if (state.test & 0000040)
				st.st_mode = X_IFCHR;
			if (X_ITYPE(st.st_mode) == X_IFREG)
			{
				state.blocksize = ap->format->regular;
				ap->io->unblocked = 1;
			}
			else
				state.blocksize = ap->format->special;
			state.buffersize = state.blocksize *= BLOCKSIZE;
		}
	}
	else
	{
		if (state.blocksize)
			state.buffersize = state.blocksize;
		else
			state.blocksize = (state.operation == (IN|OUT) ? FILBLOCKS : DEFBLOCKS) * BLOCKSIZE;
		if (state.record.size)
			error(1, "record size automatically determined on archive read");
	}
	if (ap = state.in)
	{
		if (!ap->name || streq(ap->name, "-"))
			ap->name = definput;
		else
		{
			close(0);
			if (open(ap->name, ap->io->mode|O_BINARY))
				error(ERROR_SYSTEM|3, "%s: cannot read", ap->name);
		}
		if (fstat(ap->io->fd, &st))
			error(ERROR_SYSTEM|3, "%s: cannot stat", ap->name);
		if (S_ISREG(st.st_mode))
		{
			ap->io->seekable = 1;
			ap->io->size = st.st_size;
		}
		if (state.meter.on && !(state.meter.size = ap->io->size))
			state.meter.on = 0;
	}
	if (!blocksize && (blocksize = bblock(!state.in)))
		state.blocksize = blocksize;
	if (state.buffersize < state.blocksize)
		state.buffersize = state.blocksize;
	state.tmp.buffersize = state.buffersize;
	if (!(state.tmp.buffer = newof(0, char, state.tmp.buffersize, 0)))
		nospace();
	if (state.maxout && state.maxout <= state.blocksize)
		error(3, "--maxout=%#i must be greater than --blocksize=%#i", state.maxout, state.blocksize);
	message((-1, "blocksize=%d buffersize=%d recordsize=%d", state.blocksize, state.buffersize, state.record.size));

	/*
	 * initialize the main io
	 */

	switch (state.operation)
	{
	case IN:
	case OUT:
		getarchive(state.operation);
		break;
	}
	if (ap = state.in)
	{
		binit(ap);
		if (state.append && !state.out)
		{
			error(1, "append ignored for archive read");
			state.append = 0;
		}
	}
	if (ap = state.out)
	{
		if (!ap->format)
			ap->format = state.format;
		if (state.append || state.update)
		{
			if (ap->delta)
			{
				error(1, "append/update ignored for archive delta");
				state.update = 0;
			}
			if (state.append && state.update)
			{
				error(1, "append ignored for archive update");
				state.append = 0;
			}
			if (!ap->io->seekable)
				error(3, "%s: append/update requires seekable archive", ap->name);
			else if (!ap->io->size)
				state.append = state.update = 0;
			else
			{
				initdelta(ap, NiL);
				ap->delta->base = ap;
			}
		}
		binit(ap);
		if (ap->compress)
		{
			Proc_t*		proc;
			List_t*		p;
			char*		cmd[4];

			cmd[0] = ap->compress->name;
			i = 1;
			if (cmd[i] = ((Compress_format_t*)ap->compress->data)->variant)
				i++;
			if (cmd[i] = ap->compress->details)
				i++;
			cmd[i] = 0;
			if (!(proc = procopen(*cmd, cmd, NiL, NiL, PROC_WRITE)))
				error(3, "%s: cannot execute %s filter", ap->name, ap->compress->name);
			n = proc->wfd;
			proc->wfd = 1;
			close(1);
			if (dup(n) != 1)
				error(3, "%s: cannot redirect %s filter output", ap->name, ap->compress->name);
			close(n);
			if (!(p = newof(0, List_t, 1, 0)))
				nospace();
			p->item = (void*)proc;
			p->next = state.proc;
			state.proc = p;
		}
		if (state.checksum.name)
		{
			if (!(state.checksum.path = pathtemp(NiL, 0, NiL, error_info.id, NiL)))
				nospace();
			if (!(state.checksum.sp = sfopen(NiL, state.checksum.path, "w")))
				error(3, "%s: cannot write checksum temporary", state.checksum.path);
			sfprintf(state.checksum.sp, "method=%s\n", state.checksum.sum->name);
			sfprintf(state.checksum.sp, "permissions\n");
		}
		if (state.install.name)
		{
			if (!(state.install.path = pathtemp(NiL, 0, NiL, error_info.id, NiL)))
				nospace();
			if (!(state.install.sp = sfopen(NiL, state.install.path, "w")))
				error(3, "%s: cannot write install temporary", state.install.path);
		}
	}
	if (!(state.linktab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(Fileid_t), HASH_name, "links", 0)))
		error(3, "cannot allocate hard link table");
	if ((state.operation & IN) && !state.list && !(state.restore = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_name, "restore", 0)))
		error(3, "cannot allocate directory table");
	if (state.owner)
	{
		if (state.operation & IN)
		{
			state.modemask = 0;
			if (opt.owner)
			{
				if ((state.setuid = struid(opt.owner)) < 0 || (state.setgid = strgid(opt.owner)) < 0)
					error(3, "%s: invalid user name", opt.owner);
				state.flags |= SETIDS;
			}
		}
		else
			error(1, "ownership assignment ignored on archive write");
	}
	if (state.verify)
		interactive();
	if (!(state.modemask &= (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)))
		umask(0);
	state.modemask = ~state.modemask;
#if DEBUG
	if ((state.test & 0000010) && (pos = hashscan(state.options, 0)))
	{
		while (hashnext(pos))
		{
			op = (Option_t*)pos->bucket->value;
			if (op->name == pos->bucket->name)
				sfprintf(sfstderr, "%-16s %c %2d %d perm=%ld:%s temp=%ld:%s%s%s\n", op->name, op->flag ? op->flag : '-', op->index, op->level, op->perm.number, op->perm.string, op->temp.number, op->temp.string, (op->flags & OPT_HEADER) ? " HEADER" : "", (op->flags & OPT_READONLY) ? " READONLY" : "");
		}
		hashdone(pos);
	}
#endif
	for (i = 0; i < elementsof(signals); i++)
		if (signal(signals[i], interrupt) == SIG_IGN)
			signal(signals[i], SIG_IGN);
	switch (state.operation)
	{
	case IN:
		if (*argv)
		{
			initmatch(argv);
			if (state.exact)
				state.matchsense = 1;
		}
		else if (state.exact)
			error(3, "file arguments expected");
		getcwd(state.pwd, PATH_MAX);
		state.pwdlen = strlen(state.pwd);
		if (state.pwdlen > 1)
			state.pwd[state.pwdlen++] = '/';
		copyin(state.in);
		if (state.exact)
			for (state.pattern = state.patterns; state.pattern->pattern; state.pattern++)
				if (!state.pattern->matched)
					error(2, "%s: %s: file not found in archive", state.in->name, state.pattern->pattern);
		break;

	case OUT:
		if (*argv)
			state.files = argv;
		if (!state.maxout && state.complete)
			error(3, "maximum block count required");
		copy(state.out, copyout);
		break;

	case (IN|OUT):
		if (pass || state.in || state.out)
		{
			state.pass = 1;
			if (*argv)
				initmatch(argv);
			deltapass(getarchive(IN), getarchive(OUT));
		}
		else
		{
			if (--argc < 0)
			{
				error(2, "destination directory required for pass mode");
				error(ERROR_USAGE|4, "%s", optusage(NiL));
			}
			state.destination = argv[argc];
			argv[argc] = 0;
			if (*argv)
				state.files = argv;
			if (state.record.size)
				error(1, "record size ignored in pass mode");

			/*
			 * initialize destination dir
			 */

			pathcanon(state.destination, 0, 0);
			if (stat(state.destination, &st) || !S_ISDIR(st.st_mode))
				error(3, "%s: destination must be a directory", state.destination);
			state.dev = st.st_dev;
			strcpy(state.pwd, state.destination);
			if (state.pwdlen = strlen(state.pwd))
				state.pwd[state.pwdlen++] = '/';
			if (state.update < 0)
				state.update = OPT_newer;
			getarchive(OUT);
			state.out->format = &rw;
			copy(NiL, copyinout);
		}
		break;
	}
	finish(0);
}

/*
 * print number of blocks actually copied and exit
 */

void
finish(int code)
{
	register Archive_t*	ap;
	size_t			x = state.buffersize / 4;
	register char*		x1 = &state.tmp.buffer[0];
	register char*		x2 = x1 + x;
	register char*		x3 = x2 + x;
	register char*		x4 = x3 + x;
	register off_t		n;

	while (state.proc)
	{
		procclose((Proc_t*)state.proc->item);
		state.proc = state.proc->next;
	}
	remove(state.tmp.file);
	if (state.checksum.path)
		remove(state.checksum.path);
	if (state.install.path)
		remove(state.install.path);
	if (state.restore)
		hashwalk(state.restore, 0, restore, NiL);
	sfsync(sfstdout);
	if (state.meter.last)
	{
		sfprintf(sfstderr, "%*s\r", state.meter.last, "");
		state.meter.last = 0;
	}
	else if (state.dropcount)
	{
		sfprintf(sfstderr, "\n");
		sfsync(sfstderr);
	}
	if (state.summary)
	{
		ap = getarchive(state.operation);
		n = ap->io->count + ap->io->expand + ap->io->offset;
		message((-1, "%s totals entries=%d count=%I*d expand=%I*d offset=%I*d BLOCKSIZE=%I*d n=%I*d blocks=%I*d", ap->name, ap->entries, sizeof(ap->io->count), ap->io->count, sizeof(ap->io->expand), ap->io->expand, sizeof(ap->io->offset), ap->io->offset, sizeof(BLOCKSIZE), BLOCKSIZE, sizeof(n), n, sizeof(n), (n + BLOCKSIZE - 1) / BLOCKSIZE));
		if (ap->entries)
		{
			if (ap->volume > 1)
				sfsprintf(x1, x, ", %d volumes", ap->volume);
			else
				*x1 = 0;
			if (ap->volume > 0 && ap->part > ap->volume)
				sfsprintf(x2, x, ", %d parts", ap->part - ap->volume + 1);
			else
				*x2 = 0;
			n = (n + BLOCKSIZE - 1) / BLOCKSIZE;
			if (state.verbose || state.meter.on)
			{
				sfsprintf(x3, x, "%I*u file%s, ", sizeof(ap->selected), ap->selected, ap->selected == 1 ? "" : "s");
				if (state.update)
					sfsprintf(x4, x, "%I*u updated, ", sizeof(ap->updated), ap->updated);
				else
					*x4 = 0;
			}
			else
				*x3 = *x4 = 0;
			sfprintf(sfstderr, "%s%s%I*d block%s%s%s\n", x3, x4, sizeof(n), n, n == 1 ? "" : "s", x1, x2);
		}
	}
	sfsync(sfstderr);
	if (state.interrupt)
	{
		signal(state.interrupt, SIG_DFL);
		kill(getpid(), state.interrupt);
		pause();
	}
	exit(code ? code : error_info.errors != 0);
}

/*
 * return release stamp
 */

char*
release(void)
{
	register char*	b;
	register char*	s;
	register char*	t;

	if ((s = strchr(usage, '@')) && (t = strchr(s, '\n')) && (b = fmtbuf(t - s + 1)))
	{
		memcpy(b, s, t - s);
		b[t - s] = 0;
	}
	else
		b = fmtident(usage);
	return b;
}
