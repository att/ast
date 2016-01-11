/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2012 AT&T Intellectual Property          *
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
 * OBSOLETE: use spawnvex()
 */

#include <ast.h>

pid_t
spawnveg(const char* path, char* const argv[], char* const envv[], pid_t pgid)
{
	Spawnvex_t*	vex;
	pid_t		pid;

	if (!pgid)
		vex = 0;
	else if (!(vex = spawnvex_open(0)))
		return -1;
	else
		spawnvex_add(vex, SPAWN_pgrp, pgid > 1 ? pgid : 0, 0, 0);
	pid = spawnvex(path, argv, envv, vex);
	if (vex)
		spawnvex_close(vex);
	return pid;
}
