/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2012 AT&T Intellectual Property          *
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
 * at.svc -- the other side of at
 *
 * the at control dir hierarchy, all files owned by one user
 *
 *	$INSTALLROOT/lib/at
 *		jobs			AT_DIR_MODE
 *			<host>		AT_DIR_MODE
 *				<job-i>	AT_JOB_MODE
 *
 * job names are encoded with <uid.gid.time> base 64 where
 * <time> is the earliest absolute time the job can be run
 */

static const char id[] = "\n@(#)$Id: at.svc (AT&T Research) 2012-02-29 $\0\n";

#include "at.h"

#include <cdt.h>
#include <debug.h>
#include <dirent.h>
#include <proc.h>
#include <pwd.h>
#include <sig.h>
#include <wait.h>

#ifndef SIGCHLD
#define SIGCHLD		SIGCLD
#endif

#define HOG		(NPROC*10)
#define LOAD		0
#define MSMAX		(LONG_MAX/1000)
#define NAME		22
#define NICE		2
#define NPROC		10
#define PERUSER		(NPROC/2)
#define SMAX		ULONG_MAX
#define WAIT		0

typedef struct
{
	Dtlink_t	byname;
	Dtlink_t	byuid;
	char*		home;
	int		pending;
	int		running;
	unsigned long	admin;
	unsigned long	uid;
	unsigned long	total;
	char		name[1];
} User_t;

typedef struct
{
	Dtlink_t	byuser;
	User_t*		user;
	int		allow;
	int		pending;
	int		running;
	unsigned long	total;
} Owner_t;

typedef struct
{
	Dtlink_t	byname;
	Dt_t*		owner;
	int		allow;
	int		home;
	int		nproc;
	int		peruser;
	int		load;
	int		nice;
	int		wait;
	int		pending;
	int		running;
	int		specific;
	unsigned long	total;
	char		name[1];
} Queue_t;

typedef struct
{
	Dtlink_t	byname;
	Dtlink_t	bypid;
	unsigned long	start;
	char		name[NAME];
	Queue_t*	queue;
	Owner_t*	owner;
	char*		period;
	char*		repeat;
	char*		shell;
	int		mail;
	Csid_t		id;
	unsigned long	pid;
	unsigned long	run;
	unsigned long	total;
	char		label[11];
	char		data[1];
} Job_t;

typedef struct
{
	int		fd;
	Csid_t		id;
} Connection_t;

typedef struct
{
	Dtdisc_t	discipline;
	Dt_t*		handle;
} Table_t;

typedef struct
{
	Cssdisc_t	disc;		/* css discipline		*/
	Css_t*		css;		/* css handle			*/
	struct
	{
	Table_t		job;		/* job by <start,name>		*/
	Table_t		owner;		/* user by User_t*		*/
	Table_t		pid;		/* job by pid			*/
	Table_t		queue;		/* queue by name		*/
	Table_t		uid;		/* user by uid			*/
	Table_t		user;		/* user by name			*/
	}		table;
	Queue_t*	queue;		/* default queue		*/
	time_t		update;		/* info file mtime		*/
	int		nproc;		/* total proc limit		*/
	int		peruser;	/* total per user limit		*/
	int		pending;	/* total pending jobs		*/
	int		running;	/* total running jobs		*/
	int		bufsiz;		/* sizeof(*buf)			*/
	int		init;		/* initialization		*/
	unsigned long	admin[4];	/* admin uids			*/
	unsigned long	total;		/* total number of jobs run	*/
	unsigned long	sequence;	/* job id sequence		*/
	char*		pwd;		/* jobs dir path		*/
	char*		atx;		/* setuid exec full path	*/
	char*		buf;		/* work buffer			*/
	Sfio_t*		tmp;		/* work string			*/
	Connection_t	con[1];		/* user connections		*/
} State_t;

typedef struct
{
	Connection_t*	con;
	Queue_t*	queue;
	State_t*	state;
} Visit_t;

static const char*	export[] =
{
	CO_ENV_HOST,	CO_ENV_TYPE
};

static const char*	queuedefs[] =
{
	"a.4j1n2u",	"b.2j2n90w2u",		"c.h8j2u60w"
};

static int	schedule(State_t*);

/*
 * return user info given name or uid
 */

static User_t*
user(register State_t* state, char* name, unsigned long uid)
{
	register User_t*	usr;
	register struct passwd*	pwd;
	register int		n;
	char*			home;
	char			buf[16];

	static User_t		nobody;

	if (!(usr = name ? (User_t*)dtmatch(state->table.user.handle, name) : (User_t*)dtmatch(state->table.uid.handle, &uid)))
	{
		if (pwd = name ? getpwnam(name) : getpwuid(uid))
		{
			if (name) uid = pwd->pw_uid;
			else name = pwd->pw_name;
			home = pwd->pw_dir;
		}
		else
		{
			if (!name)
				sfsprintf(name = buf, sizeof(buf), "%lu", uid);
			home = "/tmp";
		}
		n = strlen(name);
		if (!(usr = newof(0, User_t, 1, n + strlen(home) + 2)))
			usr = &nobody;
		else
		{
			usr->uid = uid;
			strcpy(usr->name, name);
			usr->home = strcpy(usr->name + n + 1, home);
			for (n = 0; n < elementsof(state->admin); n++)
				if (uid == state->admin[n])
				{
					usr->admin = 1;
					break;
				}
		}
		dtinsert(state->table.user.handle, usr);
		dtinsert(state->table.uid.handle, usr);
	}
	return usr;
}

/*
 * check if usr is permitted on que
 * Owner_t allocated and returned if ok
 */

