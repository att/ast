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
 * pax file copy support
 */

#include "pax.h"

/*
 * copy files in from archive
 */

void
copyin(register Archive_t* ap)
{
	register File_t*	f = &ap->file;

	deltabase(ap);
	while (getprologue(ap))
	{
		while (getheader(ap, f))
		{
			if (selectfile(ap, f))
				filein(ap, f);
			else
				fileskip(ap, f);
			if (ap->info)
				ap->info->checksum = ap->memsum;
			gettrailer(ap, f);
		}
		if (!getepilogue(ap))
			break;
	}
	deltaverify(ap);
}

/*
 * copy a single file out to the archive
 * called by ftwalk()
 */

int
copyout(register Ftw_t* ftw)
{
	register Archive_t*	ap = state.out;
	register File_t*	f = &ap->file;

	if (getfile(ap, f, ftw))
	{
		if (selectfile(ap, f))
		{
			f->fd = openin(ap, f);
			deltaout(NiL, ap, f);
		}
		else
			ftw->status = FTW_SKIP;
	}
	return 0;
}

/*
 * low level for copyout()
 * if rfd<0 && st_size>0 then input from bread()
 */

void
fileout(register Archive_t* ap, register File_t* f)
{
	register size_t		m;
	register ssize_t	n;
	register off_t		c;
	int			err;
	Buffer_t*		bp;

	if (f->delta.op == DELTA_verify)
		ap->selected--;
	else if (putheader(ap, f))
	{
		if (!ap->format->putdata || !(*ap->format->putdata)(&state, ap, f, f->fd))
		{
			err = 0;
			c = f->st->st_size;
			while (c > 0)
			{
				n = m = c > state.buffersize ? state.buffersize : c;
				if (!err)
				{
					if (f->fd >= 0)
					{
						if ((n = read(f->fd, ap->io->next, m)) < 0 && errno == EIO)
						{
							static char*	buf;

							if (!buf)
							{
								n = 1024 * 8;
								error(1, "EIO read error -- falling back to aligned reads");
								if (!(buf = malloc(state.buffersize + n)))
									nospace();
								buf += n - (((ssize_t)buf) & (n - 1));
							}
							if ((n = read(f->fd, buf, m)) > 0)
								memcpy(ap->io->next, buf, n);
						}
					}
					else if (bp = getbuffer(f->fd))
					{
						memcpy(ap->io->next, bp->next, m);
						if (f->extended && ap->convert[SECTION_CONTROL].f2a)
							ccmapstr(ap->convert[SECTION_CONTROL].f2a, ap->io->next, m);
						bp->next += m;
					}
					else if (bread(f->ap, ap->io->next, (off_t)0, (off_t)n, 1) <= 0)
						n = -1;
				}
				if (n <= 0)
				{
					if (n)
						error(ERROR_SYSTEM|2, "%s: read error", f->path);
					else
						error(2, "%s: file size changed", f->path);
					memzero(ap->io->next, state.buffersize);
					err = 1;
				}
				else
				{
					c -= n;
					bput(ap, n);
				}
			}
		}
		puttrailer(ap, f);
	}
	if (f->fd >= 0)
		closein(ap, f, f->fd);
}

/*
 * low level for copyin()
 */

