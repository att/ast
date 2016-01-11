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
 * syscall message server side receipt
 */

#include "msglib.h"

/*
 * read and blast message from fd into msg
 */

ssize_t
msgrecv(int fd, register Msg_call_t* msg)
{
	register ssize_t	n;

	if ((n = msgread(fd, msg->data, sizeof(msg->data))) <= 0)
		return n;
	msg->size = n;
	return msgblast(msg);
}
