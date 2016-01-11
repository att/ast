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
 * return a pointer to a pathname for an open fd
 */

#include "cslib.h"

#include <ast_dir.h>

/*
 * someone thinks FIFO and SOCK are the same (bsd, eh?)
 */

#if defined(S_IFMT) && defined(S_IFIFO)
#undef	S_ISFIFO
#define S_ISFIFO(m)	(((m)&S_IFMT)==S_IFIFO)
#endif

/*
 * scan /dev for path matching st
 */

static char*
devpath(char* path, int size, int blk, register struct stat* st)
{
	register DIR*		dirp;
	register struct dirent*	entry;
	DIR*			subp = 0;
	char*			base;
	int			n;
	int			t;
	struct stat		tst;

	strcpy(path, "/dev");
	if (!(dirp = opendir(path))) return 0;
	path[4] = '/';
	base = path + 5;
	for (n = 1;;)
	{
		while (entry = readdir(dirp))
		{
#ifdef D_FILENO
			if (n && D_FILENO(entry) != st->st_ino)
				continue;
#endif
			if (*entry->d_name == '.' || D_NAMLEN(entry) + (base - path) + 1 > size)
				continue;
			strcpy(base, entry->d_name);
			if (stat(path, &tst))
				continue;
			if (!subp && S_ISDIR(tst.st_mode) && !streq(path, "/dev/fd"))
			{
				subp = dirp;
				if (dirp = opendir(path))
				{
					base = path + strlen(path);
					*base++ = '/';
				}
				else
				{
					dirp = subp;
					subp = 0;
				}
				continue;
			}
#ifndef D_FILENO
			if (n && tst.st_ino != st->st_ino)
				continue;
#endif
			if (idevice(&tst) == idevice(st) && ((t = S_ISBLK(tst.st_mode)) || S_ISCHR(tst.st_mode)) && t == blk && (!n || tst.st_dev == st->st_dev && tst.st_ino == st->st_ino))
			{
				closedir(dirp);
				if (subp) closedir(subp);
				return path;
			}
		}
		if (subp)
		{
			closedir(dirp);
			dirp = subp;
			subp = 0;
			base = path + 5;
		}
		else if (!n--) break;
		else rewinddir(dirp);
	}
	closedir(dirp);
	return 0;
}

/*
 * return a pointer to a pathname for an open fd
 */

char*
cspath(register Cs_t* state, register int fd, int flags)
{
	register char*		s;
	char			num[16];
	struct stat		st;
	int			typ;

#if CS_LIB_V10

	struct tcpuser		tcp;

#else

#if CS_LIB_SOCKET

	struct sockaddr_in	lcl;
	struct sockaddr_in	rmt;
	Sock_size_t		namlen = sizeof(lcl);
#ifdef SO_TYPE
	Sock_size_t		typlen = sizeof(typ);
#endif

#endif

#endif

	if (fstat(fd, &st)) return "/dev/fd/-1";
	else if (S_ISFIFO(st.st_mode)) sfsprintf(state->path, sizeof(state->path), "/dev/pipe/%u", st.st_ino);
#if CS_LIB_V10
	else if (!ioctl(fd, TCPGETADDR, &tcp))
	{
		if (tcp.raddr) sfsprintf(state->path, sizeof(state->path), "/dev/tcp/%s/%d.%d", (flags & CS_PATH_NAME) ? csname(state, tcp.raddr) : csntoa(state, tcp.raddr), ntohs(tcp.rport), ntohs(tcp.lport));
		else
		{
			if (!tcp.laddr) tcp.laddr = csaddr(state, NiL);
			sfsprintf(state->path, sizeof(state->path), "/dev/tcp/%s/%d", (flags & CS_PATH_NAME) ? csname(state, tcp.laddr) : csntoa(state, tcp.laddr), ntohs(tcp.lport));
		}
	}
#endif
#if CS_LIB_SOCKET
	else if (!getsockname(fd, (struct sockaddr*)&lcl, &namlen) && namlen == sizeof(lcl) && lcl.sin_family == AF_INET)
	{
		s = "tcp";
#ifdef SO_TYPE
		if (!getsockopt(fd, SOL_SOCKET, SO_TYPE, (char*)&typ, &typlen)) switch (typ)
		{
		case SOCK_DGRAM:
			s = "udp";
			break;
		case SOCK_STREAM:
			break;
		default:
			sfsprintf(s = num, sizeof(num), "%d.p", typ);
			break;
		}
#endif
		namlen = sizeof(rmt);
		if (!getpeername(fd, (struct sockaddr*)&rmt, &namlen) && namlen == sizeof(rmt) && rmt.sin_family == AF_INET)
		{
			if (!rmt.sin_addr.s_addr) rmt.sin_addr.s_addr = csaddr(state, NiL);
			sfsprintf(state->path, sizeof(state->path), "/dev/%s/%s/%d.%d", s, (flags & CS_PATH_NAME) ? csname(state, (unsigned long)rmt.sin_addr.s_addr) : csntoa(state, (unsigned long)rmt.sin_addr.s_addr), ntohs((unsigned short)rmt.sin_port), ntohs((unsigned short)lcl.sin_port));
		}
		else
		{
			if (!lcl.sin_addr.s_addr) lcl.sin_addr.s_addr = csaddr(state, NiL);
			sfsprintf(state->path, sizeof(state->path), "/dev/%s/%s/%d", s, (flags & CS_PATH_NAME) ? csname(state, (unsigned long)lcl.sin_addr.s_addr) : csntoa(state, (unsigned long)lcl.sin_addr.s_addr), ntohs((unsigned short)lcl.sin_port));
		}
	}
#endif
	else if ((typ = S_ISBLK(st.st_mode)) || S_ISCHR(st.st_mode))
	{
		if (s = devpath(state->path, sizeof(state->path), typ, &st))
			return s;
		sfsprintf(state->path, sizeof(state->path), "/dev/%s-%u,%u", typ ? "blk" : "chr", major(idevice(&st)), minor(idevice(&st)));
	}
	else
		sfsprintf(state->path, sizeof(state->path), "/dev/ino/%u/%u", st.st_dev, st.st_ino);
	return state->path;
}

char*
_cs_path(int fd, int flags)
{
	return cspath(&cs, fd, flags);
}
