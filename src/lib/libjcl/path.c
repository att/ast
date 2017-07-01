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
 * jcl path map
 */

#include "jcllib.h"

#include <tok.h>

typedef struct Map_s
{
	Dtlink_t	link;
	char*		prefix;
	char*		map;
	char*		tail;
	char*		suffix;
	int		length;
	int		same;
} Map_t;

typedef struct Match_s
{
	Dt_t*		map;
	Map_t*		all;
	const char*	string;
	int		length;
} Match_t;

typedef struct Suf_s
{
	Dtlink_t	link;
	int		length;
	char		name[1];
} Suf_t;

typedef struct State_s
{
	Dtdisc_t	mapdisc;
	Dtdisc_t	sufdisc;
	Dt_t*		suf;
	Match_t		match[7];
	int		mapped;
	int		matched;
	int		matches;
} State_t;

static State_t		state;

static const char usage[] =
"[-?\n@(#)$Id: libjcl (AT&T Research) 2013-06-27 $\n]"
"[i:import]"
"[I:include]:[directory]"
"[k:marklength]"
"[p:parameterize]"
"[s:subdirectory]"
"[w:warn]"
;

static int
optset(Jcl_t* jcl, int c, Jcldisc_t* disc)
{
	unsigned long	f;

	switch (c)
	{
	case 'i':
		f = JCL_IMPORT;
		break;
	case 'I':
		jclinclude(NiL, opt_info.arg, JCL_JOB|JCL_PROC, disc);
		return 1;
	case 'k':
		f = JCL_MARKLENGTH;
		break;
	case 'p':
		f = JCL_PARAMETERIZE;
		break;
	case 's':
		f = JCL_SUBDIR;
		break;
	case 'w':
		f = JCL_WARN;
		break;
	case ':':
		error(2, "%s", opt_info.arg);
		return 0;
	case '?':
		error(ERROR_USAGE|4, "%s", opt_info.arg);
		return 0;
	}
	if ((jcl->roflags & (JCL_MAPPED|f)) != (JCL_MAPPED|f))
	{
		if (!(jcl->roflags & JCL_MAPPED))
			jcl->roflags |= f;
		if (opt_info.number)
			jcl->flags |= f;
		else
			jcl->flags &= ~f;
	}
	return 1;
}

#define delimiter(c)	((c)=='.'||(c)=='/')

/*
 * return a pointer to the next delimiter
 * 0 if no more delimiters
 */

static const char*
nextdelim(register const char* s)
{
	register int	c;

	while (c = *s++)
		if (delimiter(c))
			return s - 1;
	return 0;
}

/*
 * return a pointer to the last delimiter
 * 0 if no more delimiters
 */

static const char*
lastdelim(register const char* s)
{
	register int		c;
	register const char*	r;

	r = 0;
	while (c = *s++)
		if (delimiter(c) && (!r || *r == '.' || c == '/'))
			r = s - 1;
	return r;
}

/*
 * return the pointer to the matched subexpression n
 * string length returned in *z
 */

char*
matched(int n, size_t* z, Jcldisc_t* disc)
{
	if (n > state.matched)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "${%d}: not defined for this pattern", n);
		return 0;
	}
	*z = state.match[n].length;
	return (char*)state.match[n].string;
}

/*
 * match tail pattern t to s
 * set sets state.matched
 */

static int
tail(register const char* t, register const char* s, int set)
{
	register const char*	u;
	const char*		b;

	b = s;
	for (;;)
	{
		if (*t == '*')
		{
			if (!*++t)
			{
				if (set)
				{
					state.matched++;
					return (s - b) + (state.match[state.matched].length = strlen(state.match[state.matched].string = s));
				}
				return 1;
			}
			else
			{
				t++;
				if (!(u = nextdelim(s)))
					break;
				if (set)
				{
					state.matched++;
					state.match[state.matched].length = u - s;
					state.match[state.matched].string = s;
				}
				s = u + 1;
			}
		}
		else if (*t != *s++)
			break;
		else if (!*t++)
			return s - b - 1;
	}
	return 0;
}

/*
 * longest matching prefix match function
 * we don't expect many map entries
 */

