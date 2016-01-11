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
 * open|initiate /dev/fdp/local/<service>/user
 * coded for bootstrap
 */

#include "cslib.h"

#include <wait.h>

#define DEVLOCAL	"/dev/fdp/local/"

#ifdef SIGCHLD

static int	children;

static void
child(int sig)
{
	NoP(sig);
	children++;
}

#endif

static int
initiate(Cs_t* state, const char* svc, char* cmd)
{
	pid_t	pid;
	pid_t	n;
	char*	av[3];

#ifdef SIGCHLD
	Handler_t	fun;

	children = 0;
	if ((fun = signal(SIGCHLD, child)) == SIG_DFL) signal(SIGCHLD, fun);
	else if (children) children++;
#endif
	pathcanon(cmd, 0, 0);
	av[0] = cmd;
	av[1] = (char*)svc;
	av[2] = 0;
	if ((pid = spawnveg(av[0], av, environ, 0)) == -1)
	{
		messagef((state->id, NiL, -1, "local: %s: cannot initiate %s", svc, cmd));
		return -1;
	}
	while ((n = waitpid(pid, NiL, 0)) == -1 && errno == EINTR);
#ifdef SIGCHLD
	if (fun != SIG_DFL)
	{
		signal(SIGCHLD, fun);
		if (fun != SIG_IGN)
			while (--children > 0)
				(*fun)(SIGCHLD);
	}
#endif

	/*
	 * yuk: looks like we have to give fdp services time
	 *	to start up -- a few seconds shouldn't hurt
	 */

	sleep(2);
	return n;
}

int
cslocal(register Cs_t* state, const char* path)
{
	register char*	s;
	register char*	p;
	register char*	t;
	register char*	v;
	struct stat	st;
	char		cmd[PATH_MAX / 8];
	char		exe[PATH_MAX + 1];
	char		tmp[PATH_MAX + 1];
#if CS_LIB_STREAM || CS_LIB_V10 || CS_LIB_SOCKET_UN
	int		fd;
	int		n;
#endif

	messagef((state->id, NiL, -8, "local(%s) call", path));

	/*
	 * validate the path
	 */

	p = (char*)path;
	if (strncmp(p, DEVLOCAL, sizeof(DEVLOCAL) - 1))
	{
		messagef((state->id, NiL, -1, "local: %s: %s* expected", path, DEVLOCAL));
		goto sorry;
	}
	p += sizeof(DEVLOCAL) - 1;
	for (t = p; *t && *t != '/'; t++);
	if (!streq(t + 1, "user"))
	{
		messagef((state->id, NiL, -1, "local: %s: %s*/user expected", path, DEVLOCAL));
		goto sorry;
	}

	/*
	 * locate the service
	 */

	s = cmd;
	for (v = p; p <= t; *s++ = *p++);
	t = s - 1;
	for (p = v; p <= t; *s++ = *p++);
	for (p = CS_SVC_SUFFIX; *s++ = *p++;);
	p = pathbin();
	for (;;)
	{
		p = pathcat(p, ':', "../lib/cs/fdp", cmd, exe, PATH_MAX + 1);
		if (!eaccess(exe, X_OK) && !stat(exe, &st)) break;
		if (!p)
		{
			messagef((state->id, NiL, -1, "local: %s: %s: cannot locate service on ../lib/cs/fdp", path, cmd));
			goto sorry;
		}
	}
	*t = 0;
	sfsprintf(tmp, sizeof(tmp), "%s/fdp/%s/%s/%d-%d-/%c%s", csvar(state, CS_VAR_LOCAL, 0), csname(state, 0), cmd, st.st_uid, geteuid(), CS_MNT_STREAM, CS_MNT_TAIL);

#if CS_LIB_STREAM || CS_LIB_V10

	for (n = 0; (fd = open(tmp, O_RDWR)) < 0; n++)
		if (n || errno == EACCES)
		{
			messagef((state->id, NiL, -1, "local: %s: %s: cannot open service", path, tmp));
			return -1;
		}
		else if (initiate(state, path, exe) < 0)
		{
			messagef((state->id, NiL, -1, "local: %s: %s: cannot initiate service %s", path, tmp, exe));
			return -1;
		}
	messagef((state->id, NiL, -8, "local(%s) fd=%d server=%s stream=%s", path, fd, exe, tmp));
	return fd;

#else

#if CS_LIB_SOCKET_UN

	{
		int			namlen;
		struct sockaddr_un	nam;

		nam.sun_family = AF_UNIX;
		strcpy(nam.sun_path, tmp);
		namlen = sizeof(nam.sun_family) + strlen(tmp);
		n = 0;
		fd = -1;
		for (;;)
		{
			if (fd < 0 && (fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
			{
				messagef((state->id, NiL, -1, "local: %s: AF_UNIX socket error", path));
				return -1;
			}
			if (!connect(fd, (struct sockaddr*)&nam, namlen))
			{
#if CS_LIB_SOCKET_RIGHTS
				if (read(fd, cmd, 1) != 1)
					messagef((state->id, NiL, -1, "local: %s: connect ack read error", path));
				else if (cssend(state, fd, NiL, 0))
					messagef((state->id, NiL, -1, "local: %s: connect authentication send error", path));
				else
#endif
				return fd;
#if CS_LIB_SOCKET_RIGHTS
				close(fd);
				fd = -1;
#endif
			}
			else messagef((state->id, NiL, -1, "local: %s: connect error", path));
			if (errno != EACCES) errno = ENOENT;
			if (n || errno == EACCES || initiate(state, path, exe) < 0)
			{
				if (fd >= 0) close(fd);
				return -1;
			}
			n = 1;
			messagef((state->id, NiL, -1, "local: %s: connect retry", path));
		}
	}

#endif

#endif

 sorry:
	errno = ENOENT;
	return -1;
}

int
_cs_local(const char* path)
{
	return cslocal(&cs, path);
}
