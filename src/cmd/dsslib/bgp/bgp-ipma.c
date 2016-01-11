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
 * bgp ipma method
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "bgplib.h"

typedef struct Ipmastate_s
{
	Bgproute_t		route;
	Bgpnum_t		paddr;
	Bgpnum_t		pbits;
} Ipmastate_t;

static const char	magic1[] = "Internet Performance Measurement and Analysis";
static const char	magic2[] = "AS Path";

/*
 * ipma identf
 */

static int
ipmaident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	register char*	s;
	register char*	e;
	register char*	e1;
	register char*	e2;

	s = (char*)buf;
	e = s + n;
	e1 = e - sizeof(magic1) + 1;
	e2 = e - sizeof(magic2) + 1;
	e = (e1 > e2) ? e1 : e2;
	while (s < e)
	{
		if (s < e1 && strneq(s, magic1, sizeof(magic1) - 1) || s < e2 && strneq(s, magic2, sizeof(magic2) - 1))
			return 1;
		s++;
	}
	return 0;
}

/*
 * ipma openf
 */

static int
ipmaopen(Dssfile_t* file, Dssdisc_t* disc)
{
	register char*	s;
	register char*	e;
	register char*	e1;
	register char*	e2;
	register int	c;
	register int	q;

	if (!(file->data = (void*)vmnewof(file->dss->vm, 0, Ipmastate_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	if (file->flags & DSS_FILE_READ)
	{
		while (s = sfgetr(file->io, '\n', 0))
		{
			e = s + sfvalue(file->io);
			e1 = e - sizeof(magic1) + 1;
			e2 = e - sizeof(magic2) + 1;
			e = (e1 > e2) ? e1 : e2;
			while (s < e)
			{
				if (s < e1 && strneq(s, magic1, sizeof(magic1) - 1) || s < e2 && strneq(s, magic2, sizeof(magic2) - 1))
					break;
				s++;
			}
			if (s < e)
				break;
		}
		q = 0;
		for (;;)
		{
			switch (c = sfgetc(file->io))
			{
			case EOF:
				break;
			case '\n':
				q = 0;
				continue;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (q == 0)
				{
					sfungetc(file->io, c);
					break;
				}
				continue;
			case '<':
				if (q == 0 || q == '&')
					q = c;
				continue;
			case '>':
				if (q == '<' || q == '&')
					q = 0;
				else if (q == 0)
					q = '#';
				continue;
			case '&':
				if (q == 0)
					q = c;
				continue;
			default:
				if (q == 0)
				{
					if (!isspace(c))
						q = '#';
				}
				else if (q == '&')
				{
					if (!isalnum(c) && c != '#')
						q = 0;
				}
				continue;
			}
			break;
		}
		((Ipmastate_t*)file->data)->route.attr = BGP_best;
		((Ipmastate_t*)file->data)->route.type = BGP_TYPE_table_dump;
	}
	else if (!(file->flags & DSS_FILE_APPEND))
		sfprintf(file->io, "%s\n\n", magic1);
	return 0;
}

/*
 * skip over html
 * number==1 skips to next number
 */

static char*
skip(register char* s, register char* e, int number)
{
	register char*	b;
	register char*	t;

	do
	{
		b = s;
		while (s < e && isspace(*s))
			s++;
		if (*s == '<')
		{
			while (s < e && *s++ != '>');
			while (s < e && isspace(*s))
				s++;
		}
		if (*s == '&')
		{
			if (*++s == '#' && s < e)
				s++;
			while (s < e && isalnum(*s))
				s++;
		}
		if (s < e && number)
		{
			if ((isdigit(*s) || (e - s) > 5 && strneq(s, "local", 5)) && (isspace(*(s - 1)) || *(s - 1) == '>' || *(s - 1) == ';' || *(s - 1) == '='))
			{
				if (!isdigit(*s))
					break;
				for (t = s; t < e && isdigit(*t); t++);
				if (t >= e || isspace(*t) || *t == '<' || *t == '&')
					break;
				s = t;
			}
			if (*s == '=' && (e - s) > 1 && isdigit(*(s + 1)))
			{
				s++;
				break;
			}
			while (s < e && !isspace(*s) && *s != '<' && *s != '&' && *s != '=')
				s++;
		}
	} while (s != b);
	return s;
}

/*
 * ipma readf
 */

static int
ipmaread(register Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Ipmastate_t*	state = (Ipmastate_t*)file->data;
	register Bgproute_t*	rp;
	register char*		s;
	register int		i;
	int			c;
	int			n;
	int			o;
	char*			e;
	char*			p;
	Bgpasn_t*		ap;

	rp = &state->route;
	for (;;)
	{
		if (!(s = sfgetr(file->io, '\n', 0)))
			return 0;
		e = s + sfvalue(file->io);
		s = skip(s, e, 0);
		if (s >= e)
			continue;
		if (isdigit(*s))
		{
			if (strtoip4(s, &p, &rp->addr.v4, &rp->bits))
				break;
			s = skip(p, e, 1);
			if (s >= e || !isalnum(*s))
			{
				if ((c = sfgetc(file->io)) == EOF)
					break;
				if (!isspace(c))
				{
					sfungetc(file->io, c);
					s = e = "";
				}
				else if (!(s = sfgetr(file->io, '\n', 0)))
					return 0;
				else
				{
					e = s + sfvalue(file->io);
					s = skip(s, e, 1);
				}
			}
		}
		else if ((e - s) > 6 && (strneq(s, "Prefix", 6) || strneq(s, "Networ", 6) || strneq(s, "Receiv", 6) || strneq(s, "Advert", 6)))
			continue;
		else
			s = skip(s, e, 1);
		if (s >= e)
			break;
		if (*(s - 1) == '=')
		{
			if (strtoip4(s, &p, &rp->hop.v4, NiL))
				break;
			s = skip(s, e, 1);
		}
		i = 0;
		n = elementsof(rp->data) / sizeof(Bgpasn_t);
		ap = (Bgpasn_t*)rp->data;
		rp->path.offset = i;
		o = 0;
		p = s;
		do
		{
			for (s = p; *s == ' ' || *s == '\t' || *s == ','; s++);
			if (*s == '{')
			{
				if (i >= (n - 2))
				{
					if (disc->errorf && !(file->dss->flags & DSS_QUIET))
						(*disc->errorf)(NiL, disc, 1, "%s: AS path too long -- truncated to %d", file->format->name, n);
					break;
				}
				ap[i++] = BGP_SET16;
				o = i++;
				p = s + 1;
			}
			else if (*s == '}')
				s++;
			else if (!isdigit(*s))
				break;
			else
			{
				if (i >= n)
				{
					if (disc->errorf && !(file->dss->flags & DSS_QUIET))
						(*disc->errorf)(NiL, disc, 1, "%s: AS path too long -- truncated to %d", file->format->name, n);
					break;
				}
				if ((ap[i++] = strtol(s, &p, 10)) == BGP_SET16)
				{
					if (i >= (n - 2))
					{
						if (disc->errorf && !(file->dss->flags & DSS_QUIET))
							(*disc->errorf)(NiL, disc, 1, "%s: AS path too long -- truncated to %d", file->format->name, n);
						break;
					}
					ap[i++] = 0;
					ap[i++] = BGP_SET16;
				}
			}
		} while (p > s);
		rp->path.size = i;
		if (o)
			ap[o] = i - o;
		if (s[0] == 'E' || s[0] == 'e')
			rp->origin = BGP_ORIGIN_egp;
		else if ((s[0] == 'I' || s[0] == 'i') && (s[1] == 'G' || s[1] == 'g'))
			rp->origin = BGP_ORIGIN_igp;
		else
			rp->origin = BGP_ORIGIN_incomplete;
		record->data = rp;
		record->size = rp->size = (char*)(rp->data + i) - (char*)rp;
		return 1;
	}
	return 0;
}

/*
 * ipma writef
 */

static int
ipmawrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Ipmastate_t*	state = (Ipmastate_t*)file->data;
	register Bgproute_t*	rp = (Bgproute_t*)record->data;
	register Sfio_t*	io = file->io;
	register int		i;
	register int		j;
	register int		k;
	Bgpasn_t*		ap;

	if (rp->addr.v4 != state->paddr || rp->bits != state->pbits)
	{
		state->paddr = rp->addr.v4;
		state->pbits = rp->bits;
		sfprintf(io, "%s\n", fmtip4(rp->addr.v4, rp->bits));
	}
	sfprintf(io, "\tN=%s", fmtip4(rp->hop.v4, -1));
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
	switch (rp->origin)
	{
	case BGP_ORIGIN_egp:
		sfprintf(io, " EGP");
		break;
	case BGP_ORIGIN_igp:
		sfprintf(io, " IGP");
		break;
	default:
		sfprintf(io, " Incomplete");
		break;
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
 * ipma closef
 */

static int
ipmaclose(Dssfile_t* file, Dssdisc_t* disc)
{
	if (!file->data)
		return -1;
	vmfree(file->dss->vm, file->data);
	return 0;
}

Dssformat_t bgp_ipma_format =
{
	"ipma",
	"ipma router dump format (2009-03-15) [http://www.merit.edu/~ipma/routing_table]",
	CXH,
	ipmaident,
	ipmaopen,
	ipmaread,
	ipmawrite,
	0,
	ipmaclose,
	0,
	0,
	bgp_ipma_next
};