static Map_t*
match(const char* name)
{
	register Map_t*		mp;
	register Map_t*		lp;
	register int		i;
	register int		j;
	register int		n;
	int			all;
	const char*		s;
	Map_t			map;
	char			buf[2];

	buf[1] = 0;
	map.prefix = buf;
	n = 0;
	all = -1;
	state.match[0].string = name;
	state.matched = 0;
	for (;;)
	{
		lp = 0;
		if (state.match[state.matched].map)
		{
			i = 0;
			buf[0] = name[0];
			if (!(mp = (Map_t*)dtprev(state.match[state.matched].map, &map)))
				mp = (Map_t*)dtfirst(state.match[state.matched].map);
			for (; mp && mp->same >= i; mp = (Map_t*)dtnext(state.match[state.matched].map, mp))
			{
				for (j = i; name[j] == mp->prefix[j]; j++)
					if (!name[j])
					{
						if (mp->tail)
							break;
						mp->length = j + n;
						message((-9, "match[%d]   %s %s %d", state.matched, name, mp->prefix, mp->length));
						return mp;
					}
				if (!mp->prefix[j] && (!mp->tail || tail(mp->tail, &name[j], 0)))
				{
					i = j;
					lp = mp;
					lp->length = j + n;
					message((-9, "maybe[%d]   %s %s %d", state.matched, name, mp->prefix, mp->length));
				}
			}
		}
		if (lp)
		{
			if (lp->tail)
				lp->length += tail(lp->tail, &name[i], 1);
			return lp;
		}
		if (state.match[state.matched].all)
			all = state.matched;
		if (state.matched >= state.matches)
			break;
		state.matched++;
		if (!(s = nextdelim(name)))
		{
			if (state.match[state.matched].all)
				all = state.matched;
			break;
		}
		state.match[state.matched].string = name;
		n += (state.match[state.matched].length = s - name) + 1;
		name = s + 1;
	}
	if (all >= 0)
	{
		state.match[0].length = strlen(state.match[0].string);
		mp = state.match[all].all;
		switch (all)
		{
		case 0:
			all++;
			state.match[1].string = state.match[0].string;
			state.match[1].length = state.match[0].length;
			mp->length = 0;
			break;
		case 1:
			state.match[1].string = state.match[0].string;
			mp->length = state.match[1].length = state.match[0].length;
			break;
		default:
			state.match[all].length = strlen(state.match[all].string = state.match[all-1].string + state.match[all-1].length + 1);
			mp->length = n + strlen(name);
			break;
		}
		state.matched = all;
		return mp;
	}
	return 0;
}

/*
 * add s to the mapped suffix dictionary
 */

static int
suffix(Sfio_t* sp, const char* s, Jcldisc_t* disc)
{
	Suf_t*	xp;
	int	n;

	if (!state.suf)
	{
		state.sufdisc.link = offsetof(Suf_t, link);
		state.sufdisc.key = offsetof(Suf_t, name);
		state.sufdisc.size = 0;
		if (!(state.suf = dtopen(&state.sufdisc, Dtoset)))
		{
			sfclose(sp);
			nospace(NiL, disc);
			return -1;
		}
	}
	if (!dtmatch(state.suf, s))
	{
		n = strlen(s);
		if (!(xp = newof(0, Suf_t, 1, n)))
		{
			sfclose(sp);
			nospace(NiL, disc);
			return -1;
		}
		strcpy(xp->name, s);
		xp->length = n;
		dtinsert(state.suf, xp);
	}
	return 0;
}

/*
 * add maps in file to dataset map
 */

