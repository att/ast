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
 * make dump and trace routines
 */

#include "make.h"

static int		dumpall;	/* don't be selective		*/

/*
 * list a rule name in re-readable form
 */

static void
dumpname(register Sfio_t* sp, register Rule_t* r, char* sep)
{
	register char*	s;
	register char*	t;
	register int	paren;
	int		quote;

	if (!dumpall && state.list)
	{
		s = (r->property & P_state) || (r->dynamic & D_alias) ? r->name : unbound(r);
		for (t = s, paren = 0; *t && (paren || !istype(*t, C_TERMINAL)); t++)
			if (*t == '(')
				paren++;
			else if (*t == ')')
				paren--;
		if (quote = (*t || t == s))
			sfputc(sp, '"');
		for (; *s; s++)
		{
			switch (*s)
			{
			case '"':
				if (quote)
					sfputc(sp, '\\');
				break;
			case '$':
				if (*(s + 1) == '(')
					sfputc(sp, '$');
				break;
			}
			sfputc(sp, *s);
		}
		if (quote)
			sfputc(sp, '"');
	}
	else
	{
		if (*r->name)
			sfputr(sp, r->name, -1);
		else
		{
			sfputc(sp, '"');
			sfputc(sp, '"');
		}
		if (!(r->property & P_state) && r->uname)
			sfprintf(sp, "==%s", r->uname);
	}
#if DEBUG
	if (state.test & 0x00004000)
		sfprintf(sp, "@%p", r);
#endif
	sfputr(sp, sep, -1);
}

/*
 * list a single rule and its attributes
 */

