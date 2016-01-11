/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2012 AT&T Intellectual Property          *
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
 */

static const char usage[] =
"[-?\n@(#)$Id: htmlrefs (AT&T Research) 2012-01-01 $\n]"
USAGE_LICENSE
"[+NAME?htmlrefs - list html url references]"
"[+DESCRIPTION?\bhtmlrefs\b lists url references from the"
"	local closure of the input \bhtml\b \afile\as. If \afile\a is not"
"	specified then the top level default user file is read. The \bhtml\b"
"	parse is rudimentary; don't use \bhtmlrefs\b to detect valid \bhtml\b"
"	files.]"
"[+?The top level references are determined in this order (the \b--index\b,"
"	\b--root\b and \b--user\b options influence the order):]{"
"	[+$HOME/index.html?Pseudo index containing"
"		\b<LINK href=\b\adir\a \brel=\b\atype\a\b>\b references to"
"		top level directories. \atype\a may be one of:]{"
"		[+document-root?The document root directory containing URL"
"			target documents. Exactly one \bdocument-root\b must"
"			be specified.]"
"		[+program-root?The program root directory containing CGI"
"			support programs and scripts. This type is optional."
"			If specified then the program root directory should"
"			contain a pseudo index for its references.]"
"		[+data-root?The data root directory containing CGI"
"			support data. This type is optional. If specified then"
"			the data root directory should contain a pseudo index"
"			for its references.]"
"		[+dynamic?All files under \adir\a are considered referenced.]"
"		[+host?Provides a default value for the \b--hosts\b option.]"
"		[+ignore?\adir\a is a \bksh\b(1) pattern of paths to ignore.]"
"		[+internal?If \b--external\b is on then \adir\a is a \bksh\b(1)"
"			pattern of internal paths.]"
"		[+secure?Files under this dir are accessed by \bhttps:\b only.]"
"	}"
"	[+$HOME/wwwfiles/index.html?]"
"	[+$HOME/public_html/index.html?]"
"}"
"[a:all?List all references whether they exist or not.]"
"[c:copy?Copy the selected references to \adirectory\a which must already"
"	exist. If \b--external\b is also specified then lines between"
"	\b<!--INTERNAL-->\b ... \b<!--/INTERNAL-->\b lines are not"
"	copied. If \b--unreferenced\b is also specified then files and"
"	directories in \adirectory\a that have not been copied are"
"	removed. Target file modification times are set to match source"
"	times so that future copies can be avoided.]:[directory]"
"[d:dependents?List each selected local file followed by \b:\b and a list of"
"	all local files referring to the file.]"
"[e:external?Do not list references inside \b<!--INTERNAL-->\b ..."
"	\b<!--/INTERNAL-->\b lines. See \bmm2html\b(1) for an html"
"	generator that inserts these lines.]"
"[F:force?By default files are not copied if the source and target size and"
"	modification times match. \b--force\b forces all files to be copied.]"
"[h:hosts?Check only references matching the \bksh\b(1) pattern"
"	\bhttp://\b\apattern\a\b/\b.]:[pattern]"
"[i:index?\aname\a specifies the page named by directory"
"	references.]:[name:=index.html]"
"[k:keep?\apattern\a is used to match file base names that are always"
"	considered referenced.]:[pattern:=.htaccess]"
"[l:limit?Limit \b--copy\b and \b--remove\b operations to path names matching"
"	\apattern\a.]:[pattern]"
"[m:missing?List missing local file references.]"
"[n!:exec?Enable file modification operations. \b--noexec\b lists the"
"	operations but does not do them.]"
"[p!:perlwarn?Check HTML files for unintentional embedded \bperl\b(1)"
"	constructs: a left bracket followed by one of \b-+!$*#\b. Manually"
"	translating left bracket to \b&#0091;\b avoids unwanted \bperl\b"
"	interactions (why didn't they use tags like everyone else?)"
"	\bmm2html\b(1) and \boptget\b(3) do the translation by default.]"
"[X:remove?Unreferenced files are removed when \b--unreferenced\b and"
"	\b--nocopy\b are specified.]"
"[r:root?The local \adirectory\a for \b--user\b"
"	references.]:[directory:=~\auser\a]"
"[K:skip?\apattern\a is used to match file base names that are never"
"	considered referenced.]:[pattern:=00-INDEX-00]"
"[s:strict?By default unreferenced \b--index\b files and the containing"
"	directory are considered referenced; \b--strict\b considers"
"	unreferenced \b--index\b files unreferenced.]"
"[S:symlink?Instruct \b--copy\b to \bsymlink\b(2) files that do not contain"
"	\b<!--INTERNAL-->\b ... \b<!--/INTERNAL-->\b or are not in"
"	\b/cgi-bin/\b.]"
"[u:user?\b~\b\aname\a translates to the \b--root\b"
"	directory.]:[name:=caller-uid]"
"[v:verbose?List files as they are copied (see \b--copy\b.)]"
"[w:warn?Produce a warning diagnostic for missing files.]"
"[x:unreferenced?If \b--copy\b is also specified then remove files and"
"	directories in the \b--copy\b \adirectory\a that have not been copied."
"	Otherwise list unreferenced files in the \b--root\b directory."
"	A directory that contains no referenced files but does contain an"
"	\b--index\b file is considered referenced (along with the \b--index\b"
"	file) unless \b--strict\b is enabled.]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+EXAMPLES]{"
"	[+htmlrefs --hosts=www.research.att.com --missing?List missing"
"		references to the local host \bwww.research.att.com\b.]"
"	[+htmlrefs -n -h www.research.att.com -c ~/external/wwwfiles -e -x?Copy"
"		the local hierarchy to \b~/external/wwwfiles\b for external"
"		release, and remove unreferenced files in the copy.]"
"}"
"[+SEE ALSO?\bhtml2rtf\b(1), \bmm2html\b(1)]"
;

