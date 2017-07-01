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
 * pax archive format support
 */

#include "format.h"

Format_t*		formats = pax_first_format;

/*
 * read archive prologue before files are copied
 * compression and input archive formats are identified here
 */

int
getprologue(register Archive_t* ap)
{
	register Format_t*	fp;
	register File_t*	f = &ap->file;
	off_t			skipped;
	int			n;
	unsigned char		buf[MAXUNREAD];
	unsigned char		cvt[MAXUNREAD];

	message((-6, "getprologue() volume=%d eof=%d", ap->volume, ap->io->eof));
	if (ap->io->eof || state.volume && (ap->io->mode & O_ACCMODE) == O_WRONLY)
		return 0;
	state.volume[0] = 0;
	ap->entry = 0;
	ap->format = 0;
	ap->swapio = 0;
	ap->io->offset += ap->io->count;
	ap->io->count = 0;
	ap->section = SECTION_CONTROL;
	convert(ap, SECTION_CONTROL, CC_NATIVE, CC_NATIVE);
	ap->sum++;
	f->name = 0;
	f->record.format = 0;
	f->skip = 0;
	ap->checkdelta = !ap->delta || ap->delta->format && !(ap->delta->format->flags & PSEUDO);
	f->delta.base = 0;
	f->uncompressed = 0;
	f->delta.checksum = 0;
	f->delta.index = 0;
	if (state.append || state.update)
		bsave(ap);

	/*
	 * first check if input is compressed
	 */

	if ((n = bread(ap, (char*)buf, (off_t)0, (off_t)sizeof(buf), 0)) <= MINID && !bcount(ap))
	{
		if (n && ap->volume <= 0)
			goto unknown;
		return 0;
	}
	bunread(ap, buf, n);
	if (CC_NATIVE != CC_ASCII)
		ccmapm(cvt, buf, sizeof(cvt), CC_ASCII, CC_NATIVE);
	message((-2, "identify format"));
	if (ap->volume <= 0 && !ap->compress)
	{
		fp = 0;
		while (fp = nextformat(fp))
			if ((fp->flags & COMPRESS) && (*fp->getprologue)(&state, fp, ap, f, buf, sizeof(buf)) > 0)
			{
				Compress_format_t*	cp;
				Proc_t*			proc;
				long			ops[3];
				char*			cmd[3];

				message((-1, "%s compression", fp->name));
				ap->compress = fp;
				cp = (Compress_format_t*)fp->data;
				if (bseek(ap, (off_t)0, SEEK_SET, 1))
					error(3, "%s: %s input must be seekable", ap->name, ap->compress->name);
				undoable(ap, ap->compress);
				if (state.meter.on && ap == state.in && ap->uncompressed)
					state.meter.size = ap->uncompressed;
				cmd[0] = cp->undo[0];
				cmd[1] = cp->undo[1];
				cmd[2] = 0;
				ops[0] = PROC_FD_DUP(ap->io->fd, 0, PROC_FD_PARENT|PROC_FD_CHILD);
				if (ap->parent && !state.ordered)
				{
					if ((n = open(state.tmp.file, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, S_IRUSR)) < 0)
						error(ERROR_SYSTEM|3, "%s: cannot create %s base temporary file %s", ap->name, cp->undo[0], state.tmp.file);
					ops[1] = PROC_FD_DUP(n, 1, PROC_FD_PARENT|PROC_FD_CHILD);
					ops[2] = 0;
					proc = procopen(*cmd, cmd, NiL, ops, 0);
				}
				else
				{
					ops[1] = 0;
					proc = procopen(*cmd, cmd, NiL, ops, PROC_READ);
				}
				if (!proc)
					error(3, "%s: cannot execute %s filter", ap->name, cp->undo[0]);
				if (ap->parent && !state.ordered)
				{
					if (n = procclose(proc))
						error(3, "%s: %s filter exit code %d", ap->name, cp->undo[0], n);
					if ((ap->io->fd = open(state.tmp.file, O_RDONLY|O_BINARY)) < 0)
						error(ERROR_SYSTEM|3, "%s: cannot read %s base temporary file %s", ap->name, cp->undo[0], state.tmp.file);
					if (remove(state.tmp.file))
						error(ERROR_SYSTEM|1, "%s: cannot remove %s base temporary file %s", ap->name, cp->undo[0], state.tmp.file);
				}
				else
				{
					List_t*	p;

					ap->io->fd = proc->rfd;
					if (!(p = newof(0, List_t, 1, 0)))
						nospace();
					p->item = (void*)proc;
					p->next = state.proc;
					state.proc = p;
				}
				if ((n = bread(ap, (char*)buf, (off_t)0, (off_t)sizeof(buf), 0)) <= MINID && !bcount(ap))
					return 0;
				bunread(ap, buf, n);
				break;
			}
	}

	/*
	 * now identify the format
	 */

	if (!(fp = ap->expected))
	{
		skipped = 0;
		ap->entry = 1;
		for (;;)
		{
			fp = 0;
			while (fp = nextformat(fp))
				if ((fp->flags & ARCHIVE) && fp->getprologue)
				{
					message((-2, "check %s", fp->name));
					convert(ap, SECTION_CONTROL, (fp->flags & CONV) ? CC_NATIVE : CC_ASCII, CC_NATIVE);
					if ((n = (*fp->getprologue)(&state, fp, ap, f, ((fp->flags & CONV) || CC_NATIVE == CC_ASCII) ? buf : cvt, sizeof(buf))) < 0)
						return 0;
					if (n > 0)
						break;
					if (ap->data)
					{
						error(ERROR_PANIC|4, "%s: data!=0 on failed getprologue()", fp->name);
						ap->data = 0;
					}
					ap->checksum = ap->old.checksum = ap->memsum = ap->old.memsum = 0;
				}
			if (fp)
				break;
			convert(ap, SECTION_CONTROL, CC_NATIVE, CC_NATIVE);
			if (!state.keepgoing || ap->io->eof || bread(ap, buf, (off_t)0, (off_t)1, 0) < 1)
			{
			unknown:
				if (ap->expected)
					error(3, "%s: unknown input format -- %s expected", ap->name, ap->expected->name);
				if (ap->volume)
				{
					error(1, "%s: junk data after volume %d", ap->name, ap->volume);
					return 0;
				}
				error(3, "%s: unknown input format", ap->name);
			}
			skipped++;
		}
		ap->entry = 0;
		if (skipped)
			error(1, "%s: %I*d byte%s skipped before identifying %s format archive", ap->name, sizeof(skipped), skipped, skipped == 1 ? "" : "s", fp->name);
	}
	else if (!(fp->flags & ARCHIVE) || !fp->getprologue  || (*fp->getprologue)(&state, fp, ap, f, buf, sizeof(buf)) <= 0)
		error(3, "%s: %s: archive format mismatch", ap->name, fp->name);
	if (!ap->format)
		ap->format = fp;
	if (!ap->type)
		ap->type = ap->format->name;
	message((-1, "%s format", ap->type));
	if ((state.operation & IN) && !state.list && !(ap->format->flags & IN))
		error(3, "%s: %s read not implemented", ap->name, ap->type);
	if ((state.operation & OUT) && !(ap->format->flags & OUT))
		error(3, "%s: %s write not implemented", ap->name, ap->type);
	ap->flags = state.operation | (ap->format->flags & ~(IN|OUT));
	ap->sum--;
	ap->volume++;
	if (ap->peek && (ap->checkdelta || ap->delta) && deltacheck(ap, f))
		ap->peek = 0;
	if (state.summary && state.verbose && ap->volume > 0)
	{
		if (ap->io->blok)
			sfprintf(sfstderr, "BLOK ");
		if (ap->parent)
			sfprintf(sfstderr, "%s base %s", ap->parent->name, ap->name);
		else
		{
			sfprintf(sfstderr, "%s", ap->name);
			if (ap->volume > 1)
				sfprintf(sfstderr, " volume %d", ap->volume);
		}
		if (state.volume[0])
			sfprintf(sfstderr, " label %s", state.volume);
		sfprintf(sfstderr, " in");
		if (ap->package)
			sfprintf(sfstderr, " %s", ap->package);
		if (ap->compress)
			sfprintf(sfstderr, " %s", ap->compress->name);
		if (ap->delta && ap->delta->format)
			sfprintf(sfstderr, " %s", ap->delta->format->name);
		sfprintf(sfstderr, " %s format", ap->type);
		if (ap->swapio)
			sfprintf(sfstderr, " %s swapped", "unix\0nuxi\0ixun\0xinu" + 5 * ap->swapio);
		sfprintf(sfstderr, "\n");
	}
	if (ap->volume > 1)
	{
		if (ap->delta)
		{
			if (state.operation == (IN|OUT) || !(ap->delta->format->flags & DELTA))
				error(3, "%s: %s archive cannot be multi-volume", ap->name, ap->parent ? "base" : "delta");
			ap->delta = 0;
		}

		/*
		 * no hard links between volumes
		 */

		hashfree(state.linktab);
		if (!(state.linktab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(Fileid_t), HASH_name, "links", 0)))
			error(3, "cannot re-allocate hard link table");
	}
	return 1;
}