static int
listrule(const char* s, char* v, void* h)
{
	register Rule_t*	r = (Rule_t*)v;
	register List_t*	p;
	register int		n;
	register Sfio_t*	sp = (Sfio_t*)h;

	if (!dumpall && state.list && (r->name != s ||
		!(r->property & (P_accept|P_after|P_always|P_archive|P_before|P_command|P_force|P_foreground|P_functional|P_ignore|P_immediate|P_implicit|P_joint|P_local|P_make|P_multiple|P_operator|P_parameter|P_read|P_repeat|P_target|P_terminal|P_use|P_virtual)) && !((r->property & P_attribute) && r->attribute) && !r->semaphore))
			return 0;
	sfputc(sp, '\n');
	dumpname(sp, r, " : ");
	if (dumpall || !state.list)
	{
		if ((r->property & P_staterule) && isaltstate(r->name))
			sfputr(sp, "cancel=", -1);
		sfprintf(sp, "[%s] ", timestr(r->time));
	}
	if (!(r->property & P_attribute))
	{
		if (r->attribute)
			for (p = internal.attribute->prereqs; p; p = p->next)
				if (r->attribute & p->rule->attribute)
					sfputr(sp, p->rule->name, ' ');
		if (r->scan)
			for (p = internal.scan->prereqs; p; p = p->next)
				if (p->rule->scan == r->scan)
				{
					sfputr(sp, p->rule->name, ' ');
					break;
				}
	}
	if (dumpall || !state.list)
	{
		if (r->property & P_state)
		{
			if (r->property & P_staterule)
			{
				if (!isaltstate(r->name))
					sfprintf(sp, "event=[%s] ", timestr(r->event));
			}
			else if (r->property & P_statevar)
				sfputr(sp, "statevar", ' ');
			else
				sfputr(sp, "state", ' ');
		}
		if (r->view > r->preview)
			sfprintf(sp, "view=%c/%c ", VIEWOFFSET + r->view, VIEWOFFSET + r->preview);
		else if (r->view)
			sfprintf(sp, "view=%c ", VIEWOFFSET + r->view);
		else if (r->preview && r->preview <= state.maxview)
			sfprintf(sp, "preview=%c ", VIEWOFFSET + r->preview);
		if (r->action && !*r->action)
			sfputr(sp, "null", ' ');
		if (r->must)
			sfprintf(sp, "must=%d ", r->must);
		if (r->semaphore)
			sfprintf(sp, "semaphore=%d ", r->semaphore - 1);

		if (r->property & P_archive)
			sfputr(sp, "archive", ' ');
		if (r->property & P_accept)
			sfputr(sp, "accept", ' ');
		if (r->active)
			sfputr(sp, "active", ' ');
		if (r->property & P_after)
			sfputr(sp, "after", ' ');
		if (r->property & P_always)
			sfputr(sp, "always", ' ');
		if (r->property & P_attribute)
		{
			if (r->attribute)
				sfprintf(sp, "attribute=0x%08x ", r->attribute);
			else if (r->scan)
				sfprintf(sp, "scan=%d ", r->scan);
			else
				sfputr(sp, "attribute", ' ');
		}
		if (r->property & P_before)
			sfputr(sp, "before", ' ');
		if (r->property & P_command)
			sfputr(sp, "command", ' ');
		if (r->property & P_dontcare)
			sfputr(sp, "dontcare", ' ');
		if (r->property & P_force)
			sfputr(sp, "force", ' ');
		if (r->property & P_foreground)
			sfputr(sp, "foreground", ' ');
		if (r->property & P_functional)
			sfputr(sp, "functional", ' ');
		if (r->property & P_ignore)
			sfputr(sp, "ignore", ' ');
		if (r->property & P_immediate)
			sfputr(sp, "immediate", ' ');
		if (r->property & P_implicit)
			sfputr(sp, "implicit", ' ');
		if (r->property & P_internal)
			sfputr(sp, "internal", ' ');
		if (r->property & P_joint)
			sfputr(sp, "joint", ' ');
		if (r->property & P_local)
			sfputr(sp, "local", ' ');
		if (r->property & P_make)
			sfputr(sp, "make", ' ');
		if (r->property & P_metarule)
			sfputr(sp, "metarule", ' ');
		if (r->property & P_multiple)
			sfputr(sp, "multiple", ' ');
		if (r->property & P_operator)
			sfputr(sp, "operator", ' ');
		if (r->property & P_parameter)
			sfputr(sp, "parameter", ' ');
		if (r->property & P_read)
			sfputr(sp, "read", ' ');
		if (r->property & P_readonly)
			sfputr(sp, "readonly", ' ');
		if (r->property & P_repeat)
			sfputr(sp, "repeat", ' ');
		if (r->property & P_target)
			sfputr(sp, "target", ' ');
		if (r->property & P_terminal)
			sfputr(sp, "terminal", ' ');
		if (r->property & P_use)
			sfputr(sp, "use", ' ');
		if (r->property & P_virtual)
			sfputr(sp, "virtual", ' ');

		if (r->dynamic & D_alias)
			sfputr(sp, "alias", ' ');
		if (r->dynamic & D_aliaschanged)
			sfputr(sp, "aliaschanged", ' ');
#if BINDINDEX
		if (r->dynamic & D_bindindex)
			sfputr(sp, "bindindex", ' ');
#endif
		if (r->dynamic & D_built)
			sfputr(sp, "built", ' ');
		if (r->dynamic & D_cached)
			sfputr(sp, "cached", ' ');
		if (r->dynamic & D_compiled)
			sfputr(sp, "compiled", ' ');
		if (r->dynamic & D_context)
			sfputr(sp, "context", ' ');
		if (r->dynamic & D_dynamic)
			sfputr(sp, "dynamic", ' ');
		if (r->dynamic & D_entries)
			sfputr(sp, "entries", ' ');
		if (r->dynamic & D_garbage)
			sfputr(sp, "garbage", ' ');
		if (r->dynamic & D_global)
			sfputr(sp, "global", ' ');
		if (r->dynamic & D_hasafter)
			sfputr(sp, "hasafter", ' ');
		if (r->dynamic & D_hasbefore)
			sfputr(sp, "hasbefore", ' ');
		if (r->dynamic & D_hasmake)
			sfputr(sp, "hasmake", ' ');
		if (r->dynamic & D_hasscope)
			sfputr(sp, "hasscope", ' ');
		if (r->dynamic & D_hassemaphore)
			sfputr(sp, "hassemaphore", ' ');
		if (r->dynamic & D_index)
			sfputr(sp, "index", ' ');
		if (r->dynamic & D_intermediate)
			sfputr(sp, "intermediate", ' ');
		if (r->dynamic & D_lower)
			sfputr(sp, "lower", ' ');
		if (r->dynamic & D_lowres)
			sfputr(sp, "lowres", ' ');
		if (r->dynamic & D_member)
			sfputr(sp, "member", ' ');
		if (r->dynamic & D_membertoo)
			sfputr(sp, "membertoo", ' ');
		if (r->dynamic & D_regular)
			sfputr(sp, "regular", ' ');
		if (r->dynamic & D_same)
			sfputr(sp, "same", ' ');
		if (r->dynamic & D_scanned)
			sfputr(sp, "scanned", ' ');
		if (r->dynamic & D_scope)
			sfputr(sp, "scope", ' ');
		if (r->dynamic & D_select0)
			sfputr(sp, "select0", ' ');
		if (r->dynamic & D_select1)
			sfputr(sp, "select1", ' ');
		if (r->dynamic & D_source)
			sfputr(sp, "source", ' ');
		if (r->dynamic & D_triggered)
			sfputr(sp, "triggered", ' ');
		if ((r->property & (P_state|P_statevar)) == P_state)
			sfputr(sp, "state", -1);
		else if (!(r->dynamic & D_bound))
			sfputr(sp, "unbound", -1);
		else
			switch (r->status)
			{
			case NOTYET:
				break;
			case UPDATE:
				sfputr(sp, "UPDATE", -1);
				break;
			case MAKING:
				sfputr(sp, "MAKING", -1);
				break;
			case TOUCH:
				sfputr(sp, "TOUCH", -1);
				break;
			case EXISTS:
				sfputr(sp, "EXISTS", -1);
				break;
			case IGNORE:
				sfputr(sp, "IGNORE", -1);
				break;
			case FAILED:
				sfputr(sp, "FAILED", -1);
				break;
			case OLDRULE:
				sfputr(sp, "OLDRULE", -1);
				break;
#if DEBUG
			default:
				sfprintf(sp, "STATUS=%d", r->status);
				break;
#endif
			}
		if (r->mark)
		{
			sfputr(sp, " |mark", '|');
			if (r->mark & M_bind)
				sfputr(sp, "bind", '|');
			if (r->mark & M_compile)
				sfputr(sp, "compile", '|');
			if (r->mark & M_directory)
				sfputr(sp, "directory", '|');
			if (r->mark & M_generate)
				sfputr(sp, "generate", '|');
			if (r->mark & M_mark)
				sfputr(sp, "mark", '|');
			if (r->mark & M_metarule)
				sfputr(sp, "metarule", '|');
			if (r->mark & M_scan)
				sfputr(sp, "scan", '|');
			if (r->mark & M_waiting)
				sfputr(sp, "waiting", '|');
		}
		sfputc(sp, '\n');
	}
	else
	{
		if (r->property & P_accept)
			sfputr(sp, internal.accept->name, ' ');
		if (r->property & P_after)
			sfputr(sp, internal.after->name, ' ');
		if (r->property & P_always)
			sfputr(sp, internal.always->name, ' ');
		if (r->property & P_attribute)
		{
			if (r->attribute)
				sfputr(sp, internal.attribute->name, ' ');
			else if (r->scan)
				sfputr(sp, internal.scan->name, ' ');
		}
		if (r->property & P_before)
			sfputr(sp, internal.before->name, ' ');
		if (r->property & P_force)
			sfputr(sp, internal.force->name, ' ');
		if (r->property & P_foreground)
			sfputr(sp, internal.foreground->name, ' ');
		if (r->property & P_functional)
			sfputr(sp, internal.functional->name, ' ');
		if (r->property & P_joint)
			sfputr(sp, internal.joint->name, ' ');
		if (r->property & P_make)
			sfputr(sp, internal.make->name, ' ');
		if (r->property & P_multiple)
			sfputr(sp, internal.multiple->name, ' ');
		if (r->action && !*r->action)
			sfputr(sp, internal.null->name, ' ');
		if (r->property & P_archive)
			sfputr(sp, internal.archive->name, ' ');
		if (r->property & P_command)
			sfputr(sp, internal.command->name, ' ');
		if (r->property & P_ignore)
			sfputr(sp, internal.ignore->name, ' ');
		if (r->property & P_immediate)
			sfputr(sp, internal.immediate->name, ' ');
		if (r->property & P_implicit)
			sfputr(sp, internal.implicit->name, ' ');
		if (r->property & P_local)
			sfputr(sp, internal.local->name, ' ');
		if (r->property & P_operator)
			sfputr(sp, internal.op->name, ' ');
		if (r->property & P_parameter)
			sfputr(sp, internal.parameter->name, ' ');
		if (r->property & P_repeat)
			sfputr(sp, internal.repeat->name, ' ');
		if (r->semaphore)
			for (n = r->semaphore - 1; n; n--)
				sfputr(sp, internal.semaphore->name, ' ');
		if (r->property & P_terminal)
			sfputr(sp, internal.terminal->name, ' ');
		if ((r->property & (P_metarule|P_use))
			== P_use) sfputr(sp, internal.use->name, ' ');
		if (r->property & P_virtual)
			sfputr(sp, internal.virt->name, ' ');
	}
	if (p = r->prereqs)
	{
		if (dumpall || !state.list)
			sfputr(sp, " prerequisites:", ' ');
		for (; p; p = p->next)
			dumpname(sp, p->rule, " ");
		sfputc(sp, '\n');
	}
	else if (!dumpall && state.list)
		sfputc(sp, '\n');
	if (r->action && *r->action)
	{
		if (dumpall || !state.list)
			sfputr(sp, " action:", '\n');
		dumpaction(sp, NiL, r->action, "\t");
	}
	if ((dumpall || !state.list) && (r->property & P_statevar) && r->statedata)
		sfprintf(sp, " state: %s\n", r->statedata);
	return 0;
}

