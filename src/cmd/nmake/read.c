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
 * make file read routines
 */

#include "make.h"

/*
 * read the base and global rules
 */

static void
readrules(void)
{
	register char*		s;
	register Sfio_t*	tmp;
	register List_t*	p;

	state.global = 1;

	/*
	 * read the base rules
	 */

	if (!(s = state.rules))
		state.rules = null;
	else if (*s)
	{
		tmp = sfstropen();
		edit(tmp, s, KEEP, KEEP, external.object);
		readfile(sfstruse(tmp), COMP_BASE|(state.explicitrules ? COMP_RULES : 0), NiL);
		edit(tmp, s, DELETE, KEEP, DELETE);
		state.rules = strdup(sfstruse(tmp));
		sfstrclose(tmp);
	}
	setvar(external.rules, state.rules, 0)->property |= V_compiled;

	/*
	 * read the explicit global makefiles
	 *
	 * NOTE: internal.tmplist is used to handle the effects
	 *	 of load() on internal list pointers
	 */

	if (p = internal.globalfiles->prereqs)
	{
		for (p = internal.tmplist->prereqs = listcopy(p); p; p = p->next)
			readfile(p->rule->name, COMP_GLOBAL, NiL);
		freelist(internal.tmplist->prereqs);
		internal.tmplist->prereqs = 0;
	}
	state.global = 0;
}

/*
 * read a file given an open file pointer
 */

