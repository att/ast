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
 * pax ar library format
 */

#include "format.h"

#include <ardir.h>

typedef struct Ar_s
{
	Ardir_t*		dir;
	Ardirent_t*		ent;
} Ar_t;

static int
ar_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, register File_t* f, unsigned char* buf, size_t size)
{
	Ar_t*		ar;
	Ardir_t*	dir;

	if (!(dir = ardiropen(ap->name, NiL, 0)))
		return 0;
	if (!(ar = newof(0, Ar_t, 1, 0)))
	{
		ardirclose(dir);
		nospace();
		return -1;
	}
	ap->data = ar;
	ar->dir = dir;
	ap->type = (char*)dir->meth->name;
	return 1;
}

static int
ar_done(Pax_t* pax, register Archive_t* ap)
{
	register Ar_t*	ar = (Ar_t*)ap->data;

	if (ar)
	{
		if (ar->dir)
			ardirclose(ar->dir);
		free(ar);
		ap->data = 0;
	}
	return 0;
}

static int
ar_getheader(Pax_t* pax, register Archive_t* ap, register File_t* f)
{
	register Ar_t*	ar = (Ar_t*)ap->data;
	off_t		pos;

	if (!(ar->ent = ardirnext(ar->dir)))
	{
		pos = lseek(ap->io->fd, (off_t)0, SEEK_END);
		return (pos < 0 || paxseek(pax, ap, pos, SEEK_SET, 0) != pos) ? -1 : 0;
	}
	f->name = ar->ent->name;
	f->st->st_dev = 0;
	f->st->st_ino = 0;
	f->st->st_mode = X_IFREG|(ar->ent->mode&X_IPERM);
	f->st->st_uid = ar->ent->uid;
	f->st->st_gid = ar->ent->gid;
	f->st->st_nlink = 1;
	IDEVICE(f->st, 0);
	f->st->st_mtime = ar->ent->mtime;
	f->st->st_size = ar->ent->size;
	f->linktype = NOLINK;
	f->linkpath = 0;
	f->uidname = 0;
	f->gidname = 0;
	return 1;
}

static int
ar_getdata(Pax_t* pax, register Archive_t* ap, register File_t* f, int wfd)
{
	register Ar_t*	ar = (Ar_t*)ap->data;

	if (ar->ent->offset < 0)
	{
		error(3, "%s: read not supported for %s format", f->name, ap->format->name);
		return -1;
	}
	if (wfd >= 0)
	{
		if (ardircopy(ar->dir, ar->ent, wfd) < 0)
		{
			error(ERROR_SYSTEM|2, "%s: copy error", f->name);
			return -1;
		}
		closeout(ap, f, wfd);
		setfile(ap, f);
	}
	return 1;
}

static int
ar_getepilogue(Pax_t* pax, Archive_t* ap)
{
	return 1;
}

Format_t	pax_ar_format =
{
	"ar",
	"library",
	"object library archive",
	0,
	ARCHIVE|NOHARDLINKS|IN,
	0,
	0,
	2,
	PAXNEXT(ar),
	0,
	ar_done,
	ar_getprologue,
	ar_getheader,
	ar_getdata,
	0,
	ar_getepilogue
};

PAXLIB(ar)