/*
 * list a single variable and its value
 */

static int
listvar(const char* s, char* u, void* h)
{
	register Var_t*		v = (Var_t*)u;
	register char*		t;
	register char*		q;
	register Sfio_t*	sp = (Sfio_t*)h;

	if (dumpall || !(v->property & V_import) && (!state.list || !isintvar(v->name)))
	{
		if (!dumpall && state.list)
		{
			for (q = (char*)s; istype(*q, C_ID1|C_ID2); q++);
			if (*q)
				q = "\"";
		}
		else
			q = null;
		sfprintf(sp, "%s%s%s ", q, s, q);
#if DEBUG
		if (state.test & 0x00004000)
			sfprintf(sp, "@%p ", v);
#endif
		if (dumpall || !state.list)
		{
			sfputr(sp, "[", ' ');
			if (v->property & V_append)
				sfputr(sp, "append", ' ');
			if (v->property & V_auxiliary)
				sfputr(sp, "auxiliary", ' ');
			if (v->property & V_builtin)
				sfputr(sp, "builtin", ' ');
			if (v->property & V_free)
				sfputr(sp, "free", ' ');
			if (v->property & V_frozen)
				sfputr(sp, "frozen", ' ');
			if (v->property & V_functional)
				sfputr(sp, "functional", ' ');
			if (v->property & V_import)
				sfputr(sp, "import", ' ');
			if (v->property & V_local_D)
				sfputr(sp, "local_D", ' ');
			if (v->property & V_local_E)
				sfputr(sp, "local_E", ' ');
			if (v->property & V_oldvalue)
			{
				sfputr(sp, "oldvalue", -1);
				if (t = getold(s))
					sfprintf(sp, "=`%s'", t);
				sfputc(sp, ' ');
			}
			if (v->property & V_readonly)
				sfputr(sp, "readonly", ' ');
			if (v->property & V_restored)
				sfputr(sp, "restored", ' ');
			if (v->property & V_retain)
				sfputr(sp, "retain", ' ');
			if (v->property & V_scan)
				sfputr(sp, "scan", ' ');
			if (v->property & V_scope)
				sfputr(sp, "scope", ' ');
			sfputr(sp, "]", ' ');
		}
		sfprintf(sp, "= %s", t = v->value);
		if ((v->property & V_auxiliary) && (v = auxiliary(v->name, 0)))
		{
			if (!dumpall && state.list)
				sfprintf(sp, "\n%s%s%s ", q, s, q);
			else if (*t)
				sfputc(sp, ' ');
			sfprintf(sp, "&= %s", v->value);
		}
		sfputc(sp, '\n');
	}
	return 0;
}

