/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2012 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Research
 *
 * make metarule routines
 */

#include "make.h"

/*
 * return a pointer to the metarule that builds
 * a file that matches the metarule pattern out from
 * a file that matches the metarule pattern in
 * force causes the rule to be created
 */

Rule_t*
metarule(char* in, char* out, int force)
{
	register Rule_t*	r;
	register char*		s;

	sfprintf(internal.met, "%s>%s", in, out ? out : "%");
	s = sfstruse(internal.met);
	r = getrule(s);
	if (force)
	{
		if (!r)
			r = makerule(NiL);
		r->property |= (P_metarule|P_use);
		r->property &= ~(P_state|P_staterule|P_statevar);
	}
	else
	{
		/*
		 * if in -> out is a secondary metrule then return primary
		 */

		if (!r && (r = metainfo('P', in, out, 0)))
		{
			if (r->prereqs)
			{
				sfprintf(internal.met, "%s>%s", in, r->prereqs->rule->name);
				r = getrule(sfstruse(internal.met));
			}
			else
				r = 0;
		}
		if (r && (!(r->property & P_metarule) || !r->action && !r->uname))
			r = 0;
	}
	return r;
}

/*
 * metarule supplementary info
 *
 * type
 *
 *	'I'	inputs
 *	'N'	nonterminal unconstrained inputs
 *	'O'	outputs
 *	'P'	primary output
 *	'S'	primary secondary outputs
 *	'T'	terminal unconstrained inputs
 *	'X'	unconstrained output exclusions
 */

Rule_t*
metainfo(int type, char* s1, char* s2, int force)
{
	register Rule_t*	r;

	sfprintf(internal.met, "%s.%c.%s%s%s", internal.metarule->name, type, s1 ? s1 : null, s2 ? ">" : null, s2 ? s2 : null);
	s1 = sfstruse(internal.met);
	if (!(r = getrule(s1)) && force)
	{
		r = makerule(NiL);
		r->property |= P_readonly;
	}
	return r;
}

#if _WINIX

/*
 * yes, metarules are affected by the case insensitive filesystem botch
 * we do case insensitive metarule pattern matching for all patterns that
 * do not match `[-+]*'
 */

static int
metaccmp(register char* p, register int a, register int b)
{
	return a == b || *p != '-' && *p != '+' && (isupper(a) ? tolower(a) : a) == (isupper(b) ? tolower(b) : b);
}

#else

#define metaccmp(p,a,b)	((a)==(b))

#endif

/*
 * match s against metarule pattern
 * leading unmatched directories before % are ignored
 * returns 0 if no match
 * otherwise if stem!=0 then non-dir part of matched stem is copied there
 */

int
metamatch(char* stem, register char* s, char* pattern)
{
	register char*	p;
	register char*	t;
	register char*	x;
	register char*	y;
	char*		b;
	register int	targetprefix;
	
	b = s;
	p = pattern;
	targetprefix = state.targetprefix ? *state.targetprefix : -1;
	while (*p != '%')
	{
		if (!*s)
			return 0;
		else if (!metaccmp(pattern, *p, *s))
		{
			do
			{
				if (*s == '/')
				{
					while (*++s == '/');
					break;
				}
				else if (*s == targetprefix)
				{
					x = state.targetprefix;
					y = s;
					while (*++x == *++y && *x);
					if (!*x)
					{
						s = y;
						break;
					}
				}
			} while (*++s);
			if (!*(b = s))
				return 0;
			p = pattern;
		}
		else if (!*p++)
		{
			if (p = stem)
			{
				s = b;
				while (*p++ = *s++);
			}
			return 1;
		}
		else
			s++;
	}
	t = s;
	while (*s)
		s++;
	while (*p)
		p++;
	while (*--p != '%')
		if (s <= t || !metaccmp(pattern, *p, *--s))
			return 0;
	if (stem)
	{
		if ((p = strrchr(t, '/')) && p < s)
			t = p + 1;
		if (state.targetprefix)
		{
			b = t;
			while ((p = strchr(b, targetprefix)) && p < s)
			{
				x = state.targetprefix;
				y = p;
				while (*++x == *++y && *x);
				if (!*x)
					b = t = y;
				else
					b++;
			}
		}
		p = stem;
		while (t < s)
			*p++ = *t++;
		*p = 0;
	}
	return 1;
}

/*
 * expand the metarule pattern p into sp using stem
 */

void
metaexpand(Sfio_t* sp, char* stem, char* p)
{
	register int	c;

	while ((c = *p++) != '%')
	{
		if (!c)
			return;
		sfputc(sp, c);
	}
	sfputr(sp, stem, -1);
	sfputr(sp, p, -1);
}

