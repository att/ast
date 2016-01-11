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
 * pax solaris flash format
 */

#include "format.h"

#define FLASH_MAGIC	"FlAsH-aRcHiVe-"
#define FLASH_DATA	"section_begin=archive"

static int
flash_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	unsigned char*	s;
	unsigned char*	e;
	unsigned char*	t;
	char*		v;
	int		i;
	off_t		n;
	char		version[16];

	if (size < sizeof(FLASH_MAGIC) - 1 || memcmp(buf, FLASH_MAGIC, sizeof(FLASH_MAGIC) - 1))
		return 0;

	/*
	 * get the flash format version
	 */

	s = buf + sizeof(FLASH_MAGIC) - 1;
	e = buf + size;
	v = version;
	while (v < &version[sizeof(version) - 1] && s < e && *s != '\n')
		*v++ = *s++;
	*v = 0;

	/*
	 * skip over the flash headers to the embedded archive
	 */

	s = e;
	for (;;)
	{
		if (s >= e)
		{
			if (!(buf = (unsigned char*)paxget(pax, ap, -PAX_DEFBUFFER * PAX_BLOCK, &n)))
				return -1;
			s = buf;
			e = buf + n;
		}
		if (t = (unsigned char*)memchr(s, '\n', e - s))
		{
			if ((t - s) == (sizeof(FLASH_DATA) - 1) && !memcmp(s, FLASH_DATA, sizeof(FLASH_DATA) - 1))
			{
				if (paxseek(pax, ap, -(e - t - 1), SEEK_CUR, 0) < 0)
					return -1;
				break;
			}
			if (t < e)
			{
				s = t + 1;
				continue;
			}
		}
		if (paxseek(pax, ap, -(e - s), SEEK_CUR, 0) < 0)
			return -1;
		s = e;
	}
	ap->entry = 0;
	ap->swap = 0;
	ap->swapio = 0;
	ap->volume--;
	i = state.volume[0];
	if (getprologue(ap) <= 0)
	{
		error(2, "%s: %s format embedded archive expected", ap->name, fp->name);
		return -1;
	}
	state.volume[0] = i;
	ap->package = strdup(sfprints("%s %s", fp->name, version));
	return 1;
}

Format_t	pax_flash_format =
{
	"flash",
	0,
	"Solaris flash package encapsulated archive",
	0,
	ARCHIVE|IN,
	DEFBUFFER,
	DEFBLOCKS,
	0,
	PAXNEXT(flash),
	0,
	0,
	flash_getprologue,
};

PAXLIB(flash)
