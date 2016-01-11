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
 * pz header support
 */

#include "pzlib.h"

/*
 * read the pz header from pz->io
 */

int
pzheadread(register Pz_t* pz)
{
	register int		i;
	register int		n;
	register unsigned char*	s;
	size_t			m;
	Pzpart_t*		pp;

	if (pz->flags & PZ_HEAD)
		return 0;

	/*
	 * check the header magic
	 */

	if (s = (unsigned char*)sfreserve(pz->io, 4, 1))
	{
		i = s[0];
		n = s[1];
	}
	else
		i = n = 0;
	if (pz->disc->errorf)
		(*pz->disc->errorf)(pz, pz->disc, -1, "%s: pzheadread: f=%08x i=%02x n=%02x partition=%s%s", pz->path, pz->flags, i, n, pz->disc->partition, s ? "" : " (nil)");
	if (i != PZ_MAGIC_1 || n != PZ_MAGIC_2 || s[2] == 0 || s[3] >= 10)
	{
		sfread(pz->io, s, 0);
		if (pz->flags & PZ_SPLIT)
			return 0;
		if (pz->flags & PZ_DISC)
		{
			pz->flags &= ~PZ_POP;
			return -1;
		}
		if (!(pz->flags & (PZ_READ|PZ_WRITE|PZ_STAT)) && (m = pz->prefix.count))
		{
			if (pz->prefix.terminator >= 0)
			{
				while (m-- > 0)
				{
					if (!sfgetr(pz->io, pz->prefix.terminator, 0))
					{
						if (pz->disc->errorf)
							(*pz->disc->errorf)(pz, pz->disc, 2, "%s: cannot read %I*u prefix record%s from data", pz->path, sizeof(pz->prefix.count), pz->prefix.count, pz->prefix.count == 1 ? "" : "s");
						return -1;
					}
				}
			}
			else if (!sfreserve(pz->io, m, 0))
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, 2, "%s: cannot read %I*u prefix byte%s from data", pz->path, sizeof(pz->prefix.count), pz->prefix.count, pz->prefix.count == 1 ? "" : "s");
				return -1;
			}
		}
		if (!(n = pz->row))
		{
			if ((pz->flags & PZ_ACCEPT) || !sfsize(pz->io))
				n = 1;
			else if ((n = pzfixed(pz, pz->io, NiL, 0)) <= 0 && pz->disc->partition)
			{
				pz->flags |= PZ_ROWONLY;
				if (!pzpartition(pz, pz->disc->partition))
					n = pz->row;
				pz->flags &= ~PZ_ROWONLY;
			}
		}
		if (n <= 0)
		{
			if (!(pz->flags & PZ_DELAY) && (pz->disc->partition || !(pz->flags & PZ_FORCE)))
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, 2, "%s: unknown input format", pz->path);
				return -1;
			}
			pz->flags |= PZ_UNKNOWN;
			n = 1;
		}
		if (!(pp = vmnewof(pz->vm, 0, Pzpart_t, 1, 0)))
			return -1;
		pz->major = PZ_MAJOR;
		pz->minor = PZ_MINOR;
		pp->name = "";
		pp->row = n;
		return pzpartinit(pz, pp, NiL);
	}
	sfread(pz->io, s, 2);
	pz->flags &= ~PZ_FORCE;
	pz->major = sfgetc(pz->io);
	pz->minor = sfgetc(pz->io);
	switch (pz->major)
	{
	case 1:
		if (pz->minor <= 2)
			goto noway;
		break;
	case 2:
		pz->win = sfgetu(pz->io);
		break;
	default:
		goto noway;
	}
	pz->flags |= PZ_HEAD;
	return pzpartread(pz);
 noway:
	if (pz->disc->errorf)
		(*pz->disc->errorf)(pz, pz->disc, 2, "%s: data %d.%d not supported by implementation %d.%d", pz->path, pz->major, pz->minor, PZ_MAJOR, PZ_MINOR);
	return -1;
}

/*
 * write the pz header to op
 */

