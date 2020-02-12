/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2012 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * pax file support
 */

#include "pax.h"

#include <tm.h>

/*
 * "nocomment" is a hardwired "nocom"
 * should be an sfio discipline
 */

#include "nocomment.c"

#if __STDC__
#define chmod(a,b)	(error(-1,"%s#%d: chmod(%s,%05o)",__FILE__,__LINE__,a,b),chmod(a,b))
#endif

/*
 * return read file descriptor for filtered current input file
 */

int
apply(register Archive_t* ap, register File_t* f, Filter_t* fp)
{
	register int	n;
	char*		arg;
	int		rfd;
	int		wfd;

	if (state.filter.line <= 0)
		arg = f->path;
	else if (!*(arg = state.filter.command))
	{
		if ((rfd = open(f->st->st_size ? f->path : "/dev/null", O_RDONLY|O_BINARY)) < 0)
			error(ERROR_SYSTEM|2, "%s: cannot read", f->path);
		return rfd;
	}
	message((-4, "filter: %s %s", fp->command, f->path));
	if ((wfd = open(state.tmp.file, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, S_IRUSR)) < 0)
	{
		error(2, "%s: cannot create filter temporary %s", f->path, state.tmp.file);
		return -1;
	}
	if ((rfd = open(state.tmp.file, O_RDONLY|O_BINARY)) < 0)
	{
		error(2, "%s: cannot open filter temporary %s", f->path, state.tmp.file);
		close(wfd);
		if (remove(state.tmp.file))
			error(1, "%s: cannot remove filter temporary %s", f->path, state.tmp.file);
		return -1;
	}
	if (remove(state.tmp.file))
		error(1, "%s: cannot remove filter temporary %s", f->path, state.tmp.file);
	if (ap->format->checksum)
		f->checksum = 0;
	f->st->st_size = 0;
	if (streq(*fp->argv, "nocomment"))
	{
		int	errors = error_info.errors;
		off_t	count;
		Sfio_t*	ip;
		Sfio_t*	op;

		if ((ip = sfopen(NiL, f->path, "r")) && (op = sfnew(NiL, NiL, SF_UNBOUND, wfd, SF_WRITE)) && (count = nocomment(ip, op)) < 0)
			error(2, "%s: %s: filter error", f->path, *fp->argv);
		if (ip)
		{
			sfclose(ip);
			if (op)
				sfclose(op);
			else
				error(2, "%s: cannot redirect filter", f->path);
		}
		else
			error(2, "%s: cannot read", f->path);
		if (errors != error_info.errors)
		{
			close(rfd);
			close(wfd);
			return -1;
		}
		f->st->st_size = count;
	}
	else
	{
		Proc_t*		proc;

		*fp->patharg = arg;
		if (!(proc = procopen(*fp->argv, fp->argv, NiL, NiL, PROC_READ)))
		{
			error(2, "%s: cannot execute filter %s", f->path, *fp->argv);
			close(rfd);
			close(wfd);
			return -1;
		}
		holeinit(wfd);
		while ((n = read(proc->rfd, state.tmp.buffer, state.buffersize)) > 0)
		{
			if (holewrite(wfd, state.tmp.buffer, n) != n)
			{
				error(2, "%s: filter write error", f->path);
				break;
			}
			if (ap->format->checksum)
				f->checksum = (*ap->format->checksum)(&state, ap, f, state.tmp.buffer, n, f->checksum);
			f->st->st_size += n;
		}
		holedone(wfd);
		if (n < 0)
			error(ERROR_SYSTEM|2, "%s: %s filter read error", f->path, *fp->argv);
		if (n = procclose(proc))
			error(2, "%s: %s filter exit code %d", f->path, *fp->argv, n);
	}
	close(wfd);
	message((-1, "%s: filter file size = %ld", f->path, f->st->st_size));
	return rfd;
}

/*
 * return read file descriptor for current input file
 */

int
openin(register Archive_t* ap, register File_t* f)
{
	register int	n;
	Filter_t*	fp;
	int		rfd;

	if (f->type != X_IFREG)
		rfd = -1;
	else if (fp = filter(ap, f))
		rfd = apply(ap, f, fp);
	else if ((rfd = open(f->st->st_size ? f->path : "/dev/null", O_RDONLY|O_BINARY)) < 0)
		error(ERROR_SYSTEM|2, "%s: cannot read", f->path);
	else if (ap->format->checksum)
	{
		f->checksum = 0;
		if (lseek(rfd, (off_t)0, SEEK_SET) != 0)
			error(ERROR_SYSTEM|1, "%s: %s checksum seek error", f->path, ap->format->name);
		else
		{
			while ((n = read(rfd, state.tmp.buffer, state.buffersize)) > 0)
				f->checksum = (*ap->format->checksum)(&state, ap, f, state.tmp.buffer, n, f->checksum);
			if (n < 0)
				error(ERROR_SYSTEM|2, "%s: %s checksum read error", f->path, ap->format->name);
			if (lseek(rfd, (off_t)0, SEEK_SET) != 0)
				error(ERROR_SYSTEM|1, "%s: %s checksum seek error", f->path, ap->format->name);
		}
	}
	if (rfd < 0)
		f->st->st_size = 0;
	return rfd;
}

