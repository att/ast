/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2013 AT&T Intellectual Property          *
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
 * partition support
 */

static const char usage[] =
"[-1i?\n@(#)$Id: pz library 2.4 (AT&T Research) 2011-03-07 $\n]"
"[a:append]"
"[c:comment]:[text]"
"[x:crc]"
"[d:debug]#[level]"
"[D:dump]"
"[G!:gzip]"
"[i:include]:[file]"
"[l:library]:[library]"
"[n:name]:[name]"
"[X:prefix?]:[count[*terminator]]]"
"[O:sort]"
"[P!:pzip]"
"[Q:regress]"
"[r:row]#[row-size]"
"[S:split]:?[pattern]"
"[s:summary]"
"[T:test]#[mask]"
"[v:verbose]"
;

#include "pzlib.h"

#define VECTOR(z,p,n)	(((z)->flags&PZ_OVERSIZE)?(p)->row:(n))

/*
 * return partition pointer given name
 */

Pzpart_t*
pzpartget(Pz_t* pz, const char* name)
{
	if (!name || !*name || !pz->partdict)
		return pz->mainpart;
	pz->flags &= ~PZ_MAINONLY;
	return (Pzpart_t*)dtmatch(pz->partdict, name);
}

/*
 * partition iterator
 * return the next partition after pp
 * first call should set pp to 0
 * 0 returned after all partitions visited
 */

Pzpart_t*
pzpartnext(Pz_t* pz, Pzpart_t* pp)
{
	if (pz->partdict)
		return pp ? (Pzpart_t*)dtnext(pz->partdict, pp) : (Pzpart_t*)dtfirst(pz->partdict);
	return pp ? (Pzpart_t*)0 : pz->mainpart;
}

/*
 * set the current partition to pp
 * old partition returned
 */

Pzpart_t*
pzpartset(Pz_t* pz, Pzpart_t* pp)
{
	Pzpart_t*	op;

	pz->flags &= ~PZ_MAINONLY;
	if (pzsync(pz))
		return 0;
	op = pz->part;
	pz->part = pp;
	return op;
}

/*
 * parse a column range at s
 * return
 *	-1	error
 *	 0	not a range
 *	 1	ok
 */

static int
range(Pz_t* pz, register Pzpart_t* pp, char* s, char** p, int* beg, int* end)
{
	int	n;
	int	m;
	char*	e;

	for (; isspace(*s) || *s == ','; s++);
	if (*s == '-')
		n = 0;
	else
	{
		n = (int)strtol(s, &e, 10);
		if (s == e)
		{
			if (p)
				*p = e;
			return 0;
		}
		if (pp && n >= pp->row)
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "column %d is larger than row %d", n, pp->row);
			return -1;
		}
		for (s = e; isspace(*s); s++);
	}
	if (*s == '-')
	{
		if (!*++s || isspace(*s) || *s == ',')
		{
			e = s;
			m = (pp ? pp->row : INT_MAX) - 1;
		}
		else
		{
			m = (int)strtol(s, &e, 10);
			if (m < n || pp && m >= pp->row)
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, 2, "invalid column range %d-%d", n, m);
				return -1;
			}
		}
	}
	else
		m = n;
	if (p)
	{
		for (s = e; isspace(*s) || *s == ','; s++);
		*p = s;
	}
	if (beg)
		*beg = n;
	if (end)
		*end = m;
	return 1;
}

/*
 * parse a character value at s
 */

static int
value(Pz_t* pz, char* s, char** p)
{
	char*	e;
	int	q;
	int	v;

	for (; isspace(*s); s++);
	if (!(q = *s++) || ((v = chresc(s, &e)), s == e))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 2, "value expected");
		return -1;
	}
	s = e;
	if (*s++ != q)
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 2, "unbalanced %c quote in value", q);
		return -1;
	}
	if (*p)
		*p = s;
	return v;
}

/*
 * add fixed column range value to pz
 */

static int
fixed(Pz_t* pz, register Pzpart_t* pp, int n, int m, int k)
{
	int	i;
	int	v;

	if (!pp->value)
	{
		if (!(pp->value = vmnewof(pz->vm, 0, int, pp->row, 0)))
			return pznospace(pz);
		for (i = 0; i < pp->row; i++)
			pp->value[i] = -1;
	}
	v = k < 0 ? ' ' : k;
	for (; n <= m; n++)
	{
		if (pp->value[n] < 0)
			pp->nfix++;
		else if (k < 0)
			continue;
		pp->value[n] = v;
	}
	return 0;
}

/*
 * initialize the partition map from pp.{<map,nmap>,<grp,ngrp>}
 */

