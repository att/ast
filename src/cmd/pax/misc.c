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
 * AT&T Bell Laboratories
 *
 * pax miscellaneous support
 */

#include "pax.h"

#include <dlldefs.h>
#include <sfdisc.h>
#include <tmx.h>

static Format_t*
scan(void)
{
	register Format_t*	fp;
	Format_t*		lp;
	Format_t*		rp;
	Dllscan_t*		dls;
	Dllent_t*		dle;
	void*			dll;
	Paxlib_f		init;

	for (fp = formats; fp->next; fp = fp->next);
	rp = fp;
	if (dls = dllsopen(state.id, NiL, NiL))
	{
		while (dle = dllsread(dls))
			if (dll = dlopen(dle->path, RTLD_LAZY))
			{
				if (dllcheck(dll, dle->path, PAX_PLUGIN_VERSION, NiL) &&
				    (init = (Paxlib_f)dlllook(dll, "pax_lib")) &&
				    (lp = (*init)(&state)))
					fp = fp->next = lp;
				else
					dlclose(dll);
			}
			else
				message((-1, "%s: %s", dle->path, dlerror()));
		dllsclose(dls);
	}
	return rp->next;
}

/*
 * format list iterator
 * fp=0 for first element
 * dll format scan kicked in when static formats exhausted
 */

Format_t*
nextformat(Format_t* fp)
{
	if (!fp)
		return formats;
	if (!(fp = fp->next) && !state.scanned)
	{
		state.scanned = 1;
		fp = scan();
	}
	return fp;
}

/*
 * return format given name
 */

Format_t*
getformat(register char* name, int must)
{
	register Format_t*	fp;

	if (!name || !*name || streq(name, "-"))
		name = FMT_DEFAULT;
	fp = 0;
	while (fp = nextformat(fp))
		if (!strcasecmp(name, fp->name) || fp->match && strgrpmatch(name, fp->match, NiL, 0, STR_ICASE|STR_LEFT|STR_RIGHT))
			return fp;
	if (must)
		error(3, "%s: unknown format", name);
	return 0;
}

/*
 * path name strcmp()
 */

static int
pathcmp(register const char* s, register const char* t)
{
	register int	sc;
	register int	tc;

	for (;;)
	{
		tc = *t++;
		if (!(sc = *s++))
			return tc ? -1 : 0;
		if (sc != tc)
		{
			if (tc == 0 || tc == '/')
				return 1;
			if (sc == '/')
				return -1;
			return strcoll(s - 1, t - 1);
		}
	}
}

/*
 * check base archive ordering
 */

static void
ordered(Archive_t* ap, const char* prv, const char* cur)
{
	if (pathcmp(prv, cur) > 0)
		error(3, "%s: %s: archive member must appear before %s", ap->name, prv, cur);
}

/*
 * check f with patterns given on cmd line
 */