static void
readfp(Sfio_t* sp, register Rule_t* r, int type)
{
	register char*	s;
	register char*	t;
	int		n;
	int		needrules;
	int		preprocess;
	int		splice;
	char*		name;
	char*		b;
	char*		e;
	char*		objfile;
	Rule_t*		x;
	Sfio_t*		fp;

	objfile = 0;
	name = r->name;
	if (!state.makefile)
	{
		/*
		 * set up the related file names
		 */

		fp = sfstropen();
		setvar(external.file, r->name, 0)->property |= V_compiled;
		edit(fp, r->name, DELETE, KEEP, KEEP);
		state.makefile = strdup(sfstruse(fp));
		sfstrclose(fp);
		objfile = objectfile();
	}
	needrules = !state.base && !state.rules;

	/*
	 * load if object file
	 */

	if (loadable(sp, r, 0))
	{
		if (!state.base && !state.global && !state.list)
			error(3, "%s: explicit make object files must be global", r->name);
		if (!state.rules)
			readrules();
		message((-2, "loading %sobject file %s", state.global ? "global " : null, r->name));
		if (load(sp, r->name, 0, 0) > 0)
		{
			sfclose(sp);
			return;
		}
		error(3, "%s: must be recompiled", name);
	}

	/*
	 * check object corresponding to file
	 */

	if (state.global || !state.forceread && (!(type & COMP_FILE) || needrules))
	{
		fp = sfstropen();
		if (!objfile)
		{
			edit(fp, r->name, DELETE, KEEP, external.object);
			objfile = sfstruse(fp);
		}
		state.init++;
		x = bindfile(NiL, objfile, 0);
		state.init--;
		sfstrclose(fp);
		if (!x || !x->time)
			/* ignore */;
		else if (x == r)
			error(3, "%s must be recompiled", r->name);
		else if (fp = sfopen(NiL, s = x->name, "br"))
		{
			if (needrules)
				x->dynamic |= D_built;
			if (loadable(fp, x, 1))
			{
				if (needrules)
				{
					if (state.rules && !state.explicitrules)
					{
						edit(internal.tmp, state.rules, DELETE, KEEP, DELETE);
						edit(internal.wrk, b = getval(external.rules, VAL_PRIMARY), DELETE, KEEP, DELETE);
						if (strcmp(sfstruse(internal.tmp), sfstruse(internal.wrk)))
						{
							error(state.exec || state.mam.out ? -1 : 1, "%s: base rules changed to %s", sfstrbase(internal.tmp), sfstrbase(internal.wrk));
							state.rules = b;
							state.forceread = 1;
							needrules = 1;
						}
					}
					if (!state.forceread)
					{
						needrules = 0;
						readrules();
					}
				}
				if (!state.forceread)
				{
					message((-2, "loading %s file %s", state.global ? "global" : "object", s));
					n = load(fp, s, 1, 0);
					if (n > 0)
					{
						sfclose(fp);
						sfclose(sp);
						return;
					}
				}
				r = getrule(name);
			}
			sfclose(fp);
			if (state.global)
				error(1, "%s: reading%s", r->name, state.forceread ? " -- should be compiled before local makefiles" : null);
			else if (state.writeobject)
				error(state.exec || state.mam.out ? -1 : 1, "%s: recompiling", s);
		}
	}

	/*
	 * at this point we have to read it
	 * if its the first makefile then the
	 * base rules must be determined and loaded
	 * along with the global rules before the parse
	 */

	preprocess = state.preprocess;
	if (!state.global)
	{
		/*
		 * first check for and apply makefile converter
		 */

		s = 0;
		if (*(t = getval(external.convert, VAL_PRIMARY)))
		{
			char*	u;
			char*	v;
			Sfio_t*	exp;

			exp = sfstropen();
			if (e = strchr(r->name, '/'))
				e++;
			else
				e = r->name;
			b = tokopen(t, 1);
			while ((t = tokread(b)) && (t = colonlist(exp, t, 0, ' ')))
			{
				u = tokopen(t, 0);
				while ((v = tokread(u)) && !streq(e, v));
				tokclose(u);
				if (!(s = tokread(b)))
				{
					error(2, "%s: %s: no action for file", external.convert, t);
					break;
				}
				if (v)
				{
					s = getarg((e = t = strdup(s), &e), NiL);
					break;
				}
				s = 0;
			}
			tokclose(b);
			sfstrclose(exp);
		}
		if (s)
		{
			message((-2, "converting %s using \"%s\"", r->name, s));
			sfclose(sp);
			if (!(sp = fapply(internal.internal, null, r->name, s, CO_ALWAYS|CO_LOCAL|CO_URGENT)))
				error(3, "%s: error in makefile converter \"%s\"", r->name, s);
			free(t);
			preprocess = -1;
		}
		if (needrules)
		{
			if ((s = sfreserve(sp, 0, 0)) && (n = sfvalue(sp)) >= 0)
			{
				int	c;
				int	d;
				int	old;

				if (n > 0)
				{
					if (n > MAXNAME)
						n = MAXNAME;
					else
						n--;
				}

				/*
				 * quick makefile type check while
				 * checking for base rules
				 */

				old = 0;
				splice = 0;
				b = s;
				c = *(s + n);
				*(s + n) = 0;
				for (;;)
				{
					if (e = strchr(s, '\n'))
						*e = 0;
					else if (c != '\n')
						break;
					if (splice)
						/* skip */;
					else if (*s == SALT)
					{
						while (isspace(*++s));
						for (t = s; isalnum(*t); t++);
						d = *t;
						*t = 0;
						if (strneq(s, "rules", 5))
						{
							if (*t = d)
								t++;
							while (*t == ' ' || *t == '\t')
								t++;
							rules(*t == '/' && *(t + 1) == '*' ? null : t);
							break;
						}
						else if (!strmatch(s, "assert|comment|define|elif|else|endif|endmac|error|ident|if|ifdef|ifndef|include|line|macdef|pragma|unassert|undef|warning"))
							old = 1;
						else if (!preprocess)
							preprocess = 1;
						*t = d;
					}
					else if (*s == '<' && *(s + 1) == '<')
					{
						old = preprocess = 0;
						break;
					}
					else
					{
						while (isspace(*s))
							s++;
						if (strneq(s, "rules", 5))
						{
							for (s += 5; *s == ' ' || *s == '\t'; s++);
							rules(*s == '/' && *(s + 1) == '*' ? null : s);
							old = 0;
							break;
						}
						else if (strneq(s, ".SOURCE", 7) && (*(s + 7) == '.' || *(s + 7) == ':' || isspace(*(s + 7))))
						{
							old = 0;
							break;
						}
						else
						{
							d = ':';
							while (*s)
							{
								if (*s == '/' && *(s + 1) == '*' && (*(s + 2) == '*' || isspace(*(s + 2)) || !*(s + 2)))
									break;
								else if (*s == d)
								{
									if (*++s == d)
										s++;
									else if (isalnum(*s))
									{
										while (isalnum(*s))
											s++;
										if (*s == d)
											break;
									}
									d = 0;
								}
								while (*s && *s != d && !isspace(*s))
									s++;
								while (isspace(*s))
									s++;
							}
							if (*s)
							{
								old = 0;
								break;
							}
						}
					}
					if (!(s = e))
						break;
					splice = e > b && *(e - 1) == '\\';
					*s++ = '\n';
				}
				if (e)
					*e = '\n';
				*(b + n) = c;
				if (old)
					punt(1);
			}
			if (!state.rules)
				state.rules = getval(external.rules, VAL_PRIMARY);
			readrules();
			r = getrule(name);
		}
	}

	/*
	 * check for obsolete makefile preprocessor
	 */

	if (preprocess > 0)
	{
		s = "$(MAKEPP) $(MAKEPPFLAGS) $(>)";
		message((-2, "preprocessing %s using \"%s\"", r->name, s));
		sfclose(sp);
		if (!(sp = fapply(internal.internal, null, r->name, s, CO_ALWAYS|CO_LOCAL|CO_URGENT)))
			error(3, "%s: error in makefile preprocessor \"%s\"", r->name, s);
	}

	/*
	 * parse the file
	 */

	if (state.base)
	{
		if (!state.compile)
			state.compile = RECOMPILE;
		state.global = 1;
	}
	n = state.reading;
	state.reading = 1;
	parse(sp, NiL, r->name, NiL);
	sfclose(sp);
	state.reading = n;
	if (!state.compile && !state.global)
		state.compile = RECOMPILE;
	if ((state.questionable & 0x00000400) || !state.global)
		state.forceread = 1;
}

