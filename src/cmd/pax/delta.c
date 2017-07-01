/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2011 AT&T Intellectual Property          *
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
 * pax delta archive support
 */

#include "pax.h"

#include <update.h>

/*
 * interpret delta header/trailer ops
 */

static void
getdeltaops(Archive_t* ap, File_t* f)
{
	register char*	s;
	register char*	e;
	register int	n;
	char		c;

	if (state.delta2delta || (ap->format->flags & DELTAINFO))
		return;
	s = ap->delta->hdrbuf;
	e = s + sizeof(ap->delta->hdrbuf) - 1;
	n = 0;
	while (f->st->st_size > 0 && bread(ap, &c, (off_t)1, (off_t)1, 1) > 0)
	{
		f->st->st_size--;
		if (c == '\t' || c == '\n')
		{
			if (n)
			{
				*s = 0;
				s = ap->delta->hdrbuf;
				switch (n)
				{
				case DELTA_checksum:
					f->delta.checksum = strtoul(s, NiL, 16);
					break;
				case DELTA_index:
					f->delta.index = strtol(s, NiL, 0);
					break;
				case DELTA_trailer:
					if ((n = strtol(s, NiL, 0)) > 0)
					{
						ap->delta->epilogue = -1;
						ap->delta->trailer = n;
						f->st->st_size -= n;
					}
					break;

					/*
					 * ignore unknown ops for future
					 */
				}
				n = 0;
			}
			if (c == '\n')
				break;
		}
		else if (!n) n = c;
		else if (s < e) *s++ = c;
	}
}

/*
 * get supplemental delta header info
 */

void
getdeltaheader(register Archive_t* ap, register File_t* f)
{
	register char*	s;
	int		n;
	unsigned long	sum;
	Sfio_t*		sp;
	char		c;

	if (!(ap->format->flags & COMPRESSED))
	{
		if (ap->delta && ap->delta->format && (ap->delta->format->variant == DELTA_94 || ap->delta->format->variant == DELTA_IGNORE && state.delta2delta))
		{
			ap->delta->index++;
			if (ap->delta->tab && f->name && (f->delta.base = (Member_t*)hashget(ap->delta->tab, f->name)))
				f->delta.base->mark = 1;
			if (!(ap->format->flags & DELTAINFO))
			{
				if (f->st->st_size <= 0 || bread(ap, &c, (off_t)1, (off_t)1, 1) <= 0)
					f->delta.op = DELTA_create;
				else
				{
					f->st->st_size--;
					f->delta.op = c;
					getdeltaops(ap, f);
					if (f->st->st_size >= 12 && (f->delta.op == DELTA_create || f->delta.op == DELTA_update))
					{
						sum = ap->memsum;
						s = ap->delta->hdrbuf;
						n = 12;
						if (bread(ap, s, (off_t)n, (off_t)n, 1) > 0)
						{
							if (ap->delta->format->variant == DELTA_88)
							{
								unsigned char*	u = (unsigned char*)s;
								int		i;

								i = *u++;
								u += (i >> 3) & 07;
								f->uncompressed = 0;
								i &= 07;
								while (i-- > 0)
									f->uncompressed = f->uncompressed * 256 + *u++;
							}
							else if (sp = sfnew(NiL, s + 4, n, -1, SF_READ|SF_STRING))
							{
								f->uncompressed = sfgetu(sp);
								sfclose(sp);
							}
							bunread(ap, s, n);
						}
						ap->memsum = sum;
					}
				}
			}
		}
		else if (state.operation == (IN|OUT))
			f->delta.op = DELTA_pass;
	}
}

/*
 * get supplemental delta trailer info
 */

void
getdeltatrailer(register Archive_t* ap, register File_t* f)
{
	long		n;
	unsigned long	x;

	if (ap->delta && !f->extended)
	{
		if (ap->delta->trailer)
		{
			f->st->st_size += ap->delta->trailer;
			ap->delta->trailer = 0;
			getdeltaops(ap, f);
		}
		if (!f->delta.base)
		{
			if (ap->parent && ap->parent != ap && f->delta.op != DELTA_create)
				error(2, "%s: %s: corrupt archive: not in base archive %s", ap->name, f->name, ap->parent->name);
		}
		else
		{
			x = (ap->format->flags & SUM) ? f->delta.base->info->checksum : ap->memsum;
			if ((f->delta.checksum ^ x) & 0xffffffff)
				error(2, "%s: %s: corrupt archive: checksum mismatch -- expected %08lx, got %08lx", ap->name, f->name, f->delta.checksum & 0xffffffff, x & 0xffffffff);
		}
		if (n = f->delta.index - ap->delta->index)
		{
			if (n > 0)
				error(2, "%s: %s: corrupt archive: %d missing file%s", ap->name, f->name, n, n == 1 ? "" : "s");
			else
				error(2, "%s: %s: corrupt archive: delta index out of sync by %d file%s", ap->name, f->name, -n, -n == 1 ? "" : "s");
			ap->delta->index = n;
		}
	}
}

/*
 * initialize delta header output info
 */

void
setdeltaheader(register Archive_t* ap, register File_t* f)
{
	register char*	s;
	register int	n;

	if (f->delta.op && ap->delta)
	{
		ap->delta->index++;
		if (ap->format->flags & STANDARD)
		{
			switch (f->delta.op)
			{
			case DELTA_create:
				s = "create";
				break;
			case DELTA_delete:
				s = "delete";
				break;
			case DELTA_pass:
				s = "pass";
				break;
			case DELTA_update:
				s = "update";
				break;
			case DELTA_verify:
				s = "verify";
				break;
			}
			putkey(ap, ap->tmp.extended, &options[OPT_delta_op], s, 0);
			putkey(ap, ap->tmp.extended, &options[OPT_uncompressed], NiL, f->uncompressed);
			putkey(ap, ap->tmp.extended, &options[OPT_delta_index], NiL, ap->delta->index);
			if (ap->delta->tab && (f->delta.base = (Member_t*)hashget(ap->delta->tab, f->name)))
				putkey(ap, ap->tmp.extended, &options[OPT_delta_checksum], NiL, f->delta.base->info->checksum & 0xffffffff);
		}
		else
		{
			s = ap->delta->hdrbuf;
			n = sfsprintf(s, sizeof(ap->delta->hdrbuf), "%c%c%d\t%c%d\n", f->delta.op, DELTA_index, ap->delta->index, DELTA_trailer, DELTA_TRAILER);
			ap->delta->hdr = s + n;
			n += DELTA_TRAILER;
			f->st->st_size += n;
			ap->memsum = 0;
			ap->sum = 1;
		}
	}
}

