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
 * system call message data initialization
 */

#include "msglib.h"

Msg_info_t	msg_info =
{
	MSG_TIMEOUT,
	0,
	{
	"NOP",
	"break",	"chmod",	"chown",	"close",
	"control",	"dup",		"exec",		"exit",
	"fork",		"getdents",	"kill",		"link",
	"lock",		"mkdir",	"mknod",	"mount",
	"open",		"pathconf",	"pipe",		"read",
	"remove",	"rename",	"rmdir",	"seek",
	"stat",		"statfs",	"sync",		"truncate",
	"utime",	"wait",		"write",
	}
};
