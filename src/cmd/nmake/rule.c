/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1984-2013 AT&T Intellectual Property          *
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
 * make rule support
 *
 * the attribute names defined in this file must agree with ATTRNAME
 */

#include "make.h"
#include "options.h"

#define ANON(name,flags)	(void)rinternal(name,P_attribute|(flags))
#define ASOC(field,name,flags)	internal.field=rassociate(name,flags)
#define ATTR(field,name,flags)	internal.field=rinternal(name,P_attribute|(flags))
#define FUNC(name,func)		((setvar(name,NiL,V_builtin|V_functional))->builtin=(func))
#define INIT(field,name,flags)	internal.field=rinternal(name,flags)

/*
 * return the NAME_* type for name
 */

int
nametype(const char* name, char** e)
{
	register const char*	s;
	register int		t;
	register int		q;

	t = 0;
	s = name;
	switch (*s)
	{
	case 0:
		return 0;
	case MARK_CONTEXT:
		q = NAME_context;
		break;
	case '.':
		q = NAME_variable|NAME_intvar;
		break;
	case '-':
	case '+':
		return NAME_option;
	case '(':
		t = NAME_staterule;
		q = 0;
		break;
	default:
		q = istype(*s, C_ID1) ? (NAME_identifier|NAME_variable) : istype(*s, C_VARIABLE1) ? NAME_variable : 0;
		break;
	}
	for (;;)
	{
		switch (*s++)
		{
		case 0:
			s -= 2;
			break;
		case '/':
			q |= NAME_path;
			q &= ~(NAME_identifier|NAME_variable);
			continue;
		case '=':
			if (q & (NAME_identifier|NAME_variable))
				q |= NAME_assignment;
			continue;
		case '+':
			if (*s != '=')
				q &= ~(NAME_identifier|NAME_variable);
			continue;
		case '&':
			if (*s == '=')
				continue;
			/*FALLTHROUGH*/
		case '*':
		case '?':
		case '[':
		case ']':
		case '|':
			q |= NAME_glob;
			q &= ~(NAME_identifier|NAME_variable);
			continue;
		case '@':
		case '!':
		case '}':
			if (*s == '(')
				t |= NAME_glob;
			q &= ~(NAME_identifier|NAME_variable);
			continue;
		case '$':
			if (*s == '(')
				t |= NAME_dynamic;
			q &= ~(NAME_identifier|NAME_variable);
			continue;
		case '(':
			if (s > name + 1)
			{
				t &= ~NAME_staterule;
				q &= ~NAME_staterule;
			}
			continue;
		case ')':
			if (t & NAME_staterule)
			{
				t &= ~NAME_staterule;
				q |= NAME_staterule;
			}
			else
			{
				q &= ~NAME_staterule;
				q |= t & (NAME_glob|NAME_dynamic);
			}
			q &= ~(NAME_identifier|NAME_variable);
			continue;
		default:
			if ((q & NAME_variable) && !istype(*(s - 1), C_VARIABLE1|C_VARIABLE2))
				q &= ~(NAME_identifier|NAME_variable);
			else if ((q & NAME_identifier) && !istype(*(s - 1), C_ID1|C_ID2))
				q &= ~NAME_identifier;
			continue;
		}
		break;
	}
	if ((q & NAME_context) && *s == MARK_CONTEXT)
	{
		if (e)
			*e = (char*)(s + 1);
		if (e)
			*e = (char*)(s + 1);
		return NAME_context;
	}
	if (q & NAME_staterule)
	{
		if (*s == ')')
			return NAME_statevar;
		if (*(name + 1) == '+')
			return NAME_altstate;
		return NAME_staterule;
	}
	if (q & NAME_dynamic)
		return NAME_dynamic;
	if (q & NAME_assignment)
		return NAME_assignment;
	if (q & NAME_glob)
		return NAME_glob;
	if (q & NAME_identifier)
		return NAME_identifier;
	if (q & NAME_variable)
		return (q & NAME_intvar) && *s == '.' ? NAME_intvar : NAME_variable;
	return q & NAME_path;
}

/*
 * map name s to rule r
 * previous mappings must be reconciled with the prereq lists
 */

char*
maprule(char* s, Rule_t* r)
{
	register List_t*	p;
	Rule_t*			q;
	Rule_t*			o;
	Hash_position_t*	pos;

	static unsigned char	warned;

	if ((o = getrule(s)) == r)
		return r->name;
	s = putrule(0, r);
	if (o && (pos = hashscan(table.rule, 0)))
	{
		if (!warned)
		{
			warned++;
			if (state.warn)
				error(1, "%d maprule() calls -- should not happen", UCHAR_MAX+1);
		}
		while (hashnext(pos))
		{
			q = (Rule_t*)pos->bucket->value;
			if (q == o)
				pos->bucket->value = (char*)r;
			for (p = q->prereqs; p; p = p->next)
				if (p->rule == o)
					p->rule = r;
		}
		hashdone(pos);
	}
	return s;
}

/*
 * return a pointer to a rule given its name,
 * creating the rule if necessary
 */

Rule_t*
makerule(register char* name)
{
	register Rule_t*	r;
	int			n;

	if (name)
	{
		if (r = getrule(name))
			return r;
		if (((n = nametype(name, NiL)) & NAME_path) && (table.rule->flags & HASH_ALLOCATE))
		{
			pathcanon(name, 0, 0);
			if (r = getrule(name))
				return r;
		}
	}
	newrule(r);
	r->name = putrule(0, r);
	if (state.compnew)
		(*state.compnew)(r->name, (char*)r, state.comparg);
	if (!name)
		n = nametype(r->name, NiL);
	if (n & (NAME_staterule|NAME_altstate))
	{
		r->dynamic |= D_compiled;
		r->property |= P_state|P_staterule;
	}
	else if (n & NAME_statevar)
	{
		r->dynamic |= D_compiled;
		r->property |= P_state|P_statevar;
	}
	else if (state.init || state.readonly)
		r->dynamic |= D_compiled;
	else
		r->dynamic &= ~D_compiled;
	r->status = NOTYET;
	r->preview = state.maxview + 1;
#if DEBUG
	message((-13, "adding %s %s", (r->property & P_state) ? "state variable" : "atom", r->name));
#endif
	return r;
}

/*
 * check if a rule name is special
 * special names are ignored in determining the default main target(s)
 */

int
special(register Rule_t* r)
{
	register char*	s;

	if (r->property & (P_functional|P_metarule|P_operator|P_repeat|P_state))
		return 1;
	if ((s = r->name) && !istype(*s, C_ID2))
		for (;;)
			switch (*s++)
			{
			case 0:
				return 1;
			case '/':
			case '$':
				return 0;
			}
	return 0;
}