/*
 * output supplementary delta header info
 */

void
putdeltaheader(register Archive_t* ap, register File_t* f)
{
	int	n;

	if (f->delta.op && ap->delta && (n = ap->delta->hdr - ap->delta->hdrbuf))
	{
		bwrite(ap, ap->delta->hdrbuf, n);
		n += DELTA_TRAILER;
		f->st->st_size -= n;
		ap->delta->hdr = ap->delta->hdrbuf;
	}
}

/*
 * output supplementary delta trailer info
 */

void
putdeltatrailer(register Archive_t* ap, register File_t* f)
{
	register char*	s;
	register int	n;

	if (f->delta.op && ap->delta)
	{
		if (!(ap->format->flags & STANDARD))
		{
			ap->sum = 0;
			s = ap->delta->hdrbuf;
			n = sfsprintf(s, sizeof(ap->delta->hdrbuf), "%c%08X\n", DELTA_checksum, ap->memsum);
			if (n != DELTA_TRAILER)
				error(PANIC, "delta trailer size %d shoulda been %d", n, DELTA_TRAILER);
			bwrite(ap, s, n);
		}
	}
}

/*
 * initialize delta tables
 */

void
initdelta(Archive_t* ap, Format_t* dp)
{
	if (!ap->delta && !(ap->delta = newof(0, Delta_t, 1, 0)))
		nospace();
	if (!ap->delta->tab && !(ap->delta->tab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_name, "delta", 0)))
		nospace();
	ap->delta->format = dp;
}

/*
 * get delta base archive info
 */

void
deltabase(register Archive_t* ap)
{
	register Archive_t*	bp;
	Format_t*		fp;
	struct stat		st;

	if (!ap->delta || ap->delta->initialized)
		return;
	ap->delta->initialized = 1;
	if (!(bp = ap->delta->base))
		bp = ap->delta->base = initarchive("/dev/null", O_RDONLY);
	binit(bp);
	bp->parent = ap;
	if ((bp->io->mode & O_ACCMODE) != O_WRONLY)
		bp->sum++;
	if ((bp->io->fd = open(bp->name, bp->io->mode|O_BINARY)) < 0 || fstat(bp->io->fd, &st))
		error(ERROR_SYSTEM|3, "%s: %s: cannot open base archive", ap->name, bp->name);
	if (S_ISREG(st.st_mode) && st.st_size > 0)
	{
		bp->io->seekable = 1;
		bp->io->size = st.st_size;
	}
	else
		bp->io->seekable = 0;
	if (st.st_size)
	{
		fp = bp->expected;
		bp->expected = 0;
		if (state.ordered)
		{
			if (!getprologue(bp))
				error(3, "%s: %s: base archive is empty", ap->name, bp->name);
			bp->sum--;
			bp->checksum = memsum(bp->io->next, bcount(bp), 0L);
		}
		else
		{
			if (!state.append && !state.update && lseek(bp->io->fd, (off_t)0, SEEK_SET) != 0)
				error(ERROR_SYSTEM|3, "%s: %s: base archive must be seekable", ap->name, bp->name);
			copyin(bp);
			bp->size = bp->io->offset + bp->io->count;
		}
		bp->expected = fp;
		bp->checksum &= 0xffffffff;
	}
	if (state.append || state.update)
		ap->delta->format = st.st_size ? bp->format : ap->format;
	else if (!ap->delta->format)
	{
		ap->delta->format = getformat(FMT_DELTA, 1);
		ap->delta->compress = !st.st_size;
	}
}

/*
 * verify untouched base files
 */

void
deltaverify(Archive_t* ap)
{
	register int		wfd;
	register Member_t*	d;
	register off_t		c;
	register off_t		n;
	Hash_position_t*	pos;

	if (!state.delta.update && !state.list && ap->delta && ap->delta->base != ap && (pos = hashscan(ap->delta->tab, 0)))
	{
		message((-2, "verify untouched base files"));
		while (hashnext(pos))
		{
			d = (Member_t*)pos->bucket->value;
			message((-1, "%s: mark=%d", d->info->name, d->mark));
			if (!d->mark && selectfile(ap, d->info) && (wfd = openout(ap, d->info)) >= 0)
			{
				ap->entries++;
				if (!d->uncompressed)
				{
					if (!state.ordered && lseek(ap->delta->base->io->fd, d->offset, SEEK_SET) != d->offset)
						error(ERROR_SYSTEM|3, "%s: base archive seek error", ap->delta->base->name);
					holeinit(wfd);
					for (c = d->info->st->st_size; c > 0; c -= n)
					{
						n = (c > state.buffersize) ? state.buffersize : c;
						if ((n = state.ordered ? bread(ap, state.tmp.buffer, n, n, 1) : read(ap->delta->base->io->fd, state.tmp.buffer, n)) <= 0)
						{
							error(ERROR_SYSTEM|2, "%s: %s: read error", ap->delta->base->name, d->info->name);
							break;
						}
						else ap->io->count += n;
						if (holewrite(wfd, state.tmp.buffer, n) != n)
						{
							error(ERROR_SYSTEM|2, "%s: write error", d->info->name);
							break;
						}
						ap->io->count += n;
					}
					holedone(wfd);
					closeout(ap, d->info, wfd);
					setfile(ap, d->info);
					listentry(d->info);
				}
				else if (state.ordered) paxdelta(NiL, ap, d->info, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap->delta->base, d->size, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_OUTPUT|DELTA_COUNT|DELTA_LIST, wfd, 0);
				else paxdelta(NiL, ap, d->info, DELTA_DEL|DELTA_FD|DELTA_OFFSET|DELTA_SIZE, ap->delta->base->io->fd, d->offset, d->size, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_OUTPUT|DELTA_COUNT|DELTA_LIST, wfd, 0);
			}
		}
		hashdone(pos);
	}
}

