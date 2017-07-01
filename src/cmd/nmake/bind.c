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
 * make rule binding routines
 */

#include "make.h"
#include "options.h"

/*
 * embedded spaces in file name wreak havoc
 * we wreak hack to get through
 */

#define HACKSPACE(f,s)	for (s = f; s = strchr(s, ' '); *s++ = (state.test & 0x00080000) ? '?' : FILE_SPACE)
#define FIGNORE(s)	((s)[0]=='.'&&((s)[1]==0||(s)[1]=='.'&&(s)[2]==0))

#if DEBUG
#define DEBUGSOURCE(n,r,p) \
	do \
	{ \
		if (error_info.trace <= -14) \
		{ \
			List_t*		q; \
			message((-14, "  [%d] %s", n, (r)->name)); \
			for (q = p; q; q = q->next) \
				message((-14, "      %s", q->rule->name)); \
		} \
	} while (0)
#else
#define DEBUGSOURCE(n,r,p)
#endif

#if _WINIX

/*
 * we have a system in which some directories preserve
 * mixed case entries but ignore case on name lookup
 * maybe they can get a patent on that
 * hey, maybe we can get a patent on this
 */

static int
file_compare(register const char* s, register const char* t)
{
	return ((File_t*)hashgetbucket(s)->value)->dir->ignorecase ? strcasecmp(s, t) : strcmp(s, t);
}

static unsigned int
file_hash(const char* s)
{
	register const unsigned char*	p = (const unsigned char*)s;
	register unsigned int		h = 0;
	register unsigned int		c;

	while (c = *p++)
	{
		if (isupper(c))
			c = tolower(c);
		HASHPART(h, c);
	}
	return h;
}

static int
rule_compare(register const char* s, register const char* t)
{
	register int	c;
	register int	d;
	register int	x;

	x = (*s == '.') ? 0 : -1;
	while (c = *s++)
	{
		if (!x)
		{
			if (c == '%')
				x = (*(s - 2) == '.') ? 1 : -1;
			else if (c != '.' && !isupper(c))
				x = -1;
		}
		if ((d = *t++) != c)
		{
			if (x > 0)
			{
				if (isupper(c))
					c = tolower(c);
				else if (isupper(d))
					d = tolower(d);
				if (c == d)
					continue;
			}
			return c - d;
		}
	}
	return c - *t;
}

static unsigned int
rule_hash(const char* s)
{
	register const unsigned char*	p = (const unsigned char*)s;
	register unsigned int		h = 0;
	register unsigned int		c;
	register int			x;

	x = (*s == '.') ? 0 : -1;
	while (c = *p++)
	{
		if (!x)
		{
			if (c == '%')
				x = (*(p - 2) == '.') ? 1 : -1;
			else if (c != '.' && !isupper(c))
				x = -1;
		}
		if (x > 0 && isupper(c))
			c = tolower(c);
		HASHPART(h, c);
	}
	return h;
}

#endif

/*
 * initialize the hash tables
 */

void
inithash(void)
{
	table.ar	= hashalloc(NiL, HASH_name, "archives", 0);
	table.bound	= hashalloc(table.ar, HASH_name, "bound-directories", 0);
#if _WINIX
	table.file	= hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_compare, file_compare, HASH_hash, file_hash, HASH_name, "files", 0);
	table.oldvalue	= hashalloc(NiL, HASH_name, "old-values", 0);
	table.rule	= hashalloc(NiL, HASH_compare, rule_compare, HASH_hash, rule_hash, HASH_name, "atoms", 0);
#else
	table.file	= hashalloc(table.ar, HASH_set, HASH_ALLOCATE, HASH_name, "files", 0);
	table.oldvalue	= hashalloc(table.file, HASH_name, "old-values", 0);
	table.rule	= hashalloc(table.oldvalue, HASH_name, "atoms", 0);
#endif
	table.var	= hashalloc(table.oldvalue, HASH_name, "variables", 0);
	table.dir	= hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(Fileid_t), HASH_name, "directories", 0);
	optinit();
}

/*
 * determine if a directory (archive) has already been scanned
 */

Dir_t*
unique(register Rule_t* r)
{
	register Dir_t*	d;
	Rule_t*		x;
	Fileid_t	id;
	Stat_t		st;

	if (rstat(r->name, &st, 0))
	{
		r->time = 0;
		return 0;
	}
	id.dev = st.st_dev;
	id.ino = st.st_ino;
	if ((d = getdir(&id)) && state.alias && d->directory == (S_ISDIR(st.st_mode) != 0) && (!state.mam.statix || S_ISDIR(st.st_mode)))
	{
		/*
		 * internal.unbind causes directory rescan
		 */

		if (r->name == d->name || (r->dynamic & D_alias) && makerule(r->name)->name == d->name || (x = makerule(d->name)) == r)
			return d;

		/*
		 * the physical directory alias can go either way
		 * but we bias the choice towards shorter pathnames
		 */

		if (!x->uname && strlen(r->name) < strlen(x->name))
		{
			Rule_t*		t;

			t = r;
			r = x;
			x = t;
			x->time = d->time;
		}
		message((-2, "%s %s is also specified as %s", (r->property & P_archive) ? "archive" : "directory", unbound(r), x->name));
		r->dynamic |= D_alias;
		merge(r, x, MERGE_ALL|MERGE_BOUND);
		r->view = x->view;
		r->time = x->time;
		r->uname = r->name;
		d->name = r->name = x->name;
		return 0;
	}
	d = newof(0, Dir_t, 1, 0);
	d->name = r->name;
	d->time = r->time = tmxgetmtime(&st);
	d->directory = S_ISDIR(st.st_mode) != 0;
	putdir(0, d);
#if _WINIX
	d->ignorecase = d->directory;
#endif
	return d;
}

/*
 * add a directory (archive) entry to the file hash
 */

File_t*
addfile(Dir_t* d, char* name, Time_t date)
{
	register File_t*	f;
	register File_t*	n;
	register char*		s;

	HACKSPACE(name, s);

	/*
	 * this test avoids duplicate entries for systems that
	 * support viewed or covered directories in the pathname
	 * system calls
	 *
	 * we assume that the cover directories are read in order
	 * from top to bottom
	 *
	 * the entries for a directory and its covered directories
	 * all have the same file.dir value
	 */

	if ((n = getfile(name)) && n->dir == d)
	{
		if (d->archive)
		{
			if (n->time < date)
				n->time = date;
#if DEBUG
			message((-12, "%s: %s %s [duplicate member]", d->name, name, timestr(n->time)));
#endif
		}
		return n;
	}
#if DEBUG
	message((-12, "%s: %s %s%s", d->name, name, timestr(date), d->ignorecase ? " [ignorecase]" : null));
#endif
	f = newof(0, File_t, 1, 0);
	f->next = n;
	f->dir = d;
	f->time = date;
	putfile(0, f);
#if _WINIX
	if (!d->archive && (s = strchr(name, '.')))
	{
		s++;
		if (streq(s, "exe") || streq(s, "EXE"))
		{
			*--s = 0;
			addfile(d, name, date);
			*s = '.';
		}
	}
#endif
	return f;
}

