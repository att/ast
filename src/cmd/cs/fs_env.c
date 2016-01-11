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
 * /env nDFS file server
 * the top level requests are
 *
 * request
 *
 *	<op> <mount> <path> [pwd=<pwd>] [<name>=<value>] ...
 */

static const char id[] = "@(#)$Id: cs.fs_env (AT&T Research) 1997-05-05 $\0\n";

#ifndef DEBUG
#define DEBUG	1
#endif

#include <cs.h>
#include <msg.h>
#include <hashkey.h>
#include <ctype.h>
#include <error.h>
#include <debug.h>
#include <tok.h>

#define MAXIO	(4*1024)

#define CON	0
#define CMD	1
#define KEY	2
#define ACT	3

static const char*	state_name[] = { "CON", "CMD", "KEY", "ACT" };

typedef struct
{
	Cs_id_t		id;
	int		state;
	char		key[12];
	off_t		offset;
	size_t		size;
	char*		data;
} Connection_t;

typedef struct
{
	int		active;
	int		dormant;
	char*		clone;
	Connection_t	con[1];
} State_t;

static void*
svc_init(void* handle, int maxfd)
{
	register State_t*	state;
	register int		fd;

	NoP(handle);
	if (!(state = newof(0, State_t, 1, (maxfd - 1) * sizeof(Connection_t))))
		exit(1);
	cstimeout(CS_SVC_DORMANT * 1000L);
	if ((fd = csopen("/dev/tcp/local/normal", CS_OPEN_CREATE)) < 0)
		error(ERROR_SYSTEM|3, "cannot create clone connect stream");
	if (!(state->clone = strdup(cspath(fd, 0))))
		error(ERROR_SYSTEM|3, "out of space [clone]");
	state->con[fd].state = CON;
	csfd(fd, CS_POLL_READ);
	return (void*)state;
}

static int
svc_connect(void* handle, int fd, CSID* id, int clone, char** args)
{
	register State_t*	state = (State_t*)handle;

	NoP(fd);
	NoP(clone);
	NoP(args);
	state->active++;
	state->dormant = 0;
	state->con[fd].id = *id;
	state->con[fd].state = CMD;
	return 0;
}

/*
 * service a request
 */

static int
svc_read(void* handle, int fd)
{
	register State_t*	state = (State_t*)handle;
	register Connection_t*	con;
	register int		n;
	int			xd;
	int			err;
	long			ret;
	char*			op;
	char*			logical;
	char*			path;
	Cs_id_t			id;
	Msg_call_t		msg;
	struct stat		st;

	static char	buf[(3 * PATH_MAX) / 2 + 1];

	con = state->con + fd;
	message((-1, "fd=%d state=%s", fd, state_name[con->state]));
	switch (con->state)
	{
	case CMD:
		if ((n = csread(fd, buf, sizeof(buf), CS_LINE)) <= 1)
			goto drop;
		buf[n - 1] = 0;
		if (tokscan(buf, NiL, " %s %s %s ", &op, &logical, &path) < 1)
			goto nope;
		switch (strkey(op))
		{
		case HASHKEY5('d','e','b','u','g'):
			error_info.trace = -strtol(logical, NiL, 10);
			goto nope;
		case HASHKEY4('o','p','e','n'):
			message((-2, "op=%s path=%s", op, path));
			if (!(op = getenv(path)))
				goto nope;
			message((-2, "data=%s", op));
			n = sfsprintf(buf, sizeof(buf), "/#%s/#%s\n", path, state->clone);
			message((-2, "challenge `%-.*s'", n, buf));
			if (cswrite(fd, buf, n) != n)
				goto drop;
			return 0;
		case HASHKEY4('q','u','i','t'):
			exit(0);
			break;
		default:
			goto nope;
		}
		break;
	case KEY:
		if ((n = csread(fd, buf, sizeof(buf), CS_LINE)) <= 1)
			goto drop;
		buf[n - 1] = 0;
		if (n <= 2 || !(op = getenv(buf + 2)))
			goto drop;
		con->state = ACT;
		con->data = op;
		con->size = strlen(op) + 1;
		con->offset = 0;
		return 0;
	case ACT:
		if (msgrecv(fd, &msg) <= 0)
			goto drop;
		if (error_info.trace <= -4)
			msglist(sfstderr, &msg, 0, 0L);
		ret = 0;
		err = 0;
		op = 0;
		switch (msg.call)
		{
		case MSG_getdents:
			err = ENOSYS;
			break;
		case MSG_read:
			if ((long)msg.argv[2].number < 0)
				err = EINVAL;
			else if ((n = con->size - con->offset) > 0)
			{
				if (n > MAXIO)
					n = MAXIO;
				if (n > msg.argv[2].number)
					n = msg.argv[2].number;
				if (n > 0)
				{
					op = con->data + con->offset;
					con->offset += n;
					ret = n;
				}
			}
			break;
		case MSG_seek:
			switch (msg.argv[2].number)
			{
			case SEEK_SET:
				n = msg.argv[1].number;
				break;
			case SEEK_CUR:
				n = con->offset + msg.argv[1].number;
				break;
			case SEEK_END:
				n = con->size + msg.argv[1].number;
				break;
			default:
				n = -1;
				break;
			}
			if (n < 0)
				err = EINVAL;
			else
				ret = con->offset = n;
			break;
		case MSG_stat:
			memset(&st, 0, sizeof(st));
			st.st_size = con->size;
			st.st_mode = S_IRUSR|S_IWUSR;
			st.st_mtime = st.st_atime = st.st_ctime = cs.time;
			op = (char*)&st;
			break;
		case MSG_write:
			err = ENOSYS;
			break;
		default:
			err = ENOSYS;
			break;
		}
		if (msgsend(fd, &msg, msg.call, err ? -1 : ret, err, op) <= 0)
			goto drop;
		return 0;
	case CON:
		if (csrecv(fd, &id, &xd, 1) != 1)
			goto drop;
		con = state->con + xd;
		con->id = id;
		con->state = KEY;
		csfd(xd, CS_POLL_READ);
		state->active++;
		return 0;
	default:
		goto drop;
	}
 nope:
	if (cswrite(fd, "\n", 1) == 1)
		return 0;
 drop:
	state->active--;
	message((-1, "drop fd=%d", fd));
	return -1;
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
	return 0;
}

int
main(int argc, char** argv)
{
	static State_t	state;

	NoP(argc);
	cstimeout(CS_SVC_DORMANT * 1000L);
	csserve(&state, argv[1], svc_init, NiL, svc_connect, svc_read, NiL, svc_timeout);
	exit(1);
}