#include <ast.h>
#include <cdt.h>
#include <ctype.h>
#include <error.h>
#include <fts.h>
#include <glob.h>
#include <pwd.h>
#include <tm.h>

#define INDEX			"index.html"
#define KEEP			".htaccess"
#define SKIP			"00-INDEX-00"

#define CHECKED			0x001
#define COPIED			0x002
#define COPY			0x004
#define DIRECTORY		0x008
#define EXTERNAL		0x010
#define FILTER			0x020
#define INTERNAL		0x040
#define MISSING			0x080
#define SCANNED			0x100
#define SECURE			0x200
#define VERBOSE			0x400

#define HIT			(-1)
#define MISS			(-2)

#define STUFF(s, buf, c)	((s < &buf[sizeof(buf)]) ? (*s++ = c) : -1)

struct List_s;

typedef struct String_s
{
	char*		data;
	unsigned int	size;
} String_t;

typedef struct File_s
{
	Dtlink_t	link;
	unsigned long	time;
	unsigned int	flags;
	struct List_s*	refs;
	char		name[1];
} File_t;

typedef struct List_s
{
	struct List_s*	next;
	File_t*		file;
} List_t;

typedef struct State_s
{
	Dtdisc_t	disc;
	Dt_t*		files;

	int		all;
	int		dependents;
	int		exec;
	int		external;
	int		force;
	int		missing;
	int		more;
	int		perlwarn;
	int		remove;
	int		strict;
	int		symlink;
	int		unreferenced;
	int		verbose;
	int		warn;

	String_t	copy;
	String_t	dataroot;
	String_t	documentroot;
	String_t	hosts;
	String_t	ignore;
	String_t	index;
	String_t	internal;
	String_t	keep;
	String_t	limit;
	String_t	programroot;
	String_t	root;
	String_t	skip;
	String_t	user;

	char		buf[PATH_MAX];
	char		dir[PATH_MAX];
	char		tmp[PATH_MAX];
} State_t;

static const char	internal[] = "<!--INTERNAL-->";
static const char	external[] = "<!--/INTERNAL-->";

static int
keep(State_t* state, const char* name, int mode)
{
	char*	s;

	if (state->skip.size)
	{
		if (s = strrchr(name, '/'))
			s++;
		else
			s = (char*)name;
		if (strmatch(s, state->skip.data))
			return 0;
	}
	if (mode >= 0 && access(name, mode))
		return 0;
	return 1;
}

/*
 * check for glob(dir/name)
 */

static void
check(register State_t* state, const char* dir, const char* name, unsigned int flags)
{
	register File_t*	dp;
	register char*		s;
	register char**		p;
	glob_t			gl;

	memset(&gl, 0, sizeof(gl));
	sfsprintf(state->dir, sizeof(state->dir) - 1, "%s/(%s)", dir, name);
	if (!glob(state->dir, GLOB_AUGMENTED|GLOB_DISC|GLOB_STACK, 0, &gl))
		for (p = gl.gl_pathv; s = *p++;)
			if (!dtmatch(state->files, s) && keep(state, s, F_OK))
			{
				if (!(dp = newof(0, File_t, 1, strlen(s))))
					error(ERROR_SYSTEM|3, "out of space [file]");
				strcpy(dp->name, s);
				dtinsert(state->files, dp);
				dp->flags |= flags;
			}
}

/*
 * add reference path s
 */

