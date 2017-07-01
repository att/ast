/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2013 AT&T Intellectual Property          *
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
#include	"terror.h"

tmain()
{
	unsigned char	vc, rc;
	unsigned short	vs, rs;
	unsigned int	vi, ri;
	unsigned long	vl, rl;
	void		*vp, *rp;

	vc = 1;
	if ((rc = asocaschar(&vc, 1, 2)) != 1)
		terror("asocaschar return value failed -- expected %d, got %d", 1, rc);
	else if (vc != 2)
		terror("asocaschar value failed -- expected %d, got %d", 1, vc);
	else if ((rc = asocaschar(&vc, 1, 3)) != 2)
		terror("asocaschar return value failed -- expected %d, got %d", 2, rc);
	else if (vc != 2)
		terror("asocaschar value failed -- expected %d, got %d", 2, vc);
	if ((rc = asoincchar(&vc)) != 2)
		terror("asoincchar return failed -- expected %d, got %d", 2, rc);
	else if (vc != 3)
		terror("asoincchar value failed -- expected %d, got %d", 3, vc);
	if ((rc = asodecchar(&vc)) != 3)
		terror("asodecchar return failed -- expected %d, got %d", 3, rc);
	else if (vc != 2)
		terror("asodecchar value failed -- expected %d, got %d", 2, vc);

	vs = 1;
	if ((rs = asocasshort(&vs, 1, 2)) != 1)
		terror("asocasshort return value failed -- expected %d, got %d", 1, rs);
	else if (vs != 2)
		terror("asocasshort value failed -- expected %d, got %d", 2, vs);
	else if ((rs = asocasshort(&vs, 1, 3)) != 2)
		terror("asocasshort return value failed -- expected %d, got %d", 2, rs);
	else if (vs != 2)
		terror("asocasshort value failed -- expected %d, got %d", 2, vs);
	if ((rs = asoincshort(&vs)) != 2)
		terror("asoincshort return failed -- expected %d, got %d", 2, rs);
	else if (vs != 3)
		terror("asoincshort value failed -- expected %d, got %d", 3, vs);
	if ((rs = asodecshort(&vs)) != 3)
		terror("asodecshort return failed -- expected %d, got %d", 3, rs);
	else if (vs != 2)
		terror("asodecshort value failed -- expected %d, got %d", 2, vs);

	vi = 1;
	if ((ri = asocasint(&vi, 1, 2)) != 1)
		terror("asocasint return value failed -- expected %d, got %d", 1, ri);
	else if (vi != 2)
		terror("asocasint value failed -- expected %d, got %d", 2, vi);
	else if ((ri = asocasint(&vi, 1, 3)) != 2)
		terror("asocasint return value failed -- expected %d, got %d", 2, ri);
	else if (vi != 2)
		terror("asocasint value failed -- expected %d, got %d", 2, vi);
	if ((ri = asoincint(&vi)) != 2)
		terror("asoincint return failed -- expected %d, got %d", 2, ri);
	else if (vi != 3)
		terror("asoincint value failed -- expected %d, got %d", 3, vi);
	if ((ri = asodecint(&vi)) != 3)
		terror("asodecint return failed -- expected %d, got %d", 3, ri);
	else if (vi != 2)
		terror("asodecint value failed -- expected %d, got %d", 2, vi);

	vl = 1;
	if ((rl = asocaslong(&vl, 1, 2)) != 1)
		terror("asocaslong return value failed -- expected %d, got %ld", 1, rl);
	else if (vl != 2)
		terror("asocaslong value failed -- expected %d, got %ld", 2, vl);
	else if ((rl = asocaslong(&vl, 1, 3)) != 2)
		terror("asocaslong return value failed -- expected %d, got %ld", 2, rl);
	else if (vl != 2)
		terror("asocaslong value failed -- expected %d, got %ld", 2, vl);
	if ((rl = asoinclong(&vl)) != 2)
		terror("asoinclong return failed -- expected %d, got %ld", 2, rl);
	else if (vl != 3)
		terror("asoinclong value failed -- expected %d, got %ld", 3, vl);
	if ((rl = asodeclong(&vl)) != 3)
		terror("asodeclong return failed -- expected %d, got %ld", 3, rl);
	else if (vl != 2)
		terror("asodeclong value failed -- expected %d, got %ld", 2, vl);

	vp = (void*)1;
	if ((rp = asocasptr(&vp, (void*)1, (void*)2)) != (void*)1)
		terror("asocasptr return value failed -- expected %p, got %p", (void*)1, rp);
	else if (vp != (void*)2)
		terror("asocasptr value failed -- expected %p, got %p", (void*)2, vp);
	else if ((rp = asocasptr(&vp, (void*)1, (void*)3)) != (void*)2)
		terror("asocasptr return value failed -- expected %p, got %p", (void*)2, rp);
	else if (vp != (void*)2)
		terror("asocasptr value failed -- expected %p, got %p", (void*)2, vp);

	texit(0);
}