/*
 * output prefix dirs to the archive first
 */

static void
deltaprefix(Archive_t* ip, Archive_t* op, register Member_t* d)
{
	register char*		s;
	register Member_t*	m;

	d->mark = 1;
	if (s = strrchr(d->info->path, '/'))
	{
		*s = 0;
		if (!(m = (Member_t*)hashget(ip->delta->tab, d->info->name)))
		{
			if (!(m = newof(0, Member_t, 1, 0)))
				nospace();
			m->mark = 1;
			hashput(ip->delta->tab, 0, m);
		}
		else if (!m->mark)
			deltaprefix(ip, op, m);
		*s = '/';
	}
	d->info->fd = -1;
	fileout(op, d->info);
}

/*
 * delta file if necessary and copy out
 */

void
deltaout(Archive_t* ip, Archive_t* op, register File_t* f)
{
	register Member_t*	d;
	int			dfd;
	int			skip;

	skip = !!ip;
	f->delta.same = 0;
	if (d = op->delta && op->delta->tab && f->name ? (Member_t*)hashget(op->delta->tab, f->name) : (Member_t*)0)
		d->mark = 1;
	if (op->delta && (op->delta->format->flags & DELTAIO))
	{
		if (f->type == X_IFREG && f->linktype == NOLINK && (!d || f->st->st_mtime != d->mtime.tv_sec || (f->st->st_mode & X_IPERM) != (d->mode & X_IPERM)))
		{
			if (f->ordered)
			{
				f->ordered = 0;
				fileout(op, f);
				return;
			}
			if (d)
			{
				f->delta.op = DELTA_update;
				f->st->st_dev = d->dev;
				f->st->st_ino = d->ino;
				message((-2, "delta: delta: file=%s offset=%ld size=%ld expand=%d", f->name, d->offset, d->size, d->uncompressed));
				if (state.ordered)
				{
					if (!d->uncompressed)
						paxdelta(ip, op, f, DELTA_SRC|DELTA_BIO|DELTA_SIZE, op->delta->base, d->size, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_SIZE, f->fd, f->st->st_size, DELTA_DEL|DELTA_TEMP|DELTA_OUTPUT, &f->fd, 0);
					else if (!paxdelta(ip, op, d->info, DELTA_DEL|DELTA_BIO|DELTA_SIZE, op->delta->base, d->size, DELTA_TAR|DELTA_TEMP|DELTA_OUTPUT, &dfd, 0))
						paxdelta(ip, op, f, DELTA_SRC|DELTA_FD|DELTA_SIZE|DELTA_FREE, dfd, d->uncompressed, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_SIZE, f->fd, f->st->st_size, DELTA_DEL|DELTA_TEMP|DELTA_OUTPUT, &f->fd, 0);
				}
				else if (!d->uncompressed)
					paxdelta(ip, op, f, DELTA_SRC|DELTA_FD|DELTA_OFFSET|DELTA_SIZE, op->delta->base->io->fd, d->offset, d->size, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_SIZE, f->fd, f->st->st_size, DELTA_DEL|DELTA_TEMP|DELTA_OUTPUT, &f->fd, 0);
				else if (!paxdelta(ip, op, d->info, DELTA_DEL|DELTA_FD|DELTA_OFFSET|DELTA_SIZE, op->delta->base->io->fd, d->offset, d->size, DELTA_TAR|DELTA_TEMP|DELTA_OUTPUT, &dfd, 0))
					paxdelta(ip, op, f, DELTA_SRC|DELTA_FD|DELTA_SIZE|DELTA_FREE, dfd, d->uncompressed, DELTA_TAR|DELTA_FD|DELTA_SIZE|DELTA_FREE, f->fd, f->st->st_size, DELTA_DEL|DELTA_TEMP|DELTA_OUTPUT, &f->fd, 0);
			}
			else
			{
				f->delta.op = DELTA_create;
				message((-2, "delta: create: file=%s", f->name));
				paxdelta(ip, op, f, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_SIZE, f->fd, f->st->st_size, DELTA_DEL|DELTA_TEMP|DELTA_OUTPUT, &f->fd, 0);
			}
			skip = 0;
		}
		else
		{
			f->delta.op = (d && (f->type == X_IFREG || f->type == X_IFDIR)) ? DELTA_update : DELTA_create;
			message((-2, "delta: %s: file=%s", f->delta.op == DELTA_update ? "update" : "create", f->name));
			if (skip)
			{
				skip = 0;
				fileskip(ip, f);
			}
			f->st->st_size = 0;
			if (f->delta.op == DELTA_update && f->type != X_IFREG && f->st->st_mode == d->mode)
				f->st->st_mtime = d->mtime.tv_sec;
		}
	}
	if (!d || !f->delta.same && d->mtime.tv_sec != f->st->st_mtime)
	{
		register char*	s;

		if (ip && ip->delta && ip->delta->tab && f->name && (s = strrchr(f->name, '/')))
		{
			*s = 0;
			if ((d = (Member_t*)hashget(ip->delta->tab, f->name)) && !d->mark)
				deltaprefix(ip, op, d);
			*s = '/';
		}
		fileout(op, f);
	}
	else if (f->fd >= 0)
		close(f->fd);
	else if (skip)
		fileskip(ip, f);
}

/*
 * copy file from input to output archive
 */

static void
deltacopy(Archive_t* ip, Archive_t* op, register File_t* f)
{
	if (f->delta.base)
	{
		f->st->st_size = f->delta.base->size;
		if (f->delta.base->uncompressed)
		{
			if (state.ordered) paxdelta(ip, NiL, f->delta.base->info, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ip->delta->base, f->delta.base->size, DELTA_TAR|DELTA_TEMP|DELTA_OUTPUT, &f->fd, 0);
			else paxdelta(ip, NiL, f->delta.base->info, DELTA_DEL|DELTA_FD|DELTA_OFFSET|DELTA_SIZE, ip->delta->base->io->fd, f->delta.base->offset, f->delta.base->size, DELTA_TAR|DELTA_TEMP|DELTA_OUTPUT, &f->fd, 0);
		}
		else if (state.ordered)
		{
			f->ap = ip;
			f->fd = -1;
			f->ordered = 1;
		}
		else if ((f->fd = dup(ip->delta->base->io->fd)) < 0)
			error(ERROR_SYSTEM|3, "%s: cannot reopen", ip->delta->base->name);
		else if (lseek(f->fd, f->delta.base->offset, SEEK_SET) < 0)
			error(ERROR_SYSTEM|3, "%s: base archive seek error", ip->delta->base->name);
	}
	deltaout(ip, op, f);
}