static Owner_t*
permit(register Queue_t* que, User_t* usr)
{
	register Owner_t*	own;

	if (!(own = (Owner_t*)dtmatch(que->owner, &usr)))
	{
		if (que->allow > 0)
			return 0;
		if (own = newof(0, Owner_t, 1, 0))
		{
			own->allow = 1;
			own->user = usr;
			dtinsert(que->owner, own);
		}
	}
	else if (!own->allow)
		return 0;
	return own;
}

/*
 * initialize owner.allow 
 */

static int
allow(Dt_t* dt, void* object, void* handle)
{
	NoP(dt);
	((Owner_t*)object)->allow = !((Queue_t*)handle)->allow;
	return 0;
}

/*
 * add queue defined in s
 */

static void
queue(register State_t* state, register char* s)
{
	register Queue_t*	que;
	register Owner_t*	own;
	User_t*			usr;
	char*			t;
	long			n;

	while (isspace(*s))
		s++;
	if (*s && *s != '#')
	{
		if (t = strchr(s, '.'))
			*t++ = 0;
		if (!(que = (Queue_t*)dtmatch(state->table.queue.handle, s)))
		{
			if (!(que = newof(0, Queue_t, 1, strlen(s) + 1)))
				error(ERROR_SYSTEM|3, "out of space [queue]");
			strcpy(que->name, s);
			que->nproc = NPROC;
			que->peruser = PERUSER;
			que->load = LOAD;
			que->nice = NICE;
			que->wait = WAIT;
			que = (Queue_t*)dtinsert(state->table.queue.handle, que);
		}
		if (!state->queue)
			state->queue = que;
		if (!que->owner && !(que->owner = dtopen(&state->table.owner.discipline, Dtset)))
			error(ERROR_SYSTEM|3, "out of space [queue %s owner hash]", que->name);
		que->allow = -1;
		que->specific = 0;
		while ((s = t) && *s)
		{
			n = strtol(s, &t, 10);
			if (t == s)
				n = 1;
			switch (*t++)
			{
			case 0:
			case '#':
				t = 0;
				break;
			case 'h':
				que->home = n;
				break;
			case 'j':
				que->nproc = n;
				break;
			case 'l':
				que->load = n;
				break;
			case 'n':
				que->nice = n;
				break;
			case 'u':
				que->peruser = n;
				break;
			case 'w':
				que->wait = n;
				break;
			case ' ':
			case '\t':
				que->allow = *(t - 2) == '+';
				que->specific = 1;
				dtwalk(que->owner, allow, que);
				s = t;
				for (;;)
				{
					while (isspace(*s))
						s++;
					if (*(t = s) == '#')
						break;
					while (*s && !isspace(*s))
						s++;
					if (n = *s)
						*s++ = 0;
					if (!*t)
						break;
					usr = user(state, t, 0);
					if (!(own = (Owner_t*)dtmatch(que->owner, &usr)))
					{
						if (!(own = newof(0, Owner_t, 1, 0)))
							error(ERROR_SYSTEM|3, "out of space [queue owner]");
						own->user = usr;
						dtinsert(que->owner, own);
					}
					own->allow = que->allow;
				}
				t = 0;
				break;
			}
		}
	}
}

/*
 * update state from the queue, allow and deny files
 */

static void
update(register State_t* state)
{
	register char*		s;
	register Sfio_t*	sp;
	register Queue_t*	que;
	User_t*			usr;
	Owner_t*		own;
	int			permit;
	char*			file;
	char*			path;
	struct stat		st;

	sfprintf(state->tmp, "%s/%s/%s", state->pwd, state->pwd, AT_QUEUE_FILE);
	if (!(path = sfstruse(state->tmp)))
		error(ERROR_SYSTEM|3, "out of space");
	pathcanon(path, 0, 0);
	if (sp = sfopen(NiL, path, "r"))
	{
		error(0, "scan %s queue list", path);
		if (fstat(sffileno(sp), &st) || st.st_mtime != state->update)
		{
			state->update = st.st_mtime;
			if (que = state->queue)
				state->queue = 0;
			while (s = sfgetr(sp, '\n', 1))
				queue(state, s);
			if (!state->queue && !(state->queue = que))
				error(ERROR_SYSTEM|3, "%s: no queues", path);
		}
		sfclose(sp);
	}

	/*
	 * apply the global allow/deny files to queues
	 * with no specific allow/deny overrides
	 */

	for (que = (Queue_t*)dtfirst(state->table.queue.handle); que && que->specific; que = (Queue_t*)dtnext(state->table.queue.handle, que));
	if (que && !stat(file = AT_CRON_DIR, &st) && S_ISDIR(st.st_mode))
	{
		sfprintf(state->tmp, "%s/%s/%s", state->pwd, file, AT_ALLOW_FILE);
		if (!(path = sfstruse(state->tmp)))
			error(ERROR_SYSTEM|3, "out of space");
		pathcanon(path, 0, 0);
		if (sp = sfopen(NiL, path, "r"))
			permit = 1;
		else
		{
			permit = 0;
			sfprintf(state->tmp, "%s/%s/%s", state->pwd, file, AT_DENY_FILE);
			if (!(path = sfstruse(state->tmp)))
				error(ERROR_SYSTEM|3, "out of space");
			pathcanon(path, 0, 0);
			sp = sfopen(NiL, path, "r");
		}

		/*
		 * reset the queue and associated user access
		 */

		for (que = (Queue_t*)dtfirst(state->table.queue.handle); que; que = (Queue_t*)dtnext(state->table.queue.handle, que))
			if (!que->specific)
			{
				que->allow = permit;
				dtwalk(que->owner, allow, que);
			}

		/*
		 * scan and update the access
		 */

		if (sp)
		{
			error(0, "scan %s access list", path);
			while (s = sfgetr(sp, '\n', 1))
			{
				usr = user(state, s, 0);
				for (que = (Queue_t*)dtfirst(state->table.queue.handle); que; que = (Queue_t*)dtnext(state->table.queue.handle, que))
					if (!que->specific)
					{
						if (!(own = (Owner_t*)dtmatch(que->owner, &usr)))
						{
							if (!(own = newof(0, Owner_t, 1, 0)))
								error(ERROR_SYSTEM|3, "out of space [queue owner]");
							own->user = usr;
							dtinsert(que->owner, own);
						}
						own->allow = que->allow;
					}
			}
			sfclose(sp);
		}
	}
}

