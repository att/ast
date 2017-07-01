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
 * process status stream PSS_METHOD_info implementation
 */

#include "psslib.h"

#if PSS_METHOD != PSS_METHOD_info

NoN(pss_info)

#else

#include <info.h>
#include <sys/../proc.h>	/* clash with ast <proc.h> */
#include <sys/inode.h>

typedef struct State_s
{
	int			mem;
	struct pentry*		pr;
	struct pentry*		pp;
	struct pentry*		pe;
	struct pentry		ps[1];
} State_t;

static int
info_init(Pss_t* pss)
{
	register State_t*	state;
	char*			mem;
	int			fd;
	unsigned long		n;
	unsigned long		a;

	mem = "/dev/mem";
	if ((fd = open(mem, O_RDONLY)) < 0)
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "%s: cannot read", mem);
		return -1;
	}
	n = info(_I_NPROCTAB);
	a = info(_I_PROCTAB);
	if (!(state = vmnewof(pss->vm, 0, State_t, 1, (n - 1) * sizeof(struct pentry))))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space");
		goto bad;
	}
	if (lseek(fd, a, SEEK_SET) != a)
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "%s: %lu: seek error", mem, a);
		goto bad;
	}
	if (read(fd, state->ps, n * sizeof(state->ps[0])) != n * sizeof(state->ps[0]))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "%s: %lu entry read error", mem, n);
		goto bad;
	}
	state->mem = fd;
	state->pp = state->ps;
	state->pe = state->ps + n;
	pss->data = state;
	return 1;
 bad:
	close(fd);
	free(state);
	return -1;
}

static int
info_read(Pss_t* pss, Pss_id_t pid)
{
	register State_t*	state = (State_t*)pss->data;
	int			count;

	if (pid)
	{
		for (state->pp = state->ps; state->pp < state->pe; state->pp++)
			if (state->pp->pstate != PRFREE && state->pp->pid == pid)
				break;
	}
	else
		while (state->pp < state->pe && state->pp->pstate == PRFREE)
			state->pp++;
	if (state->pp >= state->pe)
		return 0;
	state->pr = state->pp++;
	pss->pid = state->pr->pid;
	return 1;
}

static int
info_part(register Pss_t* pss, register Pssent_t* pe)
{
	State_t*		state = (State_t*)pss->data;
	register struct pentry*	pr = state->pr;

	pe->pid = pr->pid;
	pe->pgrp = pr->pgroup;
	pe->tty = pr->cdevnum == -1 ? PSS_NODEV : pr->cdevnum;
	pe->uid = pr->uid;
	switch ((int)(pr->pstate & 0xf))
	{
	case PRREADY:	pe->state = 'I'; break;
	case PRCURR:	pe->state = pr->pid == getpid() ? 'R' : 'S'; break;
	case PRWAIT:	pe->state = 'W'; break;
	case SUSPVAL:	pe->state = 'S'; break;
	case PRSTOP:	pe->state = 'T'; break;
	case ZOMBIEVAL:	pe->state = 'Z'; break;
	default:	pe->state = 'O'; break;
	}
	return 1;
}

static int
info_full(register Pss_t* pss, register Pssent_t* pe)
{
	register State_t*	state = (State_t*)pss->data;
	register struct pentry*	pr = state->pr;
	unsigned long		fields = pss->disc->fields & pss->meth->fields;
	char*			s;
	int			i;
	struct pssentry		px;
	struct st_entry		st;

	if (pe->state != PSS_ZOMBIE)
	{
		if ((fields & (PSS_sid|PSS_size|PSS_time)) &&
		    lseek(state->mem, (unsigned long)pr->pss, SEEK_SET) == (unsigned long)pr->pss &&
		    read(state->mem, &px, sizeof(px)) == sizeof(px))
		{
			pe->sid = px.sid;
			pe->size = (pr->stack_total + (px.brk - px.data_start)) / 1024;
			pe->time = (px.pss_utime + px.stime) / 5;
		}
		if ((fields & (PSS_pri)) &&
		    lseek(state->mem, (unsigned long)pr->i, SEEK_SET) == (unsigned long)pr->i &&
		    read(state->mem, &st, sizeof(st)) == sizeof(st))
		{
			pe->pri = st.pprio;
			pe->nice = st.rprio;
		}
		if (fields & PSS_command)
			pe->command = ((s = strrchr(pr->pname, '/')) && *++s) ? s : pr->pname;
		pe->args = pr->pname;
	}
	pe->flags = pr->pflag;
	pe->ppid = pr->ppid;
	pe->gid = pr->gid;
	return 1;
}

static int
info_done(register Pss_t* pss)
{
	register State_t*	state = (State_t*)pss->data;

	close(state->mem);
	free(state);
	return 0;
}

static Pssmeth_t info_method =
{
	"info",
	"[-version?@(#)$Id: pss info (AT&T Research) 2005-02-11 $\n]"
	"[-author?Glenn Fowler <gsf@research.att.com>]",
	PSS_args|PSS_command|PSS_flags|PSS_gid|PSS_nice|PSS_pgrp|PSS_pid|PSS_ppid|PSS_pri|PSS_sid|PSS_size|PSS_state|PSS_time|PSS_tty|PSS_uid,
	info_init,
	info_read,
	info_part,
	info_full,
	0,
	0,
	0,
	info_done
};

Pssmeth_t*	_pss_method = &info_method;

#endif