int
selectfile(register Archive_t* ap, register File_t* f)
{
	register Archive_t*	bp;
	register Member_t*	d;
	int			linked = 0;
	int			c;
	Tv_t			t;

	ap->info = 0;
	if (f->skip || f->namesize <= 1 || f->linkpath && !*f->linkpath)
		return 0;
	if (state.ordered)
	{
		ordered(ap, ap->path.prev.string, f->name);
		stash(&ap->path.prev, f->name, 0); /*HERE*/
	}
	if (f->record.format && state.record.pattern)
	{
		static char	fmt[2];

		fmt[0] = f->record.format;
		if (!strmatch(fmt, state.record.pattern))
			return 0;
	}
	if (ap->parent)
	{
		linked = 1;
		addlink(ap, f);
		if (!ap->parent)
			return 0;
		if (!(d = newof(0, Member_t, 1, 0)))
			nospace();
		d->dev = f->st->st_dev;
		d->ino = f->st->st_ino;
		d->mode = f->st->st_mode;
		tvgetatime(&d->atime, f->st);
		tvgetmtime(&d->mtime, f->st);
		d->offset = ap->io->offset + ap->io->count;
		d->size = f->st->st_size;
		d->uncompressed = f->uncompressed;
		if (!(d->info = (File_t*)memdup(f, sizeof(File_t))) || !(d->info->st = (struct stat*)memdup(f->st, sizeof(struct stat))))
			nospace();
		d->info->name = d->info->path = hashput(ap->parent->delta->tab, f->name, d);
		if (d->info->linktype != NOLINK)
			d->info->linkpath = strdup(d->info->linkpath);
		if (d->info->uidname)
			d->info->uidname = strdup(d->info->uidname);
		if (d->info->gidname)
			d->info->gidname = strdup(d->info->gidname);
		d->info->delta.base = d;
		ap->info = d->info;
		if (!state.ordered)
			return 0;
	}
	if (!match(f->path))
		return 0;
	if (f->type != X_IFDIR && ap->update && (d = (Member_t*)hashget(ap->update, f->name)))
	{
		if (f->type != X_IFREG && d->dev != f->st->st_dev)
			/* keep */;
		else if (!(c = tvcmp(&d->mtime, tvmtime(&t, f->st))))
			return 0;
		else if (state.update != OPT_different && c > 0)
			return 0;
		else
			return 0;
	}
	if (state.verify && !verify(ap, f, NiL))
		return 0;
	ap->selected++;
	if (!linked)
	{
		if (state.list)
			addlink(ap, f);
		if (ap->tab)
		{
			if (!(d = newof(0, Member_t, 1, 0)))
				nospace();
			d->dev = f->st->st_dev;
			d->ino = f->st->st_ino;
			d->mode = f->st->st_mode;
			tvgetatime(&d->atime, f->st);
			tvgetmtime(&d->mtime, f->st);
			d->offset = ap->io->offset + ap->io->count;
			d->size = f->st->st_size;
			d->uncompressed = f->uncompressed;
			hashput(ap->tab, f->path, d);
		}
	}
	if (state.ordered && ap->delta && !(ap->delta->format->flags & COMPRESS) && (bp = ap->delta->base))
	{
		register int	n;
		register int	m;

		for (;;)
		{
			if (bp->peek)
				bp->peek = 0;
			else
			{
				if (bp->skip && bp->skip == bp->io->offset + bp->io->count)
					fileskip(bp, &bp->file);
				if (!getheader(bp, &bp->file))
					break;
				bp->skip = bp->io->offset + bp->io->count;
			}
			ordered(bp, bp->path.prev.string, bp->file.name);
			if ((m = pathcmp(bp->file.name, f->name)) > 0)
			{
				bp->peek = 1;
				break;
			}
			n = selectfile(bp, &bp->file);
			if (!m)
				break;
			if (n && !state.list)
			{
				if (ap->io->mode != O_RDONLY)
				{
					File_t		tmp;
					struct stat	st;

					initfile(ap, &tmp, &st, bp->file.name, X_IFREG);
					tmp.delta.op = DELTA_delete;
					putheader(ap, &tmp);
					puttrailer(ap, &tmp);
				}
				else
				{
					struct stat	st;

					if (!(*state.statf)(f->name, &st))
					{
						if (S_ISDIR(st.st_mode))
						{
							if (!streq(f->name, ".") && !streq(f->name, ".."))
							{
								if (rmdir(f->name))
									error(ERROR_SYSTEM|2, "%s: cannot remove directory", f->name);
								else
									listentry(f);
							}
						}
						else if (remove(f->name))
							error(ERROR_SYSTEM|2, "%s: cannot remove file", f->name);
						else
							listentry(f);
					}
				}
			}
		}
	}
	return 1;
}

/*
 * verify action on file
 *
 *	EOF	exit
 *	NULL	skip file
 *	.	keep file
 *	<else>	rename file
 */

