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
 * write a buffer to a pz stream
 *
 * code cannibalized from pzdeflate()
 * the PZ_SORT pzsync() code could be more efficient
 */

ssize_t
pzwrite(register Pz_t* pz, Sfio_t* op, const void* buf, size_t n)
{
	register int		i;
	register int		j;
	register unsigned char*	bp;
	register Pzpart_t*	pp;
	register unsigned char*	pat;
	register unsigned char*	low;
	Pzelt_t*		elt;
	unsigned char*		be;
	size_t			k;
	size_t			x;
	ssize_t			r;
	Sfio_t*			tmp;

	if (!(pz->flags & PZ_WRITE))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 2, "%s: cannot deflate -- not open for write", pz->path);
		return -1;
	}
	if (!n)
		return 0;
	if (pzheadwrite(pz, op))
		return -1;
	if (pz->flags & PZ_NOPZIP)
	{
		if ((r = sfwrite(op, buf, n)) < 0)
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: write error", pz->path);
			return -1;
		}
		return r;
	}
	pp = pz->part;
	if (pz->flags & PZ_SORT)
	{
		pz->ws.bp = pz->buf;
		if (!pz->sort.order)
		{
			k = sizeof(Dtlink_t) + roundof(pp->row, sizeof(Dtlink_t));
			pz->sort.freedisc.link = offsetof(Pzelt_t, link);
			pz->sort.orderdisc.link = offsetof(Pzelt_t, link);
			pz->sort.orderdisc.key = offsetof(Pzelt_t, buf);
			pz->sort.orderdisc.size = pp->row;
			if (!(elt = (Pzelt_t*)vmnewof(pz->vm, 0, char, pp->col * k, 0)) || !(pz->sort.order = dtnew(pz->vm, &pz->sort.orderdisc, Dtobag)) || !(pz->sort.free = dtnew(pz->vm, &pz->sort.freedisc, Dtlist)))
				return pznospace(pz);
			for (i = 0; i < pp->col; i++)
			{
				dtinsert(pz->sort.free, elt);
				elt = (Pzelt_t*)((char*)elt + k);
			}
		}
		bp = (unsigned char*)buf;
		k = n;
		if (pz->ws.sz)
		{
			x = pz->ws.sz;
			if (x > n)
				x = n;
			memcpy(pz->ws.sp, bp, x);
			bp += x;
			k -= x;
			if (pz->ws.sz -= x)
				pz->ws.sp += x;
			else
				dtinsert(pz->sort.order, pz->ws.se);
		}
		x = pp->row;
		while (k > 0)
		{
			while (!(elt = (Pzelt_t*)dtfirst(pz->sort.free)))
				if (pzsync(pz))
					return -1;
			dtdelete(pz->sort.free, elt);
			if (k < x)
			{
				memcpy(elt->buf, bp, k);
				pz->ws.sp = elt->buf + k;
				pz->ws.sz = x - k;
				pz->ws.se = elt;
				break;
			}
			memcpy(elt->buf, bp, x);
			dtinsert(pz->sort.order, elt);
			bp += x;
			k -= x;
		}
		return n;
	}
	if (pz->ws.pc || n < pp->row)
	{
		if (!pz->ws.pb && !(pz->ws.pb = vmnewof(pz->vm, 0, unsigned char, pp->row, 0)))
			return -1;
		x = pp->row - pz->ws.pc;
		if (x > n)
			x = n;
		memcpy(pz->ws.pb + pz->ws.pc, buf, x);
		if ((pz->ws.pc += x) < pp->row)
			return x;
		pz->ws.pc = 0;
		if (pzwrite(pz, op, pz->ws.pb, pp->row) != pp->row)
			return -1;
		if (!(n -= x))
			return x;
	}
	else
		x = 0;
	bp = (unsigned char*)buf + x;
	be = bp + n;
	if (k = n % pp->row)
	{
		if (!pz->ws.pb && !(pz->ws.pb = vmnewof(pz->vm, 0, unsigned char, pp->row, 0)))
			return -1;
		x += k;
		n -= k;
		be -= k;
		memcpy(pz->ws.pb + pz->ws.pc, be, k);
		pz->ws.pc += k;
	}
	pat = pz->pat;
	tmp = pz->tmp;
	low = pp->low;
	while (bp < be)
	{
		if (!pz->ws.bp)
		{
			/*
			 * initialize for a new window
			 */

			pz->ws.io = op;
			memcpy(pat, bp, pp->row);
			bp += pp->row;
			for (i = 0; i < pp->nfix; i++)
				pat[pp->fix[i]] = pp->value[pp->fix[i]];
			pz->ws.ve = (pz->ws.vp = pz->val) + pp->loq - 2 * pp->row;
			memcpy(pz->ws.vp, pat, pp->row);
			pz->ws.vp += pp->row;
			pz->ws.bp = pz->buf;
			for (j = 0; j < pp->nmap; j++)
				*pz->ws.bp++ = pat[pp->map[j]];
			pz->ws.rep = pz->ws.row = 1;
		}

		/*
		 * collect a window of hi frequency cols in buf
		 * and encode the lo frequency rows in tmp+pz->val
		 * lo frequency values must not exceed pp->loq
		 */

		k = pz->ws.row + (be - bp) / pp->row;
		if (k > pp->col)
			k = pp->col;
		while (pz->ws.row < k)
		{
			for (j = 0; j < pp->row; j++)
				if (pat[j] != bp[j] && low[j])
				{
					if (pz->ws.vp >= pz->ws.ve)
						goto dump;
					sfputu(tmp, pz->ws.rep);
					sfputu(tmp, j + 1);
					*pz->ws.vp++ = pat[j] = bp[j];
					pz->ws.rep = 0;
					while (++j < pp->row)
						if (pat[j] != bp[j] && low[j])
						{
							sfputu(tmp, j + 1);
							*pz->ws.vp++ = pat[j] = bp[j];
						}
					sfputu(tmp, 0);
					break;
				}
			for (j = 0; j < pp->nmap; j++)
				*pz->ws.bp++ = bp[pp->map[j]];
			pz->ws.rep++;
			pz->ws.row++;
			bp += pp->row;
		}
		if (k < pp->col)
			continue;
	dump:
		if (pzsync(pz))
			return -1;
	}
	return n + x;
}