/*
 * add new file r to the directory hash at dir
 */

void
newfile(register Rule_t* r, char* dir, Time_t date)
{
	register char*		s;
	register char*		t;
	char*			nam;
	Rule_t*			z;
	Dir_t*			d;
	Hash_position_t*	pos;
	Sfio_t*			tmp;

	tmp = sfstropen();
	sfputc(tmp, '.');
	sfputc(tmp, '/');
	edit(tmp, r->name, dir ? dir : KEEP, KEEP, KEEP);
	s = (nam = sfstruse(tmp)) + 1;
	do
	{
		*s = 0;
		if ((z = getrule(nam)) && (z->dynamic & D_entries))
		{
			if (t = strchr(s + 1, '/'))
				*t = 0;

			/*
			 * sequential scan is OK since this is uncommon
			 */

			if (pos = hashscan(table.dir, 0))
			{
				while (hashnext(pos))
				{
					d = (Dir_t*)pos->bucket->value;
					if (d->name == z->name)
					{
						addfile(d, s + 1, date);
						break;
					}
				}
				hashdone(pos);
			}
			if (t)
				*t = '/';
		}
		*s++ = '/';
	} while (s = strchr(s, '/'));
	sfstrclose(tmp);
}

/*
 * scan directory r and record all its entries
 */

void
dirscan(Rule_t* r)
{
	register DIR*		dirp;
	register Dir_t*		d;
	register Dirent_t*	entry;
	char*			s;
	int			n;
	Stat_t			st;

	if (r->dynamic & D_scanned)
		return;
	if ((n = strlen(r->name)) > 1)
		s = canon(r->name) - 1;
	else
		s = r->name;
	if (s > r->name && *s == '/')
		*s-- = 0;
	if ((s - r->name + 1) != n)
		r->name = maprule(r->name, r);
	r->dynamic |= D_scanned;
	r->dynamic &= ~D_entries;
	if (!(r->property & P_state))
	{
		if (d = unique(r))
		{
			s = r->name;
			if (d->directory && (dirp = opendir(s)))
			{
				debug((-5, "scan directory %s", s));
				while (entry = readdir(dirp))
					if (!FIGNORE(entry->d_name))
						addfile(d, entry->d_name, NOTIME);
				r->dynamic |= D_entries;
				if (!(r->dynamic & D_bound) && !stat(s, &st))
				{
					r->dynamic |= D_bound;
					r->time = tmxgetmtime(&st);
					if (!r->view && ((state.questionable & 0x00000800) || !(r->property & P_target)) && *s == '/' && (strncmp(s, internal.pwd, internal.pwdlen) || *(s + internal.pwdlen) != '/'))
						r->dynamic |= D_global;
				}
				closedir(dirp);
				return;
			}
			debug((-5, "dirscan(%s) failed", s));
		}
		else if (r->time)
			r->dynamic |= D_entries;
	}
}

typedef struct Globstate_s
{
	char*		name;
	DIR*		dirp;
	int		view;
	int		root;
	Hash_table_t*	overlay;
} Globstate_t;

#if 0

static void*
trace_opendir(const char* dir, int line)
{
	void*	p;

	p = opendir(dir);
	error(-1, "AHA#%d opendir %s = %p", line, dir, p);
	return p;
}

#define opendir(f)	trace_opendir(f,__LINE__)

#endif

/*
 * glob() diropen for 2d views
 */

static void*
glob_diropen(glob_t* gp, const char* path)
{
	Globstate_t*	gs = (Globstate_t*)gp->gl_handle;
	const char*	dir;
	register int	i;
	register int	n;

	if (!(gs->overlay = hashalloc(NiL, HASH_set, HASH_ALLOCATE, 0)))
		return 0;
	gs->view = 0;
	gs->root = 0;
	dir = path;
	if (*path == '/')
		for (i = 0; i <= state.maxview; i++)
		{
			if (!strncmp(path, state.view[i].root, n = state.view[i].rootlen) && (!*(path + n) || *(path + n) == '/'))
			{
				if (!*(path += n + 1))
					path = internal.dot->name;
				gs->view = i;
				gs->root = 1;
				break;
			}
		}
	gs->name = makerule((char*)path)->name;
	if (gs->dirp = opendir(dir))
		return (void*)gs;
	if (*path != '/')
		while (gs->view++ < state.maxview)
		{
			if (*gs->name == '/')
				sfprintf(internal.nam, "%s", gs->name);
			else if (gs->root)
				sfprintf(internal.nam, "%s/%s", state.view[gs->view].root, gs->name);
			else
			{
				if (*state.view[gs->view].path != '/')
					sfprintf(internal.nam, "%s/", internal.pwd);
				sfprintf(internal.nam, "%s/%s", state.view[gs->view].path, gs->name);
			}
			if (gs->dirp = opendir(sfstruse(internal.nam)))
				return (void*)gs;
		}
	hashfree(gs->overlay);
	return 0;
}

/*
 * glob() dirnext for 2d views
 */

static char*
glob_dirnext(glob_t* gp, void* handle)
{
	Globstate_t*	gs = (Globstate_t*)handle;
	Dirent_t*	dp;
	char*		s;

	for (;;)
	{
		if (dp = readdir(gs->dirp))
		{
			if (FIGNORE(dp->d_name))
				continue;
			HACKSPACE(dp->d_name, s);
			if (hashget(gs->overlay, dp->d_name))
				continue;
			hashput(gs->overlay, 0, (char*)gs);
#if defined(DT_UNKNOWN) && defined(DT_DIR) && defined(DT_LNK)
			if (dp->d_type != DT_UNKNOWN && dp->d_type != DT_DIR && dp->d_type != DT_LNK)
				gp->gl_status |= GLOB_NOTDIR;
#endif
			return dp->d_name;
		}
		closedir(gs->dirp);
		gs->dirp = 0;
		if (*gs->name == '/')
			return 0;
		do
		{
			if (gs->view++ >= state.maxview)
				return 0;
			if (gs->root)
				sfprintf(internal.nam, "%s/%s", state.view[gs->view].root, gs->name);
			else
			{
				if (*state.view[gs->view].path != '/')
					sfprintf(internal.nam, "%s/", internal.pwd);
				sfprintf(internal.nam, "%s/%s", state.view[gs->view].path, gs->name);
			}
		} while (!(gs->dirp = opendir(sfstruse(internal.nam))));
	}
}

/*
 * glob() dirclose for 2d views
 */

static void
glob_dirclose(glob_t* gp, void* handle)
{
	Globstate_t*	gs = (Globstate_t*)handle;

	if (gs->dirp)
		closedir(gs->dirp);
	if (gs->overlay)
		hashfree(gs->overlay);
}

/*
 * glob() type for 2d views
 */

