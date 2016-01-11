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
 * netflow dump type
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "flowlib.h"

#include <magicid.h>
#include <swap.h>

#define MAGIC_NAME		"netflow"
#define MAGIC_TYPE		"router"
#define MAGIC_VERSION		20080611L

typedef union Fixedheader_u
{
	Magicid_t	magic;
	Netflow_t	netflow;
} Fixedheader_t;

typedef struct State_s
{
	int		swap;
} State_t;

/*
 * identf
 */

static int
fixedident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	Magicid_t*	mp = (Magicid_t*)buf;
	Magicid_data_t	magic;
	int		swap;

	magic = MAGICID;
	if (n >= sizeof(Netflow_t) &&
	    (swap = swapop(&magic, &mp->magic, sizeof(magic))) >= 0 &&
	    streq(mp->name, MAGIC_NAME) &&
	    swapget(swap ^ int_swap, &mp->size, sizeof(mp->size)) == sizeof(Netflow_t))
	{
		file->skip = sizeof(Netflow_t);
		file->ident = swap;
		return 1;
	}
	return 0;
}

/*
 * openf
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
		if (!(file->data = (void*)vmnewof(file->dss->vm, 0, State_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		((State_t*)file->data)->swap = file->ident;
	}
	else if (!(file->flags & DSS_FILE_APPEND))
	{
		Fixedheader_t	hdr;

		memset(&hdr, 0, sizeof(hdr));
		hdr.magic.magic = MAGICID;
		strcpy(hdr.magic.name, MAGIC_NAME);
		strcpy(hdr.magic.type, MAGIC_TYPE);
		hdr.magic.version = MAGIC_VERSION;
		hdr.magic.size = sizeof(Netflow_t);
		sfwrite(file->io, &hdr, sizeof(hdr));
	}
	return 0;
}

/*
 * readf
 */

static int
fixedread(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register State_t*	state = (State_t*)file->data;
	register Netflow_t*	rp;

	if (!(rp = (Netflow_t*)sfreserve(file->io, sizeof(*rp), 0)))
	{
		if (sfvalue(file->io))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%slast record incomplete", cxlocation(file->dss->cx, record));
			return -1;
		}
		return 0;
	}
	if (state->swap)
	{
	}
	record->data = rp;
	record->size = sizeof(*rp);
	return 1;
}

/*
 * writef
 */

static int
fixedwrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	if (sfwrite(file->io, record->data, sizeof(Netflow_t)) != sizeof(Netflow_t))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%swrite error", cxlocation(file->dss->cx, record));
		return -1;
	}
	return 0;
}

/*
 * closef
 */

static int
fixedclose(Dssfile_t* file, Dssdisc_t* disc)
{
	if (file->data)
		vmfree(file->dss->vm, file->data);
	return 0;
}

Dssformat_t netflow_fixed_format =
{
	"fixed",
	"Cisco netflow fixed format (2008-06-20) compresses well with pzip(1)",
	CXH,
	fixedident,
	fixedopen,
	fixedread,
	fixedwrite,
	0,
	fixedclose,
	0,
	0,
	netflow_fixed_next
};
