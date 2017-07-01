/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2013 AT&T Intellectual Property          *
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
 * access/open/create path of the form
 *
 *	/dev/<type>/<host>/<service>[/<qualifier>][/<perm>][/#option][/#/path]
 *
 * type:	{fdp,tcp,udp}
 * host:	open={<hostname>}, create={share,local}
 * service:	{.../lib/cs/<type>/<service>/server,.../bin/<service>}
 * qualifier:	optional service qualifier
 * perm:	{user[=euid],group[=egid],other(default)}
 */

#include "cslib.h"

#include <error.h>
#include <proc.h>
#include <hashkey.h>

#define REMOTE_ARGC	10
#define REMOTE_FLAGC	12

/*
 * create the state->mount <type>/<host>/<service>/<perm> subdirs
 * state->mount modified, possibly destroyed on error
 */

static int
mkmount(register Cs_t* state, int mode, int uid, int gid, char* endserv, char* endhost, char* endtype)
{
	*(state->control - 1) = 0;
	if (eaccess(state->mount, R_OK|W_OK|X_OK))
	{
		if (errno != ENOENT)
			goto bad;
		if (!endserv && !(endserv = strrchr(state->mount, '/')))
			goto bad;
		*endserv = 0;
		if (eaccess(state->mount, X_OK))
		{
			if (!endhost && !(endhost = strrchr(state->mount, '/')))
				goto bad;
			*endhost = 0;
			if (eaccess(state->mount, X_OK))
			{
				if (!endtype && !(endtype = strrchr(state->mount, '/')))
					goto bad;
				*endtype = 0;
				if (eaccess(state->mount, X_OK) && (mkdir(state->mount, S_IRWXU|S_IRWXG|S_IRWXO) || chmod(state->mount, S_IRWXU|S_IRWXG|S_IRWXO)))
					goto bad;
				*endtype = '/';
				if (mkdir(state->mount, S_IRWXU|S_IRWXG|S_IRWXO) || chmod(state->mount, S_IRWXU|S_IRWXG|S_IRWXO))
					goto bad;
			}
			*endhost = '/';
			if (mkdir(state->mount, S_IRWXU|S_IRWXG|S_IRWXO) || chmod(state->mount, S_IRWXU|S_IRWXG|S_IRWXO))
				goto bad;
		}
		*endserv = '/';
		if (mkdir(state->mount, mode))
			goto bad;
		if (mode != (S_IRWXU|S_IRWXG|S_IRWXO) && (uid >= 0 || gid >= 0 && (mode |= S_ISGID)) && (chown(state->mount, uid, gid) || chmod(state->mount, mode)))
		{
			rmdir(state->mount);
			goto bad;
		}
	}
	*(state->control - 1) = '/';
	return 0;
 bad:
	if (endtype && !*endtype)
		*endtype = '/';
	if (endhost && !*endhost)
		*endhost = '/';
	if (endserv && !*endserv)
		*endserv = '/';
	*(state->control - 1) = '/';
	messagef((state->id, NiL, -1, "mkmount: %s: cannot access physical mount directory", state->mount));
	return -1;
}

/*
 * generate CS_REMOTE_CONTROL argv
 */

static void
remote(register Cs_t* state, const char* host, const char* user, const char* path, int agent, register char** av, register char* fv)
{
	register char*	t;

	*av++ = CS_REMOTE_SHELL;
	*av++ = (char*)host;
	if (user && *user)
	{
		*av++ = "-l";
		*av++ = (char*)user;
	}
	*av++ = CS_REMOTE_PROFILE;
	*av++ = ";";
	*av++ = CS_REMOTE_CONTROL;
	*av++ = fv;
	*av++ = (char*)path;
	*av = 0;
	*fv++ = '-';
	if (error_info.trace < 0)
		fv += sfsprintf(fv, 4, "%c%d", CS_REMOTE_DEBUG, -error_info.trace);
	*fv++ = CS_REMOTE_OPEN;
	t = fv;
	if (agent)
		*fv++ = CS_REMOTE_OPEN_AGENT;
	if (state->flags & CS_ADDR_LOCAL)
		*fv++ = CS_REMOTE_OPEN_LOCAL;
	if (state->flags & CS_ADDR_NOW)
		*fv++ = CS_REMOTE_OPEN_NOW;
	if (state->flags & CS_ADDR_SHARE)
		*fv++ = CS_REMOTE_OPEN_SHARE;
	if (state->flags & CS_ADDR_TEST)
		*fv++ = CS_REMOTE_OPEN_TEST;
	if (state->flags & CS_ADDR_TRUST)
		*fv++ = CS_REMOTE_OPEN_TRUST;
	if (fv == t)
		*fv++ = CS_REMOTE_OPEN_READ;
	*fv = 0;
}

/*
 * initiate service named by path on state->host w/state->flags
 * service is modified in place
 * 0 returned on success
 */

static int
initiate(register Cs_t* state, char* user, char* path, char* service, char* name)
{
	register char*	s;
	char*		on;
	char*		local;
	Sfio_t*		sp;
	Sfio_t*		np;
	int		n;
	char*		av[REMOTE_ARGC];
	char		buf[PATH_MAX / 4];

	local = csname(state, 0);
	s = service + strlen(service);
	*s++ = '/';
	if (!(state->flags & CS_ADDR_SHARE))
	{
		sfsprintf(buf, sizeof(buf), "%s\n", state->host);
		if (!(sp = tokline(buf, SF_STRING, NiL)))
			return -1;
	}
	else if (state->flags & CS_ADDR_LOCAL)
	{
		sfsprintf(buf, sizeof(buf), "%s\n", CS_HOST_LOCAL);
		if (!(sp = tokline(buf, SF_STRING, NiL)))
			return -1;
	}
	else
	{
		strcpy(s, CS_SVC_HOSTS);
		if (!(sp = tokline(service, SF_READ, NiL)))
		{
			if (streq(state->host, CS_HOST_SHARE))
				sfsprintf(buf, sizeof(buf), "%s\n%s\n", CS_HOST_SHARE, CS_HOST_LOCAL);
			else
				sfsprintf(buf, sizeof(buf), "%s\n%s\n%s\n", state->host, CS_HOST_SHARE, CS_HOST_LOCAL);
			if (!(sp = tokline(buf, SF_STRING, NiL)))
				return -1;
		}
	}
	sfsprintf(s, PATH_MAX - (s - service) - 1, "%s%s", name, CS_SVC_SUFFIX);
	while (s = sfgetr(sp, '\n', 1))
		if (tokscan(s, NiL, " %s ", &on) == 1)
		{
			if (streq(on, CS_HOST_LOCAL) || streq(on, local))
			{
				sfclose(sp);
				av[0] = service;
				av[1] = path;
				av[2] = 0;
				return procclose(procopen(av[0], av, NiL, NiL, PROC_PRIVELEGED|PROC_ZOMBIE)) < 0 ? -1 : 0;
			}
			else if (!streq(on, CS_HOST_SHARE))
			{
				Proc_t*		proc;
				time_t		otime;
				struct stat	st;
				char		fv[REMOTE_FLAGC];
				long		ov[3];

				remote(state, on, user, path, 0, av, fv);
				otime = lstat(state->mount, &st) ? 0 : st.st_ctime;
				n = open("/dev/null", O_RDWR);
				ov[0] = PROC_FD_DUP(n, 0, 0);
				ov[1] = PROC_FD_DUP(n, 1, PROC_FD_PARENT|PROC_FD_CHILD);
				ov[2] = 0;
				if (proc = procopen(av[0], av, NiL, ov, PROC_PRIVELEGED|PROC_ZOMBIE))
				{
					n = 1;
					for (;;)
					{
						if (!lstat(state->mount, &st) && st.st_ctime != otime)
						{
							if (!procclose(proc))
							{
								sfclose(sp);
								return 0;
							}
							break;
						}

						/*
						 * sleep() and MNT_TMP
						 * hack around remote
						 * fs cache delays
						 */

						if (n >= CS_REMOTE_DELAY)
						{
							procclose(proc);
							break;
						}
						if (n == 1)
						{
							*state->control = CS_MNT_TMP;
							if (remove(state->mount))
							{
								close(open(state->mount, O_WRONLY|O_CREAT|O_TRUNC, 0));
								remove(state->mount);
							}
							*state->control = CS_MNT_STREAM;
						}
						sleep(n);
						n <<= 1;
					}
				}
			}
			else if (!sfstacked(sp) && (np = csinfo(state, on, NiL)))
				sfstack(sp, np);
		}
	sfclose(sp);
	return -1;
}

/*
 * recursive csopen(state,path,CS_OPEN_TEST)
 * previous side effects preserved
 */

static int
reopen(register Cs_t* state, char* path)
{
	int		ret;
	Cs_t		tmp;

	static int	level;

	if (level >= 8)
		return -1;
	tmp = *state;
	level++;
	ret = csopen(&tmp, path, CS_OPEN_TEST);
	level--;
	state->addr = tmp.addr;
	state->port = tmp.port;
	return ret;
}

/*
 * use remote agent to initiate and authenticate service on path
 */

static int
agent(register Cs_t* state, const char* host, const char* user, const char* path)
{
	register int	n;
	register int	m;
	register char*	s;
	Proc_t*		proc;
	int		fd = -1;
	char*		av[REMOTE_ARGC];
	char		fv[REMOTE_FLAGC];
	char		buf[PATH_MAX / 4];
	char		num[64];
	char		tmp[64];

	remote(state, host, user, path, 1, av, fv);
	if (!(proc = procopen(av[0], av, NiL, NiL, PROC_READ|PROC_WRITE|PROC_PRIVELEGED)))
		goto sorry;
	if ((m = csread(state, proc->rfd, buf, sizeof(buf), CS_LINE)) <= 1)
		goto sorry;
	buf[--m] = 0;
	if (buf[--m] == 'A')
		buf[--m] = 0;
	else
		m = 0;
	if ((fd = reopen(state, buf)) < 0)
		goto sorry;
	if (m)
	{
		if ((m = csread(state, proc->rfd, buf, sizeof(buf), CS_LINE)) <= 1)
			goto sorry;
		if (cswrite(state, fd, buf, m) != m)
			goto sorry;
		if ((n = csread(state, fd, num, sizeof(num), CS_LINE)) <= 0)
			goto sorry;
		if (*num != '\n')
		{
			buf[m - 1] = 0;
			num[n - 1] = 0;
			n = sfsprintf(tmp, sizeof(tmp), "%s %s\n", num, (s = strchr(buf, ' ')) ? (s + 1) : buf);
			if (cswrite(state, proc->wfd, tmp, n) != n)
				goto sorry;
			if ((m = csread(state, proc->rfd, buf, sizeof(buf), CS_LINE)) <= 0)
				goto sorry;
			if (cswrite(state, fd, buf, m) != m)
				goto sorry;
			if (csread(state, fd, num, 1, CS_LINE) != 1)
				goto sorry;
			if (cswrite(state, proc->wfd, num, 1) != 1)
				goto sorry;
		}
	}
	procclose(proc);
	return fd;
 sorry:
	if (fd >= 0)
		close(fd);
	if (proc)
		procclose(proc);
	errno = EACCES;
	return -1;
}

/*
 * csattach() helper
 */

static int
doattach(register Cs_t* state, const char* path, int op, int mode, char* user, char* opath, char* tmp, char* serv, char*b)
{
	register int	n;
	int		fd;
	char*		s;

#if CS_LIB_STREAM || CS_LIB_V10

	int		fds[2];
	struct stat	st;

	if (op & CS_OPEN_CREATE)
	{
		n = errno;
		if (chmod(path, mode))
		{
			remove(path);
			if ((fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, mode)) < 0)
			{
				messagef((state->id, NiL, -1, "open: %s: %s: creat %o error", state->path, path, mode));
				return -1;
			}
			close(fd);
			chmod(path, mode);
		}
		errno = n;
		if (pipe(fds))
		{
			messagef((state->id, NiL, -1, "open: %s: pipe error", state->path, path));
			return -1;
		}

#if CS_LIB_V10

		if ((n = ioctl(fds[1], FIOPUSHLD, &conn_ld)) || fmount(3, fds[1], path, 0))

#else

		if ((n = ioctl(fds[1], I_PUSH, "connld")) || fattach(fds[1], path))

#endif

		{
			messagef((state->id, NiL, -1, "open: %s: %s: %s error", state->path, path, n ? "connld" : "fattach"));
			close(fds[0]);
			close(fds[1]);
			errno = ENXIO;
			return -1;
		}
		close(fds[1]);
		fd = fds[0];
	}
	else
		for (;;)
		{
			if ((fd = open(path, O_RDWR)) >= 0)
			{
				if (!fstat(fd, &st) && !S_ISREG(st.st_mode))
					break;
				close(fd);
				remove(path);
			}
			else if ((op & CS_OPEN_TEST) || errno == EACCES)
			{
				messagef((state->id, NiL, -1, "open: %s: %s: open error", state->path, path));
				return -1;
			}
			if (initiate(state, user, opath, tmp, serv))
			{
				messagef((state->id, NiL, -1, "open: %s: %s: service initiate error", state->path, path));
				return -1;
			}
			op = CS_OPEN_TEST;
		}

#else

#if CS_LIB_SOCKET_UN

	int			pid;
	int			namlen;
	char			c;
	struct sockaddr_un	nam;

	messagef((state->id, NiL, -8, "%s:%d state.path=`%s' state.mount=`%s' path=`%s' opath=`%s' user=`%s' serv=`%s'", __FILE__, __LINE__, state->path, state->mount, path, opath, user, serv));
	nam.sun_family = AF_UNIX;
	strcpy(nam.sun_path, path);
	namlen = sizeof(nam.sun_family) + strlen(path) + 1;
	for (n = 0;; n++)
	{
		if (n >= 10)
		{
			errno = ENXIO;
		badcon:
			close(fd);
			return -1;
		}
		if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		{
			messagef((state->id, NiL, -1, "open: %s: %s: AF_UNIX socket error", state->path, path));
			return -1;
		}
		if (!connect(fd, (struct sockaddr*)&nam, namlen))
		{
			if (op & CS_OPEN_CREATE)
			{
				errno = EEXIST;
				goto badcon;
			}
#if CS_LIB_SOCKET_RIGHTS
			if (read(fd, &c, 1) == 1 && !cssend(state, fd, NiL, 0))
				break;
#else
			break;
#endif
		}
		else
		{
			messagef((state->id, NiL, -1, "open: %s: %s: connect error", state->path, path));
			if (errno == EACCES)
				goto badcon;
			else if (errno == EADDRNOTAVAIL || errno == ECONNREFUSED)
			{
				c = 0;
				for (;;)
				{
					*b = CS_MNT_PROCESS;
					pid = pathgetlink(path, state->temp, sizeof(state->temp));
					*b = CS_MNT_STREAM;
					if (pid > 0 || ++c >= 5)
						break;
					sleep(1);
				}
				if (pid > 0 && (s = strrchr(state->temp, '/')) && (pid = strtol(s + 1, NiL, 0)) > 0)
				{
					if (!kill(pid, 0) || errno != ESRCH)
					{
						if (op & CS_OPEN_CREATE)
						{
							errno = EEXIST;
							goto badcon;
						}
						close(fd);
						if (n)
							sleep(1);
						continue;
					}
					*b = CS_MNT_PROCESS;
					remove(path);
					*b = CS_MNT_STREAM;
				}
			}
		}
		close(fd);
		errno = ENOENT;
		if (op & CS_OPEN_CREATE)
		{
			if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
			{
				messagef((state->id, NiL, -1, "open: %s: %s: AF_UNIX socket error", state->path, path));
				return -1;
			}
			if (!bind(fd, (struct sockaddr*)&nam, namlen))
			{
				chmod(path, mode);
				if (listen(fd, 32))
				{
					messagef((state->id, NiL, -1, "open: %s: %s: listen error", state->path, path));
					n = errno;
					remove(path);
					errno = n;
					goto badcon;
				}
				break;
			}
			else
				messagef((state->id, NiL, -1, "open: %s: %s: bind error", state->path, path));
			if (errno != EADDRINUSE || n && remove(path) && errno != ENOENT)
				goto badcon;
			close(fd);
		}
		else if (op & CS_OPEN_TEST)
			return -1;
		else if (!n && initiate(state, user, opath, tmp, serv))
		{
			messagef((state->id, NiL, -1, "open: %s: %s: service initiate error", state->path, path));
			return -1;
		}
		else
			sleep(2);
	}

#else

	errno = (op & CS_OPEN_CREATE) ? ENXIO : ENOENT;
	messagef((state->id, NiL, -1, "open: %s: %s: not supported", state->path, path));
	fd = -1;

#endif

#endif

#if CS_LIB_SOCKET_UN || CS_LIB_STREAM || CS_LIB_V10

	touch(path, (time_t)0, (time_t)0, 0);
	strcpy(state->path, path);

#endif

	return fd;
}

