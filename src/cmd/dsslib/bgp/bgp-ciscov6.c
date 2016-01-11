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
 * bgp cisco ipv6 method
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "bgplib.h"

typedef struct Ciscov6state_s
{
	Bgproute_t		route;
	char*			line;
	unsigned char		prefix[IP6PREFIX];
} Ciscov6state_t;

/*
 * ciscov6 identf
 */

static int
ciscov6ident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	register char*		s;
	register char*		e;
	register char*		f;
	register int		c;
	char*			v;
	int			m;

	static const char*	magic[] = { "IPv6 Routing Table - ", "Codes: " };

	m = 0;
	v = 0;
	s = (char*)buf;
	e = s + n;
	for (;;)
	{
		for (;;)
		{
			if (s >= e)
				return 0;
			c = *s++;
			if (!isspace(c))
				break;
		}
		f = s - 1;
		for (;;)
		{
			if (s >= e)
				return 0;
			if (*s++ == '\n')
				break;
		}
		if (!isascii(*f) && (s - f) > 256)
			return 0;
		v++;
		if (strneq(f, magic[m], strlen(magic[m])) && ++m >= elementsof(magic))
		{
			file->caller = v;
			break;
		}
	}
	return 1;
}

/*
 * ciscov6 openf
 */

static int
ciscov6open(Dssfile_t* file, Dssdisc_t* disc)
{
	register char*			v = file->caller;
	register Ciscov6state_t*	state;

	if (!(state = vmnewof(file->dss->vm, 0, Ciscov6state_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	file->data = state;
	if ((file->flags & (DSS_FILE_WRITE|DSS_FILE_APPEND)) == DSS_FILE_WRITE)
	{
		sfprintf(file->io, "Codes: C - Connected, L - Local, S - Static, R - RIP, B - BGP\n\
       U - Per-user Static route\n\
       I1 - ISIS L1, I2 - ISIS L2, IA - ISIS interarea\n\
       O - OSPF intra, OI - OSPF inter, OE1 - OSPF ext 1, OE2 - OSPF ext 2\n");
	}
	while (v-- && sfgetr(file->io, '\n', 0));
	while ((state->line = sfgetr(file->io, '\n', 0)) && !isupper(*state->line));
	return 0;
}

/*
 * ciscov6 readf
 */

static int
ciscov6read(register Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Ciscov6state_t*	state = (Ciscov6state_t*)file->data;
	register Bgproute_t*		rp;
	register char*			s;
	int				o;
	int				p;
	char*				t;

	rp = &state->route;
	while (s = state->line)
	{
		t = s;
		while (*s && *s != '\n' && *s != ' ' && *s != '\t')
			s++;
		switch (*t)
		{
		case 'B':
			rp->set = 0;
			rp->attr = BGP_valid;
			rp->type = BGP_TYPE_table_dump;
			rp->origin = BGP_ORIGIN_incomplete;
			o = 0;
			p = 0;
			for (;;)
			{
				while (*s == ' ' || *s == '\t')
					s++;
				if (*s && *s != '\n')
				{
					t = s;
					while (*s && *s != '\n' && *s != ' ' && *s != '\t')
						s++;
					while (*s == ' ' || *s == '\t')
						s++;
					switch (p)
					{
					case 0:
						if (!strtoip6(t, NiL, rp->prefixv6, rp->prefixv6 + IP6BITS))
							p = 1;
						break;
					case 1:
						if ((s - t) > 3 && t[0] == 'v' && t[1] == 'i' && t[2] == 'a')
							p = 2;
						break;
					case 2:
						if (!strtoip6(t, NiL, rp->hop.v6, NiL))
							p = 3;
						break;
					}
				}
				if ((!*s || *s == '\n') && (!(s = state->line = sfgetr(file->io, '\n', 0)) || *s != ' ' && *s != '\t' && *s != '\r' && *s != '\n'))
					break;
			}
			if (p != 3)
				return 0;
			rp->set |= BGP_SET_prefixv6|BGP_SET_hopv6;
			break;
		default:
			while ((state->line = sfgetr(file->io, '\n', 0)) && !isupper(*state->line));
			continue;
		}
		memset(rp->data + o, 0, elementsof(rp->data) - o);
		record->data = rp;
		record->size = rp->size = (char*)(rp->data + o) - (char*)rp;
		return 1;
	}
	return 0;
}

/*
 * ciscov6 writef
 */

static int
ciscov6write(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
#if 0
	register Ciscov6state_t*	state = (Ciscov6state_t*)file->data;
	register Bgproute_t*	rp = (Bgproute_t*)record->data;
	register Sfio_t*	io = file->io;
	register int		i;
	register int		j;
	register int		k;
	Bgpasn_t*		ap;
	Bgpnum_t*		np;

	if (rp->type == BGP_TYPE_announce)
		sfputc(io, '+');
	else if (rp->type == BGP_TYPE_withdraw)
		sfputc(io, '-');
	else if (rp->attr & BGP_valid)
		sfputc(io, '*');
	else if (rp->attr & BGP_rib_failure)
		sfputc(io, 'r');
	else
		sfputc(io, ' ');
	if (rp->attr & BGP_best)
		sfputc(io, '>');
	else if (rp->attr & BGP_history)
		sfputc(io, 'h');
	else
		sfputc(io, ' ');
	if (rp->attr & BGP_atomic)
		sfputc(io, 'a');
	else if (rp->attr & BGP_damped)
		sfputc(io, 'd');
	else if (rp->attr & BGP_internal)
		sfputc(io, 'i');
	else if (rp->attr & BGP_suppressed)
		sfputc(io, 's');
	else if (rp->attr & BGP_stale)
		sfputc(io, 'S');
	else
		sfputc(io, ' ');
	if (rp->addr == state->paddr && rp->bits == state->pbits)
		sfprintf(io, "                 ");
	else
	{
		state->paddr = rp->addr;
		state->pbits = rp->bits;
		if (sfprintf(io, "%-16s", fmtip4(rp->addr, rp->bits)) > 16)
			sfprintf(io, "\n                    ");
		else
			sfputc(io, ' ');
	}
	sfprintf(io, "%-19s ", fmtip4(rp->hop, -1));
	if (rp->med)
		sfprintf(io, "%6u ", rp->med);
	else
		sfprintf(io, "       ");
	if (rp->local)
		sfprintf(io, "%6u ", rp->local);
	else
		sfprintf(io, "       ");
	sfprintf(io, "%6u", rp->weight);
	if (j = rp->path.size)
	{
		ap = BGPPATH(rp);
		for (i = 0; i < j; i++)
			if (ap[i] == BGP_SET16)
			{
				if (k = ap[++i])
				{
					k += i - 1;
					sfprintf(io, " {%u", ap[++i]);
					while (i < k)
						sfprintf(io, ",%u", ap[++i]);
					sfputc(io, '}');
				}
				else
					sfprintf(io, " %u", ap[++i]);
			}
			else
				sfprintf(io, " %u", ap[i]);
	}
	if (rp->community.size)
	{
		sfputc(io, ' ');
		sfputc(io, 'C');
		ap = BGPCOMMUNITY(rp);
		for (i = 0, j = rp->community.size; i < j; i += 2)
			sfprintf(io, " %u:%u", ap[i], ap[i+1]);
	}
	if (rp->cluster.size)
	{
		sfputc(io, ' ');
		sfputc(io, 'I');
		np = BGPCLUSTER(rp);
		for (i = 0, j = rp->cluster.size; i < j; i++)
			sfprintf(io, " %lu", np[i]);
	}
	if (rp->time)
		sfprintf(io, " T %s", fmttime("%K", rp->time));
	if (rp->origin)
	{
		sfputc(io, ' ');
		sfputc(io, rp->origin);
	}
	if (sfputc(io, '\n') == EOF)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: write error", file->format->name);
		return -1;
	}
#endif
	return 0;
}

/*
 * ciscov6 closef
 */

static int
ciscov6close(Dssfile_t* file, Dssdisc_t* disc)
{
	if (!file->data)
		return -1;
	vmfree(file->dss->vm, file->data);
	return 0;
}

Dssformat_t bgp_ciscov6_format =
{
	"ciscov6",
	"cisco ipv6 router dump format (2009-03-15)",
	CXH,
	ciscov6ident,
	ciscov6open,
	ciscov6read,
	ciscov6write,
	0,
	ciscov6close,
	0,
	0,
	bgp_ciscov6_next
};
