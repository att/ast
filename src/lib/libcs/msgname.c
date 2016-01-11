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

#include "msglib.h"

/*
 * return name given call
 */

const char*
msgname(register unsigned long call)
{
	register int		n;

	static char		buf[12];

	if ((n = MSG_CALL(call)) > MSG_STD)
	{
		sfsprintf(buf, sizeof(buf), "user_%d", n);
		return (const char*)buf;
	}
	switch (MSG_VAR(call))
	{
	case MSG_VAR_FILE:
		if (MSG_ARG(call, 1) == MSG_ARG_file)
		{
			sfsprintf(buf, sizeof(buf), "f%s", msg_info.name[n]);
			return (const char*)buf;
		}
		break;
	case MSG_VAR_IPC:
		sfsprintf(buf, sizeof(buf), "ipc%s", msg_info.name[n]);
		return (const char*)buf;
	case MSG_VAR_SYM:
		sfsprintf(buf, sizeof(buf), "sym%s", msg_info.name[n]);
		return (const char*)buf;
	}
	return msg_info.name[n];
}