static File_t*
add(register State_t* state, register char* s, unsigned int flags, const char* path, int prefix, File_t* ref)
{
	register char*		t;
	register File_t*	fp;
	register File_t*	dp;
	register List_t*	lp;
	char*			u;
	struct stat		st;

	if (!(flags & COPIED))
	{
		if (ref && (ref->flags & SECURE))
			flags |= SECURE;
		if (state->hosts.size)
		{
			if (t = strchr(s, ':'))
			{
				if (strneq(s, "http://", t - s + 3))
				{
					s = t + 3;
					flags &= ~SECURE;
				}
				else if (strneq(s, "https://", t - s + 4))
				{
					s = t + 4;
					flags |= SECURE;
				}
				else
					return 0;
				if (t = strchr(s, '/'))
					*t = 0;
				if (!strmatch(s, state->hosts.data))
					return 0;
				if (t)
					*(s = t) = '/';
				else
					s = "/";
			}
			if (*s == '/')
			{
				if (ref && !streq(s, ref->name))
				{
					if (*(s + 1) != '~')
						return 0;
					if (*(s + 2) == '/')
						s += 2;
					else if (!state->user.size || !strneq(s + 2, state->user.data, state->user.size) || *(s + 2 + state->user.size) != '/')
						return 0;
					else
						s += 2 + state->user.size;
					if (state->documentroot.size)
					{
						sfsprintf(state->buf, sizeof(state->buf) - 1, "%s%s%s", state->documentroot.data, (flags & SECURE) ? "/secure" : "", s);
						pathcanon(s = state->buf, sizeof(state->buf), 0);
					}
					else if (state->root.size)
					{
						sfsprintf(state->buf, sizeof(state->buf) - 1, "%s%s", state->root.data, (flags & SECURE) ? "/secure" : "", s);
						pathcanon(s = state->buf, sizeof(state->buf), 0);
					}
				}
			}
			else if (prefix)
			{
				sfsprintf(state->buf, sizeof(state->buf) - 1, "%-.*s%s", prefix, path, s);
				pathcanon(s = state->buf, sizeof(state->buf), 0);
			}
			else if (flags & SECURE)
			{
				sfsprintf(state->tmp, sizeof(state->tmp), "secure/%s", s);
				s = state->tmp;
			}
		}
		if (*s == '.' && *(s + 1) == '/')
			while (*++s == '/');
		if (!*s)
			s = "/";
		for (t = s + strlen(s); t > s && *(t - 1) == '/'; t--);
		if (*t == '/' || !stat(s, &st) && S_ISDIR(st.st_mode))
		{
			if (s >= state->buf && s < state->buf + sizeof(state->buf))
			{
				if (!*t)
					*t = '/';
				sfsprintf(t + 1, sizeof(state->buf) - (t - s + 2), "%s", state->index.data);
			}
			else
			{
				sfsprintf(state->buf, sizeof(state->buf) - 1, "%-.*s/%s", t - s, s, state->index.data);
				s = state->buf;
			}
		}
	}
	if (!(fp = (File_t*)dtmatch(state->files, s)))
	{
		if (!keep(state, s, -1))
			return 0;
		if (!(fp = newof(0, File_t, 1, strlen(s))))
			error(ERROR_SYSTEM|3, "out of space [file]");
		strcpy(fp->name, s);
		dtinsert(state->files, fp);
		state->more = 1;
		if (t = strrchr(s, '/'))
			do
			{
				*t = 0;
				if (dp = (File_t*)dtmatch(state->files, s))
				{
					*t = '/';
					break;
				}
				if (!(dp = newof(0, File_t, 1, strlen(s))))
					error(ERROR_SYSTEM|3, "out of space [file]");
				strcpy(dp->name, s);
				dtinsert(state->files, dp);
				dp->flags |= DIRECTORY|flags;
				if (!(flags & COPIED))
				{
					if (!state->strict)
						check(state, s, state->index.data, flags);
					if (state->keep.size)
						check(state, s, state->keep.data, flags);
				}
				u = strrchr(s, '/');
				*t = '/';
			} while ((t = u) && (t - s) > state->root.size);
	}
	fp->flags |= flags;
	if (ref && state->dependents)
	{
		for (lp = fp->refs; lp && lp->file != ref; lp = lp->next);
		if (!lp)
		{
			if (!(lp = newof(0, List_t, 1, 0)))
				error(ERROR_SYSTEM|3, "out of space [file]");
			lp->file = ref;
			lp->next = fp->refs;
			fp->refs = lp;
		}
	}
	return fp;
}

/*
 * order directory stream by name
 */

static int
order(FTSENT* const* a, FTSENT* const* b)
{
	return strcmp((*a)->fts_name, (*b)->fts_name);
}

/*
 * parse and set root dir r from s
 * possibly using tmp buffer buf
 */

static void
rootdir(State_t* state, register String_t* r, register char* s, char* buf, size_t z)
{
	register char*	t;
	register int	n;

	if (t = strrchr(s, '/'))
		*t = 0;
	if (*s == '/')
		n = strlen(s);
	else
	{
		n = sfsprintf(buf, z, "%s/%s", state->root.data, s);
		s = buf;
	}
	if (!(r->data = strdup(s)))
		error(ERROR_SYSTEM|3, "out of space [rootdir]");
	r->size = n;
	if (t)
		*t = '/';
}

/*
 * return next directory entry
 */