/*
 * set pseudo file header+trailer info
 */

void
setinfo(register Archive_t* ap, register File_t* f)
{
	long	n;

	if (ap->delta && ap->delta->format)
	{
		if (ap->delta->format->variant != DELTA_IGNORE && ap->entry > 1 && f->st->st_mtime)
		{
			if ((n = f->st->st_mtime - ap->delta->index) < 0)
				error(3, "%s: corrupt archive: %d extra file%s", ap->name, -n, n == -1 ? "" : "s");
			else if (n > 0)
				error(3, "%s: corrupt archive: %d missing file%s", ap->name, n, n == 1 ? "" : "s");
		}
		ap->delta->epilogue = 1;
	}
}

/*
 * output pseudo file header+trailer
 */

void
putinfo(register Archive_t* ap, char* file, unsigned long mtime, unsigned long checksum)
{
	register File_t*	f = &ap->file;
	Sfio_t*			np = 0;
	Delta_format_t*		dp;

	if (!file)
	{
		if (!(np = sfstropen()))
			nospace();
		if (!ap->delta || ap->delta->format->variant == DELTA_88)
			sfprintf(np, "DELTA");
		else
		{
			sfprintf(np, "%c%s%c%c%c%s", INFO_SEP, ID, INFO_SEP, ap->delta->compress ? TYPE_COMPRESS : TYPE_DELTA, INFO_SEP, (dp = (Delta_format_t*)ap->delta->format->data) ? dp->variant : "");
			if (state.ordered)
				sfprintf(np, "%c%c", INFO_SEP, INFO_ORDERED);
		}
		sfprintf(np, "%c%c%c", INFO_SEP, INFO_SEP, INFO_SEP);
		if (!(file = sfstruse(np)))
			nospace();
	}
	initfile(ap, f, f->st, file, X_IFREG);
	f->skip = 1;
	f->st->st_mtime = mtime;
	f->st->st_uid = DELTA_LO(checksum);
	f->st->st_gid = DELTA_HI(checksum);
	if (putheader(ap, f))
		puttrailer(ap, f);
	if (np)
		sfstrclose(np);
}