/*
 * return joint list pointer for r
 * static single element list returned for non-joint target
 */

List_t*
joint(register Rule_t* r)
{
	static List_t		tmp;

	if ((r->property & (P_joint|P_target)) == (P_joint|P_target))
		return r->prereqs->rule->prereqs;
	tmp.rule = r;
	return &tmp;
}

/*
 * add a single non-duplicated prerequisite x to rule r
 * op = {PREREQ_APPEND,PREREQ_DELETE,PREREQ_INSERT,PREREQ_LENGTH}
 */

void
addprereq(register Rule_t* r, register Rule_t* x, int op)
{
	register List_t*	p;
	register List_t*	q;

	if (x != r)
	{
		if (p = r->prereqs)
		{
			q = 0;
			while (p)
			{
				if (x == p->rule)
				{
					if (op == PREREQ_DELETE)
					{
						if (q)
							q->next = p->next;
						else
							r->prereqs = p->next;
					}
					if (!(x->property & P_multiple))
						break;
				}
				if (!p->next)
				{
					if (op != PREREQ_DELETE)
					{
						if (r->property & P_state)
							state.savestate = 1;
						if (!state.init && !state.readonly)
							r->dynamic &= ~D_compiled;
						if (op == PREREQ_LENGTH)
						{
							register int	n;

							n = strlen(x->name);
							p = r->prereqs;
							q = 0;
							for (;;)
							{
								if (!p || strlen(p->rule->name) < n)
								{
									if (q)
										q->next = cons(x, p);
									else
										r->prereqs = cons(x, r->prereqs);
									break;
								}
								q = p;
								p = p->next;
							}
						}
						else if (op == PREREQ_INSERT)
							r->prereqs = cons(x, r->prereqs);
						else p->next = cons(x, NiL);
					}
					break;
				}
				q = p;
				p = p->next;
			}
		}
		else if (op != PREREQ_DELETE)
		{
			if (r->property & P_state)
				state.savestate = 1;
			if (!state.init && !state.readonly)
				r->dynamic &= ~D_compiled;
			r->prereqs = cons(x, NiL);
		}
	}
}

/*
 * return the pattern association rule with prefix a that matches r
 * if r is 0 then s is the rule name
 * if pos is not 0 then it is the last match position
 * *pos must be 0 on first call
 * *pos undefined when 0 returned
 */

Rule_t*
associate(register Rule_t* a, register Rule_t* r, register char* s, List_t** pos)
{
	register List_t*	p;
	register Rule_t*	x;
	register Rule_t*	z;
	List_t*			u;

	if (r)
	{
		if (r->property & (P_attribute|P_readonly))
			return 0;
		s = r->name;
	}
	do
	{
		u = 0;
		for (p = pos && *pos ? (*pos)->next : a->prereqs; p; p = p->next)
			if ((x = p->rule) != r)
			{
				if (x->property & P_attribute)
				{
					if (r && (hasattribute(r, x, NiL) || !r->scan && x->scan && (z = staterule(RULE, r, NiL, -1)) && z->scan == x->scan))
						break;
				}
				else if (x->name[0] == '%' && !x->name[1])
					u = p;
				else if (metamatch(NiL, s, x->name) || r && r->uname && !(r->property & P_state) && metamatch(NiL, r->uname, x->name))
					break;
			}
		if (p || (p = u))
		{
			if (pos)
				*pos = p;
			return (p->rule->property & P_attribute) ?
				catrule(a->name, "%", p->rule->name, 0) :
				catrule(a->name, p->rule->name, NiL, 0);
		}
	} while (r && s == r->name && r->uname && !(r->property & P_state) && (s = r->uname) != r->name);
	return 0;
}

/*
 * check if r's prerequisite list or named attributes have changed
 *
 * NOTE: IGNORECHANGE(r,q) prerequisites are ignored in the comparison
 */

#define IGNORECHANGE(r,q)	(((q)->property & (P_joint|P_ignore)) || ((q)->dynamic & D_alias) && getrule((q)->name) == r)

int
prereqchange(register Rule_t* r, register List_t* newprereqs, Rule_t* o, register List_t* oldprereqs)
{
	register List_t*	p;

	if ((r->property & P_accept) || state.accept)
		return 0;
	if ((r->property & (P_joint|P_target)) == (P_joint|P_target) && r != r->prereqs->rule->prereqs->rule)
		return 0;
	if ((r->attribute ^ o->attribute) & ~internal.accept->attribute)
	{
		reason((1, "%s named attributes changed", r->name));
		return 1;
	}
 more:
	for (;;)
	{
		if (newprereqs)
		{
			if (IGNORECHANGE(r, newprereqs->rule))
				newprereqs = newprereqs->next;
			else if (oldprereqs)
			{
				if (IGNORECHANGE(r, oldprereqs->rule))
					oldprereqs = oldprereqs->next;
				else if (newprereqs->rule == oldprereqs->rule || ((newprereqs->rule->dynamic ^ oldprereqs->rule->dynamic) & (D_alias|D_bound)) && getrule(newprereqs->rule) == getrule(oldprereqs->rule))
				{
					newprereqs = newprereqs->next;
					oldprereqs = oldprereqs->next;
				}
				else
					break;
			}
			else
				break;
		}
		else if (oldprereqs && IGNORECHANGE(r, oldprereqs->rule))
			oldprereqs = oldprereqs->next;
		else
			break;
	}
	if (newprereqs)
	{
		if ((r->dynamic & (D_entries|D_regular)) == D_entries || EXPLAIN)
		{
			for (p = oldprereqs; p && (newprereqs->rule != p->rule || !((newprereqs->rule->dynamic ^ p->rule->dynamic) & (D_alias|D_bound)) || getrule(newprereqs->rule) != getrule(p->rule)); p = p->next);
			if (p)
			{
				if ((r->dynamic & (D_entries|D_regular)) == D_entries)
					goto more;
				reason((1, "%s prerequisite %s re-ordered", r->name, newprereqs->rule->name));
			}
			else
				reason((1, "%s prerequisite %s added", r->name, newprereqs->rule->name));
		}
		return 1;
	}
	if (oldprereqs)
	{
		reason((1, "%s prerequisite %s deleted", r->name, oldprereqs->rule->name));
		return 1;
	}
	return 0;
}

/*
 * initialize immediate rule info
 */

static void
getimmediate(register Rule_t* r, List_t** prereqs, char** action)
{
	if (r->dynamic & D_dynamic)
		dynamic(r);
	if (*action = r->action)
		r->action = 0;
	if (*prereqs = r->prereqs)
		r->prereqs = 0;
}

/*
 * reset all rule info as if we just started
 */

