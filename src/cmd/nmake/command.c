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
 * make command execution routines
 */

#include "make.h"

#include <sig.h>

#if !_all_shells_the_same
#undef	EXIT_CODE
#define EXIT_CODE(x)	((x)&0177)
#endif

#define AFTER		0		/* done -- making after prereqs	*/
#define BEFORE		001		/* done -- before after prereqs	*/
#define BLOCKED		002		/* waiting for prereqs		*/
#define INTERMEDIATE	003		/* waiting for parent cancel	*/
#define READY		004		/* job ready to run		*/
#define RUNNING		005		/* job action sent to coshell	*/

#define PUSHED		010		/* currently push'd		*/
#define STATUS		(~PUSHED)	/* status mask			*/

#define MAMNAME(r)	((state.mam.dynamic||(r)!=state.frame->target||((r)->property&P_after))?mamname(r):(char*)0)

#if DEBUG

static char*	statusname[] =
{
	"AFTER", "BEFORE", "BLOCKED", "INTERMEDIATE", "READY", "RUNNING",
};

#define jobstatus()	do { if (state.test & 0x00001000) dumpjobs(2, JOB_status); else if (error_info.trace <= CMDTRACE) dumpjobs(CMDTRACE, JOB_status); } while (0)

#else

#define jobstatus()

#endif

struct Context_s; typedef struct Context_s Context_t;
struct Joblist_s; typedef struct Joblist_s Joblist_t;

struct Context_s			/* job target context		*/
{
	Context_t*	next;		/* for free list link		*/
	Frame_t*	frame;		/* active target frames		*/
	Frame_t*	last;		/* last active target frame	*/
	int		targetview;	/* state.targetview		*/
};

struct Joblist_s			/* job list cell		*/
{
	Joblist_t*	next;		/* next in list			*/
	Joblist_t*	prev;		/* prev in list			*/
	Cojob_t*	cojob;		/* coshell job info		*/
	Rule_t*		target;		/* target for job		*/
	List_t*		prereqs;	/* these must be done		*/
	Context_t* 	context;	/* job target context		*/
	char*		action;		/* unexpanded action		*/
	int		status;		/* job status			*/
	Flags_t		flags;		/* job flags			*/
};

typedef struct Jobstate_s		/* job state			*/
{
	Joblist_t*	firstjob;	/* first job			*/
	Joblist_t*	lastjob;	/* last job			*/
	Joblist_t*	freejob;	/* free jobs			*/
	Frame_t*	freeframe;	/* free target frames		*/
	Context_t*	freecontext;	/* free job context headers	*/
	int		intermediate;	/* # INTERMEDIATE jobs		*/
	Sfio_t*		tmp;		/* tmp stream			*/
	Rule_t*		triggered;	/* triggered but not yet a job	*/
} Jobstate_t;

static Jobstate_t	jobs;

/*
 * accept r as up to date
 */

static void
acceptrule(register Rule_t* r)
{
	if (r->property & P_state)
	{
		if (!state.silent)
			error(0, "touch %s", r->name);
		if (state.exec)
			state.savestate = 1;
		goto done;
	}
	if (r->property & P_archive)
	{
		if (state.exec)
		{
			artouch(r->name, NiL);
			statetime(r, 0);
		}
		if (!state.silent)
			error(0, "touch %s", r->name);
		goto done;
	}
	if (r->active && (r->active->parent->target->property & P_archive) && !(r->property & (P_after|P_before|P_use)))
	{
		char*	t;

		if (t = strrchr(r->name, '/'))
		{
			if (!r->uname)
				r->uname = r->name;
			r->name = maprule(t + 1, r);
		}
		if (state.exec)
			artouch(r->active->parent->target->name, r->name);
		else if (!state.silent)
			error(0, "touch %s/%s", r->active->parent->target->name, r->name);
		if (!(r->dynamic & D_regular))
			goto done;
	}
	if (!(r->property & P_virtual))
	{
		if (!state.silent)
			error(0, "touch %s", r->name);
		if (state.exec)
		{
			Stat_t		st;

			if (stat(r->name, &st) || tmxgetmtime(&st) < state.start && tmxtouch(r->name, (Time_t)0, (Time_t)0, (Time_t)0, 0))
				error(ERROR_SYSTEM|1, "cannot touch %s", r->name);
			statetime(r, 0);
		}
	}
 done:
	if (r->status != TOUCH)
		r->status = r->time ? EXISTS : (r->property & P_dontcare) ? IGNORE : FAILED;
}

/*
 * apply operator or action with attributes in r given lhs, rhs and job flags
 * blocks until action is complete
 */

int
apply(register Rule_t* r, char* lhs, char* rhs, char* act, Flags_t flags)
{
	register Rule_t*	x;
	int			oop;
	int			errors;
	Rule_t			lhs_rule;
	Rule_t			rhs_rule;
	List_t			lhs_prereqs;
	Frame_t			lhs_frame;
	Frame_t*		oframe;

	zero(lhs_rule);
	zero(rhs_rule);
	lhs_prereqs.rule = &rhs_rule;
	lhs_prereqs.next = 0;
	x = &lhs_rule;
	x->prereqs = &lhs_prereqs;
	zero(lhs_frame);
	lhs_frame.parent = state.frame;
	lhs_frame.target = x;
	x->active = &lhs_frame;
	x->property = r->property & (P_make|P_operator);
	if (x->property & P_operator)
	{
		x->action = act;
		act = r->action;
	}
	else
		x->action = r->action;
	x->name = lhs;
	x->statedata = r->name;
	x->time = state.start;
	rhs_rule.name = rhs;
	rhs_rule.time = x->time + 1;
	oframe = state.frame;
	state.frame = &lhs_frame;
	if ((r->property & (P_make|P_operator)) == (P_make|P_operator))
	{
		if (r->prereqs)
		{
			Time_t		t;
			char*		oaction;

			oaction = r->action;
			r->action = null;
			make(r, &t, NiL, 0);
			r->action = oaction;
		}
		oop = state.op;
		state.op = 1;
		errors = parse(NiL, act, r->name, NiL) == FAILED;
		state.op = oop;
	}
	else
	{
		r->status = UPDATE;
		trigger(r, NiL, act, flags);
		errors = complete(r, NiL, NiL, 0);
	}
	state.frame = oframe;
	return errors;
}