/*
 * update the metarule closure graph
 *
 * NOTE: c==0 (PREREQ_APPEND alias) is used to cut off part of the closure recursion
 */

void
metaclose(Rule_t* in, Rule_t* out, int c)
{
	char*		s;
	Rule_t*		x;
	Rule_t*		y;
	Rule_t*		z;
	List_t*		p;
	List_t*		q;
	char		stem[MAXNAME];

#if DEBUG
	static int	eloop = 0;
#endif

#if DEBUG
	if (eloop++ > 64)
	{
		eloop--;
		error(1, "metaclose: %s -> %s recursion!", in->name, out->name);
		return;
	}
#endif
	addprereq(internal.metarule, out, c == PREREQ_DELETE ? c : PREREQ_LENGTH);
	addprereq(metainfo('I', out->name, NiL, 1), in, c);
	addprereq(metainfo('O', in->name, NiL, 1), out, c);
	if (!c && !(state.questionable & 0x00000100))
		return;
	for (q = internal.metarule->prereqs; q; q = q->next)
	{
		if (metamatch(stem, out->name, q->rule->name) && (x = metainfo('O', q->rule->name, NiL, 0)))
		{
			for (p = x->prereqs; p; p = p->next)
			{
				metaexpand(internal.met, stem, p->rule->name);
				z = makerule(sfstruse(internal.met));
				x = metarule(in->name, z->name, 1);
				if (!x->action || (state.questionable & 0x00000100))
				{
					if (c == PREREQ_DELETE)
						s = 0;
					else
					{
						if (z != in && !(x->mark & M_metarule))
						{
							x->mark |= M_metarule;
							metaclose(in, z, PREREQ_APPEND);
							x->mark &= ~M_metarule;
						}
						s = ((y = metarule(q->rule->name, z->name, 0)) && y->uname) ? y->uname : q->rule->name;
					}
					x->uname = s;
					x->dynamic &= ~D_compiled;
				}
			}
		}
		if (c != PREREQ_DELETE && metamatch(stem, q->rule->name, in->name) && !streq(stem, "%") && (x = metainfo('I', q->rule->name, NiL, 0)))
		{
			metaexpand(internal.met, stem, out->name);
			if (!streq(stem, q->rule->name))
			{
				z = makerule(sfstruse(internal.met));
				for (p = x->prereqs; p; p = p->next)
				{
					x = metarule(p->rule->name, z->name, 1);
					if (!(x->mark & M_metarule))
					{
						x->mark |= M_metarule;
						metaclose(p->rule, z, 0);
						x->mark &= ~M_metarule;
					}
					x->uname = in->name;
					x->dynamic &= ~D_compiled;
				}
			}
		}
	}
	if (c != PREREQ_DELETE && (z = metainfo('I', in->name, NiL, 0)))
		for (q = z->prereqs; q; q = q->next)
			if ((z = q->rule) != out)
			{
				x = metarule(z->name, out->name, 1);
				if (!x->action || (state.questionable & 0x00000100))
				{
					if (!(x->mark & M_metarule))
					{
						x->mark |= M_metarule;
						metaclose(z, out, 0);
						x->mark &= ~M_metarule;
					}
					if (!x->uname)
					{
						x->uname = in->name;
						x->dynamic &= ~D_compiled;
					}
				}
			}
#if DEBUG
	eloop--;
#endif
}

/*
 * return the primary metarule source that generates r
 * *meta is the matching metarule
 */