void
filein(register Archive_t* ap, register File_t* f)
{
	register off_t	c;
	register int	n;
	register char*	s;
	int		dfd;
	int		wfd;
	long		checksum;
	Filter_t*	fp;
	Proc_t*		pp;
	Tv_t		t1;
	Tv_t		t2;
	struct stat	st;

	if (f->skip)
		goto skip;
	else if (state.list)
	{
		if (fp = filter(ap, f))
		{
			for (n = 0; s = fp->argv[n]; n++)
			{
				while (*s)
					if (*s++ == '%' && *s == '(')
						break;
				if (*s)
				{
					s = fp->argv[n];
					listprintf(state.tmp.str, ap, f, s);
					if (!(fp->argv[n] = sfstruse(state.tmp.str)))
						nospace();
					break;
				}
			}
			pp = procopen(*fp->argv, fp->argv, NiL, NiL, PROC_WRITE);
			if (s)
				fp->argv[n] = s;
			if (!pp)
			{
				error(2, "%s: %s: cannot execute filter %s", ap->name, f->path, *fp->argv);
				goto skip;
			}
			if (!ap->format->getdata || !(*ap->format->getdata)(&state, ap, f, pp->wfd))
			{
				checksum = 0;
				for (c = f->st->st_size; c > 0; c -= n)
				{
					n = (c > state.buffersize) ? state.buffersize : c;
					if (!(s = bget(ap, n, NiL)))
					{
						error(ERROR_SYSTEM|2, "%s: read error", f->name);
						break;
					}
					if (write(pp->wfd, s, n) != n)
					{
						error(ERROR_SYSTEM|2, "%s: write error", f->name);
						break;
					}
					if (ap->format->checksum)
						checksum = (*ap->format->checksum)(&state, ap, f, s, n, checksum);
				}
			}
			if (ap->format->checksum && checksum != f->checksum)
				error(1, "%s: %s checksum error (0x%08x != 0x%08x)", f->name, ap->format->name, checksum, f->checksum);
			/*
			 * explicitly ignore exit status
			 */

			procclose(pp);
			return;
		}
		listentry(f);
		goto skip;
	}
	else switch (f->delta.op)
	{
	case DELTA_create:
		if (f->delta.base)
			error(3, "%s: base archive mismatch [%s#%d]", f->name, __FILE__, __LINE__);
		if (ap->delta->format->flags & PSEUDO)
			goto regular;
		if ((wfd = openout(ap, f)) < 0)
			goto skip;
		else
			paxdelta(NiL, ap, f, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_OUTPUT|DELTA_COUNT, wfd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap, f->st->st_size, 0);
		break;
	case DELTA_update:
		if (!f->delta.base || (unsigned long)f->delta.base->mtime.tv_sec >= (unsigned long)f->st->st_mtime)
			error(3, "%s: base archive mismatch [%s#%d]", f->name, __FILE__, __LINE__);
		c = f->st->st_size;
		if ((wfd = openout(ap, f)) < 0)
			goto skip;
		if (state.ordered)
		{
			if (!f->delta.base->uncompressed)
				paxdelta(NiL, ap, f, DELTA_SRC|DELTA_BIO|DELTA_SIZE, ap->delta->base, f->delta.base->size, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_OUTPUT|DELTA_COUNT, wfd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap, c, 0);
			else if (!paxdelta(NiL, ap, f, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap->delta->base, f->delta.base->size, DELTA_TAR|DELTA_TEMP|DELTA_OUTPUT, &dfd, 0))
				paxdelta(NiL, ap, f, DELTA_SRC|DELTA_FD|DELTA_SIZE|DELTA_FREE, dfd, f->delta.base->uncompressed, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_OUTPUT|DELTA_COUNT, wfd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap, c, 0);
		}
		else if (!f->delta.base->uncompressed)
			paxdelta(NiL, ap, f, DELTA_SRC|DELTA_FD|DELTA_OFFSET|DELTA_SIZE, ap->delta->base->io->fd, f->delta.base->offset, f->delta.base->size, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_OUTPUT|DELTA_COUNT, wfd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap, c, 0);
		else if (!paxdelta(NiL, ap, f, DELTA_DEL|DELTA_FD|DELTA_OFFSET|DELTA_SIZE, ap->delta->base->io->fd, f->delta.base->offset, f->delta.base->size, DELTA_TAR|DELTA_TEMP|DELTA_OUTPUT, &dfd, 0))
			paxdelta(NiL, ap, f, DELTA_SRC|DELTA_FD|DELTA_SIZE|DELTA_FREE, dfd, f->delta.base->uncompressed, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_OUTPUT|DELTA_COUNT, wfd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap, c, 0);
		break;
	case DELTA_verify:
		if (!f->delta.base || f->delta.base->mtime.tv_sec != f->st->st_mtime)
			error(3, "%s: base archive mismatch [%s#%d]", f->name, __FILE__, __LINE__);
		if ((*state.statf)(f->name, &st))
			error(2, "%s: not copied from base archive", f->name);
		else if (st.st_size != f->delta.base->size || state.modtime && tvcmp(tvmtime(&t1, &st), tvmtime(&t2, f->st)))
			error(1, "%s: changed from base archive", f->name);
		break;
	case DELTA_delete:
		if (!f->delta.base)
			error(3, "%s: base archive mismatch [%s#%d]", f->name, __FILE__, __LINE__);
		/*FALLTHROUGH*/
	default:
	regular:
		wfd = openout(ap, f);
		if (wfd >= 0)
		{
			holeinit(wfd);
			if (!ap->format->getdata || !(*ap->format->getdata)(&state, ap, f, wfd))
			{
				checksum = 0;
				for (c = f->st->st_size; c > 0; c -= n)
				{
					n = (c > state.buffersize) ? state.buffersize : c;
					if (!(s = bget(ap, n, NiL)))
					{
						error(ERROR_SYSTEM|2, "%s: read error", f->name);
						break;
					}
					if (holewrite(wfd, s, n) != n)
					{
						error(ERROR_SYSTEM|2, "%s: write error", f->name);
						break;
					}
					if (ap->format->checksum)
						checksum = (*ap->format->checksum)(&state, ap, f, s, n, checksum);
				}
			}
			holedone(wfd);
			closeout(ap, f, wfd);
			setfile(ap, f);
			if (ap->format->checksum && checksum != f->checksum)
				error(1, "%s: %s checksum error (0x%08x != 0x%08x)", f->name, ap->format->name, checksum, f->checksum);
		}
		else if (ap->format->getdata)
			(*ap->format->getdata)(&state, ap, f, wfd);
		else
		{
			if (wfd < -1)
				listentry(f);
			goto skip;
		}
		break;
	}
	listentry(f);
	return;
 skip:
	fileskip(ap, f);
}

