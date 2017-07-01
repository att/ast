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
 * make state variable routines
 * this file and the is*() macros specify the state variable name format
 *
 *	(<var>)		the state of <var>
 *	(<var>)<rule>	the state of <var> qualified by <rule>
 *	()<rule>	the state of <rule>
 *
 * NOTE: update VERSION in compile.c if the format changes
 */

#include "make.h"
#include "options.h"

/*
 * return a pointer to the rule <s1><s2><s3>
 * force causes the rule to be created
 * force<0 canon's name first
 */

Rule_t*
catrule(register char* s1, register char* s2, register char* s3, int force)
{
	Rule_t*		r;

	sfputr(internal.nam, s1, *s2++);
	if (*s2)
		sfputr(internal.nam, s2, s3 ? *s3++ : -1);
	if (s3 && *s3)
		sfputr(internal.nam, s3, -1);
	s1 = sfstruse(internal.nam);
	if (!(r = getrule(s1)) && force)
	{
		if (force < 0)
		{
			pathcanon(s1, 0, 0);
			if (r = getrule(s1))
				return r;
		}
		r = makerule(NiL);
	}
	return r;
}

/*
 * return the state file name
 * assumes the main makefile has already been read
 */

char*
statefile(void)
{
	char*		dir;
	Sfio_t*		sp;
	Stat_t		st;

	if (!state.statefile && state.makefile)
	{
		sp = sfstropen();
		dir = DELETE;
		if (!state.writestate || streq(state.writestate, "-") || !stat(state.writestate, &st) && S_ISDIR(st.st_mode) && (dir = state.writestate))
			edit(sp, state.makefile, dir, KEEP, external.state);
		else
			expand(sp, state.writestate);
		state.statefile = strdup(sfstruse(sp));
		sfstrclose(sp);
	}
	return state.statefile;
}

/*
 * reconcile s with state view from r
 *
 * NOTE: requires state.maxview>0 && 0<=view<=state.maxview
 */

static Rule_t*
stateview(int op, char* name, register Rule_t* s, register Rule_t* r, int view, int accept, Rule_t** pv)
{
	register Rule_t*	v;
	register List_t*	p;
	Sfio_t*			fp;

	if (pv)
		*pv = 0;
	if (state.compile < COMPILED)
	{
#if DEBUG
		if (state.test & 0x00000200)
			error(2, "STATEVIEW %d %s state file load delayed until after makefile read", view, name);
#endif
		return 0;
	}
	if (!(state.view[view].flags & BIND_LOADED))
	{
		if (view < state.readstate)
		{
			char*	file;
			Sfio_t*	tmp;
			long	n;

			tmp = sfstropen();
			if (name && !s)
			{
				sfputr(tmp, name, 0);
				n = sfstrtell(tmp);
			}
			else
				n = 0;
			edit(tmp, statefile(), state.view[view].path, KEEP, KEEP);
			sfputc(tmp, 0);
			file = sfstrseek(tmp, n, SEEK_SET);
			if (fp = sfopen(NiL, file, "br"))
			{
				/*
				 * NOTE: this load should not be a problem for
				 *	 internal rule pointers since all
				 *	 non-state rules in state files
				 *	 are just references
				 */

				state.stateview = view;
				message((-2, "loading state view %d file %s", view, file));
				if (load(fp, file, 0, 0) > 0)
					state.view[view].flags |= BIND_EXISTS;
				else if (state.corrupt && *state.corrupt == 'a' && !(state.view[0].flags & BIND_EXISTS))
					state.accept = 1;
				state.stateview = -1;
				sfclose(fp);
			}
			if (n)
				strcpy(name, sfstrbase(tmp));
			sfstrclose(tmp);
		}
		state.view[view].flags |= BIND_LOADED;
	}
	if (name)
	{
		viewname(name, view);
		v = getrule(name);
		unviewname(name);
#if DEBUG
		if (state.test & 0x00000200)
			error(2, "STATEVIEW %s [%s] test %d [%s] -> %s [%s]", name, s ? timestr(s->time) : "no rule", view, timestr(r->time), v ? v->name : null, v ? timestr(v->time) : "no rule");
#endif
		if (v)
		{
			if (pv)
				*pv = v;
			if (s && (op == RULE && (s->event >= v->event && s->event || !v->time && (v->property & P_force)) || op == PREREQS && s->time >= v->time))
				return s;
			if (v->time == r->time || accept || r->view == view || (r->property & (P_state|P_use|P_virtual)) || state.believe && view >= (state.believe - 1))
			{
				if (r->property & P_state)
				{
					if (r->property & P_statevar)
					{
						if (r->statedata && (!v->statedata && *r->statedata || v->statedata && !streq(r->statedata, v->statedata) || r->time > v->time))
							return 0;
						s = r;
						s->statedata = v->statedata;
						if (v->property & P_parameter)
							s->property |= P_parameter;
					}
				}
				else
				{
					if (r->property & P_use)
					{
						if (r->action && (!v->action || !streq(r->action, v->action)) || !r->action && v->action)
							return 0;
						r->time = v->time;
					}
					if (!s)
						s = makerule(name);
				}
				s->time = v->time;
				s->attribute = v->attribute;
				s->event = v->event;
				s->action = v->action;
				if (v->property & P_force)
					s->property |= P_force;
				else
					s->property &= ~P_force;
				if (v->dynamic & D_built)
					s->dynamic |= D_built;
				else
					s->dynamic &= ~D_built;
				if (v->dynamic & D_lowres)
					s->dynamic |= D_lowres;
				else
					s->dynamic &= ~D_lowres;
				s->prereqs = listcopy(v->prereqs);
				s->scan = v->scan;
				for (p = s->prereqs; p; p = p->next)
				{
					v = p->rule;
					if (v->dynamic & D_lower)
					{
						unviewname(v->name);
						p->rule = makerule(v->name);
						viewname(v->name, v->view);
					}
				}
#if DEBUG
				if (state.test & 0x00000200)
				{
					error(2, "STATEVIEW %s accept %d", s->name, view);
					if (state.test & 0x00000400)
						dumprule(sfstderr, s);
				}
#endif
			}
		}
	}
	return s;
}