/*
 * apply() returning (temporary) CO_DATAFILE pointer to stdout of action
 */

Sfio_t*
fapply(Rule_t* r, char* lhs, char* rhs, char* act, Flags_t flags)
{
	Sfio_t*	fp;

	fp = 0;
	if (!apply(r, lhs, rhs, act, flags|CO_DATAFILE))
	{
		if (fp = sfopen(NiL, state.tmpfile, "re"))
			remove(state.tmpfile);
		else
			error(2, "%s: cannot read temporary data output file %s", r->name, state.tmpfile);
	}
	state.tmpfile = 0;
	return fp;
}

/*
 * call functional r with args
 * functional return value returned
 * 0 returned if not functional or empty return
 */

char*
call(register Rule_t* r, char* args)
{
	register Var_t*		v;

	if (r->property & P_functional)
	{
		maketop(r, 0, args);
		if ((v = getvar(r->name)) && *v->value)
			return v->value;
	}
	return 0;
}

/*
 * commit target/virtual directory
 */

static void
commit(Joblist_t* job, register char* s)
{
	register char*		t;
	register char*		v;
	register Rule_t*	r;
	Stat_t			st;

	if (t = strrchr(s, '/'))
	{
		*t = 0;
		if (r = bindfile(NiL, s, 0))
		{
			if (!r->view)
			{
				*t = '/';
				return;
			}
			if (*(v = r->name) != '/')
			{
				sfprintf(internal.nam, "%s/%s", state.view[r->view].root, v);
				v = sfstruse(internal.nam);
			}
			if (stat(v, &st))
				r = 0;
		}
		if (r || state.targetcontext && (!r || !r->time) && (st.st_mode = (S_IRWXU|S_IRWXG|S_IRWXO)) && tmxsetmtime(&st, state.start))
		{
			/*
			 * why not mkdir -p here?
			 */

			commit(job, s);
			if (((job->flags & CO_ALWAYS) || state.exec && state.touch) && (mkdir(s, st.st_mode & (S_IRWXU|S_IRWXG|S_IRWXO)) || stat(s, &st)))
				error(1, "%s: cannot create target directory %s", job->target->name, s);
			if (state.mam.out)
			{
				Sfio_t*	tmp = sfstropen();

				sfprintf(tmp, "mkdir %s", s);
				dumpaction(state.mam.out, MAMNAME(job->target), sfstruse(tmp), NiL);
				sfstrclose(tmp);
			}
			r = makerule(s);
			if (r->dynamic & D_alias)
				oldname(r);
			r->view = 0;
			r->time = tmxgetmtime(&st);
			if (r->dynamic & D_scanned)
				unbind(NiL, (char*)r, NiL);
		}
		*t = '/';
	}
}

/*
 * push job context
 */

static void
push(Joblist_t* job)
{
	register Context_t*	z;
	register Frame_t*	p;
	register Rule_t*	r;
	int			n;
	Time_t			tm;

	job->status |= PUSHED;
	if (z = job->context)
	{
		p = z->frame;
		for (;;)
		{
			if (!(r = getrule(p->context.name)))
				r = makerule(p->context.name);
			p->target = r;
			tm = r->time;
			r->time = p->context.time;
			p->context.time = tm;
			p->context.frame = r->active;
			r->active = p;
			if (p == p->parent)
				break;
			p = p->parent;
		}
		n = state.targetview;
		state.targetview = z->targetview;
		z->targetview = n;
		p = state.frame;
		state.frame = z->frame;
		z->frame = p;
	}
}

/*
 * pop job context
 */

static void
pop(Joblist_t* job)
{
	register Context_t*	z;
	register Frame_t*	p;
	register Rule_t*	r;
	int			n;
	Time_t			tm;

	if (z = job->context)
	{
		n = state.targetview;
		state.targetview = z->targetview;
		z->targetview = n;
		p = state.frame;
		state.frame = z->frame;
		z->frame = p;
		for (;;)
		{
			if (!(r = getrule(p->context.name)))
				r = makerule(p->context.name);
			r->active = p->context.frame;
			tm = r->time;
			r->time = p->context.time;
			p->context.time = tm;
			if (p == p->parent)
				break;
			p = p->parent;
		}
	}
	job->status &= ~PUSHED;
}

/*
 * discard (free) job context
 */

static void
discard(register Joblist_t* job)
{
	register Context_t*	z;
	register List_t*	p;

	if (job->flags & CO_SEMAPHORES)
		for (p = job->prereqs; p; p = p->next)
			if (p->rule->semaphore)
			{
				p->rule->semaphore++;
				p->rule->status = EXISTS;
			}
	if (job->flags & CO_PRIMARY)
	{
		job->prereqs->next = 0;
		freelist(job->prereqs);
	}
	if (z = job->context)
	{
		z->last->parent = jobs.freeframe;
		jobs.freeframe = z->frame;
		z->next = jobs.freecontext;
		jobs.freecontext = z;
		job->context = 0;
	}
	if (job->next)
		job->next->prev = job->prev;
	else
		jobs.lastjob = job->prev;
	if (job->prev)
		job->prev->next = job->next;
	else
		jobs.firstjob = job->next;
	job->next = jobs.freejob;
	jobs.freejob = job;
}

/*
 * save job context for later execute()
 */

static void
save(Joblist_t* job)
{
	register Frame_t*	o;
	register Frame_t*	p;
	register Frame_t*	x;
	Context_t*		z;

	if (job->action && !job->context)
	{
		if (z = jobs.freecontext)
			jobs.freecontext = jobs.freecontext->next;
		else
			z = newof(0, Context_t, 1, 0);
		z->targetview = state.targetview;
		o = state.frame;
		p = 0;
		for (;;)
		{
			if (x = jobs.freeframe)
				jobs.freeframe = jobs.freeframe->parent;
			else
				x = newof(0, Frame_t, 1, 0);
			if (p)
				p->parent = x;
			else
				z->frame = x;
			p = x;
			p->original = o->original;
			p->primary = o->primary;
			p->stem = o->stem ? strdup(o->stem) : 0;
			p->context.name = o->target->name;
			p->context.time = o->target->time;
			if (o->parent == o)
				break;
			o = o->parent;
		}
		z->last = p->parent = p;
		job->context = z;
	}
}