int
verify(Archive_t* ap, register File_t* f, register char* prompt)
{
	register char*	name;

	NoP(ap);
	if (!prompt)
	{
		if (state.yesno < 0)
			return 0;
		else if (state.yesno)
			switch (state.operation)
			{
			case IN:
				prompt = "Read";
				break;
			case OUT:
				prompt = "Write";
				break;
			default:
				prompt = "Pass";
				break;
			}
		else
			prompt = "Rename";
	}
	sfprintf(state.wtty, "%s %s: " , prompt, f->name);
	if (!(name = sfgetr(state.rtty, '\n', 1)))
	{
		sfputc(state.wtty, '\n');
		finish(2);
	}
	if (state.yesno)
	{
		if (*name == 'q' || *name == 'Q' || *name == '-')
		{
			state.yesno = -1;
			return 0;
		}
		return *name == 'y' || *name == 'Y' || *name == '1';
	}
	switch (*name)
	{
	case 0:
		return 0;
	case '.':
		if (!*(name + 1))
			break;
		/*FALLTHROUGH*/
	default:
		f->namesize = pathcanon(f->name = name, 0, 0) - name + 1;
		break;
	}
	return 1;
}

/*
 * no dos in our pathnames
 */

void
undos(File_t* f)
{
	register char*	s;

	if (strchr(f->name, '\\'))
	{
		s = f->name;
		if (s[1] == ':' && isalpha(s[0]))
		{
			if (*(s += 2) == '\\' || *s == '/')
				s++;
			f->name = s;
		}
		for (; *s; s++)
			if (*s == '\\')
				*s = '/';
	}
}

/*
 * check for file name mapping
 * static data possibly returned
 * two simultaneous calls supported
 */

char*
map(Archive_t* ap, register char* name)
{
	register Map_t*	mp;
	char*		to;
	char*		from;
	File_t		f;
	int		n;
	regmatch_t	match[10];

	if (state.filter.line > 1)
	{
		state.filter.line = 1;
		name = state.filter.name;
	}
	from = to = name;
	for (mp = state.maps; mp; mp = mp->next)
		if (!(n = regexec(&mp->re, from, elementsof(match), match, 0)))
		{
			if (n = regsubexec(&mp->re, from, elementsof(match), match))
				regfatal(&mp->re, 3, n);
			n = strlen(mp->re.re_sub->re_buf) + 1;
			if (!(to = fmtbuf(n + ((mp->flags & MAP_INDEX) ? 10 : 0))))
				nospace();
			memcpy(to, mp->re.re_sub->re_buf, n);
			if (mp->flags & MAP_INDEX)
				sfsprintf(to + n - 1, 10, ".%04d", ap->entry);
			if (mp->re.re_sub->re_flags & REG_SUB_PRINT)
				sfprintf(sfstderr, "%s >> %s\n", from, to);
			if (mp->re.re_sub->re_flags & REG_SUB_STOP)
				break;
			from = to;
		}
		else if (n != REG_NOMATCH)
			regfatal(&mp->re, 3, n);
	if (state.local && (*to == '/' || *to == '.' && *(to + 1) == '.' && (!*(to + 2) || *(to + 2) == '/')))
	{
		if (state.verify)
		{
			f.name = to;
			if (verify(NiL, &f, "Retain non-local file"))
				return f.name;
		}
		error(1, "%s: non-local path rejected", to);
		to = "";
	}
	return to;
}

typedef struct
{
	Archive_t*	archive;
	File_t*		file;
} List_handle_t;

#define TYPE_mode	1
#define TYPE_time	2

/*
 * sfkeyprintf() lookup
 */

