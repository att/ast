/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2012 AT&T Intellectual Property          *
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
 * bgp table method
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "bgplib.h"

typedef struct Tablestate_s
{
	Bgproute_t		route[2];
	int			index;
	int			v6;
} Tablestate_t;

/*
 * table identf
 */

static int
tableident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	register char*	s;
	register char*	e;
	register char*	f;
	register int	c;
	unsigned char	prefix[IP6PREFIX];
	Bgpnum_t	addr;
	unsigned char	bits;

	s = (char*)buf;
	e = s + n;
	for (;;)
	{
		for (;;)
		{
			if (s >= e)
				return 0;
			c = *s++;
			if (!isascii(c))
				return 0;
			if (!isspace(c))
				break;
		}
		f = s - 1;
		for (;;)
		{
			if (s >= e)
				return 0;
			c = *s++;
			if (!isascii(c))
				return 0;
			if (c == '\n')
				break;
		}
		if (isdigit(*f) && !strtoip4(f, NiL, &addr, &bits) || !strtoip6(f, NiL, prefix, prefix + IP6BITS) && (file->caller = file))
			break;
	}
	return 1;
}

/*
 * table openf
 */

static int
tableopen(Dssfile_t* file, Dssdisc_t* disc)
{
	Tablestate_t*	state;

	if (!(state = vmnewof(file->dss->vm, 0, Tablestate_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	state->route[0].bits = -1;
	state->v6 = file->caller == file;
	file->data = (void*)state;
	return 0;
}

/*
 * table readf
 */

static int
tableread(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Tablestate_t*	state = (Tablestate_t*)file->data;
	register Bgproute_t*	rp;
	register char*		s;
	char*			p;
	char*			e;
	Bgproute_t*		op;

	for (;;)
	{
		op = &state->route[state->index];
		rp = &state->route[state->index = !state->index];
		if (!(s = sfgetr(file->io, '\n', 1)))
			return 0;
		while (*s == ' ' || *s == '\t' || *s == '\r')
			s++;
		if (*s == 0 || *s == '#' || *s == '\n')
			continue;
		rp->set = 0;
		rp->type = BGP_TYPE_withdraw;
		rp->attr = BGP_valid;
		rp->origin = BGP_ORIGIN_incomplete;
		if (state->v6)
		{
			rp->addr.v4 = 0;
			rp->bits = 0;
			if (strtoip6(s, &p, rp->prefixv6, rp->prefixv6 + IP6BITS))
				break;
			rp->set |= BGP_SET_prefixv6;
			for (s = p; *s == ' ' || *s == '\t'; s++);
			rp->hop.v4 = strtoul(s, &p, 0);
			switch (*p)
			{
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			case ':':
				rp->hop.v4 = 0;
				if (!strtoip6(s, &e, rp->hop.v6, NiL) && e > p)
				{
					rp->type = BGP_TYPE_announce;
					rp->attr |= BGP_slot;
					rp->set |= BGP_SET_hopv6;
				}
				break;
			case '.':
				if (strtoip4(s, &e, &rp->hop.v4, NiL) || e <= p)
					break;
				/*FALLTHROUGH*/
			default:
				if (rp->hop.v4)
				{
					rp->type = BGP_TYPE_announce;
					rp->attr |= BGP_slot;
				}
				break;
			}
			if ((rp->set & BGP_SET_prefixv6) && memcmp(rp->prefixv6, op->prefixv6, sizeof(rp->prefixv6)))
				rp->attr |= BGP_best;
			else if ((rp->set & BGP_SET_hopv6) && !memcmp(rp->hop.v6, op->hop.v6, sizeof(rp->hop.v6)))
				continue;
			else if (rp->hop.v4 && op->hop.v4 && rp->hop.v4 == op->hop.v4)
				continue;
		}
		else
		{
			if (strtoip4(s, &p, &rp->addr.v4, &rp->bits))
				break;
			for (s = p; *s == ' ' || *s == '\t'; s++);
			rp->hop.v4 = strtoul(s, &p, 0);
			if (p > s && (*p != '.' || !strtoip4(s, NiL, &rp->hop.v4, NiL)) && rp->hop.v4)
			{
				rp->type = BGP_TYPE_announce;
				rp->attr |= BGP_slot;
			}
			if (rp->addr.v4 != op->addr.v4 || rp->bits != op->bits)
				rp->attr |= BGP_best;
			else if (rp->hop.v4 == op->hop.v4)
				continue;
		}
		record->data = rp;
		record->size = BGP_FIXED;
		return 1;
	}
	return 0;
}

/*
 * table writef
 */

static int
tablewrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Bgproute_t* rp	= (Bgproute_t*)record->data;

	switch (rp->attr & (BGP_best|BGP_damped|BGP_internal|BGP_suppressed|BGP_valid))
	{
	case BGP_best|BGP_valid:
	case BGP_internal|BGP_valid:
		break;
	default:
		return 0;
	}
	sfprintf(file->io, "%-16s ", fmtip4(rp->addr.v4, rp->bits));
	if (rp->hop.v4 < 256)
		sfprintf(file->io, "%-19u", rp->hop.v4);
	else
		sfprintf(file->io, "%-19s", fmtip4(rp->hop.v4, -1));
	if (sfputc(file->io, '\n') == EOF)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "write error");
		return -1;
	}
	return 0;
}

/*
 * table closef
 */

static int
tableclose(Dssfile_t* file, Dssdisc_t* disc)
{
	if (!file->data)
		return -1;
	vmfree(file->dss->vm, file->data);
	return 0;
}

Dssformat_t bgp_table_format =
{
	"table",
	"simple table format (2008-06-20) with prefix mask and optional next hop",
	CXH,
	tableident,
	tableopen,
	tableread,
	tablewrite,
	0,
	tableclose,
	0,
	0,
	bgp_table_next
};