static int
glob_type(glob_t* gp, const char* path, int flags)
{
	register int		i;
	register int		n;
	int			root;
	Stat_t			st;

	i = 0;
	if ((*gp->gl_stat)(path, &st))
	{
		root = 0;
		if (*path == '/')
			for (i = 0; i <= state.maxview; i++)
			{
				if (!strncmp(path, state.view[i].root, n = state.view[i].rootlen) && (!*(path + n) || *(path + n) == '/'))
				{
					if (!*(path += n + 1))
						path = internal.dot->name;
					root = 1;
					break;
				}
			}
		for (i = 0; i <= state.maxview; i++)
		{
			if (root)
				sfprintf(internal.nam, "%s/%s", state.view[i].root, path);
			else
			{
				if (*state.view[i].path != '/')
					sfprintf(internal.nam, "%s/", internal.pwd);
				sfprintf(internal.nam, "%s/%s", state.view[i].path, path);
			}
			if (!stat(sfstruse(internal.nam), &st))
				break;
		}
	}
	if (i > state.maxview)
		i = 0;
	else if (S_ISDIR(st.st_mode))
		i = GLOB_DIR;
	else if (!S_ISREG(st.st_mode))
		i = GLOB_DEV;
	else if (st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))
		i = GLOB_EXE;
	else
		i = GLOB_REG;
	return i;
}

/*
 * return a vector of the top view of files in all views matching pattern s
 * the vector is placed on the current stack
 */

char**
globv(register glob_t* gp, char* s)
{
	register char**		q;
	register char**		p;
	register char**		x;
	int			i;
	int			f;
	glob_t			gl;
	Globstate_t		gs;

	static char*		nope[1];

	f = GLOB_AUGMENTED|GLOB_DISC|GLOB_STARSTAR;
	if (!gp)
	{
		gp = &gl;
		f |= GLOB_STACK;
	}
	memset(gp, 0, sizeof(gl));
	gp->gl_intr = &state.caught;
	gp->gl_stat = pathstat;
	if (state.maxview && !state.fsview)
	{
		gp->gl_handle = (void*)&gs;
		gp->gl_diropen = glob_diropen;
		gp->gl_dirnext = glob_dirnext;
		gp->gl_dirclose = glob_dirclose;
		gp->gl_type = glob_type;
	}
	if (i = glob(s, f, 0, gp))
	{
		if (i != GLOB_NOMATCH && !trap())
			error(2, "glob() internal error %d", i);
		return nope;
	}
	if (state.maxview && !state.fsview)
	{
		for (i = 0, p = 0, x = q = gp->gl_pathv; *q; q++)
			if (!p || !streq(*q, *p))
			{
				*x++ = *q;
				p = q;
			}
		*x = 0;
	}
	return gp->gl_pathv;
}

/*
 * enter r as an alias for x
 * path is the canonical path name for x
 * d is the bind directory for path
 * a pointer to r merged with x is returned
 */

static Rule_t*
bindalias(register Rule_t* r, register Rule_t* x, char* path, Rule_t* d, int force)
{
	char*		s;
	int		i;
	int		n;
	int		na = 0;
	Rule_t*		z;
	Rule_t*		a[3];

	if (x->dynamic & D_alias)
	{
		a[na++] = x;
		x = makerule(x->name);
		if (x == r || (x->dynamic & D_alias))
			return r;
	}
	if (!force && !((r->dynamic|x->dynamic)&D_bound) && !d && !strchr(x->name, '/'))
	{
		debug((-5, "%s alias %s delayed until one or the other is bound", x->name, r->name));
		return x;
	}
	message((-2, "%s is also specified as %s", unbound(r), unbound(x)));
#if DEBUG
	if (state.test & 0x00000040)
		error(2, "bindalias: path=%s r=%s%s%s x=%s%s%s", path, r->name, r->uname ? "==" : null, r->uname ? r->uname : null, x->name, x->uname ? "==" : null, x->uname ? x->uname : null);
#endif
	r->dynamic |= (D_alias|D_bound);
	merge(r, x, MERGE_ALL|MERGE_BOUND);
	if (!(state.questionable & 0x00001000))
		x->attribute |= r->attribute;
	if (x->uname && !streq(x->name, path) && !streq(x->uname, path))
	{
		putrule(x->name, 0);
		z = makerule(path);
		if (z->dynamic & D_alias)
		{
			a[na++] = z;
			z = makerule(z->name);
#if DEBUG
			if (z->dynamic & D_alias)
				error(PANIC, "multiple alias from %s to %s", z->name, x->name);
#endif
		}
		if (z != x && z != r)
		{
#if DEBUG
			if (state.test & 0x00000040)
				error(2, "           z=%s%s%s", z->name, z->uname ? "==" : null, z->uname ? z->uname : null);
#endif
			x->dynamic |= (D_alias|D_bound);
			merge(x, z, MERGE_ALL|MERGE_BOUND);
			a[na++] = x;
			x = z;
		}
	}
	if (x->dynamic & D_bound)
	{
		r->time = x->time;
		r->view = x->view;
	}
	else
	{
		x->dynamic |= D_bound;
		x->dynamic &= ~D_member;
		x->time = r->time;
		if (!(x->dynamic & D_source) || (x->property & P_target))
			x->view = r->view;
	}
	s = r->uname = r->name;
	r->name = x->name;
	if (d)
	{
		if (state.fsview && strchr(s, '/') && strchr(r->name, '/') && !streq(s, r->name))
		{
			debug((-5, "%s and %s are bound in %s", s, r->name, d->name));
			putbound(s, d->name);
			putbound(r->name, d->name);
		}
		else if (s[0] == '.' && s[1] == '.' && s[2] == '/' && ((n = strlen(r->name)) < (i = strlen(s)) || r->name[n - i - 1] != '/' || !streq(s, r->name + n - i)))
			putbound(s, d->name);
	}
	if (!(state.questionable & 0x00002000))
	{
		if ((!x->uname || x->uname[0] != '.' && x->uname[1] != '.' && x->uname[2] != '/') && ((s = getbound(x->name)) || x->uname && (s = getbound(x->uname))))
		{
			debug((-5, "%s rebind %s => %s", r->name, getbound(r->name), s));
			putbound(r->name, s);
		}
		else if ((!r->uname || r->uname[0] != '.' && r->uname[1] != '.' && r->uname[2] != '/') && ((s = getbound(r->name)) || r->uname && (s = getbound(r->uname))))
		{
			debug((-5, "%s rebind %s => %s", x->name, getbound(x->name), s));
			putbound(x->name, s);
		}
		else if (r->uname && (n = strlen(r->name)) > (i = strlen(r->uname)) && r->name[n-=i+1] == '/')
		{
			r->name[n] = 0;
			s = (z = getrule(r->name)) ? z->name : strdup(r->name);
			r->name[n] = '/';
			debug((-5, "%s and %s are bound in %s", r->name, r->uname, s));
			putbound(r->name, s);
			putbound(r->uname, s);
		}
		else
		{
			debug((-5, "no rebind for %s or %s", unbound(r), unbound(x)));
			s = 0;
		}
		for (i = 0; i < na; i++)
		{
			if (s)
				putbound(a[i]->name, s);
			x->attribute |= a[i]->attribute;
		}
		for (i = 0; i < na; i++)
			a[i]->attribute |= x->attribute;
		r->attribute |= x->attribute;
	}
	return x;
}