/*
 * write archive prologue before files are copied
 */

void
putprologue(register Archive_t* ap, int append)
{
	message((-6, "putprologue()"));
	ap->section = SECTION_CONTROL;
	if (ap->delta && ap->delta->format->variant == DELTA_88)
		ap->checksum = ap->old.checksum;
	if (!(ap->format->flags & CONV))
	{
		convert(ap, SECTION_CONTROL, CC_NATIVE, CC_ASCII);
		if (!ap->convert[0].on)
			convert(ap, SECTION_DATA, CC_NATIVE, CC_NATIVE);
	}
	if ((!ap->format->putprologue || (*ap->format->putprologue)(&state, ap, append) >= 0) && !(ap->format->flags & DELTAINFO) && ap->delta && !(ap->delta->format->flags & PSEUDO))
	{
		if (ap->delta->base)
			putinfo(ap, NiL, ap->delta->base->size, ap->delta->base->checksum);
		else
			putinfo(ap, NiL, 0, 0);
	}
}

/*
 * read archive epilogue after all files have been copied
 */

int
getepilogue(register Archive_t* ap)
{
	register char*	s;
	register off_t	n;
	register int	i;
	unsigned int	z;
	int		x;
	char		buf[BLOCKSIZE];

	message((-6, "getepilogue()"));
	ap->section = SECTION_CONTROL;
	state.updated = ap->updated;
	if (ap->delta && ap->delta->epilogue < 0)
		error(3, "%s: corrupt archive: missing epilogue", ap->name);
	if (state.append || state.update && (ap->io->mode & O_RDWR))
	{
		backup(ap);
		return 0;
	}
	if (!ap->format->getepilogue || !(*ap->format->getepilogue)(&state, ap))
	{
		/*
		 * check for more volumes
		 * volumes begin on BLOCKSIZE boundaries
		 * separated by null byte filler
		 */

		if (ap->io->keep)
		{
			bskip(ap);
			if (ap->io->eof)
				ap->io->keep = 0;
			else if (ap->io->keep > 0)
				ap->io->keep--;
			goto done;
		}
		x = 0;
		z = 0;
		i = 0;
		if (!(n = roundof(ap->io->count, BLOCKSIZE) - ap->io->count) || bread(ap, buf, (off_t)0, (off_t)n, 0) > 0)
			do
			{
				for (s = buf; s < buf + n && !*s; s++);
				z += s - buf;
				if (z >= BLOCKSIZE)
					x = 1;
				if (s < buf + n)
				{
					if (n == BLOCKSIZE)
					{
						if (!x && ap->format->event && (ap->format->events & PAX_EVENT_SKIP_JUNK))
						{
							if ((*ap->format->event)(&state, ap, NiL, buf, PAX_EVENT_SKIP_JUNK) > 0)
								continue;
							if (i)
								error(1, "%s: %d junk block%s after volume %d", ap->name, i, i == 1 ? "" : "s", ap->volume);
						}
						bunread(ap, buf, BLOCKSIZE);
						goto done;
					}
					if (ap->volume > 1)
						error(1, "junk data after volume %d", ap->volume);
					break;
				}
				n = BLOCKSIZE;
				i++;
			} while (bread(ap, buf, (off_t)0, n, 0) > 0);
		bflushin(ap, 0);
	}
 done:
	if (ap->format->done)
		(*ap->format->done)(&state, ap);
	ap->swapio = 0;
	return 1;
}

