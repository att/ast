/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2011 AT&T Intellectual Property          *
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
 * pzip split partition support
 */

#include "pzlib.h"

#include <cdt.h>

struct Deflate_s;
struct Inflate_s;
struct Id_s;

typedef struct Id_s
{
	Dtlink_t	byid;
	Dtlink_t	byseq;
	int		seq;
	int		use;
	Pzpart_t*	part;
	Sfio_t*		sp;
	unsigned long	id;
	int		row;
	size_t		size;
	size_t		count;
	char*		bp;
	char*		name;
	unsigned long	used;
	unsigned long	windows;
	Sfulong_t	modules;
	Sfulong_t	total;
} Id_t;

typedef struct Deflate_s
{
	Sfio_t*		xp;
	Dt_t*		ids;
	Pz_t*		pz;
	unsigned int	seq;
	Dt_t*		sqs;
} Deflate_t;

typedef struct Inflate_s
{
	Dt_t*		ids;
	Pz_t*		pz;
} Inflate_t;

#undef	state

static struct
{
	Sfio_t*		buf;
	char*		comment;
	unsigned long	flags;
	Sfio_t*		opt;
	Sfio_t*		tmp;
	int		total;
	int		verbose;
	size_t		window;
	Sfulong_t	windows;
	Sfulong_t	records;
	Sfulong_t	modules;
} state;

#if _ast_intswap

/*
 * order by signed int
 */

static int
byint(Dt_t* dt, void*a, void* b, Dtdisc_t* disc)
{
	return *(int*)a - *(int*)b;
}

/*
 * order by unsigned long
 */

static int
byulong(Dt_t* dt, void*a, void* b, Dtdisc_t* disc)
{
	if (*(unsigned long*)a < *(unsigned long*)b)
		return -1;
	if (*(unsigned long*)a > *(unsigned long*)b)
		return 1;
	return 0;
}

#endif

/*
 * free Id_t
 */

static void
freeid(Dt_t* dt, Void_t* ip, Dtdisc_t* disc)
{
	if (((Id_t*)ip)->sp)
		sfclose(((Id_t*)ip)->sp);
	free(ip);
}

/*
 * flush the current partition window
 * this function determines the file layout
 */