static int
reset(const char* s, char* v, void* h)
{
	register Rule_t*	r = (Rule_t*)v;
	Stat_t			st;

	if (!(r->property & P_state))
	{
		if ((r->status == EXISTS || r->status == FAILED) && !(r->property & P_virtual))
		{
			r->status = NOTYET;
			if (!stat(r->name, &st))
				r->time = tmxgetmtime(&st);
			r->dynamic &= ~(D_entries|D_hasafter|D_hasbefore|D_hasmake|D_hasscope|D_hassemaphore|D_scanned|D_triggered);
		}
		else
			r->dynamic &= ~(D_entries|D_scanned);
	}
	return 0;
}

/*
 * check rule r for immediate action
 * should only be called if r->immediate==1 and r->target==1
 */

void
immediate(register Rule_t* r)
{
	register List_t*	p;
	register Rule_t*	x;
	List_t*			prereqs;
	char*			action;
	char*			e;
	Var_t*			v;
	int			i;
	int			g;
	int			u;
	Flags_t			a;
	Seconds_t		t;

	if (r == internal.retain || r == internal.state)
	{
		getimmediate(r, &prereqs, &action);
		a = r == internal.retain ? V_retain : V_scan;
		for (p = prereqs; p; p = p->next)
			if (v = varstate(p->rule, -1))
			{
				if (a == V_scan)
					setvar(v->name, v->value, V_scan);
				else
					v->property |= a;
			}
	}
	else if (r == internal.rebind || r == internal.accept)
	{
		getimmediate(r, &prereqs, &action);
		i = r == internal.accept;
		for (p = prereqs; p; p = p->next)
			rebind(p->rule, i);
	}
	else if (r == internal.unbind)
	{
		getimmediate(r, &prereqs, &action);
		for (p = prereqs; p; p = p->next)
			if ((p->rule->dynamic & (D_bound|D_scanned)) && (!(p->rule->mark & M_bind) || (state.questionable & 0x02000000)))
				p->rule->mark |= M_mark;
		hashwalk(table.rule, 0, unbind, r);
		hashwalk(table.rule, 0, unbind, NiL);
	}
	else if (r == internal.bind || r == internal.force)
	{
		getimmediate(r, &prereqs, &action);
		for (p = prereqs; p; p = p->next)
		{
			x = p->rule;
			message((-2, "bind(%s)", x->name));
			x = bind(x);
			if (r == internal.force)
			{
				if (x->time || !(x->property & P_dontcare))
					x->time = CURTIME;
				x->property |= P_force;
			}
		}
	}
	else if (r == internal.always || r == internal.local || r == internal.make || r == internal.run)
	{
		int		errors;
		Time_t		now;
		Time_t		tm = 0;

		getimmediate(r, &prereqs, &action);
		i = !prereqs;
		now = CURTIME;
		errors = 0;
		for (p = prereqs; p; p = p->next)
		{
			if ((p->rule->status == UPDATE || p->rule->status == MAKING) && !(p->rule->property & P_repeat))
				p->rule = internal.empty;
			else
			{
				errors += make(p->rule, &tm, NiL, 0);
				if (tm >= now)
					i = 1;
			}
		}
		if (r != internal.run)
		{
			if (prereqs)
				errors += complete(NiL, prereqs, &tm, 0);
			if (tm >= now)
				i = 1;
			if (action)
			{
				if (!errors && i)
				{
					r->status = UPDATE;
					trigger(r, NiL, action, 0);
					complete(r, NiL, NiL, 0);
				}
			}
			if (r == internal.make)
				r->property &= ~(P_always|P_local);
		}
		r->property &= ~(P_foreground|P_make|P_read);
	}
	else if (r == internal.include)
	{
		getimmediate(r, &prereqs, &action);
		i = COMP_INCLUDE;
		g = state.global;
		u = state.user;
		for (p = prereqs; p; p = p->next)
		{
			x = p->rule;
			if (streq(x->name, "-"))
				i ^= COMP_DONTCARE;
			else if (streq(x->name, "+"))
			{
				state.global = 1;
				state.user = 0;
			}
			else if (x->property & P_use)
				action = x->action;
			else
				readfile(x->name, i, action);
		}
		state.global = g;
		state.user = u;
	}
	else if (r == internal.alarm)
	{
		getimmediate(r, &prereqs, &action);
		if (p = prereqs)
		{
			t = strelapsed(p->rule->name, &e, 1);
			if (*e)
				t = 0;
			else
				p = p->next;
		}
		else
			t = 0;
		wakeup(t, p);
	}
	else if (r == internal.sync)
	{
		getimmediate(r, &prereqs, &action);
		if (!prereqs)
			savestate();
		else if (state.compile < COMPILED)
			error(2, "%s: cannot sync until make object compiled", prereqs->rule->name);
		else
			compile(prereqs->rule->name, prereqs->next ? prereqs->next->rule->name : (char*)0);
	}
	else if (r == internal.reset)
	{
		getimmediate(r, &prereqs, &action);
		hashwalk(table.rule, 0, reset, NiL);
	}
	else if (r == internal.wait)
	{
		getimmediate(r, &prereqs, &action);
		complete(NiL, prereqs, NiL, 0);
	}
	else if (r == internal.freeze)
	{
		getimmediate(r, &prereqs, &action);
		for (p = prereqs; p; p = p->next)
			if (v = getvar(p->rule->name))
				v->property |= V_frozen;
	}
	else if ((r->property & P_attribute) && !r->attribute)
		return;
	else if (!state.op && state.reading && state.compileonly)
		return;
	else
	{
		maketop(r, 0, NiL);
		getimmediate(r, &prereqs, &action);
	}
	if (prereqs)
		freelist(prereqs);
	if (r->prereqs)
	{
		freelist(r->prereqs);
		r->prereqs = 0;
	}
	r->action = 0;
	if (r = staterule(RULE, r, NiL, 0))
	{
		r->prereqs = 0;
		r->action = 0;
	}
}

/*
 * remove duplicate prerequisites from prerequisite list p
 */

void
remdup(register List_t* p)
{
	register List_t*	q;
	register List_t*	x;

	for (x = p, q = 0; p; p = p->next)
	{
		if (p->rule->mark & M_mark)
		{
			if (q)
				q->next = p->next;
#if DEBUG
			else
			{
				dumprule(sfstderr, p->rule);
				error(PANIC, "stray mark on %s", p->rule->name);
			}
#endif
		}
		else
		{
			if (!(p->rule->property & P_multiple))
				p->rule->mark |= M_mark;
			q = p;
		}
	}
	while (x)
	{
		x->rule->mark &= ~M_mark;
		x = x->next;
	}
}

/*
 * do dynamic expansion of rule r's prerequisites
 * the prerequisite list is permanently modified
 */

