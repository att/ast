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
 * connect stream library implementation definitions
 */

#ifndef _CSLIB_H
#define _CSLIB_H

#define CS_INTERFACE	2		/* thread safe Cs_t* 1st arg	*/

#define CS_AUTH_MASK	(S_ISUID|S_ISGID|S_IRWXU|S_IRWXG|S_IRWXO)
#define CS_AUTH_MODE	(S_ISUID|S_ISGID|S_IRUSR)

typedef struct
{
	unsigned long	addr;
	unsigned long	port;
} Inet_t;

struct Server_s;
typedef struct Server_s Server_t;

#define _CS_PRIVATE_ \
	Inet_t		proxy;		/* proxy inet addr.port 	*/ \
	int		db;		/* csdb() state			*/ \
	int		interrupt;	/* last interrupt		*/ \
	int		nostream;	/* cspeek() state		*/ \
	int		nosocket;	/* cspeek() state		*/ \
	Server_t*	server;		/* csserve() state		*/ \
	char		full[4 * CS_NAME_MAX]; /* csname() full buffer	*/ \
	char		name[CS_NAME_MAX]; /* csname() short buffer	*/ \
	char		ntoa[16];	/* csntoa() buffer		*/ \
	char		path[PATH_MAX];	/* lib work buffer		*/ \
	char		temp[CS_NAME_MAX];/* lib work buffer		*/

#include <cs.h>
#include <debug.h>
#include <errno.h>
#include <sig.h>
#include <tok.h>

#include <ast_tty.h>
#include <cs_lib.h>

#if CS_LIB_V10

#include <sys/filio.h>
#include <sys/inio.h>
#include <sys/inet/in.h>
#include <sys/inet/tcp_user.h>

#define I_ACCEPT	FIOACCEPT
#define I_RECVFD	FIORCVFD
#define I_SENDFD	FIOSNDFD

#define FDARG(f)	(&(f))

#define strrecvfd	passfd

extern int	conn_ld;

#else

#if CS_LIB_SOCKET

#include <sys/socket.h>

#if CS_LIB_SOCKET_UN

#if _sys_sockio
#include <sys/sockio.h>
#else
#include <sys/ioctl.h>
#endif
#if _sys_uio && !defined(MAX_IOVEC)
#include <sys/uio.h>
#endif
#if _sys_un
#include <sys/un.h>
#endif

#endif

#if _hdr_netinet_in
#include <netinet/in.h>
#endif
#if _hdr_netinet_tcp
#include <netinet/tcp.h>
#endif
#if _hdr_netdb
#include <netdb.h>
#endif

#endif

#if CS_LIB_STREAM

#include <stropts.h>

#define FDARG(f)	(f)

#endif

#endif

#if CS_LIB_STREAM || CS_LIB_V10

struct csfdhdr				/* send/recv fd data header	*/
{
	long	count;
	long	pid;
};

#endif

#ifndef IPPORT_RESERVED
#define IPPORT_RESERVED		1024
#endif
#ifndef IPPORT_USERRESERVED
#define IPPORT_USERRESERVED	5000
#endif

#if !defined(htons) && !_lib_htons
#define htons(x)	(x)
#endif
#if !defined(htonl) && !_lib_htonl
#define htonl(x)	(x)
#endif
#if !defined(ntohs) && !_lib_ntohs
#define ntohs(x)	(x)
#endif
#if !defined(ntohl) && !_lib_ntohl
#define ntohl(x)	(x)
#endif

#if defined(F_GETFL) && defined(F_SETFL)
#if !defined(FAPPEND)
#if _sys_file
#include <sys/file.h>
#endif
#if !defined(FAPPEND) && defined(O_APPEND)
#define FAPPEND	O_APPEND
#endif
#endif
#endif
#if defined(F_GETFL) && defined(F_SETFL) && defined(FAPPEND)
#define setauth(f,t)	((t=fcntl(f,F_GETFL,0))>=0&&fcntl(f,F_SETFL,t|FAPPEND)>=0)
#define getauth(f,t)	((t=fcntl(f,F_GETFL,0))>=0&&(t&FAPPEND))
#else
#define setauth(f,t)	(0)
#define getauth(f,t)	(0)
#endif

#if !defined(SIGCHLD) && defined(SIGCLD)
#define SIGCHLD		SIGCLD
#endif

#ifdef SIGPIPE
#if _lib_sigblock
#define csprotect(h)	do if (!((h)->flags & CS_PIPE_BLOCKED)) { (h)->flags |= CS_PIPE_BLOCKED; sigblock(sigmask(SIGPIPE)); errorf((h)->id, NiL, -99, "protect"); } while (0)
#else
#define csprotect(h)	do if (!((h)->flags & CS_PIPE_BLOCKED)) { Handler_t f; (h)->flags |= CS_PIPE_BLOCKED; if ((f = signal(SIGPIPE, SIG_IGN)) != SIG_DFL) signal(SIGPIPE, f); errorf((h)->id, NiL, -99, "protect"); } while (0)
#endif
#else
#define csprotect(h)	do if (!((h)->flags & CS_PIPE_BLOCKED)) { (h)->flags |= CS_PIPE_BLOCKED; errorf((h)->id, NiL, -99, "protect"); } while (0)
#endif

#define CS_SVC_INET	"inet."

#define CS_PROC_FD_TST	"/proc/self/fd/."
#define CS_PROC_FD_FMT	"/proc/%lu/fd/%u"

#define CS_KEY_SEND	0
#define CS_KEY_CLONE	1
#define CS_KEY_MAX	9

#ifndef ENOTDIR
#define ENOTDIR	ENOENT
#endif
#ifndef EROFS
#define EROFS	EACCES
#endif

#ifndef errno
extern int		errno;
#endif

#if CS_LIB_SOCKET
#define csdb(h)		do{if((h)->db<=0)cssetdb(h);}while(0)
#else
#define csdb(h)
#endif
#define cssetdb		_cs_setdb

extern void		cssetdb(Cs_t*);

#if CS_LIB_V10
extern int		fmount(int, int, const char*, int);
#endif
#if CS_LIB_STREAM
extern int		fattach(int, const char*);
#endif

#endif