int
pzpartmap(Pz_t* pz, register Pzpart_t* pp)
{
	int	i;
	int	j;
	int	k;

	k = 0;
	for (i = 0; i < pp->ngrp; i++)
		for (j = 0; j < pp->grp[i]; j++)
		{
			if (k >= pp->nmap)
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, 2, "%s: invalid group", pz->path);
				return -1;
			}
			pp->lab[k] = i;
			pp->inc[k++] = pp->grp[i];
		}
	if (k != pp->nmap)
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 2, "%s: invalid group", pz->path);
		return -1;
	}
	memset(pp->low, 1, pp->row);
	for (i = 0; i < pp->nmap; i++)
		pp->low[pp->map[i]] = 0;
	for (i = 0; i < pp->nfix; i++)
		pp->low[pp->fix[i]] = 0;
	if ((pz->flags & PZ_READ) && pp->value)
		for (i = j = 0; i < pp->nmap; i++)
			if (pp->value[i] < 0)
				pp->map[j++] = pp->map[i];
	pp->flags |= PZ_UPDATE;
	return 0;
}

/*
 * initialize the partition workspace
 */

int
pzpartinit(Pz_t* pz, Pzpart_t* pp, const char* name)
{
	char*	s;
	int	i;
	int	j;
	int	k;
	size_t	m;
	size_t	n;

	m = 0;
	if (!(pz->flags & PZ_FORCE) || (pz->flags & PZ_SPLIT))
	{
		if (!pp->row)
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: partition header corrupted", pz->path);
			return -1;
		}
		if (!pp->name)
		{
			if (!name)
				sfprintf(pz->tmp, "%s:%d:%d", PZ_PART_SUF, pp->row, pp->nmap);
			else
			{
				if (s = strrchr(name, '/'))
					name = (const char*)s + 1;
				if ((s = strrchr(name, '.')) && streq(s + 1, PZ_PART_SUF))
					n = s - (char*)name;
				else
					n = strlen(name);
				sfprintf(pz->tmp, "%.*s", n, name);
			}
			if (!(s = sfstruse(pz->tmp)) || !(pp->name = vmstrdup(pz->vm, s)))
				return -1;
		}
		if (!pp->nmap)
		{
			pp->nmap = pp->row;
			if (!(pp->map = vmnewof(pz->vm, pp->map, size_t, VECTOR(pz, pp, pp->nmap), 0)))
				return pznospace(pz);
			for (i = 0; i < pp->nmap; i++)
				pp->map[i] = i;
		}
		if (!pp->ngrp)
		{
			pp->ngrp = 1;
			if (!(pp->grp = vmnewof(pz->vm, pp->grp, size_t, VECTOR(pz, pp, pp->ngrp), 0)))
				return pznospace(pz);
			pp->grp[0] = pp->row;
		}
		pp->loq = ((pz->win / 8 / pp->row) + 8) * pp->row;
		k = VECTOR(pz, pp, pp->nmap);
		if (!(pp->low = vmnewof(pz->vm, 0, unsigned char, pp->row, 0)) ||
		    !(pp->mix = vmnewof(pz->vm, 0, unsigned char*, k, 0)) ||
		    !(pp->inc = vmnewof(pz->vm, 0, size_t, k, 0)) ||
		    !(pp->lab = vmnewof(pz->vm, 0, size_t, k, 0)))
			return pznospace(pz);
		if (pp->nfix)
		{
			if (!(pp->fix = vmnewof(pz->vm, 0, size_t, VECTOR(pz, pp, pp->nfix), 0)))
				return pznospace(pz);
			for (i = k = 0; i < pp->row; i++)
				if (pp->value[i] >= 0)
					pp->fix[k++] = i;
		}
		if (pzpartmap(pz, pp))
			return -1;
		if (!(j = pp->nmap))
			j = 1;
		k = pp->row * j;
		pp->col = ((pz->win / k) * k) / j;

		/*
		 * allocate the global tables
		 */

		if (pp->row > pz->mrow)
		{
			m = pz->mrow = roundof(pp->row, 1024);
			n = ((pz->win / 8 / m) + 8 ) * m;
			if (!(pz->val = vmnewof(pz->vm, pz->val, unsigned char, n, 0)) ||
			    !(pz->pat = vmnewof(pz->vm, pz->pat, unsigned char, m, 0)))
				return pznospace(pz);
		}
	}
	if (pz->win > pz->mwin)
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, -1, "%s: pzpartinit: win=%I*u mwin=%I*u buf=%p", pz->path, sizeof(pz->win), pz->win, sizeof(pz->mwin), pz->mwin, pz->buf);
		pz->mwin = roundof(pz->win, 32);
		n = pz->mwin;
		if (pz->flags & PZ_WRITE)
			n *= 2;
		if (!(pz->buf = vmnewof(pz->vm, pz->buf, unsigned char, n, 0)))
			return pznospace(pz);
		if (pz->flags & PZ_WRITE)
			pz->wrk = pz->buf + pz->mwin;
	}
	if (m && !(pz->flags & PZ_WRITE) && !(pz->wrk = vmnewof(pz->vm, pz->wrk, unsigned char, m, 0)))
		return pznospace(pz);

	/*
	 * the discipline functions may change the partition name
	 */

	n = pp->nfix;
	pz->part = pp;
	if (pz->options && pzoptions(pz, pp, pz->options, 0))
		return -1;
	if (pz->disc->eventf && (*pz->disc->eventf)(pz, PZ_PARTITION, pp, 0, pz->disc) < 0)
		return -1;
	if (pp->flags & PZ_VARIABLE)
		return 0;
	if (pp->nfix != n)
	{
		if (!(pp->fix = vmnewof(pz->vm, pp->fix, size_t, VECTOR(pz, pp, pp->nfix), 0)))
			return pznospace(pz);
		for (i = k = 0; i < pp->row; i++)
			if (pp->value[i] >= 0)
				pp->low[pp->fix[k++] = i] = 0;
	}

	/*
	 * update the partition dictionary
	 * no dictionary if there's only one part
	 */

	if (pz->mainpart)
	{
		if (!pz->partdict)
		{
			pz->partdisc.key = offsetof(Pzpart_t, name);
			pz->partdisc.size = -1;
			pz->partdisc.link = offsetof(Pzpart_t, link);
			if (!(pz->partdict = dtopen(&pz->partdisc, Dtoset)))
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, 2, "partition dictionary open error");
				return -1;
			}
			dtinsert(pz->partdict, pz->mainpart);
		}
		dtinsert(pz->partdict, pp);
	}
	else
	{
		pz->mainpart = pp;
		if ((pz->flags & (PZ_DUMP|PZ_VERBOSE)) && !(pz->flags & PZ_SPLIT))
			pzheadprint(pz, sfstderr, 0);
	}
	if ((pz->flags & PZ_DUMP) && !(pz->flags & PZ_SPLIT))
		pzpartprint(pz, pp, sfstderr);
	return 0;
}