/*
 * copy the delta base archive delete entries
 */

void
deltadelete(register Archive_t* ap)
{
	register File_t*	f;
	register Member_t*	d;
	Hash_position_t*	pos;

	if (!state.ordered && ap->delta && ap->delta->tab)
	{
		if (pos = hashscan(ap->delta->tab, 0))
		{
			message((-2, "copy the base delete entries"));
			f = &ap->file;
			while (hashnext(pos))
			{
				d = (Member_t*)pos->bucket->value;
				if (!d->mark && (!d->info || !d->info->ro))
				{
					ap->selected++;
					initfile(ap, f, f->st, pos->bucket->name, X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IROTH);
					f->delta.op = DELTA_delete;
					if (d->info && d->info->st)
					{
						f->st->st_mode = d->info->st->st_mode;
						f->st->st_gid = d->info->st->st_gid;
						f->st->st_uid = d->info->st->st_uid;
						f->st->st_mtime = d->info->st->st_mtime;
					}
					else
					{
						f->st->st_gid = state.uid;
						f->st->st_uid = state.gid;
						f->st->st_mtime = NOW;
					}
					putheader(ap, f);
					puttrailer(ap, f);
				}
			}
			hashdone(pos);
		}
	}
}

/*
 * update file deltas from archive and output to archive
 */

void
deltapass(Archive_t* ip, Archive_t* op)
{
	register File_t*	f;
	register off_t		c;
	register ssize_t	n;
	Member_t*		d;
	Member_t*		h;
	char*			p;
	Filter_t*		fp;
	Hash_position_t*	pos;

	if (state.delta2delta != 2)
		state.delta2delta = 0;
	message((-1, "delta PASS%s", state.delta2delta ? " delta2delta" : ""));
	deltabase(ip);
	deltabase(op);
	putprologue(op, 0);
	f = &ip->file;
	while (getprologue(ip))
	{
		while (getheader(ip, f))
		{
			f->fd = -1;
			f->ap = ip;
			if (f->ro)
				fileskip(ip, f);
			else if (state.delta2delta)
			{
				if (validout(op, f) && selectfile(op, f))
					fileout(op, f);
				else
					fileskip(ip, f);
			}
			else switch (f->delta.op)
			{
			case DELTA_create:
				if (f->type != X_IFREG || f->linktype != NOLINK)
				{
					f->st->st_size = 0;
					goto pass;
				}
				if (f->delta.base)
					error(3, "%s: base archive mismatch [%s#%d]", f->name, __FILE__, __LINE__);
				if (validout(op, f) && selectfile(op, f))
				{
					paxdelta(ip, op, f, DELTA_TAR|DELTA_TEMP|DELTA_OUTPUT, &f->fd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ip, f->st->st_size, 0);
					if (op->delta && (op->delta->format->flags & DELTAIO))
					{
						f->delta.op = DELTA_create;
						paxdelta(ip, op, f, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_SIZE, f->fd, f->st->st_size, DELTA_DEL|DELTA_TEMP|DELTA_OUTPUT, &f->fd, 0);
					}
					else f->delta.op = 0;
					deltaout(ip, op, f);
				}
				else fileskip(ip, f);
				break;
			case DELTA_pass:
				if (validout(op, f) && selectfile(op, f))
				{
					if (ip->delta && (ip->delta->format->flags & DELTAIO))
					{
						f->delta.op = DELTA_create;
						paxdelta(ip, op, f, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ip, f->st->st_size, DELTA_TAR|DELTA_TEMP|DELTA_OUTPUT, &f->fd, 0);
					}
					else if (fp = filter(op, f))
					{
						int		wfd;

						static char*	tmp;

						if (!tmp)
							tmp = pathtemp(NiL, 0, NiL, error_info.id, NiL);
						if ((wfd = open(tmp, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, S_IRUSR)) < 0)
						{
							error(2, "%s: cannot create filter temporary %s", f->path, tmp);
							fileskip(ip, f);
							break;
						}
						holeinit(wfd);
						for (c = f->st->st_size; c > 0; c -= state.buffersize)
						{
							n = c > state.buffersize ? state.buffersize : c;
							if (bread(ip, state.tmp.buffer, n, n, 1) <= 0)
							{
								error(ERROR_SYSTEM|2, "%s: read error", f->name);
								break;
							}
							if (holewrite(wfd, state.tmp.buffer, n) != n)
							{
								error(ERROR_SYSTEM|2, "%s: filter temporary write error to %s", f->name, tmp);
								break;
							}
						}
						holedone(wfd);
						close(wfd);
						p = f->path;
						f->path = tmp;
						f->fd = apply(op, f, fp);
						if (remove(f->path))
							error(1, "%s: cannot remove filter temporary", p, f->path);
						f->path = p;
						f->delta.op = 0;
					}
					else f->delta.op = 0;
					deltaout(ip, op, f);
				}
				else fileskip(ip, f);
				break;
			case DELTA_delete:
				if (!f->delta.base)
					error(3, "%s: base archive mismatch [%s#%d]", f->name, __FILE__, __LINE__);
				if (ip->delta && (d = (Member_t*)hashget(ip->delta->tab, f->name)))
					d->info->delta.op = DELTA_delete;
				break;
			case DELTA_update:
				if (!f->delta.base)
					error(3, "%s: base archive mismatch [%s#%d]", f->name, __FILE__, __LINE__);
				if (validout(op, f) && selectfile(op, f))
				{
					if (state.ordered) paxdelta(ip, op, f, DELTA_SRC|DELTA_BIO|DELTA_SIZE, ip->delta->base, f->delta.base->size, DELTA_TAR|DELTA_TEMP|DELTA_OUTPUT, &f->fd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ip, f->st->st_size, 0);
					else paxdelta(ip, op, f, DELTA_SRC|DELTA_FD|DELTA_OFFSET|DELTA_SIZE, ip->delta->base->io->fd, f->delta.base->offset, f->delta.base->size, DELTA_TAR|DELTA_TEMP|DELTA_OUTPUT, &f->fd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ip, f->st->st_size, 0);
					if (op->delta && (op->delta->format->flags & DELTAIO))
					{
						f->delta.op = DELTA_create;
						paxdelta(ip, op, f, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_SIZE, f->fd, f->st->st_size, DELTA_DEL|DELTA_TEMP|DELTA_OUTPUT, &f->fd, 0);
					}
					else f->delta.op = 0;
					deltaout(ip, op, f);
				}
				else fileskip(ip, f);
				break;
			case DELTA_verify:
				if (!f->delta.base || f->delta.base->mtime.tv_sec != f->st->st_mtime)
					error(3, "%s: base archive mismatch [%s#%d]", f->name, __FILE__, __LINE__);
			pass:
				if (validout(op, f) && selectfile(op, f))
				{
					f->delta.op = 0;
					deltacopy(ip, op, f);
				}
				else fileskip(ip, f);
				break;
			default:
				error(3, "%s: %s: not a delta archive (op=%d)", ip->name, f->name, f->delta.op);
				break;
			}
			gettrailer(ip, f);
		}
		if (!getepilogue(ip))
			break;
	}
	if (ip->delta && ip->delta->tab)
	{
		/*
		 * copy the non-empty untouched base hard links first
		 */

		if (pos = hashscan(ip->delta->tab, 0))
		{
			message((-2, "copy non-empty untouched base hard links"));
			while (hashnext(pos))
			{
				d = (Member_t*)pos->bucket->value;
				if (!d->mark && d->info->linktype != HARDLINK && d->info->st->st_size > 0 && selectfile(op, d->info))
				{
					d->mark = 1;
					deltacopy(ip, op, d->info);
				}
			}
			hashdone(pos);
		}

		/*
		 * copy the remaining untouched base files and deletes
		 */

		if (pos = hashscan(ip->delta->tab, 0))
		{
			message((-2, "copy remaining untouched base files"));
			while (hashnext(pos))
			{
				d = (Member_t*)pos->bucket->value;
				if (!d->mark && selectfile(op, d->info))
				{
					d->mark = 1;
					if (d->info->linktype == HARDLINK)
					{
						if (!(h = (Member_t*)hashget(ip->delta->tab, d->info->linkpath)))
							error(1, "%s: %s: %s: hard link not in base archive", ip->name, d->info->name, d->info->linkpath);
						else if (!h->mark || h->info->delta.op == DELTA_delete)
						{
							h->info->name = d->info->name;
							d = h;
						}
					}
					deltacopy(ip, op, d->info);
				}
			}
			hashdone(pos);
		}
	}
	deltadelete(op);
	putepilogue(op);
	op->volume = 0;
	op->selected = op->entries;
}

