/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2011 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#include "rshdr.h"

/*
 * create temp stream ready for write
 */

#if __STD_C
Sfio_t* rstempwrite(Rs_t* rs, Sfio_t* sp)
#else
Sfio_t* rstempwrite(rs, sp)
Rs_t*	rs;
Sfio_t*	sp;
#endif
{
	Sfio_t*		op = sp;

	if (!sp)
	{
#if _PACKAGE_ast
		char	path[PATH_MAX];
		int	fd;

		if (!pathtemp(path, sizeof(path), NiL, "sf", &fd))
			return 0;
		remove(path);
		if (!(sp = sfnew(NiL, NiL, SF_UNBOUND, fd, SF_READ|SF_WRITE)))
			return 0;
#else
		if (!(sp = sftmp(0)))
			return 0;
#endif
	}
	else
		sfresize(sp, 0);
	if ((rs->events & RS_TEMP_WRITE) && rsnotify(rs, RS_TEMP_WRITE, sp, (Void_t*)0, rs->disc) < 0)
	{
		if (!op)
			sfclose(sp);
		sp = 0;
	}
	return sp;
}

/*
 * rewind temp stream and prepare for read
 */

#if __STD_C
int rstempread(Rs_t* rs, Sfio_t* sp)
#else
int rstempread(rs, sp)
Rs_t*	rs;
Sfio_t*	sp;
#endif
{
	int	n;

	if (sfsync(sp))
		return -1;
	if (rs->events & RS_TEMP_READ)
	{
		if ((n = rsnotify(rs, RS_TEMP_READ, sp, (Void_t*)0, rs->disc)) < 0)
			return -1;
		if (n)
			return 0;
	}
	return sfseek(sp, (Sfoff_t)0, SEEK_SET) ? -1 : 0;
}

/*
 * close temp stream
 */

#if __STD_C
int rstempclose(Rs_t* rs, Sfio_t* sp)
#else
int rstempclose(rs, sp)
Rs_t*	rs;
Sfio_t*	sp;
#endif
{
	int	n;

	if (rs->events & RS_TEMP_CLOSE)
	{
		if ((n = rsnotify(rs, RS_TEMP_CLOSE, sp, (Void_t*)0, rs->disc)) < 0)
			return -1;
		if (n)
			return 0;
	}
	return sfclose(sp);
}