/*
 * create directory and all path name components leading to directory
 */

static int
missdir(register Archive_t* ap, register File_t* f)
{
	register char*	s;
	register char*	t;
	long		pp;
	struct stat*	st;
	struct stat*	sp;
	struct stat	st0;
	struct stat	st1;

	s = f->name;
	pathcanon(s, 0, 0);
	if (t = strchr(*s == '/' ? s + 1 : s, '/'))
	{
		if (!state.mkdir)
		{
			if (!state.warnmkdir)
			{
				state.warnmkdir = 1;
				error(1, "omit the --nomkdir option to create intermediate directories");
			}
			return -1;
		}
		st = 0;
		sp = &st0;
		do
		{
			*t = 0;
			if (stat(s, sp))
			{
				*t = '/';
				break;
			}
			*t = '/';
			st = sp;
			sp = (sp == &st0) ? &st1 : &st0;
		} while (t = strchr(t + 1, '/'));
		if (t)
		{
			if (!st && stat(".", st = &st0))
				error(ERROR_SYSTEM|3, "%s: cannot stat .", s);
			pp = f->perm;
			f->perm = st->st_mode & state.modemask;
			sp = f->st;
			f->st = st;
			do
			{
				*t = 0;
				if (mkdir(s, f->perm))
				{
					error(ERROR_SYSTEM|2, "%s: cannot create directory", s);
					*t = '/';
					f->perm = pp;
					f->st = sp;
					return -1;
				}
				setfile(ap, f);
				*t = '/';
			} while (t = strchr(t + 1, '/'));
			f->perm = pp;
			f->st = sp;
		}
	}
	return 0;
}

/*
 * open file for writing, set all necessary info
 */

