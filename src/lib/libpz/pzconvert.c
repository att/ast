/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1998-2011 AT&T Intellectual Property          *
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
 * pzip conversion/checksum discipline
 *
 *	pz->discdata		State_t*
 *	pp->discdata		Cvt_t*
 */

static const char usage[] =
"[-1l?\n@(#)$Id: pzip conversion library (AT&T Research) 1999-09-11 $\n]"
USAGE_LICENSE
"[+LIBRARY?pzconvert - pzip conversion library]"
"[+DESCRIPTION?The \bpzip\b convert discipline supports runtime record"
"	format conversion. The discipline is enabled by a \b--library\b"
"	that provides a conversion table and functions to \bpzconvert\b(3).]"
"[x:checksum?Enables a decompressed data checksum. The checksum is appended"
"	to the compressed data as a \bpzip\b trailer. This checksum is"
"	checked on decompression and a diagnostic is issued on mismatch."
"	The absence of a checksum trailer is not treated as an error unless"
"	\b--checksum=warn\b is specified.]:?[warn]"
"[c:convert?Specifies the input format for compression and the output format"
"	for decompression. \aformat\a may be omitted for self-identifying"
"	input data (e.g., \bpzip\b files.)]:?[format]"
"[f:from?Specifies the input format for data that does not self-identify.]:"
"	[format]"
"[s:show?Lists the conversion steps, if any, on the standard output, and exits."
"	A diagnostic is issued if the conversion is not implemented. If"
"	\ball\b is specified then a description of all supported formats is"
"	listed on the standard output and \bpzip\b exits.]:?[all]"
"[t:to?Specifies the decompression output format. This option is not needed"
"	for compression since the partition file determines the output"
"	format.]:[format]"
"[v:verbose?Emits a message on the standard error if checksumming is enabled"
"	and another message showing the conversions being applied, if any.]"
;

#include "pzlib.h"

#define CHECKSUM_OP	SFDCEVENT('P','Z',1)

#define CHECKSUM	(1<<0)		/* checksum enabled		*/
#define CHECKSUM_TAIL	(1<<1)		/* CHECKSUM tail handled	*/
#define CHECKSUM_WARN	(1<<2)		/* warn if no checksum trailer	*/
#define CONVERT		(1<<3)		/* conversion enabled		*/
#define SHOW		(1<<4)		/* show conversions and exit	*/

struct Chain_s; typedef struct Chain_s Chain_t;

struct Chain_s
{
	Chain_t*	next;
	Pzconvert_t*	convert;
	unsigned char*	buf;
};

typedef struct
{
	unsigned long	flags;
	uint32_t	checksum;
	Chain_t*	chain;
	Chain_t*	last;
} Cvt_t;

typedef struct
{
	unsigned long	flags;
	char*		from;
	char*		to;
	unsigned char*	buf;
	Pzconvert_t*	conversions;
	Pzread_f	readf;
	Pzwrite_f	writef;
	Pzevent_f	eventf;
	Sfio_t*		tmp;
} State_t;

/*
 * compute the incremental linear congruential hash checksum
 */

static uint32_t
memsum_4(register uint32_t sum, const void* buf, size_t size)
{
	register unsigned char*	s = (unsigned char*)buf;
	register unsigned char*	e = s + size;

	while (s < e)
		sum = (sum << 4) + sum + *s++ + 97531;
	return sum;
}

/*
 * read a row from sp and apply the conversion and/or checksum
 * before it is deflated
 */

static ssize_t
cvtread(Pz_t* pz, Sfio_t* sp, void* data, Pzdisc_t* disc)
{
	register State_t*	state = (State_t*)pz->discdata;
	register Pzpart_t*	pp = pz->part;
	register Cvt_t*		cvt = (Cvt_t*)pp->discdata;
	register Chain_t*	cp;
	ssize_t			n;
	unsigned char*		s;
	unsigned char*		t;

	if (cvt->flags & CONVERT)
	{
		cvt->last->buf = (unsigned char*)data;
	again:
		cp = cvt->chain;
		if (state->readf)
		{
			s = state->buf;
			if ((n = (*state->readf)(pz, sp, s, disc)) <= 0)
				return n;
		}
		else if (!(s = (unsigned char*)sfreserve(sp, cp->convert->from->row, 0)))
			return sfvalue(sp) ? -1 : 0;
		for (; cp; cp = cp->next)
		{
			pz->count.converted++;
			t = cp->buf;
			if ((n = (*cp->convert->convertf)(pz, cp->convert, s, t, disc)) <= 0)
			{
				if (n < 0)
					return -1;
				goto again;
			}
			s = t;
		}
	}
	else
		n = state->readf ? (*state->readf)(pz, sp, data, disc) : sfread(sp, data, pp->row);
	if (n > 0 && (cvt->flags & CHECKSUM))
		cvt->checksum = memsum_4(cvt->checksum, data, n);
	return n;
}

