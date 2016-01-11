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
 * opaque method
 *
 * Glenn Fowler
 * AT&T Research
 */

static const char usage[] =
"[+DESCRIPTION?The \bdss\b opaque method handles anonymous fixed record "
    "data. Field names and expressions are not supported.]"
"[n:name?The magic header generic data/application name.]:[name]"
"[s:size?The fixed record size; must be specified.]#[size]"
"[t:type?The magic header specific data type.]:[type]"
"[v:version?The magic header version stamp, either \aYYYYMMDD\a or"
" \a0xWWXXYYZZ\a.]#[version]"
;

#include <dsslib.h>
#include <magicid.h>

struct Opaque_s; typedef struct Opaque_s Opaque_t;

struct Opaque_s
{
	Dssmeth_t	meth;
	Magicid_t	magic;
};

extern Dsslib_t		dss_lib_opaque;

/*
 * identf
 */

static int
opaqueident(Dssfile_t* file, void* buf, size_t size, Dssdisc_t* disc)
{
	register Opaque_t*	opaque = (Opaque_t*)file->dss->meth->data;
	register Magicid_t*	magic = (Magicid_t*)buf;

	if (size < opaque->magic.size)
		return 0;
	if (opaque->magic.magic)
	{
		if (magic->magic != opaque->magic.magic)
			return 0;
		if (magic->size != opaque->magic.size)
			return 0;
		if (!streq(magic->name, opaque->magic.name))
			return 0;
		if (!streq(magic->type, opaque->magic.type))
			return 0;
		if (opaque->magic.version && magic->version > opaque->magic.version)
			return 0;
		file->skip = opaque->magic.size;
	}
	return 1;
}

/*
 * opaque fopenf
 */

static int
opaquefopen(Dssfile_t* file, Dssdisc_t* disc)
{
	file->data = &((Opaque_t*)file->dss->meth->data)->magic.size;
	if (file->skip && !sfreserve(file->io, file->skip, 0))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "header read error");
		return -1;
	}
	return 0;
}

/*
 * opaque fclosef
 */

static int
opaquefclose(Dssfile_t* file, Dssdisc_t* disc)
{
	if (!file)
		return -1;
	return 0;
}

/*
 * opaque readf
 */

static int
opaqueread(register Dssfile_t* file, register Dssrecord_t* record, Dssdisc_t* disc)
{
	record->data = sfreserve(file->io, *((size_t*)file->data), 0);
	record->size = sfvalue(file->io);
	if (record->data)
		return 1;
	if (record->size && disc->errorf)
		(*disc->errorf)(NiL, disc, 2, "%slast record truncated -- record ignored", cxlocation(file->dss->cx, record));
	return 0;
}

/*
 * opaque writef
 */

static int
opaquewrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	return sfwrite(file->io, record->data, record->size) == record->size ? 0 : -1;
}

static Dssformat_t opaque_format =
{
	"opaque",
	"opaque format (2010-05-28)",
	CXH,
	opaqueident,
	opaquefopen,
	opaqueread,
	opaquewrite,
	0,
	opaquefclose,
	0,
	0,
	0
};

/*
 * methf
 */

static Dssmeth_t*
opaquemeth(const char* name, const char* options, const char* schema, Dssdisc_t* disc, Dssmeth_t* meth)
{
	register Opaque_t*	opaque;
	char*			s;

	if (!(opaque = newof(0, Opaque_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "out of space");
		return 0;
	}
	opaque->meth = *meth;
	opaque->meth.data = opaque;
	if (options)
	{
		if (dssoptlib(meth->cx->buf, &dss_lib_opaque, usage, disc))
			goto drop;
		s = sfstruse(meth->cx->buf);
		for (;;)
		{
			switch (optstr(options, s))
			{
			case 'n':
				opaque->magic.magic = MAGICID;
				strncopy(opaque->magic.name, opt_info.arg, sizeof(opaque->magic.name));
				continue;
			case 's':
				opaque->magic.size = opt_info.num;
				continue;
			case 't':
				opaque->magic.magic = MAGICID;
				strncopy(opaque->magic.type, opt_info.arg, sizeof(opaque->magic.type));
				continue;
			case 'v':
				opaque->magic.magic = MAGICID;
				opaque->magic.version = opt_info.num;
				continue;
			case '?':
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
				goto drop;
			case ':':
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
				goto drop;
			}
			break;
		}
	}
	if (!opaque->magic.size)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "data size must be specified");
		goto drop;
	}
	dtinsert(opaque->meth.formats, &opaque_format);
	return &opaque->meth;
 drop:
	free(opaque);
	return 0;
}

static Dssmeth_t method =
{
	"opaque",
	"opaque fixed record data with optional magic",
	CXH,
	opaquemeth
};

Dsslib_t	dss_lib_opaque =
{
	"opaque",
	"opaque method"
	"[-1ls5Pp0?\n@(#)$Id: dss opaque method (AT&T Research) 2002-11-22 $\n]"
	USAGE_LICENSE,
	CXH,
	0,
	&method
};