int
openout(register Archive_t* ap, register File_t* f)
{
	register int	fd;
	int		exists;
	int		perm;
	int		c;
	Tv_t		t1;
	Tv_t		t2;
	size_t		updated;
	struct stat	st;

	pathcanon(f->name, 0, 0);

	/*
	 * if not found and state.update then check down the view
	 *
	 * NOTE: VPATH in app code is ugly but the benefits of the
	 *	 combination with state.update win over beauty
	 */

	if (f->ro)
	{
		f->name = "PAX-INTERNAL-ERROR";
		f->skip = 1;
		exists = 0;
	}
	else if (exists = !(*state.statf)(f->name, &st))
	{
		if (!state.clobber && !S_ISDIR(st.st_mode))
		{
			error(1, "%s: already exists -- not overwritten", f->name);
			return -1;
		}
		f->chmod = f->perm != (st.st_mode & (S_IRWXU|S_IRWXG|S_IRWXO)) &&
			(state.chmod || state.update || S_ISDIR(st.st_mode));
		st.st_mode = modex(st.st_mode);
	}
	else
	{
		typedef struct View
		{
			struct View*	next;
			char*		root;
			dev_t		dev;
			ino_t		ino;
		} View_t;

		View_t*			vp;
		View_t*			tp;
		char*			s;
		char*			e;

		static View_t*		view;
		static char*		offset;

		if (state.update && !offset)
		{
			if (s = getenv("VPATH"))
			{
				if (!(s = strdup(s)))
					nospace();
				do
				{
					if (e = strchr(s, ':'))
						*e++ = 0;
					if (!(vp = newof(0, View_t, 1, 0)))
						nospace();
					vp->root = s;
					if (stat(s, &st))
					{
						vp->dev = 0;
						vp->ino = 0;
					}
					else
					{
						vp->dev = st.st_dev;
						vp->ino = st.st_ino;
					}
					if (view)
						tp = tp->next = vp;
					else
						view = tp = vp;
				} while (s = e);
				s = state.pwd;
				e = 0;
				for (;;)
				{
					if (stat(s, &st))
						error(ERROR_SYSTEM|3, "%s: cannot stat pwd component", s);
					for (vp = view; vp; vp = vp->next)
						if (vp->ino == st.st_ino && vp->dev == st.st_dev)
						{
							offset = e ? e + 1 : ".";
							tp = view;
							view = vp->next;
							while (tp && tp != view)
							{
								vp = tp;
								tp = tp->next;
								free(vp);
							}
							if (e)
								*e = '/';
							goto found;
						}
					if (e)
						*e = '/';
					else
						e = s + strlen(s);
					while (e > s && *--e != '/');
					if (e <= s)
						break;
					*e = 0;
				}
			}
		found:
			if (!offset)
				offset = ".";
		}
		st.st_mode = 0;
		st.st_mtime = 0;
		if (*f->name != '/')
			for (vp = view; vp; vp = vp->next)
			{
				sfsprintf(state.tmp.buffer, state.tmp.buffersize - 1, "%s/%s/%s", vp->root, offset, f->name);
				if (!stat(state.tmp.buffer, &st))
					break;
			}
		f->chmod = state.chmod || state.update;
	}
	if (f->delta.op == DELTA_delete)
	{
		if (exists)
			switch (X_ITYPE(st.st_mode))
			{
			case X_IFDIR:
				if (!f->ro)
				{
					if (rmdir(f->name))
						error(ERROR_SYSTEM|2, "%s: cannot remove directory", f->name);
					else
						listentry(f);
				}
				break;
			default:
				if (remove(f->name))
					error(ERROR_SYSTEM|2, "%s: cannot remove file", f->name);
				else
					listentry(f);
				break;
			}
		return -1;
	}
	if (state.operation == (IN|OUT))
	{
		if (exists && f->st->st_ino == st.st_ino && f->st->st_dev == st.st_dev)
		{
			error(2, "attempt to pass %s to self", f->name);
			return -1;
		}
		if (state.linkf && f->type != X_IFDIR && (state.linkf == pathsetlink || f->st->st_dev == state.dev))
		{
			if (exists)
				remove(f->name);
			if ((*state.linkf)(f->path, f->name))
			{
				if (!exists && missdir(ap, f))
				{
					error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
					return -1;
				}
				if (exists || (*state.linkf)(f->path, f->name))
				{
					error(ERROR_SYSTEM|2, "%s: cannot link to %s", f->path, f->name);
					return -1;
				}
			}
			setfile(ap, f);
			return -2;
		}
	}
	switch (f->type)
	{
	case X_IFDIR:
		if (!(ap->format->flags & KEEPSIZE))
			f->st->st_size = 0;
		if (f->ro)
			return -1;
		if (exists && X_ITYPE(st.st_mode) != X_IFDIR)
		{
			if (remove(f->name))
			{
				error(ERROR_SYSTEM|2, "cannot remove current %s", f->name);
				return -1;
			}
			exists = 0;
		}
		if (!exists && mkdir(f->name, f->perm) && (missdir(ap, f) || mkdir(f->name, f->perm)))
		{
			error(ERROR_SYSTEM|2, "%s: cannot create directory", f->name);
			return -1;
		}
		updated = ap->updated;
		setfile(ap, f);
		if (!exists || f->chmod || state.update && ((c = tvcmp(tvmtime(&t1, f->st), tvmtime(&t2, &st))) > 0 || state.update == OPT_different && c))
		{
			listentry(f);
			fd = -1;
		}
		else
		{
			ap->updated = updated;
			if (state.update)
				fd = -1;
			else
				fd = -2;
		}
		return fd;
	case X_IFLNK:
		if (exists && prune(ap, f, &st))
			return -1;
		if (!*f->linkpath)
			return -2;
		if (streq(f->name, f->linkpath))
		{
			error(1, "%s: symbolic link loops to self", f->name);
			return -1;
		}
		if (exists && remove(f->name))
		{
			error(ERROR_SYSTEM|2, "cannot remove current %s", f->name);
			return -1;
		}
		if (pathsetlink(f->linkpath, f->name))
		{
			if (!exists && missdir(ap, f))
			{
				error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
				return -1;
			}
			if (exists || pathsetlink(f->linkpath, f->name))
			{
				error(ERROR_SYSTEM|2, "%s: cannot symlink to %s", f->name, f->linkpath);
				return -1;
			}
		}
		setfile(ap, f);
		listentry(f);
		return -1;
	case X_IFSOCK:
		IDEVICE(f->st, 0);
		/*FALLTHROUGH*/
	case X_IFBLK:
	case X_IFCHR:
	case X_IFIFO:
		if (exists && (prune(ap, f, &st) || state.update && f->st->st_dev != st.st_dev))
			return -1;
		if (!(ap->format->flags & KEEPSIZE))
			f->st->st_size = 0;
		break;
	case X_IFREG:
		if (exists && prune(ap, f, &st))
			return -1;
		break;
	}
	if (!addlink(ap, f))
		return -1;
	switch (f->type)
	{
	case X_IFIFO:
		if (exists && remove(f->name))
		{
			error(ERROR_SYSTEM|2, "cannot remove current %s", f->name);
			return -1;
		}
		if (mkfifo(f->name, f->st->st_mode & S_IPERM))
		{
			if (errno == EPERM)
			{
			nofifo:
				error(ERROR_SYSTEM|2, "%s: cannot create fifo file", f->name);
				return -1;
			}
			if (!exists && missdir(ap, f))
			{
				error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
				return -1;
			}
			if (exists || mkfifo(f->name, f->st->st_mode & S_IPERM))
				goto nofifo;
		}
		setfile(ap, f);
		return -2;
	case X_IFSOCK:
		IDEVICE(f->st, 0);
		/*FALLTHROUGH*/
	case X_IFBLK:
	case X_IFCHR:
		if (exists && remove(f->name))
		{
			error(ERROR_SYSTEM|2, "cannot remove current %s", f->name);
			return -1;
		}
		if (mknod(f->name, f->st->st_mode, idevice(f->st)))
		{
			if (errno == EPERM)
			{
			nospecial:
				error(ERROR_SYSTEM|2, "%s: cannot create %s special file", f->name, (f->type == X_IFBLK) ? "block" : "character");
				return -1;
			}
			if (!exists && missdir(ap, f))
			{
				error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
				return -1;
			}
			if (exists || mknod(f->name, f->st->st_mode, idevice(f->st)))
				goto nospecial;
		}
		setfile(ap, f);
		return -2;
	default:
		error(1, "%s: unknown file type 0%03o -- creating regular file", f->name, f->type >> 12);
		/*FALLTHROUGH*/
	case X_IFREG:
		if (f->ro)
			return dup(1);
		if (state.intermediate)
		{
			char*	d;
			char*	e;
			int	n;
			int	ifd;

			/*
			 * copy to intermediate output file and rename
			 * to real file only on success - a handy
			 * backup option
			 *
			 * thanks to the amazing dr. ek
			 */

			if (missdir(ap, f))
			{
				error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
				return -1;
			}
			d = (e = strrchr(f->name, '/')) ? f->name : ".";
			for (n = 0;; n++)
			{
				if (e)
					*e = 0;
				f->intermediate = pathtemp(ap->path.temp, sizeof(ap->path.temp), d, error_info.id, &ifd);
				if (e)
					*e = '/';
				message((-4, "%s: intermediate %s", f->name, f->intermediate));
				if (f->intermediate)
				{
					ap->errors = error_info.errors;
					return ifd;
				}
				if (n)
				{
					error(ERROR_SYSTEM|2, "%s: cannot create intermediate name", f->name);
					return -1;
				}
			}
		}

		/*
		 * ok, the exists bits are only used right here
		 * you do the defines if its that important
		 *
		 * <chmod u+w><remove><missdir>
		 *	4	don't attempt
		 *	2	attempted and succeeded
		 *	1	attempted and failed
		 */

		if (!exists)
			exists |= 0440;
		else if (!state.linkf)
			exists |= remove(f->name) ? 0010 : 0420;
		else if (st.st_mode & S_IWUSR)
			exists |= 0400;
		if ((perm = f->perm) & (S_ISUID|S_ISGID|S_ISVTX))
			perm &= ~(S_ISUID|S_ISGID|S_ISVTX);
		while ((fd = open(f->name, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, perm)) < 0)
		{
		again:
			if (!(exists & 0007))
			{
				if (missdir(ap, f))
				{
					error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
					return -1;
				}
				exists |= 0002;
			}
			else if (!(exists & 0700))
			{
				if (chmod(f->name, perm | S_IWUSR))
				{
					exists |= 0100;
					goto again;
				}
				exists |= 0200;
			}
			else if (!(exists & 0070))
			{
				if (remove(f->name))
				{
					exists |= 0010;
					goto again;
				}
				exists ^= 0620;
			}
			else
			{
				error(ERROR_SYSTEM|2, "%s: cannot create%s%s", f->name, (exists & 0100) ? ERROR_translate(0, 0, 0, ", cannot enable user write") : "", (exists & 0010) ? ERROR_translate(0, 0, 0, ", cannot remove") : "");
				return -1;
			}
		}
		if (perm != f->perm)
			f->chmod = 1;
		else if ((exists & 0200) && chmod(f->name, f->perm))
			error(ERROR_SYSTEM|1, "%s: cannot restore original mode %s", f->name, fmtperm(st.st_mode & S_IPERM));
		return fd;
	}
}