/*
 * apply the conversion and/or checksum to a row that was just inflated
 * and write it to sp
 */

static ssize_t
cvtwrite(Pz_t* pz, Sfio_t* sp, const void* data, Pzdisc_t* disc)
{
	register State_t*	state = (State_t*)pz->discdata;
	register Pzpart_t*	pp = pz->part;
	register Cvt_t*		cvt = (Cvt_t*)pp->discdata;
	register Chain_t*	cp;
	ssize_t			n;
	unsigned char*		b;
	unsigned char*		s;
	unsigned char*		t;

	if (cvt->flags & CONVERT)
	{
		if (state->writef)
			b = state->buf;
		else if (!(b = (unsigned char*)sfreserve(sp, cvt->last->convert->to->row, 1)))
			return -1;
		cvt->last->buf = b;
		s = (unsigned char*)data;
		for (cp = cvt->chain; cp; cp = cp->next)
		{
			pz->count.converted++;
			t = cp->buf;
			if ((n = (*cp->convert->convertf)(pz, cp->convert, s, t, disc)) <= 0)
			{
				if (!state->writef)
					sfwrite(sp, b, 0);
				return n;
			}
			s = t;
		}
	}
	else
	{
		if (cvt->flags & CHECKSUM)
			cvt->checksum = memsum_4(cvt->checksum, data, pp->row);
		b = (unsigned char*)data;
		n = pp->row;
	}
	return state->writef ? (*state->writef)(pz, sp, b, disc) : sfwrite(sp, b, n);
}

/*
 * add the next links to the stack that get from stk[i..j] => to
 * the recursion unwind returns a shortest chain of links
 * 0 returned if there is no path => to
 */

static Chain_t*
closure(Pz_t* pz, Chain_t* cp, int n, Pzconvert_t* tab, unsigned char* hit, Pzconvert_t** stk, int i, int j, const char* to)
{
	int		k;
	int		m;
	Chain_t*	xp;

	for (k = j; i < j; i++)
		for (m = 0; m < n; m++)
			if (!hit[m] && streq(tab[m].from->name, stk[i]->to->name))
			{
				if (streq(tab[m].to->name, to))
				{
					stk[k] = &tab[m];
					m = k;
					goto found;
				}
				stk[k++] = &tab[m];
				hit[m] = 1;
			}
	if (k == j || !(cp = closure(pz, cp, n, tab, hit, stk, j, k, to)))
		return 0;
	to = cp->convert->from->name;
	for (m = j; m < k && !streq(stk[m]->to->name, to); m++);
	if (m >= k)
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 2, "internal closure error -- %s not found on unwind stack", to);
		return 0;
	}
 found:
	xp = vmnewof(pz->vm, 0, Chain_t, 1, 0);
	xp->next = cp;
	xp->convert = stk[m];
	return xp;
}

/*
 * determine a shortest path of conversions from f => t in tab of n elements
 */

