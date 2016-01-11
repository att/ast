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
 *
 * pax pax external library support
 */

#include "pax.h"

#undef	paxchecksum
#undef	paxcorrupt
#undef	paxdata
#undef	paxget
#undef	paxnext
#undef	paxnospace
#undef	paxpart
#undef	paxput
#undef	paxread
#undef	paxseek
#undef	paxstash
#undef	paxsync
#undef	paxunread
#undef	paxwrite

static const char	null[] = "";
static const char	sep[] = ": ";

#define TXT(s,m)	(s)?sep:null,(s)?(m):null

static int
paxdata(Pax_t* pax, Paxarchive_t* ap, Paxfile_t* f, int fd, void* b, off_t n)
{
	return holewrite(fd, b, n) == n ? 0 : -1;
}

static void*
paxget(Pax_t* pax, Paxarchive_t* ap, off_t n, off_t* p)
{
	return bget(ap, n, p);
}

static int
paxput(Pax_t* pax, Paxarchive_t* ap, off_t n)
{
	bput(ap, n);
	return 0;
}

static off_t
paxread(Pax_t* pax, Paxarchive_t* ap, void* b, off_t n, off_t m, int must)
{
	return bread(ap, b, n, m, must);
}

static off_t
paxseek(Pax_t* pax, Paxarchive_t* ap, off_t pos, int op, int hard)
{
	return bseek(ap, pos, op, hard);
}

static char*
paxstash(Pax_t* pax, Value_t* v, const char* s, size_t z)
{
	return stash(v, s, z);
}

static int
paxsync(Pax_t* pax, Paxarchive_t* ap, int hard)
{
	if (ap->io->mode == O_RDONLY)
		bflushin(ap, hard);
	else
		bflushout(ap);
	return 0;
}

static int
paxunread(Pax_t* pax, Paxarchive_t* ap, void* b, off_t n)
{
	bunread(ap, b, n);
	return 0;
}

static int
paxwrite(Pax_t* pax, Paxarchive_t* ap, const void* b, off_t n)
{
	bwrite(ap, (void*)b, n);
	return 0;
}

static int
paxnext(Pax_t* pax, Paxarchive_t* ap, size_t c, size_t n)
{
	newio(ap, c, n);
	return 0;
}

static int
paxcorrupt(Pax_t* pax, Paxarchive_t* ap, Paxfile_t* f, const char* msg)
{
	(*pax->errorf)(NiL, pax, 2, "%s%s%s: %s archive corrupt at %I*u%s%s", ap->name, TXT(f, f->name), ap->format->name, sizeof(off_t), paxseek(pax, ap, 0, SEEK_CUR, 0), TXT(msg, msg));
	return -1;
}

static int
paxchecksum(Pax_t* pax, Paxarchive_t* ap, Paxfile_t* f, unsigned long expected, unsigned long value)
{
	int	z;

	if (expected != value)
	{
		z = ((expected | value) & 0xffff0000) ? 8 : 4;
		(*pax->errorf)(NiL, pax, 2, "%s%s%s: %s archive checksum error -- expected %0*lx != %0*lx", ap->name, TXT(f, f->name), ap->format->name, z, expected, z, value);
		return -1;
	}
	return 0;
}

static int
paxnospace(Pax_t* pax)
{
	(*pax->errorf)(NiL, pax, 2, "out of space");
	return -1;
}

/*
 * archive part sfio discipline
 */

static ssize_t
part_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register Part_t*	part = (Part_t*)disc;
	ssize_t			r;

	if (part->n <= 0)
		return part->n;
	if (n > part->n)
		n = part->n;
	if ((r = paxread(part->pax, part->ap, buf, n, 0, 0)) > 0)
		part->n -= r;
	return r;
}

static ssize_t
part_write(Sfio_t* sp, const void* buf, size_t n, Sfdisc_t* disc)
{
	register Part_t*	part = (Part_t*)disc;
	ssize_t			r;

	if ((r = paxwrite(part->pax, part->ap, buf, n)) > 0)
		part->n += r;
	return r;
}

static Sfio_t*
paxpart(Pax_t* pax, Paxarchive_t* ap, off_t n)
{
	register Part_t*	part;

	static int		fd = -1;

	if (!(part = ap->partio))
	{
		if (!(part = newof(0, Part_t, 1, 0)) || !(part->sp = sfstropen()))
		{
			paxnospace(pax);
			return 0;
		}
		part->sp->_flags &= ~(SF_READ|SF_WRITE|SF_STRING);
		if (ap->flags & PAX_IN)
			part->sp->_flags |= SF_READ;
		else
			part->sp->_flags |= SF_WRITE;
		if (fd < 0)
			fd = open("/dev/null", O_RDWR);
		part->sp->_file = fd;
		part->disc.readf = part_read;
		part->disc.writef = part_write;
		if (sfdisc(part->sp, &part->disc) != &part->disc)
		{
			sfclose(part->sp);
			free(part);
			return 0;
		}
		part->pax = pax;
		part->ap = ap;
		ap->partio = part;
	}
	part->n = n;
	return part->sp;
}

/*
 * initialize the external library callbacks
 */

void
paxinit(register Pax_t* pax, const char* id)
{
	pax->id = id;
	pax->errorf = errorf;

	pax->dataf = paxdata;
	pax->getf = paxget;
	pax->partf = paxpart;
	pax->putf = paxput;
	pax->readf = paxread;
	pax->seekf = paxseek;
	pax->stashf = paxstash;
	pax->syncf = paxsync;
	pax->unreadf = paxunread;
	pax->writef = paxwrite;

	pax->corruptf = paxcorrupt;
	pax->checksumf = paxchecksum;
	pax->nospacef = paxnospace;

	pax->nextf = paxnext;
}