static FTSENT*
scan(State_t* state, FTS* fts)
{
	FTSENT*	ent;
	Sfio_t*	sp;
	char*	s;
	int	skip;

	while (ent = fts_read(fts))
	{
		if (state->external && ent->fts_info == FTS_D)
		{
			sfsprintf(state->buf, sizeof(state->buf) - 1, "%s/%s", ent->fts_path, state->index.data);
			if (sp = sfopen(NiL, state->buf, "r"))
			{
				skip = 0;
				while (s = sfgetr(sp, '\n', 1))
				{
					if (strgrpmatch(s, internal, NiL, 0, 0))
					{
						skip = 1;
						break;
					}
					else if (strgrpmatch(s, "</HEAD>", NiL, 0, STR_ICASE))
						break;
				}
				sfclose(sp);
				if (skip)
				{
					if (fts_set(NiL, ent, FTS_SKIP))
						error(1, "%s: cannot skip", ent->fts_path);
					continue;
				}
			}
		}
		break;
	}
	return ent;
}

/*
 * process refs in path
 */

static void
refs(register State_t* state, const char* path, register Sfio_t* ip, File_t* ref)
{
	register int	c;
	register int	q;
	register int	r;
	register int	a;
	register char*	s;
	char*		p;
	char*		t;
	File_t*		f;
	String_t*	v;
	int		m;
	int		perlwarn;
	int		prefix;
	unsigned int	secure;
	unsigned int	flags;

	char		buf[8 * 1024];

	perlwarn = state->perlwarn && strmatch(path, "*.(html|htm|HTML|HTM)");
	prefix = (s = strrchr(path, '/')) ? s - (char*)path + 1 : 0;
	flags = EXTERNAL;
	for (;;)
	{
		switch (c = sfgetc(ip))
		{
		case EOF:
			break;
		case '<':
			q = 0;
			s = buf;
			for (;;)
			{
				switch (c = sfgetc(ip))
				{
				case EOF:
					return;
				case '>':
					sfungetc(ip, c);
					break;
				default:
					if (isspace(c))
						break;
					STUFF(s, buf, c);
					continue;
				}
				break;
			}
			q = 0;
			if (flags != INTERNAL && (s == (buf + 1) && (buf[0] == 'A' || buf[0] == 'a') || s == (buf + 4) && (buf[0] == 'L' || buf[0] == 'l') && (buf[1] == 'I' || buf[1] == 'i') && (buf[2] == 'N' || buf[2] == 'n') && (buf[3] == 'K' || buf[3] == 'k')))
			{
				s = buf;
				r = a = 0;
				f = 0;
				for (;;)
				{
					switch (c = sfgetc(ip))
					{
					case EOF:
						return;
					case '\'':
					case '"':
						if (q == c)
							q = 0;
						else if (q == 0)
							q = c;
						else if (r == HIT)
							STUFF(s, buf, c);
						continue;
					case '>':
					case ' ':
					case '\t':
					case '\n':
						if (!q)
						{
							if (r == HIT)
							{
								/*UNDENT...*/

	*s = 0;
	s = buf;
	if (!a)
		f = add(state, s, flags, path, prefix, ref);
	else if (f)
	{
		p = f->name;
		if (!strcasecmp(s, "data-root"))
			rootdir(state, &state->dataroot, p, buf, sizeof(buf));
		else if (!strcasecmp(s, "document-root"))
			rootdir(state, &state->documentroot, p, buf, sizeof(buf));
		else if (!strcasecmp(s, "host") || !strcasecmp(s, "hosts"))
		{
			if (!state->hosts.size && (state->hosts.size = strlen(p)) && !(state->hosts.data = strdup(p)))
				error(ERROR_SYSTEM|3, "out of space [hosts]");
		}
		else if (!strcasecmp(s, "program-root"))
			rootdir(state, &state->programroot, p, buf, sizeof(buf));
		else if ((secure = strcasecmp(s, "secure") ? 0 : SECURE) || !strcasecmp(s, "dynamic"))
		{
			FTS*	fts;
			FTSENT*	ent;

			if (t = strrchr(p, '/'))
				*t = 0;
			fts = fts_open((char**)p, FTS_ONEPATH|FTS_META|FTS_PHYSICAL|FTS_NOPOSTORDER, order);
			if (t)
				*t = '/';
			if (fts)
			{
				while (ent = scan(state, fts))
					add(state, ent->fts_path + prefix, flags|secure, f->name, prefix, f);
				if (fts_close(fts))
					error(ERROR_SYSTEM|2, "%s: directory read error", p);
			}
		}
		else if (!strcasecmp(s, "ignore") && (v = &state->ignore) || state->external && !strcasecmp(s, "internal") && (v = &state->internal))
		{
			if (state->copy.size)
			{
				s = state->copy.data;
				p += state->root.size;
			}
			else
				s = "";
			if (t = strrchr(p, '/'))
				*t = 0;
			m = v->size + strlen(s) + strlen(p) + 6;
			if (!(v->data = newof(v->data, char, m, 0)))
				error(ERROR_SYSTEM|3, "out of space [path pattern]");
			v->size += sfsprintf(v->data + v->size, m, "%s%s%s?(/*)", v->size ? "|" : "", s, p);
			if (t)
				*t = '/';
		}
	}

								/*...INDENT*/
							}
							if (c == '>')
								break;
							r = a = 0;
						}
						else if (r == HIT)
							STUFF(s, buf, c);
						continue;
					case '#':
					case '?':
						if (r == HIT)
							STUFF(s, buf, 0);
						continue;
					case 'H':
					case 'h':
						if (r == HIT)
							STUFF(s, buf, c);
						else if (!q)
							r = (r == 0) ? 1 : MISS;
						continue;
					case 'R':
					case 'r':
						if (r == HIT)
							STUFF(s, buf, c);
						else if (!q)
						{
							if (r == 0)
							{
								a = 10;
								r = a + 1;
							}
							r = (r == (a + 1)) ? (a + 2) : MISS;
						}
						continue;
					case 'E':
					case 'e':
						if (r == HIT)
							STUFF(s, buf, c);
						else if (!q)
							r = (r == (a + 2)) ? (a + 3) : MISS;
						continue;
					case 'F':
					case 'f':
						if (r == HIT)
							STUFF(s, buf, c);
						else if (!q)
							r = (r == 3) ? 4 : MISS;
						continue;
					case 'L':
					case 'l':
						if (r == HIT)
							STUFF(s, buf, c);
						else if (!q)
							r = (r == (a + 3)) ? (a + 4) : MISS;
						continue;
					case '=':
						if (r == HIT)
							STUFF(s, buf, c);
						else if (!q)
							r = (r == (a + 4)) ? HIT : MISS;
						continue;
					default:
						if (r == HIT)
							STUFF(s, buf, c);
						continue;
					}
					break;
				}
			}
			else if (flags != INTERNAL && (s == (buf + 5) && (buf[0] == 'F' || buf[0] == 'f') && (buf[1] == 'R' || buf[1] == 'r') && (buf[2] == 'A' || buf[2] == 'a') && (buf[3] == 'M' || buf[3] == 'm') && (buf[4] == 'E' || buf[4] == 'e') || s == (buf + 3) && (buf[0] == 'I' || buf[0] == 'i') && (buf[1] == 'M' || buf[1] == 'm') && (buf[2] == 'G' || buf[2] == 'g') || s == (buf + 6) && (buf[0] == 'S' || buf[0] == 's') && (buf[1] == 'C' || buf[1] == 'c') && (buf[2] == 'R' || buf[2] == 'r') && (buf[3] == 'I' || buf[3] == 'i') && (buf[4] == 'P' || buf[4] == 'p') && (buf[5] == 'T' || buf[5] == 't')))
			{
				s = buf;
				r = 0;
				for (;;)
				{
					switch (c = sfgetc(ip))
					{
					case EOF:
						return;
					case '\'':
					case '"':
						if (q == c)
							q = 0;
						else if (q == 0)
							q = c;
						else if (r == HIT)
							STUFF(s, buf, c);
						continue;
					case '>':
					case ' ':
					case '\t':
					case '\n':
						if (!q)
						{
							if (r == HIT)
							{
								*s = 0;
								s = buf;
								add(state, s, flags, path, prefix, ref);
							}
							if (c == '>')
								break;
							r = 0;
						}
						else if (r == HIT)
							STUFF(s, buf, c);
						continue;
					case 'S':
					case 's':
						if (r == HIT)
							STUFF(s, buf, c);
						else if (!q)
							r = (r == 0) ? 1 : MISS;
						continue;
					case 'R':
					case 'r':
						if (r == HIT)
							STUFF(s, buf, c);
						else if (!q)
							r = (r == 1) ? 2 : MISS;
						continue;
					case 'C':
					case 'c':
						if (r == HIT)
							STUFF(s, buf, c);
						else if (!q)
							r = (r == 2) ? 3 : MISS;
						continue;
					case '=':
						if (r == HIT)
							STUFF(s, buf, c);
						else if (!q)
							r = (r == 3) ? HIT : MISS;
						continue;
					default:
						if (r == HIT)
							STUFF(s, buf, c);
						continue;
					}
					break;
				}
			}
			else
			{
				if (state->external)
				{
					if (flags == EXTERNAL)
					{
						if (s == (buf + sizeof(internal) - 3) && strneq(buf, internal + 1, sizeof(internal) - 3))
						{
							flags = INTERNAL;
							ref->flags |= FILTER;
						}
					}
					else
					{
						if (s == (buf + sizeof(external) - 3) && strneq(buf, external + 1, sizeof(external) - 3))
							flags = EXTERNAL;
					}
				}
				for (;;)
				{
					switch (c = sfgetc(ip))
					{
					case EOF:
						return;
					case '\'':
					case '"':
						if (q == c)
							q = 0;
						else if (q == 0)
							q = c;
						continue;
					case '>':
						if (q == 0)
							break;
						continue;
					default:
						continue;
					}
					break;
				}
			}
			continue;
		case '[':
			if (perlwarn && (c = sfgetc(ip)) != EOF)
			{
				sfungetc(ip, c);
				switch (c)
				{
				case '-':
				case '+':
				case '!':
				case '$':
				case '*':
				case '#':
					error(1, "%s: file contains embedded perl constructs", path);
					perlwarn = 0;
					break;
				}
			}
			continue;
		default:
			if ((iscntrl(c) || !isprint(c)) && !isspace(c))
				break;
			continue;
		}
		break;
	}
}

