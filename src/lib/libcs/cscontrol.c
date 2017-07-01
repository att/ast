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
 * output control mark on fd
 */

#include "cslib.h"

int
cscontrol(register Cs_t* state, int fd)
{
#if CS_LIB_SOCKET
	return send(fd, "", 1, MSG_OOB) == 1 ? 0 : -1;
#else
#if CS_LIB_STREAM
	struct strbuf	buf;

	buf.maxlen = 0;
	return putmsg(fd, NiL, &buf, RS_HIPRI);
#else
	return write(fd, "", 1) == 1 ? 0 : -1;
#endif
#endif
}

int
_cs_control(int fd)
{
	return cscontrol(&cs, fd);
}