static int
listlookup(void* handle, register Sffmt_t* fmt, const char* arg, char** ps, Sflong_t* pn)
{
	List_handle_t*		gp = (List_handle_t*)handle;
	register File_t*	f = gp->file;
	register struct stat*	st = f->st;
	char*			s = 0;
	Sflong_t		n = 0;
	Time_t			t = TMX_NOTIME;
	int			type = 0;
	int			k;
	char*			e;
	Option_t*		op;

	static const char	fmt_time[] = "time=%?%l";
	static const char	fmt_mode[] = "mode";

	if (fmt->t_str)
	{
		if (!(op = (Option_t*)hashget(state.options, fmt->t_str)))
		{
			if (*fmt->t_str != '$')
				return 0;
			if (!(op = newof(0, Option_t, 1, 0)))
				nospace();
			op->name = hashput(state.options, 0, op);
			op->macro = getenv(fmt->t_str + 1);
			op->index = OPT_environ;
			op->flags |= OPT_DISABLE;
		}
		if (op->macro && !(op->flags & OPT_DISABLE))
		{
			op->flags |= OPT_DISABLE;
			if (!(state.tmp.mac && !(state.tmp.mac = sfstropen())))
				nospace();
			sfkeyprintf(state.tmp.mac, handle, op->macro, listlookup, NiL);
			if (!(s = sfstruse(state.tmp.mac)))
				nospace();
			op->flags &= ~OPT_DISABLE;
		}
		else
			switch (op->index)
			{
			case OPT_atime:
				n = st->st_atime;
				t = tmxgetatime(st);
				type = TYPE_time;
				break;
			case OPT_charset:
				s = "ASCII";
				break;
			case OPT_ctime:
				n = st->st_ctime;
				t = tmxgetctime(st);
				type = TYPE_time;
				break;
			case OPT_delta_op:
				if (f->uncompressed && (k = (st->st_size * 100) / f->uncompressed) < 100)
					sfsprintf(s = fmtbuf(32), 32, "%c%02d.%1d%%", f->delta.op ? f->delta.op : 'c', k, (int)((st->st_size * 1000) / f->uncompressed) % 10);
				else
					switch (f->delta.op)
					{
					case 0:
					case DELTA_pass:
						return 0;
					case DELTA_create:
						s = "create";
						break;
					case DELTA_delete:
						s = "delete";
						break;
					case DELTA_update:
						s = "update";
						break;
					case DELTA_verify:
						s = "verify";
						break;
					default:
						sfsprintf(s = fmtbuf(8), 8, "[op=%c]", f->delta.op);
						break;
					}
				break;
			case OPT_device:
				if (f->type == X_IFBLK || f->type == X_IFCHR)
					s = fmtdev(st);
				else
					return 0;
				break;
			case OPT_devmajor:
				n = major(st->st_dev);
				break;
			case OPT_devminor:
				n = minor(st->st_dev);
				break;
			case OPT_dir:
				if (s = strrchr(f->name, '/'))
				{
					sfwrite(state.tmp.fmt, f->name, s - f->name);
					if (!(s = sfstruse(state.tmp.fmt)))
						nospace();
				}
				else
					s = ".";
				break;
			case OPT_entry:
				n = gp->archive->entry;
				break;
			case OPT_environ:
				if (!(s = op->macro))
					return 0;
				break;
			case OPT_gname:
				if (f->gidname)
				{
					if (fmt->fmt == 's')
						s = f->gidname;
					else
						n = strgid(f->gidname);
					break;
				}
				/*FALLTHROUGH*/
			case OPT_gid:
				if (fmt->fmt == 's')
					s = fmtgid(st->st_gid);
				else
					n = st->st_gid;
				break;
			case OPT_ino:
				n = st->st_ino;
				break;
			case OPT_linkop:
				switch (f->linktype)
				{
				case HARDLINK:
					s = "==";
					break;
				case SOFTLINK:
					s = "->";
					break;
				default:
					return 0;
				}
				break;
			case OPT_linkpath:
				if (f->linktype == NOLINK)
					return 0;
				s = f->linkpath;
				break;
			case OPT_mark:
				if (f->linktype == HARDLINK)
					s = "=";
				else if (f->linktype == SOFTLINK)
					s = "@";
				else if (f->type == X_IFDIR)
					s = "/";
				else if (f->type == X_IFIFO)
					s = "|";
				else if (f->type == X_IFSOCK)
					s = "=";
				else if (f->type == X_IFBLK || f->type == X_IFCHR)
					s = "$";
				else if (st->st_mode & (X_IXUSR|X_IXGRP|X_IXOTH))
					s = "*";
				else
					return 0;
				break;
			case OPT_mode:
				n = st->st_mode;
				type = TYPE_mode;
				break;
			case OPT_mtime:
				n = st->st_mtime;
				t = tmxgetmtime(st);
				type = TYPE_time;
				break;
			case OPT_name:
				if (s = strrchr(f->name, '/'))
					s++;
				else
					s = f->name;
				break;
			case OPT_nlink:
				n = st->st_nlink;
				break;
			case OPT_path:
				s = f->name;
				break;
			case OPT_pid:
				n = state.pid;
				break;
			case OPT_release:
				s = release();
				break;
			case OPT_sequence:
				if (gp->archive->volume)
					sfsprintf(s = fmtbuf(32), 32, "%d-%d", gp->archive->volume, gp->archive->entry);
				else
					n = gp->archive->entry;
				break;
			case OPT_size:
				if (f->linktype == SOFTLINK)
					n = f->linkpathsize - 1;
				else if (f->uncompressed)
					n = f->uncompressed;
				else
					n = (Sfulong_t)st->st_size;
				break;
			case OPT_tmp:
				if (s = strrchr(state.tmp.file, '/'))
				{
					sfwrite(state.tmp.fmt, state.tmp.file, s - state.tmp.file);
					if (!(s = sfstruse(state.tmp.fmt)))
						nospace();
				}
				else
					s = ".";
				break;
			case OPT_uname:
				if (f->uidname)
				{
					if (fmt->fmt == 's')
						s = f->uidname;
					else
						n = struid(f->uidname);
					break;
				}
				/*FALLTHROUGH*/
			case OPT_uid:
				if (fmt->fmt == 's')
					s = fmtuid(st->st_uid);
				else
					n = st->st_uid;
				break;
			default:
				if (gp->archive && gp->archive->format->lookup)
				{
					if ((k = (*gp->archive->format->lookup)(&state, gp->archive, f, op->index, &s, &n)) < 0)
						return 0;
					else if (k > 0)
						break;
				}
				if (!(op->flags & OPT_SET) || !(s = op->perm.string))
					return 0;
				break;
			}
	}
	else
	{
		op = 0;
		switch (fmt->fmt)
		{
		case 'd':
			if (!op)
				s = f->name;
			if (e = strrchr(s, '/'))
			{
				sfwrite(state.tmp.fmt, s, e - s);
				if (!(s = sfstruse(state.tmp.fmt)))
					nospace();
			}
			else
				s = ".";
			*ps = s;
			fmt->fmt = 's';
			return 1;
		case 'f':
			fmt->fmt = 'F';
			break;
		case 'n':
			*pn = f->ap->entry;
			fmt->fmt = 'u';
			return 1;
		case 'p':
			*pn = state.pid;
			fmt->fmt = 'u';
			return 1;
		}
	}
	switch (fmt->fmt)
	{
	case 'D':
		if (f->type == X_IFBLK || f->type == X_IFCHR)
			s = fmtdev(st);
		else if (!op)
			s = " ";
		else
		{
			*pn = n;
			fmt->fmt = 'u';
			return 1;
		}
		break;
	case 'L':
		if (f->linktype != NOLINK)
		{
			if (!op)
				s = f->name;
			sfprintf(state.tmp.fmt, "%s %s %s", s, f->linktype == HARDLINK ? "==" : "->", f->linkpath);
			if (!(s = sfstruse(state.tmp.fmt)))
				nospace();
			break;
		}
		/*FALLTHROUGH*/
	case 'F':
		if (!op)
			s = f->name;
		if (e = strrchr(s, '/'))
			s = e + 1;
		break;
	case 'M':
		if (!op)
			n = st->st_mode;
		s = fmtmode(n, 1);
		break;
	case 'T':
		if (!arg)
			arg = "%b %e %H:%M %Y";
		if (!op)
		{
			n = st->st_mtime;
			t = tmxgetmtime(st);
		}
		if ((unsigned long)n >= state.testdate)
		{
			n = state.testdate;
			t = TMX_NOTIME;
		}
		s = t == TMX_NOTIME ? fmttime(arg, n) : fmttmx(arg, t);
		break;
	default:
		if (s)
			*ps = s;
		else if (fmt->fmt == 's' && (arg || type))
		{
			if (type == TYPE_mode || arg && *pn == ':' && strneq(arg, fmt_mode, 4))
				*ps = fmtmode(n, 1);
			else if ((k = arg && *pn == ':' && strneq(arg, fmt_time, 4)) || type == TYPE_time)
			{
				if (k && *(arg + 4) == '=')
					arg += 5;
				if (!arg || !*arg)
					arg = fmt_time + 5;
				if ((unsigned long)n >= state.testdate)
				{
					n = state.testdate;
					t = TMX_NOTIME;
				}
				*ps = t == TMX_NOTIME ? fmttime(arg, (time_t)n) : fmttmx(arg, t);
			}
		}
		else
			*pn = n;
		return 1;
	}
	*ps = s;
	fmt->fmt = 's';
	return 1;
}