/*
 * write archive epilogue after files have been copied
 */

void
putepilogue(register Archive_t* ap)
{
	register ssize_t	n;
	register off_t		boundary;

	message((-6, "putepilogue()"));
	if (state.install.path)
	{
		if (sfclose(state.install.sp))
			error(ERROR_SYSTEM|2, "%s: install temporary write error", state.install.path);
		state.filter.line = 2;
		state.filter.name = state.install.name;
		state.filter.command = "";
		ftwalk(state.install.path, copyout, state.ftwflags, NiL);
		state.filter.line = 0;
	}
	if (state.checksum.path)
	{
		if (sfclose(state.checksum.sp))
			error(ERROR_SYSTEM|2, "%s: checksum temporary write error", state.checksum.path);
		sumclose(state.checksum.sum);
		state.checksum.sum = 0;
		state.filter.line = 2;
		state.filter.name = state.checksum.name;
		state.filter.command = "";
		ftwalk(state.checksum.path, copyout, state.ftwflags, NiL);
		state.filter.line = 0;
	}
	ap->section = SECTION_CONTROL;
	if (ap->selected > state.selected)
	{
		state.selected = ap->selected;
		if (ap->delta && (ap->delta->format->flags & DELTA))
		{
			if (ap->format->event && (ap->format->events & PAX_EVENT_DELTA_EXTEND))
				(*ap->format->event)(&state, ap, NiL, NiL, PAX_EVENT_DELTA_EXTEND);
			else
				putinfo(ap, NiL, ap->delta->index + 1, 0);
		}
		if (!ap->format->putepilogue || (boundary = (*ap->format->putepilogue)(&state, ap)) <= 0)
			boundary = ap->io->count;
		if (n = ((ap->io->count > boundary) ? roundof(ap->io->count, boundary) : boundary) - ap->io->count)
		{
			memzero(state.tmp.buffer, n);
			bwrite(ap, state.tmp.buffer, n);
		}
		bflushout(ap);
		ap->volume++;
	}
	else
	{
		ap->io->count = ap->io->offset = 0;
		ap->io->next = ap->io->buffer + ap->io->unread;
	}
	if (ap->format->done)
		(*ap->format->done)(&state, ap);
}

