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
 * remote coshell server shell support
 */

#include "service.h"

#include <proc.h>

/*
 * single quote s into sp
 */

static void
quote(register Sfio_t* sp, register char* s)
{
	register int	c;

	while (c = *s++)
	{
		sfputc(sp, c);
		if (c == '\'') sfputr(sp, "\\''", -1);
	}
}

/*
 * open a new shell connection to sp
 * error output to fd
 */

int
shellopen(register Coshell_t* sp, int fd)
{
	char*		sh;
	Proc_t*		proc;
	Sfio_t*		xp;
	Sfio_t*		vp;
	char*		av[4];

	if (!sp)
	{
		if (fd >= 0)
			error(ERROR_OUTPUT|2, fd, "host not found");
	}
	else if (sp->fd || sp == &state.wait)
		return 0;
	else
	{
		update(sp);
		if (sp->stat.up < 0)
		{
			if (fd >= 0)
				error(ERROR_OUTPUT|2, fd, "%s: host is down", sp->name);
			return -1;
		}
		if (!(xp = sfstropen()) || !(vp = sfstropen()))
		{
			if (xp)
				vp = 0;
			goto nospace;
		}
		sfprintf(xp, sp->shell[0] ? sp->shell : state.sh, sp->type);
		if (!(sh = sfstruse(xp)))
			goto nospace;
		if (sp == state.shell)
		{
			sfprintf(vp, "%s=%s %s=%s %s='%s %s' COINIT='%s' %s /dev/fd/4 >/dev/null 2>&1 3<%s 4<&3 5>&- 6>&- 7>&- 8>&- 9>&- &", CO_ENV_HOST, sp->name, CO_ENV_TYPE, sp->type, CO_ENV_SHELL, opt_info.argv[0], state.service, (sp->flags & SETRATING) ? "rating=0;" : "", sh, state.mesg);
			av[0] = "sh";
			av[1] = "-c";
		}
		else
		{
			sfprintf(vp, "%s -c 'trap \"\" HUP; %s=%s %s=%s %s= COINIT='\\''%s%s'\\'' %s /dev/fd/4 >/dev/null 2>&1 3<%s 4<&3 5>&- 6>&- 7>&- 8>&- 9>&- &'", sh, CO_ENV_HOST, sp->name, CO_ENV_TYPE, sp->type, CO_ENV_SHELL, (sp->flags & SETRATING) ? "rating=0;" : "", state.profile ? state.profile : "", sh, state.mesg);
			av[0] = sh = state.remote;
			av[1] = sp->name;
		}
		if (!(av[2] = sfstruse(vp)))
			goto nospace;
		av[3] = 0;
		message((-2, "%s %s \"%s\"", sh, av[1], av[2]));
		if (!(proc = procopen(sh, av, NiL, NiL, 0)))
		{
			if (fd >= 0)
				error(ERROR_OUTPUT|2, fd, "%s: cannot exec shell", sh);
			sfstrclose(xp);
			sfstrclose(vp);
			return -1;
		}
		message((-1, "%s: open shell pid=%d", sp->name, proc->pid));
		sp->flags &= ~DEF;
		sp->fd = -proc->pid;
		sp->start = cs.time;
		if (sp->update > cs.time + UPDATE)
			sp->update = cs.time + UPDATE;
		if (sp->mode & SHELL_OVERRIDE)
		{
			if (!sp->override)
				state.override++;
			sp->override = cs.time + (sp->home ? HOME : OVERRIDE);
		}
		state.shellwait++;
		state.open += sp->cpu;
		procfree(proc);
		sfstrclose(xp);
		sfstrclose(vp);
		return 0;
	}
	return -1;
 nospace:
	if (fd >= 0)
		error(ERROR_OUTPUT|2, fd, "out of space");
	if (xp)
		sfstrclose(xp);
	if (vp)
		sfstrclose(vp);
	return -1;
}

/*
 * close shell sp if open
 * error output to fd
 */