int
jclmap(Jcl_t* jcl, const char* file, Jcldisc_t* disc)
{
	register Sfio_t*	sp;
	register Map_t*		mp;
	register Map_t*		pp;
	register char*		s;
	register char*		t;
	char*			op;
	char*			arg[32];
	char*			nv[32];
	char*			ofile;
	char*			tail;
	long			oline;
	int			c;
	int			n;
	int			k;
	int			v;
	int			dontcare;
	Opt_t			opt;
	char			buf[PATH_MAX];
	char			tmp[PATH_MAX];

	jcl->flags |= JCL_MAPPED;
	if (!file || !*file)
	{
		file = JCL_MAPFILE;
		dontcare = 1;
	}
	else if (streq(file, "-"))
		return 0;
	else
		dontcare = 0;
	if (!(sp = sfopen(NiL, file, "r")))
	{
		sp = 0;
		if (!strchr(file, '/'))
		{
			sfsprintf(tmp, sizeof(tmp), "lib/jcl/%s", file);
			if (!pathpath(tmp, "", PATH_REGULAR, buf, sizeof(buf)))
			{
				if (dontcare)
					return 0;
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%s: cannot find map file", file);
				return -1;
			}
			sp = sfopen(NiL, file = (char*)buf, "r");
		}
		if (!sp)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: cannot read map file", file);
			return -1;
		}
	}
	ofile = error_info.file;
	error_info.file = (char*)file;
	oline = error_info.line;
	error_info.line = 0;
	while (s = sfgetr(sp, '\n', 1))
	{
		error_info.line++;
		if (*s != '#' && tokscan(s, NiL, " %s %v ", &op, arg, elementsof(arg)) >= 1)
		{
			if (streq(op, "export"))
			{
				n = 0;
				while (s = arg[n++])
					if (!jclsym(jcl, s, NiL, JCL_SYM_EXPORT) && disc->errorf)
						(*disc->errorf)(NiL, disc, 1, "%s: invalid assignment", s);
			}
			else if (streq(op, "map"))
			{
				if (!(s = arg[0]) || !arg[1])
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, 2, "%s: prefix map [suffix] expected", op);
					continue;
				}
				k = 0;
				while (*s == '*' && delimiter(*(s + 1)))
				{
					s += 2;
					k++;
				}
				if (*s == '*')
				{
					if (*(s + 1))
					{
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, 2, "%s: %s: only \"*.\" at beginning, \".*.\" in middle, or \".*\" at end supported", op, s);
						sfclose(sp);
						return -1;
					}
					s++;
					k++;
				}
				if (k >= elementsof(state.match))
				{
					if (disc->errorf)
						(*disc->errorf)(NiL, disc, 2, "%s: %s: only %d *. prefixes supported", op, arg[0], elementsof(state.match) - 1);
					sfclose(sp);
					return -1;
				}
				if (state.matches < k)
					state.matches = k;
				tail = 0;
				for (t = s; *t; t++)
					if (*t == '*')
					{
						if (!delimiter(*(t - 1)) && --t || *(t + 1) && !delimiter(*(t + 1)))
						{
							if (disc->errorf)
								(*disc->errorf)(NiL, disc, 2, "%s: %s: only \"*[./]\" at beginning, \"[./]*[./]\" in middle, or \"[./]*\" at end supported", op, s);
							sfclose(sp);
							return -1;
						}
						if (!tail)
							tail = t;
					}
				if (n = t - s)
				{
					if (!state.match[k].map)
					{
						state.mapped |= (1<<k);
						state.mapdisc.link = offsetof(Map_t, link);
						state.mapdisc.key = offsetof(Map_t, prefix);
						state.mapdisc.size = -1;
						if (!(state.match[k].map = dtopen(&state.mapdisc, Dtoset)))
						{
							nospace(NiL, disc);
							return -1;
						}
					}
					if (tail)
						*tail = 0;
					c = dtmatch(state.match[k].map, s) != 0;
					if (tail)
						*tail = '*';
					if (c)
					{
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, 2, "%s: duplicate map prefix", arg[0]);
						continue;
					}
				}
				else
				{
					if (!*(s = arg[0]))
						s = "*";
					if (state.match[k].all)
					{
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, 2, "%s: duplicate map prefix", s);
						continue;
					}
				}
				if (!(mp = newof(0, Map_t, 1, n + strlen(arg[1]) + (arg[2] ? strlen(arg[2]) : 0) + 8)))
				{
					sfclose(sp);
					nospace(NiL, disc);
					return -1;
				}
				if (tail)
					n = tail - s;
				mp->length = n;
				memcpy(mp->prefix = (char*)(mp + 1), s, n);
				s = mp->prefix + n;
				*s++ = 0;
				s = stpcpy(mp->map = s, arg[1]);
				if (tail)
					s = stpcpy(mp->tail = s + 1, tail);
				if (arg[2])
					strcpy(mp->suffix = s + 1, arg[2]);
				else
					mp->suffix = s;
				if (mp->length)
					dtinsert(state.match[k].map, mp);
				else
					state.match[k].all = mp;
				if ((s = (char*)nextdelim(mp->suffix)) && suffix(sp, s, disc))
					return -1;
			}
			else if (streq(op, "set"))
			{
				const char*	use;
				Jcloptset_f	set;

				if (!(use = disc->usage) || !(set = disc->optsetf))
				{
					use = usage;
					set = optset;
				}
				jcl->roflags |= JCL_MAPPED;
				opt = opt_info;
				n = 0;
				while (s = arg[n++])
				{
					s = expand(jcl, s, JCL_SYM_EXPORT|JCL_SYM_SET);
					if (*s == '-' || *s == '+')
						while (c = optstr(s, use))
							(*set)(jcl, c, disc);
					else if (tokscan(s, NiL, " %v ", nv, elementsof(nv)) >= 1)
						for (v = 0; s = nv[v]; v++)
							if (!jclsym(jcl, s, NiL, JCL_SYM_SET) && disc->errorf)
								(*disc->errorf)(NiL, disc, 1, "%s: invalid assignment", s);
				}
				opt_info = opt;
				jcl->roflags &= ~JCL_MAPPED;
			}
			else if (streq(op, "suf") || streq(op, "suffix"))
			{
				n = 0;
				while (s = arg[n++])
					if (!delimiter(*s))
					{
						if (disc->errorf)
							(*disc->errorf)(NiL, disc, 1, "%s: invalid suffix", s);
					}
					else if (suffix(sp, s, disc))
						return -1;
			}
			else if (disc->errorf)
				(*disc->errorf)(NiL, disc, 1, "%s: unknown op", op);
		}
	}
	sfclose(sp);
	error_info.file = ofile;
	error_info.line = oline;
	for (k = 0; k <= state.matches; k++)
	{
		if (state.match[k].map)
			for (pp = 0, mp = (Map_t*)dtfirst(state.match[k].map); mp; pp = mp, mp = (Map_t*)dtnext(state.match[k].map, mp))
			{
				mp->same = 0;
				if (pp)
					while (mp->prefix[mp->same] == pp->prefix[mp->same])
						mp->same++;
				message((-9, "map[%d]   %2d %2d %s%s%s", k, mp->length, mp->same, mp->prefix, mp->tail ? " " : "", mp->tail ? mp->tail : ""));
			}
		if (mp = state.match[k].all)
			message((-9, "map[%d]   %2d %2d %s", k, mp->length, mp->same, mp->prefix));
	}
	return 0;
}

