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
 * pax vdb format
 */

#include "format.h"

#include <vdb.h>

typedef struct Vdb_s
{
	unsigned char	delimiter;	/* header delimiter		*/
	unsigned char	variant;	/* variant header delimiters	*/
	Sfio_t*		directory;	/* directory trailer stream	*/
	struct stat	st;		/* member stat prototype	*/
	struct
	{
	char*		base;		/* header buffer base		*/
	char*		next;		/* next header			*/
	}		header;
} Vdb_t;

static int
vdb_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	register Vdb_t*		vdb = (Vdb_t*)ap->data;
	char*			s;
	char*			t;
	struct stat		st;
	size_t			i;
	off_t			n;

	if (fstat(ap->io->fd, &st))
		return 0;
	s = state.tmp.buffer;
	n = sizeof(VDB_MAGIC) + sizeof(state.volume) + 1;
	if (size < n)
		return 0;
	s = (char*)buf;
	if (s[0] != VDB_DELIMITER || !strneq(s + 1, VDB_MAGIC, sizeof(VDB_MAGIC) - 1) || s[sizeof(VDB_MAGIC)] != VDB_DELIMITER)
		return 0;
	if (s[sizeof(VDB_MAGIC) + 1] != '\n')
	{
		s[n] = 0;
		if (t = strchr(s, '\n'))
			*t = 0;
		strncpy(state.volume, s + sizeof(VDB_MAGIC) + 1, sizeof(state.volume) - 2);
	}
	if (lseek(ap->io->fd, (off_t)(-(VDB_LENGTH + 1)), SEEK_END) <= 0)
		return 0;
	if (read(ap->io->fd, s, VDB_LENGTH + 1) != (VDB_LENGTH + 1))
		return 0;
	if (!vdb)
	{
		if (!(vdb = newof(0, Vdb_t, 1, 0)))
			nospace();
		ap->data = vdb;
	}
	ccmapstr(state.map.a2n, s, VDB_LENGTH + 1);
	vdb->variant = *s++ != '\n';
	if (!strneq(s, VDB_DIRECTORY, sizeof(VDB_DIRECTORY) - 1))
		goto nope;
	vdb->delimiter = s[VDB_OFFSET - 1];
	n = strtol(s + VDB_OFFSET, NiL, 10) - sizeof(VDB_DIRECTORY);
	i = lseek(ap->io->fd, (off_t)0, SEEK_CUR) - n - VDB_LENGTH - vdb->variant;
	if (!(vdb->header.base = newof(0, char, i, 1)))
		nospace();
	if (lseek(ap->io->fd, n, SEEK_SET) != n)
		goto nope;
	if (read(ap->io->fd, vdb->header.base, i) != i)
		goto nope;
	ccmapstr(state.map.a2n, vdb->header.base, i);
	*(vdb->header.base + i) = 0;
	if (!strneq(vdb->header.base, VDB_DIRECTORY, sizeof(VDB_DIRECTORY) - 1))
		goto nope;
	if (!(vdb->header.next = strchr(vdb->header.base, '\n')))
		goto nope;
	vdb->header.next++;
	vdb->st.st_mode = modex(vdb->st.st_mode);
	return 1;
 nope:
	free(vdb);
	return 0;
}

static int
vdb_done(Pax_t* pax, register Archive_t* ap)
{
	register Vdb_t*	vdb = (Vdb_t*)ap->data;

	if (vdb)
	{
		if (vdb->directory)
			sfclose(vdb->directory);
		free(vdb);
		ap->data = 0;
	}
	return 0;
}

static int
vdb_getheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	register Vdb_t*	vdb = (Vdb_t*)ap->data;
	char*		s;
	char*		t;
	off_t		n;

	t = vdb->header.next;
	if (!(vdb->header.next = strchr(t, '\n')))
		goto eof;
	*vdb->header.next++ = 0;
	message((-1, "VDB: next=`%s'", t));
	if (vdb->variant)
		vdb->delimiter = *t++;
	f->name = t;
	if (!(t = strchr(t, vdb->delimiter)))
		goto eof;
	*t++ = 0;
	n = strtol(t, &t, 10);
	if (*t++ != vdb->delimiter)
		goto eof;
	if (lseek(ap->io->fd, n, SEEK_SET) != n)
		goto eof;
	*f->st = vdb->st;
	f->st->st_size = strtol(t, &t, 10);
	if (*t++ == vdb->delimiter) do
	{
		if (s = strchr(t, vdb->delimiter))
			*s++ = 0;
		if (strneq(t, VDB_DATE, sizeof(VDB_DATE) - 1))
			f->st->st_mtime = strtol(t + sizeof(VDB_DATE), NiL, 10);
		else if (strneq(t, VDB_MODE, sizeof(VDB_MODE) - 1))
			f->st->st_mode = (strtol(t + sizeof(VDB_MODE), NiL, 8) & X_IPERM) | X_IFREG;
	} while (t = s);
	f->st->st_dev = 0;
	f->st->st_ino = 0;
	f->st->st_nlink = 1;
	IDEVICE(f->st, 0);
	f->linktype = NOLINK;
	f->linkpath = 0;
	f->uidname = 0;
	f->gidname = 0;
	paxsync(pax, ap, 0);
	return 1;
 eof:
	paxsync(pax, ap, 0);
	ap->io->eof = 1;
	ap->io->offset = 0;
	ap->io->count = lseek(ap->io->fd, (off_t)0, SEEK_END);
	return 0;
}

