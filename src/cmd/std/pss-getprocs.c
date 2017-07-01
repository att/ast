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
 * process status stream PSS_METHOD_getprocs implementation
 */

#include "psslib.h"

#if PSS_METHOD != PSS_METHOD_getprocs

NoN(pss_getprocs)

#else

#include <procinfo.h>
#include <sys/proc.h>

#if _mem_pi_pri_procsinfo64
#define procsinfo		procsinfo64
#else
#undef	PSS_pri
#define PSS_pri			0
#endif

#undef	PSS_gid
#define PSS_gid			0
#undef	PSS_npid
#define PSS_npid		0
#undef	PSS_proc
#define PSS_proc		0
#undef	PSS_sched
#define PSS_sched		0
#undef	PSS_tgrp
#define PSS_tgrp		0

typedef struct State_s
{
	struct procsinfo	entry[64];
	struct procsinfo*	pr;
	int			last;
	int			index;
	int			count;
} State_t;

static int
getprocs_init(Pss_t* pss)
{
	register State_t*	state;

	if (!(state = vmnewof(pss->vm, 0, State_t, 1, 0)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	pss->data = state;
	return 1;
}

static int
getprocs_read(Pss_t* pss, Pss_id_t pid)
{
	register State_t*	state = (State_t*)pss->data;

	if (pid)
	{
		pss->pid = pid;
		state->index = 0;
		if ((state->count = getprocs(state->entry, sizeof(state->entry[0]), NiL, 0, &pss->pid, 1)) != 1)
			return -1;
		state->last = 0;
	}
	else
		while (state->index >= state->count)
		{
			if (state->last)
				return 0;
			state->index = 0;
			state->count = getprocs(state->entry, sizeof(state->entry[0]), NiL, 0, &pss->pid, elementsof(state->entry));
			if (state->count < elementsof(state->entry))
			{
				state->last = 1;
				if (state->count < 0)
					return -1;
				if (!state->count)
					return 0;
			}
			if (!state->entry[0].pi_pid)
				state->index++;
		}
	state->pr = state->entry + state->index++;
	return 1;
}

static int
getprocs_part(register Pss_t* pss, register Pssent_t* pe)
{
	register State_t*		state = (State_t*)pss->data;
	register struct procsinfo*	pr;

	pr = state->pr;
	pe->pid = state->pr->pi_pid;
	pe->pgrp = pr->pi_pgrp;
	pe->sid = pr->pi_sid;
	pe->tty = pr->pi_ttyp ? pr->pi_ttyd : PSS_NODEV;
	pe->uid = pr->pi_uid;
	switch (pr->pi_state)
	{
	case SACTIVE:
		pe->state = 'R';
		break;
	case SIDL:
		pe->state = 'I';
		break;
	case SSTOP:
		pe->state = 'T';
		break;
	case SSWAP:
		pe->state = 'W';
		break;
	case SZOMB:
		pe->state = 'Z';
		break;
	default:
		pe->state = 'O';
		break;
	}
	return 1;
}

static int
getprocs_full(register Pss_t* pss, register Pssent_t* pe)
{
	register State_t*		state = (State_t*)pss->data;
	register struct procsinfo*	pr = state->pr;
	unsigned long			fields = pss->disc->fields & pss->meth->fields;
	char*				s;
	int				i;

	if (pe->state != PSS_ZOMBIE)
	{
		if (fields & PSS_args)
		{
			s = pr->pi_comm;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
			pe->args = s;
		}
		if (fields & PSS_command)
		{
			s = pr->pi_comm;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
			pe->command = s;
		}
	}
	pe->addr = (void*)pr->pi_adspace;
	pe->flags = pr->pi_flags;
	pe->nice = pr->pi_nice;
	pe->ppid = pr->pi_ppid;
#if PSS_pri
	pe->pri = pr->pi_pri;
#endif
	pe->refcount = pr->pi_thcount;
	pe->rss = pr->pi_drss + pr->pi_trss;
	pe->size = pr->pi_size;
	pe->start = pr->pi_start;
	pe->time = pr->pi_ru.ru_utime.tv_sec + pr->pi_ru.ru_stime.tv_sec;
	return 1;
}

static Pssmeth_t getprocs_method =
{
	"getprocs",
	"[-version?@(#)$Id: pss getprocs (AT&T Research) 2004-02-29 $\n]"
	"[-author?Glenn Fowler <gsf@research.att.com>]",
	PSS_all,
	getprocs_init,
	getprocs_read,
	getprocs_part,
	getprocs_full,
	0,
	0,
	0,
	0
};

Pssmeth_t*	_pss_method = &getprocs_method;

#endif
