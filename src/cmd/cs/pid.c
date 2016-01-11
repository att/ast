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
 * local pid monitor service
 */

static const char id[] = "\n@(#)$Id: cs.pid (AT&T Bell Laboratories) 11/11/93 $\0\n";

#include <ast.h>
#include <cs.h>
#include <error.h>
#include <hash.h>
#include <sig.h>

#define DECAY_MIN	(EXPIRE_MIN)
#define DECAY		(EXPIRE<<3)
#define DECAY_MAX	(EXPIRE_MAX<<3)
#define EXPIRE_MIN	(1)
#define EXPIRE		EXPIRE_MIN
#define EXPIRE_MAX	(60)

typedef struct notify Notify_t;

struct notify				/* notification address list	*/
{
	Notify_t*	next;		/* next in list			*/
	Cs_addr_t	addr;		/* notification address		*/
};

typedef struct				/* pid info			*/
{
	HASH_HEADER;
	Notify_t*	notify;		/* notification address list	*/
	int		decay;		/* expiration decay		*/
	unsigned long	expire;		/* kill poll expiration		*/
	pid_t		pid;		/* request pid			*/
} Pid_t;

typedef struct				/* server state			*/
{
	Hash_table_t*	pids;		/* Pid_t hash			*/
	int		decay;		/* max expiration decay		*/
	int		dormant;	/* no activity			*/
	int		expire;		/* expiration increment		*/
	unsigned long	active;		/* number of active pids	*/
	char		msg[1024];	/* msg text			*/
	char		buf[1024];	/* work buffer			*/
} State_t;

/*
 * initialize the state
 */

static void*
svc_init(void* handle, int fdmax)
{
	register State_t*	state = (State_t*)handle;

	NoP(fdmax);
	if (!(state->pids = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(pid_t), HASH_name, "pids", 0)))
		error(3, "out of space [pids]");
	state->decay = DECAY;
	state->dormant = 1;
	state->expire = EXPIRE;
	cswakeup(CS_SVC_DORMANT * 1000L);
	return(handle);
}

/*
 * service a request
 */

static int
svc_read(void* handle, int fd)
{
	register Pid_t*		pp;
	register Notify_t*	np;
	register Notify_t*	pn;
	register State_t*	state = (State_t*)handle;
	char*			m;
	char*			s;
	char*			e;
	int			n;
	int			c;
	pid_t			pid;
	Cs_addr_t		addr;
	Hash_position_t*	pos;

	if ((n = csfrom(fd, m = state->msg, sizeof(state->msg) - 1, &addr)) <= 0)
		return(-1);
	m[n] = 0;
	do
	{
		if (!(e = strchr(m, '\n')))
			return(-1);
		n = ++e - m;
		if ((pid = strtol(m, NiL, 0)) <= 1)
		{
			for (s = m; *s && (*s < '0' || *s > '9'); s++);
			switch (*m)
			{
			case 'd':
				if ((c = strtol(s, NiL, 0)) >= DECAY_MIN && c <= DECAY_MAX)
					state->decay = c;
				break;
			case 'e':
				if ((c = strtol(s, NiL, 0)) >= EXPIRE_MIN && c <= EXPIRE_MAX)
					state->expire = c;
				break;
			case 's':
			case '\n':
				m = state->buf;
				n = sfsprintf(m, sizeof(state->buf), "active=%d decay=%d expire=%d\n", state->active, state->decay, state->expire);
				if (pos = hashscan(state->pids, 0))
				{
					while (pp = (Pid_t*)hashnext(pos))
					{
						n += sfsprintf(m + n, sizeof(state->buf) - n, "%6d %ld", pp->pid, pp->expire - cs.time);
						for (np = pp->notify; np; np = np->next)
							n += sfsprintf(m + n, sizeof(state->buf) - n, " %s/%lu", csname(np->addr.addr[0]), np->addr.addr[1]);
						n += sfsprintf(m + n, sizeof(state->buf) - n, "\n");
					}
					hashdone(pos);
				}
				break;
			case 'v':
				m = state->buf;
				n = sfsprintf(m, sizeof(state->buf), "%s %s %u\n", id + 10, csname(0L), getpid());
				break;
			default:
				if (*m < '0' || *m > '9')
				{
					if (n > 0) m[n - 1] = 0;
					n = sfsprintf(state->buf, sizeof(state->buf), "%s [invalid command]\n", m);
					m = state->buf;
				}
				break;
			}
			csto(fd, m, n, &addr);
		}
		else if (pp = (Pid_t*)hashlook(state->pids, (char*)&pid, HASH_LOOKUP, NiL))
		{
			if (kill(pid, 0) && errno == ESRCH)
			{
				csto(fd, m, n, &addr);
				np = pp->notify;
				while (np)
				{
					csto(fd, m, n, &np->addr);
					pn = np;
					np = np->next;
					free(pn);
				}
				hashlook(state->pids, NiL, HASH_DELETE, NiL);
				if (!--state->active)
				{
					state->dormant = 1;
					cswakeup(CS_SVC_DORMANT * 1000L);
				}
			}
			else if (np = newof(0, Notify_t, 1, 0))
			{
				np->addr = addr;
				np->next = pp->notify;
				pp->notify = np;
			}
		}
		else if (kill(pid, 0) && errno == ESRCH)
			csto(fd, m, n, &addr);
		else if (pp = (Pid_t*)hashlook(state->pids, NiL, HASH_CREATE|HASH_SIZE(sizeof(Pid_t)), NiL))
		{
			pp->pid = pid;
			if (np = newof(0, Notify_t, 1, 0))
			{
				np->addr = addr;
				np->next = pp->notify;
				pp->notify = np;
				pp->expire = cs.time + (pp->decay = state->expire);
				if (!state->active++)
				{
					state->dormant = 0;
					cswakeup(pp->decay * 1000L);
				}
			}
		}
	} while (*(m = e));
	return(0);
}

/*
 * poll expired pids
 */

static int
svc_timeout(void* handle)
{
	register State_t*		state = (State_t*)handle;
	register Pid_t*			pp;
	register Hash_position_t*	pos;
	register Notify_t*		np;
	register Notify_t*		pn;
	int				n;
	unsigned long			wakeup;

	if (state->dormant)
		exit(0);
	wakeup = ~0;
	if (pos = hashscan(state->pids, 0))
	{
		while (pp = (Pid_t*)hashnext(pos))
		{
			if (pp->expire <= cs.time)
			{
				if (kill(pp->pid, 0) && errno == ESRCH)
				{
					n = sfsprintf(state->msg, sizeof(state->msg), "%u\n", pp->pid);
					np = pp->notify;
					while (np)
					{
						csto(0, state->msg, n, &np->addr);
						pn = np;
						np = np->next;
						free(pn);
					}
					hashlook(state->pids, (char*)&pp->pid, HASH_DELETE, NiL);
					state->active--;
					continue;
				}
				if (pp->decay < state->decay)
					pp->decay <<= 1;
				pp->expire = cs.time + pp->decay;
			}
			if (pp->expire < wakeup)
				wakeup = pp->expire;
		}
		hashdone(pos);
	}
	if (wakeup == ~0)
	{
		state->dormant = 1;
		wakeup = CS_SVC_DORMANT;
	}
	else wakeup -= cs.time;
	cswakeup(wakeup * 1000L);
	return(0);
}

int
main(int argc, char** argv)
{
	static State_t	state;

	NoP(argc);
	csserve(&state, argv[1], svc_init, NiL, NiL, svc_read, NiL, svc_timeout);
	exit(1);
}