static int
vdb_putprologue(Pax_t* pax, register Archive_t* ap, int append)
{
	register Vdb_t*	vdb = (Vdb_t*)ap->data;

	if (!vdb)
	{
		if (!(vdb = newof(0, Vdb_t, 1, 0)) || !(vdb->directory = sfstropen()))
			nospace();
		ap->data = vdb;
	}
	if (append)
		return 0;
	sfprintf(vdb->directory, "%c%s%c%s\n", VDB_DELIMITER, VDB_MAGIC, VDB_DELIMITER, state.volume);
	paxwrite(pax, ap, sfstrbase(vdb->directory), sfstrtell(vdb->directory));
	sfstrseek(vdb->directory, 0, SEEK_SET);
	sfprintf(vdb->directory, "%s\n", VDB_DIRECTORY);
	return 1;
}

static int
vdb_putheader(Pax_t* pax, Archive_t* ap, File_t* f)
{
	register Vdb_t*		vdb = (Vdb_t*)ap->data;
	register unsigned char*	s;
	register int		c;
	int			n;

	if (state.complete)
		complete(ap, f, state.record.header ? state.record.headerlen : f->namesize);
	if (state.record.header)
		paxwrite(pax, ap, state.record.header, state.record.headerlen);
	else
	{
		f->name[f->namesize - 1] = '\n';
		paxwrite(pax, ap, f->name, f->namesize);
		f->name[f->namesize - 1] = 0;
	}
	if (!(c = state.record.delimiter))
	{
		c = VDB_DELIMITER;
		s = (unsigned char*)state.tmp.buffer;
		n = 1024;
		if (f->fd >= 0)
		{
			n = read(f->fd, s, n);
			if (n > 0)
			{
				int		dp;
				int		ds;
				int		mp = 0;
				int		ms = 0;
				unsigned char	hit[UCHAR_MAX + 1];

				/*
				 * to determine the delimiter, if any
				 */

				lseek(f->fd, -n, SEEK_CUR);
				memzero(hit, sizeof(hit));
				while (--n > 0)
					hit[*(s + n)]++;
				for (n = 0; n <= UCHAR_MAX; n++)
					if (n == '_' || n == '/' || n == '.' || n == '\n' || n == '\\')
						/* nop */;
					else if (isspace(n))
					{
						if ((int)hit[n] > ms)
						{
							ms = hit[n];
							ds = n;
						}
					}
					else if ((int)hit[n] > mp && isprint(n) && !isalnum(n))
					{
						mp = hit[n];
						dp = n;
					}
				if (mp)
					c = dp;
				else if (ms)
					c = ds;
			}
		}
	}
	n = (c == '=') ? ':' : '=';
	sfprintf(vdb->directory, "%c%s%c%I*u%c%I*u%c%s%c%I*u%c%s%c0%o\n", c, f->name, c, sizeof(ap->io->offset), ap->io->offset + ap->io->count, c, sizeof(f->st->st_size), f->st->st_size, c, VDB_DATE, n, sizeof(f->st->st_mtime), f->st->st_mtime, c, VDB_MODE, n, modex(f->st->st_mode & X_IPERM));
	return 1;
}

static int
vdb_puttrailer(Pax_t* pax, Archive_t* ap, File_t* f)
{
	if (state.record.trailer)
		paxwrite(pax, ap, state.record.trailer, state.record.trailerlen);
	return 0;
}

static off_t
vdb_putepilogue(Pax_t* pax, Archive_t* ap)
{
	register Vdb_t*	vdb = (Vdb_t*)ap->data;

	if (state.record.header)
		paxwrite(pax, ap, state.record.header, state.record.headerlen);
	sfprintf(vdb->directory, "%c%s%c%0*I*d%c%0*I*d\n", VDB_DELIMITER, VDB_DIRECTORY, VDB_DELIMITER, VDB_FIXED, sizeof(ap->io->offset), ap->io->offset + ap->io->count + sizeof(VDB_DIRECTORY), VDB_DELIMITER, VDB_FIXED, sizeof(Sfoff_t), sftell(vdb->directory) - sizeof(VDB_DIRECTORY) + VDB_LENGTH + 1);
	paxwrite(pax, ap, sfstrbase(vdb->directory), sfstrtell(vdb->directory));
	sfstrclose(vdb->directory);
	return ap->io->count;
}

Format_t	pax_vdb_format =
{
	"vdb",
	0,
	"virtual database",
	0,
	ARCHIVE|NOHARDLINKS|IN|OUT,
	DEFBUFFER,
	DEFBLOCKS,
	0,
	PAXNEXT(vdb),
	0,
	vdb_done,
	vdb_getprologue,
	vdb_getheader,
	0,
	0,
	0,
	vdb_putprologue,
	vdb_putheader,
	0,
	vdb_puttrailer,
	vdb_putepilogue,
};

PAXLIB(vdb)