/*
 * add a client connection
 */

static int
client(Css_t* css, Cssfd_t* fp, Csid_t* id, char** args, Cssdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;
	register Connection_t*	con = state->con + fp->fd;

	NoP(args);
	con->id = *id;
	return con->fd = fp->fd;
}

/*
 * insert a job into the run list
 */

static void
submit(State_t* state, register Job_t* job)
{
	dtinsert(state->table.job.handle, job);
	job->owner->pending++;
	job->owner->user->pending++;
	job->queue->pending++;
	state->pending++;
	error(0, "%s %s que %s at %s \"%s\"", job->name, fmtuid(job->owner->user->uid), job->queue->name, fmttime(AT_TIME_FORMAT, job->start), job->label);
}

/*
 * remove job from the run list
 */

static void
complete(State_t* state, register Job_t* job)
{
	dtdelete(state->table.job.handle, job);
	if (job->pid)
	{
		dtdelete(state->table.pid.handle, job);
		job->pid = 0;
	}
	if (job->owner)
	{
		if (job->run)
		{
			job->owner->running--;
			job->owner->user->running--;
		}
		job->owner->pending--;
		job->owner->user->pending--;
	}
	if (job->queue)
	{
		if (job->run)
		{
			job->queue->running--;
			state->running--;
		}
		job->queue->pending--;
		state->pending--;
	}
	job->run = 0;
}

/*
 * drop a job
 */

static void
drop(register State_t* state, register Job_t* job)
{
	complete(state, job);
	error(0, "%s %s que %s drop \"%s\"", job->name, fmtuid(job->owner->user->uid), job->queue->name, job->label);
	remove(job->name);
	free(job);
}

/*
 * pass job to atx for impersonation and execution
 */

static unsigned long
execute(register State_t* state, register Job_t* job)
{
	Proc_t*		proc;
	unsigned long	pid;
	char*		argv[4];
	long		ops[3];

	argv[0] = state->atx;
	argv[1] = job->shell;
	argv[2] = job->name;
	argv[3] = 0;
	ops[0] = PROC_SYS_PGRP(1);
	ops[1] = 0;
	message((-2, "%s %s %s", argv[0], argv[1], argv[2]));
	if (!(proc = procopen(argv[0], argv, NiL, ops, 0)))
		return 0;
	pid = proc->pid;
	procfree(proc);
	return pid;
}

/*
 * reap the state for a job that has completed
 */

static void
reap(register State_t* state, register Job_t* job, int status)
{
	char*		e;
	time_t		t;

	error(0, "%s %s %lu exit %d \"%s\"", job->name, fmtuid(job->owner->user->uid), job->pid, status, job->label);
	if (job->repeat && (t = job->start) && (t = tmdate(job->repeat, &e, &t)) && !*e)
	{
		complete(state, job);
		job->start = t;
		submit(state, job);
	}
	else
		drop(state, job);
	schedule(state);
}

/*
 * execute ready jobs
 */

static int
schedule(register State_t* state)
{
	register Job_t*		job;
	register Queue_t*	que;
	Csstat_t		st;

	unsigned long		x = SMAX;

	csstat(state->css->state, NiL, &st);
	for (job = (Job_t*)dtfirst(state->table.job.handle); job; job = (Job_t*)dtnext(state->table.job.handle, job))
	{
		message((-2, "schedule job=%s start=%lu time=%lu queue=%s load=%d.%02d/%d.%02d%s", job->name, job->start, cs.time, job->queue->name, job->queue->load / 100, job->queue->load % 100, st.load / 100, st.load % 100, job->run ? " RUNNING" : job->start <= cs.time ? " READY" : ""));
		if (!job->run)
		{
			if (job->start <= cs.time)
			{
				que = job->queue;
				if ((que->nproc <= 0 || que->running < que->nproc) &&
				    (que->load <= 0 || st.load < que->load) &&
				    (state->nproc <= 0 || state->running < state->nproc) &&
				    (que->peruser <= 0 || job->owner->running < que->peruser) &&
				    (state->peruser <= 0 || job->owner->running < state->peruser))
				{
					if (job->pid = execute(state, job))
					{
						job->run = cs.time;
						job->owner->running++;
						job->owner->total++;
						job->owner->user->running++;
						job->owner->user->total++;
						que->running++;
						que->total++;
						state->running++;
						state->total++;
						dtinsert(state->table.pid.handle, job);
						message((-2, "exec job=%s pid=%lu", job->name, job->pid));
						error(0, "%s %s %lu exec \"%s\"", job->name, fmtuid(job->owner->user->uid), job->pid, job->label);
					}
					else if (x > (job->start + que->wait))
						x = job->start + que->wait;
				}
				else if (x > (job->start + que->wait))
					x = job->start + que->wait;
			}
			else
			{
				if (x > job->start)
					x = job->start;
				break;
			}
		}
	}
	if (x == SMAX)
		state->disc.wakeup = 0;
	else if (x > cs.time)
	{
		x -= cs.time;
		x = (x >= MSMAX) ? MSMAX : (x * 1000);
		state->disc.wakeup = x;
		message((-2, "wakeup %s", fmtelapsed(x, 1000)));
	}
	return 0;
}