/*
 * set delta info from pseudo file
 */

void
deltaset(register Archive_t* ap, char* s)
{
	Format_t*	dp;
	char*		t;
	int		type;

	dp = 0;
	if ((type = *++s) != TYPE_COMPRESS && type != TYPE_DELTA)
		error(3, "type %c encoding not supported", *s);
	if (*++s == INFO_SEP)
	{
		if (t = strchr(++s, INFO_SEP))
			*t++ = 0;
		if (*s)
		{
			if (isdigit(*s))
				s = sfprints("delta%s", s);
			dp = getformat(s, 1);
		}

		/*
		 * [<INFO_SEP>[<OP>]<VAL>]* may appear here
		 */

		while ((s = t) && *s != INFO_SEP)
		{
			if (t = strchr(s, INFO_SEP))
				*t++ = 0;
			switch (*s++)
			{
			case INFO_ORDERED:
				ap->ordered = 1;
				break;
			}
		}
	}
	if (!dp)
		dp = getformat(FMT_DELTA, 1);
	initdelta(ap, dp);
	ap->delta->compress = type == TYPE_COMPRESS;
	ap->checkdelta = 0;
}

/*
 * check for delta pseudo file
 */

int
deltacheck(register Archive_t* ap, register File_t* f)
{
	register char*		s;
	register Archive_t*	bp;
	register char*		t;
	off_t			size;
	unsigned long		checksum;

	if (!f || !f->st->st_size && !f->st->st_dev && !f->st->st_ino && !(f->st->st_mode & (X_IRWXU|X_IRWXG|X_IRWXO)) && strmatch(f->name, INFO_MATCH))
	{
		if (ap->checkdelta)
		{
			ap->checkdelta = 0;
			if (f)
			{
				s = f->name;
				size = f->st->st_mtime;
				checksum = (DELTA_LO(f->st->st_gid) << 16) | DELTA_LO(f->st->st_uid);
				if (streq(s, "DELTA!!!"))
					initdelta(ap, getformat("delta88", 1));
				else if (*s++ == INFO_SEP)
				{
					if (strneq(s, ID, IDLEN) && (t = strchr(s, INFO_SEP)))
						deltaset(ap, t);
					else
					{
						if (t = strchr(s += 2, INFO_SEP))
							*t = 0;
						error(1, "unknown %s header ignored", s);
						return 0;
					}
				}
			}
			else if (ap->delta)
			{
				size = ap->delta->size;
				checksum = ap->delta->checksum;
			}
			if (ap->delta && ap->delta->format)
			{
				if (!ap->delta->compress && ap->parent)
					error(3, "%s: %s: base archive cannot be a delta", ap->parent->name, ap->name);
				if (bp = ap->delta->base)
				{
					if (ap->delta->format->variant == DELTA_88)
						bp->checksum = bp->old.checksum;
					message((-5, "checkdelta: %s size=%I*d:%I*d checksum=%08x:%08x", ap->delta->format->name, sizeof(size), size, sizeof(bp->size), bp->size, checksum, bp->checksum));
					if (!ap->delta->compress)
					{
						if (!ap->ordered)
						{
							if (state.ordered)
								error(3, "%s: delta archive not ordered", ap->name);
							if (bp->size != size)
								error(3, "%s: %s: base archive size mismatch -- expected %I*u, got %I*u", ap->name, bp->name, sizeof(size), size, sizeof(bp->size), bp->size);
						}
						if ((bp->checksum ^ checksum) & 0xffffffff)
							error(1, "%s: %s: base archive checksum mismatch -- expected %08lx, got %08lx", ap->name, bp->name, checksum & 0xffffffff, bp->checksum & 0xffffffff);
					}
				}
				else if (!ap->delta->compress)
				{
					error(state.list ? 1 : 3, "%s: base archive must be specified", ap->name);
					deltabase(ap);
					ap->delta->compress = 1;
				}
				if (ap->sum <= 0)
					ap->sum++;
				return 1;
			}
			if (f)
				error(1, "%s: %s: unknown control header treated as regular file", ap->name, f->name);
		}
		else if (f && *f->name == INFO_SEP && strneq(f->name + 1, ID, IDLEN) && *(f->name + IDLEN + 1) == INFO_SEP)
		{
			getdeltaheader(ap, f);
			setinfo(ap, f);
			return 1;
		}
	}
	if (f && ap->checkdelta && ap->delta)
		ap->delta->format = getformat(FMT_PATCH, 1);
	return 0;
}