void
dynamic(register Rule_t* r)
{
	register char*		s;
	register List_t*	p;
	register List_t*	q;
	register List_t*	t;
	char**			v;
	char*			buf;
	int			added;
	int			flags;
	Rule_t*			x;
	Rule_t*			u;
	Frame_t*		oframe;
	Frame_t			frame;
	Sfio_t*			tmp;
	char*			vec[2];

	tmp = sfstropen();
	oframe = state.frame;
	if ((r->property & P_use) && !(state.frame = r->active))
	{
		zero(frame);
		frame.target = r;
		state.frame = frame.parent = &frame;
	}
	vec[1] = 0;
	added = 0;
	for (p = r->prereqs, q = 0; p; p = p->next)
	{
		if (isdynamic(p->rule->name))
		{
			if (q)
				q->next = p->next;
			else
				r->prereqs = p->next;
			u = p->rule;
			expand(tmp, u->name);
			buf = sfstruse(tmp);
			flags = 0;
			while (s = getarg(&buf, &flags))
			{
				added = 1;
				if (isglob(s))
					v = globv(NiL, s);
				else
					*(v = vec) = s;
				while (s = *v++)
				{
					x = makerule(s);
					if (x->dynamic & D_alias)
						x = makerule(x->name);
					if (flags & A_scope)
						x->dynamic |= D_scope;

					/*
					 * merge u into x
					 */

					merge(u, x, MERGE_ALL|MERGE_SCANNED);

					/*
					 * replace u with x
					 */

					t = cons(x, p->next);
					if (q)
						q->next = t;
					else
						r->prereqs = t;
					q = t;
				}
			}
		}
		else q = p;
	}
	if (added)
		remdup(r->prereqs);
	r->dynamic &= ~D_dynamic;
	state.frame = oframe;
	sfstrclose(tmp);
}

/*
 * return non-zero if r has builtin attribute a
 * if x!=0 then check if merge(x,r,MERGE_ATTR) would have attribute
 */

int
hasattribute(register Rule_t* r, register Rule_t* a, register Rule_t* x)
{
	register Flags_t	n;
	register int		attrname;
	List_t*			p;

	attrname = *a->name == ATTRNAME;
	if (a->property & P_attribute)
	{
		n = r->property;
		if (x && !(n & P_readonly))
		{
			n |= x->property & ~(P_attribute|P_immediate|P_implicit|P_internal|P_operator|P_readonly|P_state|P_staterule|P_statevar|P_target);
			if ((x->property & P_implicit) || (x->property & (P_metarule|P_terminal)) == P_terminal)
				n &= ~P_implicit;
		}

		/*
		 * the first group may conflict with a->attribute
		 */

		if (attrname)
		{
			if (a == internal.accept) return n & P_accept;
			if (a == internal.attribute) return n & P_attribute;
			if (a == internal.ignore) return n & P_ignore;
		}
		if (a != internal.retain)
		{
			if (a->attribute & r->attribute) return 1;
			if (x && (a->attribute & x->attribute & ~internal.ignore->attribute) && ((r->property & (P_attribute|P_use)) != P_attribute || r == internal.accept || r == internal.ignore || r == internal.retain)) return 1;
		}
		if (attrname)
		{
			/*
			 * the rest have no a->attribute conflicts
			 */

			if (a == internal.make) return n & P_make;
			if (a == internal.readonly) return n & P_readonly;
			if (a == internal.scan) return r->scan || x && x->scan;
			if (a == internal.semaphore) return r->semaphore;
			if (a == internal.target) return n & P_target;

			/*
			 * the rest have a corresponding P_*
			 */

			if (a->property & n & ~(P_accept|P_attribute|P_ignore|P_internal|P_metarule|P_readonly|P_staterule|P_statevar|P_target))
			{
				if (a == internal.after) return n & P_after;
				if (a == internal.always) return n & P_always;
				if (a == internal.archive) return n & P_archive;
				if (a == internal.before) return n & P_before;
				if (a == internal.command) return n & P_command;
				if (a == internal.dontcare) return n & P_dontcare;
				if (a == internal.force) return n & P_force;
				if (a == internal.foreground) return n & P_foreground;
				if (a == internal.functional) return n & P_functional;
				if (a == internal.immediate) return n & P_immediate;
				if (a == internal.implicit) return n & P_implicit;
				if (a == internal.joint) return n & P_joint;
				if (a == internal.local) return n & P_local;
				if (a == internal.multiple) return n & P_multiple;
				if (a == internal.op) return n & P_operator;
				if (a == internal.parameter) return n & P_parameter;
				if (a == internal.read) return n & P_read;
				if (a == internal.repeat) return n & P_repeat;
				if (a == internal.state) return n & P_state;
				if (a == internal.terminal) return n & P_terminal;
				if (a == internal.use) return n & P_use;
				if (a == internal.virt) return n & P_virtual;
			}
		}
		if (a->scan) return a->scan == r->scan || !r->scan && x && x->scan == a->scan;
	}
	else
	{
		if (attrname)
		{
			/*
			 * r->dynamic readonly attributes
			 */

			n = r->dynamic;
			if (a == internal.bound) return n & D_bound;
			if (a == internal.built) return n & D_built;
			if (a == internal.entries) return n & D_entries;
			if (a == internal.global) return n & D_global;
			if (a == internal.member) return n & (D_member|D_membertoo);
			if (a == internal.regular) return n & D_regular;
			if (a == internal.scanned) return n & D_scanned;
			if (a == internal.source) return n & D_source;
			if (a == internal.triggered) return n & D_triggered;
			if (a == internal.file) return (n & D_bound) && !(r->property & (P_state|P_virtual)) && r->time;

			/*
			 * r->property readonly attributes
			 */

			n = r->property;
			if (a == internal.internal) return n & P_internal;
			if (a == internal.metarule) return n & P_metarule;
			if (a == internal.staterule) return n & P_staterule;
			if (a == internal.statevar) return n & P_statevar;

			/*
			 * r->status readonly attributes
			 */

			n = r->status;
			if (a == internal.exists) return n == EXISTS;
			if (a == internal.failed) return n == FAILED;
			if (a == internal.making) return n == MAKING;
			if (a == internal.notyet) return n == NOTYET;

			/*
			 * other readonly attributes
			 */

			if (a == internal.active) return r->active != 0;
		}

		/*
		 * r prerequisites as pseudo attributes
		 */

		for (p = r->prereqs; p; p = p->next)
			if (a == p->rule)
				return 1;
	}
	return 0;
}

/*
 * return nonzero if r has an after prerequisite with exact property
 */

int
hasafter(register Rule_t* r, register Flags_t property)
{
	register List_t*	p;

	if (r->dynamic & D_hasafter)
		for (p = r->prereqs; p; p = p->next)
			if ((p->rule->property & P_failure) == property)
				return 1;
	return 0;
}