/*
 * dump rules and variables
 */

void
dump(Sfio_t* sp, int verbose)
{
	static int	dumping;

	if (!dumping++)
	{
		if (state.vardump || state.ruledump)
			state.list = 0;
		if (verbose)
			hashdump(NiL, (error_info.trace <= -20) ? HASH_BUCKET : 0);
		if (state.list)
			sfprintf(sp, "/* %s */\n\n", version);
		if (!dumpall && (state.list || state.vardump))
		{
			sfprintf(sp, "\n/* Variables */\n\n");
			hashwalk(table.var, 0, listvar, sp);
		}
		if (!dumpall && (state.list || state.ruledump))
		{
			sfprintf(sp, "\n/* Rules */\n");
			hashwalk(table.rule, 0, listrule, sp);
		}
		sfsync(sp);
		dumping--;
	}
}

/*
 * dump regression prefix,name,value
 */

void
dumpregress(register Sfio_t* sp, const char* prefix, const char* name, register char* value)
{
	register int	c;
	register int	n;
	int*		rp;
	char*		bp;
	char*		np;

	static int	index;

	sfprintf(sp, "%s%s %s ", state.mam.label, prefix, name ? name : "-");
	n = -1;
	if (value)
		for (;;)
		{
			switch (c = *value++)
			{
			case 0:
				break;
			case '\n':
				sfprintf(sp, "\n%s%s %s ", state.mam.label, prefix, name ? name : "-");
				n = 1;
				continue;
			case '\t':
				c = ' ';
				/*FALLTHROUGH*/
			case ' ':
				if (n < 0 || *value == ' ' || *value == '\t' || *value == 0 || *value == '\n')
					continue;
				/*FALLTHROUGH*/
			case '\'':
			case '"':
			case '=':
			case ':':
				sfputc(sp, c);
				n = 1;
				continue;
			case '/':
				if (n)
				{
					bp = np = value - 1;
					for (;;)
					{
						switch (*value++)
						{
						case ' ':
						case '\t':
						case '\n':
						case '\'':
						case '"':
						case ':':
						case 0:
							break;
						case '/':
							np = value - 1;
							/*FALLTHROUGH*/
						default:
							continue;
						}
						break;
					}
					c = *--value;
					*value = 0;
					if (strmatch(np, "*.*"))
					{
						*value = c;
						c = *(value = np);
						*value = 0;
					}
					if (!(rp = getreg(bp)))
					{
						rp = newof(0, int, 1, 0);
						*rp = ++index;
						putreg(0, rp);
					}
					sfprintf(sp, "${PATH_%d}", *rp);
					*value = c;
					n = 0;
				}
				else
					sfputc(sp, c);
				continue;
			case '-':
			case '+':
				sfputc(sp, c);
				if (n)
				{
					if (!(c = *value++))
						break;
					sfputc(sp, c);
					n = isalnum(c) ? 1 : 0;
				}
				continue;
			default:
				sfputc(sp, c);
				n = 0;
				continue;
			}
			break;
		}
	sfputc(sp, '\n');
}

