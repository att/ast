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
 * AT&T Research
 *
 * obsolete generic server state machine
 * <css.h> provides a discipline interface with multiple servers
 */

#include "csslib.h"

struct Server_s
{
	Cssdisc_t	disc;
	Css_t*		css;
	void*		handle;
	int		(*done)(void*, int);
	int		(*con)(void*, int, Csid_t*, int, char**);
	int		(*rd)(void*, int);
	int		(*wr)(void*, int);
	int		(*to)(void*);
};

static int
acceptf(Css_t* css, Cssfd_t* fp, Csid_t* ip, char** av, Cssdisc_t* disc)
{
	register Server_t*	server = (Server_t*)disc;

	return (*server->con)(server->handle, fp->fd, ip, 0, av) ? -1 : fp->fd;
}

static int
actionf(Css_t* css, Cssfd_t* fp, Cssdisc_t* disc)
{
	register Server_t*	server = (Server_t*)disc;

	switch (fp->status)
	{
	case CS_POLL_READ:
		if (server->rd)
			return (*server->rd)(server->handle, fp->fd) < 0 ? -1 : 1;
		break;
	case CS_POLL_WRITE:
		if (server->wr)
			return (*server->wr)(server->handle, fp->fd) < 0 ? -1 : 1;
		break;
	}
	return 0;
}

static int
exceptf(Css_t* css, unsigned long op, unsigned long arg, Cssdisc_t* disc)
{
	register Server_t*	server = (Server_t*)disc;

	switch (op)
	{
	case CSS_CLOSE:
		if (server->done)
			(*server->done)(server->handle, 0);
		return 0;
	case CSS_INTERRUPT:
		if (server->done && !(*server->done)(server->handle, EXIT_TERM(arg)))
			return 1;
		error(ERROR_SYSTEM|3, "%s: interrupt exit", fmtsignal(arg));
		return -1;
	case CSS_DORMANT:
	case CSS_TIMEOUT:
	case CSS_WAKEUP:
		return !server->to ? 0 : (*server->to)(server->handle) < 0 ? -1 : 1;
	}
	error(ERROR_SYSTEM|3, "poll error [op=%lu arg=%lu]", op, arg);
	return -1;
}

int
csfd(Cs_t* state, int fd, int op)
{
	return !state->server || cssfd(state->server->css, fd, op) ? -1 : 0;
}

/*
 * csserve() wakeup and timeout are mutually exclusive
 */

unsigned long
cstimeout(register Cs_t* state, unsigned long ms)
{
	unsigned long	rv;

	if (!state->server)
		return CS_NEVER;
	state->server->css->disc->wakeup = 0;
	rv = state->server->css->disc->timeout;
	state->server->css->disc->timeout = ms;
	return rv;
}

/*
 * csserve() wakeup and timeout are mutually exclusive
 */

unsigned long
cswakeup(Cs_t* state, unsigned long ms)
{
	unsigned long	rv;

	if (!state->server)
		return CS_NEVER;
	state->server->css->disc->timeout = 0;
	rv = state->server->css->disc->wakeup;
	state->server->css->disc->wakeup = ms;
	return rv;
}

/*
 * server state machine
 */

void
csserve(Cs_t* state, void* handle, const char* path, void* (*init)(void*, int), int (*done)(void*, int), int (*con)(void*, int, Csid_t*, int, char**), int (*rd)(void*, int), int (*wr)(void*, int), int (*to)(void*))
{
	register Server_t*	server;

	if (!con && !rd)
		error(ERROR_PANIC, "a connect or read handler must be supplied");
	if (!(server = newof(0, Server_t, 1, 0)))
		error(ERROR_SYSTEM|3, "out of space");
	state->server = server;
	server->disc.version = CSS_VERSION;
	server->disc.flags = CSS_DAEMON|CSS_LOG|CSS_CLOSE|CSS_ERROR|CSS_INTERRUPT|CSS_TIMEOUT|CSS_WAKEUP;
	server->handle = handle;
	server->con = con;
	if (server->con)
		server->disc.acceptf = acceptf;
	server->rd = rd;
	server->wr = wr;
	if (server->rd || server->wr)
		server->disc.actionf = actionf;
	server->to = to;
	server->disc.errorf = errorf;
	server->disc.exceptf = exceptf;
	server->done = done;
	close(0);
	if (!(server->css = cssopen(path, (Cssdisc_t*)server)))
		exit(1);
	error_info.id = server->css->service;
	state->id = server->css->id;
	state->cs = server->css->path;
	state->control = state->mount + (server->css->control - server->css->mount);
	strcpy(state->mount, server->css->mount);
#ifdef SIGCHLD
	if (!done)
		signal(SIGCHLD, SIG_DFL);
#endif

	/*
	 * we   the
	 *   are  
	 *         ser
	 *            ver
	 *
	 * in background
	 * no controlling tty
	 * pwd is the service data directory
	 * stdin is the connect stream
	 * stdout is /dev/null
	 * stderr is CS_MNT_LOG in the service data directory
	 * umask matches service mode
	 */

	if (init)
		server->handle = (*init)(server->handle, server->css->fdmax);
	csspoll(CS_NEVER, 0);
}

int
_cs_fd(int fd, int op)
{
	return csfd(&cs, fd, op);
}

void
_cs_serve(void* handle, const char* path, void* (*init)(void*, int), int (*done)(void*, int), int (*con)(void*, int, Csid_t*, int, char**), int (*rd)(void*, int), int (*wr)(void*, int), int (*to)(void*))
{
	csserve(&cs, handle, path, init, done, con, rd, wr, to);
}

unsigned long
_cs_timeout(unsigned long ms)
{
	return cstimeout(&cs, ms);
}

unsigned long
_cs_wakeup(unsigned long ms)
{
	return cswakeup(&cs, ms);
}