/*
 * close an openin() fd, doing atime reset if necessary
 */

int
closein(register Archive_t* ap, register File_t* f, int fd)
{
	int		r;

	r = 0;
	if (close(fd))
		r = -1;
	if (state.resetacctime && f->type != X_IFLNK && !f->skip)
	{
		Tv_t	av;
		Tv_t	mv;

		tvgetatime(&av, f->st);
		tvgetmtime(&mv, f->st);
		settime(f->path, &av, &mv, NiL);
	}
	return r;
}

/*
 * close an openout() fd, doing the intermediate rename if needed
 */

int
closeout(register Archive_t* ap, register File_t* f, int fd)
{
	register char*	s;
	int		r;

	r = 0;
	if (state.sync && fsync(fd))
		r = -1;
	if (close(fd))
		r = -1;
	if (s = f->intermediate)
	{
		f->intermediate = 0;
		if (ap->errors != error_info.errors)
		{
			if (remove(s))
				error(ERROR_SYSTEM|2, "%s: cannot remove intermediate file %s", f->name, s);
			return -1;
		}
		if (rename(s, f->name) && (remove(f->name) || rename(s, f->name)))
		{
			error(ERROR_SYSTEM|2, "%s: cannot rename from intermediate file %s", f->name, s);
			return -1;
		}
		if (chmod(f->name, f->perm))
		{
			error(ERROR_SYSTEM|1, "%s: cannot change mode to %s", f->name, fmtperm(f->perm));
			return -1;
		}
	}
	return r;
}

