/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*            Copyright (c) 2011 AT&T Intellectual Property             *
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
 * pax ico format
 */

#include <paxlib.h>
#include <ccode.h>
#include <swap.h>
#include <tm.h>

#define ICO_HEADER	6
#define ICO_DIRECTORY	16

#define ICO_MAGIC	0
#define ICO_TYPE	1

#define ICO_PNG_COLORS	0
#define ICO_PNG_PLANES	1
#define ICO_PNG_BPP	32

typedef struct Ico_header_s
{
	uint16_t		magic;
	uint16_t		type;
	uint16_t		count;
} Ico_header_t;

typedef struct Ico_directory_s
{
	uint8_t			width;
	uint8_t			height;
	uint8_t			colors;
	uint8_t			magic;
	uint16_t		panes;
	uint16_t		bpp;
	uint32_t		size;
	uint32_t		offset;
} Ico_directory_t;

typedef struct Ico_s
{
	Sfio_t*			head;
	Sfio_t*			data;
	unsigned char*		cur;
	int			mode;
	int			entries;
	char			name[32];
	unsigned char		dir[1];
} Ico_t;

static int
ico_done(Pax_t* pax, register Paxarchive_t* ap)
{
	register Ico_t*	ico = (Ico_t*)ap->data;

	if (!ico)
		return -1;
	if (ico->data)
		sfclose(ico->data);
	if (ico->head)
		sfclose(ico->head);
	free(ico);
	ap->data = 0;
	return 0;
}

static int
ico_getprologue(Pax_t* pax, Paxformat_t* fp, register Paxarchive_t* ap, Paxfile_t* f, register unsigned char* p, size_t size)
{
	register Ico_t*	ico;
	size_t		m;
	size_t		n;

	if (size < ICO_HEADER + ICO_DIRECTORY)
		return 0;
	if (swapget(3, p, 2) != ICO_MAGIC || swapget(3, p+2, 2) != ICO_TYPE || p[9] != 0 && p[9] != 255)
		return 0;
	if (paxseek(pax, ap, ICO_HEADER, SEEK_SET, 1) != ICO_HEADER)
		return 0;
	m = swapget(3, p+4, 2);
	n = m * ICO_DIRECTORY;
	if (!(ico = newof(0, Ico_t, 1, n - 1)))
		return paxnospace(pax);
	umask(ico->mode = umask(0));
	ico->mode = X_IFREG | (0666 & ~ico->mode);
	ico->entries = m;
	if (paxread(pax, ap, ico->dir, n, n, 0) <= 0)
	{
		free(ico);
		return 0;
	}
	ico->cur = ico->dir;
	ap->data = ico;
	return 1;
}

static int
ico_getheader(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f)
{
	register Ico_t*			ico = (Ico_t*)ap->data;
	register unsigned char*		p;
	int				width;
	int				height;
	size_t				offset;
	size_t				size;

	if (ap->entry > ico->entries)
	{
		ap->io->eof = 1;
		return 0;
	}
	p = ico->cur;
	ico->cur += ICO_DIRECTORY;
	width = p[0] ? p[0] : 256;
	height = p[1] ? p[1] : 256;
	size = swapget(3, p+8, 4);
	offset = swapget(3, p+12, 4);
	sfsprintf(ico->name, sizeof(ico->name), "ico-%d-c%dp%db%dw%dh%d.ico", ap->entry, p[2], (int)swapget(3, p+4, 2), (int)swapget(3, p+6, 2), width, height);
	f->name = ico->name;
	f->linktype = PAX_NOLINK;
	f->linkpath = 0;
	f->st->st_mode = ico->mode;
	f->st->st_uid = pax->uid;
	f->st->st_gid = pax->gid;
	f->st->st_size = size;
	f->st->st_mtime = time(NiL);
	f->st->st_nlink = 1;
	if (paxseek(pax, ap, offset, SEEK_SET, 1) != offset)
		return -1;
	return 1;
}

static int
ico_getdata(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f, int fd)
{
	register Ico_t*	ico = (Ico_t*)ap->data;
	Sfio_t*		sp;
	ssize_t		n;
	size_t		z;
	unsigned char*	dir;
	unsigned char	hdr[ICO_HEADER];

	if (fd < 0 || !f->st->st_size)
		return 1;
	if (!(sp = paxpart(pax, ap, f->st->st_size)))
		return -1;
	swapput(3, hdr+0, 2, ICO_MAGIC);
	swapput(3, hdr+2, 2, ICO_TYPE);
	swapput(3, hdr+4, 2, 1);
	sfwrite(sp, hdr, ICO_HEADER);
	if (paxdata(pax, ap, f, fd, hdr, ICO_HEADER))
		return -1;
	dir = ico->cur - ICO_DIRECTORY;
	swapput(3, dir+12, 4, ICO_HEADER + ICO_DIRECTORY);
	if (paxdata(pax, ap, f, fd, dir, ICO_DIRECTORY))
		return -1;
	while ((n = sfread(sp, pax->buf, sizeof(pax->buf))) > 0)
		if (paxdata(pax, ap, f, fd, pax->buf, n))
			return -1;
	if (n)
		(*pax->errorf)(NiL, pax, 2, "%s: %s: unexpected EOF", ap->name, f->name);
	return 1;
}

