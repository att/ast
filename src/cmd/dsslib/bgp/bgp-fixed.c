/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
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
 * bgp fixed method
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "bgplib.h"

#include <magicid.h>

#define MAGIC_NAME		"bgp"
#define MAGIC_TYPE		"router"
#define MAGIC_VERSION		20020311L

typedef union Fixedheader_u
{
	Magicid_t		magic;
	Bgproute_t		route;
} Fixedheader_t;

typedef struct Fixedstate_s
{
	int			swap;
} Fixedstate_t;

/*
 * fixed identf
 */

static int
fixedident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	Magicid_t*	mp = (Magicid_t*)buf;
	Magicid_data_t	magic;
	int		swap;

	magic = MAGICID;
	if (n >= sizeof(Bgproute_t) &&
	    (swap = swapop(&magic, &mp->magic, sizeof(magic))) >= 0 &&
	    streq(mp->name, MAGIC_NAME) &&
	    swapget(swap ^ int_swap, &mp->size, sizeof(mp->size)) == sizeof(Bgproute_t))
	{
		file->skip = sizeof(Bgproute_t);
		file->ident = swap;
		return 1;
	}
	return 0;
}

/*
 * fixed file openf
 */

static int
fixedopen(Dssfile_t* file, Dssdisc_t* disc)
{
	if (file->flags & DSS_FILE_READ)
	{
		if (!sfreserve(file->io, file->skip, 0))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "header read error");
			return -1;
		}
		if (!(file->data = (void*)vmnewof(file->dss->vm, 0, Fixedstate_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		((Fixedstate_t*)file->data)->swap = file->ident;
	}
	else if (!(file->flags & DSS_FILE_APPEND))
	{
		Fixedheader_t	hdr;

		memset(&hdr, 0, sizeof(hdr));
		hdr.magic.magic = MAGICID;
		strcpy(hdr.magic.name, MAGIC_NAME);
		strcpy(hdr.magic.type, MAGIC_TYPE);
		hdr.magic.version = MAGIC_VERSION;
		hdr.magic.size = sizeof(Bgproute_t);
		sfwrite(file->io, &hdr, sizeof(hdr));
	}
	return 0;
}

/*
 * fixed readf
 */

static int
fixedread(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Fixedstate_t*	state = (Fixedstate_t*)file->data;
	register Bgproute_t*	rp;

	if (!(rp = (Bgproute_t*)sfreserve(file->io, sizeof(*rp), 0)))
	{
		if (sfvalue(file->io))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: last record incomplete", file->format->name);
			return -1;
		}
		return 0;
	}
	if (state->swap)
	{
		swapmem(state->swap, rp, rp, (char*)&rp->path - (char*)rp);
		if (rp->cluster.size)
			swapmem(state->swap, rp->data + rp->cluster.offset, rp->data + rp->cluster.offset, rp->cluster.size * sizeof(Bgpnum_t));
		if (state->swap & 1)
		{
			swapmem(state->swap & 1, (char*)&rp->path, (char*)&rp->path, (char*)&rp->bits - (char*)&rp->path);
			if (rp->path.size)
				swapmem(state->swap & 1, rp->data + rp->path.offset, rp->data + rp->path.offset, rp->path.size * sizeof(Bgpasn_t));
			if (rp->community.size)
				swapmem(state->swap & 1, rp->data + rp->community.offset, rp->data + rp->community.offset, rp->community.size * sizeof(Bgpasn_t));
		}
	}
	record->data = rp;
	record->size = rp->size;
	return 1;
}

/*
 * fixed writef
 */

static int
fixedwrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	if (sfwrite(file->io, record->data, sizeof(Bgproute_t)) != sizeof(Bgproute_t))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: write error", file->format->name);
		return -1;
	}
	return 0;
}

/*
 * fixed closef
 */

static int
fixedclose(Dssfile_t* file, Dssdisc_t* disc)
{
	if (!file->data)
		return -1;
	vmfree(file->dss->vm, file->data);
	return 0;
}

Dssformat_t bgp_fixed_format =
{
	"fixed",
	"bgp fixed format (2007-09-27) compresses well with pzip(1)",
	CXH,
	fixedident,
	fixedopen,
	fixedread,
	fixedwrite,
	0,
	fixedclose,
	0,
	0,
	bgp_fixed_next
};