/*
 * get file info for output
 */

int
getfile(register Archive_t* ap, register File_t* f, register Ftw_t* ftw)
{
	register char*		name;
	register int		n;
	char*			e;

	name = ftw->path;
	message((-4, "getfile(%s)", name));
	switch (ftw->info)
	{
	case FTW_NS:
		error(2, "%s: not found", name);
		return 0;
	case FTW_DNR:
		if (state.files)
			error(2, "%s: cannot read directory", name);
		break;
	case FTW_D:
	case FTW_DNX:
	case FTW_DP:
		if (!state.descend)
			ftw->status = FTW_SKIP;
		else if (ftw->info == FTW_DNX)
		{
			error(2, "%s: cannot search directory", name);
			ftw->status = FTW_SKIP;
		}
		else if (!state.files)
		{
			/*
			 * stdin files most likely come from tw/find with
			 * directory descendents already included; in posix
			 * omitting -d would result in duplicate output copies
			 * so we avoid the problem by peeking ahead and
			 * pruning all paths with this dir prefix
			 */

			n = ftw->pathlen;
			name = stash(&ap->path.peek, name, n);
			name[n] = '/';
			if (!state.peekfile || !strncmp(state.peekfile, name, n))
				while (state.peekfile = sfgetr(sfstdin, '\n', 1))
					if (strncmp(state.peekfile, name, n))
					{
						state.peeklen = sfvalue(sfstdin) - 1;
						break;
					}
			name[n] = 0;
		}
		break;
	}
	if (ap->delta)
		ap->delta->hdr = ap->delta->hdrbuf;
	name = stash(&ap->path.name, name, ftw->pathlen);
	pathcanon(name, 0, 0);
	f->path = stash(&ap->path.path, name, ftw->pathlen);
	f->name = map(ap, name);
	if (state.files && state.operation == (IN|OUT) && dirprefix(state.destination, name, 0))
		return 0;
	f->namesize = strlen(f->name) + 1;
	ap->st = ftw->statb;
	f->st = &ap->st;
	f->perm = f->st->st_mode & S_IPERM;
	f->st->st_mode = modex(f->st->st_mode);
	f->uidname = 0;
	f->gidname = 0;
	f->link = 0;
	if ((f->type = X_ITYPE(f->st->st_mode)) == X_IFLNK)
	{
		f->linkpathsize = f->st->st_size + 1;
		f->linkpath = stash(&ap->stash.link, NiL, f->linkpathsize);
		if (pathgetlink(f->path, f->linkpath, f->linkpathsize) != f->st->st_size)
		{
			error(2, "%s: cannot read symbolic link", f->path);
			ftw->status = FTW_SKIP;
			return 0;
		}
		f->linktype = SOFTLINK;
		pathcanon(f->linkpath, 0, 0);
		if (!(state.ftwflags & FTW_PHYSICAL))
			f->linkpath = map(ap, f->linkpath);
		if (streq(f->path, f->linkpath))
		{
			error(2, "%s: symbolic link loops to self", f->path);
			ftw->status = FTW_SKIP;
			return 0;
		}
	}
	else
	{
		f->linktype = NOLINK;
		f->linkpath = 0;
		f->linkpathsize = 0;
	}
	f->ro = ropath(f->name);
	if (!validout(ap, f))
		return 0;
	if (!(state.operation & IN) && f->type != X_IFDIR)
	{
		if (!addlink(ap, f) && !state.header.linkdata)
			f->st->st_size = 0;
		message((-4, "getfile(%s): dev'=%d ino'=%d", f->name, f->st->st_dev, f->st->st_ino));
	}
	ap->entries++;
	f->delta.op = 0;
	f->longname = 0;
	f->longlink = 0;
	f->skip = 0;
	if (state.mode)
	{
		f->st->st_mode = strperm(state.mode, &e, f->st->st_mode);
		if (*e)
			error(2, "%s: invalid mode expression", state.mode);
	}
	if (state.mtime)
	{
		f->st->st_mtime = tmdate(state.mtime, &e, NiL);
		if (*e)
			error(2, "%s: invalid mtime", state.mtime);
	}
	message((-2, "getfile(): path=%s name=%s mode=%s size=%I*d mtime=%s", name, f->name, fmtmode(f->st->st_mode, 1), sizeof(f->st->st_size), f->st->st_size, fmttime("%K", f->st->st_mtime)));
	return 1;
}

/*
 * check that f is valid for archive output
 */

int
validout(register Archive_t* ap, register File_t* f)
{
	if (f->ro)
		return 0;
	switch (f->type)
	{
	case X_IFBLK:
	case X_IFCHR:
		f->st->st_size = 0;
		break;
	case X_IFREG:
		IDEVICE(f->st, 0);
		break;
	case X_IFDIR:
	case X_IFIFO:
	case X_IFLNK:
		f->st->st_size = 0;
		IDEVICE(f->st, 0);
		break;
	}
	return ap->format->validate ? ((*ap->format->validate)(&state, ap, f) > 0) : 1;
}

