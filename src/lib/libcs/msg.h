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
 * syscall message interface
 */

#ifndef _MSG_H
#define _MSG_H

#include <ast.h>

#define EMSGIO		(-1)

#define MSG_VERSION	1

#define MSG_TIMEOUT	8000

#define MSG_ARG_number	1
#define MSG_ARG_string	2
#define MSG_ARG_file	3
#define MSG_ARG_input	4
#define MSG_ARG_output	5
#define MSG_ARG_array	6
#define MSG_ARG_vector	7

#define MSG_ARG_CALL	11
#define MSG_ARG_INDEX	5
#define MSG_ARG_TYPE	3
#define MSG_ARG(c,n)	(((c)>>(MSG_ARG_CALL+MSG_ARG_TYPE*(n)))&((1<<MSG_ARG_TYPE)-1))
#define MSG_EXT(t,n)	((t)<<(MSG_ARG_CALL+MSG_ARG_TYPE*(n)))

#define MSG_CHANNEL_SYS(n)	((n)&((1<<16)-1))
#define MSG_CHANNEL_USR(n)	(((n)>>16)&((1<<16)-1))
#define MSG_CHANNEL(u,s)	(((u)<<16)|((s)&((1<<16)-1)))

#define MSG_LIST_ID	(1<<0)		/* list channel id		*/
#define MSG_LIST_STAMP	(1<<1)		/* include time stamp		*/
#define MSG_LIST_USER	(1<<8)		/* first user bit		*/

#define MSG_SIZE_ACK	4
#define MSG_SIZE_ARG	6
#define MSG_SIZE_BITS	8
#define MSG_SIZE_BUF	4096
#define MSG_SIZE_IO	(MSG_SIZE_BUF-128)
#define MSG_SIZE_SIZE	2

#define MSG_VAR_FILE	(1<<MSG_ARG_INDEX)	/* file variant		*/
#define MSG_VAR_IPC	(2<<MSG_ARG_INDEX)	/* ipc variant		*/
#define MSG_VAR_SYM	(3<<MSG_ARG_INDEX)	/* symlink variant	*/

#define MSG_ACK		(1<<(MSG_ARG_INDEX+2))	/* msgcall() ack	*/
#define MSG_FLUSH	(1<<(MSG_ARG_INDEX+3))	/* msgcall() msg flush	*/
#define MSG_RETURN	(1<<(MSG_ARG_INDEX+4))	/* msgcall()+msgreturn()*/
#define MSG_VALUE	(1<<(MSG_ARG_INDEX+5))	/* args have value	*/

#define MSG_CALL(c)	((c)&((1<<MSG_ARG_INDEX)-1))
#define MSG_MASK(c)	(1<<MSG_CALL(c))
#define MSG_PURE(c)	((c)&~(((1<<MSG_ARG_CALL)-1)&~((1<<MSG_ARG_INDEX)-1)))
#define MSG_VAR(c)	((c)&(3<<MSG_ARG_INDEX))
#define MSG_SYS(c)	((c)&((1<<(MSG_ARG_INDEX+2))-1))

#define MSG_INIT(c,a,v)	(MSG_CALL(c)|((a)<<MSG_ARG_CALL)|(v))

#define MSG_nop		MSG_INIT( 0, 00000000, 0)
#define MSG_break	MSG_INIT( 1, 00000011, 0)
#define MSG_chmod	MSG_INIT( 2, 00000121, MSG_VAR_FILE)
#define MSG_chown	MSG_INIT( 3, 00001121, MSG_VAR_FILE)
#define MSG_close	MSG_INIT( 4, 00000131, 0)
#define MSG_control	MSG_INIT( 5, 00001521, 0)
#define MSG_dup		MSG_INIT( 6, 00000033, 0)
#define MSG_exec	MSG_INIT( 7, 00007721, MSG_VAR_FILE)
#define MSG_exit	MSG_INIT( 8, 00000011, 0)
#define MSG_fork	MSG_INIT( 9, 00000001, 0)
#define MSG_getdents	MSG_INIT(10, 00001531, 0)
#define MSG_kill	MSG_INIT(11, 00000111, 0)
#define MSG_link	MSG_INIT(12, 00000221, MSG_VAR_FILE)
#define MSG_lock	MSG_INIT(13, 00011131, 0)
#define MSG_mkdir	MSG_INIT(14, 00000121, MSG_VAR_FILE)
#define MSG_mknod	MSG_INIT(15, 00001121, MSG_VAR_FILE)
#define MSG_mount	MSG_INIT(16, 00112221, 0)
#define MSG_open	MSG_INIT(17, 00011123, MSG_VAR_FILE)
#define MSG_pathconf	MSG_INIT(18, 00000121, MSG_VAR_FILE)
#define MSG_pipe	MSG_INIT(19, 00000051, 0)
#define MSG_read	MSG_INIT(20, 00001531, 0)
#define MSG_remove	MSG_INIT(21, 00000021, MSG_VAR_FILE)
#define MSG_rename	MSG_INIT(22, 00000221, MSG_VAR_FILE)
#define MSG_rmdir	MSG_INIT(23, 00000021, MSG_VAR_FILE)
#define MSG_seek	MSG_INIT(24, 00001131, 0)
#define MSG_stat	MSG_INIT(25, 00000521, MSG_VAR_FILE)
#define MSG_statfs	MSG_INIT(26, 00000521, MSG_VAR_FILE)
#define MSG_sync	MSG_INIT(27, 00000001, MSG_VAR_FILE)
#define MSG_truncate	MSG_INIT(28, 00000121, MSG_VAR_FILE)
#define MSG_utime	MSG_INIT(29, 00001121, MSG_VAR_FILE)
#define MSG_wait	MSG_INIT(30, 00001511, 0)
#define MSG_write	MSG_INIT(31, 00001431, 0)

