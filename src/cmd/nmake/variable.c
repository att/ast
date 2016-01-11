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
 * make variable routines
 */

#include "make.h"
#include "options.h"

#define BINDING(r,f)	(((f)&VAL_UNBOUND)?unbound(r):state.localview?localview(r):(r)->name)

/*
 * generator for genprereqs()
 * sp!=0 is the first pass that sets M_generate
 * sp==0 is the second pass that clears M_generate
 */

static int
scanprereqs(register Sfio_t* sp, Rule_t* r, int dostate, int all, int top, int sep, int op)
{
	register int		i;
	register List_t*	p;
	register Rule_t*	x;
	Rule_t*			y;
	Rule_t*			z;
	List_t*			prereqs[4];
	List_t			t;

	i = 0;
	if (r->scan == SCAN_IGNORE && !(state.questionable & 0x02000000))
		top = -1;
	else if ((x = staterule(PREREQS, r, NiL, 0)) && (x->property & P_implicit))
		prereqs[i++] = x->prereqs;
	else
		top = 1;
	if (top)
	{
		if ((x = staterule(RULE, r, NiL, 0)) && x->prereqs != r->prereqs)
			prereqs[i++] = x->prereqs;
		prereqs[i++] = r->prereqs;
	}
	if (r->active && r->active->primary)
	{
		t.rule = makerule(r->active->primary);
		t.next = 0;
		prereqs[i++] = &t;
	}
	while (--i >= 0)
		for (p = prereqs[i]; p; p = p->next)
		{
			x = p->rule;
			do
			{
				if (!(x->dynamic & D_alias))
					z = 0;
				else if (!(z = getrule(x->name)))
					break;
				if (x->mark & M_generate)
				{
					if (!sp)
					{
						x->mark &= ~M_generate;
						if (top >= 0)
							scanprereqs(sp, x, dostate, all, 0, sep, op);
					}
				}
				else if (sp && (all || ((x->property & P_state) || x->scan || (y = staterule(PREREQS, x, NiL, 0)) && y->scan || !r->scan) && !(x->property & (P_use|P_virtual)) && (!(x->property & P_ignore) || (x->property & P_parameter)) && (!(x->property & P_dontcare) || x->time)))
				{
					x->mark |= M_generate;
					if (all || ((x->property & P_state) != 0) == dostate)
					{
						if (sep)
							sfputc(sp, ' ');
						else
							sep = 1;
						sfputr(sp, (op & VAL_UNBOUND) ? unbound(x) : state.localview ? localview(x) : ((x->dynamic & D_alias) ? x->uname : x->name), -1);
					}
					if (top >= 0)
						sep = scanprereqs(sp, x, dostate, all, 0, sep, op);
				}
			} while (x = z);
		}
	return sep;
}

/*
 * generate the list of source|explicit prerequisites in sp
 * if sep!=0 then ' ' separation needed
 * new value of sep is returned
 */

static int
genprereqs(Sfio_t* sp, Rule_t* r, int dostate, int all, int sep, int op)
{
	state.val++;
	sep = scanprereqs(sp, r, dostate, all, 1, sep, op);
	scanprereqs(NiL, r, dostate, all, 1, sep, op);
	state.val--;
	return sep;
}

/*
 * return the value of a variable given its name
 * the internal automatic variables are (lazily) expanded here
 * op: VAL_PRIMARY|VAL_AUXILIARY|VAL_UNBOUND
 */

