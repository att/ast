/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * open canonicalized path with checks for pathdev() files
 * canon==0 to disable canonicalization
 */

#include <ast.h>
#include <error.h>
#if _sys_socket
#include <sys/socket.h>
#endif
#if _hdr_netdb
#include <netdb.h>
#endif
#if _hdr_netinet_in
#include <netinet/in.h>
#endif

#ifndef O_XATTR
#define O_XATTR		0
#endif

#if !_lib_getaddrinfo

#if !defined(htons) && !_lib_htons
#define htons(x)		(x)
#endif
#if !defined(htonl) && !_lib_htonl
#define htonl(x)		(x)
#endif

#undef	EAI_SYSTEM

#define EAI_SYSTEM		1

#undef	addrinfo
#undef	getaddrinfo
#undef	freeaddrinfo

#define addrinfo		local_addrinfo
#define getaddrinfo		local_getaddrinfo
#define freeaddrinfo		local_freeaddrinfo

struct addrinfo
{
        int			ai_flags;
        int			ai_family;
        int			ai_socktype;
        int			ai_protocol;
        socklen_t		ai_addrlen;
        struct sockaddr*	ai_addr;
        struct addrinfo*	ai_next;
};

static int
getaddrinfo(const char* node, const char* service, const struct addrinfo* hint, struct addrinfo **addr)
{
	unsigned long	    	ip_addr = 0;
	unsigned short	    	ip_port = 0;
	struct addrinfo*	ap;
	struct hostent*		hp;
	struct sockaddr_in*	ip;
	char*			prot;
	long			n;
	
	if (!(hp = gethostbyname(node)) || hp->h_addrtype != AF_INET || hp->h_length > sizeof(struct in_addr))
	{
		errno = EADDRNOTAVAIL;
		return EAI_SYSTEM;
	}
	ip_addr = (unsigned long)((struct in_addr*)hp->h_addr)->s_addr;
	if ((n = strtol(service, &prot, 10)) > 0 && n <= USHRT_MAX && !*prot)
		ip_port = htons((unsigned short)n);
	else
	{
		struct servent*	sp;
		const char*	protocol = 0;

		if (hint)
			switch (hint->ai_socktype)
			{
			case SOCK_STREAM:
				switch (hint->ai_protocol)
				{
				case 0: 	  
					protocol = "tcp";
					break;
#ifdef IPPROTO_SCTP
				case IPPROTO_SCTP:
					protocol = "sctp";
					break;
#endif
				}
				break;
			case SOCK_DGRAM:
				protocol = "udp";
				break;
			}
		if (!protocol)
		{
			errno =  EPROTONOSUPPORT;
			return 1;
		}
		if (sp = getservbyname(service, protocol))
			ip_port = sp->s_port;
	}
	if (!ip_port)
	{
		errno = EADDRNOTAVAIL;
		return EAI_SYSTEM;
	}
	if (!(ap = newof(0, struct addrinfo, 1, sizeof(struct sockaddr_in))))
		return EAI_SYSTEM;
	if (hint)
		*ap = *hint;
	ap->ai_family = hp->h_addrtype;
	ap->ai_addrlen 	= sizeof(struct sockaddr_in);
	ap->ai_addr = (struct sockaddr *)(ap+1);
	ip = (struct sockaddr_in *)ap->ai_addr;
	ip->sin_family = AF_INET;
	ip->sin_port = ip_port;
	ip->sin_addr.s_addr = ip_addr;
	*addr = ap;
	return 0;
}

static void
freeaddrinfo(struct addrinfo* ap)
{
	if (ap)
		free(ap);
}

#endif