/*
 * restore action by expanding into buf using original context
 * coexec attributes placed in att
 */

static void
restore(register Joblist_t* job, Sfio_t* buf, Sfio_t* att)
{
	register char*	s;
	register char*	b;
	char*		u;
	char*		down;
	char*		back;
	char*		sep;
	int		downlen;
	int		localview;
	void*		pos;
	Var_t*		v;
	Sfio_t*		opt;
	Sfio_t*		tmp;
	Sfio_t*		context;

	push(job);
	localview = state.localview;
	state.localview = state.mam.statix && !state.expandview && state.user && !(job->flags & CO_ALWAYS);
	if ((job->flags & CO_LOCALSTACK) || (job->target->dynamic & D_hasscope))
	{
		register Rule_t*	r;
		register List_t*	p;

		job->flags |= CO_LOCALSTACK;
		pos = pushlocal();
		opt = sfstropen();
		if (job->target->dynamic & D_hasscope)
			for (p = job->prereqs; p; p = p->next)
				if ((r = p->rule)->dynamic & D_scope)
				{
					if (*r->name == '-')
						set(r->name, 1, opt);
					else
						parse(NiL, r->name, r->name, opt);
				}
				else if ((r->property & (P_make|P_local|P_use)) == (P_make|P_local) && r->action)
					parse(NiL, r->action, r->name, opt);
	}
	context = state.context;
	if (state.targetcontext && *(u = unbound(job->target)) != '/' && (s = strrchr(u, '/')))
	{
		size_t	n;
		int	c;

		tmp = sfstropen();
		downlen = s - u;
		*s = 0;
		sfprintf(tmp, "%s%c", u, 0);
		n = sfstrtell(tmp);
		c = '/';
		do
		{
			if (u = strchr(u, '/'))
				u++;
			else
				c = 0;
			sfputr(tmp, "..", c);
		} while (c);
		*s = '/';
		back = (down = sfstrbase(tmp)) + n;
		state.context = buf;
		buf = sfstropen();
		state.localview++;
	}
	else
		state.context = 0;
	if (job->action)
		expand(buf, job->action);
	if (state.context)
	{
		s = sfstruse(buf);
		sep = strchr(s, '\n') ? "\n" : "; ";
		sfprintf(state.context, "{ cd %s%s", down, sep);
		while (b = strchr(s, MARK_CONTEXT))
		{
			sfwrite(state.context, s, b - s);
			if (!(s = strchr(++b, MARK_CONTEXT)))
				error(PANIC, "unbalanced MARK_CONTEXT");
			*s++ = 0;
			if (*b == '/' || (u = getbound(b)) && *u == '/')
				sfputr(state.context, b, -1);
			else if (*b)
			{
				if (strneq(b, down, downlen))
					switch (*(b + downlen))
					{
					case 0:
						sfputc(state.context, '.');
						continue;
					case '/':
						sfputr(state.context, b + downlen + 1, -1);
						continue;
					}
				if (streq(b, "."))
					sfputr(state.context, back, -1);
				else if (isspace(*b))
					sfputr(state.context, b, -1);
				else
					sfprintf(state.context, "%s/%s", back, b);
			}
		}
		sfprintf(state.context, "%s%s}", s, sep);
		sfstrclose(tmp);
		sfstrclose(buf);
	}
	state.context = context;
	sfprintf(att, "label=%s", job->target->name);
	if ((v = getvar(CO_ENV_ATTRIBUTES)) && !(v->property & V_import))
		sfprintf(att, ",%s", v->value);
	if (job->flags & CO_LOCALSTACK)
	{
		poplocal(pos);
		if (*(s = sfstruse(opt)))
			set(s, 1, NiL);
		sfclose(opt);
	}
	state.localview = localview;
	pop(job);
}

static int	done(Joblist_t* job, int, Cojob_t*);

/*
 * send a job to the coshell for execution
 */