void
shellclose(register Coshell_t* sp, int fd)
{
	register Cojob_t*	jp;

	if (sp->fd != -1) message((-1, "%s: close shell", sp->name));
	if (sp->fd < 0)
	{
		if (sp->fd != -1)
		{
			kill(-sp->fd, SIGKILL);
			state.shellwait--;
		}
		if (sp->idle_override)
		{
			sp->idle = sp->idle_override;
			sp->idle_override = 0;
		}
		sp->stat.up = sp->start - cs.time;
		sp->update = cs.time + FORGET;
		sp->rank = RANK * 100;
		state.open -= sp->cpu;
		if (sp->override)
		{
			sp->override = 0;
			state.override--;
			sp->update = 0;
		}
		for (jp = state.job; sp->running && jp <= state.jobmax; jp++)
			if (jp->shell == sp && jp->pid)
			{
				if (jp->cmd)
				{
					sp->running--;
					sp->total--;
					jp->shell = 0;
					shellexec(jp, jp->cmd, jp->fd);
				}
				else jobkill(jp, SIGKILL);
			}
		if (sp->fd != -1) waitpid(-sp->fd, NiL, 0);
		sp->fd = 0;
	}
	else if (sp->fd > 0)
	{
		drop(sp->fd);
		sp->start = cs.time;
	}
	else if (fd >= 0) error(ERROR_OUTPUT|2, fd, "%s: not open", sp->name);
}

/*
 * send command in the exec message msg to a shell for user fd
 * if jp!=0 then the msg was popped off jp->sp's delay queue
 */