/*
 * parse the run time options
 */

int
pzoptions(register Pz_t* pz, register Pzpart_t* pp, char* options, int must)
{
	register char*		s = options;
	char*			e;
	char*			b;
	int			i;
	int			k;
	int			n;
	int			r;
	int			x;
	int			skip;
	Sfio_t*			sp;
	Pzpart_t*		opp;

	optget(NiL, usage);
	skip = 0;
	for (;;)
	{
		for (; isspace(*s) || *s == ','; s++);
		if (!*s)
			break;
		switch (range(pz, pp, s, &e, &n, &x))
		{
		case -1:
			return -1;
		case 1:
			s = e;
			if (*s++ != '=')
				s = "";
			if ((k = value(pz, s, &e)) < 0)
				return -1;
			s = e;
			if (pp && fixed(pz, pp, n, x, k))
				return -1;
			continue;
		}
		b = s;
		opt_info.offset = 0;
		switch (optstr(s, NiL))
		{
		case 0:
			break;
		case '#':
			skip = !streq(opt_info.name, "pzip");
			continue;
		default:
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, -2, "pzoptions: %-.*s", opt_info.offset, s);
			s += opt_info.offset;
			if (skip)
				continue;

			/*
			 * the discipline gets the first crack
			 *
			 *	-1	error
			 *	 0	noticed but not consumed
			 *	 1	consumed
			 */

			if (pz->disc->eventf)
			{
				opp = pz->part;
				pz->part = pp;
				x = (*pz->disc->eventf)(pz, PZ_OPTION, opt_info.argv[1] + 2, 0, pz->disc);
				pz->part = opp;
				if (x < 0)
					return -1;
			}
			else
				x = 0;
			if (!x)
			{
				x = 2;
				switch (optstr(NiL, usage))
				{
				case 'a':
					if (opt_info.num)
						pz->flags |= PZ_APPEND;
					else
						pz->flags &= ~PZ_APPEND;
					break;
				case 'c':
					if (!pz->disc->comment && !(pz->disc->comment = vmstrdup(pz->vm, opt_info.arg)))
						return pznospace(pz);
					break;
				case 'x':
					if (opt_info.num)
						pz->flags |= PZ_CRC;
					else
						pz->flags &= ~PZ_CRC;
					break;
				case 'd':
					error_info.trace = -opt_info.num;
					break;
				case 'D':
					if (opt_info.num)
						pz->flags |= PZ_DUMP;
					else
						pz->flags &= ~PZ_DUMP;
					break;
				case 'G':
					if (!opt_info.num)
						pz->flags |= PZ_NOGZIP;
					else
						pz->flags &= ~PZ_NOGZIP;
					break;
				case 'i':
					if (pz->pin && (sp = pzfind(pz, opt_info.arg, PZ_PART_SUF, "r")))
					{
						sfstack(pz->pin, sp);
						return 0;
					}
					break;
				case 'l':
					if (!pz->pin || !sfstacked(pz->pin))
					{
						if (i = pz->options == options)
							pz->options = 0;
						r = pzlib(pz, opt_info.arg, 0);
						if (pz->disc->errorf)
							(*pz->disc->errorf)(pz, pz->disc, -2, "pzlib: %s status=%d", opt_info.arg, r);
						if (i)
							pz->options = options;
						if (r < 0)
							return -1;
					}
					break;
				case 'n':
					pz->partname = vmstrdup(pz->vm, opt_info.arg);
					break;
				case 'X':
					if (pz->prefix.count = strton(opt_info.arg, &e, NiL, 0))
					{
						pz->prefix.terminator = -1;
						if (*e == 'x' || *e == 'X' || *e == '*' || *e == '-')
							e++;
						if (*e == 'l' || *e == 'L')
							pz->prefix.terminator = '\n';
						else
						{
							if (*e == '"' || *e == '\'')
							{
								pz->prefix.terminator = chresc(e + 1, &e);
								for (s = e; *s && !isspace(*s) && *s != ','; s++);
							}
							else if (*e)
							{
								pz->prefix.count = 0;
								if (pz->disc->errorf)
									(*pz->disc->errorf)(pz, pz->disc, 2, "%s: prefix expression expected", opt_info.arg);
								return -1;
							}
						}
					}
					else if (e > opt_info.arg)
						pz->prefix.skip = 1;
					break;
				case 'O':
					if (opt_info.num)
						pz->flags |= PZ_SORT;
					else
						pz->flags &= ~PZ_SORT;
					break;
				case 'P':
					if (!opt_info.num)
						pz->flags |= PZ_NOPZIP;
					else
						pz->flags &= ~PZ_NOPZIP;
					break;
				case 'Q':
					if (opt_info.num)
						pz->flags |= PZ_REGRESS;
					else
						pz->flags &= ~PZ_REGRESS;
					break;
				case 'r':
					pz->row = opt_info.num;
					break;
				case 'S':
					if (opt_info.num)
					{
						pz->flags |= PZ_FORCE|PZ_SPLIT|PZ_SECTION;
						if (opt_info.arg)
							pz->split.match = vmstrdup(pz->vm, opt_info.arg);
					}
					else
						pz->flags &= ~PZ_SPLIT;
					break;
				case 's':
					if (opt_info.num)
						pz->flags |= PZ_SUMMARY;
					else
						pz->flags &= ~PZ_SUMMARY;
					break;
				case 'T':
					pz->test |= opt_info.num;
					break;
				case 'v':
					if (opt_info.num)
						pz->flags |= PZ_VERBOSE;
					else
						pz->flags &= ~PZ_VERBOSE;
					break;
				case '?':
					error(ERROR_USAGE|4, "%s", opt_info.arg);
					return -1;
				case ':':
					if (must && !(pz->flags & PZ_PUSHED))
					{
						if (pz->disc->errorf)
							(*pz->disc->errorf)(pz, pz->disc, 2, "%s", opt_info.arg);
						return -1;
					}
					x = 0;
					break;
				}
			}

			/*
			 * save consumed options for the header and
			 * clear so they are not processed again
			 */

			if (must >= 0 && (x > 1 || x && must))
			{
				if (pz->det)
				{
					if (sfstrtell(pz->det))
						sfputc(pz->det, ' ');
					sfwrite(pz->det, b, s - b);
				}
				memset(b, ' ', s - b);
			}
			continue;
		}
		break;
	}
	return 0;
}