static int
exception(Css_t* css, unsigned long op, unsigned long arg, Cssdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;
	int			status;
	pid_t			pid;
	Job_t*			job;

	switch (op)
	{
	case CSS_INTERRUPT:
		if (arg != SIGCHLD)
			error(ERROR_SYSTEM|3, "%s: interrupt exit", fmtsignal(arg));
		for (;;)
		{
			switch (pid = waitpid(-1, &status, WNOHANG))
			{
			case -1:
				if (errno == EINTR)
					continue;
				break;
			case 0:
				break;
			default:
				status = WIFSIGNALED(status) ?
					EXIT_TERM(WTERMSIG(status)) :
					EXIT_CODE(WEXITSTATUS(status));
				message((-2, "wait pid=%lu status=%d", pid, status));
				if (job = (Job_t*)dtmatch(state->table.pid.handle, &pid))
					reap(state, job, status);
				continue;
			}
			break;
		}
		return 1;
	case CSS_WAKEUP:
		schedule(state);
		return 1;
	}
	error(ERROR_SYSTEM|3, "poll error");
	return -1;
}

/*
 * list owner names in handle
 */

static int
listowner(Dt_t* dt, void* object, void* handle)
{
	register Owner_t*	own = (Owner_t*)object;
	register Visit_t*	vis = (Visit_t*)handle;

	NoP(dt);
	if (own->allow == vis->queue->allow)
		sfprintf(vis->state->tmp, " %s", own->user->name);
	return 0;
}

/*
 * list queue status
 */

static int
listqueue(Dt_t* dt, void* object, void* handle)
{
	register Queue_t*	que = (Queue_t*)object;
	Connection_t*		con = ((Visit_t*)handle)->con;
	State_t*		state = ((Visit_t*)handle)->state;
	char*			s;
	Visit_t			visit;

	NoP(dt);
	sfprintf(state->tmp, " %c", que->allow ? '+' : '-');
	visit.queue = que;
	visit.state = state;
	dtwalk(que->owner, listowner, &visit);
	if (!(s = sfstruse(state->tmp)))
		error(ERROR_SYSTEM|3, "out of space");
	if (!s[2])
		*s = 0;
	error(ERROR_OUTPUT|0, con->fd, "%-3s %5lu %3d %3d %3d %3d %3d %2d.%02d %5.5s%s", que->name, que->total, que->pending, que->running, que->nproc, que->peruser, que->nice, que->load / 100, que->load % 100, fmtelapsed(que->wait, 1), s);
	return 0;
}

/*
 * output name='value'
 */

static void
neqv(register Sfio_t* sp, const char* name, register char* v)
{
	register int	c;

	sfprintf(sp, " \\\n %s='", name);
	while (c = *v++)
	{
		if (c == '\'')
			sfputr(sp, "'\\'", -1);
		sfputc(sp, c);
	}
	sfputc(sp, '\'');
}

/*
 * execute the at command in s
 */