/*
 * return local view rule for r if defined
 * force forces the rule to be allocated
 * 0 always returned if !state && == r or if not in local view
 */

static Rule_t*
localrule(register Rule_t* r, int force)
{
	register char*		s;
	register Rule_t*	x;
	char*			p;
	char*			v;
	Sfio_t*			tmp;

	if (r->property & P_state)
		return force ? 0 : r;
	if (r->dynamic & D_alias)
		r = makerule(r->name);
	if (!r->view)
		return 0;
	if (!strncmp(r->name, state.view[r->view].path, state.view[r->view].pathlen))
	{
		s = r->name + state.view[r->view].pathlen;
		switch (*s++)
		{
		case 0:
			return internal.dot;
		case '/':
			if (!(x = getrule(s)) && force)
				x = makerule(s);
			if (x && (x->dynamic & D_alias))
				x = makerule(x->name);
			if (x && !x->view && (x != r || force))
			{
				merge(r, x, MERGE_ATTR|MERGE_FORCE);
				x->uname = r->uname;
				x->time = r->time;
				x->status = r->status;
				return x;
			}
			return 0;
		}
	}
	p = 0;
	s = r->name;
	v = state.view[r->view].path;
	while (*s && *s == *v++)
		if (*s++ == '/')
			p = s;
	if (p)
	{
		s = internal.pwd;
		v--;
		while (s = strchr(s, '/'))
			if (!strcmp(++s, v))
			{
				tmp = sfstropen();
				*--s = 0;
				sfprintf(tmp, "%s/%s", internal.pwd, p);
				v = sfstruse(tmp);
				*s = '/';
				if (!(x = getrule(v)) && force)
					x = makerule(v);
				if (x && (x->dynamic & D_alias))
					x = makerule(x->name);
				sfstrclose(tmp);
				if (x && !x->view && (force || x != r))
				{
					merge(r, x, MERGE_ATTR);
					if (!x->uname)
						x->uname = r->uname;
					if (!x->time)
						x->time = r->time;
					return x;
				}
				return 0;
			}
	}
	return 0;
}

/*
 * bind a rule to a file
 */