/*
 * return a pointer to the state rule of var qualified by r
 * force>0 causes the state rule to be created
 * force<0 prevents a state bind
 */

Rule_t*
staterule(int op, register Rule_t* r, char* var, int force)
{
	register Rule_t*	s;
	register int		i;
	char*			rul;
	char*			nam;
	Rule_t*			v;
	int			j;
	int			k;
	int			m;
	int			nobind = force < 0;
	Flags_t*		b;

	switch (op)
	{
	case PREREQS:
		rul = var;
		var = "+";
		if (!r)
		{
			if (nobind)
			{
				sfprintf(internal.nam, "(%s)%s", var, rul);
				r = getrule(sfstruse(internal.nam));
			}
			return r;
		}
		break;
	case RULE:
		rul = var;
		var = null;
		if (!r)
		{
			if (nobind)
			{
				sfprintf(internal.nam, "(%s)%s", var, rul);
				return getrule(sfstruse(internal.nam));
			}
			r = makerule(var);
			nobind = 1;
		}
		break;
	case VAR:
		if (!r)
		{
			sfprintf(internal.nam, "(%s)", var);
			if (!(r = getrule(sfstruse(internal.nam))))
			{
				if (!force)
					return 0;
				r = makerule(sfstrbase(internal.nam));
			}
		}
		if (nobind)
			return r;
		break;
#if DEBUG
	default:
		error(PANIC, "invalid op=%d to staterule()", op);
		break;
#endif
	}
 again:
	if (r->property & P_statevar)
	{
		if (op == PREREQS)
			return 0;
		op = RULE;
		nam = r->name;
		s = r;
	}
	else if (r->property & P_state)
		return 0;
	else
	{
		if ((r->dynamic & (D_member|D_membertoo)) && (rul = strrchr(r->name, '/')))
			rul++;
		else
			rul = unbound(r);
		sfprintf(internal.nam, "(%s)%s", var, rul);
		nam = sfstruse(internal.nam);
		if (s = getrule(nam))
			nam = s->name;
	}
	if (state.maxview && state.readstate && state.makefile && !nobind)
	{
		b = &r->checked[op];
		k = 0;
		if (!tstbit(*b, i = (r->property & (P_statevar|P_use|P_virtual)) && state.targetview > 0 ? state.targetview : r->view) && (!s || !s->time || (r->property & P_statevar) || !(k = statetimeq(r, s))))
		{
			if (!(r->property & (P_statevar|P_use|P_virtual)) && !(r->dynamic & D_bound) && !(r->mark & M_bind) && (s && s->time || !s && state.compile >= COMPILED))
			{
				/*
				 * M_bind guards staterule() recursion
				 */

				s = r;
				s->mark |= M_bind;
				r = bind(r);
				s->mark &= ~M_bind;
				goto again;
			}
			if ((r->property & P_statevar) && r->time == OLDTIME)
			{
				for (i = 0; i <= state.maxview; i++)
				{
					setbit(*b, i);
					setbit(r->checked[CONSISTENT], i);
				}
			}
			else
			{
				if (!(r->property & (P_statevar|P_use|P_virtual)) || state.targetview < 0)
				{
					m = 1;
					j = 0;
					if (!(k = r->view) && !(state.view[0].flags & BIND_EXISTS))
						k = state.maxview;
				}
				else
				{
					m = 0;
					j = k = state.targetview;
				}
#if DEBUG
				if (state.test & 0x00000200)
					error(2, "STATERULE %s search %d..%d targetview=%d", nam, j, k, state.targetview);
#endif
				for (i = j; i <= k; i++)
				{
					setbit(*b, i);
					if (i || !(r->property & P_statevar))
					{
						s = stateview(op, nam, s, r, i, i == k, &v);
						if (v)
						{
							if (m)
							{
								m = i;
								while (++m <= k)
									setbit(*b, m);
							}
							if (op != PREREQS || state.accept)
								do setbit(r->checked[CONSISTENT], i); while (--i > j);
							break;
						}
					}
					if (state.accept)
						setbit(r->checked[CONSISTENT], i);
					if (!i)
					{
						if (op != PREREQS)
							setbit(r->checked[CONSISTENT], i);
						k = state.maxview;
					}
				}
			}
		}
		else if (k)
			setbit(*b, i);
	}
	if (!s && force > 0)
		s = makerule(nam);
	if (s && op == RULE)
	{
		r->attribute |= s->attribute & internal.retain->attribute;
		r->property |= s->property & (P_dontcare|P_terminal);
	}
	return s;
}

