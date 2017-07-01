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
 * pax compress format
 */

#include "format.h"

static int
compress_getprologue(Pax_t* pax, Format_t* fp, Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	if (size < 2 || buf[0] != 0x1f || buf[1] != 0x9d)
		return 0;
	ap->uncompressed = ap->io->size * 3;
	return 1;
}

static Compress_format_t	pax_compress_data =
{
	0,
	{ 0 },
	{ "zcat" },
};

Format_t	pax_compress_format =
{
	"compress",
	"lz",
	"Lempel-Ziv compression",
	0,
	COMPRESS|IN|OUT,
	0,
	0,
	0,
	PAXNEXT(compress),
	&pax_compress_data,
	0,
	compress_getprologue,
};

PAXLIB(compress)