/*
 * dump an action placing prefix at the beginning of each line
 */

void
dumpaction(Sfio_t* sp, const char* name, register char* action, register const char* prefix)
{
	register char*	s;
	char*		mamlabel;
	char*		sep;

	if (prefix)
	{
		mamlabel = null;
		name = null;
		sep = null;
	}
	else
	{
		if (state.mam.regress)
		{
			dumpregress(sp, "exec", name, action);
			return;
		}
		if (state.mam.out)
		{
			mamlabel = state.mam.label;
			prefix = "exec";
			if (!name)
				name = "-";
			sep = " ";
		}
		else
		{
			while (isspace(*action))
				action++;
			if (!*action)
				return;
			mamlabel = null;
			prefix = "+";
			name = null;
			sep = null;
		}
	}
	for (;;)
	{
		if (s = strchr(action, '\n'))
			*s = 0;
		sfprintf(sp, "%s%s %s%s%s\n", mamlabel, prefix, name, sep, action);
		if (!s)
			break;
		*s++ = '\n';
		action = s;
	}
	sfsync(sp);
}

/*
 * dump variable info
 */

void
dumpvar(Sfio_t* sp, register Var_t* v)
{
	dumpall++;
	listvar(v->name, (char*)v, sp);
	sfsync(sp);
	dumpall--;
}

/*
 * dump rule info
 */

void
dumprule(Sfio_t* sp, register Rule_t* r)
{
	register int		i;
	register Rule_t*	z;

	dumpall++;
	z = 0;
	while ((r->dynamic & D_alias) && r != z)
	{
		listrule(r->name, (char*)r, sp);
		z = r;
		if (!(r = getrule(unbound(r))))
			r = z;
	}
	if (r != z)
		listrule(r->name, (char*)r, sp);
	if (!(r->property & P_state))
		for (i = RULE; i <= STATERULES; i++)
			if (z = staterule(i, r, NiL, -1))
				listrule(z->name, (char*)z, sp);
	sfsync(sp);
	dumpall--;
}
