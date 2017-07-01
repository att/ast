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
 * pax pds format
 */

#include "format.h"

#include <tm.h>

typedef struct Pdsdir_s
{
	char*		link;
	unsigned long	block;
	time_t		time;
	char		name[9];
} Pdsdir_t;

typedef struct Pds_s
{
	char*		format;
	unsigned char*	map;
	size_t		index;
	size_t		size;
	Pdsdir_t	dir[1];
} Pds_t;

static int
pds_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	register Pds_t*		pds;
	register unsigned char*	b;
	register unsigned char*	e;
	register size_t		n;
	register size_t		i;
	register size_t		m;
	size_t			links;

	if (size < 256 || buf[0] != 0)
		return 0;
	b = buf;
	e = b + b[1];
	while (b != e)
	{
		n = 12 + 2 * (b[11] & 0x1f);
		if ((e - b) < n)
			return 0;
		b += n;
	}
	i = 0;
	m = 32;
	if (!(pds = newof(0, Pds_t, 1, (m - 1) * sizeof(Pdsdir_t))))
		nospace();
	pds->map = ccmap(CC_EBCDIC_O, CC_NATIVE);
	links = 0;
	while ((b = (unsigned char*)paxget(pax, ap, -256, NiL)) && !b[0])
	{
		n = b[1];
		b += 2;
		e = b + n;
		while ((e - b) >= 12)
		{
			if (b[0] == 0xff && !memcmp(b, "\xff\xff\xff\xff\xff\xff\xff\xff", 8))
				goto done;
			n = 12 + 2 * (b[11] & 0x1f);
			if ((e - b) < n)
				break;
			if (i >= m)
			{
				m += 32;
				if (!(pds = newof(pds, Pds_t, 1, (m - 1) * sizeof(Pdsdir_t))))
					nospace();
			}
			memcpy(pds->dir[i].name, b, 8);
			ccmapstr(pds->map, pds->dir[i].name, 8);
			pds->dir[i].block = (b[8] << 16) | (b[9] << 8) | b[10];
			pds->dir[i].time = (n > 25) ? tmscan(sfprints("%02x %02x%01x %02x %02x", b[21], b[22], b[23] >> 4, b[24], b[25]), NiL, "%y %j %H %M", NiL, NiL, 0) : NOW;
			if (b[11] & 0x80)
			{
				links++;
				pds->dir[i].link = &pds->dir[i].name[0];
			}
			else
				pds->dir[i].link = 0;
			b += n;
			for (n = 8; n > 0 && pds->dir[i].name[n - 1] == ' '; n--);
			pds->dir[i].name[n] = 0;
			i++;
		}
	}
 done:
	if (!(pds->size = i))
	{
		free(pds);
		return 0;
	}
	m = 0;
	while (links > 0)
	{
		while (!pds->dir[m].link)
			m++;
		for (n = 0; n < i; n++)
			if (n != m && pds->dir[n].block == pds->dir[m].block)
				break;
		if (n < m)
		{
			pds->dir[n].link = pds->dir[m].name;
			pds->dir[m].link = 0;
		}
		else if (n < i)
			pds->dir[m].link = pds->dir[n].name;
		else
			pds->dir[m].link = 0;
		m++;
		links--;
	}
	pds->format = pax_pds_format.name;
	ap->data = pds;
	return 1;
}

static int
pds_done(Pax_t* pax, register Archive_t* ap)
{
	if (ap->data)
	{
		free(ap->data);
		ap->data = 0;
	}
	return 0;
}

static int
pds_getheader(Pax_t* pax, register Archive_t* ap, register File_t* f)
{
	register Pds_t*		pds = (Pds_t*)ap->data;
	register Pdsdir_t*	dp;

	if (pds->index >= pds->size)
		return 0;
	dp = pds->dir + pds->index++;
	f->name = dp->name;
	f->linkpath = dp->link;
	f->st->st_atime = f->st->st_mtime = dp->time;
	f->st->st_dev = 0;
	f->st->st_ino = 0;
	f->st->st_mode = X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IROTH;
	f->st->st_uid = state.uid;
	f->st->st_gid = state.gid;
	f->st->st_nlink = 1;
	IDEVICE(f->st, 0);
	f->st->st_size = dp->block;
	return 1;
}

static int
pds_getdata(Pax_t* pax, register Archive_t* ap, register File_t* f, int wfd)
{
	return 1;
}

Format_t	pax_pds_format =
{
	"pds",
	0,
	"mvs partitioned dataset",
	0,
	ARCHIVE|NOHARDLINKS,
	DEFBUFFER,
	DEFBLOCKS,
	0,
	PAXNEXT(pds),
	0,
	pds_done,
	pds_getprologue,
	pds_getheader,
	pds_getdata,
};

PAXLIB(pds)