static Chain_t*
chain(Pz_t* pz, Pzconvert_t* tab, int n, const char* f, const char* t)
{
	register State_t*	state = (State_t*)pz->discdata;
	int			i;
	int			j;
	size_t			m;
	unsigned char*		a;
	unsigned char*		b;
	unsigned char*		x;
	Chain_t*		cp;
	Chain_t*		tp;
	unsigned char*		hit;
	Pzconvert_t**		stk;

	if (!(hit = newof(0, unsigned char, n, 0)))
		return 0;
	if (!(stk = newof(0, Pzconvert_t*, n, 0)))
	{
		free(hit);
		return 0;
	}
	for (i = j = 0; i < n; i++)
		if (streq(tab[i].from->name, f))
		{
			stk[j++] = &tab[i];
			hit[i] = 1;
		}
	if (!j || !(cp = closure(pz, NiL, n, tab, hit, stk, 0, j, t)))
	{
		free(hit);
		free(stk);
		if (pz->disc->errorf)
		{
			if (!j)
				(*pz->disc->errorf)(pz, pz->disc, 2, "conversion to %s not implemented", t);
			else
				(*pz->disc->errorf)(pz, pz->disc, 2, "conversion from %s to %s not implemented", f, t);
		}
		return 0;
	}
	free(hit);
	t = cp->convert->from->name;
	for (i = 0; i < j && !streq(stk[i]->to->name, t); i++);
	if (i >= j)
	{
		free(stk);
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 2, "internal closure error -- %s not found on unwind stack", t);
		return 0;
	}
	tp = vmnewof(pz->vm, 0, Chain_t, 1, 0);
	tp->next = cp;
	tp->convert = stk[i];
	cp = tp;
	free(stk);

	/*
	 * determine the largest convert to row size
	 * and allocate the temporary buffers
	 */

	m = 0;
	for (tp = cp; tp && tp->next; tp = tp->next)
		if (tp->convert->to->row > m)
			m = tp->convert->to->row;
	if (m)
	{
		a = b = 0;
		for (tp = cp; tp && tp->next; tp = tp->next)
		{
			if (!a && !(a = vmnewof(pz->vm, 0, unsigned char, m, 0)))
				return 0;
			tp->buf = a;
			x = a;
			a = b;
			b = x;
		}
		if ((state->readf || state->writef) && !(state->buf = vmnewof(pz->vm, 0, unsigned char, m, 0)))
			return 0;
	}
	return cp;
}

/*
 * handle pzip events
 */