static int
flush(Deflate_t* dp, size_t w, Sfio_t* op)
{
	register Id_t*	ip;
	register size_t	n;
	Sfio_t*		io;
	char*		file;
	int		i;
	int		line;

	static size_t	use;

	if (!use && (!(dp->pz->test & 04) || !(file = getenv("_AST_pzip_debug_use")) || !(use = strton(file, NiL, NiL, 1))))
		use = 8 * 1024;
	dp->pz->count.windows++;
	if ((dp->pz->flags & (PZ_DUMP|PZ_VERBOSE)) && dp->pz->disc->errorf)
		(*dp->pz->disc->errorf)(dp->pz, dp->pz->disc, 0, "window %I*u %I*u", sizeof(dp->pz->count.windows), dp->pz->count.windows, sizeof(w), w);

	/*
	 * write the actual window size
	 */

	sfputu(op, w);

	/*
	 * count the number of active partitions for this window
	 */

	i = 0;
	for (ip = (Id_t*)dtfirst(dp->ids); ip; ip = (Id_t*)dtnext(dp->ids, ip))
		if (n = sfstrtell(ip->sp))
		{
			i++;
			ip->windows++;
			if (n >= use && (!ip->row || (n / ip->row) > 16))
			{
				if (!ip->used++ && !ip->part && !streq(ip->name, "0") && dp->pz->disc->errorf && (dp->pz->flags & (PZ_SUMMARY|PZ_VERBOSE|PZ_DUMP)))
					(*dp->pz->disc->errorf)(dp->pz, dp->pz->disc, 1, "%s: generate a partition to improve compression", ip->name);
				if (ip->use = !(dp->pz->flags & PZ_NOPZIP) && ip->part)
					ip->part->flags |= PZ_UPDATE;
			}
			else
				ip->use = 0;
		}
		else if (ip->part)
			ip->part->flags &= ~PZ_UPDATE;

	/*
	 * write any PZ_UPDATE partition headers
	 */

	if (pzpartwrite(dp->pz, op))
		return -1;

	/*
	 * write the number of partitions for this window
	 * followed by the table entry for each partition
	 */

	sfputu(op, i);
	for (ip = (Id_t*)dtfirst(dp->sqs); ip; ip = (Id_t*)dtnext(dp->sqs, ip))
		if (n = sfstrtell(ip->sp))
		{
			sfputu(op, ip->row);
			sfputu(op, ip->use);
			sfputu(op, n);
			sfputr(op, ip->name, 0);
			if ((dp->pz->flags & PZ_DUMP) && dp->pz->disc->errorf)
				(*dp->pz->disc->errorf)(dp->pz, dp->pz->disc, 0, "%8d %12s %2d %4d %4I*u %12I*u%s", ip->seq, ip->name, !!ip->part, ip->use, sizeof(ip->row), ip->row, sizeof(n), n, ip->windows == 1 ? "  NEW" : "");
		}
	if (sferror(op))
	{
		if (dp->pz->disc->errorf)
			(*dp->pz->disc->errorf)(dp->pz, dp->pz->disc, ERROR_SYSTEM|2, "partition table write error");
		return -1;
	}

	/*
	 * write the data for each pzip partition
	 */

	file = error_info.file;
	line = error_info.line;
	io = dp->pz->io;
	for (ip = (Id_t*)dtfirst(dp->sqs); ip; ip = (Id_t*)dtnext(dp->sqs, ip))
		if (ip->use && (n = sfstrtell(ip->sp)))
		{
			error_info.file = ip->name;
			error_info.line = n;
			sfstrseek(ip->sp, 0, SEEK_SET);
			if (!pzpartset(dp->pz, ip->part))
				goto bad;
			sfstrbuf(state.buf, sfstrbase(ip->sp), n, 0);
			dp->pz->io = state.buf;
			if (pzdeflate(dp->pz, op))
				goto bad;
			ip->seq = 0;
		}

	/*
	 * write the data for the remaining partitions
	 */

	for (ip = (Id_t*)dtfirst(dp->sqs); ip; ip = (Id_t*)dtnext(dp->sqs, ip))
		if (n = sfstrtell(ip->sp))
		{
			sfstrseek(ip->sp, 0, SEEK_SET);
			if (sfwrite(op, sfstrbase(ip->sp), n) != n || sferror(op))
			{
				error_info.file = ip->name;
				error_info.line = n;
				if (dp->pz->disc->errorf)
					(*dp->pz->disc->errorf)(dp->pz, dp->pz->disc, ERROR_SYSTEM|2, "partition data write error");
				goto bad;
			}
			ip->seq = 0;
		}
	error_info.file = file;
	error_info.line = line;
	dp->pz->io = io;

	/*
	 * end the record sequence number list and write it
	 */

	sfputu(dp->xp, 0);
	n = sfstrtell(dp->xp);
	sfstrseek(dp->xp, 0, SEEK_SET);
	if (sfwrite(op, sfstrbase(dp->xp), n) != n)
	{
		if (dp->pz->disc->errorf)
			(*dp->pz->disc->errorf)(dp->pz, dp->pz->disc, ERROR_SYSTEM|2, "record sequence write error");
		return -1;
	}

	/*
	 * done with this window
	 */

	if (sferror(op))
	{
		if (dp->pz->disc->errorf)
			(*dp->pz->disc->errorf)(dp->pz, dp->pz->disc, ERROR_SYSTEM|2, "write error");
		return -1;
	}
	dtclear(dp->sqs);
	dp->seq = 0;
	return 0;
 bad:
	error_info.file = file;
	error_info.line = line;
	dp->pz->io = io;
	return -1;
}

/*
 * deflate
 */