/*
 * filter out internal text
 * return: <0:error 0:drop >0:keep
 */

static int
filter(register State_t* state, register Sfio_t* ip, Sfio_t* op)
{
	register char*	s;
	register size_t	n;
	register size_t	lines = 0;
	register int	head = 1;

	for (;;)
	{
		if (!(s = sfgetr(ip, '\n', head)))
			break;
		if ((n = sfvalue(ip)) != sizeof(internal) || !strneq(s, internal, sizeof(internal) - 1))
		{
			if (head)
				sfputr(op, s, '\n');
			else
				sfwrite(op, s, n);
			lines++;
			if (head && strgrpmatch(s, "</HEAD>", NiL, 0, STR_ICASE))
				head = 0;
		}
		else
		{
			while ((s = sfgetr(ip, '\n', 0)) && (sfvalue(ip) != sizeof(external) || !strneq(s, external, sizeof(external) - 1)));
			if (!s)
			{
				if (head)
					return 0;
				break;
			}
		}
	}
	if (sfvalue(ip) && (s = sfgetr(ip, -1, 0)) && (n = sfvalue(ip)))
		sfwrite(op, s, n);
	return lines > 1;
}

int
main(int argc, char** argv)
{
	register char*		s;
	register char*		p;
	register Sfio_t*	ip;
	register State_t*	state;
	register File_t*	fp;
	register List_t*	lp;
	FTS*			fts;
	FTSENT*			ent;
	struct passwd*		pwd;
	Sfio_t*			op;
	char*			dirs[4];
	int			i;
	int			n;
	struct stat		st;
	struct stat		ts;

	static const char*	www[] = { 0, 0, "wwwfiles", "public_html" };

	NoP(argc);
	error_info.id = "htmlrefs";
	if (!(state = newof(0, State_t, 1, 0)))
		error(ERROR_SYSTEM|3, "out of space [state]");
	state->disc.key = offsetof(File_t, name);
	state->disc.size = 0;
	if (!(state->files = dtopen(&state->disc, Dtoset)))
		error(ERROR_SYSTEM|3, "out of space [dict]");
	state->exec = 1;
	state->perlwarn = 1;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			state->all = opt_info.num;
			continue;
		case 'c':
			state->copy.size = strlen(state->copy.data = opt_info.arg);
			continue;
		case 'd':
			state->dependents = opt_info.num;
			continue;
		case 'e':
			state->external = opt_info.num;
			continue;
		case 'F':
			state->force = opt_info.num;
			continue;
		case 'h':
			state->hosts.size = strlen(state->hosts.data = opt_info.arg);
			continue;
		case 'i':
			state->index.size = strlen(state->index.data = opt_info.arg);
			continue;
		case 'k':
			state->keep.size = strlen(state->keep.data = opt_info.arg);
			continue;
		case 'K':
			state->skip.size = strlen(state->skip.data = opt_info.arg);
			continue;
		case 'l':
			state->limit.size = strlen(state->limit.data = opt_info.arg);
			continue;
		case 'm':
			state->missing = opt_info.num ? MISSING : 0;
			continue;
		case 'n':
			state->exec = opt_info.num;
			continue;
		case 'r':
			state->root.size = strlen(state->root.data = opt_info.arg);
			continue;
		case 's':
			state->strict = opt_info.num;
			continue;
		case 'u':
			state->user.size = strlen(state->user.data = opt_info.arg);
			continue;
		case 'v':
			state->verbose = opt_info.num;
			continue;
		case 'w':
			state->warn = opt_info.num;
			continue;
		case 'x':
			state->unreferenced = opt_info.num;
			continue;
		case 'S':
			state->symlink = opt_info.num;
		case 'X':
			state->remove = opt_info.num;
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
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (state->copy.size && (stat(state->copy.data, &st) || !S_ISDIR(st.st_mode)))
		error(ERROR_SYSTEM|3, "%s: not a directory", state->copy.data);
	if (!state->index.size)
		state->index.size = strlen(state->index.data = INDEX);
	if (!state->keep.size)
		state->keep.size = strlen(state->keep.data = KEEP);
	if (!state->skip.size)
		state->skip.size = strlen(state->skip.data = SKIP);
	if (!state->user.size)
		state->user.size = strlen(state->user.data = fmtuid(geteuid()));
	if (!state->root.size || *state->root.data != '/')
	{
		www[0] = (const char*)state->index.data;
		if (state->root.size)
			www[1] = (const char*)state->root.data;
		if (!(pwd = getpwnam(state->user.data)))
			error(3, "%s: unknown user", state->user.data);
		s = pwd->pw_dir;
		for (i = 0; i < elementsof(www); i++)
			if (www[i])
			{
				n = sfsprintf(state->buf, sizeof(state->buf) - 1, "%s/%s", s, www[i]);
				if (!access(state->buf, F_OK))
				{
					if (i == 0)
						n = strlen(s);
					else
						s = state->buf;
					if (!(state->root.data = strdup(s)))
						error(ERROR_SYSTEM|3, "out of space [root]");
					state->root.size = n;
					break;
				}
			}
	}
	while (s = *argv++)
		add(state, s, EXTERNAL|VERBOSE, NiL, 0, NiL);
	if (!state->more)
	{
		sfsprintf(state->buf, sizeof(state->buf) - 1, "%s/%s", state->root.data, state->index.data);
		add(state, state->buf, EXTERNAL|VERBOSE, NiL, 0, NiL);
	}
	while (state->more)
	{
		state->more = 0;
		for (fp = (File_t*)dtfirst(state->files); fp; fp = (File_t*)dtnext(state->files, fp))
		{
			if (!(fp->flags & SCANNED))
			{
				fp->flags |= SCANNED;
				if (streq(fp->name, "-") || streq(fp->name, "/dev/stdin") || streq(fp->name, "/dev/fd/0"))
					ip = sfstdin;
				else if (!(ip = sfopen(NiL, fp->name, "r")))
				{
					fp->flags |= MISSING;
					if (state->warn || (fp->flags & VERBOSE))
						error(ERROR_SYSTEM|2, "%s: cannot read", fp->name);
					continue;
				}
				refs(state, fp->name, ip, fp);
				if (ip != sfstdin)
					sfclose(ip);
			}
		}
	}
	if (state->copy.size)
	{
		p = state->buf;
		for (fp = (File_t*)dtfirst(state->files); fp; fp = (File_t*)dtnext(state->files, fp))
			if (!(fp->flags & (CHECKED|COPIED|MISSING)))
			{
				fp->flags |= CHECKED;
				sfsprintf(p, sizeof(state->buf) - 1, "%s%s", state->copy.data, fp->name + state->root.size);
				if (state->internal.size && strmatch(p, state->internal.data))
					continue;
				add(state, p, COPIED, NiL, 0, NiL);
				if (stat(fp->name, &st))
					error(ERROR_SYSTEM|3, "%s: cannot stat", fp->name);
				if (state->limit.size && !strmatch(p, state->limit.data))
					continue;
				if (stat(p, &ts))
				{
					ts.st_mtime = 0;
					ts.st_mode = 0;
				}
				if (strmatch(p, "*/cgi-bin/*|*.cgi|*.html"))
					fp->flags |= COPY;
				if (!state->exec)
				{
					if (fp->flags & DIRECTORY)
					{
						if (!ts.st_mtime)
							sfprintf(sfstdout, " mkdir %s\n", p);
					}
					else if (state->force || st.st_mtime != ts.st_mtime)
					{
						if (fp->flags & FILTER)
							sfprintf(sfstdout, "filter %s\n", p);
						else if (state->symlink && !(fp->flags & COPY))
							sfprintf(sfstdout, "  link %s\n", p);
						else
							sfprintf(sfstdout, "  copy %s\n", p);
					}
				}
				else if (fp->flags & DIRECTORY)
				{
					if (!ts.st_mtime)
					{
						if (state->verbose)
							sfprintf(sfstdout, " mkdir %s\n", p);
						if (mkdir(p, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH))
							error(ERROR_SYSTEM|2, "%s: cannot create directory", p);
					}
				}
				else if (state->symlink && !(fp->flags & (COPY|FILTER)))
				{
					if (st.st_mtime != ts.st_mtime)
					{
						if (state->verbose)
							sfprintf(sfstdout, " ln -s %s %s\n", fp->name, p);
						if (ts.st_mtime)
							remove(p);
						if (symlink(fp->name, p))
							error(ERROR_SYSTEM|2, "%s: cannot symlink to %s", fp->name, p);
					}
				}
				else if (state->force || st.st_mtime != ts.st_mtime)
				{
					if (!(ip = sfopen(NiL, fp->name, "r")))
						error(ERROR_SYSTEM|2, "%s: cannot read", fp->name);
					else if (!(op = sfopen(NiL, p, "w")))
					{
						error(ERROR_SYSTEM|2, "%s: cannot write", p);
						sfclose(ip);
					}
					else
					{
						if (fp->flags & FILTER)
						{
							if (state->verbose)
								sfprintf(sfstdout, "filter %s\n", p);
							n = filter(state, ip, op);
						}
						else
						{
							if (state->verbose)
								sfprintf(sfstdout, "  copy %s\n", p);
							if (sfmove(ip, op, SF_UNBOUND, -1) >= 0 && sfeof(ip))
								n = 1;
							else
								n = -1;
						}
						if (n < 0)
							error(ERROR_SYSTEM|2, "%s: read error", fp->name);
						if (sfclose(op))
							error(ERROR_SYSTEM|2, "%s: write error", p);
						sfclose(ip);
						if (n > 0)
						{
							if ((st.st_mode &= S_IPERM) != (ts.st_mode &= S_IPERM) && chmod(p, st.st_mode))
								error(ERROR_SYSTEM|2, "%s: cannot set mode", p);
							if (touch(p, st.st_mtime, st.st_mtime, 0))
								error(ERROR_SYSTEM|2, "%s: cannot set times", p);
						}
						else if (!n)
						{
							if (state->verbose)
								sfprintf(sfstdout, " %s %s\n", (fp->flags & DIRECTORY) ? "rmdir" : "   rm", p);
							if (((fp->flags & DIRECTORY) ? rmdir : remove)(p))
								error(ERROR_SYSTEM|2, "%s: cannot remove", p);
						}
					}
				}
			}
		if (state->unreferenced)
		{
			if (!(fts = fts_open((char**)state->copy.data, FTS_ONEPATH|FTS_META|FTS_PHYSICAL|FTS_NOPREORDER, order)))
				error(ERROR_SYSTEM|3, "%s: cannot search directory", state->copy.data);
			while (ent = scan(state, fts))
				if ((!(fp = dtmatch(state->files, ent->fts_path)) || !(fp->flags & COPIED)) && (!state->ignore.size || !strmatch(ent->fts_path, state->ignore.data)) && (!state->limit.size || strmatch(ent->fts_path, state->limit.data)))
				{
					if (state->verbose || !state->exec)
						sfprintf(sfstdout, " %s %s\n", (ent->fts_info & FTS_D) ? "rmdir" : "   rm", ent->fts_path);
					if (state->exec && ((ent->fts_info & FTS_D) ? rmdir : remove)(ent->fts_path))
						error(ERROR_SYSTEM|2, "%s: cannot remove", ent->fts_path);
				}
			if (fts_close(fts))
				error(ERROR_SYSTEM|3, "%s: directory read error", state->copy.data);
		}
	}
	else if (state->unreferenced)
	{
		i = 0;
		if (state->documentroot.data)
			dirs[i++] = state->documentroot.data;
		else
		{
			if (!state->root.data)
				state->root.size = strlen(state->root.data = ".");
			dirs[i++] = state->root.data;
		}
		if (state->dataroot.data)
			dirs[i++] = state->dataroot.data;
		if (state->programroot.data)
			dirs[i++] = state->programroot.data;
		dirs[i] = 0;
		if (!(fts = fts_open(dirs, FTS_META|FTS_PHYSICAL|FTS_NOPREORDER, order)))
			error(ERROR_SYSTEM|3, "%s: cannot search directory", state->root.data);
		while (ent = scan(state, fts))
			if (!dtmatch(state->files, ent->fts_path) && (!strmatch(ent->fts_name, state->keep.data) || state->skip.size && strmatch(ent->fts_name, state->skip.data) || state->ignore.size && strmatch(ent->fts_path, state->ignore.data)))
			{
				if (state->strict || !streq(ent->fts_name, state->index.data))
				{
					if (!state->remove)
						sfprintf(sfstdout, "%s\n", fmtquote(ent->fts_path, "\"", "\"", ent->fts_pathlen, 0));
					else if (!state->limit.size || strmatch(ent->fts_path, state->limit.data))
					{
						if (state->verbose || !state->exec)
							sfprintf(sfstdout, " %s %s\n", (ent->fts_info & FTS_D) ? "rmdir" : "   rm", ent->fts_path);
						if (state->exec && ((ent->fts_info & FTS_D) ? rmdir : remove)(ent->fts_path))
							error(ERROR_SYSTEM|2, "%s: cannot remove", ent->fts_path);
					}
				}
				else if (s = strrchr(ent->fts_path, '/'))
				{
					*s = 0;
					add(state, ent->fts_path, COPIED, NiL, 0, NiL);
					*s = '/';
				}
			}
		if (fts_close(fts))
			error(ERROR_SYSTEM|3, "%s: directory read error", state->root.data);
	}
	else
	{
		for (fp = (File_t*)dtfirst(state->files); fp; fp = (File_t*)dtnext(state->files, fp))
			if (state->all || (fp->flags & MISSING) == state->missing)
			{
				sfprintf(sfstdout, "%s", fmtquote(fp->name, "\"", "\"", strlen(fp->name), 0));
				if (state->dependents && fp->refs)
				{
					sfputc(sfstdout, ' ');
					sfputc(sfstdout, ':');
					for (lp = fp->refs; lp; lp = lp->next)
						sfprintf(sfstdout, " %s", fmtquote(lp->file->name, "\"", "\"", strlen(lp->file->name), 0));
				}
				sfputc(sfstdout, '\n');
			}
	}
	return error_info.errors != 0;
}