static char*
partline(Pz_t* pz, Sfio_t* sp)
{
	char*	s;

	if ((s = sfgetr(sp, '\n', 1)) && pz->disc->errorf)
		error_info.line++;
	return s;
}

/*
 * parse and load a partition file
 */

int
pzpartition(register Pz_t* pz, const char* partition)
{
	register Pzpart_t*	pp;
	int			i;
	int			k;
	int			m;
	char*			s;
	char*			e;
	char*			t;
	char*			np;
	int			n;
	int			g;
	int			gi;
	int			x;
	int*			cv;
	int*			ce;
	int*			cp;
	int*			gv;
	int*			hv;
	int			line;
	long			f;
	char*			file;
	Sfio_t*			sp;
	Vmalloc_t*		vm;
	char			buf[PATH_MAX];

	if (pz->disc->errorf)
	{
		file = error_info.file;
		line = error_info.line;
	}
	sp = 0;
	vm = 0;
	if (!(s = (char*)partition))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 2, "partition file omitted");
		goto bad;
	}
	if (s[0] == '/' && s[i=strlen(s)-1] == '/')
	{
		if (streq(s, "/") || streq(s, "//") || streq(s, "/gzip/"))
			n = sfsprintf(buf, sizeof(buf), "nopzip\n1\n0-0\n");
		else
		{
			n = (int)strtol(s + 1, &e, 10);
			if (e[0] == '/' && !e[1])
				n = sfsprintf(buf, sizeof(buf), "%d\n0-%d\n", n, n - 1);
			else
			{
				n = sfsprintf(buf, sizeof(buf), "%.*s\n", i - 1, s + 1);
				for (s = buf; *s; s++)
					if (isspace(*s) || *s == ',')
						*s = '\n';
			}
		}
		if (!(sp = sfstropen()) || sfstrbuf(sp, buf, n, 0))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: string stream open error", s);
			goto bad;
		}
	}
	else
	{
		/*
		 * consume url-ish options
		 */

		if ((e = strchr(s, '?')) || (e = strchr(s, '#')))
		{
			if (!(t = vmoldof(pz->vm, 0, char, e - s, 1)))
				goto bad;
			memcpy(t, s, e - s);
			t[e - s] = 0;
			s = t;
			if (*e == '#' && !(pz->partname = vmstrdup(pz->vm, e + 1)) || *e == '?' && pzoptions(pz, NiL, e + 1, 1))
				goto bad;
		}
		if (!(sp = pzfind(pz, s, PZ_PART_SUF, "r")))
			goto bad;
	}
	if (pz->disc->errorf)
	{
		error_info.file = s;
		error_info.line = 0;
	}
	if (!(vm = vmopen(Vmdcheap, Vmlast, 0)))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "partition temporary vmalloc region open error");
		goto bad;
	}
	np = 0;
	pp = 0;
	s = "";
	pz->pin = sp;
	do
	{
		vmclear(vm);
		do
		{
			if (*s != '"' && !(s = partline(pz, sp)))
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, 2, "invalid partition file");
				goto bad;
			}
			for (; isspace(*s); s++);
			if (*s == '"')
			{
				if (np)
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 2, "%s: partition already named", np);
					goto bad;
				}
				for (e = ++s; *s && *s != '"'; s++);
				if (!*s)
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 2, "unbalanced \" in partition name");
					goto bad;
				}
				*s++ = 0;
				if (!(np = vmstrdup(pz->vm, e)))
					goto bad;
				for (; isspace(*s); s++);
			}
			if (isalpha(*s))
			{
				if (pzoptions(pz, pp, s, 1))
					goto bad;
				s = "";
			}
		} while (!(n = strtol(s, &t, 10)));
		if (*t == '@')
		{
			switch (m = strtol(t + 1, &t, 10))
			{
			case 1:
				m = 0;
				break;
			case 2:
				m = 1;
				break;
			case 4:
				m = 2;
				break;
			case 8:
				m = 3;
				break;
			default:
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, 2, "%s: %d: invalid size -- power of 2 from 1..8 expected", s, m);
				goto bad;
			}
			n |= (m << 14);
			f = PZ_VARIABLE;
		}
		else
			f = 0;
		if (pz->flags & PZ_ROWONLY)
		{
			if (!np || !pz->partname || streq(np, pz->partname))
			{
				vmclose(vm);
				pz->row = n;
				return 0;
			}
			np = 0;
			do
			{
				if (!(s = partline(pz, sp)))
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 2, "%s: partition not found", pz->partname);
					goto bad;
				}
				for (; isspace(*s); s++);
			} while (*s != '"');
			continue;
		}
		if (!(pp = vmnewof(pz->vm, 0, Pzpart_t, 1, 0)))
			goto nope;
		pp->row = n;
		pp->flags = f;
		if (np)
		{
			pp->name = np;
			np = 0;
		}
		if (!(cv = vmnewof(vm, 0, int, (pp->row + 1) * 4, 0)))
			goto nope;
		cp = cv;
		ce = hv = cv + (pp->row + 1) * 2;
		gv = hv + pp->row + 1;
		m = 0;
		g = 0;
		for (s = t; isspace(*s); s++);
		if (*s != '-')
			s = partline(pz, sp);
		for (; s; s = partline(pz, sp))
		{
			for (; isspace(*s); s++);
			if (*s == '"')
				break;
			else if (isalpha(*s))
			{
				if (pzoptions(pz, pp, s, 1))
					goto bad;
				continue;
			}
			gi = 0;
			for (;;)
			{
				if (range(pz, pp, s, &e, &n, &x) <= 0)
				{
					if (*e && *e != '#')
						goto bad;
					break;
				}
				s = e;
				if (*s == '=')
				{
					if ((k = value(pz, ++s, &e)) < 0)
						return -1;
					s = e;
					if (fixed(pz, pp, n, x, k))
						goto bad;
					continue;
				}
				if (cp >= ce)
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 2, "too many columns");
					goto bad;
				}
				for (; n <= x; n++)
					if (!pp->value || pp->value[n] < 0)
					{
						if (hv[n])
						{
							if (pz->disc->errorf)
								(*pz->disc->errorf)(pz, pz->disc, 2, "column %d already specified", n);
							goto bad;
						}
						hv[n] = 1;
						*cp++ = n;
						gv[m] = g;
						m++;
						gi = 1;
					}
			}
			if (gi)
			{
				if (cp >= ce)
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 2, "too many columns");
					goto bad;
				}
				*cp++ = -1;
				g += gi;
			}
		}
		if (cp >= ce)
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "too many columns");
			goto bad;
		}
		*cp++ = -1;

		/*
		 * allocate the map data and work space
		 */

		pp->nmap = m;
		if (!(pp->ngrp = g))
			pp->ngrp = 1;
		if (!(pp->map = vmnewof(pz->vm, 0, size_t, VECTOR(pz, pp, pp->nmap), 0)) ||
		    !(pp->grp = vmnewof(pz->vm, 0, size_t, VECTOR(pz, pp, pp->ngrp), 0)))
			goto nope;
		m = 0;
		cp = cv;
		g = 0;
		k = 0;
		while ((i = *cp++) >= 0)
			do
			{
				if (g != gv[m])
				{
					pp->grp[g] = k;
					g = gv[m];
					k = 0;
				}
				pp->map[m] = i;
				k++;
				m++;
			} while ((i = *cp++) >= 0);
		pp->grp[g] = k;
		if (pzpartinit(pz, pp, partition))
			goto bad;
	} while (s);
	sfclose(sp);
	sp = 0;
	vmclose(vm);
	vm = 0;
	if (pz->partname)
	{
		if (!(pp = pzpartget(pz, pz->partname)))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: partition not found", pz->partname);
			goto bad;
		}
		pz->flags |= PZ_MAINONLY;
		pz->part = pz->mainpart = pp;
	}
	if (pz->disc->errorf)
	{
		error_info.file = file;
		error_info.line = line;
	}
	pz->pin = 0;
	return 0;
 nope:
	pznospace(pz);
 bad:
	pz->pin = 0;
	if (sp)
		sfclose(sp);
	if (vm)
		vmclose(vm);
	if (pz->disc->errorf)
	{
		error_info.file = file;
		error_info.line = line;
	}
	return -1;
}