#include <vdelta.h>

typedef struct
{
	Vddisc_t	vd;
	Archive_t*	ap;
	off_t		base;
	Archive_t*	bp;
	off_t		offset;
	int*		pfd;
	int		fd;
	int		op;
} Vdio_t;

#define Vdoff_t		long

/*
 * delta discipline read
 */

static int
delread(void* buf, int n, Vdoff_t off, Vddisc_t* vd)
{
	register Vdio_t*	dp = (Vdio_t*)vd;
	register Vdoff_t	diff;

	message((-6, "delread: op=%o buf=%p n=%d off=%I*d nxt=%I*d", dp->op, buf, n, sizeof(off), off, sizeof(dp->offset), dp->offset));
	if (diff = off - dp->offset)
	{
		dp->offset = off;
		if (dp->op & DELTA_BIO)
		{
			if (bseek(dp->bp, diff, SEEK_CUR, 0) < 0)
				error(PANIC, "BIO seek: have=%I*d need=%I*d", sizeof(dp->offset), dp->offset, sizeof(off), off);
		}
		else
		{
			off += dp->base;
			if (lseek(dp->fd, off, SEEK_SET) != off)
			{
				message((-8, "delread: fd=%d n=%d off=%I*d: seek error", dp->fd, n, sizeof(off), off));
				return -1;
			}
		}
	}
	if (n > (dp->vd.size - dp->offset))
		n = dp->vd.size - dp->offset;
	if (n <= 0)
	{
		message((-8, "delread: fd=%d n=%d siz=%I*d off=%I*d: seek error", dp->fd, n, sizeof(dp->vd.size), dp->vd.size, sizeof(dp->offset), dp->offset));
		return 0;
	}
	n = (dp->op & DELTA_BIO) ? bread(dp->bp, buf, (off_t)0, (off_t)n, 1) : read(dp->fd, buf, n);
	if (n > 0)
		dp->offset += n;
	return n;
}

/*
 * delta discipline write
 */

static int
delwrite(void* buf, int n, Vdoff_t off, Vddisc_t* vd)
{
	register Vdio_t*	dp = (Vdio_t*)vd;
	Buffer_t*		bp;
	ssize_t			k;

	message((-6, "delwrite: op=%o buf=%p n=%d off=%I*d", dp->op, buf, n, sizeof(off), off));
	if (dp->op & DELTA_BIO)
	{
		bwrite(dp->bp, buf, n);
		return n;
	}
	if (dp->op & DELTA_HOLE)
		return holewrite(dp->fd, buf, n);
	if (bp = getbuffer(dp->fd))
	{
		if (bp->next + n < bp->past)
		{
			memcpy(bp->next, buf, n);
			bp->next += n;
			return n;
		}
		if ((dp->fd = open(state.tmp.file, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, S_IRUSR)) < 0)
			error(3, "%s: cannot create delta temporary file", state.tmp.file);
		k = bp->next - bp->base;
		if (k > 0 && write(dp->fd, bp->base, k) != k)
			return -1;
	}
	return write(dp->fd, buf, n);
}

/*
 * delta/update algorithm wrapper
 */