/*
 * return a non-state rule pointer corresponding to the staterule r
 * force causes the non-state rule to be created
 */

Rule_t*
rulestate(register Rule_t* r, int force)
{
	register char*		s;

	if (r->property & P_staterule)
	{
		s = r->name;
		while (*s && *s++ != ')');
		if (!(r = getrule(s)) && force)
			r = makerule(s);
	}
	return r;
}

/*
 * return a variable pointer corresponding to the state variable r
 * force causes the variable to be created
 */

Var_t*
varstate(register Rule_t* r, int force)
{
	register char*		s;
	register char*		t;
	register Var_t*		v;

	s = r->name;
	if (r->property & P_state)
	{
		if (r->property & P_statevar)
		{
			s++;
			*(t = s + strlen(s) - 1) = 0;
		}
		else
			return 0;
	}
	else
		t = 0;
	if (!(v = getvar(s)) && force)
		v = setvar(s, null, force < 0 ? V_retain : 0);
	if (t)
		*t = ')';
	return v;
}

/*
 * return the auxiliary variable pointer for s
 * force causes the variable to be created
 */

Var_t*
auxiliary(char* s, int force)
{
	Var_t*		v;

	sfprintf(internal.nam, "(&)%s", s);
	if (!(v = getvar(sfstruse(internal.nam))) && force)
		v = setvar(sfstrbase(internal.nam), null, 0);
	return v;
}

/*
 * force r->scan == (*(unsigned char*)h) or all files to be re-scanned
 */

