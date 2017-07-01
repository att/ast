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
 * sync outstanding data in a pz stream
 */

int
pzsync(register Pz_t* pz)
{
	register unsigned char*	buf;
	register size_t		i;
	register size_t		j;
	size_t			n;
	size_t			m;
	Pzpart_t*		pp;
	Sfio_t*			tmp;
	Sfio_t*			op;
	Pzelt_t*		elt;
	Pzelt_t*		old;
	int			r;

	op = pz->ws.io ? pz->ws.io : pz->io;
	if (pz->ws.bp)
	{
		/*
		 * flush the pzwrite() window
		 */

		pz->ws.bp = 0;
		pp = pz->part;
		if (pz->flags & PZ_SORT)
		{
			pz->flags &= ~PZ_SORT;
			elt = (Pzelt_t*)dtfirst(pz->sort.order);
			n = pp->row;
			while (elt && pzwrite(pz, op, elt->buf, n) == n)
			{
				old = elt;
				elt = (Pzelt_t*)dtnext(pz->sort.order, elt);
				dtdelete(pz->sort.order, old);
				dtinsert(pz->sort.free, old);
			}
			r = pzsync(pz);
			pz->flags |= PZ_SORT;
			pz->ws.bp = pz->buf;
			return r;
		}
		tmp = pz->tmp;
		n = pz->ws.row;
		if (pz->flags & PZ_SECTION)
			pz->count.sections++;
		else
			pz->count.windows++;
		pz->count.records += pz->ws.rep;
		sfputu(tmp, pz->ws.rep);
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

		m = pz->ws.vp - pz->val;
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
	}
	else if ((pz->flags & PZ_WRITE) && sfsync(op))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "write error");
		return -1;
	}
	return 0;
}
