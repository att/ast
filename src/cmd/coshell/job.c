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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * remote coshell server job and connection support
 */

#include "service.h"

/*
 * drop a connection
 */

void
drop(register int fd)
{
	register int	i;
	register int	n;
	Coshell_t*	sp;
	Cojob_t*	jp;
	Sfio_t*		tp;

	switch (state.con[fd].type)
	{
	case USER:
		if (state.con[fd].info.user.running)
			for (jp = state.job; jp <= state.jobmax; jp++)
				if (jp->fd == fd && jp->pid)
				{
					jp->fd = 0;
					jobkill(jp, SIGKILL);
				}
		/*FALLTHROUGH*/
	case INIT:
		for (i = 0; i < elementsof(state.con[fd].info.user.fds); i++)
			if ((n = state.con[fd].info.user.fds[i]) != fd && n >= 0 && state.con[n].type)
			{
				close(n);
				state.con[n].type = 0;
			}
		if (sp = state.con[fd].info.user.home)
			sp->home--;
		if (state.con[fd].info.user.pump)
			free(state.con[fd].info.user.pump);
		if (state.con[fd].info.user.expr)
			free(state.con[fd].info.user.expr);
		break;
	case PASS:
		if (tp = state.con[fd].info.pass.serialize)
		{
			state.con[fd].info.pass.serialize = 0;
			cswrite(state.con[fd].info.pass.fd, sfstrbase(tp), sfstrtell(tp));
			sfstrclose(tp);
		}
		if ((jp = state.con[fd].info.pass.job) && jp->pid)
		{
			if (--jp->ref <= 0)
				jobdone(jp);
			else if (!jp->lost)
				jp->lost = cs.time + UPDATE;
		}
		break;
	case POLL:
		error(ERROR_SYSTEM|3, "lost connect stream");
		break;
	case SHELL:
		sp = state.con[fd].info.shell;
		sp->fd = -1;
		shellclose(sp, -1);
		break;
	}
	state.con[fd].type = 0;
	state.con[fd].error = 0;
	csfd(fd, CS_POLL_CLOSE);
}

/*
 * check the job table for hogs running on busy shells
 * or jobs queued on hung shells
 * if only!=0 then only that shell is checked
 */

void
jobcheck(Coshell_t* only)
{
	register Cojob_t*	jp;
	register Coshell_t*	sp;
	char*			s;

	for (jp = state.job; jp <= state.jobmax; jp++)
		if (jp->pid && ((sp = jp->shell) == only || !only))
		{
			if (jp->pid > 0)
			{
				if (sp->update <= cs.time && sp->errors < ERRORS) update(sp);
				if (jp->lost && jp->lost < cs.time)
				{
					message((-4, "jobcheck: %s: job %d pid %d lost", sp->name, jp->rid, jp->pid));
					jp->sig = SIGKILL;
					jobdone(jp);
					continue;
				}
				if (sp->idle)
				{
					if (sp->stat.idle < state.busy && (!sp->bypass || !miscmatch(sp, sp->bypass)))
					{
						if (!jp->busy && state.grace) jp->busy = cs.time + state.grace;
						else if (jp->busy < cs.time)
						{
							if (state.migrate)
							{
								int	n;

								error(ERROR_OUTPUT|2, state.con[jp->fd].info.user.fds[2], "%s: job=%d pid=%d %s", sp->name, jp - state.job, jp->pid, state.migrate);
								n = sfprintf(state.string, "job=%d; pid=%d; host=%s; type=%s; %s\n", jp - state.job, jp->pid, sp->name, sp->type, state.migrate);
								if (s = sfstruse(state.string))
									cswrite(jp->shell->fd, s, n);
								else
									error(ERROR_OUTPUT|2, state.con[jp->fd].info.user.fds[2], "out of space");
								jp->sig = SIGKILL;
								jobdone(jp);
							}
							else
							{
#ifdef SIGSTOP
								error(ERROR_OUTPUT|2, state.con[jp->fd].info.user.fds[2], "%s: job=%d pid=%d stopped", sp->name, jp - state.job, jp->pid);
								jobkill(jp, SIGSTOP);
#else
								error(ERROR_OUTPUT|2, state.con[jp->fd].info.user.fds[2], "%s: job=%d pid=%d is a hog", sp->name, jp - state.job, jp->pid);
#endif
								jp->busy = HOG;
							}
						}
						continue;
					}
				}
				if (jp->busy == HOG)
				{
#ifdef SIGCONT
					error(ERROR_OUTPUT|2, state.con[jp->fd].info.user.fds[2], "job %d pid %d restarted on %s", jp - state.job, jp->pid, sp->name);
					jobkill(jp, SIGCONT);
#else
					error(ERROR_OUTPUT|2, state.con[jp->fd].info.user.fds[2], "job %d pid %d is no longer hogging %s", jp - state.job, jp->pid, sp->name);
					jp->busy = 0;
#endif
				}
			}
			else if (jp->cmd)
			{
				if (jp->pid == QUEUE && (sp->fd > 0 || sp == &state.wait))
				{
					if (state.running < (state.perserver + state.jobwait) && state.con[jp->fd].info.user.running < (state.peruser + 1) && (sp == &state.wait || sp->running < ((state.perhost ? state.perhost : sp->cpu * state.percpu) + 1)))
						shellexec(jp, jp->cmd, jp->fd);
				}
				else if (cs.time > jp->start + LOST)
				{
					message((-4, "jobcheck: %s: possibly hung %s", sp->name, fmtelapsed(cs.time - sp->start, 1)));
					shellclose(sp, -1);
				}
			}
		}
	if (state.jobwait || state.shellwait) cswakeup(state.wakeup = UPDATE * 1000L);
	else if (!state.busy || !state.running) cswakeup(state.wakeup = 0L);
	else if (!only) cswakeup(state.wakeup = UPDATE * 1000L);
}