int
pathopen(int dfd, const char* path, char* canon, size_t size, int flags, int oflags, mode_t mode)
{
	char*		b;
	Pathdev_t	dev;
	int		f;
	int		fd;
	int		oerrno;

	b = canon ? canon : (char*)path;
	if (!pathdev(dfd, path, canon, size, flags, &dev))
		return 0;
	if (dev.path.offset)
	{
		oerrno = errno;
		oflags |= dev.oflags;
		if (dev.fd >= 0)
		{
			if (dev.pid > 0 && dev.pid != getpid())
			{
				if (flags & PATH_DEV)
					return access(b, F_OK) ? -1 : 1;
				return openat(AT_FDCWD, b, oflags|O_INTERCEPT, mode);
			}

			/* check for auxilliary directory fd which must be closed before returning */

			if (dev.oflags & O_INTERCEPT)
			{
				if (!*(b += dev.path.offset))
					b = ".";
				if (flags & PATH_DEV)
					f = faccessat(dev.fd, b, F_OK, 0) ? -1 : 1;
				else
					f = openat(dev.fd, b, oflags, mode);
				close(dev.fd);
				return f;
			}

			/* prevent getting here again with this path */

			oflags |= O_INTERCEPT;

			/* dev.fd must be valid */

			if ((f = fcntl(dev.fd, F_GETFL, 0)) < 0)
				return -1;
			if (flags & PATH_DEV)
			{
				if (!b[dev.path.offset])
					return 1;
				return faccessat(dev.fd, b + dev.path.offset, F_OK, 0) < 0 ? -1 : 1;
			}

			/* a trailing path component means dev.fd must be a directory -- easy */

			if (b[dev.path.offset])
				return openat(dev.fd, b + dev.path.offset, oflags, mode);

			/* the path boils down to just dev.fd -- F_GETFL must match oflags */

			if (!(f & O_RDWR) && (f & O_ACCMODE) != (oflags & O_ACCMODE))
			{
				errno = EACCES;
				return -1;
			}

			/* preserve open() semantics if possible */

			if (oflags & (O_DIRECTORY|O_SEARCH|O_XATTR))
				return openat(dev.fd, ".", oflags|O_INTERCEPT, mode);
#if O_XATTR
			if ((f = openat(dev.fd, ".", O_INTERCEPT|O_RDONLY|O_XATTR)) >= 0)
			{
				fd = openat(f, "..", oflags|O_INTERCEPT, mode);
				close(f);
				return fd;
			}
#endif

			/* see if the filesystem groks .../[<pid>]/<fd>/... paths */

			if ((!(oflags & O_CREAT) || !access(b, F_OK)) && (fd = openat(AT_FDCWD, b, oflags|O_INTERCEPT, mode)) >= 0)
				return fd;

			/* stuck with dup semantics -- the best we can do at this point */

			if ((fd = fcntl(dev.fd, (oflags & O_CLOEXEC) ? F_DUPFD_CLOEXEC : F_DUPFD, 0)) >= 0)
				goto adjust;
			return -1;
		}
		else if (dev.fd == AT_FDCWD)
			return (flags & PATH_DEV) ? 1 : -1;
		else if (dev.prot.offset)
		{
			int			server = (oflags&(O_CREAT|O_NOCTTY)) == (O_CREAT|O_NOCTTY);
			int			prot;
			int			type;
			char*			p;
			char*			q;
			struct addrinfo		hint;
			struct addrinfo*	addr;
			struct addrinfo*	a;
		
			memset(&hint, 0, sizeof(hint));
			hint.ai_family = PF_UNSPEC;
			p = b + dev.prot.offset;
			switch (p[0])
			{
#ifdef IPPROTO_SCTP
			case 's':
				if (dev.prot.size != 4 || p[1] != 'c' || p[2] != 't' || p[3] != 'p' || p[4] != '/')
				{
					errno = ENOTDIR;
					return -1;
				}
				hint.ai_socktype = SOCK_STREAM;
				hint.ai_protocol = IPPROTO_SCTP;
				break;
#endif
			case 't':
				if (dev.prot.size != 3 || p[1] != 'c' || p[2] != 'p' || p[3] != '/')
				{
					errno = ENOTDIR;
					return -1;
				}
				hint.ai_socktype = SOCK_STREAM;
				break;
			case 'u':
				if (dev.prot.size!=3 || p[1]!='d' || p[2]!='p' || p[3]!='/')
				{
					errno = ENOTDIR;
					return -1;
				}
				hint.ai_socktype = SOCK_DGRAM;
				break;
			default:
				errno = ENOTDIR;
				return -1;
			}
			if (!dev.host.offset)
			{
				if (flags & PATH_DEV)
					return 1;
				errno = ENOENT;
				return -1;
			}
			if (!dev.port.offset)
				dev.port.size = 0;
			p = fmtbuf(dev.host.size + dev.port.size + 2);
			memcpy(p, b + dev.host.offset, dev.host.size);
			q = p + dev.host.size;
			*q++ = 0;
			if (dev.port.size)
			{
				memcpy(q, b + dev.port.offset, dev.port.size);
				q[dev.port.size] = 0;
			}
			else
				q = 0;
			if (streq(p, "local"))
				p = "localhost";
			fd = getaddrinfo(p, q, &hint, &addr);
			if (fd)
			{
				if (fd != EAI_SYSTEM)
					errno = ENOTDIR;
				return -1;
			}
			if (flags & PATH_DEV)
			{
				fd = 1;
				goto done;
			}
			errno = 0;
			fd = -1;
			for (a = addr; a; a = a->ai_next)
			{
				/* some api's don't take the hint */
		
				if (!(type = a->ai_socktype))
					type = hint.ai_socktype;
				if (oflags & O_CLOEXEC)
					type |= SOCK_CLOEXEC;
				if (!(prot = a->ai_protocol))
					prot = hint.ai_protocol;
				if ((fd = socket(a->ai_family, type, prot)) >= 0)
				{
					if (server && !bind(fd, a->ai_addr, a->ai_addrlen) && !listen(fd, 5) || !server && !connect(fd, a->ai_addr, a->ai_addrlen))
						goto done;
					close(fd);
					fd = -1;
				}
			}
		done:
			freeaddrinfo(addr);
			if (fd < 0)
				return -1;
		adjust:
			if (dev.oflags && (f = fcntl(fd, F_GETFL, 0)) >= 0 && (dev.oflags & f) != dev.oflags && fcntl(fd, F_SETFL, f|dev.oflags) < 0)
			{
				close(fd);
				errno = EINVAL;
				return -1;
			}
			errno = oerrno;
			return fd;
		}
		else
			b += dev.path.offset;
	}
	if (flags & PATH_DEV)
	{
		errno = ENODEV;
		return -1;
	}
	return openat(dfd, b, oflags|dev.oflags|O_INTERCEPT, mode);
}
