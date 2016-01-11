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
 * pax tnef format
 */

#include "format.h"

#include <tm.h>

#define TNEF_MAGIC	0x223e9f78

typedef struct Tnef_s
{
	char*		format;
	off_t		offset;
} Tnef_t;

static int
tnef_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	Tnef_t*		tnef;
	uint32_t	magic;

	if (size < 6)
		return 0;
	magic = TNEF_MAGIC;
	if ((ap->swap = swapop(&magic, buf, 4)) < 0 || paxread(pax, ap, NiL, (off_t)6, (off_t)6, 0) != 6)
		return 0;
	if (!(tnef = newof(0, Tnef_t, 1, 0)))
		nospace();
	ap->data = (void*)tnef;
	ap->swap = 3;
	return 1;
}

static int
tnef_getheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	Tnef_t*			tnef = (Tnef_t*)ap->data;
	char*			s;
	char*			e;
	unsigned char*		x;
	size_t			n;
	size_t			m;
	size_t			z;
	unsigned int		level;
	unsigned int		type;
	unsigned int		name;
	unsigned int		checksum;
	size_t			size;
	size_t			part;
	Tm_t			tm;

	*(f->name = paxstash(pax, &ap->stash.head, NiL, PATH_MAX)) = 0;
	f->st->st_dev = 0;
	f->st->st_ino = 0;
	f->st->st_mode = X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IROTH;
	f->st->st_uid = state.uid;
	f->st->st_gid = state.gid;
	f->st->st_nlink = 1;
	IDEVICE(f->st, 0);
	f->st->st_mtime = f->st->st_ctime = f->st->st_atime = NOW;
	f->st->st_size = 0;
	while (paxread(pax, ap, s = state.tmp.buffer, (off_t)0, (off_t)9, 0) > 0)
	{
		level = swapget(ap->swap, s + 0, 1);
		n = swapget(ap->swap, s + 1, 4);
		type = n >> 16;
		name = n & ((1<<16)-1);
		n = 0;
		size = swapget(ap->swap, s + 5, 4);
		part = ap->io->buffersize;
		if (size > part)
		{
			if (level == 2)
			{
				level = 0;
				if (name == 0x800f)
				{
					f->st->st_size = size;
					tnef->offset = paxseek(pax, ap, (off_t)0, SEEK_CUR, 0);
				}
				else
					error(1, "%s: %s format 0x%04x attribute ignored", ap->name, ap->format->name, name);
			}
			while (size > part)
			{
				size -= part;
				if (!(s = paxget(pax, ap, part, NiL)))
					error(3, "%s: %s format 0x%04x attribute data truncated -- %d expected", ap->name, ap->format->name, name, part);
				x = (unsigned char*)s;
				e = s + part;
				while (x < (unsigned char*)e)
					n += *x++;
			}
		}
		if (!(s = paxget(pax, ap, size + 2, NiL)))
			error(3, "%s: %s format 0x%04x attribute data truncated -- %d expected", ap->name, ap->format->name, name, size);
		checksum = swapget(ap->swap, s + size, 2);
		x = (unsigned char*)s;
		e = s + size;
		while (x < (unsigned char*)e)
			n += *x++;
		n &= ((1<<16)-1);
		if (checksum != n)
			error(3, "%s: %s format attribute data checksum error", ap->name, ap->format->name);
		if (level == 2)
			switch (name)
			{
			case 0x800f: /* data */
				f->st->st_size = size;
				tnef->offset = paxseek(pax, ap, (off_t)0, SEEK_CUR, 0) - size - 2;
				break;
			case 0x8010: /* title */
				if (!f->name)
					f->name = paxstash(pax, &ap->stash.head, s, size);
				break;
			case 0x8013: /* date */
				if (type == 0x0003 && size >= 12)
				{
					tm.tm_year = swapget(ap->swap, s +  0, 2) - 1900;
					tm.tm_mon  = swapget(ap->swap, s +  2, 2) - 1;
					tm.tm_mday = swapget(ap->swap, s +  4, 2);
					tm.tm_hour = swapget(ap->swap, s +  6, 2);
					tm.tm_min  = swapget(ap->swap, s +  8, 2);
					tm.tm_sec  = swapget(ap->swap, s + 10, 2);
					f->st->st_mtime = tmtime(&tm, TM_LOCALZONE);
				}
				break;
			case 0x9005: /* attachment */
				if (size >= 4)
				{
					e = s + size - 4;
					n = swapget(ap->swap, s, 4); s += 4;
					while (n-- && s < e)
					{
						type = swapget(ap->swap, s, 2); s += 2;
						name = swapget(ap->swap, s, 2); s += 2;
						switch (type)
						{
						case 0x0002:
						case 0x000b:
							s += 2;
							break;
						case 0x0003:
						case 0x0004:
						case 0x0007:
						case 0x000a:
						case 0x000d:
						case 0x0048:
							s += 4;
							break;
						case 0x0005:
						case 0x0006:
						case 0x0014:
						case 0x0040:
							s += 8;
							break;
						case 0x001e:
						case 0x001f:
						case 0x0102:
							m = swapget(ap->swap, s, 4); s += 4;
							while (m--)
							{
								z = swapget(ap->swap, s, 4); s += 4;
								z = roundof(z, 4);
								if (name == 0x3707)
									f->name = paxstash(pax, &ap->stash.head, s, z);
								s += z;
							}
							break;
						}
					}
				}
				n = tnef->offset;
				tnef->offset = paxseek(pax, ap, (off_t)0, SEEK_CUR, 0);
				if (paxseek(pax, ap, n, SEEK_SET, 1) != n)
					error(3, "%s: %s input must be seekable", ap->name, ap->format->name);
				if (!*f->name)
				{
					if (s = strrchr(ap->name, '/'))
						s++;
					else
						s = ap->name;
					if (!(e = strrchr(s, '.')) || isdigit(*(e + 1)))
						e = s + strlen(s);
					z = sfsprintf(state.tmp.buffer, state.tmp.buffersize, "%-.*s.%03u", e - s, s, ap->entry);
					f->name = paxstash(pax, &ap->stash.head, state.tmp.buffer, z);
				}
				return 1;
			}
	}
	if (ap->io->eof && ap->entry == 1)
		error(1, "%s: no members in %s file", ap->name, ap->format->name);
	return 0;
}

static int
tnef_done(Pax_t* pax, register Archive_t* ap)
{
	if (ap->data)
	{
		free(ap->data);
		ap->data = 0;
	}
	return 0;
}

static int
tnef_gettrailer(Pax_t* pax, Archive_t* ap, File_t* f)
{
	register Tnef_t*	tnef = (Tnef_t*)ap->data;

	if (paxseek(pax, ap, tnef->offset, SEEK_SET, 1) != tnef->offset)
		error(3, "%s: %s format seek error", ap->name, ap->format->name);
	return 1;
}

Format_t	pax_tnef_format =
{
	"tnef",
	0,
	"MS outlook transport neutral encapsulation format",
	0,
	ARCHIVE|DOS|NOHARDLINKS|IN,
	DEFBUFFER,
	DEFBLOCKS,
	0,
	PAXNEXT(tnef),
	0,
	tnef_done,
	tnef_getprologue,
	tnef_getheader,
	0,
	tnef_gettrailer,
};

PAXLIB(tnef)