/*
 * set up lookup() handle and call sfkeyprintf()
 */

int
listprintf(Sfio_t* sp, Archive_t* ap, File_t* f, const char* format)
{
	List_handle_t	list;

	list.archive = ap;
	list.file = f;
	return sfkeyprintf(sp, &list, format, listlookup, NiL);
}

/*
 * list entry information based on state.drop, state.list and state.verbose
 */

void
listentry(register File_t* f)
{
	int	n;
	int	p;
	int	i;
	int	j;
	int	k;
	char*	s;
	char*	e;
	char	bar[METER_parts + 1];

	if (!f->extended && !f->skip && (state.drop || state.list || state.meter.on || state.verbose))
	{
		if (state.meter.on)
		{
			for (s = f->name; *s; s++)
				if (s[0] == ' ' && s[1] == '-' && s[2] == '-' && s[3] == ' ')
					break;
			if (*s)
			{
				if (state.meter.last)
				{
					sfprintf(sfstderr, "%*s", state.meter.last, "\r");
					state.meter.last = 0;
				}
				sfprintf(sfstderr, "\n");
				listprintf(sfstderr, state.in, f, state.listformat);
				sfprintf(sfstderr, "\n\n");
				return;
			}
			n = state.in->io->count > 1024 ? 10 : 0;
			if ((p = ((state.in->io->count >> n) * 100) / (state.meter.size >> n)) >= 100)
				p = 99;
			n = listprintf(state.meter.tmp, state.in, f, state.listformat);
			if (!(s = sfstruse(state.meter.tmp)))
				nospace();
			if (state.meter.fancy)
			{
				if (n > (state.meter.width - METER_width))
				{
					e = "*";
					s += n - (state.meter.width - METER_width) + 1;
					n = state.meter.width - METER_width;
				}
				else
					e = "";
				j = n + METER_width;
				if (!state.meter.last)
					state.meter.last = j;
				if ((k = state.meter.last - j) < 0)
					k = 0;
				if ((i = (p / (100 / METER_parts))) >= sizeof(bar))
					i = sizeof(bar) - 1;
				n = 0;
				while (n < i)
					bar[n++] = '*';
				while (n < elementsof(bar) - 1)
					bar[n++] = ' ';
				bar[n] = 0;
				state.meter.last = sfprintf(sfstderr, "%02d%% |%s| %s%s%*s\r", p, bar, e, s, k, "") - k - 1;
			}
			else
				sfprintf(sfstderr, "%02d%% %s\n", p, s);
			sfsync(sfstderr);
			if (state.test & 0000200)
				sleep(1);
		}
		else if (state.drop)
		{
			if (++state.dropcount >= 50)
			{
				state.dropcount = 0;
				sfprintf(sfstderr, ".\n");
			}
			else
			{
				sfprintf(sfstderr, ".");
				sfsync(sfstderr);
			}
		}
		else
			listprintf(state.list ? sfstdout : sfstderr, state.in, f, state.listformat);
	}
}