Rule_t*
bindfile(register Rule_t* r, char* name, int flags)
{
	register Rule_t*	d;
	register File_t*	f;
	register char*		s;
	List_t*			p;
	File_t*			files;
	File_t*			ofiles;
	char*			dir = 0;
	char*			base;
	char*			b;
	int			found;
	int			i;
	int			n;
	int			c;
	int			ndirs;
	int			allocated = 0;
	int			aliased = 0;
	int			reassoc = 0;
	unsigned int		view;
	Time_t			tm;
	Stat_t			st;
	Rule_t*			a;
	Rule_t*			od;
	Rule_t*			x;
	Rule_t*			z;
	List_t*			dirs[5];
	List_t			dot;
	Sfio_t*			buf;
	Sfio_t*			tmp;

	static Dir_t		tmp_dir;
	static File_t		tmp_file = { 0, &tmp_dir, NOTIME };

	if (r || (r = getrule(name)))
	{
		if ((r->property & P_state) || (r->property & P_virtual) && !(flags & BIND_FORCE))
			return 0;
		if (r->dynamic & D_alias)
		{
			a = r;
			r = makerule(unbound(r));
			if (r == a)
			{
				error(1, "%s: alias loop", r->name);
				r->dynamic &= ~D_alias;
			}
		}
		if (r->dynamic & D_bound)
			return r;
		if (!name)
			name = r->name;
	}
	buf = sfstropen();
	tmp = sfstropen();
	for (;;)
	{
		found = 0;
		view = 0;
		od = 0;

		/*
		 * at this point name!=r->name is possible
		 */

#if _WINIX
		if (*name == '/' || isalpha(*name) && *(name + 1) == ':' && (*(name + 2) == '/' || *(name + 2) == '\\'))
#else
		if (*name == '/')
#endif
		{
			sfputr(buf, name, 0);
			s = sfstrseek(buf, 0, SEEK_SET);
			canon(s);
			if (!rstat(s, &st, 0))
			{
				tm = tmxgetmtime(&st);
				f = 0;
				found = 1;
			}
			else if (state.maxview && !state.fsview)
				for (i = 0; i <= state.maxview; i++)
				{
					if (!strncmp(s, state.view[i].root, n = state.view[i].rootlen) && (!*(s + n) || *(s + n) == '/'))
					{
						if (!*(s += n + 1))
							s = internal.dot->name;
						for (i = 0; i <= state.maxview; i++)
						{
							sfprintf(internal.nam, "%s/%s", state.view[i].root, s);
							if (!rstat(b = sfstruse(internal.nam), &st, 0))
							{
								sfputr(buf, b, 0);
								tm = tmxgetmtime(&st);
								f = 0;
								found = 1;
#if 0 /* not sure about this */
								view = i;
#endif
								break;
							}
						}
						break;
					}
				}
			break;
		}

		/*
		 * dir contains the directory path name component (usually 0)
		 * base contains the base path name component
		 */

		edit(tmp, name, KEEP, DELETE, DELETE);
		dir = sfstruse(tmp);
		if (*dir)
			base = name + strlen(dir) + 1;
		else
		{
			dir = 0;
			base = name;
		}

		/*
		 * gather the directories to search in reverse order
		 */

#if DEBUG
		message((-11, "bindfile(%s): dir=%s base=%s", name, dir ? dir : internal.dot->name, base));
#endif
		ndirs = 0;
		if (!(flags & BIND_DOT))
		{
			if (!r)
				a = associate(internal.source_p, NiL, name, NiL);
			else if (name == r->name)
				a = associate(internal.source_p, r, NiL, NiL);
			else
				a = 0;
			if (!a || !(a->property & P_force))
			{
				if ((x = internal.source) != a)
				{
					if (!(x->dynamic & D_cached))
						x = source(x);
					if ((p = x->prereqs) && (p->rule != internal.dot || (p = p->next)))
					{
						DEBUGSOURCE(ndirs, x, p);
						dirs[ndirs++] = p;
					}
				}
				if ((flags & BIND_MAKEFILE) && (x = catrule(internal.source->name, external.source, NiL, 0)) && x != a)
				{
					if (!(x->dynamic & D_cached))
						x = source(x);
					if ((p = x->prereqs) && (p->rule != internal.dot || (p = p->next)))
					{
						DEBUGSOURCE(ndirs, x, p);
						dirs[ndirs++] = p;
					}
				}
				edit(buf, name, DELETE, internal.source->name, KEEP);
				if ((z = getrule(sfstruse(buf))) && z != x && z != internal.source && z != internal.source_p)
				{
					if (!(z->dynamic & D_cached))
						z = source(z);
					if ((p = z->prereqs) && (p->rule != internal.dot || (p = p->next)))
					{
						DEBUGSOURCE(ndirs, z, p);
						dirs[ndirs++] = p;
					}
				}
			}
			if (a)
			{
				if (!(a->dynamic & D_cached))
					a = source(a);
				if ((p = a->prereqs) && (p->rule != internal.dot || (p = p->next)))
				{
					DEBUGSOURCE(ndirs, a, p);
					dirs[ndirs++] = p;
				}
			}
		}
		dot.rule = internal.dot;
		dot.next = 0;
#if DEBUG
		message((-14, "  [%d] %s", ndirs, internal.dot->name));
#endif
		dirs[ndirs++] = &dot;

		/*
		 * the nested loops preserve the directory search order
		 *
		 * .			current directory
		 * <source>.<pattern>	alternate directories (may pre-empt)
		 * <source>.x		suffix association directories
		 * <source>		default source directories
		 */

		files = getfile(base);
		ofiles = 0;

		/*
		 * first check if r is an archive member
		 */

		if (r && r->active && (r->active->parent->target->property & P_archive) && !(r->dynamic & D_membertoo))
		{
			Dir_t*		ar;

			i = 0;
			for (f = files; f; f = f->next)
				if (r->active->parent->target->name == f->dir->name && f->dir->archive)
				{
					i = 1;
					if (r->dynamic & D_member)
						error(1, "%s is duplicated within %s", r->name, r->active->parent->target->name);
					if (f->time >= r->time)
					{
						r->dynamic |= D_member;
						r->time = f->time;
					}
				}
			if (!i && (ar = getar(r->active->parent->target->name)) && (i = ar->truncate) && strlen(base) > i)
			{
				c = base[i];
				base[i] = 0;
				for (f = getfile(base); f; f = f->next)
				{
					if (r->active->parent->target->name == f->dir->name && f->dir->archive)
					{
						if (r->dynamic & D_member)
							error(1, "%s is duplicated within %s", r->name, r->active->parent->target->name);
						if (f->time >= r->time)
						{
							r->dynamic |= D_member;
							r->time = f->time;
						}
					}
				}
				base[i] = c;
			}
		}

		/*
		 * now check the directories
		 */

		view = state.maxview;
		for (i = ndirs; i-- > 0;)
		{
			for (p = dirs[i]; p; p = p->next)
			{
				d = p->rule;
				if (!(d->mark & M_directory))
				{
					/*UNDENT*/
	d->mark |= M_directory;
	c = (s = dir) && (s[0] != '.' || s[1] != '.' || s[2] != '/' && s[2] != 0);
	if (c && (d->property & P_terminal))
	{
		if (state.test & 0x00000008)
			error(2, "prune: %s + %s TERM", d->name, s);
		continue;
	}
	if (!(d->dynamic & D_scanned))
	{
		dirscan(d);
		files = getfile(base);
	}
	od = d;
	if (s)
	{
		if (c)
		{
			if (s = strchr(s, '/'))
				*s = 0;
			z = catrule(d->name, "/", dir, -1);
			if (z->dynamic & (D_entries|D_scanned))
			{
				if (s)
					*s = '/';
				if (!(z->dynamic & D_entries))
					continue;
			}
			else
			{
				for (f = getfile(dir), b = d->name; f; f = f->next)
					if (f->dir->name == b)
						break;
				if (!f && !(z->uname))
				{
					z->dynamic |= D_scanned;
					z->dynamic &= ~D_entries;
					if (s)
						*s = '/';
					if (state.test & 0x00000008)
						error(2, "prune: %s + %s HASH", d->name, dir);
					continue;
				}
				if (s)
				{
					*s = '/';
					z->dynamic |= D_entries;
				}
			}
			if (s)
				d = catrule(d->name, "/", dir, -1);
			else
				d = z;
		}
		else if (!(state.questionable & 0x00000080))
			d = catrule(d->name, "/", dir, -1);
		else
			for (;;)
				if (*s++ != '.' || *s++ != '.' || *s++ != '/')
				{
					d = *(s - 1) ? catrule(d->name, "/", dir, -1) : makerule(dir);
					break;
				}
		if (!(d->dynamic & D_scanned))
		{
			d->view = od->view;
			dirscan(d);
			files = getfile(base);
		}
	}
	if (!(d->dynamic & D_entries))
		continue;
	else if (!files && name[0] == '.' && (name[1] == 0 || name[1] == '/' || name[1] == '.' && (name[2] == 0 || name[2] == '/' || state.fsview && name[2] == '.' && (name[3] == 0 || name[3] == '/'))))
	{
		tmp_dir.name = d->name;
		ofiles = files;
		files = &tmp_file;
	}
#if DEBUG
	message((-11, "bindfile(%s): dir=%s", name, d->name));
#endif
	if (d->view <= view)
	{
		for (f = files; f; f = f->next)
		{
			s = f->dir->name;
			if (s == d->name && !f->dir->archive)
			{
				if (s == internal.dot->name)
					sfputr(buf, base, -1);
				else
					sfprintf(buf, "%s/%s", s, base);
				tm = f->time;
				s = sfstruse(buf);
				canon(s);
				st.st_mode = 0;
				if (tm == NOTIME)
				{
					x = 0;
					if (state.believe && d->view >= (state.believe - 1))
					{
						if (!r)
							r = makerule(name);
						r->view = d->view;
						r->mark |= M_bind;
						x = staterule(RULE, r, NiL, 0);
						r->mark &= ~M_bind;
					}
					if (x)
					{
						view = d->view;
						tm = x->time;
						message((-3, "%s: believe time [%s] from view %d", r->name, timestr(tm), view));
					}
					else if (rstat(s, &st, 0))
						tm = 0;
					else
					{
						tm = tmxgetmtime(&st);
						view = state.fsview ? iview(&st) : d->view;
					}
				}
				else
					view = d->view;
				if (tm)
				{
					if (!(flags & BIND_DOT) || !view)
					{
						if (view && state.fsview && state.expandview)
							mount(s, s, FS3D_GET|FS3D_VIEW|FS3D_SIZE(sfstrsize(buf)), NiL);
						found = 1;
						goto clear;
					}
					if (d->view > view)
						break;
				}
				else if (errno == ENODEV)
					view = d->view;
			}
		}
	}
	if (ofiles)
	{
		files = ofiles;
		ofiles = 0;
	}
					/*INDENT*/
				}
			}
		}
	clear:

		/*
		 * clear the visit marks
		 */

		for (i = ndirs; i-- > 0;)
			for (p = dirs[i]; p; p = p->next)
				p->rule->mark &= ~M_directory;
		if (!found && r && (name == r->name || reassoc) && (a = associate(internal.bind_p, r, NiL, NiL)) && (b = call(a, name)) && (s = getarg(&b, NiL)))
		{
			char*	t;

			/*
			 * [+|-] value [time]
			 *
			 *	-	name = name, time = time(value)
			 *	+	name = value, time = OLDTIME
			 *		name = value, time = time(time)
			 */

			if (streq(s, "-") || streq(s, "+"))
			{
				n = *s;
				s = getarg(&b, NiL);
			}
			else
				n = 0;
			if (s && !streq(s, name))
			{
				t = n ? getarg(&b, NiL) : (char*)0;
				message((-3, "%s name=%s n='%c' s=%s t=%s", a->name, name, n ? n : ' ', s, t));
				a = getrule(s);
				if (n == '+')
				{
					st.st_mode = 0;
					tmxsetmtime(&st, OLDTIME);
					sfputr(buf, s, 0);
				}
				else
				{
					if (t)
						st.st_mode = 0;
					else if (rstat(s, &st, 0))
					{
						if (!a || !(a->dynamic & D_bound) && !(a->property & P_target))
						{
							if (allocated)
								free(name);
							else
								allocated = 1;
							reassoc = !(state.questionable & 0x80000000);
							name = strdup(s);
							continue;
						}
						if (!(a->dynamic & D_bound))
						{
							st.st_mode = 0;
							tmxsetmtime(&st, 0);
						}
						else if (!a->time)
						{
							st.st_mode = 0;
							tm = CURTIME;
							tmxsetmtime(&st, tm);
						}
						else
						{
							st.st_mode = !(a->dynamic & D_regular);
							tmxsetmtime(&st, a->time);
						}
					}
					if (n == '-' || *b)
					{
						if (*s)
						{
							x = makerule(s);
							if (!(state.questionable & 0x00000004) && r != x && !r->time && !x->time && !(r->property & P_virtual))
							{
								r->property |= P_virtual;
								return bind(r);
							}
							putbound(name, x->name);
						}
						sfputr(buf, name, 0);
					}
					else
					{
						aliased = 1;
						sfputr(buf, s, 0);
						s = sfstrseek(buf, 0, SEEK_SET);
						canon(s);
						if (state.fsview && state.expandview && tmxgetmtime(&st) && iview(&st))
							mount(s, s, FS3D_GET|FS3D_VIEW|FS3D_SIZE(sfstrsize(buf)), NiL);
					}
				}
				tm = t ? timenum(t, NiL) : tmxgetmtime(&st);
				view = a ? a->view : state.maxview + 1;
				od = 0;
				found = 1;
			}
			else
				message((-3, "%s name=%s n='%c' s=%s", a->name, name, n ? n : ' ', s));
		}
		break;
	}
	if (!found && state.targetcontext && r && name != r->name && (x = getrule(base)) && (x->dynamic & D_context))
	{
		for (i = ndirs; i-- > 0;)
		{
			for (p = dirs[i]; p; p = p->next)
			{
				d = p->rule;
				if (!(d->mark & M_directory))
				{
					d->mark |= M_directory;
					if (d->name != internal.dot->name)
					{
						sfprintf(buf, "%s/%s", d->name, base);
						s = sfstruse(buf);
						canon(s);
						if ((x = getrule(s)) && (x->property & P_target))
						{
							found = 1;
							st.st_mode = 0;
							view = 0;
							tm = (x->time || (x = staterule(RULE, x, NiL, 0)) && x->time) ? x->time : CURTIME;
							goto context;
						}
					}
				}
			}
		}
	context:
		for (i = ndirs; i-- > 0;)
			for (p = dirs[i]; p; p = p->next)
				p->rule->mark &= ~M_directory;
	}
	if (found)
	{
		/*
		 * the file exists with:
		 *
		 *	buf	canonical file path name
		 *	od	original directory pointer
		 *	st	file stat() info
		 *	tm	file time
		 *	view	view index of dir containing file
		 */

		if (view > state.maxview)
			view = 0;
		b = sfstrseek(buf, 0, SEEK_SET);
#if DEBUG
		message((-11, "bindfile(%s): path=%s rule=%s alias=%s view=%d time=%s", name, b, r ? r->name : (char*)0, (x = getrule(b)) && x != r ? x->name : (char*)0, view, timestr(tm)));
#endif
		if (!r)
			r = makerule(name);
		if (internal.openfile)
			internal.openfile = r->name;
		if (!(r->dynamic & D_member) || tm > r->time)
		{
			if (r->dynamic & D_member)
			{
				r->dynamic &= ~D_member;
				r->dynamic |= D_membertoo;
			}
			r->time = tm;
			if (!(r->dynamic & D_entries))
			{
				if (S_ISREG(st.st_mode) || !st.st_mode)
					r->dynamic |= D_regular;
				if (!(r->dynamic & D_source) || (r->property & P_target))
					r->view = view;
			}
			if (!r->view && *b == '/')
			{
				if (strncmp(b, internal.pwd, internal.pwdlen) || *(b + internal.pwdlen) != '/')
				{
					if ((state.questionable & 0x00000800) || !(r->property & P_target))
						r->dynamic |= D_global;
				}
				else if ((r->dynamic & D_regular) && (x = getrule(b + internal.pwdlen + 1)) && x != r)
					r = bindalias(r, x, b + internal.pwdlen + 1, od, reassoc);
			}
			if (!(r->dynamic & D_global))
				r->preview = r->view;
			if (x = getrule(b))
			{
				if (x->dynamic & D_alias)
					x = makerule(x->name);
				else if (x == r && (r->property & P_terminal))
				{
					putrule(b, 0);
					x = 0;
				}
			}
			if (!(state.questionable & 0x00001000) && aliased && !x && !streq(name, r->name))
				x = makerule(name);
			if (x && x != r)
			{
				if (internal.openfile && st.st_mode)
					internal.openfile = x->name;
				if (r->property & x->property & P_target)
				{
					message((-2, "%s not aliased to %s", unbound(r), unbound(x)));
					if (!(state.questionable & 0x00000040))
						found = 0;
				}
				else if (r->dynamic & D_regular)
					r = bindalias(r, x, b, od, reassoc);
			}
			else
			{
				/*
				 * bind the rule to file name in b
				 * saving the unbound name
				 */

				s = r->name;
				r->name = maprule(b, r);
				if (internal.openfile && st.st_mode)
					internal.openfile = r->name;
				if (r->name != s)
				{
					r->uname = s;
					if (od && (s != name || state.mam.statix || (n = strlen(r->name)) < (i = strlen(s)) || r->name[n - i - 1] != '/' || !streq(s, r->name + n - i)))
						putbound(s, od->name);
				}
			}
			if ((r->dynamic & D_source) && r->uname)
				r->view = r->preview = view;
			if ((r->dynamic & D_regular) && r->view && (x = localrule(r, 0)))
				merge(x, r, MERGE_ALL|MERGE_BOUND);
		}
		else if (!state.accept && !view)
			r->view = r->active ? r->active->parent->target->view : state.maxview;
	}
	else if (!r)
	{
		if (!(flags & BIND_RULE))
			goto done;
		r = makerule(name);
	}

	/*
	 * propagate pattern association attributes
	 */

	bindattribute(r);

	/*
	 * archive binding and final checks
	 */

	if (found)
	{
		if (r->scan == SCAN_IGNORE)
			r->dynamic |= D_scanned;
		else if ((r->property & (P_archive|P_target)) == (P_archive|P_target))
			arscan(r);

		/*
		 * if name is partially qualified then check for
		 * alias bindings in the path suffixes of name
		 */

		if (!(state.questionable & 0x00010000) && dir && (r->dynamic & D_regular) && *(s = name) != '/')
			while (s = strchr(s, '/'))
				if ((a = getrule(++s)) && !(a->dynamic & D_bound))
					bindfile(a, NiL, 0);
	}
	else if (!(r->dynamic & D_member))
	{
		r->time = ((r->property & P_dontcare) || !(flags & BIND_FORCE)) ? 0 : CURTIME;
		if (r->property & P_dontcare)
			r->view = state.maxview;
	}
 done:
	if (allocated)
		free(name);
	sfstrclose(buf);
	sfstrclose(tmp);
	return r;
}