#define MSG_STD		31

/* common fd variants */

#define MSG_fchmod	MSG_INIT(MSG_chmod,	00000131, MSG_VAR_FILE)
#define MSG_fchown	MSG_INIT(MSG_chown,	00001131, MSG_VAR_FILE)
#define MSG_fpathconf	MSG_INIT(MSG_pathconf,	00000131, MSG_VAR_FILE)
#define MSG_fstat	MSG_INIT(MSG_stat,	00000531, MSG_VAR_FILE)
#define MSG_fstatfs	MSG_INIT(MSG_statfs,	00000531, MSG_VAR_FILE)
#define MSG_fsync	MSG_INIT(MSG_sync,	00000031, MSG_VAR_FILE)
#define MSG_ftruncate	MSG_INIT(MSG_truncate,	00000131, MSG_VAR_FILE)

/* *at() variants */

#define MSG_fstatat	MSG_INIT(MSG_stat,	00005311, MSG_VAR_FILE)
#define MSG_openat	MSG_INIT(MSG_open,	00011231, MSG_VAR_FILE)
#define MSG_unlinkat	MSG_INIT(MSG_remove,	00011231, 0)

/* common ipc variants */

#define MSG_accept	MSG_INIT(MSG_dup,	00005533, MSG_VAR_IPC)
#define MSG_bind	MSG_INIT(MSG_rename,	00001531, MSG_VAR_IPC)
#define MSG_connect	MSG_INIT(MSG_link,	00001531, MSG_VAR_IPC)
#define MSG_recv	MSG_INIT(MSG_read,	00011531, MSG_VAR_IPC)
#define MSG_recvmsg	MSG_INIT(MSG_read,	05511531, MSG_VAR_IPC)
#define MSG_send	MSG_INIT(MSG_write,	00011431, MSG_VAR_IPC)
#define MSG_sendmsg	MSG_INIT(MSG_write,	01411431, MSG_VAR_IPC)
#define MSG_socket	MSG_INIT(MSG_open,	00001113, MSG_VAR_IPC)

/* common symbolic variants */

#define MSG_lstat	MSG_INIT(MSG_stat,	00000521, MSG_VAR_SYM)
#define MSG_readlink	MSG_INIT(MSG_read,	00001521, MSG_VAR_SYM)
#define MSG_symlink	MSG_INIT(MSG_link,	00000221, MSG_VAR_SYM)

typedef struct
{
	long		fid[2];
} Msg_file_t;

typedef union
{
	long*		array;
	Msg_file_t*	file;
	unsigned long	number;
	char*		pointer;
	char*		string;
	char**		vector;
} Msg_arg_t;

typedef union
{
	Msg_file_t	file;
	unsigned long	number;
} Msg_return_t;

typedef struct
{
	char		data[MSG_SIZE_BUF];
	char		value[MSG_SIZE_BUF];
	int		version;
	unsigned long	call;
	size_t		size;
	unsigned long	channel;
	unsigned long	stamp;
	struct
	{
	unsigned long	addr;
	unsigned long	port;
	}		ack;
	int		argc;
	Msg_arg_t	argv[MSG_SIZE_ARG];
	Msg_return_t	ret;
} Msg_call_t;

typedef struct
{
	int		timeout;
	int		timestamp;
	const char*	name[MSG_STD + 1];
} Msg_info_t;

#define msg_info	_msg_info_

#if _BLD_cs && defined(__EXPORT__)
#define extern		__EXPORT__
#endif
#if !_BLD_cs && defined(__IMPORT__)
#define extern		extern __IMPORT__
#endif

extern Msg_info_t	msg_info;

#undef	extern

#ifndef msgreserve
#define msgreserve(p)
#endif

#define msggetsize(p)	((((p)[0]&((1<<MSG_SIZE_BITS)-1))<<MSG_SIZE_BITS)|((p)[1]&((1<<MSG_SIZE_BITS)-1)))

#define msgsetsize(p,n)	((p)[0]=((n)>>MSG_SIZE_BITS)&((1<<MSG_SIZE_BITS)-1),(p)[1]=(n)&((1<<MSG_SIZE_BITS)-1))

#if _BLD_cs && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int		msggetmask(char*, int, unsigned long);
extern int		msgindex(const char*);
extern int		msglist(Sfio_t*, Msg_call_t*, int, unsigned long);
extern const char*	msgname(unsigned long);
extern unsigned long	msgsetmask(const char*);

extern unsigned long	msggetu(char**, char*);
extern int		msgputu(char**, char*, unsigned long);
extern size_t		msggetz(char**, char*, void*, size_t);
extern int		msgputz(char**, char*, void*, size_t);

extern ssize_t		msgcall(int, unsigned long, unsigned long, Msg_return_t*, ...);
extern ssize_t		msgvcall(int, unsigned long, unsigned long, Msg_return_t*, void**, va_list);
extern long		msgreturn(int, unsigned long, ...);
extern long		msgvreturn(int, unsigned long, void**, va_list);

extern ssize_t		msgblast(Msg_call_t*);
extern ssize_t		msgread(int, char*, size_t);
extern ssize_t		msgrecv(int, Msg_call_t*);
extern ssize_t		msgsend(int, Msg_call_t*, unsigned long, long, int, void*);

#undef	extern

#endif
