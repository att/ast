/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2012 AT&T Intellectual Property          *
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
 * pax gzip format
 */

#include "format.h"

static int
gzip_getprologue(Pax_t* pax, Format_t* fp, Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	off_t		pos;
	unsigned char	num[4];

	if (size < 2 || buf[0] != 0x1f || buf[1] != 0x8b)
		return 0;
	if ((pos = lseek(ap->io->fd, (off_t)0, SEEK_CUR)) < 0 || lseek(ap->io->fd, (off_t)-4, SEEK_END) <= 0 || read(ap->io->fd, num, sizeof(num)) != sizeof(num))
		ap->uncompressed = ap->io->size * 6;
	else
		ap->uncompressed = (num[0]) |
				   (num[1] << 8) |
				   (num[2] << 16) |
				   (num[3] << 24);
	lseek(ap->io->fd, pos, SEEK_SET);
	return 1;
}

static Compress_format_t	pax_gzip_data =
{
	"-9",
	{ "gunzip" },
	{ "ratz", "-c" },
};

Format_t	pax_gzip_format =
{
	"gzip",
	"gz",
	"gzip compression",
	0,
	COMPRESS|IN|OUT,
	0,
	0,
	0,
	PAXNEXT(gzip),
	&pax_gzip_data,
	0,
	gzip_getprologue,
};

PAXLIB(gzip)
