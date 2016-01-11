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

#define FLOW_TEMPLATE	0
#define FLOW_OPTION	1
#define FLOW_META	255

#define BE2(p)		((p)+=2,(*(p-2)<<8)|(*(p-1)))
#define BE4(p)		((p)+=4,(*(p-4)<<24)|(*(p-3)<<16)|(*(p-2)<<8)|(*(p-1)))

/*
 * identf
 */

static int
dumpv9ident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	register unsigned char*		b = buf;

	return n >= 20 && BE2(b) == 9 && (n = BE2(b)) >= 1 && n <= 255;
}

/*
 * fopenf
 */

static int
dumpv9fopen(Dssfile_t* file, Dssdisc_t* disc)
{
	Netflow_file_t*	pp;

	if (!(pp = vmnewof(file->vm, 0, Netflow_file_t, 1, (file->flags & DSS_FILE_WRITE) ? NETFLOW_PACKET : 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	file->data = pp;
	pp->data = pp->last = 0;
	pp->data++;
	return 0;
}

/*
 * freadf
 */

static int
dumpv9fread(register Dssfile_t* file, register Dssrecord_t* record, Dssdisc_t* disc)
{
	register Netflow_file_t*	pp = (Netflow_file_t*)file->data;
	register Netflow_method_t*	mp = (Netflow_method_t*)file->dss->data;
	register Netflow_t*		rp = &pp->record;
	register Netflow_template_t*	tp;
	register Netflow_template_t*	bp;
	register Netflow_field_t*	fp;
	int				n;
	int				m;
	int				k;
	int				z;
	int				o;

	if ((pp->data += pp->next) <= (pp->last - pp->next))
	{
		if (file->dss->flags & DSS_DEBUG)
			sfprintf(sfstderr, "count %d\n", pp->count);
		pp->count--;
	}
	else
	{
		pp->data = pp->last;
		for (;;)
		{
			if (file->dss->flags & DSS_DEBUG)
				sfprintf(sfstderr, "count %d\n", pp->count);
			while (!pp->count--)
			{
				file->offset = sftell(file->io);
				if (!(pp->data = (unsigned char*)sfreserve(file->io, NETFLOW_PACKET, 0)))
				{
					if (sfvalue(file->io))
						goto incomplete;
					return 0;
				}
				if ((rp->version = BE2(pp->data)) != 9)
					goto corrupt;
				pp->count = rp->count = BE2(pp->data);
				if (file->dss->flags & DSS_DEBUG)
					sfprintf(sfstderr, "header version %d size %d flowsets %d offset %I*d\n", rp->version, 20, pp->count, sizeof(file->offset), file->offset);
				rp->uptime = BE4(pp->data);
				rp->time = BE4(pp->data);
				rp->flow_sequence = BE4(pp->data);
				rp->source_id = BE4(pp->data);
				pp->boot = ((Nftime_t)rp->time * MS - (Nftime_t)rp->uptime) * US + (Nftime_t)rp->nsec;
			}
			if (file->dss->flags & DSS_DEBUG)
				sfprintf(sfstderr, "record %d flowset %d\n", file->count, rp->count - pp->count);
			n = BE2(pp->data);
			if ((z = BE2(pp->data) - 4) <= 0)
			{
				pp->count = 0;
				continue;
			}
			pp->last = pp->data + z;
			if (file->dss->flags & DSS_DEBUG)
				sfprintf(sfstderr, "id %d size %d\n", n, z);
			if (n > FLOW_META)
			{
				for (tp = mp->templates; tp && tp->id != n; tp = tp->next);
				if (!tp || !tp->size)
				{
					if (!(file->dss->flags & DSS_QUIET) && disc->errorf)
						(*disc->errorf)(NiL, disc, 1, "%s%d: undefined template -- flowset ignored", cxlocation(file->dss->cx, record), n);
					file->count++;
					pp->count = 0;
					continue;
				}

				/* HERE: add an options indicator to the record schema instead of discarding? */

				if (tp->options)
				{
					pp->count = 0;
					continue;
				}
				pp->template = tp;
				pp->next = tp->size;
				break;
			}
			if (n == FLOW_TEMPLATE)
			{
				bp = ((Netflow_method_t*)file->dss->data)->base;
				pp->count++;
				while (pp->data < (pp->last - 4))
				{
					n = BE2(pp->data);
					m = BE2(pp->data);
					if (file->dss->flags & DSS_DEBUG)
						sfprintf(sfstderr, "template %d elements %d\n", n, m);
					if ((pp->last - pp->data) < m * 4)
						break;
					for (tp = mp->templates; tp && tp->id != n; tp = tp->next);
					if (tp)
						memset(tp->field, 0, sizeof(tp->field));
					else if (!(tp = vmnewof(file->dss->vm, 0, Netflow_template_t, 1, 0)))
					{
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, 2, "out of space");
						return -1;
					}
					else
					{
						tp->id = n;
						tp->next = mp->templates;
						mp->templates = tp;
					}
					tp->elements = m;
					pp->count--;
					file->count++;
					o = 0;
					while (m--)
					{
						n = BE2(pp->data);
						z = BE2(pp->data);
						if (n >= 1 && n <= NETFLOW_TEMPLATE)
						{
							fp = &tp->field[n - 1];
							fp->type = bp->field[n - 1].type;
							fp->offset = o;
							o += (fp->size = z);
							if (file->dss->flags & DSS_DEBUG)
								sfprintf(sfstderr, "template %2d field %2d type %2d size %2d offset %3d\n", tp->id, n, fp->type, fp->size, fp->offset);
						}
					}
					tp->size = o;
					if (file->dss->flags & DSS_DEBUG)
						sfprintf(sfstderr, "template %d elements %d size %d\n", tp->id, tp->elements, tp->size);
				}
			}
			else if (n == FLOW_OPTION)
			{
				pp->count++;
				bp = ((Netflow_method_t*)file->dss->data)->base;
				while (pp->data < (pp->last - 6))
				{
					n = BE2(pp->data);
					m = BE2(pp->data) / 4;
					k = BE2(pp->data) / 4;
					if (file->dss->flags & DSS_DEBUG)
						sfprintf(sfstderr, "template %d scopes %d options %d\n", n, m, k);
					for (tp = mp->templates; tp && tp->id != n; tp = tp->next);
					if (tp)
						memset(tp->field, 0, sizeof(tp->field));
					else if (!(tp = vmnewof(file->dss->vm, 0, Netflow_template_t, 1, 0)))
					{
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, 2, "out of space");
						return -1;
					}
					else
					{
						tp->id = n;
						tp->next = mp->templates;
						mp->templates = tp;
					}
					tp->elements = m + k;
					tp->options = k;
					pp->count--;
					file->count++;
					o = 0;
					while (m--)
					{
						n = BE2(pp->data);
						z = BE2(pp->data);
						if (file->dss->flags & DSS_DEBUG)
							sfprintf(sfstderr, "template %2d scope %2d size %2d offset %3d\n", tp->id, n, z, o);
						o += z;
					}
					while (k--)
					{
						n = BE2(pp->data);
						z = BE2(pp->data);
						if (n >= 1 && n <= NETFLOW_TEMPLATE)
						{
							fp = &tp->field[n - 1];
							fp->type = bp->field[n - 1].type;
							fp->offset = o;
							fp->size = z;
							if (file->dss->flags & DSS_DEBUG)
								sfprintf(sfstderr, "template %2d field %2d type %2d size %2d offset %3d\n", tp->id, n, fp->type, fp->size, fp->offset);
						}
						o += z;
					}
					tp->size = o;
					if ((file->dss->flags & DSS_DEBUG) && disc->errorf)
						sfprintf(sfstderr, "template %d elements %d size %d\n", tp->id, tp->elements, tp->size);
				}
			}
			else
			{
				if (!(file->dss->flags & DSS_QUIET) && disc->errorf)
					(*disc->errorf)(NiL, disc, 1, "%s%d: template index %d definition ignored", cxlocation(file->dss->cx, record), n);
				file->count++;
			}
			pp->data = pp->last;
		}
	}
	record->size = sizeof(*rp);
	record->data = rp;
	return 1;
 corrupt:
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%sheader corrupt", cxlocation(file->dss->cx, record));
	return -1;
 incomplete:
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, 2, "%slast packet incomplete", cxlocation(file->dss->cx, record));
	return -1;
}

/*
 * fwritef
 */

static int
dumpv9fwrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, 2, "%s: record write not implemented", file->format->name);
	return -1;
}

/*
 * fclosef
 */

static int
dumpv9fclose(Dssfile_t* file, Dssdisc_t* disc)
{
	return 0;
}

Dssformat_t netflow_dumpv9_format =
{
	"dumpv9",
	"Cisco netflow v9 dump format (2008-12-10) flow templates are retained across input files",
	CXH,
	dumpv9ident,
	dumpv9fopen,
	dumpv9fread,
	dumpv9fwrite,
	0,
	dumpv9fclose,
	0,
	0,
	netflow_dumpv9_next
};
