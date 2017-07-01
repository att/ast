/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
 * keep track of --list dups
 */

#include "jcllib.h"

typedef struct Uniq_s
{
	Dtlink_t	link;
	char*		name;
	char*		value;
	size_t		size;
	unsigned long	count;
	unsigned long	flags;
	int		recfm;
} Uniq_t;

typedef struct State_s
{
	Dt_t*		diff;
	Dt_t*		mark;
	Dt_t*		uniq;
	Dtdisc_t	diffdisc;
	Dtdisc_t	markdisc;
	Dtdisc_t	uniqdisc;
} State_t;

static State_t		state;

/*
 * return 1 if path is marked
 * if dd!=0 then set dd->recfm and dd->lrecl if marked
 */

int
marked(const char* path, Jcldd_t* dd, Jcldisc_t* disc)
{
	char*	s;
	char*	e;
	size_t	n;
	int	f;

	if (path && (s = strrchr(path, '%')) && s > (char*)path && *(s - 1) != '%')
	{
		f = 0;
		for (;;)
		{
			switch (*++s)
			{
			case 0:
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if ((n = strtoul(s, &e, 10)) && (!*e || *e == '.' && !strchr(e, '/')))
				{
					if (dd)
					{
						dd->recfm = f;
						dd->lrecl = n;
					}
					return 1;
				}
				break;
			case 'A':
			case 'a':
				f |= JCL_RECFM_A;
				continue;
			case 'B':
			case 'b':
				f |= JCL_RECFM_B;
				continue;
			case 'D':
			case 'd':
				f |= JCL_RECFM_D;
				continue;
			case 'F':
			case 'f':
				f |= JCL_RECFM_F;
				continue;
			case 'M':
			case 'm':
				f |= JCL_RECFM_M;
				continue;
			case 'S':
			case 's':
				f |= JCL_RECFM_S;
				continue;
			case 'U':
			case 'u':
				f |= JCL_RECFM_U;
				continue;
			case 'V':
			case 'v':
				f |= JCL_RECFM_V;
				continue;
			default:
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 1, "%s: %c: unknown RECFM", path, *s);
				continue;
			}
			break;
		}
	}
	return 0;
}

/*
 * if size>0 then name is marked with %size
 * and the marked name is returned
 * marked names are global
 *
 * if size==0 then marked name, if any, is returned
 * otherwise 0 returned and name is not marked
 * recfm is JCL_RECFM_F or JCL_RECFM_V
 */

char*
mark(const char* name, int recfm, size_t size, Jcldisc_t* disc)
{
	register Uniq_t*	u;
	register char*		s;
	register size_t		n;
	int			m;

	if (marked(name, NiL, disc))
		return (char*)name;
	if (!state.mark)
	{
		if (!size)
			return 0;
		state.markdisc.link = offsetof(Uniq_t, link);
		state.markdisc.key = offsetof(Uniq_t, name);
		state.markdisc.size = -1;
		if (!(state.mark = dtopen(&state.markdisc, Dtoset)))
		{
			nospace(NiL, disc);
			return 0;
		}
	}
	if (u = (Uniq_t*)dtmatch(state.mark, name))
	{
		if (disc->errorf)
		{
			if (size != u->size && size && u->size)
				(*disc->errorf)(NiL, disc, 2, "%s: inconsistent size %I*u", u->value, sizeof(size), size);
			if (recfm != u->recfm && recfm && u->recfm)
				(*disc->errorf)(NiL, disc, 2, "%s: inconsistent recfm %s", u->value, (recfm & JCL_RECFM_V) ? "v" : "f");
		}
		return u->value;
	}
	if (!size)
		return 0;
	m = strlen(name);
	n = 2 * (m + 1) + 8;
	if (!(u = newof(NiL, Uniq_t, 1, n)))
	{
		nospace(NiL, disc);
		return 0;
	}
	s = u->value = stpcpy(u->name = (char*)(u + 1), name) + 1;
	u->size = size;
	m -= suflen(name);
	sfsprintf(s, n, "%-.*s%%%s%I*u%s", m, name, (recfm & JCL_RECFM_V) ? "v" : "", sizeof(size), size, name + m);
	message((-7, "mark      %s => %s", name, u->value));
	dtinsert(state.mark, u);
	return s;
}

static int
uniqcmp(Dt_t* dt, register void* a, register void* b, Dtdisc_t* disc)
{
	int	n;

	if (!(n = strcmp(((Uniq_t*)a)->name, ((Uniq_t*)b)->name)))
	{
		if (((Uniq_t*)a)->value)
		{
			if (((Uniq_t*)b)->value)
				n = strcmp(((Uniq_t*)a)->value, ((Uniq_t*)b)->value);
			else
				n = 1;
		}
		else if (((Uniq_t*)b)->value)
			n = -1;
	}
	return n;
}