/*
 * add file which may be a link
 * 0 returned if <dev,ino> already added
 */

int
addlink(register Archive_t* ap, register File_t* f)
{
	register Link_t*	p;
	register char*		s;
	int			n;
	Fileid_t		id;
	unsigned short		us;

	id.dev = f->st->st_dev;
	id.ino = f->st->st_ino;
	if (!ap->delta)
		switch (state.operation)
		{
		case IN:
			us = id.dev;
			if (us > state.devcnt)
			{
				state.devcnt = us;
				state.inocnt = id.ino;
			}
			else if (us == state.devcnt)
			{
				us = id.ino;
				if (us > state.inocnt)
					state.inocnt = us;
			}
			break;
		case IN|OUT:
			if (!state.pass)
				break;
			/*FALLTHROUGH*/
		case OUT:
			if (!++state.inocnt)
			{
				if (!++state.devcnt)
					goto toomany;
				state.inocnt = 1;
			}
			f->st->st_dev = state.devcnt;
			f->st->st_ino = state.inocnt;
			break;
		}
	if (f->type == X_IFDIR)
		return 0;
	if (ap->format->flags & NOHARDLINKS)
	{
		if (state.operation == IN || f->st->st_nlink <= 1)
			return 1;
	}
	else if ((ap->format->flags & LINKTYPE) && state.operation == IN)
	{
		if (f->linktype == NOLINK)
			return 1;
		f->linkpath = map(ap, f->linkpath);
		goto linked;
	}
	else if (f->st->st_nlink <= 1)
		return 1;
	if (p = (Link_t*)hashget(state.linktab, (char*)&id))
	{
		if (ap->format->flags & NOHARDLINKS)
		{
			error(1, "%s: hard link information lost in %s format", f->name, ap->format->name);
			return 1;
		}
		f->st->st_dev = p->id.dev;
		f->st->st_ino = p->id.ino;
		f->link = p;
		f->linktype = HARDLINK;
		f->linkpath = p->name;
		if (state.pass && (state.operation & OUT) || !state.pass && state.operation == OUT)
			return 0;
	linked:
		message((-1, "addlink(%s,%s)", f->name, f->linkpath));
		if (ap->format->event && (ap->format->events & PAX_EVENT_BUG_19951031))
			(*ap->format->event)(&state, ap, f, NiL, PAX_EVENT_BUG_19951031);
		if (streq(f->name, f->linkpath))
		{
			error(2, "%s: hard link loops to self", f->name);
			return 0;
		}
		if (!state.list)
		{
			s = f->linkpath;
			if (access(s, F_OK))
			{
				f->skip = 1;
				error(2, "%s must exist for hard link %s", s, f->name);
				return 0;
			}
			remove(f->name);
			if (state.operation == IN && *s != '/')
			{
				strcpy(state.pwd + state.pwdlen, s);
				s = state.pwd;
			}
			if (link(s, f->name))
			{
				if (missdir(ap, f))
				{
					error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
					return 0;
				}
				if (link(s, f->name))
				{
					error(ERROR_SYSTEM|2, "%s: cannot link to %s", f->linkpath, f->name);
					return -1;
				}
			}
			listentry(f);
		}
		return 0;
	}
	n = strlen(f->name) + 1;
	if (!(p = newof(0, Link_t, 1, n)))
		goto toomany;
	f->link = p;
	strcpy(p->name = (char*)p + sizeof(*p), f->name);
	p->namesize = n;
	p->id.dev = f->st->st_dev;
	p->id.ino = f->st->st_ino;
	hashput(state.linktab, NiL, p);
	return -1;
 toomany:
	if (!state.warnlinknum)
	{
		state.warnlinknum = 1;
		error(1, "too many hard links -- some links may become copies");
	}
	return -1;
}

/*
 * get file uid and gid names given numbers
 */

void
getidnames(register File_t* f)
{
	if (!f->uidname)
		f->uidname = fmtuid(f->st->st_uid);
	if (!f->gidname)
		f->gidname = fmtgid(f->st->st_gid);
}

/*
 * set file uid and gid numbers given names
 */

void
setidnames(register File_t* f)
{
	register int	id;

	if (f->uidname)
	{
		if ((id = struid(f->uidname)) < 0)
		{
			if (id == -1 && state.owner)
				error(1, "%s: invalid user name", f->uidname);
			f->uidname = 0;
			id = state.uid;
		}
		f->st->st_uid = id;
	}
	if (f->gidname)
	{
		if ((id = strgid(f->gidname)) < 0)
		{
			if (id == -1 && state.owner)
				error(1, "%s: invalid group name", f->gidname);
			f->gidname = 0;
			id = state.gid;
		}
		f->st->st_gid = id;
	}
}