char*
getval(register char* s, int op)
{
	register List_t*	p;
	register Rule_t*	r;
	Rule_t*			x;
	Rule_t*			z;
	Var_t*			v;
	Var_t*			a;
	char*			t;
	char*			o;
	char*			val;
	char*			next;
	int			c;
	int			n;
	int			var;
	int			tokens;
	int			pop;
	int			sep;
	Time_t			e;
	char**			ap;
	char*			arg[16];

	if (!*s)
		return null;
	else if (isstatevar(s))
	{
		if (!(r = getrule(s)))
			return null;
		if ((r->property & (P_functional|P_virtual)) && r->status != UPDATE)
			maketop(r, 0, NiL);
		return r->statedata ? r->statedata : null;
	}
	else if (!istype(*s, C_VARIABLE1|C_ID1|C_ID2))
	{
		sep = 0;

		/*
		 * some internal vars have no associated rule
		 */

		switch (var = *s)
		{

#if __OBSOLETE__ < 20100101
		case '+':	/* 20051122 restore for backwards compatibility -- shoulda thunk it */
#if __OBSOLETE__ > 20070101
			error(1, "$(%s): obsolete -- use $(-...)", var);
#endif
			s--;
			/*FALLTHROUGH*/
#endif
		case '-':	/* option settings suitable for command line */
			/*
			 * -name	option value if set
			 * --name	option name and value for subsequent set
			 * -+name	option value
			 * -?name	1 if option value was set/unset
			 * -		non-default settings
			 * -+		non-default internal settings
			 * --		all settings
			 * -?		:= default settings
			 * -[^:alnum:]	defined by genop()/listops()
			 */

			if (c = *++s)
			{
				if (isalnum(c))
					c = 0;
				else
					s++;
			}
			if (*s)
			{
				getop(internal.val, s, c);
				return sfstruse(internal.val);
			}
			if (state.mam.statix && (state.never || state.frame->target && !(state.frame->target->property & P_always)))
				return "${NMAKEFLAGS}";
			listops(internal.val, c);
			if (c == '-')
				for (p = internal.preprocess->prereqs; p; p = p->next)
					sfprintf(internal.val, " %s", p->rule->name);
			return sfstruse(internal.val);

		case '=':	/* command line script args and export vars */
			for (n = 1; n < state.argc; n++)
				if (state.argf[n] & (ARG_ASSIGN|ARG_SCRIPT))
				{
					if (sep)
						sfputc(internal.val, ' ');
					else
						sep = 1;
					shquote(internal.val, state.argv[n]);
				}
			for (p = internal.script->prereqs; p; p = p->next)
				if (v = getvar(p->rule->name))
				{
					if (sep)
						sfputc(internal.val, ' ');
					else
						sep = 1;
					sfprintf(internal.val, "%s=%s", v->name, (v->property & V_scan) ? "=" : null);
					shquote(internal.val, v->value);
				}
				else if (strchr(p->rule->name, '='))
				{
					if (sep)
						sfputc(internal.val, ' ');
					else
						sep = 1;
					shquote(internal.val, p->rule->name);
				}
			return sfstruse(internal.val);
		}
		for (pop = -1; *s == var; s++, pop++);
		next = 0;
		tokens = 0;
		for (;;)
		{
			val = null;
			while (*s == ' ')
				s++;
			if (!*s)
			{
				if (tokens)
					goto done;
				r = state.frame->target;
			}
			else
			{
				if (next = strchr(s, ' '))
				{
					*next = 0;
					if (!tokens++)
						state.val++;
				}
				if (*s == MARK_CONTEXT)
				{
					if (!(t = next))
						t = s + strlen(s);
					if (*--t == MARK_CONTEXT)
					{
						*t = 0;
						s++;
					}
					else
						t = 0;
				}
				else
					t = 0;
				if (!(r = getrule(s)))
					switch (var)
					{
					case '!':
					case '&':
					case '?':
						if (staterule(RULE, NiL, s, -1) || staterule(PREREQS, NiL, s, -1))
							r = makerule(s);
						break;
					}
				if (t)
					*t = MARK_CONTEXT;
				if (!r)
					goto done;
				s = r->name;
				if (*s == ATTRNAME && !r->active && !(r->dynamic & D_cached) && strmatch(s, internal.issource))
					r = source(r);
			}
			for (n = pop; n > 0; n--)
			{
				if (!r->active)
					goto done;
				r = r->active->parent->target;
			}
			switch (c = var)
			{

			case '#': /* local arg count */
				val = 0;
				argcount();
				break;

			case ';': /* target data */
				if (r->property & P_statevar)
					r = bind(r);
				if (r->statedata && !(r->property & P_staterule))
					val = r->statedata;
				break;

			case '<': /* target name */
				if ((r->property & (P_joint|P_target)) != (P_joint|P_target))
				{
					val = BINDING(r, op);
					break;
				}
				r = r->prereqs->rule;
				c = '~';
				/*FALLTHROUGH*/

			case '>': /* updated target file prerequisites */
			case '*': /* all target file prerequisites */
			case '~': /* all target prerequisites */
				n = 0;
				if (r->active && (t = r->active->primary))
				{
					x = makerule(t);
					t = BINDING(x, op);
					if (c == '>')
					{
						val = t;
						break;
					}
					sfputr(internal.val, t, -1);
					n = 1;
				}
				else
				{
					x = 0;
					n = 0;
				}
				val = 0;
				e = (c == '>' && !(state.questionable & 0x01000000) && (z = staterule(RULE, r, NiL, -1))) ? z->time : r->time;
				for (p = r->prereqs; p; p = p->next)
				{
					if (p->rule != x && (c == '~' && (!(op & VAL_FILE) || !notfile(p->rule) || (op & VAL_BRACE) && (*p->rule->name == '{' || *p->rule->name == '}') && !*(p->rule->name + 1)) || !notfile(p->rule) &&
					    (c != '>' || !(p->rule->dynamic & D_same) &&
					     (!(r->property & P_archive) && (p->rule->time >= state.start || p->rule->time > e || !(z = staterule(RULE, p->rule, NiL, -1)) || !z->time || !(state.questionable & 0x01000000) && z->time > e) ||
					      (r->property & P_archive) && !(p->rule->dynamic & D_member) && p->rule->time))))
					{
						t = BINDING(p->rule, op);
						if (n)
							sfputc(internal.val, ' ');
						else
						{
							if (!p->next)
							{
								val = t;
								break;
							}
							else
								n = 1;
							if (sep)
								sfputc(internal.val, ' ');
							else
								sep = 1;
						}
						sfputr(internal.val, t, -1);
					}
				}
				break;

			case '@':	/* target action */
				if (r->action)
					val = r->action;
				break;

			case '%':	/* target stem or functional args */
				if (r->active && r->active->stem)
				{
					val = r->active->stem;
					if (state.context && (t = strrchr(val, '/')))
						val = t + 1;
				}
				else
					val = unbound(r);
				break;

			case '!':	/* explicit and generated file prerequisites  */
			case '&':	/* explicit and generated state prerequisites */
			case '?':	/* all explicit and generated prerequisites   */
				sep = genprereqs(internal.val, r, c == '&', c == '?', sep, op);
				val = 0;
				break;

			case '^':	/* original bound name */
				if (!r->active)
					break;
				if ((r->property & (P_joint|P_target)) != (P_joint|P_target))
				{
					if (r->active->original && !streq(r->active->original, r->name))
						val = state.localview ? localview(makerule(r->active->original)) : r->active->original;
					break;
				}
				for (p = r->prereqs->rule->prereqs; p; p = p->next)
					if (p->rule->active && (t = p->rule->active->original))
					{
						if (sep)
							sfputc(internal.val, ' ');
						else
							sep = 1;
						sfputr(internal.val, state.localview ? localview(makerule(t)) : t, -1);
					}
				val = 0;
				break;

			default:
#if DEBUG
				error(1, "%c: invalid internal variable name", c);
#endif
				return null;
			}
		done:
			if (tokens)
			{
				if (val && *val)
				{
					if (sep)
						sfputc(internal.val, ' ');
					else
						sep = 1;
					sfputr(internal.val, val, -1);
				}
				if (!next)
				{
					state.val--;
					return sfstruse(internal.val);
				}
				*next++ = ' ';
				s = next;
			}
			else if (val)
				return val;
			else
				return sfstruse(internal.val);
		}
	}
	else if ((v = getvar(s)) || (t = strchr(s, ' ')))
	{
		if (v)
		{
			t = 0;
			if (!(v->property & V_functional) || (r = getrule(v->name)) && !(r->property & P_functional))
				r = 0;
		}
		else
		{
			/*
			 * functional var with args
			 */

			*t = 0;
			if (!(v = getvar(s)) || !v->builtin)
			{
				if (!(r = getrule(s)) || !(r->property & P_functional))
					r = catrule(".", s, ".", 0);
				if (!r || !(r->property & P_functional))
				{
					*t++ = ' ';
					return null;
				}
				if (v = getvar(r->name))
					v->property |= V_functional;
				else
					v = setvar(r->name, NiL, V_functional);
			}
			*t++ = ' ';
		}
		if (v->builtin)
		{
			ap = arg;
			if (t)
			{
				for (;;)
				{
					while (isspace(*t))
						t++;
					if (!*t)
						break;
					*ap++ = t;
					if (ap >= &arg[elementsof(arg) - 1])
						break;
					while (*t && !isspace(*t))
						t++;
					if (*t == '"' || *t == '\'')
					{
						o = t;
						n = *t++;
						while (*t && (n || !isspace(*t)))
						{
							if (*t == n)
								n = 0;
							else if (!n && (*t == '"' || *t == '\''))
								n = *t;
							else
								*o++ = *t;
							t++;
						}
						*o = 0;
					}
					if (!*t)
						break;
					*t++ = 0;
				}
			}
			*ap = 0;
			return (t = (*v->builtin)(arg)) ? t : null;
		}
		if (r)
			maketop(r, 0, t ? t : null);
		if (state.reading && !state.global)
		{
			v->property &= ~V_compiled;
			if (istype(*s, C_ID1))
				v->property |= V_frozen;
		}
		if (state.mam.regress && state.user > 1 && (v->property & (V_import|V_local_E)) == V_import)
		{
			v->property |= V_local_E;
			dumpregress(state.mam.out, "setv", v->name, v->value);
		}
		t = (op & VAL_PRIMARY) ? v->value : null;
		if ((v->property & V_auxiliary) && (op & VAL_AUXILIARY) && (a = auxiliary(v->name, 0)) && *a->value)
		{
			if (!*t)
				return a->value;
			sfprintf(internal.val, "%s %s", t, a->value);
			t = sfstruse(internal.val);
		}
		return t;
	}
	if (state.reading && !state.global && istype(*s, C_ID1) && (v = setvar(s, null, 0)))
		v->property |= V_frozen;
	return null;
}

