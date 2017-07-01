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
 * AT&T Bell Laboratories
 *
 * cs remote shell service
 * no tty/pty but good enough for nt
 * if its the only way in
 */

static const char id[] = "@(#)$Id: cs.rsh (AT&T Bell Laboratories) 1995-10-13 $\0\n";

#include <cs.h>
#include <proc.h>
#include <wait.h>

static int
svc_connect(void* handle, int fd, Cs_id_t* id, int clone, char** argv)
{
	Proc_t*		p;
	int		n;
	long		ops[4];

	static char*	args[] = { "sh", "-i", 0 };

	NoP(handle);
	NoP(clone);
	waitpid(-1, NiL, WNOHANG);
	n = 0;
	ops[n++] = PROC_FD_DUP(fd, 0, 0);
	ops[n++] = PROC_FD_DUP(fd, 1, 0);
	ops[n++] = PROC_FD_DUP(fd, 2, PROC_FD_CHILD);
	ops[n] = 0;
	if (!(p = procopen(NiL, args, NiL, ops, 0)))
		return(-1);
	procfree(p);
	csfd(fd, CS_POLL_CLOSE);
	return(0);
}

int
main(int argc, char** argv)
{
	NoP(argc);
	csserve(NiL, argv[1], NiL, NiL, svc_connect, NiL, NiL, NiL);
	exit(1);
}