void
shellexec(Cojob_t* jp, char* msg, int fd)
{
	register Coshell_t*	sp;
	Connection_t*		con;
	int			n;
	int			id;
	int			flags;
	char*			s;
	char*			cmd;
	char*			act;
	char*			pwd;
	char*			out;
	char*			err;
	char*			att;
	char*			env;
	char*			end;
	char*			red;
	Coattr_t		attr;

	attr.set = attr.global.set = 0;
	con = state.con;
	cmd = strdup(msg);
	if (tokscan(msg, &end, "%s %d %d %s %s %s %s %s %s", NiL, &id, &flags, &pwd, &out, &err, &att, &env, &act) != 9)
	{
		error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "invalid exec message");
		goto noexec;
	}
	if (!jp || !(sp = jp->shell) || sp == &state.wait)
	{
		/*
		 * find a shell slot
		 */

		if (cs.time > state.access) info(SET, NiL);
		if (sp == &state.wait) state.joblimit--;
		if (state.running >= (state.perserver + state.jobwait) || con[fd].info.user.running >= state.peruser)
		{
			sp = &state.wait;
			if (!jp) attributes(att, &attr, &con[fd].info.user.attr);
		}
		else if (!(sp = search((flags & CO_LOCAL) ? DEF|JOB : JOB, att, &attr, &con[fd].info.user.attr)))
		{
			if (s = con[fd].info.user.expr)
			{
				if (att) error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%s && %s: invalid host", s, att);
				else error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%s: invalid host", s);
			}
			else if (att) error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%s: invalid host", att);
			else error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "all hosts closed");
			goto noexec;
		}
		if (sp->fd <= 0 && shellopen(sp, con[fd].info.user.fds[2]))
		{
			error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%s: cannot open", sp->name);
			goto noexec;
		}
		if (!jp)
		{
			/*
			 * find a free job slot
			 */

			jp = state.jobnext;
			for (;;)
			{
				if (jp++ >= state.jobmax) jp = state.job;
				if (!jp->pid) break;
				if (jp == state.jobnext)
				{
					jp = 0;
					error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "job table full");
					goto noexec;
				}
			}
			state.jobnext = jp;
			if (attr.set & SETLABEL) strcpy(jp->label, attr.label);
			else *jp->label = 0;
			jp->rid = id;
			jp->fd = fd;
			jp->sig = 0;
			jp->status = 0;
			jp->busy = 0;
			jp->lost = 0;
			jp->user = 0;
			jp->sys = 0;
			if (!state.running++)
			{
				state.clock = cs.time;
				if (state.busy) cswakeup(state.wakeup = UPDATE * 1000L);
			}
			con[fd].info.user.running++;
			con[fd].info.user.total++;
			state.jobs++;
		}
		jp->shell = sp;
		sp->running++;
		sp->total++;
	}
	jp->flags = flags;
	jp->start = cs.time;
	if (!(msg = jp->cmd)) state.jobwait++;
	jp->cmd = cmd;
	cmd = msg;
	if (sp->fd <= 0)
	{
		jp->pid = QUEUE;
		if (sp == &state.wait) state.joblimit++;
		if (cmd) free(cmd);
		return;
	}
	jp->pid = START;
	jp->ref = 1;
	red = (flags & CO_APPEND) ? ">>" : ">";
	sfprintf(state.string, "{\ntrap 'set %s$?; trap \"\" 0; print -u3 x %d $1 $(times); 3>&-; exit $1' 0 HUP INT QUIT TERM%s\n",
		(flags & CO_SILENT) ? "" : "+x ",
		jp - state.job,
		(flags & CO_IGNORE) ? "" : " ERR");
	if (!out)
	{
		if (con[fd].info.user.pump) sfprintf(state.string, "print '#%05d'\n", 1);
		else
		{
			sfprintf(state.string, "print '#%05d.%05d'\n", jp - state.job, con[fd].info.user.fds[1]);
			jp->ref++;
		}
	}
	if (!err)
	{
		if (!out && con[fd].info.user.fds[1] == con[fd].info.user.fds[2]) err = "&1";
		else if (con[fd].info.user.pump) sfprintf(state.string, "print -u2 '#%05d'\n", 2);
		else
		{
			sfprintf(state.string, "print -u2 '#%05d.%05d'\n", jp - state.job, con[fd].info.user.fds[2]);
			jp->ref++;
		}
	}
	sfprintf(state.string, "%s\ntypeset -i%d ___=${!:-$$}\nexport %s=%..*u${___#%d#}\neval '%s", env ? env : "", TEMPBASE, CO_ENV_TEMP, TEMPBASE, sp->addr, TEMPBASE, (flags & CO_SILENT) ? "" : "set -x\n");
	if (act) quote(state.string, act);
	sfprintf(state.string, "\n'\n} </dev/null %s", red);

	/*
	 * NOTE: state.pump could be replaced by remote /dev/tty*
	 */

	if (!out && !(out = con[fd].info.user.pump)) out = state.pump;
	else if (*out != '/' && *out != '&') sfprintf(state.string, "%s/", pwd);
	sfprintf(state.string, "%s 2%s", out, red);
	if (!err && !(err = con[fd].info.user.pump)) err = state.pump;
	else if (*err != '/' && *err != '&') sfprintf(state.string, "%s/", pwd);
	sfprintf(state.string, "%s &\nprint -u3 j %d $!\n", err, jp - state.job);
	n = sfstrtell(state.string);
	if (!(s = sfstruse(state.string)))
	{
		error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "out of space");
		goto nojob;
	}
	message((-5, "job: %s", s));
	if (cswrite(sp->fd, s, n) != n)
	{
		error(ERROR_OUTPUT|2, con[fd].info.user.fds[2], "%s: lost host connection", sp->name);
		goto nojob;
	}
	if (cmd) free(cmd);
	n = sfsprintf(state.buf, state.buflen, "a %d\n", id);
	if (cswrite(con[fd].info.user.fds[0], state.buf, n) != n)
		goto noexec;
	return;
 nojob:
	if (cmd) free(cmd);
	jp->status = EXIT_NOEXEC;
	jobdone(jp);
	return;
 noexec:
	if (cmd) free(cmd);
	if (jp) jobdone(jp);
	n = sfsprintf(state.buf, state.buflen, "x %d %d\n", id, EXIT_NOEXEC);
	if (cswrite(con[fd].info.user.fds[0], state.buf, n) != n)
		drop(fd);
}

/*
 * check for queued jobs on shells hung during open
 * call if state.shellwait>0
 */

void
shellcheck(void)
{
	register Coshell_t*	sp;

	sp = state.shell;
	do
	{
		if (sp->fd < 0 && cs.time > (sp->start + LOST))
		{
			message((-4, "shellcheck: %s: hung %s", sp->name, fmtelapsed(cs.time - sp->start, 1)));
			shellclose(sp, -1);
		}
	} while ((sp = sp->next) != state.shell);
}