int
forcescan(const char* s, char* v, void* h)
{
	register Rule_t*	r = (Rule_t*)v;
	register int		n = h ? *((unsigned char*)h) : r->scan;

	NoP(s);
	if ((r->property & P_staterule) && r->scan == n && !(r->dynamic & D_scanned))
		r->property |= P_force;
	return 0;
}

/*
 * report state file lock fatal error
 */

static void
badlock(char* file, int view, Time_t date)
{
	long	d;

	/*
	 * probably a bad lock if too old
	 */

	d = state.regress ? 0 : (CURSECS - tmxsec(date));
	if (d > 24 * 60 * 60)
		error(1, "%s is probably an invalid lock file", file);
	else if (d > 0)
		error(1, "another make has been running on %s in %s for the past %s", state.makefile, state.view[view].path, fmtelapsed(d, 1));
	else
		error(1, "another make is running on %s in %s", state.makefile, state.view[view].path);
	error(3, "use -%c to override", OPT(OPT_ignorelock));
}

/*
 * get|release exclusive (advisory) access to state file
 *
 * this is a general solution that should work on all systems
 * the following problems need to be addressed
 *
 *	o  flock() or lockf() need to work for distributed
 *	   as well as local file systems
 *
 *	o  the file system clock may be different than the
 *	   local system clock
 *
 *	o  creating a specific lock file opens the door for
 *	   lock files laying around after program and system
 *	   crashes -- placing the pid of the locking process
 *	   as file data may not work on distributed process
 *	   systems
 */

#define LOCKTIME(p,m)	((m)?tmxgetmtime(p):tmxgetctime(p))

void
lockstate(int set)
{
	register int		fd;
	register char*		file;
	Time_t			t;
	Stat_t			st;

	static char*		lockfile;
	static Time_t		locktime;
	static int		lockmtime;

	if (set)
	{
		if (!state.exec || state.virtualdot || !state.writestate)
			return;
		edit(internal.nam, statefile(), KEEP, KEEP, external.lock);
		file = strdup(sfstruse(internal.nam));
		if (!state.ignorelock)
		{
			int	uid = geteuid();

			for (fd = 1; fd <= state.maxview; fd++)
			{
				edit(internal.nam, file, state.view[fd].path, KEEP, KEEP);
				if (!stat(sfstruse(internal.nam), &st) && st.st_uid != uid)
					badlock(sfstrbase(internal.nam), fd, LOCKTIME(&st, lockmtime));
			}
		}
		locktime = 0;
		for (;;)
		{
			lockfile = file;
			if ((fd = open(file, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL|O_BINARY|O_CLOEXEC, 0)) >= 0)
				break;
			lockfile = 0;
			if (stat(file, &st) < 0)
				error(3, "cannot create lock file %s", file);
			if (!state.ignorelock)
				badlock(file, 0, LOCKTIME(&st, lockmtime));
			if (remove(file) < 0)
				error(3, "cannot remove lock file %s", file);
		}

		/*
		 * fstat() here would be best but some systems
		 * like cygwin which shall remain nameless
		 * return different time values after the close()
		 */

		close(fd);
		if (stat(file, &st) < 0)
			error(3, "cannot stat lock file %s", file);
		lockmtime = tmxgetatime(&st) < tmxgetmtime(&st) || tmxgetctime(&st) < tmxgetmtime(&st);
		locktime = LOCKTIME(&st, lockmtime);
	}
	else if (lockfile)
	{
		if (locktime)
		{
			if (stat(lockfile, &st) < 0 || (t = LOCKTIME(&st, lockmtime)) != locktime && t != tmxsns(tmxsec(locktime), 0))
			{
				if (state.writestate)
					error(1, "the state file lock on %s has been overridden", state.makefile);
			}
			else if (remove(lockfile) < 0)
				error(1, "cannot remove lock file %s", lockfile);
		}
		else
			remove(lockfile);
		free(lockfile);
		lockfile = 0;
	}
}

/*
 * read state from a previous make
 */