static int
command(register State_t* state, Connection_t* con, register char* s, int n, char* data)
{
	register Queue_t*	que;
	register Job_t*		job;
	Owner_t*		own;
	User_t*			usr;
	char*			t;
	char*			u;
	char*			b;
	char*			h;
	int			c;
	int			admin;
	int			mail;
	int			skip;
	unsigned long		m;
	unsigned long		w;
	long			x;
	Job_t*			next;
	Sfio_t*			sp;
	Visit_t			visit;

	usr = user(state, NiL, con->id.uid);
	b = s;
	admin = 0;
	if (*++s == AT_ADMIN)
	{
		s++;
		if (!usr->admin)
			goto denied;
		if (!++usr->admin)
			usr->admin = 1;
		admin = 1;
	}
	if (*s == AT_QUEUE)
	{
		t = ++s;
		if (!(s = strchr(s, ' ')))
		{
			error(ERROR_OUTPUT|2, con->fd, "%s: invalid queue name", t);
			return -1;
		}
		*s = 0;
		if (!(que = (Queue_t*)dtmatch(state->table.queue.handle, t)))
		{
			error(ERROR_OUTPUT|2, con->fd, "%s: unknown queue", t);
			return -1;
		}
		*s++ = ' ';
		if (!permit(que, usr))
			goto noqueue;
	}
	else
		que = 0;
	if (mail = *s == AT_MAIL)
		s++;
	switch (c = *s++)
	{
	case AT_ACCESS:
		if (!que && !permit(que = state->queue, usr))
			goto noqueue;
		break;
	case AT_DEBUG:
		if (!usr->admin)
			goto denied;
		if (!++usr->admin)
			usr->admin = 1;
		error_info.trace = -strtol(s, &t, 0);
		message((error_info.trace, "%s", fmtident(id)));
		break;
	case AT_INFO:
		error(ERROR_OUTPUT|0, con->fd, "at service daemon pid %ld user %s", state->con[0].id.pid, fmtuid(state->admin[0]));
		error(ERROR_OUTPUT|0, con->fd, "QUE TOTAL SUB RUN MAX USR PRI  LOAD  WAIT ACCESS");
		visit.state = state;
		visit.con = con;
		if (que)
			listqueue(NiL, que, &visit);
		else
			dtwalk(state->table.queue.handle, listqueue, &visit);
		break;
	case AT_JOB:
		if (!que)
			que = state->queue;
		if (!(own = permit(que, usr)))
			goto noqueue;
		if (own->user->pending >= HOG)
		{
			error(ERROR_OUTPUT|2, con->fd, "%s: hog", own->user->name);
			return -1;
		}
		skip = strtol(s, &t, 10);
		if (*t++ != ' ' || !state->init && skip > n)
		{
			error(ERROR_OUTPUT|2, con->fd, "garbled job message");
			return -1;
		}
		for (s = t; *s && *s != '\n'; s++);
		h = s + 1;
		if (!(job = newof(0, Job_t, 1, s - t)))
		{
			error(ERROR_SYSTEM|ERROR_OUTPUT|2, con->fd, "out of space [job]");
			return -1;
		}
		if (state->sequence < cs.time)
			state->sequence = cs.time;
		if (data)
			sfsprintf(job->name, sizeof(job->name), "%s", data);
		else
			sfsprintf(job->name, sizeof(job->name), "%..36lu.%..36lu.%..36lu", con->id.uid, con->id.gid, state->sequence++);
		s = (char*)memcpy(job->data, t, s - t);
		job->start = cs.time;
		job->shell = s;
		while (*s)
			if (*s++ == ' ')
			{
				*(s - 1) = 0;
				break;
			}
		job->id = con->id;
		job->queue = que;
		job->owner = own;
		job->mail = mail;
		while (c = *s++)
		{
			switch (c)
			{
			case AT_LABEL:
				t = job->label;
				for (t = job->label; *s && !isspace(*s); s++)
					if (t < &job->label[sizeof(job->label)-1])
						*t++ = *s;
				if (*s)
					s++;
				continue;
			case AT_TIME:
				job->start = strtol(s, &t, 0);
				s = t;
				if (*s == ' ')
					s++;
				if (*s)
				{
					job->repeat = s;
					if (*s == '+')
					{
						while (*++s && !isspace(*s));
						while (isspace(*s))
							s++;
					}
				}
				job->period = s;
				break;
			default:
				break;
			}
			break;
		}
		if (state->init && job->repeat)
			job->start = tmdate(job->repeat, NiL, NiL);
		if (!state->init && !*(t = job->label))
		{
			m = 0;
			t = b + skip;
			if (*t == ':')
			{
				while (isspace(*++t));
				if (u = strchr(t, ';'))
				{
					while (u > t && isspace(*(u - 1)))
						u--;
					m = u - t;
				}
			}
			if (m)
			{
				if (m >= sizeof(job->label))
					m = sizeof(job->label) - 1;
				memcpy(job->label, t, m);
				job->label[m] = 0;
			}
			else
			{
				for (t = b + skip; *t; t++)
					if (isalnum(*t))
					{
						u = t;
						while (isalnum(*++t));
						c = t - u;
						if (c == 3 && strneq(u, "for", 3) || c == 2 && strneq(u, "if", 2) || c == 5 && strneq(u, "while", 5))
							continue;
						break;
					}
				m = 0;
				x = -1;
				while (c = *t++)
				{
					if (isalnum(c))
					{
						if (x)
						{
							if (x > 0)
							{
								w = sfstrtell(state->tmp);
								if (++m >= sizeof(job->label))
									break;
								sfputc(state->tmp, '.');
							}
							x = 0;
						}
						if (++m >= sizeof(job->label))
							break;
						sfputc(state->tmp, c);
					}
					else if (!x)
						x = 1;
				}
				if (m >= sizeof(job->label))
					sfstrseek(state->tmp, w, SEEK_SET);
				if (!(t = sfstruse(state->tmp)))
					error(ERROR_SYSTEM|3, "out of space");
				strcpy(job->label, t);
			}
		}
		submit(state, job);
		if (!state->init)
		{
			if (!(sp = sfopen(NiL, job->name, "w")))
				goto noexec;
			chmod(job->name, AT_JOB_MODE);
			sfprintf(sp, "#%c%s %c0 %s %c%s %c%lu", AT_QUEUE, job->queue->name, AT_JOB, job->shell, AT_LABEL, t, AT_TIME, job->start);
			if (job->repeat)
				sfprintf(sp, " %s", job->repeat);
			sfputc(sp, '\n');
			c = h - b;
			b += c;
			n -= c;
			skip -= c;
			sfprintf(sp, "tmp=/tmp/at$$\ntrap \"rm -f $tmp\" 0 1 2 3 15\n");
			sfprintf(sp, "{\necho at job %s exec $(date)\n{\n", job->name);
			sfprintf(sp, "export");
			x = 0;
			for (c = 0; c < elementsof(export); c++)
				if ((s = getenv(export[c])) && *s)
				{
					neqv(sp, export[c], s);
					x = 1;
				}
			if (que->home)
			{
				b += skip;
				n -= skip;
				neqv(sp, "HOME", job->owner->user->home);
				neqv(sp, "LOGNAME", job->owner->user->name);
				neqv(sp, "USER", job->owner->user->name);
				neqv(sp, "PATH", pathbin());
				neqv(sp, "SHELL", job->shell);
				sfputr(sp, "\ncd \"$HOME\"\n", -1);
				x = 1;
			}
			sfputr(sp, x ? "" : " HOME", '\n');
			b[n - 1] = '\n';
			if (sfwrite(sp, b, n) != n)
				goto noexec;
			sfprintf(sp, "\n} </dev/null\necho at job %s exit $(date) status $?\n} >$tmp 2>&1\n", job->name);
			if (!job->mail)
				sfprintf(sp, "test \"$(wc -l < $tmp)\" -le 2 || ");
			sfprintf(sp, "mailx -s \"at job status\" %s < $tmp\n", job->owner->user->name);
			if (sfclose(sp))
			{
				sp = 0;
				goto noexec;
			}
			error(ERROR_OUTPUT|0, con->fd, "job %s at %s%s%s", job->name, fmttime("%a %b %e %T %Y", job->start), job->repeat ? " repeat " : "", job->period);
			schedule(state);
		}
		break;
	case AT_LIST:
	case AT_REMOVE:
	case AT_STATUS:
		if (*s++ != ' ')
			s = 0;
		m = 0;
		for (job = (Job_t*)dtfirst(state->table.job.handle); job; job = next)
		{
			next = (Job_t*)dtnext(state->table.job.handle, job);
			if ((!que || que == job->queue) && (s && strmatch(job->name, s) || !s && (admin || con->id.uid == job->id.uid)) && job->owner->allow)
				switch (c)
				{
				case AT_LIST:
					error(ERROR_OUTPUT|0, con->fd, "%s\t%s", job->name, fmttime("%a %b %e %T %Y", job->start));
					break;
				case AT_STATUS:
					if (!m++)
						error(ERROR_OUTPUT|0, con->fd, "JOB                   LABEL        PID   Q USER     START               REPEAT");
					error(ERROR_OUTPUT|0, con->fd, "%-21s %-*s%7d %-1s %.-8s %s %s", job->name, sizeof(job->label), job->label, job->pid, job->queue->name, job->owner->user->name, fmttime(AT_TIME_FORMAT, job->start), job->period);
					break;
				case AT_REMOVE:
					if (con->id.uid == job->id.uid || !con->id.uid)
					{
						drop(state, job);
						m++;
					}
					else
						error(ERROR_OUTPUT|2, con->fd, "%s: only %s can remove this job", job->name, fmtuid(job->id.uid));
					break;
				}
		}
		if (s && !m)
		{
			error(ERROR_OUTPUT|0, con->fd, "%s: no matching jobs", s ? s : "*");
			return -1;
		}
		break;
	case AT_LOG:
		sfprintf(state->tmp, "%s/%s", state->pwd, AT_LOG_FILE);
		if (!(s = sfstruse(state->tmp)))
			error(ERROR_SYSTEM|3, "out of space");
		pathcanon(s, 0, 0);
		error(ERROR_OUTPUT|0, con->fd, "%s", s);
		break;
	case AT_QUIT:
		if (!usr->admin)
			goto denied;
		if (!++usr->admin)
			usr->admin = 1;
		error(0, "daemon quit by %s", usr->name);
		exit(0);
		break;
	case AT_UPDATE:
		if (!usr->admin)
			goto denied;
		if (!++usr->admin)
			usr->admin = 1;
		if (*s)
			queue(state, s);
		update(state);
		break;
	case AT_USER:
		error(ERROR_OUTPUT|0, con->fd, "USER      ADMIN TOTAL SUB RUN HOME");
		for (usr = (User_t*)dtfirst(state->table.user.handle); usr; usr = (User_t*)dtnext(state->table.user.handle, usr))
			if (admin || con->id.uid == usr->uid)
				error(ERROR_OUTPUT|0, con->fd, "%-9.9s %5lu %5lu %3d %3d %s", usr->name, usr->admin, usr->total, usr->pending, usr->running, usr->home);
		break;
	case AT_VERSION:
		error(ERROR_OUTPUT|0, con->fd, "%s", fmtident(id));
		break;
	default:
		error(ERROR_OUTPUT|2, con->fd, "%c: unknown command", *(s - 1));
		return -1;
	}
	return 0;
 denied:
	error(ERROR_OUTPUT|2, con->fd, "%s: access denied", usr->name);
	return -1;
 noexec:
	if (sp)
		sfclose(sp);
	error(ERROR_SYSTEM|ERROR_OUTPUT|2, con->fd, "%s: cannot save action", job->name);
	drop(state, job);
	return -1;
 noqueue:
	error(ERROR_OUTPUT|2, con->fd, "%s: queue %s access denied", usr->name, que->name);
	return -1;
}