/*
 * get key [ug]id value
 */

static void
getkeyid(Archive_t* ap, File_t* f, int index, uid_t* ip, int d)
{
	register Option_t*	op;

	op = &options[index];
	if (op->level < 7)
	{
		if (op->entry == ap->entry)
			*ip = op->temp.number;
		else if (op->level > 0 && op->perm.string)
			*ip = op->perm.number;
	}
	else if (op->level >= 8)
		*ip = d;
}

/*
 * get key name value
 */

static void
getkeyname(Archive_t* ap, File_t* f, int index, char** sp, uid_t* ip, int d)
{
	register Option_t*	op;

	op = &options[index];
	if (op->level < 7)
	{
		if (op->entry == ap->entry)
			*sp = op->temp.string;
		else if (op->level > 0 && op->perm.string)
			*sp = op->perm.string;
	}
	else if (ip && op->level >= 8)
	{
		*sp = 0;
		*ip = d;
	}
}

/*
 * get key size value
 */

static void
getkeysize(Archive_t* ap, File_t* f, int index, off_t* zp)
{
	register Option_t*	op;

	NoP(f);
	op = &options[index];
	if (op->level < 7)
	{
		if (op->entry == ap->entry)
		{
			if (op->temp.string)
				*zp = strtoll(op->temp.string, NiL, 10);
		}
		else if (op->level > 0)
		{
			if (op->perm.string)
				*zp = strtoll(op->perm.string, NiL, 10);
		}
	}
}

/*
 * get key time value
 */

static void
getkeytime(Archive_t* ap, File_t* f, int index)
{
	register Option_t*	op;
	register Value_t*	vp;
	Tv_t			tv;

	NoP(f);
	op = &options[index];
	message((-5, "getkeytime %s level=%d entry=%d:%d", op->name, op->level, op->entry, ap->entry));
	if (op->level < 7)
	{
		if (op->entry == ap->entry)
			vp = &op->temp;
		else if (op->level > 0)
			vp = &op->perm;
		else
			return;
		tv.tv_sec = vp->number;
		tv.tv_nsec = vp->fraction;
		if (!tv.tv_sec && !tv.tv_nsec && index != OPT_mtime)
			tvgetmtime(&tv, f->st);
		switch (index)
		{
		case OPT_atime:
			tvsetatime(&tv, f->st);
			break;
		case OPT_mtime:
			tvsetmtime(&tv, f->st);
			break;
		case OPT_ctime:
			tvsetctime(&tv, f->st);
			break;
		}
	}
	else if (op->level >= 8)
	{
		tvgettime(&tv);
		vp = &op->perm;
		vp->number = tv.tv_sec;
		vp->fraction = tv.tv_nsec;
	}
}

/*
 * read next archive entry header
 */

