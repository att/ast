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
 * open cs paths and return to sender
 * the requests are
 *
 * request
 *
 *	<op> <mount> <path> [pwd=<pwd>] [<name>=<value>] ...
 *
 * <mount> == /dev/fd translates fd to path if possible
 */

static const char id[] = "@(#)$Id: cs.nam (AT&T Research) 1996-02-29 $\0\n";

#include <cs.h>
#include <hashkey.h>
#include <ctype.h>
#include <error.h>
#include <tok.h>

#define DEVFD	"/dev/fd\n"

typedef struct
{
	int		active;
	int		dormant;
} State_t;

static int
svc_connect(void* handle, int fd, CSID* id, int clone, char** args)
{
	register State_t*	state = (State_t*)handle;

	NoP(fd);
	NoP(id);
	NoP(clone);
	NoP(args);
	state->active++;
	state->dormant = 0;
	return(0);
}

/*
 * service a request
 */

static int
svc_read(void* handle, int fd)
{
	State_t*	state = (State_t*)handle;
	register char*	b;
	register int	n;
	int		getfd;
	int		ud;
	char*		msg;
	char*		op;
	char*		logical;
	char*		path;
	char*		name;
	char*		value;

	static char	buf[(3 * PATH_MAX) / 2 + 1];

	if ((n = csread(fd, buf, sizeof(buf), CS_LINE)) <= 1)
		goto drop;
	buf[n - 1] = 0;
	msg = buf;
	if (tokscan(msg, &msg, " %s %s %s ", &op, &logical, &path) < 1)
		goto nope;
	switch (strkey(op))
	{
	case HASHKEY5('d','e','b','u','g'):
		error_info.trace = -strtol(logical, NiL, 10);
		goto nope;
	case HASHKEY4('o','p','e','n'):
		getfd = 1;
		break;
	case HASHKEY4('p','a','t','h'):
		if (csrecv(fd, NiL, &ud, 1) != 1)
			goto drop;
		b = cspath(ud, CS_PATH_NAME);
		close(ud);
		if (!b)
			goto nope;
		n = strlen(b);
		b[n++] = '\n';
		if (cswrite(fd, b, n) != n)
			goto drop;
		return(0);
	case HASHKEY4('q','u','i','t'):
		exit(0);
		break;
	case HASHKEY4('s','t','a','t'):
		getfd = 0;
		break;
	default:
		goto nope;
	}
	while (tokscan(msg, &msg, " %s=%s ", &name, &value) == 2)
		switch (strkey(name))
		{
		case HASHKEY3('p','w','d'):
			chdir(value);
			break;
		}
	if (!getfd)
		goto nope;
	if (*path)
		do
		{
			n = *--path;
			*path = '/';
		} while (n);
	if ((ud = csopen(logical, CS_OPEN_READ)) >= 0)
	{
		if (cswrite(fd, DEVFD, sizeof(DEVFD) - 1) != sizeof(DEVFD) - 1 || cssend(fd, &ud, 1))
		{
			close(ud);
			goto drop;
		}
		close(ud);
		return(0);
	}
 nope:
	if (cswrite(fd, "\n", 1) == 1)
		return(0);
 drop:
	state->active--;
	return(-1);
}

/*
 * exit if inactive on timeout
 */

static int
svc_timeout(void* handle)
{
	State_t*	state = (State_t*)handle;

	if (!state->active)
	{
		if (state->dormant)
			exit(0);
		state->dormant = 1;
	}
	return(0);
}

int
main(int argc, char** argv)
{
	static State_t	state;

	NoP(argc);
	cstimeout(CS_SVC_DORMANT * 1000L);
	csserve(&state, argv[1], NiL, NiL, svc_connect, svc_read, NiL, svc_timeout);
	exit(1);
}
