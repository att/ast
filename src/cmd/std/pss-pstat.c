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
 * process status stream PSS_METHOD_pstat implementation
 */

#include "psslib.h"

#if PSS_METHOD != PSS_METHOD_pstat

NoN(pss_pstat)

#else

#define _PSTAT64		1

#if _sys_param
#include <sys/param.h>
#endif
#if _sys_pstat
#include <sys/pstat.h>
#endif

typedef struct State_s
{
	struct pst_status	pst[256];
	struct pst_status*	pr;
	struct pst_status*	pp;
	struct pst_status*	pe;
	int			last;
} State_t;

static int
pstat_init(Pss_t* pss)
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
pstat_read(Pss_t* pss, Pss_id_t pid)
{
	register State_t*	state = (State_t*)pss->data;
	int			count;

	if (pid)
	{
		if ((count = pstat_getproc(state->pst, sizeof(state->pst[0]), 0, pid)) != 1)
			return -1;
		state->pp = state->pst;
		state->pe = state->pp + count;
		state->last = 0;
	}
	else if (state->pp >= state->pe)
	{
		count = state->pe ? (state->pe-1)->pst_idx + 1 : 0;
		if ((count = pstat_getproc(state->pst, sizeof(state->pst[0]), elementsof(state->pst), count)) < 0)
			return -1;
		if (!count)
			return 0;
		state->pp = state->pst;
		state->pe = state->pp + count;
		state->last = 0;
	}
	state->pr = state->pp++;
	pss->pid = state->pr->pst_pid;
	return 1;
}

static int
pstat_part(register Pss_t* pss, register Pssent_t* pe)
{
	State_t*			state = (State_t*)pss->data;
	register struct pst_status*	pr = state->pr;

	pe->pid = pr->pst_pid;
	pe->pgrp = pr->pst_pgrp;
	pe->tty = pr->pst_term.psd_major == -1 && pr->pst_term.psd_minor == -1 ? PSS_NODEV : makedev(pr->pst_term.psd_major, pr->pst_term.psd_minor);
	pe->uid = pr->pst_uid;
	pe->sid = pr->pst_sid;
	switch ((int)pr->pst_stat)
	{
	case PS_IDLE:	pe->state = 'I'; break;
	case PS_RUN:	pe->state = 'R'; break;
	case PS_SLEEP:	pe->state = 'S'; break;
	case PS_STOP:	pe->state = 'T'; break;
	case PS_ZOMBIE:	pe->state = 'Z'; break;
	default:	pe->state = 'O'; break;
	}
	return 1;
}

static int
pstat_full(register Pss_t* pss, register Pssent_t* pe)
{
	register State_t*		state = (State_t*)pss->data;
	register struct pst_status*	pr = state->pr;
	unsigned long			fields = pss->disc->fields & pss->meth->fields;
	char*				s;
	int				i;

	if (pe->state != PSS_ZOMBIE)
	{
		if (fields & PSS_args)
		{
			s = pr->pst_cmd;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
			pe->args = s;
		}
		if (fields & PSS_command)
		{
			s = pr->pst_cmd;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
			pe->command = s;
		}
	}
	pe->addr = (void*)pr->pst_addr;
	pe->wchan = (void*)pr->pst_wchan;
	pe->flags = pr->pst_flag;
	pe->nice = pr->pst_nice;
	pe->ppid = pr->pst_ppid;
	pe->pri = pr->pst_pri;
	pe->rss = pr->pst_rssize;
	pe->size = pr->pst_tsize + pr->pst_dsize + pr->pst_ssize;
	pe->start = pr->pst_start;
	pe->time = pr->pst_utime + pr->pst_stime;
	return 1;
}

static Pssmeth_t pstat_method =
{
	"pstat",
	"[-version?@(#)$Id: pss pstat (AT&T Research) 2003-02-01 $\n]"
	"[-author?Glenn Fowler <gsf@research.att.com>]",
	PSS_all,
	pstat_init,
	pstat_read,
	pstat_part,
	pstat_full,
	0,
	0,
	0,
	0
};

Pssmeth_t*	_pss_method = &pstat_method;

#endif