/*
 * merge <from> into <to> according to op
 */

void
merge(register Rule_t* from, register Rule_t* to, int op)
{
	register List_t*	p;

	if (from->name)
	{
		if (from == to || to->status != NOTYET && (to->status != UPDATE || !(from->property & P_use)))
		{
			/*
			 * this is a workaround to separate the view vs. local binding for this case:
			 *	from	sub/foo == foo
			 *	to	/dir/foo == foo
			 * it may need to be examined for cases other than (state.mam.statix)
			 */

			if (!state.exec && state.mam.statix && (from->dynamic & D_alias) && (to->property & P_terminal) && from->uname && to->uname && *from->name != '/' && *to->name == '/')
			{
				Rule_t*		fromstate;
				Rule_t*		tostate;

				to->uname = from->name;
				if (!(tostate = staterule(RULE, to, NiL, 0)) && (fromstate = staterule(RULE, from, NiL, 0)))
					*staterule(RULE, to, NiL, 1) = *fromstate;
			}
			return;
		}
#if DEBUG
		if (to->name)
			debug((-4, "merging %s%s into %s", (op & MERGE_ATTR) ? "attributes of " : null, from->name, to->name));
#endif
	}
	if (!(to->dynamic & D_bound) || (op & (MERGE_ASSOC|MERGE_FORCE)))
		to->property |= from->property & (P_accept|P_after|P_always|P_archive|P_before|P_command|P_dontcare|P_force|P_foreground|P_functional|P_ignore|P_implicit|P_joint|P_local|P_make|P_multiple|P_parameter|P_read|P_repeat|P_terminal|P_virtual);
	else
		to->property |= from->property & (P_accept|P_after|P_always|P_archive|P_before|P_command|P_force|P_foreground|P_functional|P_implicit|P_joint|P_local|P_make|P_multiple|P_parameter|P_read|P_repeat|P_terminal|P_virtual);
	if (from->property & P_implicit)
		to->property &= ~P_terminal;
	if ((from->property & (P_metarule|P_terminal)) == P_terminal)
		to->property &= ~P_implicit;
	if (op & MERGE_ALL)
	{
		if (!to->action)
			to->action = from->action;
		to->attribute |= from->attribute;
		to->property |= from->property & (P_accept|P_immediate|P_target|P_use);
		to->dynamic |= from->dynamic & (D_dynamic|D_global|D_regular);
		if (!(op & MERGE_SCANNED))
			to->dynamic |= from->dynamic & (D_entries|D_scanned);
		if (from->scan && from->scan != SCAN_NULL)
			to->scan = from->scan;
		if (to->status == NOTYET)
			to->status = from->status;
		for (p = from->prereqs; p; p = p->next)
			addprereq(to, p->rule, PREREQ_APPEND);
		if (!(to->property & P_state))
		{
			if (op & MERGE_BOUND)
			{
				from->mark |= M_bind;
				to->mark |= M_bind;
			}
			mergestate(from, to);
			if (op & MERGE_BOUND)
			{
				from->mark &= ~M_bind;
				to->mark &= ~M_bind;
			}
		}
	}
	else if (op & MERGE_FORCE)
	{
		if (from->attribute && from != internal.accept && from != internal.ignore && from != internal.retain && ((to->property & (P_attribute|P_use)) != P_attribute || to == internal.accept || to == internal.ignore || to == internal.retain))
			to->attribute |= from->attribute;
		if (from->scan)
			to->scan = from->scan;
	}
	else
	{
		if (from->attribute && from != internal.accept && from != internal.ignore && from != internal.retain && ((to->property & (P_attribute|P_use)) != P_attribute || to == internal.accept || to == internal.ignore || to == internal.retain))
			to->attribute |= from->attribute & ~internal.ignore->attribute;
		if (!to->scan)
			to->scan = from->scan;
	}
}

/*
 * merge <from> state rules into <to>
 */

void
mergestate(Rule_t* from, Rule_t* to)
{
	register int		i;
	register Rule_t*	fromstate;
	register Rule_t*	tostate;
	Rule_t*			t;
	char*			s;

	/*
	 * if RULE merges then RULE+1..STATERULES also merge
	 */

	if (fromstate = staterule(RULE, from, NiL, 0))
		tostate = staterule(RULE, to, NiL, 1);
	else if ((from->dynamic & D_alias) && (tostate = staterule(RULE, to, NiL, 0)))
		fromstate = staterule(RULE, from, NiL, 1);
	else
		return;
#if DEBUG
	if (state.test & 0x00000800)
	{
		error(2, "MERGESTATE from: %s: %s time=[%s] event=[%s]", from->name, fromstate->name, timestr(fromstate->time), timestr(fromstate->event));
		error(2, "MERGESTATE   to: %s: %s time=[%s] event=[%s]", to->name, tostate->name, timestr(tostate->time), timestr(tostate->event));
	}
#endif
	if ((from->dynamic & D_alias) && fromstate->time && !statetimeq(fromstate, tostate))
	{
		/*
		 * the solution is conservative but ok
		 * since aliases probably don't change
		 * very often -- every target depending
		 * on <to> will be forced out of date rather
		 * than those that just depend on <from>
		 */

		reason((1, "%s alias has changed to %s", unbound(from), unbound(to)));
		to->dynamic |= D_aliaschanged;
	}
	if (fromstate->event != tostate->event)
	{
		if (fromstate->event < tostate->event)
		{
			/*
			 * merge in the other direction
			 */

			t = from;
			from = to;
			to = t;
			t = fromstate;
			fromstate = tostate;
			tostate = t;
		}
		s = tostate->name;
		*tostate = *fromstate;
		tostate->prereqs = listcopy(fromstate->prereqs);
		tostate->name = s;
		for (i = RULE + 1; i <= STATERULES; i++)
			if ((fromstate = staterule(i, from, NiL, 0)) && !staterule(i, to, NiL, 0))
			{
				tostate = staterule(i, to, NiL, 1);
				s = tostate->name;
				*tostate = *fromstate;
				tostate->prereqs = listcopy(fromstate->prereqs);
				tostate->name = s;
			}
	}
}

/*
 * negate <from> attributes in <to>
 */

void
negate(register Rule_t* from, register Rule_t* to)
{
	to->attribute &= ~from->attribute;
	to->property &= ~(from->property & (P_accept|P_after|P_always|P_archive|P_before|P_command|P_dontcare|P_force|P_functional|P_ignore|P_immediate|P_implicit|P_local|P_make|P_multiple|P_parameter|P_repeat|P_target|P_terminal|P_use|P_virtual));
	to->dynamic &= ~(from->dynamic & (D_dynamic|D_entries|D_regular));
	if (from->scan)
		to->scan = 0;
	if (from->semaphore)
		to->semaphore = 0;
}