/*
 * propagate pattern association attributes
 */

void
bindattribute(register Rule_t* r)
{
	register Rule_t*	x;
	register Rule_t*	z;

	r->dynamic |= D_bound;
	if (x = associate(internal.attribute_p, r, NiL, NiL))
	{
		merge(x, r, MERGE_ASSOC|MERGE_ATTR);
		*x->name = ATTRCLEAR;
		if (z = getrule(x->name))
			negate(z, r);
		*x->name = ATTRNAME;
	}
}

/*
 * bind a rule, possibly changing the rule name
 */

Rule_t*
bind(register Rule_t* r)
{
	register Rule_t*	b;

	if (!r)
		return 0;
	if (r->dynamic & D_alias)
		r = makerule(r->name);
	if (r->dynamic & D_bound)
		return r;
	switch (r->property & (P_state|P_virtual))
	{
	case 0:
		if (b = bindfile(r, NiL, 0))
			return b;
		break;
	case P_state:
		if (b = bindstate(r, NiL))
		{
			if (state.mam.regress && (r->property & P_statevar))
				dumpregress(state.mam.out, "bind", r->name, r->statedata);
			return b;
		}
		break;
	case P_virtual:
		r->time = staterule(RULE, r, NiL, 1)->time;
		break;
	}
	bindattribute(r);
	return r;
}