/*
 * read a makefile
 */

int
readfile(register char* file, int type, char* filter)
{
	register Rule_t*	r;
	Sfio_t*			rfp;
	Stat_t			st;

	if (streq(file, "-") && (file = "/dev/null") || isdynamic(file))
	{
		rfp = sfstropen();
		expand(rfp, file);
		state.init++;
		file = makerule(sfstruse(rfp))->name;
		state.init--;
		sfstrclose(rfp);
	}
	state.init++;
	r = bindfile(NiL, file, BIND_MAKEFILE|BIND_RULE);
	state.init--;
	if (r && (r->time || strneq(r->name, "/dev/", 5) && !rstat(r->name, &st, 0)))
	{
		compref(r, type);
		r->dynamic |= D_scanned;
		file = r->name;
		if (rfp = filter ? fapply(internal.internal, null, file, filter, CO_ALWAYS|CO_LOCAL|CO_URGENT) : rsfopen(file))
		{
			if (state.mam.dynamic || state.mam.regress)
				mampush(state.mam.out, r, P_force);
			if (state.user)
			{
				r->status = EXISTS;
				parse(rfp, NiL, file, NiL);
				sfclose(rfp);
			}
			else
				readfp(rfp, r, type);
			if (state.mam.dynamic || state.mam.regress)
				mampop(state.mam.out, r, 0);
			if ((type & COMP_BASE) && r->uname)
			{
				oldname(r);
				r->dynamic &= ~D_bound;
			}
			if (state.pushed)
			{
				state.pushed = 0;
				state.global = state.push_global;
				state.user = state.push_user;
			}
			return(1);
		}
		if ((type & COMP_DONTCARE) || (r->property & P_dontcare))
		{
			r->property |= P_dontcare;
			return(0);
		}
	}
	if (!(type & COMP_DONTCARE))
		error((type & COMP_INCLUDE) ? 2 : 3, "%s: cannot read%s", file, (type & COMP_INCLUDE) ? " include file" : (type & COMP_GLOBAL) ? " global rules" : (type & COMP_BASE) ? " base rules" : null);
	else if ((type & COMP_INCLUDE) && error_info.line)
		compref(r ? r : makerule(file), type);
	return(0);
}
