/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
 * process status stream PSS_METHOD_ps implementation
 */

#include "psslib.h"

#if !defined(PSS_ps) || !defined(PSS_pso)

Pssmeth_t*	_pss_ps = 0;

#else

#include <tm.h>

#define PSS_default	(PSS_pgrp|PSS_pid|PSS_ppid|PSS_sid|PSS_state|PSS_tgrp|PSS_tty|PSS_uid)

typedef struct State_s
{
	Sfio_t*			ps;
	unsigned long		fields;
	unsigned long		now;
	int			scan;
	int			debug;
} State_t;

typedef struct Pso_s
{
	unsigned long		field;
	const char*		name;
} Pso_t;

static Pso_t	pso[] =
{
	PSS_pso
};

static int
ps_init(Pss_t* pss)
{
	register State_t*	state;
	register Pso_t*		po;
	register unsigned long	fields;
	char*			s;

	if (!(state = vmnewof(pss->vm, 0, State_t, 1, 0)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	state->now = (unsigned long)time(NiL) + 60;
	if ((s = getenv("_AST_PSS_ps")) && *s == 'd')
		state->debug = 1;
	fields = 0;
	for (po = pso; po->field; po++)
		fields |= po->field;
	pss->meth->fields = state->fields = fields;
	pss->data = state;
	return 1;
}

static int
ps_done(Pss_t* pss)
{
	register State_t*	state = (State_t*)pss->data;

	if (state->ps)
		sfclose(state->ps);
	return 1;
}

static int
ps_read(Pss_t* pss, Pss_id_t pid)
{
	register State_t*	state = (State_t*)pss->data;
	register Pso_t*		po;
	register char*		s;
	register char*		e;
	register unsigned long	fields;
	int			sep;

	if (pid || !state->scan)
	{
		if (state->ps)
		{
			sfclose(state->ps);
			state->ps = 0;
		}
		s = pss->buf;
		e = s + sizeof(pss->buf);
		s += sfsprintf(s, e - s, "%s", PSS_ps);
		if (!pid)
		{
			if (pss->disc->flags & (PSS_ALL|PSS_UNMATCHED))
				s += sfsprintf(s, e - s, " %s", PSS_ps_every);
			else
				switch (pss->disc->flags & (PSS_ATTACHED|PSS_DETACHED|PSS_LEADER))
				{
				case PSS_ATTACHED|PSS_DETACHED|PSS_LEADER:
					s += sfsprintf(s, e - s, " %s", PSS_ps_every);
					break;
				case PSS_ATTACHED|PSS_DETACHED:
					s += sfsprintf(s, e - s, " %s", PSS_ps_detached);
					break;
				case PSS_DETACHED|PSS_LEADER:
					s += sfsprintf(s, e - s, " %s", PSS_ps_noleader);
					break;
				case PSS_ATTACHED|PSS_LEADER:
				case PSS_ATTACHED:
					s += sfsprintf(s, e - s, " %s", PSS_ps_all);
					break;
				}
		}
		s += sfsprintf(s, e - s, " -o");
		fields = pss->disc->fields|PSS_default;
		sep = ' ';
		for (po = pso; po->field; po++)
			if (po->field & fields)
			{
				s += sfsprintf(s, e - s, "%c%s", sep, po->name);
				sep = ',';
			}
		if (pid)
		{
			state->scan = 0;
			s += sfsprintf(s, e - s, " -p %lu", (unsigned long)pid);
		}
		else
			state->scan = 1;
		if ((pss->disc->flags & PSS_VERBOSE) && pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, 0, "%s", pss->buf);
		if (!(state->ps = sfpopen(NiL, pss->buf, "r")) || !sfgetr(state->ps, '\n', 0))
			return -1;
	}
	return 1;
}

static unsigned long
number(char* s, char** p, int base)
{
	unsigned long	n;
	char*		e;

	for (;;)
	{
		n = strtoul(s, &e, base);
		if (*e && !isspace(*e))
			switch (base)
			{
			case 8:
				if (*e == '8' || *e == '9')
				{
					base = 10;
					continue;
				}
				/*FALLTHROUGH*/
			case 10:
				if (isxdigit(*e))
				{
					base = 16;
					continue;
				}
				break;
			}
		break;
	}
	while (*e && !isspace(*e))
		e++;
	*p = e;
	return n;
}

static int
ps_part(register Pss_t* pss, register Pssent_t* pe)
{
	State_t*		state = (State_t*)pss->data;
	register Pso_t*		po;
	register unsigned long	fields;
	register char*		s;
	char*			e;
	char*			t;
	int			c;

	if (!(s = sfgetr(state->ps, '\n', 1)))
		return -1;
	memset(pe, sizeof(*pe), 0);
	fields = pss->disc->fields|PSS_default;
	for (po = pso; po->field; po++)
		if (po->field & fields)
		{
			while (isspace(*s))
				s++;
			if (!*s)
				break;
			if (state->debug && pss->disc->errorf)
				(*pss->disc->errorf)(pss, pss->disc, 2, "%s: %s", po->name, s);
			switch (po->field)
			{
			case PSS_addr:
				pe->addr = (void*)number(s, &e, 16);
				break;
			case PSS_args:
				pe->args = e = s;
				return 1;
			case PSS_sched:
				pe->sched = e = s;
				break;
			case PSS_command:
				pe->command = (e = strrchr(s, '/')) ? (e + 1) : s;
				e = s;
				break;
			case PSS_cpu:
				pe->cpu = number(s, &e, 10);
				break;
			case PSS_flags:
				pe->flags = number(s, &e, 8);
				break;
			case PSS_gid:
				for (e = s; *e; e++)
					if (isspace(*e))
					{
						*e++ = 0;
						break;
					}
				pe->gid = strgid(s);
				break;
			case PSS_job:
				pe->job = number(s, &e, 10);
				break;
			case PSS_nice:
				pe->nice = number(s, &e, 10);
				break;
			case PSS_npid:
				pe->npid = number(s, &e, 10);
				break;
			case PSS_pgrp:
				pe->pgrp = number(s, &e, 10);
				break;
			case PSS_pid:
				pe->pid = number(s, &e, 10);
				break;
			case PSS_ppid:
				pe->ppid = number(s, &e, 10);
				break;
			case PSS_pri:
				pe->pri = number(s, &e, 10);
				break;
			case PSS_proc:
				pe->proc = number(s, &e, 10);
				break;
			case PSS_refcount:
				pe->refcount = number(s, &e, 10);
				break;
			case PSS_rss:
				pe->rss = number(s, &e, 10);
				break;
			case PSS_sid:
				pe->sid = number(s, &e, 10);
				break;
			case PSS_size:
				pe->size = number(s, &e, 10);
				break;
			case PSS_start:
				c = 0;
				for (t = s; *s; s++)
					if (*s == '_' || isalpha(*s) && isdigit(*(s + 1)) || isdigit(*s) && isalpha(*(s + 1)))
						c = 1;
					else if (isspace(*s) && (c || !isdigit(*(s + 1))))
						break;
				c = *s;
				*s = 0;
				pe->start = tmdate(t, &e, NiL);
				if ((unsigned long)pe->start > state->now)
				{
					sfsprintf(pss->buf, sizeof(pss->buf), "last %s", t);
					pe->start = tmdate(pss->buf, NiL, NiL);
				}
				*s = c;
				if (e > s)
					e = s;
				break;
			case PSS_state:
				if ((pe->state = *(e = s)) == PSS_ZOMBIE)
					*e = 0;
				break;
			case PSS_tgrp:
				pe->tgrp = number(s, &e, 10);
				break;
			case PSS_time:
				pe->time = strelapsed(s, &e, 1);
				break;
			case PSS_tty:
				for (e = s; *e; e++)
					if (isspace(*e))
					{
						*e++ = 0;
						break;
					}
				pe->tty = pssttydev(pss, s);
				break;
			case PSS_uid:
				for (e = s; *e; e++)
					if (isspace(*e))
					{
						*e++ = 0;
						break;
					}
				pe->uid = struid(s);
				break;
			case PSS_wchan:
				pe->wchan = (void*)number(s, &e, 16);
				break;
			}
			if (e == s)
				while (*s && !isspace(*s))
					s++;
			else
				s = e;
			if (isspace(*s))
				*s++ = 0;
		}
	return 1;
}

static Pssmeth_t ps_method =
{
	"ps",
	"[-version?@(#)$Id: pss ps (AT&T Research) 2003-02-01 $\n]"
	"[-author?Glenn Fowler <gsf@research.att.com>]",
	PSS_all,
	ps_init,
	ps_read,
	ps_part,
	0,
	0,
	0,
	0,
	ps_done
};

Pssmeth_t*	_pss_ps = &ps_method;

#endif