int
getheader(register Archive_t* ap, register File_t* f)
{
	register char*	s;
	long		i;

	message((-6, "getheader()"));
	ap->section = SECTION_CONTROL;
	ap->sum++;
	ap->entry++;
	if (state.append || state.update)
		bsave(ap);
	if (!ap->peek)
	{
		f->delta.base = 0;
		f->delta.checksum = 0;
		f->delta.index = 0;
		f->uncompressed = 0;
	}
	do
	{
		if (ap->peek)
			ap->peek = 0;
		else
		{
			f->name = 0;
			f->record.format = 0;
			f->skip = 0;
			i = f->st->st_ino;
			memset(f->st, 0, sizeof(*f->st));
			f->st->st_ino = i;
			f->st->st_nlink = 1;
			if ((*ap->format->getheader)(&state, ap, f) <= 0)
			{
				ap->entry--;
				return 0;
			}
		}
		if (!f->name)
			error(3, "%s: %s format entry %d.%d name not set", ap->name, ap->type, ap->volume, ap->entry);
	} while ((ap->checkdelta || ap->delta) && deltacheck(ap, f));
	ap->entries++;
	getkeysize(ap, f, OPT_size, &f->st->st_size);
	getkeytime(ap, f, OPT_mtime);
	getkeytime(ap, f, OPT_atime);
	getkeytime(ap, f, OPT_ctime);
	getkeyid(ap, f, OPT_gid, &f->st->st_gid, state.gid);
	getkeyname(ap, f, OPT_gname, &f->gidname, &f->st->st_gid, state.gid);
	getkeyname(ap, f, OPT_path, &f->name, NiL, 0);
	getkeyname(ap, f, OPT_linkpath, &f->linkpath, NiL, 0);
	getkeyid(ap, f, OPT_uid, &f->st->st_uid, state.uid);
	getkeyname(ap, f, OPT_uname, &f->uidname, &f->st->st_uid, state.uid);
	if (!state.list)
		setidnames(f);
	if (f->name != ap->stash.head.string)
		f->name = stash(&ap->stash.head, f->name, 0);
	if (ap->flags & DOS)
		undos(f);
	f->type = X_ITYPE(f->st->st_mode);
	s = pathcanon(f->name, 0, 0);
	if (s > f->name + 1 && *--s == '/')
	{
		*s = 0;
		if ((ap->format->flags & SLASHDIR) && f->type == X_IFREG)
		{
			f->st->st_mode &= ~X_IFREG;
			f->st->st_mode |= (f->type = X_IFDIR);
			f->datasize = f->st->st_size;
		}
	}
	f->path = stash(&ap->path.name, f->name, 0);
	f->name = map(ap, f->name);
	f->namesize = strlen(f->name) + 1;
	if (f->linkpath)
	{
		pathcanon(f->linkpath, 0, 0);
		if (!(state.ftwflags & FTW_PHYSICAL))
			f->linkpath = map(ap, f->linkpath);
		f->linkpathsize = strlen(f->linkpath) + 1;
	}
	else
		f->linkpathsize = 0;
	f->perm = modei(f->st->st_mode);
	f->ro = ropath(f->name);
	getdeltaheader(ap, f);
#if DEBUG
	if (error_info.trace)
	{
		s = &state.tmp.buffer[0];
		if (f->record.format)
			sfsprintf(s, state.tmp.buffersize, " [%c,%d,%d]", f->record.format, state.blocksize, state.record.size);
		else
			*s = 0;
		message((-1, "archive=%s path=%s name=%s entry=%d.%d size=%I*u uncompressed=%I*u delta=%c%s", ap->name, f->path, f->name, ap->volume, ap->entry, sizeof(f->st->st_size), f->st->st_size, sizeof(f->uncompressed), f->uncompressed, f->delta.op ? f->delta.op : DELTA_nop, s));
	}
#endif
	if (ap->sum > 0)
	{
		if (ap->format->flags & SUM)
			ap->memsum = FNV_INIT;
		else if (!ap->delta || !ap->delta->trailer)
			ap->memsum = 0;
		ap->old.memsum = 0;
	}
	ap->section = SECTION_DATA;
	ap->convert[ap->section].on = ap->convert[ap->section].f2t != 0;
	return 1;
}

/*
 * write next archive entry header
 */

