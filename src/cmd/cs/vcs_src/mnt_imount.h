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
#include <ast.h>
#include <cs.h>
#include <error.h>
#include <hash.h>
#include <pwd.h>
#include <stdio.h>

struct istate_t 
{
	char*		cs_svc;
	int		fd;
	HASHTABLE*	mtab;
};

int vcs_read();
int vcs_write();
void printmtmsg();

extern struct istate_t istate;