void
readstate(void)
{
	register Sfio_t*	fp;
	char*			file;

	if (state.makefile)
	{
		lockstate(1);
		if (state.readstate && (fp = sfopen(NiL, file = statefile(), "br")))
		{
			state.stateview = 0;
			message((-2, "loading state file %s", file));
			makerule(file)->dynamic |= D_built;
			if (load(fp, file, 0, 10) > 0)
				state.view[0].flags |= BIND_EXISTS;
			else if (!state.corrupt)
				error(3, "use -%c%c to accept current state or -%c to remake", OPT(OPT_accept), OPT(OPT_readstate), OPT(OPT_readstate));
			else if (*state.corrupt == 'a')
				state.accept = 1;
			state.stateview = -1;
			sfclose(fp);
		}
	}
	state.view[0].flags |= BIND_LOADED;
}

/*
 * update the superimposed code
 */

static void
code(register const char* s)
{
	debug((-6, "enter candidate state variable %s", s));
	settype(*s++, C_VARPOS1);
	if (*s)
	{
	 settype(*s++, C_VARPOS2);
	 if (*s)
	 {
	  settype(*s++, C_VARPOS3);
	  if (*s)
	  {
	   settype(*s++, C_VARPOS4);
	   if (*s)
	   {
	    settype(*s++, C_VARPOS5);
	    if (*s)
	    {
	     settype(*s++, C_VARPOS6);
	     if (*s)
	     {
	      settype(*s++, C_VARPOS7);
	      if (*s) settype(*s, C_VARPOS8);
	     }
	    }
	   }
	  }
	 }
	}
}

/*
 * bind() and scan() r->parameter file prerequisites
 * this catches all implicit state variables before the
 * parent files are scanned
 */

static int
checkparam(const char* s, char* v, void* h)
{
	register Rule_t*	r = (Rule_t*)v;
	register List_t*	p;
	register char*		t;
	Time_t			tm;

	NoP(s);
	NoP(h);
	if ((r->property & (P_attribute|P_parameter|P_state)) == P_parameter)
	{
		r->property |= P_ignore;
		maketop(r, 0L, NiL);
		for (p = scan(r, &tm); p; p = p->next)
			if (((r = p->rule)->property & (P_parameter|P_statevar)) == (P_parameter|P_statevar))
			{
				r->dynamic |= D_scanned;
				if (t = strchr(r->name, ')'))
				{
					*t = 0;
					code(r->name + 1);
					*t = ')';
				}
			}
	}
	return 0;
}

/*
 * check implicit state variable vars
 */

static int
checkvar1(register const char* s, char* u, void* h)
{
	register Var_t*		v = (Var_t*)u;
	register Rule_t*	r;

	NoP(h);
	if (v->property & V_scan)
	{
		state.fullscan = 1;
		r = staterule(VAR, NiL, (char*)s, 1);
		if (!r->scan)
		{
			debug((-5, "%s and %s force re-scan", v->name, r->name));
			r->scan = SCAN_STATE;
			state.forcescan = 1;
		}
		code(s);
	}
	return 0;
}

/*
 * check implicit state variable rules
 */

static int
checkvar2(const char* s, char* u, void* h)
{
	register Rule_t*	r = (Rule_t*)u;
	Var_t*			v;

	NoP(s);
	NoP(h);
	if ((r->property & P_statevar) && r->scan && !r->view && (!(v = varstate(r, 0)) || !(v->property & V_scan)) && (!(r->property & P_parameter) || !(r->dynamic & D_scanned)))
	{
		debug((-5, "%s forces re-scan", r->name));
		r->scan = 0;
		state.forcescan = 1;
	}
	return 0;
}

/*
 * freeze the parameter files and candidate state variables
 */

void
candidates(void)
{
	int	view;

	if (state.scan)
	{
		message((-2, "freeze candidate state variables and parameter files"));
		hashwalk(table.rule, 0, checkparam, NiL);
		hashwalk(table.var, 0, checkvar1, NiL);
		hashwalk(table.rule, 0, checkvar2, NiL);
		if (state.forcescan)
		{
			for (view = 1; view <= state.maxview; view++)
				stateview(0, NiL, NiL, NiL, view, 0, NiL);
			hashwalk(table.rule, 0, forcescan, NiL);
		}
	}
}