/*
 * kill job with sig
 */

void
jobkill(Cojob_t* jp, int sig)
{
	int	n;
	char	buf[128];

	if (jp->pid)
	{
		jp->sig = sig;
		jp->busy = 0;
		if (jp->pid > 0 && jp->shell->fd > 0)
		{
			n = sfsprintf(buf, sizeof(buf), "kill -%s -%d\n", fmtsignal(-sig), jp->pid);
			cswrite(jp->shell->fd, buf, n);
			message((-2, "killpg -%s %s.%d", fmtsignal(-sig), jp->shell->name, jp->pid));
		}
		if (sig == SIGKILL) jobdone(jp);
	}
}

/*
 * job jp is done
 */

void
jobdone(register Cojob_t* jp)
{
	register int	n;
	char		buf[64];

	jp->pid = 0;
	jp->ref = 0;
	if (jp->cmd)
	{
		free(jp->cmd);
		jp->cmd = 0;
		state.jobwait--;
	}
	if (jp->shell == &state.wait) state.joblimit--;
	else if (jp->shell->running)
	{
		jp->shell->running--;
		if (!--state.running)
		{
			state.real += cs.time - state.clock;
			cswakeup(state.wakeup = 0L);
		}
	}
	if (jp->fd > 0)
	{
		jp->shell->user += jp->user;
		jp->shell->sys += jp->sys;
		state.con[jp->fd].info.user.running--;
		state.user += jp->user;
		state.sys += jp->sys;
		if (state.disable && (jp->sig == SIGKILL || jp->status > 128 && (jp->status % 128) == SIGKILL))
		{
			jp->shell->mode |= SHELL_DISABLE;
			jp->shell->update = cs.time + state.disable;
		}
#ifdef SIGCONT
		if (jp->sig && jp->sig != SIGCONT)
#else
		if (jp->sig)
#endif
			jp->status = jp->sig + 128;
		n = sfsprintf(buf, sizeof(buf), "x %d %d %s %s\n", jp->rid, jp->status, fmtelapsed(jp->user, CO_QUANT), fmtelapsed(jp->sys, CO_QUANT));
		if (cswrite(state.con[jp->fd].info.user.fds[0], buf, n) != n)
			drop(jp->fd);
	}
	if (state.joblimit) jobcheck(NiL);
}
