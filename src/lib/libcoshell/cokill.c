/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * if co==0 then kill all coshell jobs with sig
 * elif cj==0 then kill co jobs with sig
 * else kill cj with sig
 *
 * if sig==0 then cause all CO_SERVICE jobs to fail
 */

#include "colib.h"

/*
 * kill job cj in shell co with signal sig
 */

static int
cokilljob(register Coshell_t* co, register Cojob_t* cj, int sig)
{
	int	n;

	if (co->flags & CO_DEBUG)
		errormsg(state.lib, 2, "coshell %d kill co=%d cj=%d sig=%d", co->index, co->pid, cj->pid, sig);
	if (cj->pid < 0)
		return 0;
	if (cj->pid == 0)
	{
		if (cj->service)
			co->svc_running--;
		else
			co->running--;
		cj->pid = CO_PID_ZOMBIE;
		cj->status = EXIT_TERM(sig);
		return 0;
	}
	if (sig == SIGKILL)
	{
		co->running--;
		cj->pid = CO_PID_ZOMBIE;
		cj->status = EXIT_TERM(sig);
	}
	n = kill(cj->pid, sig);
	killpg(cj->pid, sig);
	return n;
}

/*
 * kill cj (or all jobs if cj==0) in shell co with sig
 */

static int
cokillshell(register Coshell_t* co, register Cojob_t* cj, int sig)
{
	int	n;

	if (sig && (co->flags & CO_SERVER))
	{
		char	buf[CO_BUFSIZ];

		n = sfsprintf(buf, sizeof(buf), "#%05d\nk %d %d\n", 0, cj ? cj->id : 0, sig);
		sfsprintf(buf, 7, "#%05d\n", n - 7);
		return write(co->cmdfd, buf, n) == n ? 0 : -1;
	}
	if (cj)
		return cokilljob(co, cj, sig);
	n = 0;
	for (cj = co->jobs; cj; cj = cj->next)
		if (cj->pid > 0)
			n |= cokilljob(co, cj, sig);
	return n;
}

int
cokill(register Coshell_t* co, register Cojob_t* cj, int sig)
{
	int	any;
	int	n;

	if (cj)
	{
		if (!co)
			co = cj->coshell;
		else if (co != cj->coshell)
			return -1;
		any = 0;
	}
	else if (co)
		any = 0;
	else if (!(co = state.coshells))
		return -1;
	else
		any = 1;
	if (co->flags & CO_DEBUG)
		errormsg(state.lib, 2, "coshell %d kill co=%d cj=%d sig=%d", co->index, co ? co->pid : 0, cj ? cj->pid : 0, sig);
	switch (sig)
	{
	case SIGINT:
		sig = SIGTERM;
		break;
#if defined(SIGSTOP) && defined(SIGTSTP)
	case SIGTSTP:
		sig = SIGSTOP;
		break;
#endif
	}
	n = 0;
	do
	{
		cowait(co, (Cojob_t*)co, 0);
		n |= cokillshell(co, cj, sig);
	} while (any && (co = co->next));
	return n;
}
