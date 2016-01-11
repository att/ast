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
 * send an action to the coshell for execution
 */

#include "colib.h"

#include <proc.h>
#include <ls.h>

static Cojob_t*
service(register Coshell_t* co, Coservice_t* cs, Cojob_t* cj, int flags, Sfio_t* sp)
{
	Proc_t*		proc;
	size_t		n;
	int		i;
	int		j;
	int		fds[2];
	long		ops[4];
	char*		s;
	char**		a;

	if (flags & CO_DEBUG)
	{
		for (a = cs->argv; *a; a++)
			sfprintf(sp, " %s", *a);
		if (!(s = costash(sp)))
			goto nospace;
		errormsg(state.lib, ERROR_LIBRARY|2, "service %s:%s", cs->path, s);
	}
	if (pipe(fds) < 0)
	{
		errormsg(state.lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "%s: cannot allocate service pipe", cs->name);
		return 0;
	}
	if (co->flags & CO_SHELL)
		for (i = 0; i < elementsof(fds); i++)
			if (fds[i] < 10 && (j = fcntl(fds[i], F_DUPFD, 10)) >= 0)
			{
				close(fds[i]);
				fds[i] = j;
			}
	cs->fd = fds[1];
	ops[0] = PROC_FD_DUP(fds[0], 0, PROC_FD_PARENT);
	ops[1] = PROC_FD_CLOSE(fds[1], PROC_FD_CHILD);
	ops[2] = PROC_FD_DUP(co->gsmfd, 1, 0);
	ops[3] = 0;
	if (!(proc = procopen(cs->path, cs->argv, NiL, ops, PROC_DAEMON|PROC_IGNORE)))
	{
		errormsg(state.lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "%s: cannot connect to %s service", cs->path, cs->name);
		close(fds[0]);	
		close(fds[1]);	
		return 0;
	}
	fcntl(cs->fd, F_SETFD, FD_CLOEXEC);
	cs->pid = proc->pid;
	procfree(proc);
	sfprintf(sp, "id=%d info\n", cj->id);
	n = sfstrtell(sp);
	if (!(s = costash(sp)))
		goto bad;
	if (write(cs->fd, s, n) != n || sfpoll(&co->msgfp, 1, 5 * 1000) <= 0)
		goto bad;
	cj->pid = 0;
	cj->status = 0;
	cj->local = 0;
	cj->service = cs;
	co->svc_outstanding++;
	co->svc_running++;
	if (!cowait(co, cj, -1))
		goto bad;
	return cj;
 bad:
	errormsg(state.lib, ERROR_LIBRARY|ERROR_SYSTEM|2, "%s: service not responding", cs->name);
 nospace:
	cj->pid = CO_PID_FREE;
	cs->pid = 0;
	close(cs->fd);
	cs->fd = -1;
	return 0;
}

static Cojob_t*
request(register Coshell_t* co, Cojob_t* cj, Coservice_t* cs, const char* action, int flags)
{
	ssize_t		n;
	ssize_t		i;
	Sfio_t*		sp;

	if (!(sp = sfstropen()))
	{
		errormsg(state.lib, ERROR_LIBRARY|2, "out of space");
		return 0;
	}
	if (!cs->fd && !service(co, cs, cj, flags, sp))
		goto bad;
	if (!cs->pid)
		goto bad;
	if (flags & CO_DEBUG)
		errormsg(state.lib, ERROR_LIBRARY|2, "job %d commands:\n\n%s %s\n", cj->id, cs->name, action);
	if (!(flags & CO_SILENT))
		sfprintf(sfstderr, "+ %s %s\n", cs->name, action);
	sfprintf(sp, "id=%d %s\n", cj->id, action);
	n = sfstrtell(sp);
	action = sfstrbase(sp);
	while ((i = write(cs->fd, action, n)) > 0 && (n -= i) > 0)
		action += i;
	sfstrclose(sp);
	if (n)
		goto bad;
	sfclose(sp);
	cj->pid = 0;
	cj->status = 0;
	cj->local = 0;
	cj->service = cs;
	co->svc_outstanding++;
	co->svc_running++;
	co->total++;
	return cj;
 bad:
	cj->pid = CO_PID_FREE;
	sfclose(sp);
	return 0;
}