/*
 * make an internal rule pointer
 */

static Rule_t*
rinternal(char* s, register int flags)
{
	register Rule_t*	r;

	r = makerule(s);
	r->property |= flags;
	if (!r->prereqs && !r->action)
		r->dynamic |= D_compiled;
	return r;
}

/*
 * make an internal pattern association rule pointer
 * NOTE: this is required to sync pre 2001-05-09 make objects 
 */

static Rule_t*
rassociate(char* s, register int flags)
{
	register Rule_t*	r;
	register Rule_t*	a;
	register List_t*	p;

	r = rinternal(s, flags);
	for (p = r->prereqs; p; p = p->next)
		if (!(p->rule->property & P_attribute) && p->rule->name[0] == '%' && p->rule->name[1] == ATTRNAME && (a = getrule(p->rule->name + 1)) && (a->property & P_attribute))
			p->rule = a;
	return r;
}

/*
 * maintain atomic dir-rule names
 */

static int
diratom(const char* s, char* v, void* h)
{
	Dir_t*		d = (Dir_t*)v;

	NoP(s);
	NoP(h);
	d->name = getrule(d->name)->name;
	return 0;
}

/*
 * # outstanding jobs builtin
 */

static char*
b_outstanding(char** args)
{
	sfprintf(internal.val, "%d", state.coshell ? state.coshell->outstanding : 0);
	return sfstruse(internal.val);
}

/*
 * getconf() builtin
 */

static char*
b_getconf(char** args)
{
	char*	name;
	char*	path;
	char*	value;

	if (name = *args)
		args++;
	if (path = *args)
	{
		if (path[0] == '-' && !path[1])
			path = 0;
		args++;
	}
	if ((value = *args) && value[0] == '-' && !value[1])
		value = 0;
	return astconf(name, path, value);
}

/*
 * getopts() builtin
 */

static char*
b_getopts(char** args)
{
	char*	id;
	char*	usage;
	char*	prefix;
	char*	oid;
	char*	s;
	Rule_t*	r;
	Opt_t	info;

	s = getval(">", 0);
	if (*s == '-' && (id = *args++) && (r = getrule(*args++)) && (usage = r->action) && (prefix = *args))
	{
		if (streq(id, "-"))
			oid = 0;
		else
		{
			oid = error_info.id;
			error_info.id = id;
		}
		info = opt_info;
		opt_info.index = 0;
		opt_info.offset = 0;
		for (;;)
		{
			while (isspace(s[opt_info.offset]))
				opt_info.offset++;
			if (s[opt_info.offset] != '-')
				break;
			switch (optstr(s, usage))
			{
			case 0:
				break;
			case '?':
				error(ERROR_USAGE|0, "%s", opt_info.arg);
				opt_info.offset = 0;
				s = null;
				break;
			case ':':
				error(2, "%s", opt_info.arg);
				opt_info.offset = 0;
				s = null;
				break;
			default:
				sfprintf(internal.wrk, "%s%s", prefix, opt_info.name);
				setvar(sfstruse(internal.wrk), opt_info.arg && *opt_info.arg ? opt_info.arg : opt_info.num ? "1" : null, 0);
				continue;
			}
			break;
		}
		s += opt_info.offset;
		opt_info = info;
		if (oid)
			error_info.id = oid;
		return s;
	}
	return null;
}

typedef int (*Systab_f)(void);

typedef struct Systab_s
{
	const char*	name;
	Systab_f	call;
} Systab_t;

static const Systab_t	systab[] =
{
	"getegid",	(Systab_f)getegid,
	"geteuid",	(Systab_f)geteuid,
	"getgid",	(Systab_f)getgid,
	"getpid",	(Systab_f)getpid,
	"getppid",	(Systab_f)getppid,
	"getuid",	(Systab_f)getuid,
	{0}
};

/*
 * void arg system call catchall builtin
 */

static char*
b_syscall(char** args)
{
	Systab_t*	call;

	if (!*args)
		return null;
	sfprintf(internal.val, "%d", (call = (Systab_t*)strlook(systab, sizeof(systab[0]), *args)) ? (*call->call)() : -1);
	return sfstruse(internal.val);
}

/*
 * external engine name initialization -- the rest are in initrule()
 *
 * NOTE: version.c may reference some of these names, not to mention
 *	 non-engine source makefiles
 */

External_t	external =
{
	/*
	 * variable names
	 */

	"MAKEARGS",
	"MAKECONVERT",
	"MAKEFILE",
	"MAKEFILES",
	"MAKEIMPORT",
	"MAKELIB",
	"MAKE",
	CO_ENV_PROC,
	"OLDMAKE",
	"PWD",
	"MAKERULES",
	"MAKESKIP",
	"MAKEVERSION",
	"MAKEPATH",
	"VPATH",

	/*
	 * infrequently used engine interface names
	 */

	".COMPDONE",
	".COMPINIT",
	".DONE",
	".INIT",
	".INTERRUPT",
	".JOBDONE",
	".MAKEDONE",
	".MAKEINIT",
	".MAKEPROMPT",
	".MAKERUN",
	".MAMNAME.",
	".MAMACTION.",
	".ORDER",

	/*
	 * related file suffixes
	 */

	".ml",
	".mo",
	".mk",
	".ms",
	".mt",
};

/*
 * initialize some attribute and internal rule pointers
 */