/*
 * allocate and read an array from the input partition header
 */

static int
array(register Pz_t* pz, Pzpart_t* pp, size_t** pv, size_t* pn, size_t check)
{
	register size_t		n;
	register size_t		m;
	register size_t*	v;

	n = sfgetu(pz->io);
	if (check && n > check)
		return -1;
	if (pv)
	{
		if (!n)
			v = 0;
		else if (!(v = vmnewof(pz->vm, *pv, size_t, VECTOR(pz, pp, n), 0)))
			return pznospace(pz);
		*pv = v;
		if (pn)
			*pn = n;
		while (n--)
		{
			m = sfgetu(pz->io);
			if (check && m >= check)
				return -1;
			*v++ = m;
		}
	}
	else
		while (n--)
		{
			m = sfgetu(pz->io);
			if (check && m >= check)
				return -1;
		}
	return 0;
}

/*
 * allocate and read a buffer from the input partition header
 */

static int
buffer(register Pz_t* pz, Pzpart_t* pp, char** pv, size_t* pn)
{
	register size_t		n;
	register char*		v;

	if (!(n = sfgetu(pz->io)))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 2, "%s: partition header corrupted", pz->path);
		return -1;
	}
	if (pv)
	{
		if (!(v = vmnewof(pz->vm, *pv, char, n, 0)))
			return pznospace(pz);
		*pv = v;
		if (pn)
			*pn = n;
		sfread(pz->io, v, n);
	}
	else
		sfseek(pz->io, (Sflong_t)n, SEEK_CUR);
	return 0;
}