/*
 * prepare patterns for match()
 */

void
initmatch(char** v)
{
	register char*	s;
	register char**	a;
	Pattern_t*	p;
	size_t		n;
	size_t		m;

	m = 0;
	a = v;
	while (*a)
		m += strlen(*a++) + 1;
	n = a - v + 1;
	if (!(p = newof(0, Pattern_t, n, m)))
		nospace();
	state.pattern = state.patterns = p;
	s = (char*)(p + n);
	for (a = v; *a; a++, p++)
	{
		s = stpcpy(p->pattern = s, *a) + 1;
		pathcanon(p->pattern, s - p->pattern, 0);
	}
}

/*
 * determine if file s matches input patterns
 */

int
match(register char* s)
{
	register Pattern_t*	p;
	int			n;

	if (!(p = state.pattern))
		return state.matchsense;
	if (state.exact)
	{
		for (n = 0; p->pattern; p++)
			if (!p->matched || p->directory)
			{
				if (state.descend && dirprefix(p->pattern, s, p->directory))
				{
					state.pattern = p;
					p->directory = p->matched = 1;
					return 1;
				}
				else if (p->directory)
					p->directory = 0;
				else if (strmatch(s, p->pattern))
				{
					state.pattern = p;
					p->matched = 1;
					return 1;
				}
				else
					n = 1;
			}
		if (!n)
			finish(0);
	}
	else
		for (; p->pattern; p++)
			if (state.descend && dirprefix(p->pattern, s, 0) || strmatch(s, p->pattern))
				return state.matchsense;
	return !state.matchsense;
}