Cojob_t*
coexec(register Coshell_t* co, const char* action, int flags, const char* out, const char* err, const char* att)
{
	register Cojob_t*	cj;
	register Sfio_t*	sp;
	register Coservice_t*	cs;
	int			n;
	int			i;
	int			og;
	int			cg;
	char*			s;
	char*			t;
	char*			env;
	char*			red;
	char*			sh[4];
	struct stat		sto;
	struct stat		ste;

	/*
	 * get a free job slot
	 */

	for (cj = co->jobs; cj; cj = cj->next)
		if (cj->pid == CO_PID_FREE)
			break;
	if (cj)
		cj->service = 0;
	else if (!(cj = vmnewof(co->vm, 0, Cojob_t, 1, 0)))
		return 0;
	else
	{
		cj->coshell = co;
		cj->pid = CO_PID_FREE;
		cj->id = ++co->slots;
		cj->next = co->jobs;
		co->jobs = cj;
	}

	/*
	 * set the flags
	 */

	flags &= ~co->mask;
	flags |= co->flags;
	cj->flags = flags;

	/*
	 * check service intercepts
	 */

	for (cs = co->service; cs; cs = cs->next)
	{
		for (s = cs->name, t = (char*)action; *s && *s == *t; s++, t++);
		if (!*s && *t == ' ')
			return request(co, cj, cs, t + 1, flags);
	}
	cj->flags &= ~CO_SERVICE;
	red = (cj->flags & CO_APPEND) ? ">>" : ">";

	/*
	 * package the action
	 */

	if (!(env = coinitialize(co, co->flags)))
		return 0;
	if (!(sp = sfstropen()))
		return 0;
	n = strlen(action);
	if (co->flags & CO_SERVER)
	{
		/*
		 * leave it to server
		 */

		sfprintf(sp, "#%05d\ne %d %d %s %s %s",
			0,
			cj->id,
			cj->flags,
			state.pwd,
			out,
			err);
		if (att)
			sfprintf(sp, " (%d:%s)", strlen(att), att);
		else
			sfprintf(sp, " %s", att);
		sfprintf(sp, " (%d:%s) (%d:%s)\n", strlen(env), env, n, action);
	}
	else if (co->flags & CO_INIT)
	{
		if (flags & CO_DEBUG)
			sfprintf(sp, "set -x\n");
		sfprintf(sp, "%s%s\necho x %d $? >&$%s\n",
			env,
			action,
			cj->id,
			CO_ENV_MSGFD);
	}
	else if (flags & CO_KSH)
	{
#if !_lib_fork && defined(_map_spawnve)
		Sfio_t*	tp;

		tp = sp;
		if (!(sp = sfstropen()))
			sp = tp;
#endif
		sfprintf(sp, "{\ntrap 'set %s$?; trap \"\" 0; IFS=\"\n\"; print -u$%s x %d $1 $(times); exit $1' 0 HUP INT QUIT TERM%s\n%s%s%s",
			(flags & CO_SILENT) ? "" : "+x ",
			CO_ENV_MSGFD,
			cj->id,
			(flags & CO_IGNORE) ? "" : " ERR",
			env,
			n > CO_MAXEVAL ? "" : "eval '",
			(flags & CO_SILENT) ? "" : "set -x\n");
		if (n > CO_MAXEVAL)
			sfputr(sp, action, -1);
		else
		{
			coquote(sp, action, 0);
			sfprintf(sp, "\n'");
		}
		sfprintf(sp, "\n} </dev/null");
		if (out)
		{
			if (*out == '/')
				sfprintf(sp, " %s%s", red, out);
			else
				sfprintf(sp, " %s%s/%s", red, state.pwd, out);
		}
		else if ((flags & CO_SERIALIZE) && (cj->out = pathtemp(NiL, 64, NiL, "coo", NiL)))
			sfprintf(sp, " >%s", cj->out);
		if (err)
		{
			if (out && streq(out, err))
				sfprintf(sp, " 2>&1");
			else if (*err == '/')
				sfprintf(sp, " 2%s%s", red, err);
			else
				sfprintf(sp, " 2%s%s/%s", red, state.pwd, err);
		}
		else if (flags & CO_SERIALIZE)
		{
			if (!out && !fstat(1, &sto) && !fstat(2, &ste) && sto.st_dev == ste.st_dev && sto.st_ino == ste.st_ino)
				sfprintf(sp, " 2>&1");
			else if (cj->err = pathtemp(NiL, 64, NiL, "coe", NiL))
				sfprintf(sp, " 2>%s", cj->err);
		}
#if !_lib_fork && defined(_map_spawnve)
		if (sp != tp)
		{
			sfprintf(tp, "%s -c '", state.sh);
			if (!(s = costash(sp)))
				return 0;
			coquote(tp, s, 0);
			sfprintf(tp, "'");
			sfstrclose(sp);
			sp = tp;
		}
#endif
		sfprintf(sp, " &\nprint -u$%s j %d $!\n",
			CO_ENV_MSGFD,
			cj->id);
	}
	else
	{
#if !_lib_fork && defined(_map_spawnve)
		Sfio_t*	tp;

		tp = sp;
		if (!(sp = sfstropen())) sp = tp;
#endif
		flags |= CO_IGNORE;
		if (co->mode & CO_MODE_SEPARATE)
		{
			flags &= ~CO_SERIALIZE;
			og = '{';
			cg = '}';
		}
		else
		{
			og = '(';
			cg = ')';
		}
		sfprintf(sp, "%c\n%s%sset -%s%s\n",
			og,
			env,
			n > CO_MAXEVAL ? "" : "eval '",
			(flags & CO_IGNORE) ? "" : "e",
			(flags & CO_SILENT) ? "" : "x");
		if (n > CO_MAXEVAL)
			sfprintf(sp, "%s", action);
		else
		{
			coquote(sp, action, 0);
			sfprintf(sp, "\n'");
		}
		sfprintf(sp, "\n%c </dev/null", cg);
		if (out)
		{
			if (*out == '/')
				sfprintf(sp, " %s%s", red, out);
			else
				sfprintf(sp, " %s%s/%s", red, state.pwd, out);
		}
		else if ((flags & CO_SERIALIZE) && (cj->out = pathtemp(NiL, 64, NiL, "coo", NiL)))
			sfprintf(sp, " >%s", cj->out);
		if (err)
		{
			if (out && streq(out, err))
				sfprintf(sp, " 2>&1");
			else if (*err == '/')
				sfprintf(sp, " 2%s%s", red, err);
			else
				sfprintf(sp, " 2%s%s/%s", red, state.pwd, err);
		}
		else if (flags & CO_SERIALIZE)
		{
			if (out)
				sfprintf(sp, " 2>&1");
			else if (cj->err = pathtemp(NiL, 64, NiL, "coe", NiL))
				sfprintf(sp, " 2>%s", cj->err);
		}
		if (!(co->mode & CO_MODE_SEPARATE))
		{
			if (flags & CO_OSH)
				sfprintf(sp, " && echo x %d 0 >&$%s || echo x %d $? >&$%s",
					cj->id,
					CO_ENV_MSGFD,
					cj->id,
					CO_ENV_MSGFD);
			else
				sfprintf(sp, " && echo x %d 0 `times` >&$%s || echo x %d $? `times` >&$%s",
					cj->id,
					CO_ENV_MSGFD,
					cj->id,
					CO_ENV_MSGFD);
		}
#if !_lib_fork && defined(_map_spawnve)
		if (sp != tp)
		{
			sfprintf(tp, "%s -c '", state.sh);
			if (!(s = costash(sp)))
				return 0;
			coquote(tp, s, 0);
			sfprintf(tp, "'");
			sfstrclose(sp);
			sp = tp;
		}
#endif
		if (!(co->mode & CO_MODE_SEPARATE))
			sfprintf(sp, " &\necho j %d $! >&$%s\n",
				cj->id,
				CO_ENV_MSGFD);
	}
	n = sfstrtell(sp);
	if (!costash(sp))
		return 0;
	if (flags & CO_SERVER)
		sfprintf(sp, "#%05d\n", n - 7);
	s = sfstrseek(sp, 0, SEEK_SET);
	if (flags & CO_DEBUG)
		errormsg(state.lib, ERROR_LIBRARY|2, "job %d commands:\n\n%s\n", cj->id, s);
	if (co->mode & CO_MODE_SEPARATE)
	{
		sh[0] = state.sh;
		sh[1] = "-c";
		sh[2] = s;
		sh[3] = 0;
		cj->status = procrun(state.sh, sh, 0);
		sfstrclose(sp);
		cj->pid = CO_PID_ZOMBIE;
		cj->local = 0;
		co->outstanding++;
		co->total++;
	}
	else
	{
		/*
		 * send it off
		 */

		while ((i = write(co->cmdfd, s, n)) > 0 && (n -= i) > 0)
			s += i;
		sfstrclose(sp);
		if (n)
			return 0;

		/*
		 * it's a job
		 */

		cj->pid = 0;
		cj->status = 0;
		cj->local = 0;
		co->outstanding++;
		co->running++;
		co->total++;
		if (co->mode & CO_MODE_ACK)
			cj = cowait(co, cj, -1);
	}
	return cj;
}