/*
 * read a pz partition header(s) from pz->io
 */

int
pzpartread(register Pz_t* pz)
{
	register Pzpart_t*	pp;
	register int		i;
	Pzpart_t*		po;

	if (pz->major > 1)
	{
		if (!(i = sfgetc(pz->io)))
			return 0;
		if (i == EOF)
			return -1;
		sfungetc(pz->io, i);
	}
	if (!(pp = vmnewof(pz->vm, 0, Pzpart_t, 1, 0)))
		return pznospace(pz);
	if (pz->major == 1)
	{
		pp->row = sfgetu(pz->io);
		pp->col = sfgetu(pz->io);
		pz->win = sfgetu(pz->io);
	}
	po = 0;
	for (;;)
	{
		switch (i = sfgetc(pz->io))
		{
		case EOF:
			goto bad;
		case 0:
			if (!po && pzpartinit(pz, pp, NiL))
				return -1;
			break;
		case PZ_HDR_comment:
			buffer(pz, pp, (char**)&pz->disc->comment, NiL);
			continue;
		case PZ_HDR_fix:
			if (array(pz, pp, &pp->fix, &pp->nfix, pp->row))
				goto bad;
			if (!pp->value)
			{
				if (!(pp->value = vmnewof(pz->vm, 0, int, pp->row, 0)))
					return pznospace(pz);
				for (i = 0; i < pp->row; i++)
					pp->value[i] = -1;
			}
			for (i = 0; i < pp->nfix; i++)
				if (pp->value[pp->fix[i]] < 0)
					pp->value[pp->fix[i]] = ' ';
			continue;
		case PZ_HDR_grp:
			if (array(pz, pp, &pp->grp, &pp->ngrp, pp->row + 1))
				goto bad;
			continue;
		case PZ_HDR_map:
			if (array(pz, pp, &pp->map, &pp->nmap, pp->row))
				goto bad;
			continue;
		case PZ_HDR_options:
			buffer(pz, pp, (char**)&pz->headoptions, NiL);
			continue;
		case PZ_HDR_prefix:
			buffer(pz, pp, &pz->prefix.data, &pz->prefix.count);
			pz->prefix.terminator = -1;
			continue;
		case PZ_HDR_part:
			if (pp->row)
			{
				if (!po && pzpartinit(pz, pp, NiL))
					return -1;
				if (pp = pz->freepart)
					pz->freepart = 0;
				else if (!(pp = vmnewof(pz->vm, 0, Pzpart_t, 1, 0)))
					return pznospace(pz);
			}
			buffer(pz, pp, (char**)&pp->name, NiL);
			pp->row = sfgetu(pz->io);
			pp->col = sfgetu(pz->io);
			if (pz->partdict && (po = (Pzpart_t*)dtsearch(pz->partdict, pp)) || (po = pz->mainpart) && streq(pp->name, po->name))
			{
				if (pp->row != po->row || pp->col != po->col)
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 1, "%s: %s: partition redefinition ignored", pz->path, pp->name);
				}
				else if (pz->flags & PZ_DUMP)
					sfprintf(sfstderr, "\n# %s benign redefinition\n", pp->name);
				vmfree(pz->vm, pp->name);
				pz->freepart = pp;
				pp->name = 0;
				pp = po;
				for (;;)
				{
					switch (i = sfgetc(pz->io))
					{
					case EOF:
						break;
					case 0:
					case PZ_HDR_part:
						sfungetc(pz->io, i);
						break;
					default:
						if (PZ_HDR_ARR(i))
							array(pz, pp, NiL, NiL, 0);
						else if (PZ_HDR_BUF(i))
							buffer(pz, pp, NiL, NiL);
						else
							goto bad;
						continue;
					}
					break;
				}
			}
			else
				po = 0;
			continue;
		case PZ_HDR_split:
			array(pz, pp, &pz->split.data, &pz->split.size, 0);
			pz->split.flags |= PZ_SPLIT_INFLATE;
			pz->flags |= PZ_SECTION;
			continue;
		default:
			if (PZ_HDR_ARR(i))
				array(pz, pp, NiL, NiL, 0);
			else if (PZ_HDR_BUF(i))
				buffer(pz, pp, NiL, NiL);
			else
				goto bad;
			continue;
		}
		break;
	}
	return 0;
 bad:
	if (pz->disc->errorf)
		(*pz->disc->errorf)(pz, pz->disc, 2, "%s: partition header corrupted", pz->path);
	return -1;
}