static int
cvtevent(Pz_t* pz, int op, void* data, size_t size, Pzdisc_t* disc)
{
	register State_t*	state = (State_t*)pz->discdata;
	register Pzpart_t*	pp = pz->part;
	register Cvt_t*		cvt;
	register Pzconvert_t*	xp;
	Chain_t*		cp;
	Pzconvert_t*		zp;
	Pzconvert_t*		rp;
	char*			f;
	char*			t;
	char**			vp;
	int			i;
	size_t			n;
	unsigned long		k;
	Sfio_t*			sp;
	Pz_t*			iz;

	int			r = 0;

	if (state->eventf && (r = (*state->eventf)(pz, op, data, size, disc)))
		return r;
	if (!pp)
	{
		if (op == PZ_OPTION)
		{
			switch (optstr(NiL, usage))
			{
			case 'f':
				if (!pz->row)
					for (xp = state->conversions; ; xp++)
						if (!xp->from)
						{
							if (disc->errorf)
								(*disc->errorf)(pz, disc, 2, "%s: unknown format", opt_info.arg);
							return -1;
						}
						else if (streq(opt_info.arg, xp->from->name))
						{
							pz->row = xp->from->row;
							break;
						}
						else if (streq(opt_info.arg, xp->to->name))
						{
							pz->row = xp->to->row;
							break;
						}
				break;
			case 's':
				if (opt_info.arg)
				{
					for (xp = state->conversions; xp->from; xp++);
					i = (xp - state->conversions) * 2 + 1;
					if (!(vp = vmnewof(pz->vm, 0, char*, i, 0)))
						exit(1);
					sfprintf(sfstdout, "%-16s  %5s  %s\n", "NAME", "ROW", "DESCRIPTION");
					for (xp = state->conversions; xp->from; xp++)
					{
						for (i = 0; vp[i] && !streq(xp->from->name, vp[i]); i++);
						if (!vp[i])
						{
							vp[i++] = (char*)xp->from->name;
							sfprintf(sfstdout, "%-16s  %5u  %s\n", xp->from->name, xp->from->row, xp->from->description);
						}
						for (i = 0; vp[i] && !streq(xp->to->name, vp[i]); i++);
						if (!vp[i])
						{
							vp[i++] = (char*)xp->to->name;
							sfprintf(sfstdout, "%-16s  %5u  %s\n", xp->to->name, xp->to->row, xp->to->description);
						}
					}
					exit(0);
				}
				break;
			}
		}
		return 0;
	}
	if (!(cvt = (Cvt_t*)pp->discdata))
	{
		if (!(cvt = vmnewof(pz->vm, 0, Cvt_t, 1, 0)))
			return -1;
		pp->discdata = (void*)cvt;
	}
	switch (op)
	{
	case PZ_CLOSE:
		if ((pz->flags & PZ_READ) && (cvt->flags & (CHECKSUM|CHECKSUM_WARN|CHECKSUM_TAIL)) == (CHECKSUM|CHECKSUM_WARN))
		{
			r = -1;
			if (disc->errorf)
				(*disc->errorf)(pz, disc, 1, "%s: no checksum -- expected 0x%08I*x", pz->path, sizeof(cvt->checksum), cvt->checksum);
		}
		if (cvt->flags & CONVERT)
			for (cp = cvt->chain; cp; cp = cp->next)
				if (cp->convert->eventf && (r = (*cp->convert->eventf)(pz, op, cp->convert, cp->next == 0, disc)))
					break;
		sfstrclose(state->tmp);
		break;
	case PZ_OPTION:
		switch (optstr(NiL, usage))
		{
		case 'x':
			r = 1;
			state->flags |= CHECKSUM;
			if (opt_info.arg)
				state->flags |= CHECKSUM_WARN;
			if (disc->errorf && (pz->flags & PZ_NOPZIP) && (state->flags & CHECKSUM_WARN))
				(*disc->errorf)(pz, disc, 1, "%s: enabled for pzip data only", opt_info.name);
			break;
		case 'c':
		case 'f':
		case 't':
			r = 1;
			if (opt_info.arg && *opt_info.arg)
			{
				for (xp = state->conversions;; xp++)
				{
					if (!xp->from)
					{
						if (disc->errorf)
							(*disc->errorf)(pz, disc, 2, "%s: unknown format", opt_info.arg);
						return -1;
					}
					if (streq(opt_info.arg, xp->from->name) || streq(opt_info.arg, xp->to->name))
						break;
				}
				switch (opt_info.option[1])
				{
				case 'c':
					vp = (pz->flags & PZ_WRITE) ? &state->from : &state->to;
					break;
				case 'f':
					vp = &state->from;
					break;
				case 't':
					vp = &state->to;
					break;
				}
				if ((!*vp || !streq(opt_info.arg, *vp)) && !(*vp = vmstrdup(pz->vm, opt_info.arg)))
					return -1;
				state->flags |= CONVERT;
			}
			else if (pz->flags & PZ_WRITE)
				state->flags |= CONVERT;
			break;
		case 's':
			r = 1;
			state->flags |= SHOW;
			break;
		default:
			if (cvt->flags & CONVERT)
				for (cp = cvt->chain; cp; cp = cp->next)
					if (cp->convert->eventf && (r = (*cp->convert->eventf)(pz, op, data, size, disc)))
						break;
			break;
		}
		break;
	case PZ_PARTITION:
		if (state->flags & CONVERT)
		{
			if (pz->flags & PZ_WRITE)
			{
				if (sfraise(pz->io, SFPZ_HANDLE, &iz) <= 0)
				{
					if (disc->errorf)
						(*disc->errorf)(pz, disc, 2, "%s: cannot determine input format", pz->path);
					return -1;
				}
				if (state->from && !*iz->part->name)
				{
					n = 0;
					f = state->from;
				}
				else
				{
					n = iz->part->row;
					f = iz->part->name;
				}
				t = pp->name;
			}
			else
			{
				if (!state->to)
				{
					if (disc->errorf)
						(*disc->errorf)(pz, disc, 2, "ouput convert format omitted");
					return -1;
				}
				if (state->from && !*pp->name)
				{
					n = 0;
					f = state->from;
				}
				else
				{
					n = *pp->name ? 0 : pp->row;
					f = pp->name;
				}
				t = state->to;
			}
			if (!streq(f, t))
			{
				cp = 0;
				for (xp = state->conversions, rp = 0, zp = 0;; xp++)
				{
					if (!xp->from)
					{
						i = xp - state->conversions;
						if (xp = zp)
						{
							if (n != xp->to->row)
							{
								if (!(cp = vmnewof(pz->vm, 0, Chain_t, 1, 0)))
									return -1;
								cp->convert = xp;
							}
							break;
						}
						if (rp)
							break;
						if (cp = chain(pz, state->conversions, i, f, t))
							break;
						return -1;
					}
					if (streq(t, xp->to->name))
					{
						if (streq(f, xp->from->name))
						{
							if (!(cp = vmnewof(pz->vm, 0, Chain_t, 1, 0)))
								return -1;
							cp->convert = xp;
							break;
						}
						if (!zp && n == xp->from->row)
							zp = xp;
						if (!rp && n == xp->to->row)
							rp = xp;
					}
				}
				if (cvt->chain = cp)
				{
					cvt->flags |= CONVERT;
					do
					{
						if (cp->convert->eventf && (*cp->convert->eventf)(pz, op, cp->convert, cp->next == 0, disc) < 0)
							return -1;
						cvt->last = cp;
					} while (cp = cp->next);
				}
			}
		}
		if ((cvt->flags |= (state->flags & CHECKSUM)) & (CHECKSUM|CONVERT))
		{
			disc->readf = cvtread;
			disc->writef = cvtwrite;
			if (disc->errorf && (pz->flags & PZ_VERBOSE))
			{
				if (cvt->flags & CHECKSUM)
					(*disc->errorf)(pz, disc, 0, "%s: %s checksum", pz->path, (pz->flags & PZ_WRITE) ? "generating" : "verifying");
				if (cvt->flags & CONVERT)
				{
					sfprintf(pz->tmp, "%s", cvt->chain->convert->from->name);
					for (cp = cvt->chain; cp; cp = cp->next)
						sfprintf(pz->tmp, " => %s", cp->convert->to->name);
					(*disc->errorf)(pz, disc, 0, "%s: convert: %s", pz->path, sfstruse(pz->tmp));
				}
			}
		}
		if (state->flags & SHOW)
		{
			if (cvt->flags & CHECKSUM)
			{
				sfprintf(sfstdout, "checksum");
				if (cvt->flags & CONVERT)
					sfprintf(sfstdout, " + ");
			}
			if (cvt->flags & CONVERT)
			{
				sfprintf(sfstdout, "%s", cvt->chain->convert->from->name);
				for (cp = cvt->chain; cp; cp = cp->next)
					sfprintf(sfstdout, " => %s", cp->convert->to->name);
			}
			if (cvt->flags & (CHECKSUM|CONVERT))
				sfprintf(sfstdout, "\n");
			exit(0);
		}
		break;
	case PZ_REOPEN:
		cvt->checksum = 0;
		cvt->flags &= ~CHECKSUM_TAIL;
		break;
	case PZ_TAILREAD:
		if (cvt->flags & CHECKSUM)
		{
			sfstrbuf(state->tmp, data, size, 0);
			if (sfgetu(state->tmp) == CHECKSUM_OP)
			{
				cvt->flags |= CHECKSUM_TAIL;
				if ((k = sfgetu(state->tmp)) != cvt->checksum)
				{
					if (disc->errorf)
						(*disc->errorf)(pz, disc, 2, "%s: checksum mismatch -- expected 0x%08I*x != 0x%08I*x", pz->path, sizeof(k), k, sizeof(cvt->checksum), cvt->checksum);
					pz->flags |= PZ_ERROR;
					r = -1;
				}
				else
					r = 1;
			}
		}
		break;
	case PZ_TAILWRITE:
		if (cvt->flags & CHECKSUM)
		{
			cvt->flags |= CHECKSUM_TAIL;
			sp = (Sfio_t*)data;
			sfputu(state->tmp, CHECKSUM_OP);
			sfputu(state->tmp, cvt->checksum);
			n = sfstrtell(state->tmp);
			sfputu(sp, n);
			sfwrite(sp, sfstrseek(state->tmp, 0, SEEK_SET), n);
		}
		break;
	}
	return r;
}

/*
 * install the conversion/checksum table and event function
 */

int
pzdcconvert(Pz_t* pz, const Pzconvert_t* conversions)
{
	register State_t*	state;

	if (pz->disc->eventf != cvtevent && !(pz->flags & PZ_PUSHED))
	{
		if (!(state = vmnewof(pz->vm, 0, State_t, 1, 0)))
			return -1;
		if (!(state->tmp = sfstropen()))
		{
			vmfree(pz->vm, state);
			return -1;
		}
		state->conversions = (Pzconvert_t*)conversions;
		state->readf = pz->disc->readf;
		state->writef = pz->disc->writef;
		state->eventf = pz->disc->eventf;
		pz->discdata = (void*)state;
		pz->disc->eventf = cvtevent;
		optget(NiL, usage);
	}
	return 0;
}