/*
 * return the length of the mapped suffix in path
 * 0 returned if path has no mapped suffix
 */

int
suflen(const char* path)
{
	Suf_t*		xp;
	const char*	s;

	return (state.suf && (s = strrchr(path, '.')) && (xp = (Suf_t*)dtmatch(state.suf, s))) ? xp->length : 0;
}

/*
 * dataset library(member) => library/member
 * converted name returned in jcl->vp
 * jcl->tp may be clobbered
 */

char*
jclpath(Jcl_t* jcl, const char* name)
{
	register char*	s;
	register char*	t;
	register Map_t*	m;
	char*		e;
	const char*	oname = name;
	int		n;
	Sfio_t*		pp;

	if (s = mark(name, 0, 0, jcl->disc))
		name = (const char*)s;
	if (*name != '/' && (*name != '.' || *(name + 1) != '/') && state.mapped && (m = match(name)))
	{
		name += m->length;
		if (*m->suffix && *name && (n = suflen(name)))
		{
			n = strlen(name) - n;
			sfprintf(jcl->vp, "%s%-.*s%s%s", m->map, n, name, m->suffix, name + n);
		}
		else if (*m->suffix && *m->map && (n = suflen(m->map)))
		{
			n = strlen(m->map) - n;
			sfprintf(jcl->vp, "%-.*s%s%s%s", n, m->map, name, m->suffix, m->map + n);
		}
		else
			sfprintf(jcl->vp, "%s%s%s", m->map, name, m->suffix);
		if (!(name = sfstruse(jcl->vp)))
			nospace(jcl, NiL);
		name = (const char*)expand(jcl, name, JCL_SYM_SET);
		message((-7, "match     %s => %s", oname, name));
	}
	else
		m = 0;
	if ((s = strrchr(name, '(')) && (t = strrchr(s, ')')) && !*(t + 1))
	{
		if (name != (const char*)sfstrbase(jcl->vp))
		{
			sfprintf(jcl->vp, "%s", name);
			if (!(name = (const char*)sfstruse(jcl->vp)))
				nospace(jcl, NiL);
			s = strrchr(name, '(');
		}
		strtol(s + 1, &e, 0);
		if (*e == ')' && !*(e + 1))
		{
			jcl->flags |= JCL_GDG;
			*s++ = '/';
			if (*(s + 1) == '0' && (s + 2) == e || *(s + 1) == '+' && *(s + 2) == '0' && (s + 3) == e)
			{
				*s++ = '0';
				*s = 0;
			}
			else
			{
				*e = 0;
				sfprintf(jcl->tp, "$(gdginstance %s)", name);
				if (!(s = sfstruse(jcl->tp)))
					nospace(jcl, NiL);
				if (jcl->flags & JCL_EXEC)
				{
					if (!(pp = sfpopen(NiL, s, "r")) || !(t = sfgetr(pp, '\n', 0)) || sfprintf(jcl->vp, "%s", t) || sfclose(pp))
					{
						if (jcl->disc->errorf)
							(*jcl->disc->errorf)(NiL, jcl->disc, 1, "%s: cannot map generation data group", s);
					}
					else if (!(name = (const char*)sfstruse(jcl->vp)))
						nospace(jcl, NiL);
				}
				else
				{
					sfprintf(jcl->vp, "%s", s);
					if (!(name = (const char*)sfstruse(jcl->vp)))
						nospace(jcl, NiL);
				}
			}
		}
		else
		{
			*s = '/';
			*strrchr(s, ')') = 0;
		}
	}
	if (s = mark(name, 0, 0, jcl->disc))
		name = (const char*)s;
	if (name != oname)
		message((-7, "map       %s => %s", oname, name));
	return (char*)name;
}