/*
 * save state for the next make
 */

void
savestate(void)
{
	char*	file;

	if (state.makefile && state.user && state.compile == COMPILED)
	{
		if (state.writestate)
		{
			if (state.finish)
				state.compile = SAVED;
			if (state.exec && state.savestate)
			{
				file = statefile();
				message((-2, "saving state in %s", file));
				state.stateview = 0;
				compile(file, NiL);
				state.stateview = -1;
				state.savestate = 0;
			}
		}
		if (state.finish)
			lockstate(0);
	}
}

/*
 * bind statevar r to a variable
 */

Rule_t*
bindstate(register Rule_t* r, register char* val)
{
	Rule_t*		s;
	Time_t		t;

#if DEBUG
	if (!(r->property & P_state))
		error(PANIC, "bindstate(%s) called for non-state rule", r->name);
#endif
	if (state.maxview && (r->property & P_statevar) && (s = staterule(VAR, r, NiL, 0)))
		r = s;
	if ((r->dynamic & D_bound) && !val)
		return r;
	if (r->property & P_statevar)
	{
		register Var_t*		v;
		char*			e;
		Sfio_t*			tmp = 0;

		/*
		 * determine the current state variable value
		 */

		if (val)
			r->property |= P_virtual;
		else if (r->property & P_virtual)
			val = null;
		else if (v = varstate(r, 0))
		{
			tmp = sfstropen();
			r->dynamic |= D_bound;
			expand(tmp, getval(v->name, VAL_PRIMARY));
			r->dynamic &= ~D_bound;
			val = sfstruse(tmp);
		}
		else if ((r->property & P_parameter) && r->statedata)
			val = r->statedata;
		else if (*(r->name + 1) == '-')
		{
			*(e = r->name + strlen(r->name) - 1) = 0;
			val = getval(r->name + 1, 0);
			*e = ')';
		}
		else
			val = null;
		if (!r->time && state.maxview && (state.view[0].flags & BIND_LOADED))
		{
			/*
			 * see if some other view has an initial value
			 */

			r->statedata = strdup(val);
			staterule(RULE, r, NiL, 0);
		}

		/*
		 * check if the state variable value changed
		 * the previous value, if defined, has already
		 * been placed in r->statedata by readstate()
		 */

		message((-2, "checking state variable %s value `%s'", r->name, r->statedata ? r->statedata : null));
		if (!r->time || r->statedata && !streq(r->statedata, val) || !r->statedata && *val)
		{
			/*
			 * state variable changed
			 */

			if (!r->view && !(r->property & P_accept))
			{
				if (r->time)
					reason((1, "state variable %s changed to `%s' from `%s'", r->name, val, r->statedata));
				else
					reason((1, "state variable %s initialized to `%s'", r->name, val));
			}
			state.savestate = 1;
			if (r->statedata != val)
				r->statedata = strdup(val);
			if (r->time == (t = CURTIME))
				t++;
			r->time = t;
		}
		if ((r->property & P_accept) || state.accept)
			r->time = OLDTIME;
		if (tmp)
			sfstrclose(tmp);
	}
	bindattribute(r);
	return r;
}

/*
 * check and stat built target r
 * otherwise check for motion from . to dir of r
 *
 * NOTE: this needs clarification
 */

static int
checkcurrent(register Rule_t* r, Stat_t* st)
{
	register int	n;
	register char*	s;
	long		pos;

	if (r->uname && !(n = rstat(r->uname, st, 1)))
		oldname(r);
	else if ((n = rstat(r->name, st, 1)) && (state.exec || state.mam.out && !state.mam.port))
	{
		rebind(r, -1);
		n = rstat(r->name, st, 1);
	}
	if (!n && !(r->dynamic & D_entries) && S_ISREG(st->st_mode))
		r->dynamic |= D_regular;
	if (!(r->dynamic & D_triggered))
		return n;
	edit(internal.nam, r->name, KEEP, DELETE, DELETE);
	if (!(pos = sfstrtell(internal.nam)))
		return n;
	sfputc(internal.nam, 0);
	sfputr(internal.nam, r->name, 0);
	s = sfstrseek(internal.nam, pos + 1, SEEK_SET);
	pathcanon(s, 0, 0);
	if (!streq(r->name, s))
	{
		if (!r->uname)
			r->uname = r->name;
		r->name = strdup(s);
	}
	s = sfstrseek(internal.nam, 0, SEEK_SET);
#if DEBUG
	if (state.test & 0x00000100)
		error(2, "statetime(%s): dir=%s n=%d time=[%s]", r->name, s, n, timestr(n ? NOTIME : tmxgetmtime(st)));
#endif
	newfile(r, s, n ? NOTIME : tmxgetmtime(st));
	return n;
}