/*
 * return 1 if p is a directory prefix of s
 */

int
dirprefix(register char* p, register char* s, int proper)
{
	if (*p == '.' && !*(p + 1) && *s != '/' && (*s != '.' || *(s + 1) != '.' || *(s + 2) && *(s + 2) != '/'))
		return !proper;
	if (*p == '/' && !*(p + 1))
		return *s == '/';
	while (*p)
		if (*p++ != *s++)
			return 0;
	return *s == '/' || !proper && !*s;
}

/*
 * allocate and copy a tmp string
 *	a!=0	for lifetime of a
 *	f!=0	for lifetime of f
 */

char*
stash(register Value_t* v, const char* s, size_t z)
{
	if (!z)
	{
		if (!s)
			return 0;
		z = strlen(s);
	}
	z++;
	if (z > v->size)
	{
		v->size = roundof(z, 256);
		if (!(v->string = newof(v->string, char, v->size, 0)))
			nospace();
	}
	if (s)
	{
		memcpy(v->string, s, z - 1);
		v->string[z - 1] = 0;
	}
	return v->string;
}

/*
 * out of space panic
 */

void
nospace(void)
{
	error(ERROR_SYSTEM|3, "out of space");
}

/*
 * if current file cannot fit completely in current archive
 * then bump it to another volume
 */

void
complete(Archive_t* ap, register File_t* f, size_t header)
{
	off_t	n;

	n = header + f->st->st_size;
	if (ap->io->count + n > state.maxout)
	{
		if (n > state.maxout)
			error(1, "%s: too large to fit in one volume", f->name);
		else
		{
			state.complete = 0;
			putepilogue(ap);
			newio(ap, 0, 0);
			putprologue(ap, 0);
			state.complete = 1;
		}
	}
}

/*
 * verify that compress undo command exists
 * alternate undotoo checked if undo not found
 */

void
undoable(Archive_t* ap, Format_t* fp)
{
	register Compress_format_t*	cp = (Compress_format_t*)fp->data;
	char				buf[PATH_MAX];

	if (!pathpath(cp->undo[0], NiL, PATH_EXECUTE, buf, sizeof(buf)))
	{
		if (!cp->undotoo[0] || !pathpath(cp->undotoo[0], NiL, PATH_EXECUTE, buf, sizeof(buf)))
			error(3, "%s: %s: command required to read compressed archive", ap->name, cp->undo[0]);
		cp->undo[0] = cp->undotoo[0];
		cp->undotoo[0] = 0;
		cp->undo[1] = cp->undotoo[1];
	}
}