int
paxdelta(Archive_t* ip, Archive_t* ap, File_t* f, int op, ...)
{
	register Vdio_t*	dp;
	va_list			vp;
	ssize_t			n;
	int			bufferclash = 0;
	int			hole = 0;
	Format_t*		fp;
	Buffer_t*		bp;
	Sfio_t*			mp = 0;
	Vdio_t*			gen = 0;
	Vdio_t			data[3];

#if DEBUG
	static const char*	dataname[] = { "src", "tar", "del", "HUH" };
#endif

	va_start(vp, op);
#if 0
	if (ap && ap->delta && !ap->delta->format)
		ap->delta->format = getformat(FMT_DELTA, 1);
	if (ip && ip->delta && !ip->delta->format)
		ip->delta->format = getformat(FMT_DELTA, 1);
#else
	if (ap && ap->delta)
	{
		if (!ap->delta->format)
			ap->delta->format = getformat(FMT_DELTA, 1);
		fp = ap->delta->format;
	}
	if (ip && ip->delta)
	{
		if (!ip->delta->format)
			ip->delta->format = getformat(FMT_DELTA, 1);
		fp = ip->delta->format;
	}
#endif
#if DEBUG
	if (error_info.trace <= -5) mp = sfstropen();
#endif
	memzero(data, sizeof(data));
	while (op)
	{
		dp = &data[op & DELTA_DATA];
		dp->ap = ap && ap->delta ? ap : ip;
#if DEBUG
		if (mp) sfprintf(mp, " %s [", dataname[op & DELTA_DATA]);
#endif
		if (op & DELTA_BIO)
			dp->bp = va_arg(vp, Archive_t*);
		else if (op & DELTA_FD)
		{
			if ((dp->fd = va_arg(vp, int)) == -1)
			{
				op &= ~(DELTA_FD|DELTA_FREE);
				op |= DELTA_BIO;
				dp->bp = ip ? ip : ap;
			}
			else if (bp = getbuffer(dp->fd))
			{
				op &= ~(DELTA_FD|DELTA_FREE);
				op |= DELTA_BUFFER;
				dp->vd.data = bp->base;
				bufferclash = 1;
			}
#if DEBUG
			else if (mp) sfprintf(mp, " fd=%d", dp->fd);
#endif
		}
		else if (op & DELTA_TEMP)
		{
			dp->pfd = va_arg(vp, int*);
			op |= DELTA_FD;
			op &= ~DELTA_FREE;
#if DEBUG
			if (mp) sfprintf(mp, " TEMP");
#endif
		}
		else if (op & DELTA_BUFFER)
		{
			dp->vd.data = va_arg(vp, char*);
#if DEBUG
			if (mp) sfprintf(mp, " buffer=%p", dp->vd.data);
#endif
		}
		if (op & DELTA_OFFSET)
		{
			dp->base = va_arg(vp, off_t);
#if DEBUG
			if (mp) sfprintf(mp, " offset=%I*d", sizeof(dp->base), dp->base);
#endif
		}
		if (op & DELTA_SIZE)
		{
			dp->vd.size = va_arg(vp, off_t);
#if DEBUG
			if (mp) sfprintf(mp, " size=%I*d", sizeof(dp->vd.size), dp->vd.size);
#endif
		}
		if ((op & (DELTA_BIO|DELTA_OUTPUT)) == DELTA_BIO && dp->vd.size > 0 && dp->vd.size <= state.buffersize && (dp->vd.data = bget(dp->bp, dp->vd.size, NiL)))
		{
			op &= ~(DELTA_BIO|DELTA_FREE);
			op |= DELTA_BUFFER;
#if DEBUG
			if (mp) sfprintf(mp, " buffer=%p", dp->vd.data);
#endif
		}
		if (op & (DELTA_BIO|DELTA_FD))
		{
			if (op & DELTA_OUTPUT)
			{
				dp->vd.writef = delwrite;
				if (!hole && (op & (DELTA_FD|DELTA_TEMP)) == DELTA_FD)
				{
					hole = 1;
					op |= DELTA_HOLE;
					holeinit(dp->fd);
				}
			}
			else dp->vd.readf = delread;
			if ((op & (DELTA_FD|DELTA_TEMP)) == DELTA_FD && dp->base && lseek(dp->fd, dp->base, SEEK_SET) != dp->base)
				error(3, "%s: cannot seek delta", f->name);
#if DEBUG
			if (mp && (op & DELTA_BIO)) sfprintf(mp, " bio=%s", dp->bp->name);
#endif
		}
		if (op & DELTA_OUTPUT)
		{
			if (gen) error(PANIC, "paxdelta(): more than one DELTA_OUTPUT");
			gen = dp;
#if DEBUG
			if (mp) sfprintf(mp, " OUTPUT");
#endif
		}
#if DEBUG
		if (mp && (op & DELTA_COUNT)) sfprintf(mp, " COUNT");
#endif
#if DEBUG
		if (mp) sfprintf(mp, " ]");
#endif
		dp->op = op;
		op = va_arg(vp, int);
	}
	va_end(vp);
	if (!gen)
		error(PANIC, "paxdelta(): no DELTA_OUTPUT");
#if 0
	fp = (gen == &data[DELTA_DEL]) ? ap->delta->format : ip->delta->format;
#endif
	if (gen->pfd)
	{
		if (!(state.test & 0000020) && fp->variant != DELTA_88 && (state.buffer[bufferclash ? (state.delta.bufferindex = !state.delta.bufferindex) : state.delta.bufferindex].base || (state.buffer[state.delta.bufferindex].base = newof(0, char, state.delta.buffersize, 0)) || (state.delta.buffersize >>= 1) && (state.buffer[state.delta.bufferindex].base = newof(0, char, state.delta.buffersize, 0))))
		{
			gen->fd = setbuffer(state.delta.bufferindex);
			bp = getbuffer(gen->fd);
			bp->next = bp->base;
			bp->past = bp->base + state.delta.buffersize;
		}
		else if ((gen->fd = open(state.tmp.file, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, S_IRUSR)) < 0)
			error(3, "%s: cannot create delta temporary file", state.tmp.file);
	}
	switch (fp->variant)
	{
	case DELTA_88:
	case DELTA_PATCH:
		/*
		 * force the new interface into the old
		 * not the most efficient way but at least
		 * the rest of the code is clean
		 */

		{
			static char*	tmp;

			if (gen == &data[DELTA_DEL])
			{
				if (!(data[DELTA_DEL].op & DELTA_FD))
					error(PANIC, "paxdelta: %s %s must be DELTA_TEMP or DELTA_FD", fp->name, dataname[DELTA_DEL]);
				for (op = 0; op < elementsof(data); op++)
				{
					if (op == DELTA_DEL) /*NOP*/;
					else if (!data[op].vd.size)
					{
						data[op].vd.data = state.tmp.buffer;
						data[op].op &= ~DELTA_FREE;
					}
					else switch (data[op].op & (DELTA_BIO|DELTA_FD|DELTA_OUTPUT))
					{
					case DELTA_BIO:
						if (!(data[op].vd.data = malloc(data[op].vd.size)))
							nospace();
						if (bread(data[op].bp, data[op].vd.data, data[op].vd.size, data[op].vd.size, 1) != data[op].vd.size)
							error(3, "%s: delta bread error", f->name);
						data[op].op &= ~DELTA_BIO;
						data[op].op |= DELTA_BUFFER|DELTA_FREE;
						break;
					case DELTA_FD:
						if (!(data[op].vd.data = malloc(data[op].vd.size)))
							nospace();
						if (data[op].base && lseek(data[op].fd, data[op].base, SEEK_SET) != data[op].base)
							error(3, "%s: delta seek error", f->name);
						if (read(data[op].fd, data[op].vd.data, data[op].vd.size) != data[op].vd.size)
							error(3, "%s: delta read error", f->name);
						if (data[op].op & DELTA_FREE)
							close(data[op].fd);
						data[op].op &= ~DELTA_FD;
						data[op].op |= DELTA_BUFFER|DELTA_FREE;
						break;
					}
				}
				switch (fp->variant)
				{
				case DELTA_88:
					n = delta(data[DELTA_SRC].vd.data, data[DELTA_SRC].vd.size, data[DELTA_TAR].vd.data, data[DELTA_TAR].vd.size, data[DELTA_DEL].fd) ? -1L : lseek(data[DELTA_DEL].fd, (off_t)0, SEEK_END);
					break;
				case DELTA_PATCH:
					error(1, "AHA %s %s/%s SRC=%d:%p TAR=%d:%p DEL=%d:%p", fp->name, ap->name, f->path, data[DELTA_SRC].vd.size, data[DELTA_SRC].vd.data, data[DELTA_TAR].vd.size, data[DELTA_TAR].vd.data, data[DELTA_DEL].vd.size, data[DELTA_DEL].vd.data);
					n = 0;
					break;
				}
			}
			else
			{
				if (!(data[DELTA_TAR].op & DELTA_FD))
					error(PANIC, "paxdelta: %s %s must be DELTA_TEMP or DELTA_FD", fp->name, dataname[DELTA_TAR]);
				for (op = 0; op < elementsof(data); op++)
				{
					if (op == DELTA_TAR) /*NOP*/;
					else if (!data[op].vd.size)
					{
						data[op].fd = 0;
						data[op].op &= ~DELTA_FREE;
					}
					else switch (data[op].op & (DELTA_BIO|DELTA_BUFFER|DELTA_OUTPUT))
					{
					case DELTA_BIO:
						if (!(data[op].vd.data = malloc(data[op].vd.size)))
							nospace();
						if (bread(data[op].bp, data[op].vd.data, data[op].vd.size, data[op].vd.size, 1) != data[op].vd.size)
							error(3, "%s: delta bread error", f->name);
						/*FALLTHROUGH*/
					case DELTA_BUFFER:
						if (!tmp) tmp = pathtemp(NiL, 0, NiL, error_info.id, NiL);
						if ((data[op].fd = open(tmp, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, S_IRUSR)) < 0)
							error(3, "%s: cannot create delta temporary file", tmp);
						if (write(data[op].fd, data[op].vd.data, data[op].vd.size) != data[op].vd.size)
							error(ERROR_SYSTEM|3, "%s: delta write error", f->name);
						close(data[op].fd);
						if ((data[op].fd = open(tmp, O_RDONLY|O_BINARY)) < 0)
							error(ERROR_SYSTEM|3, "%s: cannot read delta temporary file", tmp);
						if (remove(tmp))
							error(ERROR_SYSTEM|1, "%s: cannot remove delta temporary file", tmp);
						if (data[op].op & DELTA_BUFFER)
						{
							if (data[op].op & DELTA_FREE)
								free(data[op].vd.data);
							data[op].vd.data = 0;
						}
						data[op].op &= ~(DELTA_BIO|DELTA_BUFFER);
						data[op].op |= DELTA_FD|DELTA_FREE;
						break;
					}
				}
				switch (fp->variant)
				{
				case DELTA_88:
					n = update(data[DELTA_SRC].fd, data[DELTA_SRC].base, data[DELTA_DEL].fd, data[DELTA_TAR].fd) ? -1L : lseek(data[DELTA_TAR].fd, (off_t)0, SEEK_END);
					break;
				case DELTA_PATCH:
					error(PANIC, "paxdelta: %s update not supported", fp->name);
					break;
				}
			}
		}
		break;
	default:
		if (gen != &data[DELTA_DEL] && !data[DELTA_SRC].vd.size && !data[DELTA_DEL].vd.size)
			n = 0;
		else if (gen == &data[DELTA_DEL])
		{
			n = vddelta((Vddisc_t*)&data[DELTA_SRC], (Vddisc_t*)&data[DELTA_TAR], (Vddisc_t*)&data[DELTA_DEL]);
			if (n == 15)
				f->delta.same = 1;
		}
		else
			n = vdupdate((Vddisc_t*)&data[DELTA_SRC], (Vddisc_t*)&data[DELTA_TAR], (Vddisc_t*)&data[DELTA_DEL]);
		break;
	}
#if DEBUG
	if (mp)
	{
		message((-5, "%s %s: %s:%s return=%ld", gen == &data[DELTA_DEL] ? "delta" : "update", fp->name, f->name, sfstruse(mp), n));
		sfstrclose(mp);
	}
#endif
	if (n < 0)
	{
		error(ERROR_SYSTEM|2, "%s: delta error", f->name);
		return -1;
	}
	f->uncompressed = f->st->st_size;
	f->st->st_size = n;
	for (op = 0; op < elementsof(data); op++)
	{
		if (data[op].op & DELTA_HOLE)
			holedone(data[op].fd);
		switch (data[op].op & (DELTA_BUFFER|DELTA_FD|DELTA_FREE))
		{
		case DELTA_BUFFER|DELTA_FREE:
			free(data[op].vd.data);
			break;
		case DELTA_FD|DELTA_FREE:
			if ((data[op].op & (DELTA_TEMP|DELTA_OUTPUT)) == DELTA_OUTPUT)
				closeout(ap, f, data[op].fd);
			else
				close(data[op].fd);
			if (data[op].vd.data)
				free(data[op].vd.data);
			break;
		}
		if (data[op].op & DELTA_COUNT)
		{
			ap->io->expand += n;
			if (data[op].op & DELTA_OUTPUT)
				setfile(ap, f);
		}
		if (data[op].op & DELTA_LIST)
			listentry(f);
	}
	if (gen->pfd)
	{
		if (bp = getbuffer(gen->fd))
		{
			*gen->pfd = gen->fd;
			bp->next = bp->base;
		}
		else
		{
			close(gen->fd);
			if ((*gen->pfd = open(state.tmp.file, O_RDONLY|O_BINARY)) < 0)
				error(ERROR_SYSTEM|3, "%s: cannot read delta temporary file", state.tmp.file);
			if (remove(state.tmp.file))
				error(ERROR_SYSTEM|1, "%s: cannot remove delta temporary file", state.tmp.file);
		}
	}
	return 0;
}