/*
 * write any new pz partition headers to op
 */

int
pzpartwrite(Pz_t* pz, Sfio_t* op)
{
	register Pzpart_t*	pp;
	register int		i;
	register size_t		m;
	int			all;

	if (pz->flags & PZ_MAINONLY)
	{
		pz->flags &= ~PZ_MAINONLY;
		all = 0;
		pp = pz->mainpart;
	}
	else if (pz->partdict)
	{
		all = 1;
		pp = (Pzpart_t*)dtfirst(pz->partdict);
	}
	else
	{
		all = 0;
		pp = pz->part;
	}
	while (pp)
	{
		if ((pp->flags & (PZ_UPDATE|PZ_HEAD)) == PZ_UPDATE)
		{
			pp->flags |= PZ_HEAD;
			sfputc(op, PZ_HDR_part);
			m = strlen(pp->name) + 1;
			sfputu(op, m);
			sfwrite(op, pp->name, m);
			sfputu(op, pp->row);
			sfputu(op, pp->col);
			if (pp->nmap != pp->row || pp->ngrp != 1)
			{
				sfputc(op, PZ_HDR_map);
				sfputu(op, pp->nmap);
				for (i = 0; i < pp->nmap; i++)
					sfputu(op, pp->map[i]);
				sfputc(op, PZ_HDR_grp);
				sfputu(op, pp->ngrp);
				for (i = 0; i < pp->ngrp; i++)
					sfputu(op, pp->grp[i]);
			}
			if (pp->nfix)
			{
				sfputc(op, PZ_HDR_fix);
				sfputu(op, pp->nfix);
				for (i = 0; i < pp->nfix; i++)
					sfputu(op, pp->fix[i]);
			}
			if ((pz->split.flags & (PZ_SPLIT_DEFLATE|PZ_SPLIT_HEADER)) == PZ_SPLIT_DEFLATE)
			{
				pz->split.flags |= PZ_SPLIT_HEADER;
				sfputc(op, PZ_HDR_split);
				sfputu(op, 0);
			}
		}
		if (!all)
			break;
		pp = (Pzpart_t*)dtnext(pz->partdict, pp);
	}
	sfputc(op, 0);
	return 0;
}