static void
execute(register Joblist_t* job)
{
	register List_t*	p;
	char*			s;
	char*			t;
	int			flags;
	Rule_t*			r;
	Var_t*			v;
	Sfio_t*			tmp;
	Sfio_t*			att;
	Sfio_t*			sp;

	att = sfstropen();
	tmp = sfstropen();
	restore(job, tmp, att);
	job->status = RUNNING;
	job->target->mark &= ~M_waiting;
	if (state.targetcontext || state.maxview && !state.fsview && *job->target->name != '/' && (!(job->target->dynamic & D_regular) || job->target->view))
		commit(job, job->target->name);
	if ((state.mam.dynamic || state.mam.regress) && state.user && !(job->target->property & (P_after|P_before|P_dontcare|P_make|P_state|P_virtual)))
		sfprintf(state.mam.out, "%sinit %s %s\n", state.mam.label, mamname(job->target), timefmt(NiL, CURTIME));
	t = sfstruse(tmp);
	if (!(job->flags & CO_ALWAYS))
	{
		if (state.touch)
		{
			if (state.virtualdot)
			{
				state.virtualdot = 0;
				lockstate(1);
			}
			if (!(job->target->property & (P_attribute|P_virtual)))
			{
				acceptrule(job->target);
				if ((job->target->property & (P_joint|P_target)) == (P_joint|P_target))
					for (p = job->target->prereqs->rule->prereqs; p; p = p->next)
						if (p->rule != job->target)
							acceptrule(p->rule);
			}
		}
		else if (*t && (!state.silent || state.mam.regress))
			dumpaction(state.mam.out ? state.mam.out : sfstdout, NiL, t, NiL);
		done(job, 0, NiL);
	}
	else
	{
		if (state.virtualdot && !notfile(job->target))
		{
			state.virtualdot = 0;
			lockstate(1);
		}
		if (!state.coshell)
		{
			sp = sfstropen();
			sfprintf(sp, "label=%s", idname);
			expand(sp, " $(" CO_ENV_OPTIONS ")");
			flags = CO_ANY;
			if (state.cross)
				flags |= CO_CROSS;
			if (state.serialize && state.jobs > 1)
				flags |= CO_SERIALIZE;
			if (!(state.coshell = coopen(getval(CO_ENV_SHELL, VAL_PRIMARY), flags, sfstruse(sp))))
				error(ERROR_SYSTEM|3, "coshell open error");
			sfstrclose(sp);
		}
		if (p = internal.exports->prereqs)
		{
			Sfio_t*	exp;

			exp = sfstropen();
			do
			{
				if (v = getvar(p->rule->name))
				{
					expand(exp, v->value);
					coexport(state.coshell, p->rule->name, sfstruse(exp));
				}
				else if (s = strchr(p->rule->name, '='))
				{
					*s = 0;
					expand(exp, s + 1);
					coexport(state.coshell, p->rule->name, sfstruse(exp));
					*s = '=';
				}
			} while (p = p->next);
			sfstrclose(exp);
#if 0
			freelist(internal.exports->prereqs);
#endif
			internal.exports->prereqs = 0;
		}
		if (job->flags & CO_DATAFILE)
		{
			static char*	dot;
			static char*	tmp;

			if (job->target->property & P_read)
			{
				if (!dot)
					dot = pathtemp(NiL, 0, null, idname, NiL);
				state.tmpfile = dot;
			}
			else
			{
				if (!tmp)
					tmp = pathtemp(NiL, 0, NiL, idname, NiL);
				state.tmpfile = tmp;
			}
		}
#if !_HUH_1992_02_29 /* i386 and ftx m68k dump without this statement -- help */
		message((-99, "execute: %s: t=0x%08x &t=0x%08x", job->target->name, t, &t));
#endif
		if (state.mam.out)
			dumpaction(state.mam.out, MAMNAME(job->target), t, NiL);
		if (r = getrule(external.makerun))
			maketop(r, P_dontcare|P_foreground, NiL);
		if (!(job->cojob = coexec(state.coshell, t, job->flags, state.tmpfile, NiL, sfstruse(att))))
			error(3, "%s: cannot send action to coshell", job->target->name);
		job->cojob->local = (void*)job;

		/*
		 * grab semaphores
		 */

		if (job->target->dynamic & D_hassemaphore)
		{
			job->flags |= CO_SEMAPHORES;
			for (p = job->prereqs; p; p = p->next)
				if (p->rule->semaphore && --p->rule->semaphore == 1)
					p->rule->status = MAKING;
		}

		/*
		 * check status and sync
		 */

		if (job->target->dynamic & D_hasafter)
			save(job);
		if (job->flags & (CO_DATAFILE|CO_FOREGROUND))
		{
			complete(job->target, NiL, NiL, 0);
			if (job->target->property & (P_functional|P_read))
			{
				if (sp = sfopen(NiL, state.tmpfile, "r"))
				{
					remove(state.tmpfile);
					if (job->target->property & P_read)
						parse(sp, NiL, job->target->name, NiL);
					else
					{
						char*	e;

						sfmove(sp, tmp, SF_UNBOUND, -1);
						t = sfstrbase(tmp);
						e = sfstrseek(tmp, 0, SEEK_CUR);
						while (e > t && *(e - 1) == '\n')
							e--;
						sfstrseek(tmp, e - t, SEEK_SET);
						setvar(job->target->name, sfstruse(tmp), 0);
					}
					sfclose(sp);
				}
				else
					error(2, "%s: cannot read temporary data output file %s", job->target->name, state.tmpfile);
				state.tmpfile = 0;
			}
		}
	}
	sfstrclose(att);
	sfstrclose(tmp);
}

/*
 * check if job for r with completed prereqs p can be cancelled
 */

static int
cancel(register Rule_t* r, register List_t* p)
{
	register Rule_t*	a;
	register Rule_t*	s;
	register Rule_t*	t;

	if (r->must)
	{
		s = staterule(RULE, r, NiL, 0);
		for (; p; p = p->next)
		{
			if ((a = p->rule)->dynamic & D_alias)
				a = makerule(a->name);
			if ((a->dynamic & D_same) && (!s || !(t = staterule(RULE, a, NiL, 0)) || s->event >= t->event))
				r->must--;
		}
		if (!r->must)
		{
			if (error_info.trace || state.explain)
				error(state.explain ? 0 : -1, "cancelling %s action", r->name);
			r->status = EXISTS;
			r->dynamic |= D_same;
			r->dynamic &= ~D_triggered;
			if (t = staterule(PREREQS, r, NiL, 0))
				t->time = CURTIME;
			return 1;
		}
	}
	return 0;
}

/*
 * all actions on behalf of job are done
 * with the possible exception of after prereqs
 * clear!=0 to clear job queue
 * 0 returned if further blocking required
 */