static int
deflate(Pz_t* pz, Sfio_t* op)
{
	Pzsplit_t*	rp;
	size_t		m;
	size_t		z;
	Dtdisc_t	iddisc;
	Dtdisc_t	sqdisc;
	char*		file;
	char*		s;
	int		line;
	int		i;
	Id_t*		ip;
	Pzindex_f	indexf;
	Sfoff_t		o;
	Deflate_t	def;
	Pzindex_t	index;
	Sfulong_t	extra;
	char		num[16];

	file = error_info.file;
	line = error_info.line;
	memset(&iddisc, 0, sizeof(iddisc));
	iddisc.key = offsetof(Id_t, id);
	iddisc.link = offsetof(Id_t, byid);
	iddisc.size = sizeof(unsigned long);
#if _ast_intswap
	iddisc.comparf = byulong;
#endif
	iddisc.freef = freeid;
	memset(&sqdisc, 0, sizeof(sqdisc));
	sqdisc.key = offsetof(Id_t, seq);
	sqdisc.link = offsetof(Id_t, byseq);
	sqdisc.size = sizeof(int);
#if _ast_intswap
	sqdisc.comparf = byint;
#endif
	memset(&def, 0, sizeof(def));
	def.pz = pz;
	if (!(state.buf = sfstropen()) || !(def.xp = sfstropen()))
		goto nospace;
	if (!(def.ids = dtopen(&iddisc, Dtoset)) || !(def.sqs = dtopen(&sqdisc, Dtoset)))
		goto nospace;
	def.seq = 0;

	/*
	 * write the pzip header
	 */

	if (pzheadwrite(def.pz, op))
		goto bad;
	if (sferror(op))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "magic write error");
		goto bad;
	}

	/*
	 * loop on the records and deflate a window at a time
	 */

	if (indexf = pz->disc->indexf)
		sfraise(op, SFGZ_GETPOS, &index.block);
	else
		index.block = 0;
	index.offset = extra = 0;
	m = pz->win - 8;
	error_info.file = (char*)pz->path;
	error_info.line = 0;
	while (rp = (*pz->disc->splitf)(pz, pz->io, pz->disc))
	{
		if (rp->record)
		{
			z = rp->record;
			error_info.line++;
			pz->count.records++;
		}
		else
			z = rp->size;
		if (!rp->size)
			continue;
		if ((index.offset + z) > m)
		{
			if (flush(&def, index.offset, op))
				goto bad;
			if (indexf)
				sfraise(op, SFGZ_GETPOS, &index.block);
			pz->count.uncompressed += index.offset - extra;
			index.offset = extra = 0;
		}
		if (rp->record && indexf && (*indexf)(pz, &index, rp->data, pz->disc) < 0)
			goto bad;
		index.offset += rp->size;
		if (!(ip = (Id_t*)dtmatch(def.ids, &rp->id)))
		{
			if (pz->disc->namef)
				s = (*pz->disc->namef)(pz, rp->id, pz->disc);
			else
				sfsprintf(s = num, sizeof(num), "%lu", rp->id);
			if (!(ip = newof(0, Id_t, 1, strlen(s) + 1)))
				goto nospace;
			if (ip->id = rp->id)
				ip->row = rp->size;
			ip->name = strcpy((char*)(ip + 1), s);
			if (!(ip->sp = sfstropen()))
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: cannot create tmp stream", ip->name);
				goto bad;
			}
			if ((ip->part = pzpartget(def.pz, ip->name)) && pz->disc->errorf && ip->row && ip->part->row != ip->row)
				(*pz->disc->errorf)(pz, pz->disc, 1, "%s: partition row %I*u != data row %I*u", ip->name, sizeof(ip->part->row), ip->part->row, sizeof(ip->row), ip->row);
			dtinsert(def.ids, ip);
		}
		else if (!ip->id)
			ip->total += rp->size;
		else if (pz->disc->errorf && ip->row != rp->size && (ip->row % rp->size))
			(*pz->disc->errorf)(pz, pz->disc, 1, "%s: size %I*u not a multiple of %I*u", ip->name, sizeof(rp->size), rp->size, sizeof(ip->row), ip->row);
		if (!ip->seq)
		{
			ip->seq = ++def.seq;
			dtinsert(def.sqs, ip);
		}
		sfputu(def.xp, ip->seq);
		if (!ip->id)
		{
			o = sfstrtell(ip->sp);
			sfputu(ip->sp, rp->size);
			o = sfstrtell(ip->sp) - o;
			index.offset += o;
			extra += o;
		}
		if (sfwrite(ip->sp, rp->data, rp->size) != rp->size)
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: %I*u byte write error", ip->name, sizeof(rp->size), rp->size);
			goto bad;
		}
		ip->modules++;
		pz->count.modules++;
	}
	if (index.offset)
	{
		if (flush(&def, index.offset, op))
			goto bad;
		pz->count.uncompressed += index.offset - extra;
	}

	/*
	 * done with all the data
	 * a 0 actual window size marks the end of data
	 */

	sfputu(op, 0);
	if (sferror(op))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "write error");
		goto bad;
	}
	if ((pz->flags & PZ_DUMP) && pz->disc->errorf)
	{
		(*pz->disc->errorf)(pz, pz->disc, 0, "totals");
		for (ip = (Id_t*)dtfirst(def.ids); ip; ip = (Id_t*)dtnext(def.ids, ip))
			(*pz->disc->errorf)(pz, pz->disc, 0, "%8I*u %12s %2u %4I*u %4I*u %12I*u %12I*u%s", sizeof(ip->windows), ip->windows, ip->name, !!ip->part, sizeof(ip->used), ip->used, sizeof(ip->row), ip->row, sizeof(ip->total), ip->total ? ip->total : ip->modules * ip->row, sizeof(ip->modules), ip->modules, ip->used && !ip->part ? "  GENERATE PARTITION" : "");
	}
	i = 0;
	goto done;
 nospace:
	pznospace(pz);
 bad:
	i = -1;
 done:
	if (state.buf)
		sfclose(state.buf);
	if (def.xp)
		sfclose(def.xp);
	if (def.ids)
		dtclose(def.ids);
	if (def.sqs)
		dtclose(def.sqs);
	error_info.file = file;
	error_info.line = line;
	return i;
}