void
initrule(void)
{
	Rule_t*		r;
	int		i;

	static int	repeat;

	hashclear(table.rule, HASH_ALLOCATE);

	/*
	 * dynamic rule attributes
	 */

	ATTR(accept,		".ACCEPT",	P_accept|P_immediate);
	ATTR(after,		".AFTER",	P_after);
	ATTR(always,		".ALWAYS",	P_always|P_immediate);
	ATTR(archive,		".ARCHIVE",	P_archive);
	ATTR(attribute,		".ATTRIBUTE",	P_readonly);
	ATTR(before,		".BEFORE",	P_before);
	ATTR(command,		".COMMAND",	P_command);
	ATTR(dontcare,		".DONTCARE",	P_dontcare);
	ATTR(force,		".FORCE",	P_force|P_immediate);
	ATTR(foreground,	".FOREGROUND",	P_foreground);
	ATTR(functional,	".FUNCTIONAL",	P_functional);
	ATTR(ignore,		".IGNORE",	P_ignore);
	ATTR(immediate,		".IMMEDIATE",	P_immediate);
	ATTR(implicit,		".IMPLICIT",	P_implicit);
	ATTR(joint,		".JOINT",	P_joint);
	ATTR(local,		".LOCAL",	P_local|P_immediate);
	ATTR(make,		".MAKE",	P_immediate);
	ATTR(multiple,		".MULTIPLE",	P_multiple);
	ATTR(op,		".OPERATOR",	P_operator);
	ATTR(parameter,		".PARAMETER",	P_parameter);
	ATTR(read,		".READ",	P_read);
	ATTR(readonly,		".READONLY",	P_readonly);
	ATTR(repeat,		".REPEAT",	P_repeat);
	ATTR(state,		".STATE",	P_state|P_immediate);
	ATTR(terminal,		".TERMINAL",	P_terminal);
	ATTR(use,		".USE",		P_use);
	ATTR(virt,		".VIRTUAL",	P_virtual);

	/*
	 * anonymous attributes (no internal handle)
	 */

	ANON(			".FAILURE",	P_failure);

	/*
	 * readonly rule attributes
	 */

	INIT(active,		".ACTIVE",	0);
	INIT(bound,		".BOUND",	0);
	INIT(built,		".BUILT",	0);
	INIT(entries,		".ENTRIES",	0);
	INIT(exists,		".EXISTS",	0);
	INIT(failed,		".FAILED",	0);
	INIT(file,		".FILE",	0);
	INIT(global,		".GLOBAL",	0);
	INIT(member,		".MEMBER",	0);
	INIT(notyet,		".NOTYET",	0);
	INIT(regular,		".REGULAR",	0);
	INIT(scanned,		".SCANNED",	0);
	INIT(staterule,		".STATERULE",	0);
	INIT(statevar,		".STATEVAR",	0);
	INIT(target,		".TARGET",	P_attribute|P_target);
	INIT(triggered,		".TRIGGERED",	0);

	/*
	 * special rules and names
	 */

	INIT(alarm,		".ALARM",	P_immediate);
	INIT(args,		".ARGS",	P_internal);
	INIT(bind,		".BIND",	P_immediate);
	INIT(clear,		".CLEAR",	P_attribute);
	INIT(copy,		".COPY",	P_attribute);
	INIT(delete,		".DELETE",	P_attribute);
	INIT(dot,		".",		0);
	INIT(empty,		"",		0);
	INIT(error,		".ERROR",	0);
	INIT(exports,		".EXPORT",	P_attribute|P_immediate);
	INIT(freeze,		".FREEZE",	P_immediate);
	INIT(globalfiles,	".GLOBALFILES",	P_internal|P_readonly);
	INIT(include,		".INCLUDE",	P_immediate);
	INIT(insert,		".INSERT",	P_attribute);
	INIT(internal,		".INTERNAL",	P_internal|P_readonly);
	INIT(main,		".MAIN",	0);
	INIT(makefiles,		".MAKEFILES",	P_internal|P_readonly);
	INIT(making,		".MAKING",	0);
	INIT(metarule,		".METARULE",	0);
	INIT(null,		".NULL",	P_attribute);
	INIT(preprocess,	".PREPROCESS",	P_internal|P_readonly);
	INIT(query,		".QUERY",	P_immediate|P_multiple|P_virtual);
	INIT(rebind,		".REBIND",	P_immediate);
	INIT(reset,		".RESET",	P_immediate);
	INIT(retain,		".RETAIN",	P_immediate);
	INIT(run,		".RUN",		P_immediate);
	INIT(scan,		".SCAN",	P_attribute|P_readonly);
	INIT(script,		".SCRIPT",	P_attribute|P_immediate);
	INIT(semaphore,		".SEMAPHORE",	P_attribute);
	INIT(serialize,		"-",		P_make|P_virtual|P_force|P_repeat|P_foreground|P_ignore|P_multiple);
	INIT(source,		".SOURCE",	0);
	INIT(special,		".SPECIAL",	P_attribute|P_internal|P_readonly);
	INIT(sync,		".SYNC",	P_immediate);
	INIT(tmplist,		".TMPLIST",	P_internal|P_readonly);
	INIT(unbind,		".UNBIND",	P_immediate);
	INIT(view,		".VIEW",	P_internal|P_readonly);
	INIT(wait,		".WAIT",	P_immediate);

	/*
	 * pattern association rules
	 */

	ASOC(append_p,		".APPEND.",	0);
	ASOC(assert_p,		".ASSERT.",	0);
	ASOC(assign_p,		".ASSIGN.",	0);
	ASOC(attribute_p,	".ATTRIBUTE.",	0);
	ASOC(bind_p,		".BIND.",	0);
	ASOC(dontcare_p,	".DONTCARE.",	0);
	ASOC(insert_p,		".INSERT.",	0);
	ASOC(require_p,		".REQUIRE.",	0);
	ASOC(source_p,		".SOURCE.",	0);

	/*
	 * builtin functions
	 */

	FUNC(			".GETCONF",	b_getconf);
	FUNC(			".GETOPTS",	b_getopts);
	FUNC(			".OUTSTANDING",	b_outstanding);
	FUNC(			".SYSCALL",	b_syscall);

#if DEBUG
	putrule(".DEBUG", internal.query);
#endif
	hashset(table.rule, HASH_ALLOCATE);

	/*
	 * initialize the builtin attributes
	 */

	if (!repeat)
	{
		static Frame_t		frame;

		frame.target = internal.internal;
		state.frame = frame.parent = &frame;
		internal.attribute->attribute = 1;
		internal.empty->dynamic |= D_bound;
		internal.empty->status = IGNORE;
		internal.empty->time = OLDTIME;
		internal.scan->scan = SCAN_USER;
		internal.serialize->action = null;
		addprereq(internal.source, internal.dot, PREREQ_APPEND);
		addprereq(internal.special, internal.attribute, PREREQ_APPEND);
		addprereq(internal.special, internal.scan, PREREQ_APPEND);
		sfprintf(internal.tmp, "%s?(.*)", internal.source->name);
		internal.issource = strdup(sfstruse(internal.tmp));
	}
	initscan(repeat);
	initwakeup(repeat);

	/*
	 * maintain atomic dir-rule names
	 */

	hashwalk(table.dir, 0, diratom, NiL);
#if !BINDINDEX
	for (i = 0; i <= state.maxview; i++)
	{
		r = getrule(state.view[i].path);
		r->view = i;
		state.view[i].path = r->name;
	}
#endif

	/*
	 * expand some dynamic values now for efficiency
	 */

	if (internal.metarule->dynamic & D_dynamic)
		dynamic(internal.metarule);

	/*
	 * some things are only done once
	 */

	repeat++;
}

/*
 * low level for initview()
 */