/*
 * reset variable p value to v
 * append!=0 if v is from append
 */

static void
resetvar(register Var_t* p, char* v, int append)
{
	register int	n;

	n = strlen(v);
	if (!p->value || (p->property & V_import) || n > p->length)
	{
		if (append)
			n = (n + 1023) & ~1023;
		if (n < MINVALUE)
			n = MINVALUE;
		if (!(p->property & V_free))
		{
			p->property |= V_free;
			p->value = 0;
		}
		p->value = newof(p->value, char, n + 1, 0);
		p->length = n;
	}
	strcpy(p->value, v);
}

/*
 * set the value of a variable
 */

Var_t*
setvar(char* s, char* v, int flags)
{
	register char*		t;
	register Var_t*		p;
	register int		n;
	int			isid;
	int			undefined;

	if (!v)
		v = null;

	/*
	 * the name determines the variable type
	 */

	n = nametype(s, NiL);
	if (n & NAME_statevar)
	{
		bindstate(makerule(s), v);
		return 0;
	}
	if (!(isid = !!(n & NAME_identifier)) && !(n & (NAME_variable|NAME_intvar)) && !istype(*s, C_VARIABLE1|C_ID1|C_ID2) && *s != '(')
	{
		if (flags & V_retain)
			return 0;
		error(2, "%s: invalid variable name", s);
	}

	/*
	 * check for a previous definition
	 */

	if (undefined = !(p = getvar(s)))
	{
		newvar(p);
		if (p->property & V_import)
		{
			p->property &= ~V_import;
			p->value = 0;
		}
		else if (p->value)
			*p->value = 0;
		p->name = putvar(0, p);
		p->builtin = 0;
	}

	/*
	 * check the variable attributes for precedence
	 */

	if (flags & V_auxiliary)
	{
		if (!p->value)
		{
			p->value = null;
			p->property |= V_import;
		}
		p->property |= V_auxiliary;
		p = auxiliary(s, 1);
	}
	p->property |= flags & (V_builtin|V_functional);
	if (state.user || state.readonly || undefined || !(p->property & V_readonly) && (!state.pushed && !(p->property & V_import) || state.global != 1 || (flags & V_import) || state.base && !state.init))
	{
		if (flags & V_import)
		{
			if (p->property & V_free)
			{
				p->property &= ~V_free;
				free(p->value);
			}
			p->value = v;
		}
		else
		{
			t = v;
			if (state.user)
				p->property &= ~V_append;
			if (n = (flags & V_append))
			{
				if (state.reading && !state.global && isid)
					p->property |= V_frozen;
				if (p->value && *p->value)
				{
					if (*v)
					{
						sfprintf(internal.nam, "%s %s", p->value, v);
						t = sfstruse(internal.nam);
					}
					else
						t = p->value;
				}
			}
			resetvar(p, t, n);
		}
		if (flags & V_import)
			p->property |= V_import;
		else
			p->property &= ~V_import;
		if (state.readonly)
		{
			p->property |= V_readonly;
			if (flags & V_append)
				p->property |= V_append;
		}
		else if (state.init)
			p->property |= V_compiled;
		else
			p->property &= ~V_compiled;
		if ((flags & V_scan) && !(p->property & V_scan))
		{
			if (isid && state.user <= 1)
			{
				p->property |= V_scan;
				staterule(VAR, NiL, p->name, -1);
			}
			else
				error(1, "%s: not marked as candidate state variable", p->name);
		}
		if (state.vardump && !(p->property & V_import))
			dumpvar(sfstdout, p);
	}
	else if (state.reading)
	{
		if (p->property & V_readonly)
		{
			/*
			 * save old value for makefile compiler
			 */

			s = (p->property & V_oldvalue) ? getold(p->name) : (char*)0;
			t = v;
			if (flags & V_append)
			{
				if (state.reading && !state.global && isid)
					p->property |= V_frozen;
				if (s && *s)
				{
					if (*v)
					{
						sfprintf(internal.nam, "%s %s", s, v);
						t = sfstruse(internal.nam);
					}
					else
						t = s;
				}
			}
			putold(p->name, strdup(t));
			p->property |= V_oldvalue;
			if ((p->property & V_append) && p->value && *p->value)
			{
				sfprintf(internal.nam, "%s %s", t, p->value + (s ? strlen(s) + 1 : 0));
				resetvar(p, sfstruse(internal.nam), 1);
			}
		}
		if (isid && (flags & V_scan) && state.makefile)
		{
			p->property |= V_scan;
			staterule(VAR, NiL, p->name, -1);
		}
	}
	if (!p->value)
	{
		p->value = strdup(null);
		p->property |= V_free;
	}
	return p;
}

