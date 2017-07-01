/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2013 AT&T Intellectual Property          *
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
 * partitioned fixed record zip implementation
 */

#include "pzlib.h"

#include <ls.h>

Pzstate_t	state = { "libpz:pz" };

/*
 * find and open file with optional suffix and sfio mode
 */

Sfio_t*
pzfind(Pz_t* pz, const char* file, const char* suffix, const char* mode)
{
	Sfio_t*		sp;
	char		buf[PATH_MAX];

	if (!(sp = sfopen(NiL, file, mode)))
	{
		if (!pathfind(file, pz->disc->lib ? pz->disc->lib : pz->id, suffix, buf, sizeof(buf)))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: cannot find %s file", file, suffix ? suffix : "data");
			return 0;
		}
		if (!(sp = sfopen(NiL, buf, mode)))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: cannot read", buf);
			return 0;
		}
	}
	return sp;
}

/*
 * open a new pz stream
 */

Pz_t*
pzopen(Pzdisc_t* disc, const char* path, unsigned long flags)
{
	register Pz_t*	pz;
	Vmalloc_t*	vm;
	struct stat	st;

	if (flags & PZ_AGAIN)
	{
		pz = (Pz_t*)path;
		pz->mainpart = 0;
		pz->flags &= ~PZ_HEAD;
	}
	else
	{
		if (!(vm = vmopen(Vmdcheap, Vmlast, 0)))
			return 0;
		if (!(pz = vmnewof(vm, 0, Pz_t, 1, 0)) || !(pz->tmp = sfstropen()) || !(pz->str = sfstropen()) || (flags & PZ_WRITE) && !(pz->det = sfstropen()))
			goto bad;
		memfatal(NiL);
		tvgettime(&pz->start);
		pz->major = PZ_MAJOR;
		pz->minor = PZ_MINOR;
		pz->id = state.id;
		pz->disc = disc;
		pz->flags = flags|PZ_MAINONLY;
		pz->vm = vm;
		pz->win = disc->window ? disc->window : PZ_WINDOW;
		if (!path)
		{
			if (!isatty(sffileno(sfstdin)))
			{
				pz->path = "/dev/stdin";
				if (!(pz->io = sfnew(NiL, NiL, SF_UNBOUND, sffileno(sfstdin), SF_READ)))
				{
					if (disc->errorf)
						(*disc->errorf)(pz, disc, 2, "%s: cannot read", pz->path);
					goto bad;
				}
			}
			else if (!(pz->io = sfopen(NiL, pz->path = "/dev/null", "r")))
			{
				if (disc->errorf)
					(*disc->errorf)(pz, disc, 2, "%s: cannot read", path);
				goto bad;
			}
			else
				pz->flags |= PZ_STREAM;
		}
		else if (pz->flags & PZ_STREAM)
		{
			pz->path = "stream";
			pz->io = (Sfio_t*)path;
		}
		else if (!(pz->io = strneq(path, "pipe://", 7) ? sfpopen(NiL, path + 7, "r") : sfopen(NiL, path, "r")))
		{
			if (disc->errorf)
				(*disc->errorf)(pz, disc, 2, "%s: cannot read", path);
			goto bad;
		}
		else if (!(pz->path = vmstrdup(vm, path)))
			goto bad;

		/*
		 * without this large .pz and .gz terminate short of EOF
		 */

		if (!(pz->test & 0x00001000))
			sfsetbuf(pz->io, (void*)pz->io, SF_UNBOUND);
		if (pz->flags & PZ_DIO)
			sfdcdio(pz->io, 0);

		/*
		 * there is an sfio discipline bug (probably usage,
		 * not implementation) for piped input; the S_ISFIFO
		 * test avoids the bug until the root problem is fixed
		 */

		if (!fstat(sffileno(pz->io), &st) && S_ISFIFO(st.st_mode))
			pz->flags &= ~PZ_CRC;
		else if (!(pz->flags & PZ_POP) && (!(pz->flags & PZ_WRITE) && sfdcgzip(pz->io, (pz->flags & PZ_CRC) ? 0 : SFGZ_NOCRC) > 0 || (pz->flags & (PZ_PUSHED|PZ_WRITE)) == PZ_WRITE && sfdcpzip((Sfio_t*)pz, pz->path, pz->flags|PZ_FORCE|PZ_PUSHED|PZ_HANDLE, disc) > 0))
			pz->flags |= PZ_POP;
		else
			pz->flags &= ~PZ_CRC;
		pz->oip = pz->io;
		if (disc->initf && !(*disc->initf)(pz, disc))
			goto bad;
		if (disc->options && (!(pz->options = vmstrdup(pz->vm, disc->options)) || pzoptions(pz, NiL, pz->options, 0) < 0))
			goto bad;
	}
	if ((pz->flags & (PZ_READ|PZ_SPLIT|PZ_STAT)) || !disc->partition)
	{
		if (pzheadread(pz))
			goto bad;
		if ((pz->flags & PZ_SPLIT) && disc->partition && pzpartition(pz, disc->partition))
			goto bad;
		if (pz->options && pzoptions(pz, pz->part, pz->options, 1) < 0)
			goto bad;
	}
	else
	{
		if (disc->partition && pzpartition(pz, disc->partition))
			goto bad;
		if (pz->disc->version >= PZ_VERSION_SPLIT && pz->disc->splitf)
		{
			pz->split.flags |= PZ_SPLIT_DEFLATE;
			pz->flags |= PZ_ACCEPT|PZ_SECTION;
			pz->flags &= ~PZ_SORT;
		}
		if (pz->options && pzoptions(pz, pz->part, pz->options, 1) < 0)
			goto bad;
		if ((pz->flags & (PZ_NOPZIP|PZ_UNKNOWN)) == PZ_UNKNOWN && !pz->disc->readf && !pz->disc->splitf)
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: unknown input format", pz->path);
			goto bad;
		}
	}

	/*
	 * ready to process
	 */

	pz->part = pz->mainpart;
	if (pz->disc->eventf && (*pz->disc->eventf)(pz, (flags & PZ_AGAIN) ? PZ_REOPEN : PZ_OPEN, NiL, 0, pz->disc) < 0)
		goto bad;
	if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, -1, "%s: pzopen: flags=%08x win=(%I*u:%I*u) row=(%I*u:%I*u) buf=%p wrk=%p", pz->path, pz->flags, sizeof(pz->win), pz->win, sizeof(pz->mwin), pz->mwin, sizeof(pz->row), pz->row, sizeof(pz->mrow), pz->mrow, pz->buf, pz->wrk);
	return pz;
 bad:
 	pzclose(pz);
	return 0;
}

