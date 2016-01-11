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
 * close a coshell
 */

#include "colib.h"

/*
 * called when coshell is hung
 */

static void
hung(int sig)
{
	NoP(sig);
	kill(state.current->pid, SIGKILL);
}

/*
 * shut down one coshell
 */

static int
shut(register Coshell_t* co)
{
	register Coshell_t*	cs;
	int			n;
	int			status;
	Coshell_t*		ps;
	Coservice_t*		sv;
	Sig_handler_t		handler;

	sfclose(co->msgfp);
	close(co->cmdfd);
	if (co->pid)
	{
		if (co->running > 0)
			killpg(co->pid, SIGTERM);
		state.current = co;
		handler = signal(SIGALRM, hung);
		n = alarm(3);
		if (waitpid(co->pid, &status, 0) != co->pid)
			status = -1;
		alarm(n);
		signal(SIGALRM, handler);
		killpg(co->pid, SIGTERM);
	}
	else
		status = 0;
	if (co->flags & CO_DEBUG)
		errormsg(state.lib, 2, "coshell %d jobs %d user %s sys %s", co->index, co->total, fmtelapsed(co->user, CO_QUANT), fmtelapsed(co->sys, CO_QUANT));
	for (sv = co->service; sv; sv = sv->next)
	{
		if (sv->fd > 0)
			close(sv->fd);
		if (sv->pid)
			waitpid(sv->pid, &status, 0);
	}
	cs = state.coshells;
	ps = 0;
	while (cs)
	{
		if (cs == co)
		{
			cs = cs->next;
			if (ps)
				ps->next = cs;
			else
				state.coshells = cs;
			vmclose(co->vm);
			break;
		}
		ps = cs;
		cs = cs->next;
	}
	return status;
}

/*
 * close coshell co
 */

int
coclose(register Coshell_t* co)
{
	if (co)
		return shut(co);
	while (state.coshells)
		shut(state.coshells);
	return 0;
}