/*
 * add name [value] to the uniq dict
 */

void
uniq(const char* name, const char* value, unsigned long flags, Jcldisc_t* disc)
{
	register Uniq_t*	u;
	register char*		s;
	register size_t		n;
	Uniq_t			k;

	if (!state.uniq)
	{
		state.uniqdisc.link = offsetof(Uniq_t, link);
		state.uniqdisc.comparf = uniqcmp;
		if (!(state.uniq = dtopen(&state.uniqdisc, Dtoset)))
		{
			nospace(NiL, disc);
			return;
		}
	}
	k.name = (char*)name;
	k.value = (char*)value;
	if (u = (Uniq_t*)dtsearch(state.uniq, &k))
		u->count++;
	else
	{
		n = strlen(name) + 1;
		if (value)
			n += strlen(value) + 1;
		if (!(u = newof(NiL, Uniq_t, 1, n)))
		{
			nospace(NiL, disc);
			return;
		}
		u->count = 1;
		s = stpcpy(u->name = (char*)(u + 1), name);
		if (value)
			strcpy(u->value = s + 1, value);
		dtinsert(state.uniq, u);
	}
	u->flags |= flags;
}

/*
 * add name=value to the diff dict
 * return 1 if value is different from previous
 */

int
diff(const char* name, const char* value, Jcldisc_t* disc)
{
	register Uniq_t*	u;
	register size_t		n;

	if (!state.diff)
	{
		state.diffdisc.link = offsetof(Uniq_t, link);
		state.diffdisc.key = offsetof(Uniq_t, name);
		state.diffdisc.size = -1;
		if (!(state.diff = dtopen(&state.diffdisc, Dtoset)))
		{
			nospace(NiL, disc);
			return -1;
		}
	}
	if (u = (Uniq_t*)dtmatch(state.diff, name))
	{
		u->count++;
		if (streq(value, u->value))
			return 0;
	}
	else
	{
		if (!(u = newof(NiL, Uniq_t, 1, strlen(name) + 1)))
		{
			nospace(NiL, disc);
			return -1;
		}
		u->count = 1;
		strcpy(u->name = (char*)(u + 1), name);
		dtinsert(state.diff, u);
	}
	n = strlen(value);
	if (n >= u->size)
	{
		u->size = roundof(n + 1, 16);
		if (!(u->value = newof(u->value, char, u->size, 0)))
		{
			nospace(NiL, disc);
			return -1;
		}
	}
	strcpy(u->value, value);
	return 1;
}

typedef struct Label_s
{
	unsigned int	flag;
	int		label;
} Label_t;

static const Label_t	label[] =
{
	{ JCL_LISTAUTOEDITS,	'A' },
	{ JCL_LISTEXEC,		'E' },
	{ JCL_LISTINPUTS,	'I' },
	{ JCL_LISTJOBS,		'J' },
	{ JCL_LISTOUTPUTS,	'O' },
	{ JCL_LISTPROGRAMS,	'P' },
	{ JCL_LISTSCRIPTS,	'S' },
	{ JCL_LISTVARIABLES,	'V' },
};

static void
stats(Sfio_t* sp, Uniq_t* u, int c, int h)
{
	register int		i;

	if (h)
	{
		for (i = 0; i < elementsof(label); i++)
			if (u->flags & label[i].flag)
				sfputc(sp, label[i].label);
		sfputc(sp, ' ');
	}
	sfprintf(sp, "%s", u->name);
	if (u->value)
		sfprintf(sp, " %s", u->value);
	if (c)
		sfprintf(sp, " %lu", u->count);
	sfputc(sp, '\n');
}

/*
 * list uniq name [value] with optional count
 */

int
jclstats(Sfio_t* sp, unsigned long flags, Jcldisc_t* disc)
{
	register Uniq_t*	u;
	register int		c;
	register int		h;
	register unsigned long	m;

	if (state.uniq)
	{
		c = (flags & JCL_LISTCOUNTS) != 0;
		m = (flags & JCL_LIST);
		h = m & (m - 1);
		if (h && (m & (JCL_LISTJOBS|JCL_LISTSCRIPTS)))
		{
			for (u = (Uniq_t*)dtfirst(state.uniq); u; u = (Uniq_t*)dtnext(state.uniq, u))
				if (u->flags & JCL_LISTJOBS)
				{
					u->flags &= ~JCL_LISTSCRIPTS;
					stats(sp, u, c, h);
				}
			m &= ~JCL_LISTJOBS;
		}
		for (u = (Uniq_t*)dtfirst(state.uniq); u; u = (Uniq_t*)dtnext(state.uniq, u))
			if (!m || (u->flags & m))
				stats(sp, u, c, h);
	}
	if (sfsync(sp))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "write error");
		return -1;
	}
	return 0;
}