/*
 * order job by <start,name>
 */

static int
order(Dt_t* dt, void* a, void* b, Dtdisc_t* disc)
{
	register long	r1;
	register int	r2;

	NoP(dt);
	NoP(disc);
	if (!(r2 = strcmp((char*)a + sizeof(unsigned long), (char*)b + sizeof(unsigned long))))
		return 0;
	if (!(r1 = *(unsigned long*)a - *(unsigned long*)b))
		return r2;
	return r1;
}

static unsigned long	rollover;	/* XXX stampwrite() has no discipline	*/

/*
 * commit to the log file
 * if its too big then rename to .old and start fresh
 */

static void
commit(void)
{
	int		rolled;
	int		fd;
	time_t		t;
	unsigned long	now;
	struct stat	st;
	char		buf[PATH_MAX];
	Tm_t*		tm;

	static int	commiting = 0;

	if (commiting++)
	{
		commiting--;
		return;
	}
	now = NOW;
	if (!rollover)
	{
		t = stat(AT_LOG_FILE, &st) ? now : st.st_mtime;
		tm = tmmake(&t);
		tm->tm_mon++;
		tm->tm_mday = 1;
		tm->tm_hour = 0;
		tm->tm_min = 0;
		tm->tm_sec = 0;
		rollover = tmtime(tm, TM_LOCALZONE);
	}
	if (now >= rollover)
	{
		rolled = 1;
		error(0, "log file rollover");
		sfsprintf(buf, sizeof(buf), "%s.old", AT_LOG_FILE);
		remove(buf);
		if (rename(AT_LOG_FILE, buf))
			error(ERROR_SYSTEM|AT_STRICT, "%s: cannot rename log file to %s", AT_LOG_FILE, buf);
		t = rollover;
		tm = tmmake(&t);
		tm->tm_mon++;
		rollover = tmtime(tm, TM_LOCALZONE);
	}
	else
		rolled = 0;
	if ((fd = open(AT_LOG_FILE, O_CREAT|O_WRONLY|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0 || fd != 2 && (dup2(fd, 2) != 2 || close(fd)))
		error(ERROR_SYSTEM|AT_STRICT, "%s: cannot append to log file", AT_LOG_FILE);
	if (rolled)
		error(0, "log file rollover");
	commiting--;
}

/*
 * prepend current date-time to buffer on fd==2
 * and drop the initial command label if any
 */

static ssize_t
stampwrite(int fd, const void* buf, size_t n)
{
	register char*		s;
	register int		i;
	register ssize_t	r;
	register ssize_t	z;
	unsigned long		now;

	r = 0;
	if (rollover)
	{
		now = NOW;
		if (now >= rollover)
			commit();
	}
	else if (!now)
		now = NOW;
	if (fd == 2 && (s = fmttime(AT_TIME_FORMAT, now)))
	{
		i = strlen(s);
		s[i++] = ' ';
		if ((z = write(fd, s, i)) < 0)
			r = -1;
		else
			r += z;
		for (s = (char*)buf; s < ((char*)buf + n - 1) && !isspace(*s); s++)
			if (*s == ':')
			{
				while (++s < ((char*)buf + n - 1) && isspace(*s));
				n -= s - (char*)buf;
				buf = (void*)s;
				break;
			}
	}
	if ((z = write(fd, buf, n)) < 0)
		r = -1;
	else if (r >= 0)
		r += z;
	return r;
}

/*
 * service a request
 */

static int
request(Css_t* css, Cssfd_t* fp, Cssdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;
	register Connection_t*	con = state->con + fp->fd;
	register char*		s;
	char*			t;
	unsigned long		n;
	int			c;
	
	if (fp->status != CS_POLL_READ)
		return -1;
	if ((c = csread(state->css->state, fp->fd, state->buf, 7, CS_EXACT|CS_RESTART)) != 7)
	{
		if (c)
			error(ERROR_OUTPUT|2, con->fd, "message size read error");
		return -1;
	}
	state->buf[6] = 0;
	if ((n = strtol(state->buf + 1, &t, 10)) <= 0 || *t)
	{
		error(ERROR_OUTPUT|2, con->fd, "invalid message size");
		return -1;
	}
	if (n > state->bufsiz)
	{
		if (n > INT_MAX || (c = roundof(n, PATH_MAX)) <= 0 || !(s = newof(state->buf, char, c, 0)))
		{
			error(ERROR_OUTPUT|2, con->fd, "%ld: message too big", n);
			return -1;
		}
		state->buf = s;
		state->bufsiz = c;
	}
	if (csread(state->css->state, fp->fd, s = state->buf, n, CS_EXACT|CS_RESTART) != n)
	{
		error(ERROR_OUTPUT|2, con->fd, "message body read error");
		return -1;
	}
	s[n - 1] = 0;
	command(state, con, s, n, NiL);
	return -1;
}

/*
 * initialize the state
 */

static int
init(const char* path)
{
	register State_t*	state;
	register DIR*		dir;
	register struct dirent*	ent;
	char*			s;
	char*			b;
	Sfio_t*			sp;
	int			i;
	unsigned long		limited;
	unsigned long*		ap;
	struct stat		ds;
	struct stat		hs;
	struct stat		js;
	struct stat		xs;

	umask(S_IWGRP|S_IWOTH);
	if ((i = (int)strtol(astconf("OPEN_MAX", NiL, NiL), NiL, 0)) < 20)
		i = 20;
	if (!(state = newof(0, State_t, 1, (i - 1) * sizeof(Connection_t))))
		error(ERROR_SYSTEM|3, "out of space [state]");
	state->bufsiz = 4 * PATH_MAX;
	if (!(state->buf = newof(0, char, state->bufsiz, 0)))
		error(ERROR_SYSTEM|3, "out of space [buf]");
	if (!(state->tmp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [tmp]");
	ap = state->admin;
	*ap++ = state->con[0].id.uid = geteuid();
	*ap++ = 0;
	s = state->buf;
	if (!pathpath(AT_JOB_DIR, "", PATH_ABSOLUTE|PATH_EXECUTE, s, state->bufsiz) || lstat(s, &ds) || !AT_DIR_OK(&ds))
		error(ERROR_SYSTEM|3, "%s: job directory not found", AT_JOB_DIR);
	if (ds.st_uid != state->admin[0])
		error(ERROR_SYSTEM|3, "%s: job directory uid %d != effective uid %d", s, ds.st_uid, state->admin[0]);
	state->disc.version = CSS_VERSION;
	state->disc.flags = CSS_DAEMON|CSS_ERROR|CSS_INTERRUPT|CSS_WAKEUP;
	state->disc.errorf = errorf;
	state->disc.acceptf = client;
	state->disc.actionf = request;
	state->disc.exceptf = exception;
	if (!(state->css = cssopen(path, &state->disc)))
		return -1;
	state->con[0].id.gid = getegid();
	state->con[0].id.pid = getpid();
	state->con[0].id.hid = csaddr(state->css->state, NiL);
	state->con[0].fd = 2;
	state->init = 1;
	b = s + strlen(s);
	*b++ = '/';
	sfsprintf(b, state->bufsiz - (b - s), "%s", csname(state->css->state, 0L));
	if (lstat(s, &hs) && (mkdir(s, AT_DIR_MODE) || chmod(s, AT_DIR_MODE) || lstat(s, &hs)))
		error(ERROR_SYSTEM|3, "%s: cannot create job host directory", s);
	if (!AT_DIR_OK(&hs) || ds.st_uid != hs.st_uid || chdir(s))
		error(ERROR_SYSTEM|3, "%s: invalid job host directory %s [dir.uid=%d host.uid=%d]", s, fmtmode(hs.st_mode, 0), ds.st_uid, hs.st_uid);
	if (!(state->pwd = strdup(s)))
		error(ERROR_SYSTEM|3, "out of space [pwd]");
	if (hs.st_uid != state->admin[0])
		error(AT_STRICT, "%s: directory owner %s does not match daemon %s", s, fmtuid(hs.st_uid), fmtuid(state->admin[0]));
	sfsprintf(s, state->bufsiz, "%s/%s", state->pwd, AT_EXEC_FILE);
	pathcanon(s, 0, 0);
	if (lstat(s, &xs))
		error(ERROR_SYSTEM|3, "%s: job exec command not found", s);
	if (!S_ISREG(xs.st_mode))
		error(3, "%s: invalid mode %s -- regular file expected", s, fmtmode(xs.st_mode, 0));
	if ((xs.st_mode&(S_IXUSR|S_IXGRP|S_IWOTH|S_IXOTH)) != (S_IXUSR|S_IXGRP|S_IXOTH))
		error(3, "%s: invalid mode %s", s, fmtmode(xs.st_mode, 0));
	if (!(xs.st_mode&S_ISUID) && geteuid() != 0 && geteuid() != xs.st_uid)
		error(3, "%s: invalid euid %d -- %d expected", s, geteuid(), xs.st_uid);
#if 0
	if (!AT_EXEC_OK(&ds, &xs))
		error(3, "%s: invalid [ mode=%04o uid=%d euid=%d t1=%04o t2=%04o==%04o ]", s, xs.st_mode, xs.st_uid, geteuid(), S_ISREG(xs.st_mode), xs.st_mode&(S_IXUSR|S_IXGRP|S_IWOTH|S_IXOTH), (S_IXUSR|S_IXGRP|S_IXOTH));
#endif
	*ap++ = ds.st_uid;
	*ap = xs.st_uid;
	limited = (xs.st_mode & S_ISUID) ? (unsigned long)xs.st_uid : state->admin[0];
	if (!(state->atx = strdup(s)))
		error(ERROR_SYSTEM|3, "out of space [atx]");
	state->table.job.discipline.key = offsetof(Job_t, start);
	state->table.job.discipline.comparf = order;
	if (!(state->table.job.handle = dtopen(&state->table.job.discipline, Dtoset)))
		error(ERROR_SYSTEM|3, "out of space [job table]");
	state->table.owner.discipline.key = offsetof(Owner_t, user);
	state->table.owner.discipline.size = sizeof(User_t*);
	state->table.pid.discipline.key = offsetof(Job_t, pid);
	state->table.pid.discipline.size = sizeof(long);
	state->table.pid.discipline.link = offsetof(Job_t, bypid);
	if (!(state->table.pid.handle = dtopen(&state->table.pid.discipline, Dtset)))
		error(ERROR_SYSTEM|3, "out of space [pid table]");
	state->table.queue.discipline.key = offsetof(Queue_t, name);
	if (!(state->table.queue.handle = dtopen(&state->table.queue.discipline, Dtoset)))
		error(ERROR_SYSTEM|3, "out of space [queue table]");
	state->table.uid.discipline.key = offsetof(User_t, uid);
	state->table.uid.discipline.size = sizeof(long);
	state->table.uid.discipline.link = offsetof(User_t, byuid);
	if (!(state->table.uid.handle = dtopen(&state->table.uid.discipline, Dtset)))
		error(ERROR_SYSTEM|3, "out of space [uid table]");
	state->table.user.discipline.key = offsetof(User_t, name);
	if (!(state->table.user.handle = dtopen(&state->table.user.discipline, Dtoset)))
		error(ERROR_SYSTEM|3, "out of space [user table]");
	for (i = 0; i < elementsof(queuedefs); i++)
		queue(state, strcpy(state->buf, queuedefs[i]));
	commit();
	error(0, "daemon restart pid %ld user %s", state->con[0].id.pid, fmtuid(state->admin[0]));
	if (limited)
		error(0, "service limited to user %s", fmtuid(limited));

	/*
	 * update the queue definitions
	 */

	update(state);

	/*
	 * resubmit old jobs
	 */

	if (dir = opendir("."))
	{
		while (ent = readdir(dir))
			if (lstat(s = ent->d_name, &js))
			{
				error(0, "cannot stat old job %s", s);
				remove(s);
			}
			else if (!S_ISREG(js.st_mode))
			{
				if (!streq(s, ".") && !streq(s, ".."))
				{
					error(0, "invalid old job %s type %s rejected", s, fmtmode(js.st_mode, 0));
					if (S_ISDIR(js.st_mode))
						rmdir(s);
					else
						remove(s);
				}
			}
			else if (sfsscanf(s, "%..36lu.%..36lu.%..36lu", &state->con[0].id.uid, &state->con[0].id.gid, &cs.time) != 3)
			{
				error(0, "invalid old job %s name rejected", s);
				remove(s);
			}
			else if (!AT_OLD_OK(&hs, &js))
			{
				error(0, "invalid old job %s mode %s rejected [dir.uid=%d job.uid=%d]", s, fmtmode(js.st_mode, 0), hs.st_uid, js.st_uid);
				remove(s);
			}
			else if (!(sp = sfopen(NiL, s, "r")))
			{
				error(0, "cannot read old job %s", s);
				remove(s);
			}
			else
			{
				if (b = (char*)sfreserve(sp, SF_UNBOUND, SF_LOCKR))
					command(state, state->con, b, sfvalue(sp), s);
				sfclose(sp);
			}
		closedir(dir);
	}
	CSTIME();
	state->con[0].id.uid = geteuid();
	state->con[0].id.gid = getegid();
	state->init = 0;
	schedule(state);
	return 0;
}

int
main(int argc, char** argv)
{
	char*	path;
	pid_t	pid;
	int	status;

	NoP(argc);
	NoP(argv);
	error_info.id = "at.svc";
	error_info.write = stampwrite;
	if ((path = argv[1]) && !(path = strdup(path)))
		path = argv[1];

	/*
	 * monitor the daemon and restart if it dies
	 */

	csdaemon(&cs, (1<<0)|(1<<1)|(1<<2));
	for (;;)
	{
		if ((pid = fork()) <= 0)
		{
			if (!init(path))
				csspoll(CS_NEVER, 0);
			return 1;
		}
		while (waitpid(pid, &status, 0) != pid);
		if (!status)
			break;
		sleep(60);
	}
	return 0;
}