/*
 * allocate and initialize new archive pointer
 */

Archive_t*
initarchive(const char* name, int mode)
{
	Archive_t*	ap;

	if (!(ap = newof(0, Archive_t, 1, 0)))
		nospace();
	initfile(ap, &ap->file, &ap->st, NiL, 0);
	ap->name = (char*)name;
	ap->expected = ap->format = 0;
	ap->section = 0;
	ap->sum = -1;
	ap->mio.mode = ap->tio.mode = mode;
	ap->io = &ap->mio;
	return ap;
}

/*
 * return pointer to archive for op
 */

Archive_t*
getarchive(int op)
{
	Archive_t**	app;

	app = (op & OUT) ? &state.out : &state.in;
	if (!*app)
		*app = initarchive(NiL, (op & OUT) ? (state.append ? (O_WRONLY|O_CREAT) : (O_WRONLY|O_CREAT|O_TRUNC)) : O_RDONLY);
	return *app;
}

/*
 * initialize file info with name and mode
 */

void
initfile(register Archive_t* ap, register File_t* f, struct stat* st, register char* name, int mode)
{
	memzero(f, sizeof(*f));
	f->st = st;
	memzero(f->st, sizeof(*f->st));
	if (name)
	{
		f->id = f->name = f->path = name;
		f->namesize = strlen(name) + 1;
	}
	f->st->st_mode = modex(mode);
	f->st->st_nlink = 1;		/* system V needs this!!! */
}

/*
 * set copied file info
 */

void
setfile(register Archive_t* ap, register File_t* f)
{
	register Post_t*	p;
	int			updated;
	Post_t			post;

	if (f->skip || f->extended)
		return;
	switch (f->type)
	{
	case X_IFLNK:
		updated = 0;
#if _lib_lchown
		if (state.owner)
		{
			if (state.flags & SETIDS)
			{
				post.uid = state.setuid;
				post.gid = state.setgid;
			}
			else
			{
				post.uid = f->st->st_uid;
				post.gid = f->st->st_gid;
			}
			if (lchown(f->name, post.uid, post.gid) < 0)
				error(1, "%s: cannot chown to (%d,%d)", f->name, post.uid, post.gid);
		}
#endif
#if _lib_lchmod
		if (f->chmod)
		{
			int		m;
			struct stat	st;

			if (lstat(f->name, &st))
				error(1, "%s: not found", f->name);
			else if ((f->perm ^ st.st_mode) & state.modemask & (S_ISUID|S_ISGID|S_ISVTX|S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH))
			{
				if (lchmod(f->name, f->perm & state.modemask))
					error(1, "%s: cannot chmod to %s", f->name, fmtmode(f->perm & state.modemask, 0) + 1);
				else if (m = f->perm & (S_ISUID|S_ISGID|S_ISVTX))
				{
					if (lstat(f->name, &st))
						error(1, "%s: not found", f->name);
					else if (m ^= (st.st_mode & (S_ISUID|S_ISGID|S_ISVTX)))
						error(1, "%s: mode %s not set", f->name, fmtmode(m, 0) + 1);
				}
			}
		}
#endif
		ap->updated += updated;
		return;
	case X_IFDIR:
		if (f->chmod || state.acctime || state.modtime || state.owner || (f->perm & S_IRWXU) != S_IRWXU)
		{
			if (!(p = newof(0, Post_t, 1, 0)))
				error(3, "not enough space for file restoration info");
			tvgetatime(&p->atime, f->st);
			tvgetmtime(&p->mtime, f->st);
			p->uid = f->st->st_uid;
			p->gid = f->st->st_gid;
			p->mode = f->perm;
			if ((f->perm & S_IRWXU) != S_IRWXU)
			{
				p->chmod = 1;
				if (chmod(f->name, f->perm|S_IRWXU))
					error(1, "%s: cannot chmod to %s", f->name, fmtmode(f->st->st_mode|X_IRWXU, 1) + 1);
			}
			else
				p->chmod = f->chmod;
			hashput(state.restore, f->name, p);
			ap->updated++;
			return;
		}
		break;
	}
	ap->updated++;
	p = &post;
	if (state.acctime)
		tvgetatime(&p->atime, f->st);
	tvgetmtime(&p->mtime, f->st);
	p->uid = f->st->st_uid;
	p->gid = f->st->st_gid;
	p->mode = f->perm;
	p->chmod = f->chmod;
	restore(f->name, (char*)p, NiL);
}

/*
 * set access and modification times of file
 */

void
settime(const char* name, Tv_t* ap, Tv_t* mp, Tv_t* cp)
{
	if (*name && tvtouch(name, ap, mp, cp, 0) && errno != ENOENT && errno != ENOTDIR)
		error(1, "%s: cannot set times", name);
}

/*
 * restore file status after processing
 */