/*
 * skip over archive member f file data
 */

void
fileskip(register Archive_t* ap, register File_t* f)
{
	Member_t*	d;
	off_t		n;

	if (ap->delta && (d = (Member_t*)hashget(ap->delta->tab, f->name)))
		d->info->delta.op = DELTA_delete;
	if ((!ap->format->getdata || !(*ap->format->getdata)(&state, ap, f, -1)) && ((n = f->st->st_size) > 0 && f->type == X_IFREG || (n = f->datasize)) && bread(ap, NiL, (off_t)0, n, 1) < 0)
		error(ERROR_SYSTEM|2, "%s: skip error", f->name);
}

/*
 * single file copyin() and copyout() smashed together
 * called by ftwalk()
 */

int
copyinout(Ftw_t* ftw)
{
	register File_t*	f = &state.out->file;
	register char*		s;
	register off_t		c;
	register ssize_t	n;
	register int		rfd;
	register int		wfd;

	if (getfile(state.out, f, ftw) && selectfile(state.out, f))
	{
		s = f->name;
		f->name = stash(&state.out->path.copy, NiL, state.pwdlen + f->namesize);
		strcpy(stpcpy(f->name, state.pwd), s + (*s == '/'));
		if ((wfd = openout(state.out, f)) >= 0)
		{
			if ((rfd = openin(state.out, f)) >= 0)
			{
#if defined(SEEK_DATA) && defined(SEEK_HOLE)
				off_t		data;
				off_t		hole;
				int		more;

				data = 0;
				more = 1;
				while (more)
				{
					if ((hole = lseek(rfd, data, SEEK_HOLE)) < data)
					{
						hole = lseek(rfd, 0, SEEK_END);
						more = 0;
					}
					while ((c = hole - data) > 0)
					{
						if (c > state.buffersize)
							c = state.buffersize;
						if (lseek(rfd, data, SEEK_SET) != data || (n = read(rfd, state.tmp.buffer, (size_t)c)) <= 0)
						{
							error(ERROR_SYSTEM|2, "%s: read error", f->name);
							more = 0;
							break;
						}
						if (lseek(wfd, data, SEEK_SET) != data || write(wfd, state.tmp.buffer, n) != n)
						{
							error(ERROR_SYSTEM|2, "%s: write error", f->name);
							more = 0;
							break;
						}
						state.out->io->count += n;
						data += n;
					}
					if (!more)
						break;
					if ((data = lseek(rfd, hole, SEEK_DATA)) < hole)
					{
						if ((data = lseek(rfd, -1, SEEK_END)) < 0 || (data + 1) > hole && (lseek(wfd, data, SEEK_SET) != data || write(wfd, "", 1) != 1))
							error(ERROR_SYSTEM|2, "%s: write error", f->name);
						state.out->io->count += 1;
						break;
					}
				}
#else
				holeinit(wfd);
				for (c = f->st->st_size; c > 0; c -= n)
				{
					if ((n = read(rfd, state.tmp.buffer, (size_t)((c > state.buffersize) ? state.buffersize : c))) <= 0)
					{
						error(ERROR_SYSTEM|2, "%s: read error", f->name);
						break;
					}
					if (holewrite(wfd, state.tmp.buffer, n) != n)
					{
						error(ERROR_SYSTEM|2, "%s: write error", f->name);
						break;
					}
					state.out->io->count += n;
				}
				holedone(wfd);
#endif
				closeout(state.out, f, wfd);
				closein(state.out, f, rfd);
				setfile(state.out, f);
				listentry(f);
			}
			else
				closeout(state.out, f, wfd);
		}
		else if (wfd != -1)
			listentry(f);
	}
	return 0;
}