static int
ico_putprologue(Pax_t* pax, register Paxarchive_t* ap, int append)
{
	register Ico_t*	ico = (Ico_t*)ap->data;
	unsigned char	hdr[ICO_HEADER];

	if (!ico)
	{
		if (!(ico = newof(0, Ico_t, 1, 0)) || !(ico->head = sfstropen()) || !(ico->data = sftmp(64 * 1024)))
			return paxnospace(pax);
		swapput(3, hdr+0, 2, ICO_MAGIC);
		swapput(3, hdr+2, 2, ICO_TYPE);
		swapput(3, hdr+4, 2, 0);
		sfwrite(ico->head, hdr, ICO_HEADER);
		ap->data = ico;
	}
	return 1;
}

static int
ico_putheader(Pax_t* pax, Paxarchive_t* ap, Paxfile_t* f)
{
	return 1;
}

static int
ico_putdata(Pax_t* pax, Paxarchive_t* ap, Paxfile_t* f, int fd)
{
	register Ico_t*	ico = (Ico_t*)ap->data;
	Sfio_t*		sp;
	unsigned char*	p;
	int		d;
	ssize_t		n;
	size_t		z;
	uint32_t	size;
	unsigned char	dir[ICO_HEADER+ICO_DIRECTORY];

	if (!(sp = sfnew(NiL, NiL, SF_UNBOUND, fd, SF_READ)))
		return paxnospace(pax);
	d = 1;
	z = f->st->st_size;
	do
	{
		if ((n = sizeof(pax->buf)) > z)
			n = z;
		if ((n = sfread(sp, pax->buf, n)) <= 0)
		{
			if (pax->errorf)
				(*pax->errorf)(NiL, pax, 2, "%s: %s: read error", ap->name, f->name);
			return -1;
		}
		p = (unsigned char*)pax->buf;
		if (d)
		{
			if (n < ICO_HEADER + ICO_DIRECTORY)
			{
				if (pax->errorf)
					(*pax->errorf)(NiL, pax, 2, "%s: %s: read error", ap->name, f->name);
				return -1;
			}
			if (swapget(3, p+0, 2) == ICO_MAGIC && swapget(3, p+2, 2) == ICO_TYPE)
			{
				sfwrite(ico->head, p+ICO_HEADER, ICO_DIRECTORY);
				p += ICO_HEADER + ICO_DIRECTORY;
				n -= ICO_HEADER + ICO_DIRECTORY;
				z -= ICO_HEADER + ICO_DIRECTORY;
			}
			else if (!memcmp(p, "\x89PNG\x0d\x0a\x1a\x0a", 8))
			{
				dir[0] = (d = swapget(0, p+16, 4)) < 256 ? d : 0;
				dir[1] = (d = swapget(0, p+20, 4)) < 256 ? d : 0;
				dir[2] = ICO_PNG_COLORS;
				dir[3] = 0;
				swapput(3, dir+4, 2, ICO_PNG_PLANES);
				swapput(3, dir+6, 2, ICO_PNG_BPP);
				swapput(3, dir+8, 2, f->st->st_size);
				swapput(3, dir+12, 2, 0);
				sfwrite(ico->head, dir, ICO_DIRECTORY);
			}
			else
			{
				if (pax->errorf)
					(*pax->errorf)(NiL, pax, 2, "%s: %s: not an icon resource or image", ap->name, f->name);
				return -1;
			}
			d = 0;
		}
		sfwrite(ico->data, p, n);
	} while (z -= n);
	if (sfclose(sp))
	{
		if (pax->errorf)
			(*pax->errorf)(NiL, pax, 2, "%s: %s: write error", ap->name, f->name);
		return -1;
	}
	return n;
}

static off_t
ico_putepilogue(Pax_t* pax, Paxarchive_t* ap)
{
	register Ico_t*		ico = (Ico_t*)ap->data;
	register unsigned char*	p;
	unsigned char*		b;
	int			i;
	uint32_t		size;
	uint32_t		offset;

	b = p = (unsigned char*)sfstrbase(ico->head);
	swapput(3, p+4, 2, ap->entry);
	p += ICO_HEADER;
	size = offset = ICO_HEADER + ap->entries * ICO_DIRECTORY;
	for (i = 0; i < ap->entry; i++)
	{
		swapput(3, p+12, 4, offset);
		offset += swapget(3, p+8, 4);
		p += ICO_DIRECTORY;
	}
	paxwrite(pax, ap, b, size);
	sfseek(ico->data, 0, SEEK_SET);
	while ((i = sfread(ico->data, pax->buf, sizeof(pax->buf))) > 0)
		paxwrite(pax, ap, pax->buf, i);
	if (i < 0)
	{
		if (pax->errorf)
			(*pax->errorf)(NiL, pax, 2, "%s: write error", ap->name);
		return -1;
	}
	return 1;
}

Paxformat_t	pax_ico_format =
{
	"ico",
	0,
	"windows icon file",
	0,
	PAX_ARCHIVE|PAX_NOHARDLINKS|PAX_IN|PAX_OUT,
	PAX_DEFBUFFER,
	PAX_DEFBLOCKS,
	0,
	PAXNEXT(ico),
	0,
	ico_done,
	ico_getprologue,
	ico_getheader,
	ico_getdata,
	0,
	0,
	ico_putprologue,
	ico_putheader,
	ico_putdata,
	0,
	ico_putepilogue,
};

PAXLIB(ico)