/*
 * inflate
 */

static int
inflate(Pz_t* pz, Sfio_t* op)
{
	register char*		s;
	register Id_t*		ip;
	register size_t		m;
	register size_t		n;
	register size_t		u;
	register unsigned char*	p;
	size_t			w;
	size_t			i;
	size_t			parts;
	char*			id;
	int			row;
	int			use;
	Dt_t*			ids;
	Dtdisc_t		iddisc;

	register Id_t**		tab = 0;
	size_t			tabsiz = 0;
	char*			win = 0;

	memset(&iddisc, 0, sizeof(iddisc));
	iddisc.key = offsetof(Id_t, name);
	iddisc.link = offsetof(Id_t, byid);
	iddisc.size = -1;
	iddisc.freef = freeid;
	if (!(ids = dtopen(&iddisc, Dtoset)))
		goto nospace;
	if (!(state.buf = sfstropen()))
		goto nospace;
	if (!(win = newof(0, char, pz->win, 0)))
		goto nospace;

	/*
	 * loop on all windows
	 * w is the actual window size
	 * w <= pz->win guaranteed
	 */

	while ((w = sfgetu(pz->io)) && w <= pz->win)
	{
		if ((pz->flags & (PZ_DUMP|PZ_VERBOSE)) && pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 0, "window %I*u", sizeof(w), w);

		/*
		 * read the partition headers
		 */

		if (pzpartread(pz))
			goto bad;

		/*
		 * read the number of partitions
		 */

		parts = sfgetu(pz->io);
		if (parts > tabsiz)
		{
			n = roundof(parts, 64);
			if (!(tab = newof(tab, Id_t*, n, 0)))
				goto nospace;
			tabsiz = n;
		}

		/*
		 * read the partition table
		 */

		u = 0;
		s = win;
		for (i = 0; i < parts; i++)
		{
			row = sfgetu(pz->io);
			use = sfgetu(pz->io);
			m = sfgetu(pz->io);
			id = sfgetr(pz->io, 0, 0);
			if (!(ip = (Id_t*)dtmatch(ids, id)))
			{
				if (!(ip = newof(0, Id_t, 1, sfvalue(pz->io))))
					goto nospace;
				ip->name = strcpy((char*)(ip + 1), id);
				ip->row = row;
				if ((ip->part = pzpartget(pz, ip->name)) && pz->disc->errorf && ip->row && ip->part->row != ip->row)
					(*pz->disc->errorf)(pz, pz->disc, 1, "%s: partition row %I*u != data row %I*u", ip->name, sizeof(ip->part->row), ip->part->row, sizeof(ip->row), ip->row);
				dtinsert(ids, ip);
			}
			ip->use = use;
			ip->used++;
			tab[i] = ip;
			if (m >= pz->win || ip->row && m % ip->row)
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, 2, "input corrupted [%s partition table size %I*u]", ip->name, sizeof(m), m);
				goto bad;
			}
			ip->size = m;
			if (ip->use)
			{
				ip->bp = s;
				s += m;
			}
			else
				u += m;
			if ((pz->flags & PZ_DUMP) && pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 0, "%8d %12s %2d %4I*u %4I*u %12I*u%s", i + 1, ip->name, ip->use, sizeof(ip->used), ip->used, sizeof(ip->row), ip->row, sizeof(ip->size), ip->size, ip->used == 1 ? "  NEW" : "");
		}

		/*
		 * read the pzip partition data
		 */

		for (i = 0; i < parts; i++)
			if (tab[i]->use)
			{
				sfstrbuf(state.buf, tab[i]->bp, tab[i]->size, 0);
				if (!pzpartset(pz, tab[i]->part))
					goto bad;
				if (pzinflate(pz, state.buf))
					goto bad;
			}

		/*
		 * read the remaining partition data
		 * and set up the table buffer pointers
		 */

		if (sfread(pz->io, s, u) != u)
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "cannot read %I*u byte partition data buffer", sizeof(u), u);
			goto bad;
		}
		for (i = 0; i < parts; i++)
			if (!tab[i]->use)
			{
				tab[i]->bp = s;
				s += tab[i]->size;
			}

		/*
		 * read the record table indices and reconstruct the records
		 */
		
		while (m = sfgetu(pz->io))
		{
			if (m > parts)
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, 2, "input corrupted [partition record index %I*u > %I*u]", sizeof(m), m, sizeof(parts), parts);
				goto bad;
			}
			ip = tab[m - 1];
			p = (unsigned char*)ip->bp;
			if (!(n = ip->row) && ((n = *p++) & SF_MORE))
			{
				n &= (SF_MORE - 1);
				while ((i = *p++) & SF_MORE)
					n = (n << SF_UBITS) | (i & (SF_MORE - 1));
				n = (n << SF_UBITS) | i;
			}