/*
 * update internal time of r after its action has completed
 * sync>0 syncs the state rule prereqs and action
 * sync<0 resolves r but does not update state
 */

Time_t
statetime(register Rule_t* r, int sync)
{
	register Rule_t*	s;
	int			a;
	int			n;
	int			skip = 0;
	int			zerostate = 0;
	Time_t			t;
	Time_t			q;
	Rule_t*			x;
	Stat_t			st;
	Stat_t			ln;

	if (r->property & P_state)
	{
		if ((r->dynamic & D_triggered) && state.exec)
			r->time = ((r->property & P_statevar) && r->status == FAILED) ? (Time_t)0 : CURTIME;
		return r->time;
	}
	s = 0;
	if (state.interrupt && r->status != EXISTS)
		zerostate = 1;
	else if (r->status == FAILED)
	{
		r->time = 0;
		tmxsetmtime(&st, r->time);
		if ((state.test & 0x00040000) && (s = staterule(RULE, r, NiL, 0)))
		{
			r->time = s->time;
			if (r->property & (P_metarule|P_state))
				r->event = s->event;
			state.savestate = 1;
			skip = 1;
		}
	}
	else if (r->property & P_virtual)
	{
		r->time = CURTIME;
		tmxsetmtime(&st, r->time);
	}
	else if (checkcurrent(r, &st))
	{
		if (r->property & P_dontcare)
			t = 0;
		else
		{
			t = CURTIME;
			zerostate = 1;
		}
		tmxsetmtime(&st, t);
	}
	else if (sync < 0)
		return r->time;
	else if ((s = staterule(RULE, r, NiL, 1)) && s->time == tmxgetmtime(&st))
	{
		if (state.exec && !state.touch)
		{
			/*
			 * this alternate event time prevents the action from
			 * triggering next time if nothing else changes
			 */

			x = staterule(PREREQS, r, NiL, 1);
			x->dynamic &= ~D_lowres;
			x->time = r->time;
			r->time = s->event;
			if (r->dynamic & D_triggered)
				r->dynamic |= D_same;
		}
		state.savestate = 1;
	}
	else if ((r->dynamic & D_triggered) && state.exec)
	{
		static int	localsync;
		static int	localtest;
		static Time_t	localskew;

		/*
		 * r is built since its time changed after its action triggered
		 */

		s->dynamic |= D_built;
		if (x = staterule(PREREQS, r, NiL, 0))
			x->property |= P_force;

		/*
		 * check for file system and local system time consistency
		 * directories, archives and multi hard link files not sync'd
		 */

		if (st.st_nlink <= 1 && !S_ISDIR(st.st_mode) && !(r->property & P_archive))
		{

#if DEBUG
			if (state.test & 0x00000100)
				error(2, "%s: r[%s] s[%s] f[%s]", r->name, timestr(r->time), timestr(s->time), timestr(tmxgetmtime(&st)));
#endif
			if (!localsync && !state.override && r->time && r->time != OLDTIME && !(r->property & P_force) && tmxgetmtime(&st) == tmxgetctime(&st))
			{
				if (((n = (tmxsec(r->time) - (unsigned long)st.st_mtime - 1)) >= 0 || (n = (CURSECS - (unsigned long)st.st_mtime + 2)) <= 0) && (lstat(r->name, &ln) || !S_ISLNK(ln.st_mode)))
				{
					/*
					 * warn if difference not tolerable
					 */

					a = (n > 0) ? n : -n;
					if (a > 1)
						error(state.regress ? -1 : 1, "%s file system time %s local time by at least %s", r->name, n > 0 ? "lags" : "leads", fmtelapsed(a, 1));
					localsync = a > state.tolerance ? 1 : -1;
				}
			}
			if (localsync > 0)
			{
				/*
				 * NOTE: time stamp syncs work on the assumption that
				 *	 all source files have an mtime that is older
				 *	 than CURTIME -- this isn't too bad since
				 *	 only built files are sync'd; also note that
				 *	 nsec is set to 0 to avoid resolution mismatches
				 */

				for (;;)
				{
					t = tmxsns(CURSECS + tmxsec(localskew), 0);
					if (tmxtouch(r->name, TMX_NOTIME, t, TMX_NOTIME, 0))
					{
						error(ERROR_SYSTEM|1, "%s not sync'd to local time", r->name);
						break;
					}
					if (localtest)
					{
						tmxsetmtime(&st, t);
						break;
					}

					/*
					 * some systems try to fix up the local
					 * remote skew in the utime() call
					 * >> this never works <<
					 * members of the club include
					 *	darwin.ppc
					 *	netbsd.i386
					 */

					if (stat(r->name, &st))
					{
						error(ERROR_SYSTEM|1, "%s not found", r->name);
						break;
					}
					localtest = 1;
					q = tmxgetmtime(&st);
					if (tmxsec(q) == tmxsec(t))
						break;
					localskew = tmxsns(tmxsec(q)-tmxsec(t),0);
					error(state.regress ? -1 : 1, "the utime(2) or utimes(2) system call is botched for the filesystem containing %s (the current time is adjusted by %lu seconds) -- the state may be out of sync", r->name, tmxsec(localskew));

					/*
					 * the botch may only be for times near "now"
					 * localskew=1s handles this
					 */

					if (localskew > tmxsns(1,0))
					{
						t = CURTIME + tmxsns(1,0);
						t = tmxsns(tmxsec(t),0);
						if (!tmxtouch(r->name, TMX_NOTIME, t, TMX_NOTIME, 0) && !stat(r->name, &st) && tmxgetmtime(&st) == t)
							localskew = tmxsns(1,0);
					}
				}
			}
		}
	}
	if (!s)
		s = staterule(RULE, r, NiL, 1);
	if (sync)
	{
		s->dynamic |= D_built;
		s->attribute = r->attribute;
		s->action = r->action;
		if (s->prereqs != r->prereqs)
		{
			if ((r->property & (P_joint|P_target)) != (P_joint|P_target))
				freelist(s->prereqs);
			s->prereqs = r->prereqs;
		}
		state.savestate = 1;
	}
	if (!skip && (s->time != ((r->property & P_virtual) ? r->time : tmxgetmtime(&st)) || zerostate && s->time))
	{
		s->dynamic &= ~D_lowres;
		s->time = zerostate ? 0 : (r->property & P_virtual) ? r->time : tmxgetmtime(&st);
		s->event = CURTIME;
		state.savestate = 1;
	}
	return s->time;
}

/*
 * return 1 if rule r time matches state s time modulo
 * tolerance and low resolution time state
 */

int
statetimeq(Rule_t* r, Rule_t* s)
{
	long		d;

	static int	warned;

	if (r->time == s->time)
		return 1;
	if (state.tolerance || (s->dynamic & D_lowres))
	{
		if (!(d = tmxsec(r->time) - tmxsec(s->time)) && (s->dynamic & D_lowres))
		{
			s->dynamic &= ~D_lowres;
			s->time = r->time;
			state.savestate = 1;
			return 1;
		}
		if (d >= -state.tolerance && d <= state.tolerance)
			return 1;
	}
	else if (r->time < s->time && !(r->property & P_state) && tmxsec(r->time) == tmxsec(s->time) && !tmxnsec(r->time))
	{
		if (!warned)
		{
			warned = 1;
			if (state.warn)
				error(1, "file timestamp subsecond truncation");
		}
		return 1;
	}
	return 0;
}