int
restore(register const char* name, char* ap, void* handle)
{
	register Post_t*	p = (Post_t*)ap;
	int			m;
	struct stat		st;

	NoP(handle);
	if (!*name)
		return 0;
	if (state.owner)
	{
		if (state.flags & SETIDS)
		{
			p->uid = state.setuid;
			p->gid = state.setgid;
		}
		if (chown(name, p->uid, p->gid) < 0)
			error(1, "%s: cannot chown to (%d,%d)", name, p->uid, p->gid);
	}
	if (p->chmod)
	{
		if (chmod(name, p->mode & state.modemask))
			error(1, "%s: cannot chmod to %s", name, fmtmode(p->mode & state.modemask, 0) + 1);
		else if (m = p->mode & (S_ISUID|S_ISGID|S_ISVTX))
		{
			if (stat(name, &st))
				error(1, "%s: not found", name);
			else if (m ^= (st.st_mode & (S_ISUID|S_ISGID|S_ISVTX)))
				error(1, "%s: mode %s not set", name, fmtmode(m, 0) + 1);
		}
	}
	if (state.modtime)
		settime(name, state.acctime ? &p->atime : 0, &p->mtime, NiL);
	return 0;
}

/*
 * return 1 if f output can be pruned
 */

int
prune(register Archive_t* ap, register File_t* f, register struct stat* st)
{
	Tv_t		t1;
	Tv_t		t2;
	struct stat	so;
	int		c;

	if (state.operation != (IN|OUT) && state.update == OPT_update && !streq(f->name, f->path))
	{
		if ((*state.statf)(f->path, &so))
			return 0;
		st = &so;
	}
	if (st->st_mode == f->st->st_mode)
	{
		if (ap->delta && !tvcmp(tvmtime(&t1, f->st), tvmtime(&t2, st)))
			return 1;
		if (state.update && (!(c = tvcmp(tvmtime(&t1, f->st), tvmtime(&t2, st))) || state.update != OPT_different && c < 0))
		{
			if (state.exact)
				state.pattern->matched = 0;
			return 1;
		}
	}
	return 0;
}

/*
 * write siz bytes of buf to fd checking for HOLE_MIN hole chunks
 * we assume siz is rounded nicely until the end
 */

ssize_t
holewrite(int fd, void* buf, size_t siz)
{
	register char*	t = (char*)buf;
	register char*	e = t + siz;
	register char*	b = 0;
	register char*	s;
	ssize_t		i;
	ssize_t		n = 0;

	static char	hole[HOLE_MIN];

#if DEBUG
	if (state.test & 0000100)
		b = t;
	else
#endif
	while (t < e)
	{
		s = t;
		if ((t += HOLE_MIN) > e)
			t = e;
		if (!*s && !*(t - 1) && !memcmp(s, hole, t - s))
		{
			if (b)
			{
				if (state.hole)
				{
					if (lseek(fd, state.hole, SEEK_CUR) < state.hole)
						return -1;
					state.hole = 0;
				}
				if ((i = write(fd, b, s - b)) != (s - b))
					return i;
				n += i;
				b = 0;
			}
			state.hole += t - s;
			n += t - s;
		}
		else if (!b)
			b = s;
	}
	if (b)
	{
		if (state.hole)
		{
			if (lseek(fd, state.hole, SEEK_CUR) < state.hole)
				return -1;
			state.hole = 0;
		}
		if ((i = write(fd, b, e - b)) != (e - b))
			return i;
		n += i;
	}
	return n;
}

/*
 * make a seekable copy of ap->io input
 */

void
seekable(Archive_t* ap)
{
	off_t		m;
	off_t		z;
	char*		s;
	int		rfd;
	int		wfd;

	if ((wfd = open(state.tmp.file, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, S_IRUSR)) < 0)
		error(ERROR_SYSTEM|3, "%s: cannot create seekable temporary %s", ap->name, state.tmp.file);
	if ((rfd = open(state.tmp.file, O_RDONLY|O_BINARY)) < 0)
		error(ERROR_SYSTEM|3, "%s: cannot open seekable temporary %s", ap->name, state.tmp.file);
	if (remove(state.tmp.file))
		error(ERROR_SYSTEM|1, "%s: cannot remove seekable temporary %s", ap->name, state.tmp.file);
	ap->io->seekable = 1;
	z = 0;
	s = ap->io->buffer + ap->io->unread;
	m = ap->io->last - s;
	do
	{
		if (write(wfd, s, m) != m)
			error(ERROR_SYSTEM|3, "%s: seekable temporary %s write error", ap->name, state.tmp.file);
		z += m;
	} while ((m = read(ap->io->fd, s, state.buffersize)) > 0);
	close(wfd);
	close(ap->io->fd);
	ap->io->size = z;
	z = ap->io->count;
	ap->io->next = ap->io->last = s;
	ap->io->offset = ap->io->count = 0;
	if (ap->io->fd || (ap->io->fd = dup(rfd)) < 0)
		ap->io->fd = rfd;
	else
		close(rfd);
	bread(ap, NiL, z, z, 0);
}