int
putheader(register Archive_t* ap, register File_t* f)
{
	register int	n;

	message((-6, "putheader()"));
	if (!f->extended)
	{
		setdeltaheader(ap, f);
		ap->entry++;
		ap->entries++;
	}
	ap->section = SECTION_CONTROL;
	if ((n = (*ap->format->putheader)(&state, ap, f)) < 0)
		return -1;
	if (!n)
	{
		if (!ap->incomplete)
			return 0;
		ap->incomplete = 0;
		if ((ap->io->count + f->st->st_size) > state.maxout)
		{
			error(2, "%s: too large to fit in one volume", f->name);
			return -1;
		}
		state.complete = 0;
		putepilogue(ap);
		newio(ap, 0, 0);
		putprologue(ap, 0);
		state.complete = 1;
		if ((n = (*ap->format->putheader)(&state, ap, f)) <= 0)
			return n;
	}
	putdeltaheader(ap, f);
	if (state.checksum.sum)
		suminit(state.checksum.sum);
	ap->section = SECTION_DATA;
	ap->convert[ap->section].on = ap->convert[ap->section].f2t != 0;
	if (state.install.sp && !f->extended)
	{
		n = 0;
		if (f->st->st_gid != state.gid && ((f->st->st_mode & S_ISGID) || (f->st->st_mode & S_IRGRP) && !(f->st->st_mode & S_IROTH) || (f->st->st_mode & S_IXGRP) && !(f->st->st_mode & S_IXOTH)))
		{
			sfprintf(state.install.sp, "chgrp %s %s\n", fmtgid(f->st->st_gid), f->name);
			n = 1;
		}
		if (f->st->st_uid != state.uid && ((f->st->st_mode & S_ISUID) || (f->st->st_mode & S_IRUSR) && !(f->st->st_mode & (S_IRGRP|S_IROTH)) || (f->st->st_mode & S_IXUSR) && !(f->st->st_mode & (S_IXGRP|S_IXOTH))))
		{
			sfprintf(state.install.sp, "chown %s %s\n", fmtuid(f->st->st_uid), f->name);
			n = 1;
		}
		if (n || (f->st->st_mode & (S_ISUID|S_ISGID)))
			sfprintf(state.install.sp, "chmod %04o %s\n", modex(f->st->st_mode & S_IPERM), f->name);
	}
	return 1;
}

/*
 * read entry trailer
 */

void
gettrailer(register Archive_t* ap, File_t* f)
{
	register off_t	n;

	message((-6, "gettrailer()"));
	NoP(f);
	ap->section = SECTION_CONTROL;
	if (ap->sum-- > 0)
	{
		ap->checksum ^= ap->memsum;
		ap->old.checksum ^= ap->old.memsum;
	}
	if (ap->format->gettrailer)
		(*ap->format->gettrailer)(&state, ap, f);
	getdeltatrailer(ap, f);
	if ((n = ap->format->align) && (n = roundof(ap->io->count, n) - ap->io->count))
		bread(ap, state.tmp.buffer, (off_t)0, n, 1);
	if (!(ap->format->flags & SUM) && ap->sum >= 0)
	{
		ap->memsum = 0;
		ap->old.memsum = 0;
	}
}

/*
 * write entry trailer
 */

void
puttrailer(register Archive_t* ap, register File_t* f)
{
	register int	n;
	char*		s;

	message((-6, "puttrailer()"));
	if (state.checksum.sum && !f->extended)
	{
		sumdone(state.checksum.sum);
		if (f->link)
		{
			if (!f->link->checksum)
			{
				sumprint(state.checksum.sum, state.tmp.str, 0, 0);
				if (!(s = sfstruse(state.tmp.str)) || !(f->link->checksum = strdup(s)))
					nospace();
			}
			sfputr(state.checksum.sp, f->link->checksum, -1);
		}
		else
			sumprint(state.checksum.sum, state.checksum.sp, 0, 0);
		sfprintf(state.checksum.sp, " %04o %s %s %s\n"
			, modex(f->st->st_mode & S_IPERM)
			, (f->st->st_uid != state.uid && ((f->st->st_mode & S_ISUID) || (f->st->st_mode & S_IRUSR) && !(f->st->st_mode & (S_IRGRP|S_IROTH)) || (f->st->st_mode & S_IXUSR) && !(f->st->st_mode & (S_IXGRP|S_IXOTH)))) ? fmtuid(f->st->st_uid) : "-"
			, (f->st->st_gid != state.gid && ((f->st->st_mode & S_ISGID) || (f->st->st_mode & S_IRGRP) && !(f->st->st_mode & S_IROTH) || (f->st->st_mode & S_IXGRP) && !(f->st->st_mode & S_IXOTH))) ? fmtgid(f->st->st_gid) : "-"
			, f->name
			);
	}
	ap->section = SECTION_CONTROL;
	putdeltatrailer(ap, f);
	if (ap->format->puttrailer)
		(*ap->format->puttrailer)(&state, ap, f);
	if ((n = ap->format->align) && (n = roundof(ap->io->count, n) - ap->io->count))
	{
		memzero(state.tmp.buffer, n);
		bwrite(ap, state.tmp.buffer, n);
	}
	listentry(f);
}