/*
 * rebind rule r
 * op > 0 skips bindfile()
 * op < 0 skips statetime()
 */

void
rebind(register Rule_t* r, register int op)
{
	char*		t;
	Rule_t*		x;

	if (!(r->property & P_state))
	{
		if (r->uname)
			oldname(r);
		r->dynamic &= ~(D_bound|D_entries|D_member|D_scanned);
		if (op > 0)
			r->dynamic |= D_bound;
		else
		{
			newfile(r, NiL, NOTIME);
			if ((t = strchr(r->name, '/')) && (x = getrule(t + 1)) && (x = bindfile(x, NiL, 0)))
				r = x;
			else
				bindfile(r, NiL, 0);
		}
		if (op >= 0)
			r->dynamic |= D_triggered;
		if (!state.exec)
		{
			r->time = CURTIME;
			r->status = EXISTS;
		}
		else if (op >= 0)
		{
			statetime(r, !op);
			r->status = r->time ? EXISTS : (op || (r->property & P_dontcare)) ? IGNORE : FAILED;
		}
	}
	else if (r->property & P_statevar)
	{
		if (op <= 0)
			r->dynamic &= ~D_bound;
		if (!(r->dynamic & D_bound) && !(r = bindstate(r, NiL)))
			return;
		if (op > 0)
			r->time = OLDTIME;
	}
	message((-2, "%s(%s) = %s", op > 0 ? "accept" : "rebind", r->name, timestr(r->time)));
}

/*
 * remove binding on r
 * candidates have s==0 or r->mark==1
 * h!=0 for alias reference
 */

int
unbind(const char* s, char* v, void* h)
{
	register Rule_t*	r = (Rule_t*)v;

	if (!s || !h && (r->mark & M_mark) || h && (r->dynamic & D_alias) && (makerule(r->name)->mark & M_mark))
	{
		message((-2, "unbind(%s)%s%s", r->name, h ? " check-alias" : null, s && streq(s, r->name) ? null : " diff-hash"));
		r->mark &= ~M_mark;
		if (r->property & P_metarule)
			r->uname = 0;
		else
		{
			int	u = 0;

			if (!(r->property & P_state))
			{
				newfile(r, NiL, NOTIME);
				if (r->uname)
				{
					oldname(r);
					u = 1;
				}
			}
			else if (r->property & P_staterule)
			{
				error(2, "%s: cannot unbind staterules", r->name);
				return 0;
			}
			r->dynamic &= ~(D_bound|D_entries|D_global|D_member|D_membertoo|D_regular|D_scanned);
			r->must = 0;
			r->scan = 0;
			r->status = NOTYET;
			r->time = 0;
			if (u || !(r->dynamic & D_source) || (r->property & P_target))
			{
				r->preview = 0;
				r->view = 0;
			}
		}
	}
	return 0;
}

/*
 * append views of absolute path s to source list z
 */

static List_t*
absolute(List_t* z, char* s, Sfio_t* tmp)
{
	int	i;
	int	j;
	int	n;
	Rule_t*	x;
	char*	t;

	if (state.questionable & 0x00000004)
		return z;
	for (i = 0; i < state.maxview; i++)
	{
		if (!strncmp(s, state.view[i].root, n = state.view[i].rootlen) && (!*(s + n) || *(s + n) == '/'))
		{
			s += n;
			j = i;
			while (++j <= state.maxview)
			{
				sfprintf(tmp, "%s%s", state.view[j].root, s);
				t = sfstruse(tmp);
				if (!(x = getrule(t)))
					x = makerule(t);
				if (!(x->mark & M_directory))
				{
					x->mark |= M_directory;
					z = z->next = cons(x, NiL);
				}
			}
			break;
		}
	}
	return z;
}

/*
 * fix up .SOURCE prereqs after user assertion
 */