/*
 * return connect stream to fdp path
 * if (op & CS_) then path is created
 * and server connect stream is returned
 * otherwise client connect stream returned
 */

int
csattach(register Cs_t* state, const char* path, int op, int mode)
{
	return doattach(state, path, op, 0, 0, 0, 0, 0, 0);
}

int
_cs_attach(const char* path, int op, int mode)
{
	return csattach(&cs, path, op, mode);
}

/*
 * access/open/create service on path
 */

int
csopen(register Cs_t* state, const char* apath, int op)
{
	register char*	path = (char*)apath;
	register char*	b;
	register char*	s;
	register int	n;
	int		fd;
	char*		t;
	char*		u;
	char*		type;
	char*		endtype;
	char*		host;
	char*		endhost;
	char*		serv;
	char*		endserv;
	char*		qual;
	char*		endqual;
	char*		opath;
	char*		user = 0;
	char*		group = 0;
	char*		trust = 0;
	char*		arg = 0;
	int		nfd = -1;
	int		uid = -1;
	int		gid = -1;
	int		sid = -1;
	int		auth = 1;
	int		mode;
	unsigned long	addr;
	unsigned long	port = 0;
	struct stat	st;
	char		buf[PATH_MAX];
	char		tmp[PATH_MAX];

	if (!path)
	{
		errno = EFAULT;
		return -1;
	}
	csprotect(&cs);
	if (op < 0)
		op = CS_OPEN_TEST;
	messagef((state->id, NiL, -8, "open(%s,%o) call", path, op));

	/*
	 * blast out the parts
	 */

	opath = path;
	if (pathgetlink(path, buf, sizeof(buf)) <= 0)
	{
		if (strlen(path) >= sizeof(buf))
			return -1;
		strcpy(buf, path);
	}
	else if ((state->flags & CS_ADDR_LOCAL) && (s = strrchr(buf, '/')))
	{
		/*
		 * dynamic ip assignment can change the addr
		 * underfoot in some implementations so we
		 * double check the local ip here
		 */

		strcpy(tmp, buf);
		if (tokscan(tmp, NiL, "/dev/%s/%s/%s", &type, NiL, &serv) == 3)
			sfsprintf(buf, sizeof(buf), "/dev/%s/%s/%s", type, csntoa(state, 0), serv);
	}
	path = buf;
	pathcanon(path, 0, 0);
	errno = ENOENT;
	strcpy(state->path, path);
	b = path;
	if ((*b++ != '/') || !(s = strchr(b, '/')))
		return -1;
	*s++ = 0;
	if (!streq(b, "dev"))
		return -1;
	if (b = strchr(s, '/'))
		*b++ = 0;
	if (streq(s, "fdp"))
	{
#if !( CS_LIB_SOCKET_UN || CS_LIB_STREAM || CS_LIB_V10 )
		if (access(CS_PROC_FD_TST, F_OK))
		{
			errno = ENODEV;
			messagef((state->id, NiL, -1, "open: %s: %s: not supported", state->path, s));
			return -1;
		}
#endif
	}
	else if (!streq(s, "tcp") && !streq(s, "udp"))
	{
		messagef((state->id, NiL, -1, "open: %s: %s: invalid type", state->path, s));
		return -1;
	}
#if !( CS_LIB_SOCKET || CS_LIB_STREAM || CS_LIB_V10 )
	else
	{
		errno = ENODEV;
		messagef((state->id, NiL, -1, "open: %s: %s: not supported", state->path, s));
		return -1;
	}
#endif
	type = s;
	qual = state->qual;
	if (!b)
		host = serv = 0;
	else
	{
		host = b;
		if (!(s = strchr(b, '/')))
			serv = 0;
		else
		{
			*s++ = 0;
			serv = s;

			/*
			 * grab the next fd to preserve open semantics
			 */

			for (n = 0; n < 10; n++)
				if ((nfd = dup(n)) >= 0)
					break;

			/*
			 * get qual, perm and arg
			 */

			mode = S_IRWXU|S_IRWXG|S_IRWXO;
			if (b = strchr(s, '/'))
			{
				*b++ = 0;
				do
				{
					if (*b == '#')
					{
						arg = b + 1;
						break;
					}
					if (u = strchr(b, '/'))
						*u++ = 0;
					if (s = strchr(b, '='))
						*s++ = 0;
					for (n = 0, t = b; *t; n = HASHKEYPART(n, *t++));
					switch (n)
					{
					case HASHKEY5('g','r','o','u','p'):
						group = s ? s : "";
						break;
					case HASHKEY5('l','o','c','a','l'):
						op |= CS_OPEN_LOCAL;
						break;
					case HASHKEY3('n','o','w'):
						op |= CS_OPEN_NOW;
						break;
					case HASHKEY5('o','t','h','e','r'):
						auth = 0;
						break;
					case HASHKEY6('r','e','m','o','t','e'):
						op |= CS_OPEN_REMOTE;
						break;
					case HASHKEY5('s','h','a','r','e'):
						op |= CS_OPEN_SHARE;
						break;
					case HASHKEY5('s','l','a','v','e'):
						op |= CS_OPEN_SLAVE;
						break;
					case HASHKEY4('t','e','s','t'):
						op |= CS_OPEN_TEST;
						break;
					case HASHKEY5('t','r','u','s','t'):
						op |= CS_OPEN_TRUST;
						trust = s;
						break;
					case HASHKEY4('u','s','e','r'):
						user = s ? s : "";
						break;
					default:
						qual += sfsprintf(qual, sizeof(state->qual) - (qual - state->qual) - 1, "%s%s", qual == state->qual ? "" : "-", b);
						if (s)
							*(s - 1) = '=';
						break;
					}
				} while (b = u);
			}
		}
	}
	if (*type != 't')
		auth = 0;
	strncpy(state->type, type, sizeof(state->type) - 1);
	qual = (qual == state->qual) ? (char*)0 : state->qual;
	messagef((state->id, NiL, -8, "open: type=%s host=%s serv=%s qual=%s", type, host, serv, qual));
	if (host)
	{
		/*
		 * validate host
		 */

		if (!(state->addr = addr = csaddr(state, host)))
		{
			if (serv && !(op & CS_OPEN_CREATE) && *type == 't' && (port = csport(state, type, serv)) >= CS_PORT_MIN && port <= CS_PORT_MAX)
			{
				/*
				 * attempt proxy connection
				 */

				if (nfd >= 0)
				{
					close(nfd);
					nfd = -1;
				}
				if ((fd = state->proxy.addr ? csbind(state, type, state->proxy.addr, state->proxy.port, 0L) : reopen(state, csvar(state, CS_VAR_PROXY, 0))) >= 0)
				{
					state->proxy.addr = state->addr;
					state->proxy.port = state->port;
					n = sfsprintf(tmp, sizeof(tmp), "\n%s!%s!%d\n\n%s\n%s\n0\n-1\n-1\n", type, host, port, csname(state, 0), error_info.id ? error_info.id : state->id);
					if (cswrite(state, fd, tmp, n) == n && (n = csread(state, fd, tmp, sizeof(tmp), CS_LINE)) >= 2)
					{
						if (tmp[0] == '0' && tmp[1] == '\n')
							return fd;
						if (error_info.trace <= -4 && n > 2)
						{
							s = tmp;
							s[n - 1] = 0;
							while (*s && *s++ != '\n');
							messagef((state->id, NiL, -4, "%s error message `%s'", csvar(state, CS_VAR_PROXY, 0), s));
						}
					}
					close(fd);
				}
			}
#ifdef EADDRNOTAVAIL
			errno = EADDRNOTAVAIL;
#else
			errno = ENOENT;
#endif
			goto bad;
		}
		if (op & CS_OPEN_LOCAL)
		{
			state->flags |= CS_ADDR_LOCAL;
			state->flags &= ~CS_ADDR_REMOTE;
		}
		if (op & CS_OPEN_NOW)
			state->flags |= CS_ADDR_NOW;
		if ((op & (CS_OPEN_AGENT|CS_OPEN_REMOTE)) == CS_OPEN_REMOTE)
		{
			state->flags |= CS_ADDR_REMOTE;
			state->flags &= ~CS_ADDR_LOCAL;
		}
		if (op & CS_OPEN_SHARE)
			state->flags |= CS_ADDR_SHARE;
		if (op & CS_OPEN_SLAVE)
			state->flags |= CS_DAEMON_SLAVE;
		if (op & CS_OPEN_TEST)
			state->flags |= CS_ADDR_TEST;
		if (op & CS_OPEN_TRUST)
			state->flags |= CS_ADDR_TRUST;
		if ((state->flags & CS_ADDR_REMOTE) && (!serv || !strneq(serv, CS_SVC_INET, sizeof(CS_SVC_INET) - 1) && (strtol(serv, &t, 0), *t)))
			return agent(state, state->host, state->user, state->path);
		if (s = user)
		{
			n = geteuid();
			if (*s)
			{
				if ((uid = struid(s)) < 0)
				{
					uid = strtol(s, &t, 0);
					if (*t)
					{
						errno = EACCES;
						goto bad;
					}
				}
				if (n && uid != n)
				{
					errno = EACCES;
					goto bad;
				}
			}
			else
				uid = n;
			mode &= ~(S_IRWXG|S_IRWXO);
		}
		if (s = group)
		{
			n = getegid();
			if (*s)
			{
				if ((gid = strgid(s)) < 0)
				{
					gid = strtol(s, &t, 0);
					if (*t)
					{
						errno = EACCES;
						goto bad;
					}
				}
				if (geteuid() && gid != n)
				{
					gid_t*	groups;
					int	g;

					if ((g = getgroups(0, NiL)) <= 0)
						g = getconf("NGROUPS_MAX");
					if (groups = newof(0, gid_t, g, 0))
					{
						for (n = getgroups(g, groups); n >= 0; n--)
							if (gid == groups[n])
								break;
						free(groups);
					}
					else
						n = -1;
					if (n < 0)
					{
						errno = EACCES;
						goto bad;
					}
				}
			}
			else
				gid = n;
			mode &= ~S_IRWXO;
		}
		if (s = trust)
		{
			if (!*s)
				sid = geteuid();
			else if ((sid = struid(s)) < 0)
			{
				sid = strtol(s, &t, 0);
				if (*t)
				{
					errno = EACCES;
					goto bad;
				}
			}
		}
		if (state->flags & CS_ADDR_SHARE)
			host = CS_HOST_SHARE;
		else
		{
			host = state->host;
			if (!(state->flags & CS_ADDR_LOCAL))
			{
				if (*type == 'f')
				{
					errno = ENODEV;
					goto bad;
				}
				if (op & CS_OPEN_CREATE)
				{
					errno = EROFS;
					goto bad;
				}
			}
			if (serv && !qual && *type != 'f' && (port = csport(state, type, serv)) != CS_PORT_INVALID)
			{
				if (op & CS_OPEN_CREATE)
					addr = 0;
				else if (port == CS_PORT_RESERVED || port == CS_PORT_NORMAL)
					goto bad;
				if (nfd >= 0)
				{
					close(nfd);
					nfd = -1;
				}
				state->control = 0;
				if ((fd = csbind(state, type, addr, port, 0L)) >= 0)
				{
					if (mode != (S_IRWXU|S_IRWXG|S_IRWXO) && csauth(state, fd, NiL, NiL))
					{
						close(fd);
						return -1;
					}
					return fd;
				}
			}
		}
	}

	/*
	 * get the mount dir prefix
	 */

	if (opath == (b = path = state->mount))
	{
#ifdef ELOOP
		errno = ELOOP;
#else
		errno = EINVAL;
#endif
		goto bad;
	}
	if (*type == 'f')
	{
		if (host && !(state->flags & CS_ADDR_LOCAL))
		{
			errno = ENODEV;
			goto bad;
		}
		b += sfsprintf(b, sizeof(state->mount) - (b - path), "%s", csvar(state, CS_VAR_LOCAL, 0));
		if ((op & CS_OPEN_CREATE) && eaccess(path, X_OK) && (mkdir(path, S_IRWXU|S_IRWXG|S_IRWXO) || chmod(path, S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)))
			goto bad;
	}
	else
	{
		if (op & CS_OPEN_TRUST)
		{
			if (!pathaccess(csvar(state, CS_VAR_TRUST, 1), csvar(state, CS_VAR_SHARE, 1), NiL, PATH_EXECUTE, b, sizeof(state->mount) - (b - state->mount)))
				goto bad;
		}
		else if (!pathpath(csvar(state, CS_VAR_SHARE, 0), "", PATH_EXECUTE, b, sizeof(state->mount) - (b - state->mount)))
			goto bad;
		b += strlen(b);
	}

	/*
	 * add the type
	 */

	b += sfsprintf(b, sizeof(state->mount) - (b - path), "/%s", type);
	if (!host)
	{
		*(state->control = b + 1) = 0;
		if (nfd >= 0)
			close(nfd);
		if ((fd = open(path, O_RDONLY)) < 0)
		{
			mkmount(state, S_IRWXU|S_IRWXG|S_IRWXO, -1, -1, NiL, NiL, NiL);
			fd = open(path, O_RDONLY);
		}
		if (fd < 0)
			messagef((state->id, NiL, -1, "open: %s: %s: open error", state->path, path));
		return fd;
	}
	endtype = b;

	/*
	 * add the host
	 */

	if (strlen(host) <= CS_MNT_MAX)
		b += sfsprintf(b, sizeof(state->mount) - (b - path), "/%s", host);
	else
	{
		s = csntoa(state, addr);
		if (strlen(s) <= CS_MNT_MAX)
			b += sfsprintf(b, sizeof(state->mount) - (b - path), "/%s", s);
		else
		{
			unsigned char*	a = (unsigned char*)&addr;

			b += sfsprintf(b, sizeof(state->mount) - (b - path), "/0x%X.%X.%X.%X", a[0], a[1], a[2], a[3]);
		}
	}
	messagef((state->id, NiL, -8, "%s:%d host=`%s' path=`%s'", __FILE__, __LINE__, host, path));
	if (!serv)
	{
		*(state->control = b + 1) = 0;
		if (nfd >= 0)
			close(nfd);
		if ((fd = open(path, O_RDONLY)) < 0)
			messagef((state->id, NiL, -1, "open: %s: %s: open error", state->path, path));
		return fd;
	}
	endhost = b;

	/*
	 * add the service
	 */

	sfsprintf(b, sizeof(state->mount) - (b - path), "%s/%s/%s/%s%s", CS_SVC_DIR, type, serv, serv, CS_SVC_SUFFIX);
	if (!pathpath(b, "", PATH_ABSOLUTE|PATH_EXECUTE, tmp, sizeof(tmp)) || stat(tmp, &st))
		op |= CS_OPEN_TEST;
	else
	{
		*strrchr(tmp, '/') = 0;
		if (!(op & CS_OPEN_TRUST))
			sid = st.st_uid;
		if (!st.st_size)
			op |= CS_OPEN_TEST;
	}
	b += sfsprintf(b, sizeof(state->mount) - (b - path), "/%s", serv);
	endserv = b;

	/*
	 * add the qualifier and perm
	 */

	if (sid >= 0)
		b += sfsprintf(b, sizeof(state->mount) - (b - path), "/%d-", sid);
	else
		b += sfsprintf(b, sizeof(state->mount) - (b - path), "/-");
	if (uid >= 0)
		b += sfsprintf(b, sizeof(state->mount) - (b - path), "%d-", uid);
	else if (gid >= 0)
		b += sfsprintf(b, sizeof(state->mount) - (b - path), "-%d", gid);
	else
		b += sfsprintf(b, sizeof(state->mount) - (b - path), "-");
#if limit_qualifier_length
	endqual = endserv + CS_MNT_MAX + 1;
#else
	endqual = state->mount + sizeof(state->mount) - 1;
#endif
	if (qual)
	{
		if (b < endqual)
			*b++ = '-';
		while (b < endqual && *qual)
			*b++ = *qual++;
	}
	if (*type == 't' && !auth)
	{
		if (b >= endqual)
			b--;
		*b++ = CS_MNT_OTHER;
	}

	/*
	 * add in the connect stream control
	 */

	*b++ = '/';
	*b = CS_MNT_STREAM;
	strcpy(b + 1, CS_MNT_TAIL);
	messagef((state->id, NiL, -8, "%s:%d %s", __FILE__, __LINE__, state->mount));
	state->control = b;

	/*
	 * create the mount subdirs if necessary
	 */

	if ((op & CS_OPEN_CREATE) && mkmount(state, mode, uid, gid, endserv, endhost, endtype))
		goto bad;
	mode &= S_IRWXU|S_IRWXG|S_IRWXO;
	if (nfd >= 0)
	{
		close(nfd);
		nfd = -1;
	}
	if (op & CS_OPEN_MOUNT)
	{
		messagef((state->id, NiL, -1, "open(%s,%o) = %d, mount = %s", state->path, op, state->mount));
		return 0;
	}
	if (*type == 'f')
	{
		/*
		 * {fdp}
		 */

		if ((fd = doattach(state, path, op, mode, user, opath, tmp, serv, b)) < 0)
			return -1;
	}
	else
	{
		/*
		 * {tcp,udp}
		 */

		messagef((state->id, NiL, -8, "%s:%d %s", __FILE__, __LINE__, state->mount));
		if ((fd = reopen(state, path)) < 0)
		{
			/*
			 * check for old single char cs mount
			 */

			*(state->control + 1) = 0;
			if ((fd = reopen(state, path)) < 0)
				messagef((state->id, NiL, -1, "open: %s: %s: reopen error", state->path, path));
			*(state->control + 1) = CS_MNT_TAIL[0];
		}
		if (op & CS_OPEN_CREATE)
		{
			if (fd >= 0)
			{
				close(fd);
				errno = EEXIST;
				return -1;
			}
			if (errno != ENOENT && errno != ENOTDIR)
				return -1;
			sigcritical(1);
			*state->control = CS_MNT_LOCK;
			if ((fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0)) < 0)
			{
				if (stat(path, &st))
				{
					messagef((state->id, NiL, -1, "open: %s: %s: creat error", state->path, path));
					goto unblock;
				}
				if ((CSTIME() - (unsigned long)st.st_ctime) < 2 * 60)
				{
					errno = EEXIST;
					messagef((state->id, NiL, -1, "open: %s: %s: another server won the race", state->path, path));
					goto unblock;
				}
				if (remove(path))
				{
					messagef((state->id, NiL, -1, "open: %s: %s: remove error", state->path, path));
					goto unblock;
				}
				if ((fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0)) < 0)
				{
					messagef((state->id, NiL, -1, "open: %s: %s: creat error", state->path, path));
					goto unblock;
				}
			}
			close(fd);
			if (!port && (n = strtol(serv, &t, 0)) && t > serv && !*t)
				port = n;
			else if (geteuid())
				port = CS_NORMAL;
			else
				port = CS_RESERVED;
			if ((fd = csbind(state, type, 0L, port, 0L)) >= 0)
			{
				*state->control = CS_MNT_STREAM;
				remove(path);
				if (pathsetlink(cspath(state, fd, 0), path))
				{
					messagef((state->id, NiL, -1, "open: %s: %s: link error", cspath(state, fd, 0), path));
					close(fd);
					fd = -1;
				}
			}
		unblock:
			*state->control = CS_MNT_LOCK;
			remove(path);
			sigcritical(0);
			*state->control = CS_MNT_STREAM;
			if (fd < 0)
				return -1;
		}
		else if (fd < 0 && ((op & CS_OPEN_TEST) || initiate(state, user, opath, tmp, serv) || (fd = reopen(state, path)) < 0))
		{
			messagef((state->id, NiL, -1, "open: %s: %s: reopen/initiate error", state->path, path));
			return -1;
		}
		else if (!(op & CS_OPEN_AGENT))
		{
			*state->control = CS_MNT_AUTH;
			n = csauth(state, fd, path, arg);
			*state->control = CS_MNT_STREAM;
			if (n)
			{
				close(fd);
				messagef((state->id, NiL, -1, "open: %s: %s: authentication error", state->path, path));
				return -1;
			}
		}
	}

	/*
	 * fd is open at this point
	 * make sure its not a bogus mount
	 */

	if (mode != (S_IRWXU|S_IRWXG|S_IRWXO))
	{
		*state->control = 0;
		n = stat(path, &st);
		*state->control = CS_MNT_STREAM;
		if (n)
		{
			messagef((state->id, NiL, -1, "open: %s: %s: stat error", state->path, path));
			close(fd);
			return -1;
		}
		if (uid >= 0 && st.st_uid != uid || gid >= 0 && st.st_gid != gid)
		{
			close(fd);
			errno = EPERM;
			messagef((state->id, NiL, -1, "open: %s: %s: uid/gid error", state->path, path));
			return -1;
		}
	}
	return fd;
 bad:
	if (nfd >= 0)
		close(nfd);
	return -1;
}

int
_cs_open(const char* apath, int op)
{
	return csopen(&cs, apath, op);
}