int
pzheadwrite(Pz_t* pz, Sfio_t* op)
{
	register size_t	i;
	register size_t	m;
	register size_t	n;
	register char*	s;

	if (pz->flags & PZ_HEAD)
		return 0;
	pz->oop = op;
	if (!(pz->flags & PZ_NOGZIP))
		sfdcgzip(op, 0);
	if (pz->flags & PZ_NOPZIP)
		return 0;
	sfputc(op, PZ_MAGIC_1);
	sfputc(op, PZ_MAGIC_2);
	if (sfsync(op))
		return -1;
	sfputc(op, pz->major = PZ_MAJOR);
	sfputc(op, pz->minor = PZ_MINOR);
	sfputu(op, pz->win);
	if (pz->disc->comment)
	{
		sfputc(op, PZ_HDR_comment);
		m = strlen(pz->disc->comment) + 1;
		sfputu(op, m);
		sfwrite(op, pz->disc->comment, m);
	}
	if (pz->det && (m = sfstrtell(pz->det)))
	{
		sfputc(op, PZ_HDR_options);
		if (!(s = sfstruse(pz->det)))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		m++;
		sfputu(op, m);
		sfwrite(op, s, m);
	}
	if (i = pz->prefix.count)
	{
		sfputc(op, PZ_HDR_prefix);
		if (pz->prefix.terminator >= 0)
		{
			m = 0;
			while (i-- > 0)
			{
				if (!(s = sfgetr(pz->io, pz->prefix.terminator, 0)))
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 2, "%s: cannot read %I*u prefix record%s from data", pz->path, sizeof(pz->prefix.count), pz->prefix.count, pz->prefix.count == 1 ? "" : "s");
					return -1;
				}
				m += n = sfvalue(pz->io);
				sfwrite(pz->tmp, s, n);
			}
			s = sfstrseek(pz->tmp, 0, SEEK_SET);
		}
		else
		{
			m = i;
			if (!(s = (char*)sfreserve(pz->io, m, 0)))
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, 2, "%s: cannot read %I*u prefix byte%s from data", pz->path, sizeof(pz->prefix.count), pz->prefix.count, pz->prefix.count == 1 ? "" : "s");
				return -1;
			}
		}
		sfputu(op, m);
		sfwrite(op, s, m);
	}
	pz->flags |= PZ_HEAD;
	return pzpartwrite(pz, op);
}

/*
 * pretty print header info to op
 */

int
pzheadprint(register Pz_t* pz, register Sfio_t* op, int parts)
{
	register Pzpart_t*	pp;
	char			t;

	if (pz->flags & PZ_FORCE)
		sfprintf(op, "# fixed record input\n");
	else
	{
		sfprintf(op, "# pzip %d.%d partition\n", pz->major, pz->minor);
		if (pz->disc->comment)
			sfprintf(op, "# %s\n", pz->disc->comment);
		sfprintf(op, "# window %I*u\n", sizeof(pz->win), pz->win);
		if (pz->prefix.count)
		{
			sfprintf(op, "\nprefix=%I*u", sizeof(pz->prefix.count), pz->prefix.count);
			if (pz->prefix.terminator >= 0)
			{
				t = pz->prefix.terminator;
				sfprintf(op, "*%s\n", fmtquote(&t, "'", "'", 1, FMT_ALWAYS));
			}
			else
				sfputc(op, '\n');
		}
		if (pz->headoptions || pz->det)
		{
			sfputc(op, '\n');
			if (pz->headoptions)
				sfputr(op, pz->headoptions, '\n');
			if (pz->det)
			{
				sfwrite(op, sfstrbase(pz->det), sfstrtell(pz->det));
				sfputc(op, '\n');
			}
		}
	}
	if (parts)
	{
		pp = pz->partdict ? (Pzpart_t*)dtfirst(pz->partdict) : pz->part;
		while (pp)
		{
			if (pzpartprint(pz, pp, op))
				return -1;
			if (!pz->partdict)
				break;
			pp = (Pzpart_t*)dtnext(pz->partdict, pp);
		}
	}
	return sferror(op) ? -1 : 0;
}

/*
 * pzip files may be concatenated
 * pzfile() is called to determine if EOF has been reached
 * or if another file is concatenated
 * return:
 *	-1	error
 *	 0	EOF
 *	 1	another file found (and initialized in pz)
 */

int
pzfile(Pz_t* pz)
{
	unsigned char*	s;
	int		i;
	int		j;
	size_t		n;

	/*
	 * 0 or more nul's mean clean EOF
	 */

	while (!(i = sfgetc(pz->io)));
	if (pz->disc->errorf)
		(*pz->disc->errorf)(pz, pz->disc, -1, "%s: pzfile: i=%02x", pz->path, i);
	if (i == -1)
		return 0;
	if (i == PZ_MARK_TAIL)
	{
		/*
		 * file trailer
		 */

		while ((n = sfgetu(pz->io)) && !sferror(pz->io) && !sfeof(pz->io) && (s = (unsigned char*)sfreserve(pz->io, n, 0)))
			if (pz->disc->eventf && (*pz->disc->eventf)(pz, PZ_TAILREAD, s, n, pz->disc) < 0)
				return -1;
		if ((i = sfgetc(pz->io)) == -1)
			return 0;
	}
	j = sfgetc(pz->io);
	if (pz->disc->errorf)
		(*pz->disc->errorf)(pz, pz->disc, -1, "%s: pzfile: i=%02x j=%02x", pz->path, i, j);
	if (i == PZ_MAGIC_1 && j == PZ_MAGIC_2)
	{
		/*
		 * next file header
		 */

		sfungetc(pz->io, j);
		sfungetc(pz->io, i);
		return pzopen(pz->disc, (char*)pz, PZ_AGAIN) ? 1 : -1;
	}
	if (pz->disc->errorf)
		(*pz->disc->errorf)(pz, pz->disc, 2, "%s: data corrupted", pz->path);
	return -1;
}