#if 0
			if (s = (char*)sfreserve(op, n, 0)) /* NOTE: investigate how reserve can fail but write work */
				memcpy(s, p, n);
			else
#endif
			if (sfwrite(op, p, n) != n)
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%I*u byte write error", sizeof(n), n);
				goto bad;
			}
			ip->bp = (char*)p + n;
		}
	}
	if (w || sfgetc(pz->io) != EOF)
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 2, "%s: input corrupted [EOF expected at %I*u]", pz->path, sftell(pz->io));
		goto bad;
	}
	if (sferror(op))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "write error");
		goto bad;
	}
	i = 0;
	goto done;
 nospace:
	pznospace(pz);
 bad:
	i = -1;
 done:
	if (state.buf)
		sfclose(state.buf);
	if (ids)
		dtclose(ids);
	if (tab)
		free(tab);
	if (win)
		free(win);
	return i;
}

int
pzsdeflate(Pz_t* pz, Sfio_t* op)
{
	int		r;

	pz->split.flags |= PZ_SPLIT_PART;
	r = deflate(pz, op);
	pz->split.flags &= ~PZ_SPLIT_PART;
	return r;
}

int
pzsinflate(Pz_t* pz, Sfio_t* op)
{
	int	r;

	pz->split.flags |= PZ_SPLIT_PART;
	r = inflate(pz, op);
	pz->split.flags &= ~PZ_SPLIT_PART;
	return r;
}

