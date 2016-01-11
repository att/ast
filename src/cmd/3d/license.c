/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * 3d license via agent to license server
 */

#include "3d.h"

#if LICENSED

#define LICENSE_OPEN(s,m,l)	licopen(s,m,l)
#define LICENSE_CLOSE(f)	close(f)

static int
licopen(const char* svc, const char* msg, int len)
{
	char*	service;
	char*	env;
	int	n;
	int	fd;
	int	lic;
	char	buf[64];

	if (state.in_2d) return(-1);
	service = "/dev/fdp/local/nam/user";
	env = *environ;
	*environ = "_3D_DISABLE=1";
	if ((fd = cslocal(&cs, service)) < 0)
	{
		error(ERROR_SYSTEM|2, "%s: cannot connect to server", service);
		return(-1);
	}
	*environ = env;
	n = strlen(svc) + 1;
	if (cswrite(&cs, fd, svc, n) != n)
	{
		error(ERROR_SYSTEM|2, "%s: cannot write to server", service);
		close(fd);
		return(-1);
	}
	if (read(fd, buf, sizeof(buf)) <= 0)
	{
		error(ERROR_SYSTEM|2, "%s: cannot read from server", service);
		close(fd);
		return(-1);
	}
	if (csrecv(&cs, fd, NiL, &lic, 1) != 1)
	{
		error(ERROR_SYSTEM|2, "%s: cannot connect to %s", service, svc);
		close(fd);
		return(-1);
	}
	close(fd);
	if (cswrite(&cs, lic, msg, len) != len)
	{
		error(ERROR_SYSTEM|2, "%s: cannot write to service", svc);
		close(lic);
		return(-1);
	}
	fcntl(lic, F_SETFD, FD_CLOEXEC);
	state.in_2d = 0;
	return(lic);
}

#define tokscan		_3d_tokscan

#include "tokscan.c"
#include "../license/service.c"

#else

NoN(license)

#endif
