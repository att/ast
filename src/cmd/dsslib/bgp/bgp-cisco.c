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
 * bgp cisco method
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "bgplib.h"

typedef struct Ciscostate_s
{
	Bgproute_t		route;
	Bgpnum_t		paddr;
	Bgpnum_t		pbits;
} Ciscostate_t;

/*
 * cisco identf
 */

static int
ciscoident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	register char*	s;
	register char*	e;
	register char*	f;
	register int	c;
	char*		t;
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
			if (c == '<' && s < (e - 4) && s[0] == 'b' && s[1] == 'r' && s[2] == '>' && s[3] == ' ')
				s += 4;
			else if (!isspace(c))
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
		if (((c = *f++) == '*' || c == '-' || c == '+' || c == 'r' || c == ' ') &&
		    ((c = *f++) == '>' || c == 'h' || c == ' ') &&
		    ((c = *f++) == 'a' || c == 'd' || c == 'i' || c == 's' || c == ' ') &&
		    isdigit(*f) && !strtoip4(f, &t, &addr, &bits) && *t == ' ')
			break;
	}
	return 1;
}

/*
 * cisco openf
 */

static int
ciscoopen(Dssfile_t* file, Dssdisc_t* disc)
{
	if (!(file->data = (void*)vmnewof(file->dss->vm, 0, Ciscostate_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	if ((file->flags & (DSS_FILE_WRITE|DSS_FILE_APPEND)) == DSS_FILE_WRITE)
	{
		sfprintf(file->io, "Status codes: s suppressed, d damped, h history, * valid, > best, i internal, r rib-failure, S stale\n");
		sfprintf(file->io, "Origin codes: i IGP, e EGP, ? incomplete\n");
		sfprintf(file->io, "   Network          Next Hop            Metric LocPrf Weight Path\n");
	}
	return 0;
}

/*
 * cisco readf
 */

static int
ciscoread(register Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Ciscostate_t*	state = (Ciscostate_t*)file->data;
	register Bgproute_t*	rp;
	register char*		s;
	register int		i;
	int			n;
	int			o;
	char*			e;
	char*			p;
	Bgpasn_t*		ap;
	Bgp_t*			bgp;
	Cxoperand_t		r;

	rp = &state->route;
	for (;;)
	{
		if (!(s = sfgetr(file->io, '\n', 0)))
			return 0;
		e = s + sfvalue(file->io);
		if (s[0] == ' ' && s[1] == '<' && s[2] == 'b' && s[3] == 'r' && s[4] == '>' && s[5] == ' ')
			s += 6;
		n = e - s;
		if (n > 20 && (s[0] == ' ' || s[0] == '*' || s[0] == '-' || s[0] == '+' || s[0] == 'r') && (s[3] >= '0' && s[3] <= '9' || n > 61 && s[3] == ' ' && s[20] >= '0' && s[20] <= '9'))
		{
			rp->attr = 0;
			rp->type = BGP_TYPE_table_dump;
			rp->origin = BGP_ORIGIN_incomplete;
			for (i = 0; i < 3; i++)
				switch (s[i])
				{
				case '>':
					rp->attr |= BGP_best;
					break;
				case 'a':
					rp->attr |= BGP_atomic;
					break;
				case 'd':
					rp->attr |= BGP_damped;
					break;
				case 'h':
					rp->attr |= BGP_history;
					break;
				case 'i':
					rp->attr |= BGP_internal;
					break;
				case 'r':
					rp->attr |= BGP_rib_failure;
					break;
				case 's':
					rp->attr |= BGP_suppressed;
					break;
				case 'S':
					rp->attr |= BGP_stale;
					break;
				case '*':
					rp->attr |= BGP_valid;
					break;
				case '+':
					rp->type = BGP_TYPE_announce;
					break;
				case '-':
					rp->type = BGP_TYPE_withdraw;
					break;
				}
			s += 3;
			if (*s == ' ')
				s += 17;
			else if (strtoip4(s, &p, &rp->addr.v4, &rp->bits))
				break;
			else
				for (s = p; *s == ' '; s++);
			if (*s == '\r' || *s == '\n')
			{
				if (!(s = sfgetr(file->io, '\n', 0)))
					return 0;
				if ((n = sfvalue(file->io)) <= 61)
					continue;
				e = s + n;
				while (*s == ' ')
					s++;
			}
			p = s + 41;
			if (strtoip4(s, NiL, &rp->hop.v4, NiL))
				break;
			rp->med = isdigit(s[25]) ? strtol(s+20, NiL, 10) : 0;
			rp->local = isdigit(s[32]) ? strtol(s+27, NiL, 10) : 0;
			rp->weight = isdigit(s[39]) ? strtol(s+34, NiL, 10) : 0;
			bgp = (Bgp_t*)file->dss->data;
			if ((i = (*bgp->type_as16path->internalf)(file->dss->cx, bgp->type_as16path, NiL, &bgp->type_as16path->format, &r, p, e - p, file->dss->cx->rm, disc)) < 0)
				break;
			p += i;
			if (r.value.buffer.size > elementsof(rp->data))
			{
				ap = (Bgpasn_t*)r.value.buffer.data;
				n = r.value.buffer.size / sizeof(Bgpasn_t);
				for (o = 0, i = 1; i < n; i++)
					if (ap[i] != ap[o])
						ap[o++] = ap[i];
				r.value.buffer.size = o * sizeof(Bgpasn_t);
				if (r.value.buffer.size > elementsof(rp->data))
				{
					o = elementsof(rp->data);
					if (disc->errorf && !(file->dss->flags & DSS_QUIET))
						(*disc->errorf)(NiL, disc, 1, "%s: AS path too long -- truncated to %d", file->format->name, o / sizeof(Bgpasn_t));
				}
				else if (disc->errorf && !(file->dss->flags & DSS_QUIET))
					(*disc->errorf)(NiL, disc, 1, "%s: AS path too long -- %d duplicates removed", file->format->name, n - o);
			}
			o = r.value.buffer.size;
			memcpy(rp->data, r.value.buffer.data, o);
			rp->path.offset = 0;
			rp->path.size = o / sizeof(Bgpasn_t);
			rp->cluster.offset = rp->cluster.size = rp->community.offset = rp->community.size = 0;
			for (;;)
			{
				switch (*p++)
				{
				case 'C':
					if ((i = (*bgp->type_community->internalf)(file->dss->cx, bgp->type_community, NiL, &bgp->type_community->format, &r, p, e - p, file->dss->cx->rm, disc)) < 0)
						break;
					p += i;
					if ((n = r.value.buffer.size) > (elementsof(rp->data) - o))
					{
						n = (elementsof(rp->data) - o) & (sizeof(Bgpasn_t) - 1);
						if (disc->errorf && !(file->dss->flags & DSS_QUIET))
							(*disc->errorf)(NiL, disc, 1, "%s: cluster list too long -- truncated to %d", file->format->name, n);
					}
					rp->community.offset = o;
					rp->community.size = n / sizeof(Bgpasn_t);
					memcpy(rp->data + o, r.value.buffer.data, n);
					o += n;
					continue;
				case 'I':
					if ((i = (*bgp->type_cluster->internalf)(file->dss->cx, bgp->type_cluster, NiL, &bgp->type_cluster->format, &r, p, e - p, file->dss->cx->rm, disc)) < 0)
						break;
					p += i;
					o = roundof(o, sizeof(Bgpnum_t));
					if ((n = r.value.buffer.size) > (elementsof(rp->data) - o))
					{
						n = (elementsof(rp->data) - o) & (sizeof(Bgpnum_t) - 1);
						if (disc->errorf && !(file->dss->flags & DSS_QUIET))
							(*disc->errorf)(NiL, disc, 1, "%s: cluster list too long -- truncated to %d", file->format->name, n);
					}
					rp->cluster.offset = o;
					rp->cluster.size = n / sizeof(Bgpnum_t);
					memcpy(rp->data + o, r.value.buffer.data, n);
					o += n;
					continue;
				case 'T':
					rp->time = tmdate(p, &e, NiL);
					if (p == e && disc->errorf && !(file->dss->flags & DSS_QUIET))
						(*disc->errorf)(NiL, disc, 1, "%s: %s: invalid date/time", file->format->name, p);
					p = e;
					continue;
				case 'e':
					rp->origin = BGP_ORIGIN_egp;
					break;
				case 'i':
					rp->origin = BGP_ORIGIN_igp;
					break;
				case '?':
					rp->origin = BGP_ORIGIN_incomplete;
					break;
				case ' ':
				case '\t':
					continue;
				}
				break;
			}
			memset(rp->data + o, 0, elementsof(rp->data) - o);
			record->data = rp;
			record->size = rp->size = (char*)(rp->data + o) - (char*)rp;
			return 1;
		}
	}
	return 0;
}

/*
 * cisco writef
 */

static int
ciscowrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Ciscostate_t*	state = (Ciscostate_t*)file->data;
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
	if (rp->addr.v4 == state->paddr && rp->bits == state->pbits)
		sfprintf(io, "                 ");
	else
	{
		state->paddr = rp->addr.v4;
		state->pbits = rp->bits;
		if (sfprintf(io, "%-16s", fmtip4(rp->addr.v4, rp->bits)) > 16)
			sfprintf(io, "\n                    ");
		else
			sfputc(io, ' ');
	}
	sfprintf(io, "%-19s ", fmtip4(rp->hop.v4, -1));
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
	return 0;
}

/*
 * cisco closef
 */

static int
ciscoclose(Dssfile_t* file, Dssdisc_t* disc)
{
	if (!file->data)
		return -1;
	vmfree(file->dss->vm, file->data);
	return 0;
}

Dssformat_t bgp_cisco_format =
{
	"cisco",
	"cisco router dump format (2009-03-15)",
	CXH,
	ciscoident,
	ciscoopen,
	ciscoread,
	ciscowrite,
	0,
	ciscoclose,
	0,
	0,
	bgp_cisco_next
};