/*
 * close an open pz stream
 */

int
pzclose(register Pz_t* pz)
{
	int		r;
	unsigned long	e;
	Sfoff_t		p;
	Sfio_t*		op;
	Tv_t		now;

	if (!pz)
		return -1;
	if (!(r = pzsync(pz)) && (op = pz->ws.io))
	{
		/*
		 * finish up the pzwrite() stream
		 */

		sfputu(op, 0);
		if (pz->ws.pc)
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 1, "last record incomplete %u/%u", pz->ws.pc, pz->row);
			sfputc(op, PZ_MARK_PART);
			sfputu(op, pz->ws.pc);
			sfwrite(op, pz->ws.pb, pz->ws.pc);
		}
		if (!(pz->flags & PZ_SECTION))
		{
			sfputc(op, PZ_MARK_TAIL);
			if (pz->disc->eventf && (*pz->disc->eventf)(pz, PZ_TAILWRITE, op, 0, pz->disc) < 0)
				return -1;
			sfputc(op, 0);
		}
		if (sfsync(op))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "write error");
			return -1;
		}
	}
	if ((pz->flags & PZ_WRITE) && (pz->flags & (PZ_DUMP|PZ_VERBOSE|PZ_SUMMARY)) && pz->disc->errorf && pz->count.records)
	{
		if (!pz->count.uncompressed)
			pz->count.uncompressed = pz->part->row * pz->count.records;
		if (!pz->count.compressed)
		{
			pz->count.compressed = pz->count.uncompressed;
			if (pz->oop)
			{
				if (!(pz->flags & PZ_NOGZIP))
					while (sfdisc(pz->oop, SF_POPDISC));
				if ((p = sfsize(pz->oop)) > 0)
					pz->count.compressed = p;
			}
		}
		if (pz->flags & PZ_REGRESS)
			e = 100;
		else
		{
			tvgettime(&now);
			if (!(e = ((unsigned long)now.tv_sec - (unsigned long)pz->start.tv_sec) * 100 + ((long)now.tv_nsec - (long)pz->start.tv_nsec) / 10000000L))
				e = 1;
		}
		sfprintf(pz->tmp, _("total %s rate %.2f time %s bpr %.2f bps %s size %I*u/%I*u windows %I*u")
			, pz->path
			, (double)((Sflong_t)pz->count.uncompressed) / (double)((Sflong_t)pz->count.compressed)
			, fmtelapsed(e, 100)
			, (double)((Sflong_t)pz->count.compressed) / (double)((Sflong_t)pz->count.records)
			, fmtnum((pz->count.uncompressed * 100) / e, 0)
			, sizeof(pz->count.uncompressed)
			, pz->count.uncompressed
			, sizeof(pz->count.compressed)
			, pz->count.compressed
			, sizeof(pz->count.windows)
			, pz->count.windows
				);
		if (pz->count.sections)
			sfprintf(pz->tmp, _(" sections %I*u")
				, sizeof(pz->count.sections)
				, pz->count.sections
					);
		sfprintf(pz->tmp, _(" records %I*u")
			, sizeof(pz->count.records)
			, pz->count.records
				);
		if (pz->count.modules)
			sfprintf(pz->tmp, _(" modules %I*u")
				, sizeof(pz->count.modules)
				, pz->count.modules
					);
		if (pz->count.converted)
			sfprintf(pz->tmp, _(" converted %I*u")
				, sizeof(pz->count.converted)
				, pz->count.converted
					);
		if (pz->count.repaired)
			sfprintf(pz->tmp, _(" repaired %I*u")
				, sizeof(pz->count.repaired)
				, pz->count.repaired
					);
		if (pz->count.truncated)
			sfprintf(pz->tmp, _(" truncated %I*u")
				, sizeof(pz->count.truncated)
				, pz->count.truncated
					);
		if (pz->count.dropped)
			sfprintf(pz->tmp, _(" dropped %I*u")
				, sizeof(pz->count.dropped)
				, pz->count.dropped
					);
		(*pz->disc->errorf)(pz, pz->disc, 0, "%s", sfstruse(pz->tmp));
	}
	if (pz->io)
	{
		if (pz->oip)
		{
			if (pz->disc->eventf)
				r = (*pz->disc->eventf)(pz, PZ_CLOSE, NiL, 0, pz->disc);
			if (!(pz->flags & PZ_STREAM))
			{
				if (sfclose(pz->oip) < 0)
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: data corrupted (close error)", pz->path);
					r = -1;
				}
			}
			else if (pz->flags & PZ_POP)
				sfdisc(pz->oip, SF_POPDISC);
			pz->oip = 0;
		}
		else if (pz->flags & PZ_POP)
		{
			sfdisc(pz->io, SF_POPDISC);
			if (sferror(pz->io))
			{
				pz->flags |= PZ_ERROR;
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, (pz->flags & PZ_WRITE) ? "write error" : "read error");
			}
		}
	}
	if (pz->flags & PZ_ERROR)
		r = -1;
	if (pz->partdict)
		dtclose(pz->partdict);
	if (pz->vm)
		vmclose(pz->vm);
	if (pz->tmp)
		sfclose(pz->tmp);
	if (pz->str)
		sfclose(pz->str);
	if (pz->det)
		sfclose(pz->det);
	return r;
}

/*
 * common out of space message
 */

int
pznospace(Pz_t* pz)
{
	if (pz->disc->errorf)
		(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "out of space");
	return -1;
}

