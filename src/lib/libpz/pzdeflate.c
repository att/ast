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

#include "pzlib.h"

/*
 * pz deflate from pz->io to op
 */

int
pzdeflate(register Pz_t* pz, Sfio_t* op)
{
	register Pzpart_t*	pp;
	register int		i;
	register int		j;
	register size_t		n;
	register size_t		m;
	register unsigned char*	buf;
	register unsigned char*	wrk;
	register unsigned char*	pat;
	register unsigned char*	low;
	unsigned char*		incomplete;
	Pzelt_t*		elt;
	Pzelt_t*		old;
	Dt_t*			order;
	unsigned char*		vp;
	unsigned char*		ve;
	Sfoff_t			r;
	int			peek;
	Pzread_f		readf;
	Pzindex_f		indexf;
	Pzindex_t		index;
	Sfio_t*			tmp;
	
	if (!(pz->flags & PZ_WRITE))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: cannot deflate -- not open for write", pz->path);
		return -1;
	}
	if (pzheadwrite(pz, op))
		return -1;
	if (pz->flags & PZ_NOPZIP)
	{
		n = pz->part->row;
		if (readf = pz->disc->readf)
			for (;;)
			{
				if (!(buf = (unsigned char*)sfreserve(op, n, 1)))
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 2, "%s: output write error", pz->path);
					return -1;
				}
				if ((r = (*readf)(pz, pz->io, buf, pz->disc)) < 0)
					return -1;
				if (sfwrite(op, buf, r) != r)
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 2, "%s: output write error", pz->path);
					return -1;
				}
				if (r < n)
					break;
			}
		else
			r = sfmove(pz->io, op, SF_UNBOUND, -1);
		if (r < 0 || sferror(pz->io))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: read error", pz->path);
			return -1;
		}
		if (sfsync(op))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: output write error", pz->path);
			return -1;
		}
		return 0;
	}
	if ((pz->split.flags & (PZ_SPLIT_DEFLATE|PZ_SPLIT_PART)) == PZ_SPLIT_DEFLATE)
	{
		if (pzsdeflate(pz, op) < 0)
			return -1;
	}
	else
	{
		/*
		 * deflate each window
		 */

		pp = pz->part;
		readf = pz->disc->readf;
		indexf = (pz->disc->version >= PZ_VERSION_SPLIT) ? pz->disc->indexf : (Pzindex_f)0;
		wrk = pz->wrk;
		pat = pz->pat;
		low = pp->low;
		tmp = pz->tmp;
		peek = 0;
		if (pz->flags & PZ_SORT)
		{
			if (!pz->sort.order)
			{
				m = sizeof(Dtlink_t) + roundof(pp->row, sizeof(Dtlink_t));
				pz->sort.freedisc.link = offsetof(Pzelt_t, link);
				pz->sort.orderdisc.link = offsetof(Pzelt_t, link);
				pz->sort.orderdisc.key = offsetof(Pzelt_t, buf);
				pz->sort.orderdisc.size = pp->row;
				if (!(elt = (Pzelt_t*)vmnewof(pz->vm, 0, char, pp->col * m, 0)) || !(pz->sort.order = dtnew(pz->vm, &pz->sort.orderdisc, Dtobag)) || !(pz->sort.free = dtnew(pz->vm, &pz->sort.freedisc, Dtlist)))
					return pznospace(pz);;
				for (i = 0; i < pp->col; i++)
				{
					dtinsert(pz->sort.free, elt);
					elt = (Pzelt_t*)((char*)elt + m);
				}
			}
			elt = 0;
		}
		incomplete = 0;
		order = pz->sort.order;
		do
		{
			if (peek)
				peek = 0;
			else if ((r = readf ? (*readf)(pz, pz->io, pat, pz->disc) : sfread(pz->io, pat, pp->row)) != pp->row)
			{
				incomplete = pat;
				break;
			}
			if (indexf)
			{
				sfraise(op, SFGZ_GETPOS, &index.block);
				index.offset = 0;
				if ((*indexf)(pz, &index, pat, pz->disc) < 0)
					break;
			}
			if (order)
			{
				if (!elt)
				{
					elt = (Pzelt_t*)dtfirst(pz->sort.free);
					dtdelete(pz->sort.free, elt);
					memcpy(elt->buf, pat, pp->row);
					dtinsert(order, elt);
				}
				for (i = dtsize(order); i < pp->col; i++)
				{
					elt = (Pzelt_t*)dtfirst(pz->sort.free);
					dtdelete(pz->sort.free, elt);
					if ((r = readf ? (*readf)(pz, pz->io, elt->buf, pz->disc) : sfread(pz->io, elt->buf, pp->row)) != pp->row)
					{
						incomplete = elt->buf;
						break;
					}
					dtinsert(order, elt);
				}
				elt = (Pzelt_t*)dtfirst(order);
				memcpy(pat, elt->buf, pp->row);
			}

			/*
			 * collect a window of hi frequency cols in buf
			 * and encode the lo frequency rows in tmp+pz->val
			 * lo frequency values must not exceed pp->loq
			 */

			for (i = 0; i < pp->nfix; i++)
				pat[pp->fix[i]] = pp->value[pp->fix[i]];
			ve = (vp = pz->val) + pp->loq - 2 * pp->row;
			memcpy(vp, pat, pp->row);
			vp += pp->row;
			buf = pz->buf;
			for (j = 0; j < pp->nmap; j++)
				*buf++ = pat[pp->map[j]];
			m = 1;
			for (n = 1; n < pp->col; n++)
			{
				if (order)
				{
					old = elt;
					elt = (Pzelt_t*)dtnext(order, elt);
					dtdelete(order, old);
					dtinsert(pz->sort.free, old);
					if (!elt)
						break;
					wrk = elt->buf;
				}
				if ((r = readf ? (*readf)(pz, pz->io, wrk, pz->disc) : sfread(pz->io, wrk, pp->row)) != pp->row)
				{
					incomplete = wrk;
					break;
				}
				for (j = 0; j < pp->row; j++)
					if (pat[j] != wrk[j] && low[j])
					{
						if (vp >= ve)
						{
							memcpy(pat, wrk, pp->row);
							peek = 1;
							goto dump;
						}
						sfputu(tmp, m);
						sfputu(tmp, j + 1);
						*vp++ = pat[j] = wrk[j];
						m = 0;
						while (++j < pp->row)
							if (pat[j] != wrk[j] && low[j])
							{
								sfputu(tmp, j + 1);
								*vp++ = pat[j] = wrk[j];
							}
						sfputu(tmp, 0);
						break;
					}
				m++;
				for (j = 0; j < pp->nmap; j++)
					*buf++ = wrk[pp->map[j]];
				if (indexf)
				{
					index.offset += pp->row;
					if ((*indexf)(pz, &index, wrk, pz->disc) < 0)
						break;
				}
			}
		dump:
			if (pz->flags & PZ_SECTION)
				pz->count.sections++;
			else
			{
				pz->count.windows++;
				pz->count.records += n;
			}
			sfputu(tmp, m);
			sfputu(tmp, 0);

			/*
			 * transpose the hi frequency from row major to col major
			 * and write it by group to op
			 */

			if (pp->nmap)
			{
				pp->mix[0] = buf = pz->wrk;
				m = 0;
				for (j = 1; j < pp->nmap; j++)
				{
					m += n;
					pp->mix[j] = (pp->lab[j] == pp->lab[j - 1]) ?  (pp->mix[j - 1] + 1) : (buf + m);
				}
				buf = pz->buf;
				for (i = 0; i < n; i++)
					for (j = 0; j < pp->nmap; j++)
					{
						*pp->mix[j] = *buf++;
						pp->mix[j] += pp->inc[j];
					}
				m = n * pp->nmap;
				sfputu(op, m);
				buf = pz->wrk;
				for (i = 0; i < pp->ngrp; i++)
				{
					m = n * pp->grp[i];
					if (sfwrite(op, buf, m) != m || sfsync(op))
					{
						if (pz->disc->errorf)
							(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "hi frequency write error");
						return -1;
					}
					buf += m;
				}
			}
			else
			{
				/*
				 * this is a phony size that is verified on inflate
				 * 0 here would terminate the inflate loop in the
				 * first window
				 */

				sfputu(op, 1);
			}

			/*
			 * now write the lo frequency encoding
			 */

			m = vp - pz->val;
			sfputu(op, m);
			if (sfwrite(op, pz->val, m) != m || sfsync(op))
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "lo frequency value write error");
				return -1;
			}
			m = sfstrtell(tmp);
			if (sfwrite(op, sfstrseek(tmp, 0, SEEK_SET), m) != m || sfsync(op))
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "lo frequency code write error");
				return -1;
			}
		} while (!incomplete);
		sfputu(op, 0);
		if (incomplete && !readf)
		{
			if (r < 0)
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "read error");
			}
			else if (r > 0)
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, 1, "last record incomplete %u/%u", r, pp->row);
				sfputc(op, PZ_MARK_PART);
				sfputu(op, r);
				sfwrite(op, incomplete, r);
			}
		}
		if (!(pz->flags & PZ_SECTION))
		{
			sfputc(op, PZ_MARK_TAIL);
			if (pz->disc->eventf && (*pz->disc->eventf)(pz, PZ_TAILWRITE, op, 0, pz->disc) < 0)
				return -1;
			sfputc(op, 0);
		}
	}
	if (sfsync(op))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "write error");
		return -1;
	}
	return 0;
}
