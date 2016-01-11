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
 * make signal traps
 */

#include "make.h"

#include <sig.h>

#undef	trap

#if defined(SIGIOT) && SIGIOT == SIGEMT
#undef	SIGIOT
#endif

struct Alarms_s; typedef struct Alarms_s Alarms_t;

struct Alarms_s
{
	Alarms_t*	next;
	Rule_t*		rule;
	Seconds_t	time;
};

static int	signals[] =		/* signals to catch		*/
{
	SIGHUP,
	SIGINT,
	SIGALRM,
	SIGTERM,
	SIGPIPE,
#ifdef SIGILL
	SIGILL,
#endif
#ifdef SIGIOT
	SIGIOT,
#endif
#ifdef SIGEMT
	SIGEMT,
#endif
#if !DEBUG
	SIGQUIT,
#ifdef SIGBUS
	SIGBUS,
#endif
#ifdef SIGSEGV
	SIGSEGV,
#endif
#endif
#ifdef SIGUSR1
	SIGUSR1,
#endif
#ifdef SIGUSR2
	SIGUSR2,
#endif
};

static struct Trap_s
{
	int*		caught;		/* caught signals		*/
	Alarms_t*	alarms;		/* sorted alarm list		*/
	Alarms_t*	freealarms;	/* free alarm list		*/
} trap;

/*
 * catch and mark interrupts
 */

static void
interrupt(register int sig)
{
	signal(sig, sig == SIGPIPE ? SIG_IGN : interrupt);
	trap.caught[sig] = 1;
	state.caught = 1;
}

/*
 * (re)initialize wakeup rule pointers
 */

void
initwakeup(int repeat)
{
	register Alarms_t*	a;

	NoP(repeat);
	for (a = trap.alarms; a; a = a->next)
		a->rule = makerule(a->rule->name);
}

/*
 * set next wakeup time
 */

static void
setwakeup(void)
{
	register Seconds_t	t;
	register Seconds_t	now;
	int			level;

	now = CURSECS;
	if (!trap.alarms)
		t = 0;
	else if (trap.alarms->time <= now)
		t = 1;
	else
		t = trap.alarms->time - now;
	alarm(t);
	sfsprintf(tmpname, MAXNAME, "%lu", t ? (now + t) : t);
	setvar(internal.alarm->name, fmtelapsed(t, 1), 0);
	if (error_info.trace <= (level = (state.test & 0x00010000) ? 2 : CMDTRACE))
	{
		register Alarms_t*	a;

		if (a = trap.alarms)
		{
			error(level, "ALARM  TIME                 RULE");
			do
			{
				error(level, "%6s %s %s", fmtelapsed((a->time >= now) ? (a->time - now) : 0, 1), timestr(tmxsns(a->time, 0)), a->rule->name);
			} while (a = a->next);
		}
		else
			error(level, "ALARM -- NONE");
	}
}

/*
 * wakeup in t seconds and make rules in p
 * t==0 to drop alarms for list p, all for p==0
 */

void
wakeup(Seconds_t t, register List_t* p)
{
	register Alarms_t*	a;
	register Alarms_t*	z;
	register Alarms_t*	x;
	Alarms_t*		alarms;
	Seconds_t		now;

	alarms = trap.alarms;
	now = CURSECS;
	if (t)
	{
		t += now;
		if (!p)
			p = cons(catrule(external.interrupt, ".", fmtsignal(-SIGALRM), 1), NiL);
	}
	if (p)
	{
		do
		{
			x = 0;
			for (z = 0, a = trap.alarms; a; z = a, a = a->next)
				if (a->rule == p->rule)
				{
					x = a;
					if (z)
						z->next = a->next;
					else
						trap.alarms = a->next;
					break;
				}
			if (t)
			{
				if (!x)
				{
					if (x = trap.freealarms)
						trap.freealarms = trap.freealarms->next;
					else
						x = newof(0, Alarms_t, 1, 0);
					x->rule = p->rule;
				}
				x->time = t;
				x->next = 0;
				for (z = 0, a = trap.alarms; a; z = a, a = a->next)
					if (t <= a->time)
					{
						x->next = a;
						break;
					}
				if (z)
					z->next = x;
				else
					trap.alarms = x;
			}
		} while (p = p->next);
	}
	else if (a = trap.alarms)
	{
		trap.alarms = 0;
		while (x = a->next)
			a = x;
		a->next = trap.freealarms;
		trap.freealarms = a;
	}
	if (trap.alarms != alarms)
		setwakeup();
}

