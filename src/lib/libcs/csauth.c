/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2012 AT&T Intellectual Property          *
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
 * authenticate fd given service authentication path
 * if path==0 then it is requested from server
 * otherwise if fd>=0 then state->host and state->flags are assumed valid
 * only used by tcp streams
 * fd<0 for remote authentication on fd 0,1
 *
 *	csauth(state,fd,path,arg)	called by csopen()
 *	csauth(state,-1,path,arg)	called by `cs -O...'
 *	csauth(state,fd,0,arg)	normal user call
 */

#include "cslib.h"

#include <error.h>

#define AUTH_BASE	1000

int
csauth(register Cs_t* state, int fd, const char* path, const char* arg)
{
	register char*	s;
	register char*	t;
	char*		b;
	char*		key = 0;
	int		n;
	int		m;
	int		wfd;
	unsigned long	t1;
	unsigned long	t2;
	struct stat	st;
	char		num[64];
	char		buf[PATH_MAX + 1];
	char		tmp[PATH_MAX + 1];

	static int	auth = -1;

	messagef((state->id, NiL, -8, "auth(%d,%s,%s) call", fd, path, arg));
	if (!path)
	{
		if (fd < 0)
			goto sorry;
		if (!getauth(fd, n))
			goto ok;
		n = sfsprintf(buf, sizeof(buf), "%d\n", CS_KEY_SEND);
		if (cswrite(state, fd, buf, n) != n)
		{
			messagef((state->id, NiL, -1, "auth: `%-.*s': KEY_SEND write error", n - 1, buf));
			goto sorry;
		}
		if ((n = csread(state, fd, buf, sizeof(buf), CS_LINE)) <= 1)
		{
			messagef((state->id, NiL, -1, "auth: KEY_SEND read error"));
			goto sorry;
		}
		buf[n - 1] = 0;
		path = (const char*)buf;
	}
	if (stat(path, &st))
	{
		if (errno == ENOENT)
			goto ok;
		messagef((state->id, NiL, -1, "auth: %s: stat error", path));
		return -1;
	}
	if (fd < 0)
	{
		/*
		 * the remote side of remote authentication
		 */

		fd = 0;
		wfd = 1;
	}
	else wfd = fd;
	m = getpid();
	s = tmp + sfsprintf(tmp, sizeof(tmp) - 1, "%lu %d", (unsigned long)st.st_mtime, m);
	if ((t = (char*)arg) && !(st.st_mode & S_IXOTH))
	{
		b = tmp + sizeof(tmp) - 1;
		if (s < b)
		{
			*s++ = ' ';
			while (s < b && *t)
				if ((*s++ = *t++) == '/' && *t == '#' && *++t != '#')
					*(s - 1) = ' ';
		}
	}
	*s++ = '\n';
	n = s - tmp;
	if (cswrite(state, wfd, tmp, n) != n)
	{
		messagef((state->id, NiL, -1, "auth: `%-.*s': key write error", n - 1, tmp));
		goto sorry;
	}
	if (csread(state, fd, num, sizeof(num), CS_LINE) <= 0)
	{
		messagef((state->id, NiL, -1, "auth: key read error"));
		goto sorry;
	}
	if (*num != '\n')
	{
		n = 0;
		if (state->addr == csaddr(state, NiL)) b = tmp + sfsprintf(tmp, sizeof(tmp), "%s/AUTH.%05d.", csvar(state, CS_VAR_LOCAL, 0), m);
		else
		{
			s = (char*)path + strlen(path);
			while (s > (char*)path)
				if (*--s == '/' && ++n >= 4) break;
			if (n != 4) goto sorry;
			b = tmp + sfsprintf(tmp, sizeof(tmp), "%-.*s/AUTH.%05d.", s - path, path, m);
		}
		if (s = strrchr(tmp, '/'))
		{
			*s = 0;
			if (eaccess(tmp, X_OK) && (mkdir(tmp, S_IRWXU|S_IRWXG|S_IRWXO) || chmod(tmp, S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)))
			{
				messagef((state->id, NiL, -1, "auth: %s: challenge directory error", tmp));
				goto sorry;
			}
			*s = '/';
		}
		t1 = CSTIME();
		m += t1 + getppid();
		t1 -= CS_STAT_DOWN;
		if (auth < 0) auth = (unsigned int)CSTOSS(m, 0) % AUTH_BASE;
		n = auth;
		for (;;)
		{
			if (++auth >= AUTH_BASE) auth = 0;
			if (auth == n)
			{
				messagef((state->id, NiL, -1, "auth: %s: challenge directory full", tmp));
				goto sorry;
			}
			s = b + sfsprintf(b, sizeof(tmp) - (b - tmp), "%03d", auth);
			if ((stat(tmp, &st) || t1 <= (unsigned long)st.st_ctime && !remove(tmp)) && !close(open(tmp, O_CREAT|O_TRUNC|O_cloexec, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))) break;
		}
		key = tmp;
		if (tokscan(num, NiL, "%lu %lu", &t1, &t2) != 2)
		{
			messagef((state->id, NiL, -1, "auth: `%s': challenge syntax error", num));
			goto sorry;
		}
		if (cschallenge(state, tmp, &t1, &t2))
			goto sorry;
		if (chmod(tmp, S_ISUID|S_ISGID|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))
		{
			messagef((state->id, NiL, -1, "auth: %s: challenge chmod error", tmp));
			goto sorry;
		}
		t = s;
		if (arg)
		{
			b = tmp + sizeof(tmp) - 1;
			if (s < b)
			{
				*s++ = ' ';
				while (s < b && *arg)
					if ((*s++ = *arg++) == '/' && *arg == '#')
					{
						arg++;
						*(s - 1) = ' ';
					}
			}
		}
		*s++ = '\n';
		n = cswrite(state, wfd, tmp, s - tmp);
		*t = 0;
		if (n != s - tmp)
		{
			messagef((state->id, NiL, -1, "auth: `%s': ack write error", tmp));
			goto sorry;
		}
		if (csread(state, fd, num, 1, CS_LINE) != 1)
		{
			messagef((state->id, NiL, -1, "auth: ack read error"));
			goto sorry;
		}
		if (remove(tmp))
		{
			messagef((state->id, NiL, -1, "auth: %s: challenge remove error", tmp));
			goto sorry;
		}
	}
	if (fd >= 0) setauth(fd, n);
 ok:
	messagef((state->id, NiL, -8, "auth(%d,%s,%s) = 0", fd, path, arg));
	return 0;
 sorry:
	if (key) remove(key);
	errno = EACCES;
	return -1;
}

/*
 * set up the challenge {v1,v2} on path
 */

int
cschallenge(Cs_t* state, const char* path, unsigned long* v1, unsigned long* v2)
{
	struct stat	st;

	if (touch(path, (time_t)(v1 ? *v1 : cs.time), (time_t)(v2 ? *v2 : cs.time), 0))
	{
		messagef((state->id, NiL, -1, "auth: %s: challenge touch error", path));
		return -1;
	}
	if (stat(path, &st))
	{
		messagef((state->id, NiL, -1, "auth: %s: challenge stat error", path));
		return -1;
	}
	if (v1)
		*v1 = st.st_atime;
	if (v2)
		*v2 = st.st_mtime;
	return 0;
}

int
_cs_auth(int fd, const char* path, const char* arg)
{
	return csauth(&cs, fd, path, arg);
}

int
_cs_challenge(const char* path, unsigned long* v1, unsigned long* v2)
{
	return cschallenge(&cs, path, v1, v2);
}