int
pzssplit(Pz_t* pz)
{
	Pzsplit_t*	rp;
	Dtdisc_t	iddisc;
	char*		file;
	char*		s;
	int		line;
	int		i;
	size_t		window;
	Id_t*		ip;
	Dt_t*		ids;
	char		num[16];

	if (!pz->disc->splitf)
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 2, "%s: split discipline library expected", pz->path);
		return -1;
	}
	window = pz->disc->window ? pz->disc->window : PZ_WINDOW;
	file = error_info.file;
	line = error_info.line;
	memset(&iddisc, 0, sizeof(iddisc));
	iddisc.key = offsetof(Id_t, id);
	iddisc.link = offsetof(Id_t, byid);
	iddisc.size = sizeof(unsigned long);
#if _ast_intswap
	iddisc.comparf = byulong;
#endif
	iddisc.freef = freeid;
	if (!(ids = dtopen(&iddisc, Dtoset)))
		goto nospace;

	/*
	 * loop on the records and split by id
	 */

	error_info.file = (char*)pz->path;
	error_info.line = 0;
	while (rp = (*pz->disc->splitf)(pz, pz->io, pz->disc))
	{
		if (rp->record)
		{
			error_info.line++;
			pz->count.records++;
		}
		if (!(ip = (Id_t*)dtmatch(ids, &rp->id)))
		{
			if (pz->disc->namef)
				s = (*pz->disc->namef)(pz, rp->id, pz->disc);
			else
				sfsprintf(s = num, sizeof(num), "%lu", rp->id);
			if (!(ip = newof(0, Id_t, 1, strlen(s) + 1)))
				goto nospace;
			if (ip->id = rp->id)
				ip->row = rp->size;
			ip->name = strcpy((char*)(ip + 1), s);
			if (!pz->split.match || strmatch(ip->name, pz->split.match))
			{
				if (!(ip->sp = sfopen(NiL, ip->name, (pz->flags & PZ_APPEND) ? "a" : "w")))
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: cannot create split stream", ip->name);
					goto bad;
				}
				ip->count = sfsize(ip->sp);
			}
			dtinsert(ids, ip);
			if ((pz->flags & PZ_DUMP) && pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 0, "split %s size %I*u", ip->name, sizeof(ip->row), ip->row);
		}
		else if (!ip->id)
			ip->total += rp->size;
		else if (pz->disc->errorf && ip->row != rp->size && rp->size && (ip->row % rp->size))
			(*pz->disc->errorf)(pz, pz->disc, 1, "%s: size %I*u not a multiple of %I*u", ip->name, sizeof(rp->size), rp->size, sizeof(ip->row), ip->row);
		if (ip->sp)
		{
			if (ip->count < window)
			{
				if (sfwrite(ip->sp, rp->data, rp->size) != rp->size)
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: %I*u byte write error", ip->name, sizeof(rp->size), rp->size);
					goto bad;
				}
				ip->count += rp->size;
			}
			ip->modules++;
			pz->count.modules++;
		}
	}
	if ((pz->flags & PZ_DUMP) && pz->disc->errorf)
	{
		(*pz->disc->errorf)(pz, pz->disc, 0, "totals");
		for (ip = (Id_t*)dtfirst(ids); ip; ip = (Id_t*)dtnext(ids, ip))
			if (ip->sp)
				(*pz->disc->errorf)(pz, pz->disc, 0, "%8I*u %12s %2u %4I*u %4I*u %12I*u %12I*u%s", sizeof(ip->windows), ip->windows, ip->name, !!ip->part, sizeof(ip->used), ip->used, sizeof(ip->row), ip->row, sizeof(ip->total), ip->total ? ip->total : ip->modules * ip->row, sizeof(ip->modules), ip->modules, ip->used && !ip->part ? "  GENERATE PARTITION" : "");
	}
	for (ip = (Id_t*)dtfirst(ids); ip; ip = (Id_t*)dtnext(ids, ip))
		if (ip->sp && sfclose(ip->sp))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: write error", ip->name);
			goto bad;
		}
	i = 0;
	goto done;
 nospace:
	pznospace(pz);
 bad:
	i = -1;
 done:
	if (ids)
		dtclose(ids);
	error_info.file = file;
	error_info.line = line;
	return i;
}