/*
 * compare ft1 and ft2 for ftwalk() sort
 */

int
cmpftw(Ftw_t* ft1, Ftw_t* ft2)
{
	return strcoll(ft1->name, ft2->name);
}

/*
 * skip to the next unquoted occurrence of d in s
 */

static char*
skip(register char* s, register int d)
{
	register int	c;
	register int	q;

	q = 0;
	while (c = *s++)
		if (c == q)
			q = 0;
		else if (c == '\\')
		{
			if (*s)
				s++;
		}
		else if (!q)
		{
			if (c == d)
				return s - 1;
			else if (c == '"' || c == '\'')
				q = c;
		}
	return 0;
}

/*
 * copy files out using copyfile
 */

typedef int (*Ftw_cmp_t)(Ftw_t*, Ftw_t*);

void
copy(register Archive_t* ap, register int (*copyfile)(Ftw_t*))
{
	register char*	s;
	register char*	t;
	register char*	v;
	register int	c;
	unsigned long	flags;
	char*		mode;
	char*		mtime;

	if (ap)
	{
		deltabase(ap);
		if (ap->delta && ap->delta->format != ap->expected && ap->expected)
			error(3, "%s: archive format %s does not match requested format %s", ap->name, ap->delta->format->name, ap->expected->name);
		if (state.append || state.update)
		{
			ap->format = ap->delta->format;
			if (!(ap->format->flags & APPEND))
				error(3, "%s: archive format %s does support append/update", ap->name, ap->format->name);
			if (state.update)
				ap->update = ap->delta->tab;
			ap->delta = 0;
			ap->parent = 0;
			ap->volume--;
		}
		putprologue(ap, state.append || state.update);
	}
	if (state.files)
		ftwalk((char*)state.files, copyfile, state.ftwflags|FTW_MULTIPLE, state.exact ? (Ftw_cmp_t)0 : cmpftw);
	else
	{
		sfopen(sfstdin, NiL, "rt");
		sfset(sfstdin, SF_SHARE, 0);
		mode = state.mode;
		mtime = state.mtime;
		for (;;)
		{
			if (s = state.peekfile)
			{
				state.peekfile = 0;
				c = state.peeklen;
			}
			else if (!(s = sfgetr(sfstdin, '\n', 1)))
				break;
			else
				c = sfvalue(sfstdin) - 1;
			sfwrite(state.tmp.lst, s, c);
			if (!(s = sfstruse(state.tmp.lst)))
				nospace();
			flags = state.ftwflags;
			if (state.filter.line)
			{
				if (!(c = *s++))
					continue;
				state.filter.options = s;
				if (!(s = skip(s, c)))
					continue;
				*s++ = 0;
				state.filter.command = s;
				if (!(s = skip(s, c)))
					continue;
				*s++ = 0;
				state.filter.path = s;
				if (!(s = skip(s, c)))
					state.filter.name = state.filter.path;
				else
				{
					*s++ = 0;
					state.filter.name = s;
					if (s = skip(s, c))
						*s = 0;
				}
				s = state.filter.options;
				for (;;)
				{
					if (t = strchr(s, ','))
						*t = 0;
					if (v = strchr(s, '='))
					{
						*v++ = 0;
						c = strtol(v, NiL, 0);
					}
					else
						c = 1;
					if (s[0] == 'n' && s[1] == 'o')
					{
						s += 2;
						c = !c;
					}
					if (streq(s, "logical") || streq(s, "physical"))
					{
						if (s[0] == 'p')
							c = !c;
						if (c)
							flags &= ~(FTW_META|FTW_PHYSICAL);
						else
						{
							flags &= ~(FTW_META);
							flags |= FTW_PHYSICAL;
						}
					}
					else if (streq(s, "mode"))
						state.mode = v;
					else if (streq(s, "mtime"))
						state.mtime = v;
					if (!t)
						break;
					s = t + 1;
				}
				s = state.filter.path;
				state.filter.line = *state.filter.name ? 2 : 1;
			}
			c = *s ? ftwalk(s, copyfile, flags, NiL) : 0;
			state.mode = mode;
			state.mtime = mtime;
			if (c)
			{
				error(2, "%s: not completely copied", s);
				break;
			}
		}
	}
	if (ap)
	{
		deltadelete(ap);
		putepilogue(ap);
	}
}