Rule_t*
source(register Rule_t* r)
{
	register Rule_t*	x;

	if (state.compile > COMPILED)
		return r;
	if (state.compile < COMPILED)
	{
		x = r;
		r = catrule(x->name, internal.internal->name, NiL, 1);
#if _HUH_2001_10_31
		freelist(r->prereqs);
#endif
		r->prereqs = listcopy(x->prereqs);
		r->dynamic |= D_compiled | (x->dynamic & D_dynamic);
		r->property |= P_readonly;
	}
	r->dynamic |= D_cached;
	if (r->dynamic & D_dynamic)
		dynamic(r);
	if (state.maxview && !state.fsview)
	{
		register char*		s;
		register char*		t;
		register List_t*	p;
		int			dot;
		unsigned int		view;
		List_t*			z;
		List_t			lst;
		Sfio_t*			tmp;

		/*
		 * recompute view cross product
		 *
		 *	.	unit multiplication operand
		 *	A	absolute path rooted at /
		 *	R	path relative to .
		 *
		 *	lhs	rhs	cross-product
		 *	----	-----	-------------
		 *	.	.	.	*note (1)*
		 *	.	A	A	*note (2)*
		 *	.	R	R
		 *	A	.	A
		 *	A	R	A/R
		 *	A	A	A	*note (2)*
		 *	R	.	R
		 *	R	A	A	*note (2)*
		 *	R	R	R/R
		 *
		 *	(1) the first . lhs operand produces a . in the product
		 *
		 *	(2) the first A rhs operand is placed in the product
		 */

		debug((-5, "%s: recompute view cross product", r->name));
		tmp = sfstropen();
		z = &lst;
		z->next = 0;
		if (state.strictview)
		{
			/*
			 * this follows 3d fs semantics
			 */

			for (p = r->prereqs; p; p = p->next)
			{
				if (*(t = unbound(p->rule)) == '/')
				{
					sfputr(tmp, t, -1);
					t = sfstruse(tmp);
					pathcanon(t, 0, 0);
					if (!(x = getrule(t)))
						x = makerule(t);
					if (!(x->mark & M_directory))
					{
						x->mark |= M_directory;
						z = z->next = cons(x, NiL);
					}
					if (*(t = unbound(x)) == '/')
						z = absolute(z, t, tmp);
				}
				else if (!p->rule->view)
				{
					dot = (*t == '.' && !*(t + 1));
					for (view = 0; view <= state.maxview; view++)
					{
#if BINDINDEX
						s = state.view[view].path->name;
#else
						s = state.view[view].path;
#endif
						if (dot || *s != '.' || *(s + 1))
						{
							if (dot)
								sfputr(tmp, s, -1);
							else
								sfprintf(tmp, "%s/%s", s, t);
						}
						else
							sfputr(tmp, t, -1);
						s = sfstruse(tmp);
						pathcanon(s, 0, 0);
						x = makerule(s);
						if (!(x->dynamic & D_source))
						{
							x->dynamic |= D_source;
							if (x->view < view)
								x->view = view;
						}
						if (!(x->mark & M_directory))
						{
							x->mark |= M_directory;
							z = z->next = cons(x, NiL);
						}
					}
				}
			}
		}
		else
		{
			List_t*		q;
			int		dotted = 0;

			q = r->prereqs;
			do
			{
				for (view = 0; view <= state.maxview; view++)
				{
#if BINDINDEX
					s = state.view[view].path->name;
#else
					s = state.view[view].path;
#endif
					if ((dot = (*s == '.' && !*(s + 1))) && !dotted)
					{
						dotted = 1;
#if BINDINDEX
						z = z->next = cons(state.view[view].path, NiL);
#else
						z = z->next = cons(makerule(s), NiL);
#endif
					}
					for (p = q; p; p = p->next)
					{
						if (*(t = unbound(p->rule)) == '/')
						{
							z = absolute(z, t, tmp);
							break;
						}
						if (!p->rule->view || (p->rule->property & P_target) && !strchr(t, '/'))
						{
							if (*t == '.' && !*(t + 1))
							{
								if (!dotted)
								{
									dotted = 1;
									sfputr(tmp, t, -1);
									t = sfstruse(tmp);
									pathcanon(t, 0, 0);
									z = z->next = cons(makerule(t), NiL);
								}
								if (dot)
									continue;
								sfputr(tmp, s, -1);
							}
							else
							{
								if (dot)
									sfputr(tmp, t, -1);
								else
									sfprintf(tmp, "%s/%s", s, t);
							}
							t = sfstruse(tmp);
							pathcanon(t, 0, 0);
							x = makerule(t);
							if (!(x->dynamic & D_source))
							{
								x->dynamic |= D_source;
								if (x->view < view)
									x->view = view;
							}
							if (!(x->mark & M_directory))
							{
								x->mark |= M_directory;
								z = z->next = cons(x, NiL);
							}
						}
					}
				}
				for (; p && *(t = unbound(p->rule)) == '/'; p = p->next)
				{
					sfputr(tmp, t, -1);
					t = sfstruse(tmp);
					pathcanon(t, 0, 0);
					if (!(x = getrule(t)))
						x = makerule(t);
					if (!(x->mark & M_directory))
					{
						x->mark |= M_directory;
						z = z->next = cons(x, NiL);
					}
					if (*(s = unbound(x)) == '/')
						z = absolute(z, s, tmp);
				}
			} while (q = p);
		}
		sfstrclose(tmp);
		freelist(r->prereqs);
		for (r->prereqs = p = lst.next; p; p = p->next)
			p->rule->mark &= ~M_directory;
	}
	return r;
}

#if BINDINDEX
/*
 * copy path name of r to s
 * end of s is returned
 */

char*
pathname(register char* s, register Rule_t* r)
{
	if ((r->dynamic & D_bound) && !(r->property & (P_state|P_virtual)) && *r->name != '/')
	{
		if (!state.logical && r->view && !(r->dynamic & D_bindindex))
		{
			s = stpcpy(s, state.view[r->view].path);
			*s++ = '/';
		}
		if (r->source && !(r->dynamic & D_bindindex))
		{
			s = stpcpy(s, state.source[r->source].path);
			*s++ = '/';
		}
	}
	return stpcpy(s, r->name);
}
#endif

/*
 * return local view path name for r
 */

char*
localview(register Rule_t* r)
{
	register Rule_t*	x;
	int			i;

	if (r->dynamic & D_alias)
		r = makerule(unbound(r));
	if (state.context && !(r->property & (P_state|P_virtual)))
	{
		register char*	s = r->name;

		if (*s == '/' || iscontext(s))
			return s;
		sfprintf(state.context, "%c%s%c", MARK_CONTEXT, s, MARK_CONTEXT);
		x = makerule(sfstruse(state.context));
		if (!(x->dynamic & D_alias))
		{
			x->property |= P_internal;
			x->dynamic |= D_alias;
			x->uname = x->name;
			x->name = s;
		}
		return x->uname;
	}
	if (!state.maxview || state.fsview && !state.expandview)
		return r->name;
	if (x = localrule(r, 1))
		return x->name;
	if (r->view && r->uname)
	{
		if (state.mam.statix)
		{
			if (*r->name != '/')
				return r->name;
			for (i = 1; i <= state.maxview; i++)
				if (strneq(r->name, state.view[i].path, state.view[i].pathlen) && r->name[state.view[i].pathlen] == '/')
					return r->name + state.view[i].pathlen + 1;
		}
		return r->uname;
	}
	return r->name;
}