/*
 * pretty print pp on op
 */

int
pzpartprint(Pz_t* pz, register Pzpart_t* pp, register Sfio_t* op)
{
	register int		i;
	register int		j;
	register int		g;
	register char*		s;
	char			esc[2];

	sfprintf(op, "\n\"%s\"\n", pp->name);
	sfprintf(op, "\n%I*u\t# high frequency %I*u\n", sizeof(pp->row), pp->row, sizeof(pp->nmap), pp->nmap);
	if (pp->nfix)
	{
		sfprintf(op, "\n");
		esc[1] = 0;
		for (i = 0; i < pp->nfix; i++)
		{
			for (j = i + 1; j < pp->nfix && pp->fix[j] == pp->fix[j - 1] + 1 && pp->value[pp->fix[j]] == pp->value[pp->fix[j - 1]]; j++);
			sfprintf(op, "%I*u", sizeof(pp->fix[i]), pp->fix[i]);
			if (j > (i + 2))
			{
				i = j - 1;
				sfprintf(op, "-%I*u", sizeof(pp->fix[i]), pp->fix[i]);
			}
			s = fmtesc((esc[0] = pp->value[pp->fix[i]], esc));
			j = *s == '\'' ? '"' : '\'';
			sfprintf(op, "=%c%s%c\n", j, s, j);
		}
	}
	g = -1;
	for (i = 0; i < pp->nmap; i++)
	{
		if (g != pp->lab[i])
		{
			g = pp->lab[i];
			sfprintf(op, "\n");
		}
		else
			sfprintf(op, " ");
		for (j = i + 1; j < pp->nmap && pp->map[j] == pp->map[j - 1] + 1 && pp->lab[j] == g; j++);
		sfprintf(op, "%I*u", sizeof(pp->map[i]), pp->map[i]);
		if (j > (i + 2))
		{
			i = j - 1;
			sfprintf(op, "-%I*u", sizeof(pp->map[i]), pp->map[i]);
		}
	}
	sfprintf(op, "\n");
	return sferror(op) ? -1 : 0;
}