static int
done(register Joblist_t* job, int clear, Cojob_t* cojob)
{
	register List_t*	p;
	register Rule_t*	a;
	Time_t			tm;
	int			n;
	int			semaphore;
	Rule_t*			jammed;
	Rule_t*			waiting;

	if (clear && jobs.triggered && (((a = jobs.triggered)->property & P_state) || (a = staterule(RULE, a, NiL, 0))) && (a->property & P_force))
	{
		a->time = 0;
		state.savestate = 1;
	}
 another:
	jobstatus();
	if (job->status == INTERMEDIATE)
		state.intermediate--;
	else if (!clear && job->status == RUNNING && (job->target->dynamic & D_hasafter) && hasafter(job->target, (job->flags & CO_ERRORS) ? P_failure : P_after))
	{
		job->status = BEFORE;
		for (p = job->target->prereqs; p; p = p->next)
		{
			if ((a = p->rule)->dynamic & D_alias)
				a = makerule(a->name);
			if (a->status == MAKING && !a->semaphore || !(a->property & P_make) && a->status == UPDATE)
				return 0;
		}
	after:
#if __GNUC__ == 3 && ( sparc || _sparc || __sparc ) && !_HUH_2008_10_25 /* gcc code generation bug -- first hit on solaris -- not sure if for all gcc 3.* */
#ifndef __GNUC_MINOR__
#define __GNUC_MINOR__ 0
#endif
#ifndef __GNUC_PATCHLEVEL__
#define __GNUC_PATCHLEVEL__ 1
#endif
		if (!jobs.firstjob)
		{
			static int	warned = 0;

			if (!warned)
			{
				warned = 1;
				error(state.mam.regress || state.regress ? -1 : 1, "command.c:%d: gcc %d.%d.%d code generation bug workaround -- pass this on to the vendor", __LINE__, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
			}
			return !clear;
		}
#endif
		push(job);
		n = makeafter(job->target, (job->flags & CO_ERRORS) ? P_failure : P_after);
		pop(job);
		if (n)
			job->flags |= CO_ERRORS;
		else
		{
			job->flags &= ~CO_ERRORS;
			for (p = job->prereqs; p; p = p->next)
			{
				if ((a = p->rule)->dynamic & D_alias)
					a = makerule(a->name);
				if (!a->semaphore && a->status == MAKING)
				{
					job->status = AFTER;
					return !state.coshell || cojobs(state.coshell) < state.jobs;
				}
			}
		}
	}

	/*
	 * update rule times and status
	 */

	if (job->target->status != TOUCH)
		job->target->status = (job->flags & CO_ERRORS) ? ((job->target->property & P_dontcare) ? IGNORE : FAILED) : EXISTS;
	tm = statetime(job->target, 0);
	if (n = cojob && (state.mam.dynamic || state.mam.regress) && state.user && !(job->target->property & (P_after|P_before|P_dontcare|P_make|P_state|P_virtual)))
		sfprintf(state.mam.out, "%scode %s %d %s %s%s%s\n", state.mam.label, (job->target != state.frame->target || (job->target->property & P_after)) ? mamname(job->target) : "-", EXIT_CODE(cojob->status), timefmt(NiL, tm), timefmt(NiL, CURTIME), (job->target->dynamic & D_same) ? " same" : null, cojob->status && (job->flags & CO_IGNORE) ? " ignore" : null);
	if ((job->target->property & (P_joint|P_target)) == (P_joint|P_target))
		for (p = job->target->prereqs->rule->prereqs; p; p = p->next)
			if (p->rule != job->target)
			{
				if (p->rule->status != TOUCH)
					p->rule->status = (job->flags & CO_ERRORS) ? ((p->rule->property & P_dontcare) ? IGNORE : FAILED) : EXISTS;
				tm = statetime(p->rule, 0);
				if (n)
					sfprintf(state.mam.out, "%scode %s %d %s%s%s\n", state.mam.label, mamname(p->rule), EXIT_CODE(cojob->status), timefmt(NiL, tm), (p->rule->dynamic & D_same) ? " same" : null, cojob->status && (job->flags & CO_IGNORE) ? " ignore" : null);
			}

	/*
	 * update the job list
	 */

	discard(job);
 again:
	jammed = 0;
	if (job = jobs.firstjob)
		for (;;)
		{
			switch (job->status)
			{
			case AFTER:
			case BEFORE:
			case BLOCKED:
			case READY:
				n = READY;
				semaphore = 1;
				waiting = 0;
				for (p = job->prereqs; p; p = p->next)
				{
					if ((a = p->rule)->dynamic & D_alias)
						a = makerule(a->name);
					if ((a->property & P_after) && job->status != BEFORE && job->status != AFTER)
						continue;
					switch (a->status)
					{
					case FAILED:
						if (a->property & P_repeat)
							continue;
						job->flags |= CO_ERRORS;
						goto another;
					case MAKING:
						if (!jammed && (a->mark & M_waiting) && !(a->property & P_archive))
						{
							waiting = a;
							continue;
						}
						waiting = 0;
						n = BLOCKED;
						if (!a->semaphore)
							semaphore = 0;
						break;
					default:
						continue;
					}
					break;
				}
				if (waiting)
					jammed = waiting;
				else if (!clear && job->status == AFTER)
				{
					if (n == READY || semaphore)
						goto another;
				}
				else if (!clear && job->status == BEFORE)
				{
					if (n == READY || semaphore)
						goto after;
				}
				else if ((job->status = n) == READY)
				{
				unjam:
					if (clear || cancel(job->target, job->prereqs))
						goto another;
					if ((job->target->dynamic & D_intermediate) && job->target->must == 1)
					{
						job->status = INTERMEDIATE;
						jobs.intermediate++;
					}
					else if ((job->target->dynamic & (D_hasbefore|D_triggered)) == (D_hasbefore|D_triggered))
					{
						push(job);
						n = makebefore(job->target);
						pop(job);
						if (n)
						{
							job->flags |= CO_ERRORS;
							goto another;
						}
					}
					else if (!state.coshell || cojobs(state.coshell) < state.jobs)
					{
						execute(job);
						goto again;
					}
				}
				break;
			case RUNNING:
				if (clear && job->cojob && (job->cojob->flags & CO_SERVICE))
				{
					job->status = FAILED;
					job->flags |= CO_ERRORS;
					cokill(state.coshell, job->cojob, 0);
				}
				break;
			}
			if (!(job = job->next))
			{
				/*
				 * jammed is the first discovered member
				 * of a possible deadlock and we arbitrarily
				 * break it here
				 */

				if (jammed)
				{
					if (error_info.trace || state.explain)
						error(state.explain ? 0 : -1, "breaking possible job deadlock at %s", jammed->name);
					for (job = jobs.firstjob; job; job = job->next)
#if __GNUC__ >= 4 && !_HUH_2006_01_11 /* gcc code generation bug -- first hit on macos -- not sure if for all gcc 4.* */
#ifndef __GNUC_MINOR__
#define __GNUC_MINOR__ 0
#endif
#ifndef __GNUC_PATCHLEVEL__
#define __GNUC_PATCHLEVEL__ 1
#endif

						if (!job)
						{
							static int	warned = 0;

							if (!warned)
							{
								warned = 1;
								error(state.mam.regress || state.regress ? -1 : 1, "command.c:%d: gcc %d.%d.%d code generation bug workaround -- pass this on to the vendor", __LINE__, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
							}
							break;
						}
						else
#endif
						if (job->target == jammed)
						{
							if (job->status == AFTER)
								goto another;
							if (job->status != RUNNING)
							{
								jammed = 0;
								job->status = READY;
								state.jobs++;
								goto unjam;
							}
						}
				}
				break;
			}
		}
	return !clear;
}

/*
 * block until one job completes
 * update the job list
 * clear job list on any unexpected action error
 */

int
block(int check)
{
	register Cojob_t*	cojob;
	register Joblist_t*	job;
	Rule_t*			r;
	int			n;
	int			clear = 0;
	int			resume = 0;

	if (!state.coshell || !copending(state.coshell))
	{
		if (jobs.intermediate)
		{
			/*
			 * mark the jobs that must be generated
			 */

			n = 0;
			for (job = jobs.firstjob; job; job = job->next)
				if (job->target->must > ((unsigned int)(job->target->dynamic & D_intermediate) != 0))
				{
					n = 1;
					break;
				}
			if (n)
			{
				/*
				 * some intermediates must be generated
				 */


				error(2, "some intermediates must be generated");
			}
			else
			{
				/*
				 * accept missing intermediates
				 */

				while (job = jobs.firstjob)
				{
					if (error_info.trace || state.explain)
						error(state.explain ? 0 : -1, "cancelling %s action -- %s", job->target->name, job->status == INTERMEDIATE ? "intermediate not needed" : "missing intermediates accepted");
					job->target->status = EXISTS;
					discard(job);
				}
				jobs.intermediate = 0;
				return 1;
			}
		}
		return 0;
	}
	for (;;)
	{
		state.waiting = 1;
		if ((cojob = cowait(state.coshell, check ? (Cojob_t*)state.coshell : (Cojob_t*)0, -1)) && (job = (Joblist_t*)cojob->local))
			job->cojob = 0;
		if (trap())
		{
			if (state.interpreter)
				clear = resume = 1;
			if (!cojob)
				continue;
		}
		state.waiting = 0;
		if (!cojob)
		{
			if (check)
				return 0;
			break;
		}
		if (r = getrule(external.jobdone))
		{
			if (!jobs.tmp)
				jobs.tmp = sfstropen();
			sfprintf(jobs.tmp, "%s %d %s %s", job->target->name, cojob->status, fmtelapsed(cojob->user, CO_QUANT), fmtelapsed(cojob->sys, CO_QUANT));
			call(r, sfstruse(jobs.tmp));
		}
		if (cojob->status)
		{
			if (n = !EXITED_TERM(cojob->status) || EXIT_CODE(cojob->status))
			{
				if ((job->target->dynamic & D_hasafter) && hasafter(job->target, P_failure))
					n = 0;
				error(n ? 2 : state.explain ? 0 : -1, "%s%s code %d making %s%s", n ? "*** " : null, ERROR_translate(NiL, NiL, NiL, EXITED_TERM(cojob->status) ? "termination" : "exit"), EXIT_CODE(cojob->status), job->target->name, (job->flags & CO_IGNORE) ? ERROR_translate(NiL, NiL, NiL, " ignored") : null);
			}
			if (!(job->flags & CO_IGNORE))
			{
				job->flags |= CO_ERRORS;
				if (state.keepgoing || !n)
				{
					if (n)
						state.errors++;
					job->flags |= CO_KEEPGOING;
				}
			}
			if (state.interrupt || !(job->flags & (CO_IGNORE|CO_KEEPGOING)))
				clear = 1;
		}
		message((-3, "job: %s: interrupt=%d clear=%d status=%d flags=%08x", job->target->name, state.interrupt, clear, cojob->status, job->flags));

		/*
		 * job is done
		 */

		if (done(job, clear, cojob))
			return 1;
	}
	if (resume)
		longjmp(state.resume.label, 1);
	if (!state.finish)
	{
		if (!copending(state.coshell))
		{
			if (clear)
				finish(1);
		}
		else if (!state.interrupt)
			error(3, "lost contact with coshell");
	}
	return 0;
}

/*
 * wait until all actions for r and/or the rules in list p have completed
 * r==0 and p==0 waits for all actions
 * the number of FAILED actions is returned
 */

int
complete(register Rule_t* r, register List_t* p, Time_t* tm, Flags_t flags)
{
	register int	errors = 0;
	int		check = 0;
	int		recent;
	List_t		tmp;
	List_t*		q;
	Time_t		tprereqs;

	if (r)
	{
		tmp.next = p;
		p = &tmp;
		p->rule = r;
	}
	else
	{
		if (p && p->rule == internal.serialize)
		{
			p = p->next;
			check = 1;
		}
		if (!p)
		{
			while (block(check));
			return 0;
		}
	}
	for (tprereqs = 0; p; p = p->next)
	{
		if ((r = p->rule)->dynamic & D_alias)
			r = makerule(r->name);
		if (recent = r->status == MAKING)
		{
			message((-1, "waiting for %s%s", r->semaphore ? "semaphore " : null, r->name));
			r->mark |= M_waiting;
			do
			{
				if (!block(check))
				{
					if (recent = r->status == MAKING)
					{
						r->status = (r->property & P_dontcare) ? IGNORE : FAILED;
						error(1, "%s did not complete", r->name);
					}
					break;
				}
			} while (r->status == MAKING);
			r->mark &= ~M_waiting;
		}
		if (r->status == UPDATE && !(r->property & P_make) && !(flags & P_implicit))
			r->status = EXISTS;
		if (recent && (r->property & (P_joint|P_target)) == (P_joint|P_target))
		{
			register Rule_t*	x;
			register Rule_t*	s;

			s = staterule(RULE, r, NiL, 1);
			for (q = r->prereqs->rule->prereqs; q; q = q->next)
				if ((x = q->rule) != r)
				{
					if (x->status != r->status)
					{
						x->status = r->status;
						x->time = r->time;
					}
					if (s && (x = staterule(RULE, x, NiL, 1)))
					{
						x->dynamic |= D_built;
						x->action = s->action;
						x->prereqs = s->prereqs;
					}
				}
		}
		if (r->status == FAILED)
			errors++;
		if (!(r->property & (P_after|P_before|P_ignore)) && r->time > tprereqs)
			tprereqs = r->time;
	}
	if (tm)
		*tm = tprereqs;
	return errors;
}

/*
 * terminate all jobs
 */

void
terminate(void)
{
	if (state.coshell && cokill(state.coshell, NiL, SIGTERM))
		error(2, "coshell job kill error");
}

/*
 * complete all jobs and drop the coshell
 */

void
drop(void)
{
	if (state.coshell)
	{
		while (block(0));
		message((-1, "jobs %d user %s sys %s", state.coshell->total, fmtelapsed(state.coshell->user, CO_QUANT), fmtelapsed(state.coshell->sys, CO_QUANT)));
		coclose(state.coshell);
		state.coshell = 0;
	}
}

/*
 * serialize all pending prereqs
 */

static void
serial(Rule_t* r, register List_t* p)
{
	while (p && p->rule != internal.serialize && copending(state.coshell) > cojobs(state.coshell))
	{
		if (p->rule->prereqs)
			serial(NiL, p->rule->prereqs);
		complete(p->rule, NiL, NiL, 0);
		if (p->rule == r)
			break;
		p = p->next;
	}
}

/*
 * trigger action to build r
 * a contains the action attributes
 *
 * NOTE: the prereqs cons() may not be freed
 */

void
trigger(register Rule_t* r, Rule_t* a, char* action, Flags_t flags)
{
	register Joblist_t*	job;
	register List_t*	p;
	List_t*			prereqs;
	int			n;

	/*
	 * update flags
	 */

	if (!a)
		a = r;
	if (state.exec && !state.touch || (a->property & P_always) && (!state.never || (flags & CO_URGENT)))
		flags |= CO_ALWAYS;
	if ((a->property | r->property) & P_local)
		flags |= CO_LOCAL;
	if (!state.jobs || (r->property & P_foreground) || (r->property & (P_make|P_functional)) == P_functional || (r->dynamic & D_hasmake))
		flags |= CO_FOREGROUND|CO_LOCAL;
	if (state.keepgoing || state.unwind)
		flags |= CO_KEEPGOING;
	if (state.silent)
		flags |= CO_SILENT;
	if (state.ignore)
		flags |= CO_IGNORE;
	if (r->property & (P_functional|P_read))
		flags |= CO_DATAFILE;
	if (action)
	{
		message((-1, "triggering %s action%s%s", r->name, r == a ? null : " using ", r == a ? null : a->name));
		if (state.exec)
			jobs.triggered = r;
		r->dynamic |= D_triggered;
		if ((r->property & (P_joint|P_target)) == (P_joint|P_target))
			for (p = r->prereqs->rule->prereqs; p; p = p->next)
				p->rule->dynamic |= D_triggered;
		if (!*action)
			action = 0;
	}
	if (state.coshell && (action && !(r->property & P_make) || (flags & CO_FOREGROUND)))
	{
		/*
		 * the make thread blocks when too many jobs are outstanding
		 */

		n = (flags & CO_FOREGROUND) ? 0 : (state.jobs - 1);
		while ((cozombie(state.coshell) || cojobs(state.coshell) > n) && block(0));
		if ((flags & CO_FOREGROUND) && r->active && r->active->parent && r->active->parent->prereqs && copending(state.coshell) > cojobs(state.coshell))
			serial(r, r->active->parent->prereqs);
	}
	prereqs = r->prereqs;
	if (r->active && r->active->primary)
	{
		prereqs = cons(getrule(r->active->primary), prereqs);
		flags |= CO_PRIMARY;
	}
	if (r->property & P_make)
	{
		if (r->property & P_local)
		{
			r->status = EXISTS;
			return;
		}

		/*
		 * make actions are done immediately, bypassing the job queue
		 */

		if (prereqs && complete(NiL, prereqs, NiL, 0))
			r->status = (r->property & P_dontcare) ? IGNORE : FAILED;
		else
		{
			if (action && cancel(r, prereqs))
				r->status = EXISTS;
			else if ((r->dynamic & (D_hasbefore|D_triggered)) == (D_hasbefore|D_triggered) && (makebefore(r) || complete(NiL, prereqs, NiL, 0)))
				r->status = (r->property & P_dontcare) ? IGNORE : FAILED;
			else
			{
				if (r->property & P_functional)
					setvar(r->name, null, 0);
				if (action)
					switch (parse(NiL, action, r->name, NiL))
					{
					case EXISTS:
						if (!(r->property & (P_state|P_virtual)))
							statetime(r, 0);
						break;
					case FAILED:
						r->status = (r->property & P_dontcare) ? IGNORE : FAILED;
						break;
					case TOUCH:
						r->time = internal.internal->time;
						break;
					case UPDATE:
						if ((r->property & (P_state|P_virtual)) != (P_state|P_virtual))
							r->time = CURTIME;
						break;
					}
				if (r->status == UPDATE)
					r->status = EXISTS;
			}
		}
		if ((r->property & (P_joint|P_target)) == (P_joint|P_target))
			for (p = r->prereqs->rule->prereqs; p; p = p->next)
				if (p->rule != r)
				{
					p->rule->status = r->status;
					p->rule->time = r->time;
				}
		if ((r->dynamic & (D_hasafter|D_triggered)) == (D_hasafter|D_triggered))
		{
			if (r->status == FAILED)
			{
				if (hasafter(r, P_failure) && !makeafter(r, P_failure) && !complete(NiL, prereqs, NiL, 0))
					r->status = EXISTS;
			}
			else if (hasafter(r, P_after) && (makeafter(r, P_after) || complete(NiL, prereqs, NiL, 0)))
				r->status = (r->property & P_dontcare) ? IGNORE : FAILED;
		}
	}
	else
	{
		/*
		 * only one repeat action at a time
		 */

		if ((r->property & P_repeat) && (r->property & (P_before|P_after)) && !(r->dynamic & D_hassemaphore))
		{
			a = catrule(internal.semaphore->name, ".", r->name, 1);
			a->semaphore = 2;
			r->prereqs = append(r->prereqs, cons(a, NiL));
			r->dynamic |= D_hassemaphore;
		}

		/*
		 * check if any prerequisites are blocking execution
		 * FAILED prerequisites cause the target to fail too
		 */

		n = READY;
		for (;;)
		{
			for (p = prereqs; p; p = p->next)
			{
				if ((a = p->rule)->dynamic & D_alias)
					a = makerule(a->name);
				if (a->property & P_after)
					continue;
				switch (a->status)
				{
				case FAILED:
					if (a->property & P_repeat)
						continue;
					r->status = (r->property & P_dontcare) ? IGNORE : FAILED;
					if ((r->property & (P_joint|P_target)) == (P_joint|P_target))
						for (p = r->prereqs->rule->prereqs; p; p = p->next)
							if (p->rule != r)
								p->rule->status = (p->rule->property & P_dontcare) ? IGNORE : FAILED;
					return;
				case MAKING:
					if (a->active)
						error(1, "%s: prerequisite %s is active", r->name, a->name);
					else
						n = BLOCKED;
					break;
				}
			}
			if (n != READY)
				break;
			if (action)
			{
				if (cancel(r, prereqs))
					return;
				if ((r->dynamic & D_intermediate) && r->must == 1)
				{
					n = INTERMEDIATE;
					jobs.intermediate++;
					break;
				}
			}
			if ((r->dynamic & (D_hasbefore|D_triggered)) != (D_hasbefore|D_triggered))
				break;
			if (makebefore(r))
			{
				r->status = (r->property & P_dontcare) ? IGNORE : FAILED;
				if ((r->property & (P_joint|P_target)) == (P_joint|P_target))
					for (p = r->prereqs->rule->prereqs; p; p = p->next)
						if (p->rule != r)
							p->rule->status = (p->rule->property & P_dontcare) ? IGNORE : FAILED;
				return;
			}
		}
		if (action || n != READY)
		{
			/*
			 * allocate a job cell and add to job list
			 * the first READY job from the top is executed next
			 */

			if (job = jobs.freejob)
				jobs.freejob = jobs.freejob->next;
			else
				job = newof(0, Joblist_t, 1, 0);
			if (flags & CO_URGENT)
			{
				job->prev = 0;
				if (job->next = jobs.firstjob)
					jobs.firstjob->prev = job;
				else
					jobs.lastjob = job;
				jobs.firstjob = job;
			}
			else
			{
				job->next = 0;
				if (job->prev = jobs.lastjob)
					jobs.lastjob->next = job;
				else
					jobs.firstjob = job;
				jobs.lastjob = job;
			}

			/*
			 * fill in the info
			 */

			job->target = r;
			job->prereqs = prereqs;
			job->status = n;
			job->flags = flags;
			job->action = action;
			r->status = MAKING;
			if ((r->property & (P_joint|P_target)) == (P_joint|P_target))
				for (p = r->prereqs->rule->prereqs; p; p = p->next)
					if (p->rule != r)
						p->rule->status = r->status;
			if (n == READY)
			{
				execute(job);
				if (r->dynamic & D_hasafter)
					save(job);
			}
			else
				save(job);
			jobstatus();
		}
		else
		{
			if (r->status == UPDATE)
				r->status = EXISTS;
			if ((r->property & (P_joint|P_target)) == (P_joint|P_target))
				for (p = r->prereqs->rule->prereqs; p; p = p->next)
					if (p->rule->status == UPDATE)
						p->rule->status = EXISTS;
			if ((r->dynamic & (D_hasafter|D_triggered)) == (D_hasafter|D_triggered))
			{
				if (r->status == FAILED)
				{
					if (hasafter(r, P_failure) && !makeafter(r, P_failure) && !complete(NiL, prereqs, NiL, 0))
						r->status = EXISTS;
				}
				else if (hasafter(r, P_after) && (makeafter(r, P_after) || complete(NiL, prereqs, NiL, 0)))
					r->status = (r->property & P_dontcare) ? IGNORE : FAILED;
				if (r->status == EXISTS)
				{
					char*	t;
					Sfio_t*	tmp;

					tmp = sfstropen();
					edit(tmp, r->name, KEEP, DELETE, DELETE);
					if (*(t = sfstruse(tmp)))
						newfile(r, t, r->time);
					sfstrclose(tmp);
				}
			}
		}
		if (r->dynamic & D_triggered)
		{
			r->time = CURTIME;
			if ((r->property & (P_joint|P_target)) == (P_joint|P_target))
				for (p = r->prereqs->rule->prereqs; p; p = p->next)
					p->rule->time = r->time;
		}
	}
}

/*
 * resolve any cached info on file opened on fd
 */

int
resolve(char* file, int fd, int mode)
{
	return state.coshell ? cosync(state.coshell, file, fd, mode) : 0;
}

#if DEBUG
/*
 * dump the job table
 */

void
dumpjobs(int level, int op)
{
	register Joblist_t*	job;
	register List_t*	p;
	register Rule_t*	a;
	int			indent;
	int			line;

	if (state.coshell && error_info.trace <= level)
	{
		indent = error_info.indent;
		error_info.indent = 0;
		line = error_info.line;
		error_info.line = 0;
		switch (op)
		{
		case JOB_blocked:
			for (job = jobs.firstjob; job; job = job->next)
				if (job->status == BLOCKED)
				{
					sfprintf(sfstderr, "%8s %s :", statusname[job->status & STATUS], job->target->name);
					for (p = job->prereqs; p; p = p->next)
					{
						if ((a = p->rule)->dynamic & D_alias)
							a = makerule(a->name);
						if ((a->property & P_after) && job->status != BEFORE && job->status != AFTER)
							continue;
						if (a->status == MAKING)
							sfprintf(sfstderr, " %s", a->name);
					}
					sfprintf(sfstderr, "\n");
				}
				else if (job->status == READY)
					sfprintf(sfstderr, "%8s %s\n", statusname[job->status & STATUS], job->target->name);
			break;
		case JOB_status:
			sfprintf(sfstderr, "JOB  STATUS       TARGET  tot=%d pend=%d done=%d\n", state.coshell->total, copending(state.coshell), cozombie(state.coshell));
			for (job = jobs.firstjob; job; job = job->next)
			{
				if (job->cojob)
					sfsprintf(tmpname, MAXNAME, "[%d]%s", job->cojob->id, job->cojob->id > 99 ? null : job->cojob->id > 9 ? " " : "  ");
				else
					sfsprintf(tmpname, MAXNAME, "[-]  ");
				sfprintf(sfstderr, "%s %-13s%s\t%s%s%s\n"
					, tmpname
					, job->status == RUNNING && !job->cojob ? "DONE" : statusname[job->status & STATUS]
					, job->target->name
					, (job->target->must > ((unsigned int)(job->target->dynamic & D_intermediate) != 0)) ? " [must]" : null
					, job->context ? null : " [popped]"
					, (job->target->mark & M_waiting) ? " [waiting]" : null
					);
			}
			break;
		default:
			error(1, "%d: unknown op index", op);
			break;
		}
		error_info.indent = indent;
		error_info.line = line;
	}
}
#endif