static List_t*
view(register char* s, register char* d, List_t* p)
{
	register int		i;
	register Rule_t*	r;

	if (!d)
		d = s;
	i = pathcanon(d, 0, 0) - d;
	if (i > 2 && d[i - 1] == '/')
		d[--i] = 0;
	r = makerule(d);
	if ((unique(r) || !r->time) && !streq(r->name, internal.dot->name) && !streq(r->name, internal.pwd))
	{
		if (state.maxview < MAXVIEW - 1)
		{
#if BINDINDEX
			r->dynamic |= D_bindindex;
			state.view[++state.maxview].path = r;
#else
			state.view[++state.maxview].path = r->name;
#endif
			state.view[state.maxview].pathlen = i;
			r->view = state.maxview;
			p = p->next = cons(r, NiL);
			if (s != d)
			{
				i = pathcanon(s, 0, 0) - s;
				if (i > 2 && s[i - 1] == '/')
					s[--i] = 0;
				r = makerule(s);
				r->view = state.maxview;
			}
			state.view[state.maxview].root = r->name;
			state.view[state.maxview].rootlen = i;
#if BINDINDEX
			message((-2, "view[%d]: %s %s", state.maxview, state.view[state.maxview].path->name, state.view[state.maxview].root));
#else
			message((-2, "view[%d]: %s %s", state.maxview, state.view[state.maxview].path, state.view[state.maxview].root));
#endif
		}
		else
			error(1, "view level %s ignored -- %d max", r->name, MAXVIEW);
	}
	return p;
}

/*
 * initialize the views
 */

void
initview(void)
{
	register char*	s;
	register char*	t;
	register int	n;
	int		c;
	int		pwdlen;
	char*		pwd;
	char*		tok;
	char*		u;
	List_t*		p;
	Rule_t*		r;
	Stat_t		top;
	Stat_t		bot;
	Sfio_t*		tmp;

	p = internal.view->prereqs = cons(internal.dot, NiL);
	tmp = sfstropen();
	if (fs3d(FS3D_TEST))
	{
		if ((n = (s = colonlist(tmp, external.viewnode, 1, ' ')) != 0) || (s = colonlist(tmp, external.viewdot, 1, ' ')))
		{
			tok = tokopen(s, 0);
			if (s = n ? tokread(tok) : ".")
				while (t = tokread(tok))
				{
					message((-2, "vpath %s %s", s, t));
					mount(t, s, FS3D_VIEW, NiL);
					s = t;
				}
			tokclose(tok);
		}
		for (n = 1; n <= MAXVIEW; n++)
			sfputr(tmp, "/...", -1);
		sfputc(tmp, 0);
		s = t = sfstrseek(tmp, 0, SEEK_CUR);
		while (!stat(t -= 4, &top))
		{
			if (state.maxview >= MAXVIEW - 1)
			{
				error(1, "view levels past %s ignored -- %d max", t += 4, MAXVIEW);
				break;
			}
			p = p->next = cons(r = makerule(t), NiL);
#if BINDINDEX
			state.view[++state.maxview].path = r;
#else
			state.view[++state.maxview].path = r->name;
#endif
			state.view[state.maxview].pathlen = s - t;
			message((-2, "view[%d]: %s", state.maxview, r->name));
		}
		state.fsview = 1;
		setvar(external.viewnode, NiL, 0);
		if (!stat(".", &top) && !stat("...", &bot) && top.st_ino == bot.st_ino && top.st_dev == bot.st_dev)
			state.virtualdot = 1;
	}
	else
	{
		unique(internal.dot);
		if (s = colonlist(tmp, external.viewnode, 1, ' '))
		{
			tok = tokopen(s, 1);
			while (s = tokread(tok))
			{
				if (*s != '/')
					sfprintf(internal.tmp, "%s/", internal.pwd);
				sfputr(internal.tmp, s, -1);
				t = sfstruse(internal.tmp);
				s = pathcanon(t, 0, 0);
				if (*(s - 1) == '/')
					*--s = 0;
				n = s - t;
				pwd = 0;
				if (strncmp(internal.pwd, t, n) || (c = internal.pwd[n]) && c != '/')
				{
					/*
					 * ksh pwd and ast getcwd() are logical
					 * others are physical
					 * this gets PWD and VPATH in sync
					 * if they're not
					 */

					if (!stat(t, &top))
					{
						sfputr(internal.nam, internal.pwd, -1);
						u = sfstruse(internal.nam);
						c = 0;
						for (;;)
						{
							if (!stat(u, &bot) && bot.st_ino == top.st_ino && bot.st_dev == top.st_dev)
							{
								sfprintf(internal.nam, "%s%s", t, internal.pwd + (s - u));
								pwd = strdup(sfstruse(internal.nam));
								pwdlen = strlen(pwd);
								break;
							}
							if (!(s = strrchr(u, '/')))
								break;
							*s = 0;
							c = 1;
						}
					}
					if (!pwd)
					{
						setvar(external.viewnode, NiL, 0);
						break;
					}
				}
				if (pwd)
				{
					internal.pwd = pwd;
					internal.pwdlen = pwdlen;
					setvar(external.pwd, internal.pwd, V_import);
				}
				state.view[0].root = makerule(t)->name;
				state.view[0].rootlen = n;
#if BINDINDEX
				message((-2, "view[%d]: %s %s", state.maxview, state.view[state.maxview].path->name, state.view[state.maxview].root));
#else
				message((-2, "view[%d]: %s %s", state.maxview, state.view[state.maxview].path, state.view[state.maxview].root));
#endif
				while (s = tokread(tok))
				{
					if (!c)
						p = view(s, NiL, p);
					else
					{
						if (*s != '/')
							sfprintf(internal.tmp, "%s/", internal.pwd);
						sfprintf(internal.tmp, "%s%s", s, internal.pwd + n);
						p = view(s, sfstruse(internal.tmp), p);
					}
				}
				if (!(optflag(OPT_strictview)->flags & OPT_READONLY))
					state.strictview = 1;
				break;
			}
			tokclose(tok);
		}
		if (s = colonlist(tmp, external.viewdot, 1, ' '))
		{
			n = state.maxview;
			tok = tokopen(s, 1);
			while (s = tokread(tok))
			{
				pathcanon(s, 0, 0);
				if (!n || *s != '.' || *(s + 1) != '.' || *(s + 2) && *(s + 2) != '/')
					p = view(s, NiL, p);
				else for (c = 0; c <= n; c++)
				{
#if BINDINDEX
					sfprintf(internal.tmp, "%s/%s", c ? state.view[c].path->name : internal.pwd, s);
#else
					sfprintf(internal.tmp, "%s/%s", c ? state.view[c].path : internal.pwd, s);
#endif
					p = view(sfstruse(internal.tmp), NiL, p);
				}
			}
			tokclose(tok);
		}
	}
	if (!state.maxview)
		state.believe = 0;
	sfstrclose(tmp);
}
