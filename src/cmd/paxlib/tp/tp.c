/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2004-2011 AT&T Intellectual Property          *
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
 * pax tp format
 */

#include <paxlib.h>
#include <ccode.h>
#include <tm.h>

#define TP_BLOCK	512

#define DIRDEC		192
#define DIRMAG		496

typedef  uint8_t ui1;
typedef uint16_t ui2;
typedef uint32_t ui4;

typedef struct Dir_s
{
	char	pathname[32];
	ui1	mode[2];
	ui1	uid;
	ui1	gid;
	ui1	unused1;
	ui1	size[3];
	ui1	modtime[4];
	ui1	tapeaddr[2];
	ui1	unused2[16];
	ui1	checksum[2];
} Dir_t;

typedef struct Ar_s
{
	Pax_t*			pax;
	Paxarchive_t*		ap;
	off_t			offset;
	Dir_t*			dp;
	Dir_t*			ep;
	Dir_t			dir[1];
} Ar_t;

static int
tp_done(Pax_t* pax, register Paxarchive_t* ap)
{
	register Ar_t*	ar = (Ar_t*)ap->data;

	if (!ar)
		return -1;
	ap->io->eof = 1;
	free(ar);
	ap->data = 0;
	return 0;
}

static int
tp_getprologue(Pax_t* pax, Paxformat_t* fp, register Paxarchive_t* ap, Paxfile_t* f, unsigned char* buf, size_t size)
{
	register Ar_t*		ar;
	register ui1*		s;
	register ui1*		e;
	int16_t			w;
	size_t			n;

	if (size < 2 * TP_BLOCK)
		return 0;
	w = 0;
	for (e = (s = (ui1*)buf + TP_BLOCK) + TP_BLOCK; s < e; s += 2)
		w += s[0] + (s[1]<<8);
	if (w)
		return 0;
	w = ((Dir_t*)buf)->checksum[0] + (((Dir_t*)buf)->checksum[1]<<8);
	if (w < 0 || w > DIRMAG)
		w = DIRMAG;
	n = (size_t)w * sizeof(Dir_t);
	if (!(ar = newof(0, Ar_t, 1, n - sizeof(Dir_t))))
		return paxnospace(pax);
	if (paxread(pax, ap, NiL, PAX_BLOCK, 0, 0) != PAX_BLOCK || paxread(pax, ap, ar->dir, n, 0, 0) != n)
	{
		error(2, "%s: %s format directory read error", ap->name, fp->name);
		free(ar);
		return -1;
	}
	ar->dp = ar->dir;
	ar->ep = ar->dir + w;
	ar->pax = pax;
	ar->ap = ap;
	ap->data = ar;
	return 1;
}

static int
tp_getheader(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f)
{
	register Ar_t*		ar = (Ar_t*)ap->data;
	register Dir_t*		dp;
	register ui1*		s;
	register ui1*		e;
	int16_t			w;

	do
	{
		if (ar->dp >= ar->ep)
			return 0;
		dp = ar->dp++;
	} while (!*dp->pathname);
	w = 0;
	for (e = (s = (ui1*)ar->dir) + sizeof(Dir_t); s < e; s += 2)
		w += s[0] + (s[1]<<8);
	if (w)
	{
		error(2, "%s: %s format directory entry %d checksum error", ap->name, ap->format->name, ar->dp - ar->dir);
		return -1;
	}
	if (CC_NATIVE != CC_ASCII)
		ccmaps(dp->pathname,  sizeof(dp->pathname), CC_ASCII, CC_NATIVE);
	f->name = dp->pathname;
	f->linktype = PAX_NOLINK;
	f->linkpath = 0;
	f->st->st_mode = X_IFREG | ((dp->mode[0] + (dp->mode[1]<<8)) & 07777);
	f->st->st_uid = dp->uid;
	f->st->st_gid = dp->gid;
	f->st->st_size = (dp->size[0]<<16) + dp->size[1] + (dp->size[2]<<8);
	f->st->st_mtime = dp->modtime[2] + (dp->modtime[3]<<8) + (dp->modtime[0]<<16) + (dp->modtime[1]<<24);
	f->st->st_nlink = 1;
	ar->offset = (off_t)(dp->tapeaddr[0] + (dp->tapeaddr[1]<<8)) * TP_BLOCK;
	return 1;
}

static int
tp_getdata(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f, int fd)
{
	Sfio_t*		sp;
	off_t		skip;
	ssize_t		n;
	int		r;

	if (!(n = f->st->st_size))
		return 1;
	skip = roundof(f->st->st_size, TP_BLOCK);
	r = -1;
	if (fd < 0)
		r = 1;
	else if (sp = paxpart(pax, ap, f->st->st_size))
		for (;;)
		{
			if ((n = sfread(sp, pax->buf, sizeof(pax->buf))) < 0)
			{
				(*pax->errorf)(NiL, pax, 2, "%s: %s: unexpected EOF", ap->name, f->name);
				break;
			}
			else if (n == 0)
			{
				r = 1;
				break;
			}
			skip -= n;
			if (paxdata(pax, ap, f, fd, pax->buf, n))
				break;
		}
	if (skip && paxread(pax, ap, NiL, skip, 0, 0) != skip)
	{
		(*pax->errorf)(NiL, pax, 2, "%s: %s: cannot skip past %s format data", ap->name, f->name, ap->format->name);
		r = -1;
	}
	return r;
}

Paxformat_t	pax_tp_format =
{
	"tp",
	0,
	"unix 4th-7th edition PDP-11 tp archive",
	0,
	PAX_ARCHIVE|PAX_NOHARDLINKS|PAX_IN,
	PAX_DEFBUFFER,
	PAX_DEFBLOCKS,
	PAX_BLOCK,
	PAXNEXT(tp),
	0,
	tp_done,
	tp_getprologue,
	tp_getheader,
	tp_getdata,
};

PAXLIB(tp)