/*
 * translate ':' in s's expanded value to del in sp for list generators
 * 0 returned if s empty
 */

char*
colonlist(register Sfio_t* sp, register char* s, int exp, register int del)
{
	register char*	p;
	Var_t*		v;

	if (exp)
	{
		if (!(v = getvar(s)))
			return 0;
		s = v->value;
	}
	expand(sp, s);
	for (p = sfstruse(sp); isspace(*p); p++);
	if (!*(s = p))
		return 0;
	for (;;)
		switch (*p++)
		{
		case ':':
			if (*(p - 1) = del)
				break;
			/*FALLTHROUGH*/
		case 0:
			return s;
		}
}

/*
 * copy variable reference into sp
 */

void
localvar(Sfio_t* sp, register Var_t* v, char* value, int property)
{
	register char*	s;
	register int	c;
	char*		prefix;
	char*		t;
	Var_t*		x;

	prefix = (property & V_local_D) ? null : "_";
	if (state.mam.out && (!(v->property & property) || !sp))
	{
		v->property |= property;
		sfprintf(state.mam.out, "%ssetv %s%s ", state.mam.label, prefix, v->name);
		if (*(s = value))
		{
			sfprintf(state.mam.out, "%s%s", (property & V_local_D) ? "-D" : null, v->name);
			if (!(property & V_local_D) || *s != '1' || *(s + 1))
			{
				sfputc(state.mam.out, '=');
				sfputc(state.mam.out, '"');

				/*
				 * this quoting allows simple parameterization
				 */

				while (c = *s++)
				{
					switch (c)
					{
					case '$':
						if (istype(*s, C_ID1))
						{
							for (t = s; istype(*t, C_ID1|C_ID2); t++);
							c = *t;
							*t = 0;
							x = getvar(s);
							*t = c;
							c = '$';
							if (x)
								break;
						}
						else if (*s == '{')
							break;
						/*FALLTHROUGH*/
					case '\\':
					case '"':
					case '`':
						sfputc(state.mam.out, '\\');
						break;
					}
					sfputc(state.mam.out, c);
				}
				sfputc(state.mam.out, '"');
			}
		}
		else if (property & V_local_D)
			sfprintf(state.mam.out, "-U%s%s", prefix, v->name);
		sfputc(state.mam.out, '\n');
	}
	if (sp)
		sfprintf(sp, "\"${%s%s}\"", prefix, v->name);
}

/*
 * read the environment and set internal variables with setvar()
 * complicated by those who pollute the environment
 */

void
readenv(void)
{
	register char**	e;
	register char*	t;

	for (e = environ; t = *e; e++)
		if (istype(*t, C_ID1))
		{
			while (istype(*t, C_ID2))
				sfputc(internal.nam, *t++);
			if (*t++ == '=')
				setvar(sfstruse(internal.nam), t, V_import);
			else
				sfstrseek(internal.nam, 0, SEEK_SET);
		}
}
