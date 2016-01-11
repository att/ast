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
 * pax xz format
 */

#include "format.h"

static int
xz_getprologue(Pax_t* pax, Format_t* fp, Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	off_t		pos;
	unsigned char	foot[6];

	if (size < 5 || buf[0] != 0xfd || buf[1] != '7' || buf[2] != 'z' || buf[3] != 'X' || buf[4] != 'Z')
		return 0;
	if ((pos = lseek(ap->io->fd, (off_t)0, SEEK_CUR)) < 0 || lseek(ap->io->fd, (off_t)-sizeof(foot), SEEK_END) <= 0 || read(ap->io->fd, foot, sizeof(foot)) != sizeof(foot) || foot[4] != 0x59 || foot[5] != 0x5A)
		return 0;
	else
		ap->uncompressed = (foot[0]) |
				   (foot[1] << 8) |
				   (foot[2] << 16) |
				   (foot[3] << 24);
	lseek(ap->io->fd, pos, SEEK_SET);
	return 1;
}

static Compress_format_t	pax_xz_data =
{
	0,
	{ "xz", "--decompress" },
	{ 0 },
};

Format_t	pax_xz_format =
{
	"xz",
	0,
	"xz compression",
	0,
	COMPRESS|IN|OUT,
	0,
	0,
	0,
	PAXNEXT(xz),
	&pax_xz_data,
	0,
	xz_getprologue,
};

PAXLIB(xz)