Rule_t*
metaget(Rule_t* r, Frame_t* active, char* stem, Rule_t** meta)
{
	register List_t*	p;
	register List_t*	q;
	register List_t*	v;
	Rule_t*			m;
	Rule_t*			s;
	Rule_t*			x;
	Rule_t*			y;
	List_t*			prereqs;
	List_t*			terminal;
	char*			b;
	char*			u;
	char*			t;
	Flags_t			f;
	int			matched;

	u = unbound(r);
	prereqs = 0;
	if (active)
	{
		f = active->flags & P_implicit;
		do
		{
			if (active->prereqs)
			{
				prereqs = active->prereqs;
				break;
			}
			else if (active == active->parent)
				break;
			else
				active = active->parent;
		} while (active);
	}
	else
		f = 0;
	if (prereqs)
	{
		/*
		 * check explicit constrained prereqs -- easy
		 */

		debug((-9, "check explicit constrained metarule prerequisites"));
		t = sfstrbase(internal.met);
		matched = 0;
		for (p = internal.metarule->prereqs; p; p = p->next)
			if (metamatch(stem, u, p->rule->name) && (x = metainfo('I', p->rule->name, NiL, 0)) && (matched = 1, x->prereqs))
				for (q = prereqs; q; q = q->next)
					if (q->rule->status != UPDATE)
						for (v = x->prereqs; v; v = v->next)
							if (metamatch(t, unbound(q->rule), v->rule->name) && streq(t, stem) && (m = metarule(v->rule->name, p->rule->name, 0)) && (!(r->property & P_terminal) || (m->property & P_terminal)))
							{
								s = q->rule;
								goto primary;
							}
	}
	else
		matched = 1;
	if (b = strrchr(u, '/'))
		b++;
	else
		b = u;
	terminal = (x = metainfo('T', NiL, NiL, 0)) ? x->prereqs : 0;
	if (matched)
	{
		Sfio_t*	buf = 0;
		Sfio_t*	tmp = 0;

		/*
		 * check implicit constrained prereqs -- a little harder
		 */

		debug((-9, "check implicit constrained metarule prerequisites"));
		matched = 0;
		for (p = internal.metarule->prereqs; p; p = p->next)
			if (metamatch(stem, u, p->rule->name) && (x = metainfo('I', p->rule->name, NiL, 0)))
				for (matched = 1, q = x->prereqs; q; q = q->next)
					if ((m = metarule(q->rule->name, p->rule->name, 0)) && (!(r->property & P_terminal) || (m->property & P_terminal)) && !(m->property & f))
					{
						if (!tmp)
							tmp = sfstropen();
						if (b != u && *u != '/' && *q->rule->name && ((state.questionable & 0x00000008) || !strchr(q->rule->name, '/')))
						{
							char*	x = u;

							for (;;)
							{
								sfprintf(tmp, "%-.*s", b - x, x);
								metaexpand(tmp, stem, q->rule->name);
								t = sfstruse(tmp);
								if (m->dynamic & D_dynamic)
								{
									if (!buf)
										buf = sfstropen();
									expand(buf, t);
									t = sfstruse(buf);
								}
								if ((s = bindfile(NiL, t, 0)) && s->status != UPDATE && (s->time || (s->property & P_target)) || !(state.questionable & 0x08000000) && m->action && !*m->action && (s = makerule(t)))
								{
									sfstrclose(tmp);
									if (buf)
										sfstrclose(buf);
									goto primary;
								}
								if (!(x = strchr(x, '/')))
									break;
								x++;
							}
						}
						else
						{
							metaexpand(tmp, stem, q->rule->name);
							t = sfstruse(tmp);
							if (m->dynamic & D_dynamic)
							{
								if (!buf)
									buf = sfstropen();
								expand(buf, t);
								t = sfstruse(buf);
							}
							if (isstatevar(t) && (s = getrule(t)) && (s = bindstate(s, NiL)))
								goto primary;
							if ((s = bindfile(NiL, t, 0)) && s->status != UPDATE && (s->time || (s->property & P_target)) || !(state.questionable & 0x08000000) && m->action && !*m->action && (s = makerule(t)))
							{
								sfstrclose(tmp);
								if (buf)
									sfstrclose(buf);
								goto primary;
							}
						}

						/*
						 * check terminal unconstrained metarules
						 */

						debug((-9, "check terminal unconstrained metarules"));
						for (v = terminal; v; v = v->next)
							if (metarule(v->rule->name, NiL, 0))
							{
								if (!buf)
									buf = sfstropen();
								metaexpand(buf, t, v->rule->name);
								if ((s = bindfile(NiL, sfstruse(buf), 0)) && s->status != UPDATE && s->time)
								{
									s->status = EXISTS;
									s = makerule(t);
									sfstrclose(buf);
									sfstrclose(tmp);
									goto primary;
								}
							}
					}
		if (buf)
			sfstrclose(buf);
		if (tmp)
			sfstrclose(tmp);
	}

	/*
	 * unconstrained metarules ignored on recursive calls
	 */

	if (!meta)
		return 0;

	/*
	 * check terminal unconstrained metarules
	 */

	if (prereqs)
	{
		/*
		 * check explicit terminal unconstrained prereqs
		 */

		t = sfstrbase(internal.met);
		for (p = terminal; p; p = p->next)
			if (m = metarule(p->rule->name, NiL, 0))
				for (q = prereqs; q; q = q->next)
					if (metamatch(t, q->rule->name, p->rule->name) && streq(t, b) && (s = bindfile(q->rule, NiL, 0)) && s->time)
					{
						s->status = EXISTS;
						goto unconstrained;
					}
	}

	/*
	 * check implicit terminal unconstrained prereqs
	 */

	for (p = terminal; p; p = p->next)
		if ((m = metarule(p->rule->name, NiL, 0)) && (!(r->property & P_terminal) || (m->property & P_terminal)) && !(m->property & f))
		{
			metaexpand(internal.met, b, p->rule->name);
			if ((s = bindfile(NiL, sfstruse(internal.met), 0)) && s->time)
			{
				s->status = EXISTS;
				goto unconstrained;
			}
		}

	/*
	 * check nonterminal unconstrained metarules
	 * nonterminals skipped if r matched any constrained metarule target
	 */

	if (!matched && !(r->property & P_terminal) && (x = metainfo('N', NiL, NiL, 0)) && x->prereqs)
	{
		/*
		 * don't try to match excluded patterns
		 */

		if ((s = metainfo('X', NiL, NiL, 0)))
			for (p = s->prereqs; p; p = p->next)
				if (metamatch(NiL, u, p->rule->name))
				{
					matched = 1;
					break;
				}
		if (!matched)
		{
			debug((-9, "check nonterminal unconstrained metarules"));
			if (prereqs)
			{
				/*
				 * check explicit prereqs -- easy
				 */

				debug((-9, "check explicit nonterminal unconstrained metarule prerequisites"));
				t = sfstrbase(internal.met);
				for (p = x->prereqs; p; p = p->next)
					if (m = metarule(p->rule->name, NiL, 0))
						for (q = prereqs; q; q = q->next)
							if (metamatch(t, q->rule->name, p->rule->name) && streq(t, b))
							{
								s = q->rule;
								goto unconstrained;
							}
			}

			/*
			 * check implicit prereqs -- hardest
			 */

			debug((-9, "check implicit nonterminal unconstrained metarule prerequisites"));
			for (q = x->prereqs; q; q = q->next)
				if (m = metarule(q->rule->name, NiL, 0))
				{
					metaexpand(internal.met, b, q->rule->name);
					t = sfstruse(internal.met);
					s = getrule(t);
					if (isstatevar(t))
					{
				 		if (s && (s = bindstate(s, NiL)))
							goto unconstrained;
					}
					else if (s = bindfile(s, t, BIND_RULE))
					{
						/*
						 * use s if it exists
						 */

						if (s->status != UPDATE && (s->time || (s->property & P_target)))
							goto unconstrained;

						/*
						 * use s if it can be generated
						 */

						if (metaget(s, NiL, stem, NiL))
							goto unconstrained;
					}
				}
		}
	}

	/*
	 * no metarule match
	 */

	return 0;

 unconstrained:

	/*
	 * unconstrained match
	 */

	strcpy(stem, b);

 primary:

	/*
	 * primary match
	 */

	if (meta)
	{
		if (m->uname)
		{
			/*
			 * metarule intermediate prerequisite
			 */

			if (!(x = metarule(m->uname, p->rule->name, 0)) || x->uname)
			{
				x = 0;
				if (y = metainfo('O', m->uname, NiL, 0))
					for (q = y->prereqs; q; q = q->next)
						if (metamatch(stem, u, q->rule->name) && (x = metarule(m->uname, q->rule->name, 0)))
						{
							if (!x->uname)
								break;
							x = 0;
						}
			}
			if (x)
			{
				metaexpand(internal.met, stem, m->uname);
				if (!((y = makerule(sfstruse(internal.met)))->property & P_terminal) && !(y->property & f))
				{
					*meta = x;
					s = y;
					goto found;
				}
			}
		}
		*meta = m;
	}
	if (m->property & P_terminal)
		s->property |= P_terminal;
	else if ((x = metainfo('S', m->name, NiL, 0)) && !(x->property & P_terminal))
		for (p = x->prereqs; p; p = p->next)
		{
			metaexpand(internal.met, stem, p->rule->name);
			if ((x = getrule(sfstruse(internal.met))) && (x->property & P_terminal))
				return 0;
		}
 found:
	if (meta && (state.targetcontext || state.targetprefix) && (t = strrchr(u = unbound(r), '/')))
	{
		*t = 0;
		if (!strchr(s->name, '/'))
		{
			sfprintf(internal.met, "%s%s%s", u, state.targetprefix ? state.targetprefix : "/", s->name);
			s = makerule(sfstruse(internal.met));
		}
		if (!strchr(stem, '/'))
		{
			sfputr(internal.met, stem, 0);
			sfsprintf(stem, MAXNAME - 1, "%s%s%s", u, state.targetprefix ? state.targetprefix : "/", sfstruse(internal.met));
		}
		*t = '/';
	}
	return s;
}