/*
 * initialize the traps
 */

void
inittrap(void)
{
	register int	i;

	memfatal(NiL);
	trap.caught = newof(0, int, sig_info.sigmax + 1, 0);
	for (i = 0; i < elementsof(signals); i++)
		if (signal(signals[i], interrupt) == SIG_IGN)
			signal(signals[i], SIG_IGN);
}

/*
 * handle interrupts
 * called by trap() in safe regions
 * 0 returned if no interrupts to handle
 */

int
handle(void)
{
	register int		sig;
	register Rule_t*	r;
	register Alarms_t*	a;
	char*			s;
	char*			w;
	Var_t*			v;
	Seconds_t		t;

	if (!state.caught)
		return 0;
	while (state.caught)
	{
		state.caught = 0;
		for (sig = 1; sig <= sig_info.sigmax; sig++)
			if (trap.caught[sig])
			{
				trap.caught[sig] = 0;

				/*
				 * flush the output streams
				 */

				sfsync(sfstderr);
				sfsync(sfstdout);

				/*
				 * continue if already in finish
				 */

				if (state.finish)
				{
					if (!state.interrupt)
						state.interrupt = sig;
					for (sig = 1; sig <= sig_info.sigmax; sig++)
						trap.caught[sig] = 0;
					return 0;
				}

				/*
				 * check user trap (some cannot be trapped)
				 */

				w = 0;
				if (!state.compileonly)
					switch (sig)
					{
					case SIGALRM:
						s = fmtsignal(-sig);
						t = CURSECS;
						while ((a = trap.alarms) && a->time <= t)
						{
							trap.alarms = a->next;
							r = a->rule;
							a->next = trap.freealarms;
							trap.freealarms = a;
							maketop(r, (P_dontcare|P_force|P_ignore|P_repeat)|((r->property & P_make)?0:P_foreground), s);
						}
						setwakeup();
						continue;
					default:
						s = fmtsignal(-sig);
						if ((r = catrule(external.interrupt, ".", s, 0)) || (r = getrule(external.interrupt)))
						{
							if (!(r->property & P_functional))
								v = setvar(external.interrupt, s, 0);
							maketop(r, (P_dontcare|P_force|P_ignore|P_repeat)|((r->property & P_make)?0:P_foreground), s);
							if (r->property & P_functional)
								v = getvar(r->name);
							w = v->value;
							if (r->status == EXISTS && (streq(w, s) || streq(w, "continue")))
							{
								message((-1, "trap %s handler %s status CONTINUE return %s", s, r->name, w));
								continue;
							}
							message((-1, "trap %s handler %s status TERMINATE return %s", s, r->name, w));
						}
						/*FALLTHROUGH*/
#ifdef SIGILL
					case SIGILL:
#endif
#ifdef SIGIOT
					case SIGIOT:
#endif
#ifdef SIGEMT
					case SIGEMT:
#endif
#ifdef SIGBUS
					case SIGBUS:
#endif
#ifdef SIGSEGV
					case SIGSEGV:
#endif
						break;
					}

				/*
				 * terminate outstanding jobs
				 */

				terminate();

				/*
				 * the interpreter resumes without exit
				 */

				if (state.interpreter)
				{
					if (state.waiting)
						return 1;
					longjmp(state.resume.label, 1);
				}

				/*
				 * if external.interrupt=""|"exit" then exit
				 * otherwise terminate via original signal
				 */

				if (w && (!*w || streq(w, "exit")))
					state.interrupt = 0;
				else if (!state.interrupt)
					state.interrupt = sig;
				finish(3);

				/*
				 * shouldn't get here
				 */

				exit(3);
			}
	}
	return 1;
}
